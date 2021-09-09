#pragma once
#include <cstdint>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

constexpr auto evm2_registers_count = 16;
constexpr auto evm_default_entry_point = 0;
constexpr auto evm2_magic = "ESET-VM2";
constexpr auto evm2_magic_size = 8;

typedef boost::dynamic_bitset<uint8_t> evm2_code;
typedef std::vector<int8_t> evm2_memory;
typedef std::vector<int64_t> evm2_registers;
typedef std::vector<uint32_t> evm2_stack;
typedef std::shared_ptr<std::vector<int64_t>> evm2_io_stream;

struct evm2_header
{
	char magic[evm2_magic_size];
	uint32_t code_size;
	uint32_t data_size;
	uint32_t initial_data_size;
};
