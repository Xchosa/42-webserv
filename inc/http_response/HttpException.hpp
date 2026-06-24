#pragma once

#include <stdexcept>

class HttpException : public std::exception
{
	private:
		int	_code;
	
	public:
		HttpException() = delete;
		HttpException(const HttpException &other) = default;
		HttpException& operator=(const HttpException &other) = default;
		~HttpException() = default;

		HttpException(int code);
		int code() const;
};
