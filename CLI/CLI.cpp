#include <csignal>
#include "pch.h"

void setup();
void show_usage();
void sig_int_handler(int);

std::shared_ptr<process> process;

int main(const int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			show_usage();
			return 0;
		}
		setup();
		
		process = process::factory::create(argv[1]);
		
		if (argc > 2)
			process->binary_file_name = argv[2];

		process->start();
		
		process.reset();
		
		return 0;
	}
	catch (const exception& ex)
	{
		std::cout << ex.message << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch (...){}

	try
	{
		if (process)
			process.reset();
		std::cout << "Process has been stopped";					
	}
	catch (...){}
	
	return -1;
}

void show_usage()
{
	std::cout << "Usage: evm2.exe program.evm [file.bin]" << std::endl;
}

void setup()
{
	std::cout.setf(std::ios::hex, std::ios::basefield);
	signal(SIGINT, sig_int_handler);
}

void sig_int_handler(int)
{
	try
	{
		signal(SIGINT, sig_int_handler);
		if (process)
			process->stop();
	}
	catch (...)
	{
		std::cout << "sig_int_handler() error" << std::endl;
	}
}
