#pragma once

#include <string>

enum TokenType
{
	WORD,
	SEMICOLIN,
	LBRACE,
	RBRACE,
	END_OF_FILE,
};

struct Token
{
	TokenType	type;
	std::string	value;
	size_t		line;
};
