#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <iostream>

#include "Token.hpp"

class Lexer
{
	private:
		// ...

	public:
		// OCF
		Lexer() = delete;
		Lexer(std::string& conf_file_path);
		Lexer(const Lexer& other) = delete;
		Lexer& operator=(const Lexer& other) = delete;
		~Lexer() = default;

};
