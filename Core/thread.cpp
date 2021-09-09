#include "pch.h"

evm2_op_code thread::run()
{
	try
	{
		while (can_run())
		{
			switch (const auto op_code = machine->Run()) {

				case sleep: 
					thread_sleep(machine->arg1);
					break;

				case ukn010000:
					throw not_implemented_exception("Unimplemented instruction 010000");
				
				case ukn01111:
					throw not_implemented_exception("Unimplemented instruction 01111");

				case ukn01011:
					throw not_implemented_exception("Unimplemented instruction 01011");
				
				default:
					return op_code;
			}
		}

		return stopped;
	}
	catch (exception& ex)
	{
		std::cout << ex.message << std::endl;
		std::cout << "Thread has been stopped" << std::endl;
		return stopped;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		std::cout << "Thread has been stopped" << std::endl;
		return stopped;
	}	
	catch (...)
	{
		std::cout << "Thread has been stopped";
		return stopped;
	}	
}

void thread::thread_sleep(int64_t milliseconds)
{
	auto sleep_units = milliseconds / 100;
	const auto rest_of_time = milliseconds % 100;
	const std::chrono::milliseconds a_rest_of_div(rest_of_time);
	constexpr std::chrono::milliseconds a100_ms(100);

	if (sleep_units)
		while (can_run() && sleep_units--)
			std::this_thread::sleep_for(a100_ms);
	if (rest_of_time)
		std::this_thread::sleep_for(a_rest_of_div);
}

thread::thread(evm2_code& code, evm2_memory& data)
{
	machine = machine::factory::create(code, data);
}

thread::thread(const std::shared_ptr<thread>& parent, uint32_t entry_point)
{
	machine = machine::factory::duplicate(parent->machine, entry_point);
}

std::shared_ptr<thread> thread::factory::create_main_thread(evm2_code& code, evm2_memory& data)
{
	return std::make_shared<thread>(code, data);
}

std::shared_ptr<thread> thread::factory::create_thread(const std::shared_ptr<thread>& parent, uint32_t entry_point)
{
	return std::make_shared<thread>(parent, entry_point);
}
