#pragma once
#include <memory>
#include "machine.h"
#include "evm2_types.h"
#include "stoppable_task.h"

class thread : public stoppable_task
{
	std::shared_ptr<machine> machine;
	void thread_sleep(int64_t);
	friend class process;

public:
	evm2_op_code run();
	thread(evm2_code&, evm2_memory&);
	thread(const std::shared_ptr<thread>&, uint32_t);

	struct factory
	{
		static std::shared_ptr<thread> create_main_thread(evm2_code&, evm2_memory&);
		static std::shared_ptr<thread> create_thread(const std::shared_ptr<thread>&, uint32_t);
	};
};
