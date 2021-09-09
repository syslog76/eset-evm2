#pragma once
#include <iostream>
#include <functional>
#include <fstream>
#include <memory>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <utility>
#include <fstream>
#include <future>

#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>

#include <concurrent_vector.h>

#include "evm2_op_code.h"
#include "evm2_types.h"
#include "misc.h"
#include "stoppable_task.h"
#include "exception.h"
#include "decoder.h"
#include "machine.h"
#include "thread.h"
#include "process.h"

//#define _HAS_DEPRECATED_ALLOCATOR_MEMBERS 1