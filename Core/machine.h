#pragma once
#include <memory>
#include <vector>
#include "misc.h"
#include "decoder.h"
#include "evm2_types.h"
#include "stoppable_task.h"

class machine : public stoppable_task
{
	evm2_code& code;
	evm2_memory& memory;	
	evm2_stack stack;
	uint32_t stack_position;
	evm2_registers registers;
	
	std::shared_ptr<decoder> decoder;

	int64_t read(instruction_argument&);
	void write(instruction_argument&, int64_t);
	
	friend class thread;
	friend class process;
public:

	machine(evm2_code&, evm2_memory&, uint32_t);
	
	evm2_op_code Run();

	struct factory
	{
		static std::shared_ptr<machine> create(evm2_code&, evm2_memory&);
		static std::shared_ptr<machine> duplicate(const std::shared_ptr<machine>&, uint32_t);
	};

	property<int64_t> arg1 {
		[this](int64_t value)
		{
			this->write(decoder->instruction.arguments[0], value);
		},
		[this]() -> int64_t
		{
			return this->read(decoder->instruction.arguments[0]);
		}
	};

	property<int64_t> arg2 {
	[this](int64_t value)
		{
			this->write(decoder->instruction.arguments[1], value);
		},
		[this]() -> int64_t
		{
			return this->read(decoder->instruction.arguments[1]);
		}
	};

	property<int64_t> arg3 {
	[this](int64_t value)
		{
			this->write(decoder->instruction.arguments[2], value);
		},
		[this]() -> int64_t
		{
			return this->read(decoder->instruction.arguments[2]);
		}
	};

	property<int64_t> Arg4 {
	[this](int64_t value)
		{
		this->write(decoder->instruction.arguments[3], value);
		},
	[this]() -> int64_t
		{
			return this->read(decoder->instruction.arguments[3]);
		}
	};
};
