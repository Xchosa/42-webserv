#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>

#include "Token.hpp"
#include "Config.hpp"

class Parser
{
	private:
		std::vector<Token> 	_tokens;	// tokens build by lexer
		size_t				_pos;		// current pos in tokens

		Token	current() const;
		Token	consume();
		Token	peek() const;

	public:
		// OCF
		Parser() = delete;
		Parser(const std::vector<Token>& tokens);
		Parser(const Parser& other) = delete;
		Parser& operator=(const Parser& other) = delete;
		~Parser() = default;

		// methods
		Config parseConfig();
};
