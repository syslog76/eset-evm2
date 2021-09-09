#include "pch.h"

evm2_op_code decoder::fetch()
{
	instruction = {};
	if (c >= padding_position && size_in_bits-c < 8)
		return padding;
	
	if (code[c++])
	{ // 1
		if (code[c++])
		{ // 11
			if (code[c++])
			{ // 111
				if (code[c++])
				{ // 1111
					fetch_arguments(1);
					return unlock;
				}
				// 1110
				fetch_arguments(1);
				return lock;
			}
			// 110
			if (code[c++])
				return ret; // 1101
			// 1100
			fetch_address();
			return call;
		}
		// 10
		if (code[c++])
		{ // 101
			if (code[c++])
			{ // 1011
				if (code[c++])
				{ // 10111
					fetch_arguments(1);
					return sleep;
				}
				// 10110
				return halt;
			}
			// 1010
			if (code[c++])
			{ // 10101
				fetch_arguments(1);
				return thread_join;
			}
			// 10100
			fetch_address();
			fetch_arguments(1);
			return thread_create;
		}
		// 100
		if (code[c++])
		{ // 1001
			if (code[c++])
			{ // 10011
				fetch_arguments(1);
				return con_write;
			}
			// 10010
			fetch_arguments(1);
			return con_read;
		}
		// 1000
		if (code[c++])
		{ // 10001
			fetch_arguments(3);
			return write;
		}
		// 10000
		fetch_arguments(4);
		return read;
	}
	// 0
	if (code[c++])
	{ // 01
		if (code[c++])
		{ // 011
			if (code[c++])
			{ // 0111
				if (code[c++])
					return ukn01111; // unknown instruction 01111
				// 01110
				fetch_address();
				fetch_arguments(2);
				return jump_equal;
			}
			// 0110
			if (code[c++])
			{ // 01101
				fetch_address();
				return jump_address;
			}
			// 01100
			fetch_arguments(3);
			return compare;
		}
		// 010
		if (code[c++])
		{ // 0101
			if (code[c++])
				return ukn01011; // unknown instruction 01011
			// 01010
			if (code[c++])
			{ // 010101
				fetch_arguments(3);
				return mul;
			}
			// 010100
			fetch_arguments(3);
			return mod;
		}
		// 0100
		if (code[c++])
		{ // 01001
			if (code[c++])
			{ // 010011
				fetch_arguments(3);
				return divide;
			}
			// 010010
			fetch_arguments(3);
			return sub;
		}
		// 01000
		if (code[c++])
		{ // 010001
			fetch_arguments(3);
			return add;
		}
		// 010000
		return ukn010000; // unknown instruction 010000
	}
	// 00
	if (code[c++])
	{ // 001
		instruction.constant = static_cast<int64_t>(fetch_bits(64));
		fetch_arguments(1);
		return load_const;
	}
	// 000
	fetch_arguments(2);
	return mov;
}

uint64_t decoder::fetch_bits(int bits_count)
{
	uint64_t result = 0;

	for (auto i = 0; i < bits_count; i++)
	{
		result = result >> 1;
		result += code[c + i] ? 0x8000000000000000 : 0;
	}
	c += bits_count;

	if (bits_count < 64)
		result = result >> (64 - bits_count);

	return result;
}

void decoder::fetch_arguments(uint64_t count)
{
	for (auto i = 0; i < count; i++)
	{
		const bool memory_access = code[c++];
		const uint8_t memory_access_size = memory_access ? static_cast<uint8_t>(fetch_bits(2)) : 0;
		const auto register_number = static_cast<uint8_t>(fetch_bits(4));
		
		instruction.arguments.push_back(
			instruction_argument
			{
				memory_access,
				memory_access_size,
				register_number
			});
	}
}

void decoder::fetch_address()
{
	instruction.address = static_cast<uint32_t>(fetch_bits(32));
}

void decoder::jump(uint32_t new_address)
{
	if (new_address >= code.size())
		throw out_of_range_exception("Instruction call/jump/jumpEqual out of range exception");

	c = new_address;
}

uint32_t decoder::get_address() const
{
	return c;
}

decoder::decoder(evm2_code& code, uint32_t entry_point) : c(entry_point), code(code)
{
	size_in_bits = static_cast<uint32_t>(code.size());
	padding_position = size_in_bits - 1;
	while (!code[padding_position] && padding_position > 0)
		padding_position--;
}

std::shared_ptr<decoder> decoder::factory::create(evm2_code& code, uint32_t entry_point = 0)
{
	return std::make_shared<decoder>(code, entry_point);
}
  