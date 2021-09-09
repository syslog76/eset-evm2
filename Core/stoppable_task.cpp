#include "pch.h"

stoppable_task::stoppable_task() : future_obj(exit_signal.get_future())
{
	can_run_cache = true;
}

bool stoppable_task::can_run()
{
	if (!can_run_cache)
		return false;
	can_run_cache = future_obj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
	return can_run_cache;
}

void stoppable_task::stop()
{
	if (can_run())
		exit_signal.set_value();
}
