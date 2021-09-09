#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <concurrent_vector.h>
#include <fstream>
#include <thread>
#include <thread>
#include "thread.h"
#include "evm2_types.h"

struct thread_item
{
	std::shared_ptr<thread> evm2_thread;
	std::shared_ptr<std::thread> std_thread;
};

struct lock_item
{
	uint64_t index = 0;
	int64_t thread_ix = -1; // (-1) means free, un-locked mutex
	std::timed_mutex mutex;
};

class process : public stoppable_task
{
	std::mutex lock_table_mutex;
	Concurrency::concurrent_vector<std::shared_ptr<lock_item>> lock_table;
	
	Concurrency::concurrent_vector<std::shared_ptr<thread_item>> thread_table;

	std::mutex io_mutex;
	int64_t console_read();
	void console_write(uint64_t);
	
	std::fstream binary_file;
	std::mutex binary_file_mutex;
	size_t file_read(size_t, size_t, size_t);
	void file_write(size_t, size_t, size_t);

	int64_t create_thread(const std::shared_ptr<thread>&, uint32_t);	
	void join_thread(uint64_t);

	bool lock_exists(uint64_t ix);
	bool lock_create(uint64_t, uint64_t);
	void process_lock(uint64_t, uint64_t);	
	bool thread_holds_any_lock(uint64_t);
	void process_unlock(uint64_t, uint64_t);

	void run(uint64_t);

	void hlt(uint64_t);

	void terminate() noexcept;

public:
	evm2_header header;	
	evm2_code code;
	evm2_memory memory;

	std::string binary_file_name;
	
	evm2_io_stream input;
	evm2_io_stream output;
	
	void start();
	void stop();
	
	process(evm2_header&, evm2_code&, evm2_memory&);

	struct factory
	{
		static std::shared_ptr<process> create(const std::string&);
	};
};
