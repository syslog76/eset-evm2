#pragma once
#include <boost/format/format_class.hpp>

class exception: public std::exception
{
public:
	std::string message;
	explicit exception(boost::basic_format<char> text);
	explicit exception(std::string text);	
};

class image_exception : public exception
{
public:
	image_exception(boost::basic_format<char>& text);
};

class out_of_range_exception : public exception
{
public:
	out_of_range_exception(std::string text);
};

class not_implemented_exception : public exception
{
public:
	not_implemented_exception(std::string text);
};
