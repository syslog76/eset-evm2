#include "pch.h"

exception::exception(const std::string text)
{
	message = text.c_str();
}

exception::exception(boost::basic_format<char> text)
{
	message = text.str();
}

image_exception::image_exception(boost::basic_format<char>& text)
	: exception(text) {}

not_implemented_exception::not_implemented_exception(const std::string text)
	: exception(text) {}

out_of_range_exception::out_of_range_exception(const std::string text)
	: exception(text) {}
