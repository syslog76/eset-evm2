#pragma once
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include "evm2_types.h"
#include "evm2_op_code.h"

struct instruction_argument
{
	bool is_memory_access;      // if (true) memory will be accessed, otherwise register
	uint8_t memory_access_size; // 1,2,4 or 8 bytes
	uint8_t register_number;	
};

struct evm2_instruction
{
	uint32_t address = 0; // if decoded: jmp/je address
	int64_t constant = 0; // if decoded: 64-bit const value
	std::vector<instruction_argument> arguments; // instruction arguments
};

class decoder
{
	uint32_t c;            // internal decoder position (bit in instruction stream)
	uint32_t size_in_bits; // image size (in bits)
	evm2_code& code;       // program code
	uint32_t padding_position; // guessed position of program padding

	void fetch_address();
	uint64_t fetch_bits(int);
	void fetch_arguments(uint64_t);
		
public:
	decoder(evm2_code&, uint32_t);

	evm2_op_code fetch();
	evm2_instruction instruction;

	uint32_t get_address() const;
	void jump(uint32_t);
		
	struct factory
	{
		static std::shared_ptr<decoder> create(evm2_code&, uint32_t);
	};
};
