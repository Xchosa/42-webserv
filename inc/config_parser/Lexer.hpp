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
		std::string _source;	// config as a string
		size_t		_pos;		// current pos in source
		size_t		_line;		// current line in config file

		char 		current() const;
		char 		consume();
		std::string consumeWord();

		bool isEof() const;
		bool isSpecialChar() const;

		void skipComments();
		void skipWhitespaces();

	public:
		// OCF
		Lexer() = delete;
		Lexer(std::string& conf_file_path);
		Lexer(const Lexer& other) = delete;
		Lexer& operator=(const Lexer& other) = delete;
		~Lexer() = default;

		// methods
		std::vector<Token> buildTokens();


		// for testing
		void print(const std::string& x) const;

};
