#include "HttpException.hpp"

HttpException::HttpException(int code)
	: _code(code)
{}

int HttpException::code() const
{
	return (_code);
}
