#include "pch.h"

evm2_op_code machine::Run()
{
	while(can_run())
	{
		switch (const auto op_code = decoder->fetch()) {

			case load_const: 
				arg1 = decoder->instruction.constant;
				break;
			
			case mov: 
				arg2 = static_cast<int64_t>(arg1);
				break;

			case add: 
				arg3 = arg1 + arg2;
				break;
			
			case sub:
				arg3 = arg1 - arg2;
				break;
			
			case divide:
				arg3 = arg1 / arg2;
				break;

			case mod:
				arg3 = arg1 % arg2;
				break;

			case mul:
				arg3 = arg1 * arg2;
				break;
			
			case compare:
				arg3 = arg1 == arg2 ? 0 : arg1 > arg2 ? 1 : -1;
				break;

			case jump_address: 
				decoder->jump(decoder->instruction.address);
				break;
			
			case jump_equal:
				if (arg1 == arg2)
					decoder->jump(decoder->instruction.address);
				break;

			case call:
				stack[--stack_position] = decoder->get_address();
				if (stack_position == 0)
					throw out_of_range_exception("Stack overflow");
				decoder->jump(decoder->instruction.address);
				break;

			case ret:
				decoder->jump(stack[stack_position++]);
				break;
			
			default:
				return op_code;
		}
	}
	
	return stopped;
}

machine::machine(evm2_code& code, evm2_memory& memory, uint32_t entry_point)
	:code(code), memory(memory), stack(0x1000, 0), stack_position(0x0fff), registers(evm2_registers_count, 0)
{
	decoder = decoder::factory::create(code, entry_point);
}

std::shared_ptr<machine> machine::factory::create(evm2_code& code, evm2_memory& memory)
{
	return std::make_shared<machine>(code, memory, evm_default_entry_point);
}

std::shared_ptr<machine> machine::factory::duplicate(const std::shared_ptr<machine>& source, uint32_t entryPoint)
{
	auto result = std::make_shared<machine>(source->code, source->memory, entryPoint);
	result->registers = source->registers;
	return result;
}

int64_t machine::read(instruction_argument& argument)
{
	if (argument.is_memory_access)
	{
		uint64_t result = 0;
		for (int64_t i = (1l << argument.memory_access_size) - 1l; i >= 0; i--)
		{
			result = result << 8;

			if (argument.register_number >= evm2_registers_count)
				throw out_of_range_exception("Access register out of range");

			if (registers[argument.register_number] + i >= static_cast<int64_t>(memory.size()))
				throw out_of_range_exception("Write memory out of range");

			result += static_cast<uint8_t>(memory[registers[argument.register_number] + i]);
		}
		return result;
	}

	if (argument.register_number >= evm2_registers_count)
		throw out_of_range_exception("Access register out of range");

	return registers[argument.register_number];
}

void machine::write(instruction_argument& argument, int64_t value)
{
	if (argument.register_number >= evm2_registers_count)
		throw out_of_range_exception("Access register out of range");

	if (argument.is_memory_access)
	{
		for (auto i = 0; i < 1 << argument.memory_access_size; i++)
		{
			const size_t address = registers[argument.register_number] + i;

			if (address >= memory.size())
				throw out_of_range_exception("Read memory out of range");

			memory[address] = static_cast<int8_t>(value & 0xff);
			value = value >> 8;
		}
		return;
	}

	registers[argument.register_number] = value;
}
