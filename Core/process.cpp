#include "pch.h"

using bytes_buffer = std::vector<uint8_t>;

void process::start()
{
	const auto main_thread = std::make_shared<thread_item>();
	main_thread->evm2_thread = thread::factory::create_main_thread(code, memory);
	thread_table.push_back(main_thread);

	if (!binary_file_name.empty())
	{
		auto open_mode = std::ios::in | std::ios::out | std::ios::binary;
		if (!boost::filesystem::exists(binary_file_name))
			open_mode |= std::ios::trunc;		
		binary_file.open(binary_file_name, open_mode);
		binary_file.unsetf(std::ios::skipws);
	}
	
	run(0);
}

void process::run(uint64_t thread_id)
{
	const auto thread = thread_table[thread_id]->evm2_thread;
	while (can_run())
	{
		try
		{
			switch (const auto op_code = thread->run())
			{			
				case thread_create:
					thread->machine->arg1 = 
						create_thread(thread, thread->machine->decoder->instruction.address);
					break;

				case thread_join:
					join_thread(thread->machine->arg1);
					break;

				case read:
					thread->machine->Arg4 = file_read(
						thread->machine->arg1,
						thread->machine->arg2,
						thread->machine->arg3);
					break;

				case write:
					file_write(
						thread->machine->arg1,
						thread->machine->arg2,
						thread->machine->arg3);
					break;

				case con_read:
					thread->machine->arg1 = console_read();
					break;

				case con_write:
					console_write(thread->machine->arg1);
					break;

				case lock: 
					process_lock(thread->machine->arg1, thread_id);
					break;

				case unlock: 
					process_unlock(thread->machine->arg1, thread_id);
					break;

				case halt:
				case padding:
				case stopped:
					return hlt(thread_id);
				
				default:
					// default case means sth wasn't implemented or was bad refactored
					throw not_implemented_exception("Unimplemented instruction");
			}
		}	
		catch (...)
		{
			hlt(thread_id);
			throw;
		}
	}
	hlt(thread_id);
}

void process::hlt(uint64_t thread_ix)
{
	if (thread_ix == 0) // thread_ix 0 means main thread
		terminate();
}

void process::stop()
{
	stoppable_task::stop();
	if (thread_table[0] && thread_table[0]->evm2_thread)
		thread_table[0]->evm2_thread->stop();
}

void process::terminate() noexcept
{
	foreach_no_except(thread_table, [](auto thread) {
		// According to specification:
		// "If initial thread is ended, end whole program."
		if (thread && thread->evm2_thread)
			thread->evm2_thread->stop();
	});

	foreach_no_except(thread_table, [](auto thread) {
		if (thread && thread->std_thread)
			if (thread->std_thread->joinable())
				thread->std_thread->join();
	});

	foreach_no_except(thread_table, [](auto thread) {
		if (thread && thread->std_thread)
			thread->std_thread.reset();
		if (thread && thread->evm2_thread)
			thread->evm2_thread.reset();
	});

	try
	{
		if (binary_file.is_open())
			binary_file.close();
	}
	catch (...)	{}

}

int64_t process::create_thread(const std::shared_ptr<thread>& current_thread, uint32_t entry_point)
{
	auto new_thread_no = static_cast<uint64_t>(thread_table.size());
	const std::shared_ptr<thread_item> thread = std::make_shared<thread_item>();
	thread->evm2_thread = thread::factory::create_thread(current_thread, entry_point);
	thread->std_thread = nullptr;
	thread_table.push_back(thread);

	thread->std_thread = std::make_shared<std::thread>([this, new_thread_no]
		{
			this->run(new_thread_no);
		});

	return new_thread_no;
}

void process::join_thread(uint64_t thread_to_join)
{
	if (thread_to_join >= thread_table.size())
		throw out_of_range_exception("Invalid join thread argument");

	const auto thread = thread_table[thread_to_join];
	
	if (!thread || !thread->std_thread)
		throw not_implemented_exception("Threads should be joined once");
	
	if (thread->std_thread->joinable())
	{
		thread->std_thread->join();
		thread->std_thread.reset();
	}	
}

bool process::lock_exists(const uint64_t ix)
{
	return contains(lock_table, [ix](auto item) 
	{
		return item && item->index == ix;
	});
}

bool process::lock_create(const uint64_t lock_ix, const uint64_t thread_ix)
{
	// quick, unreliable check, because meanwhile other thread might create lock
	if (lock_exists(lock_ix))
		return false;

	std::lock_guard lock_guard(lock_table_mutex);
	if (lock_exists(lock_ix)) // reliable check
		return false;

	const auto new_lock = std::make_shared<lock_item>();
	new_lock->index = lock_ix;
	new_lock->thread_ix = thread_ix;
	new_lock->mutex.lock();
	lock_table.push_back(new_lock);
		
	return true;
}

void process::process_lock(const uint64_t lock_ix, const uint64_t thread_ix)
{
	if (lock_create(lock_ix, thread_ix))
		return;	
	/*
	 According to specification:
	 "It is an undefined behavior to lock same lock multiple times within same thread
	  – locks are not guaranteed to be re-entrant"

	   If you run script:
	   .\reentrantlock.ps1
	   you will get dead-lock after 1-5 seconds
	   To fix this initialise is_re_entrant_lock = true;
	*/
	
	bool is_re_entrant_lock; // purposely skipping variable initialization
	int64_t ix;
	for (ix = 0; ix < lock_table.size(); ix++)
		if (lock_table[ix] && lock_table[ix]->index == lock_ix)
		{
			if (lock_table[ix]->thread_ix == thread_ix)
				break; // getting "undefined behavior" of isReEntrantLock
			is_re_entrant_lock = false;
			break;
		}

	if (is_re_entrant_lock) // re-entrant lock not guaranteed
		return;

	const std::chrono::milliseconds nice_philosopher_wait_time(10);
	while (can_run())
		if (lock_table[ix]->mutex.try_lock_for(nice_philosopher_wait_time))
		{
			lock_table[ix]->thread_ix = thread_ix;
			break;
		}	
}

bool process::thread_holds_any_lock(const uint64_t thread_ix)
{
	return contains(lock_table,[thread_ix](auto item) {
		
		return item && item->thread_ix == static_cast<int64_t>(thread_ix);
		
	});
}

void process::process_unlock(const uint64_t lock_ix, const uint64_t thread_ix)
{
	for (auto& lock : lock_table)
		if (lock && lock->index == lock_ix)
		{
			if (lock->thread_ix >= 0)
			{
				lock->thread_ix = -1;
				lock->mutex.unlock();
			}						
			break;
		}

	// forcing philosophers to "think" after eating and stop fighting each other
	if (!thread_holds_any_lock(thread_ix))
	{
		constexpr std::chrono::milliseconds philosophers_think_time(50);
		std::this_thread::sleep_for(philosophers_think_time);
	}
}

int64_t process::console_read()
{
	std::lock_guard lock_guard(io_mutex);

	int64_t result = -1;
	if (input && !input->empty())
	{
		result = (*input)[0];
		input->erase(input->begin());
		return result;
	}
	std::cin >> std::hex >> result;
	return result;
}

void process::console_write(uint64_t number) 
{
	std::lock_guard lock_guard(io_mutex);

	if (output)
	{
		output->push_back(number);
		return;
	}

	std::cout.width(16);
	std::cout.fill('0');
	std::right(std::cout);
	std::cout << std::hex << number << std::endl;
}

size_t process::file_read(size_t file_offset, size_t bytes_count, size_t memory_address)
{
	std::lock_guard lock_guard(binary_file_mutex);

	if (bytes_count == 0 || !binary_file.is_open())
		return 0;

	binary_file.seekg(0, SEEK_END);
	const size_t file_size = binary_file.tellg();
	if (file_offset >= file_size)
		return 0;

	binary_file.seekg(file_offset, SEEK_SET);
	if (file_offset + bytes_count > file_size)
		bytes_count = file_size - file_offset - bytes_count; // fix bytes_count
	binary_file.read(reinterpret_cast<char*>(memory.data()) + memory_address, bytes_count);

	return bytes_count; // original or fixed value
}

void process::file_write(size_t file_offset, size_t bytes_to_write, size_t memoryAddress)
{
	std::lock_guard lock_guard(binary_file_mutex);

	if (!binary_file.is_open())
		return;

	// grow file
	binary_file.seekg(0, SEEK_END);
	const size_t file_size = binary_file.tellg();
	if (file_offset > file_size)
	{
		const size_t bytes_to_grow = file_offset - file_size;
		bytes_buffer buffer(bytes_to_grow);
		binary_file.write(reinterpret_cast<char*>(buffer.data()), bytes_to_grow);
	}

	// perform write
	binary_file.seekg(file_offset, SEEK_SET);
	binary_file.write(reinterpret_cast<char*>(memory.data()) + memoryAddress, bytes_to_write);
}

process::process(evm2_header& header, evm2_code& code, evm2_memory& data)
	:header(header), code(code), memory(data) {}

std::shared_ptr<process> process::factory::create(const std::string& file_name)
{
	// base file check
	if (!boost::filesystem::exists(file_name))
		throw image_exception(boost::format("File %1% does not exists") % file_name);

	const auto file_size = static_cast<unsigned int>(boost::filesystem::file_size(file_name));
	if (file_size < sizeof(evm2_header))
		throw image_exception(boost::format("Invalid image %1% format - file is too short") % file_name);

	// read file into a buffer
	std::ifstream file;
	file.open(file_name, std::ios::binary);
	if (!file.is_open())
		throw image_exception(boost::format("Image %1% open error") % file_name);

	file.unsetf(std::ios::skipws);
	bytes_buffer buffer(file_size);
	buffer.insert(buffer.begin(),
		std::istream_iterator<uint8_t>(file),
		std::istream_iterator<uint8_t>());
	file.close();

	// examine header
	evm2_header header = {};
	std::copy_n(buffer.begin(), sizeof header, reinterpret_cast<char*>(&header));

	const bool dataSizeIsValid = header.data_size >= header.initial_data_size;
	if (!dataSizeIsValid)
		throw image_exception(boost::format("Invalid image %1% data size is not valid") % file_name);

	if (std::strncmp(evm2_magic, header.magic, evm2_magic_size) != 0)
		throw image_exception(boost::format("Invalid image %1% format - bad magic string") % file_name);
		
	const uint32_t expected_file_size =
		header.code_size + header.initial_data_size + sizeof(header);
	
	if (file_size != expected_file_size)
		throw image_exception(boost::format(
			"Invalid image %1% - bad image size %2%, should be %3% ")
			% file_name % file_size % expected_file_size);

	// prepare evm code
#pragma warning( disable : 4244 ) 
	for (auto i = sizeof header; i < sizeof header + header.code_size; i++)
		buffer[i] = (buffer[i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;

	boost::dynamic_bitset<uint8_t> code(8 * header.code_size);
	from_block_range(buffer.begin() + sizeof header,
		buffer.begin() + sizeof header + header.code_size, code);

	// prepare evm data 
	evm2_memory data(header.data_size);
	std::copy_n(buffer.begin() + sizeof(header) + header.code_size,
		header.initial_data_size, data.begin());

	return std::make_shared<process>(header, code, data);
}
