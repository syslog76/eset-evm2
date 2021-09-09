#include "pch.h"
#include <boost/filesystem/file_status.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using bytes_buffer = std::vector<uint8_t>;

namespace tests
{
	TEST_CLASS(evm2_test)
	{
	public:

		std::string get_path(std::string file_name) const
		{
			auto dir = std::filesystem::current_path();
			auto parentDir = dir.parent_path();
			while (dir != parentDir)
			{
				if (is_directory(dir.append("evm")) && is_regular_file(dir.append(file_name)))
				{
					file_name = dir.string();
					break;
				}
				dir = parentDir;
				parentDir = dir.parent_path();
			}
			
			return file_name;									
		}

		bool compare_two_small_files_are_equal(std::string& file_name1, std::string& file_name2) const
		{
			try
			{
				if (file_name1 == file_name2)
					return true;

				if (!std::filesystem::exists(file_name1) || !std::filesystem::exists(file_name2))
					return false;

				std::ifstream file1;
				std::ifstream file2;

				file1.open(file_name1, std::ios::in | std::ios::binary);
				file2.open(file_name2, std::ios::in | std::ios::binary);

				const bool bothOpen = file1.is_open() && file2.is_open();

				if (!bothOpen)
				{
					if (file1.is_open())
						file1.close();
					if (file2.is_open())
						file2.close();
					return false;
				}

				file1.seekg(0, SEEK_END);
				const size_t file1Size = file1.tellg();
				file2.seekg(0, SEEK_END);
				const size_t file2Size = file1.tellg();
				if (file1Size != file2Size)
				{
					file1.close();
					file2.close();
					return false;
				}

				auto bufferSize = file1Size;

				bytes_buffer buffer1(bufferSize);
				bytes_buffer buffer2(bufferSize);

				file1.read(reinterpret_cast<char*>(buffer1.data()), bufferSize);
				file2.read(reinterpret_cast<char*>(buffer1.data()), bufferSize);

				return buffer1 == buffer2;

			}
			catch (...)
			{
				return false;
			}
		}
		
		TEST_METHOD(create_process_from_file)
		{
			
			auto process = process::factory::create(get_path("math.evm"));

			Assert::AreEqual(42l, static_cast<long>(process->header.code_size));

			process.reset();
			
			process = process::factory::create(get_path("Memory.evm"));

			Assert::AreEqual(64l, static_cast<long>(process->header.code_size));
			
			process.reset();
		}

		// Check if decoder can recognize any instruction
		TEST_METHOD(decoder_check)
		{
			auto process = process::factory::create(get_path("math.evm"));
			auto decoder = decoder::factory::create(process->code, 0);

			const evm2_op_code opcode = decoder->fetch();

			Assert::AreEqual(256l, static_cast<long>(decoder->instruction.constant));
			Assert::AreEqual(static_cast<long>(load_const), static_cast<long>(opcode));

			decoder.reset();
			process.reset();
		}

		// Test if running math.evm gives expected results
		TEST_METHOD(run_math) 
		{
		
			auto process = process::factory::create(get_path("math.evm"));
	
			process->output = std::make_shared<std::vector<int64_t>>();
			
			process->start();

			const std::vector<int64_t> validOutput = { 0x118, 0xe8, 0xa, 0x10, 0x1800, 0x1 };

			const auto outputIsValid = *process->output == validOutput;

			Assert::IsTrue(outputIsValid);
			
			process.reset();
		}

		// Test if running Memory.evm gives expected results
		TEST_METHOD(run_memory)
		{
			auto process = process::factory::create(get_path("Memory.evm"));

			process->output = std::make_unique<std::vector<int64_t>>();
			process->start();

			const std::vector<int64_t> validOutput = { 0x0123456789abcdef, 0x89abcdef, 0xcdef, 0xef, 0x1234567, 0x1234567, 0x4567, 0x67, 0, 0, 0, 0};

			const auto outputIsValid = *process->output == validOutput;

			Assert::IsTrue(outputIsValid);

			process.reset();
		}

		// Test if running fibonacci_loop.evm gives expected results
		TEST_METHOD(test_fibonacci)
		{
			auto process = process::factory::create(get_path("fibonacci_loop.evm"));

			process->input = std::make_unique<std::vector<int64_t>>();
			process->input->push_back(92);
			
			process->output = std::make_unique<std::vector<int64_t>>();
			process->start();

			const std::vector<int64_t> validOutput = {
				1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,10946,17711,28657,46368,
				75025,121393,196418,317811,514229,832040,1346269,2178309,3524578,5702887,9227465,14930352,
				24157817,39088169,63245986,102334155,165580141,267914296,433494437,701408733,1134903170,1836311903,
				2971215073,4807526976,7778742049,12586269025,20365011074,32951280099,53316291173,86267571272,
				139583862445,225851433717,365435296162,591286729879,956722026041,1548008755920,2504730781961,
				4052739537881,6557470319842,10610209857723,17167680177565,27777890035288,44945570212853,
				72723460248141,117669030460994,190392490709135,308061521170129,498454011879264,806515533049393,
				1304969544928657,2111485077978050,3416454622906707,5527939700884757,8944394323791464,
				14472334024676221,23416728348467685,37889062373143906,61305790721611591,99194853094755497,
				160500643816367088,259695496911122585,420196140727489673,679891637638612258,1100087778366101931,
				1779979416004714189,2880067194370816120,4660046610375530309,7540113804746346429
			};

			const auto outputIsValid = *process->output == validOutput;

			Assert::IsTrue(outputIsValid);

			process.reset();
		}

		static uint64_t rand64()
		{
			int64_t r = RAND_MAX;
			int64_t randMaxBits = 0;
			while (r > 0)
			{
				r = r >> 1;
				randMaxBits++;
			}
			uint64_t result = rand();
			for (int i = 0; i < 64 / randMaxBits; i++)
				result = (result << randMaxBits) | rand();
			return result;
		}
		
		// Test if running xor.evm gives expected results
		TEST_METHOD(run_100_xor)
		{
			srand(time(nullptr));
			for (auto i = 0; i < 100; i++)
			{
				auto process = process::factory::create(get_path("xor.evm"));

				const auto number1 = rand64();
				const auto number2 = rand64();
				const int64_t valid_xor = number1 ^ number2;

				process->input = std::make_unique<std::vector<int64_t>>();
				process->output = std::make_unique<std::vector<int64_t>>();
				process->input->push_back(number1);
				process->input->push_back(number2);
				process->start();

				const auto computedXor = (*process->output)[0];
				Assert::IsTrue(computedXor == valid_xor);

				process.reset();
			}
		}

		// Test if running xor-with-stack-frame.evm gives expected results
		TEST_METHOD(run_100_xor_with_stack_frame)
		{
			srand(time(nullptr));
			for (auto i = 0; i < 100; i++)
			{
				auto process = process::factory::create(get_path("xor-with-stack-frame.evm"));
				const auto number1 = rand64();
				const auto number2 = rand64();
				const auto valid_xor = number1 ^ number2;

				process->input = std::make_unique<std::vector<int64_t>>();
				process->output = std::make_unique<std::vector<int64_t>>();
				process->input->push_back(number1);
				process->input->push_back(number2);
				process->start();

				const auto computed_xor = (*process->output)[0];
				Assert::IsTrue(computed_xor == valid_xor);

				process.reset();
			}
		}

		// Test if running crc.evm gives expected results
		TEST_METHOD(test_crc)
		{
			auto process = process::factory::create(get_path("crc.evm"));
			
			process->input = std::make_unique<std::vector<int64_t>>();
			process->output = std::make_unique<std::vector<int64_t>>();
			process->binary_file_name = get_path("crc.bin");
			
			process->start();

			int64_t computed_crc = (*process->output)[0];
			Assert::IsTrue(computed_crc == 0x08407759b);

			process.reset();
		}

		// Test if running threadingBase.evm gives expected results
		TEST_METHOD(test_threading_base)
		{
			auto process = process::factory::create(get_path("threadingBase.evm"));

			process->output = std::make_unique<std::vector<int64_t>>();

			process->start();

			const int64_t result = (*process->output)[0];
			Assert::IsTrue(result == 0x0123456789abcdef);

			process.reset();
		}
		
		// Test if running threadingBase.evm gives expected results
		TEST_METHOD(stop_1000_threads)
		{
			auto process = process::factory::create(get_path("stop1000threads.evm"));
			process->output = std::make_unique<std::vector<int64_t>>();
			process->start();
			process.reset();
		}

		// Test if running lock.evm gives expected results
		TEST_METHOD(test_lock)
		{
			auto process = process::factory::create(get_path("lock.evm"));
			process->output = std::make_unique<std::vector<int64_t>>();
			process->start();

			const int64_t result = (*process->output)[0];
			Assert::IsTrue(result == 0x300);
			
			process.reset();
		}

		// Test if running multithreaded_file_write.evm gives expected results
		TEST_METHOD(test_multithreaded_file_write)
		{
			auto process = process::factory::create(get_path("multithreaded_file_write.evm"));
			process->output = std::make_unique<std::vector<int64_t>>();

			std::string file_name = "file.bin";
			process->binary_file_name = file_name;
			if (std::filesystem::exists(file_name))
				remove(file_name.c_str());

			std::string correctFileName = get_path("multithreaded_file_write.bin");

			process->start();

			Assert::IsTrue(compare_two_small_files_are_equal(file_name, correctFileName));

			process.reset();
		}
	};
}
