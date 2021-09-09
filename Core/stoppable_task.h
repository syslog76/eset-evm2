#pragma once
#include <future>

class stoppable_task
{
    bool can_run_cache;
	std::promise<void> exit_signal;
    std::future<void> future_obj;
public:
    stoppable_task();
    void stop();
    bool can_run();
};
