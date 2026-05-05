#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens)
	: _tokens(tokens), _pos(0)
{
	if (_tokens.size() == 0)
		throw std::runtime_error("Empty config file!");
}

Token Parser::current() const
{
	if (_pos >= _tokens.size())
		return (_tokens.back());
	return (_tokens[_pos]);
}

Token Parser::consume()
{
	Token t = current();
	_pos++;
	return (t);
}

Token Parser::peek() const
{
	if (_pos >= _tokens.size() - 1)
		return (_tokens.back());
	return (_tokens[_pos + 1]);
}










Config Parser::parseConfig()
{
	Config config;

	while (_tokens.at(_pos).type != END_OF_FILE)
	{
		Token current = this->current();
		Token next = peek();

		if (current.type == WORD && next.type == LBRACE)
		{
			// parseBlock();
		}
		else if (current.type == WORD && next.type == SEMICOLIN)
		{
			// parseSetting();
		}

		_pos++;
		std::cout << current.value << std::endl;

	}


	return (config);
}