#include "Lexer.hpp"

Lexer::Lexer(std::string& conf_file_path)
	: _pos(0), _line(1)
{
	std::ifstream file(conf_file_path);
	if (!file.is_open())
		throw std::runtime_error("Unable to open config file: " + conf_file_path);

	std::ostringstream ss;
	ss << file.rdbuf();
	_source = ss.str();
}

bool Lexer::isEof() const
{
	if (_pos >= _source.size())
		return (true);
	return (false);
}

char Lexer::current() const
{
	return (_source.at(_pos));
}

char Lexer::consume()
{
	char c = current();
	if (c == '\n')
		_line++;
	_pos++;
	return (c);
}

char Lexer::peek() const
{
	return (_source.at(_pos + 1));
}

void Lexer::print(const std::string& x) const
{
	if (x == "POS")
		std::cout << _source.substr(_pos);
	else
	std::cout << _source.substr(_pos);
	(void)_pos;
	(void)_line;
}

void Lexer::skipComments()
{
	if (current() == '#')
	{
		while (!isEof() && current() != '\n')
			consume();
	}
}

void Lexer::skipWhitespaces()
{
	while (!isEof() && std::isspace(current()))
		consume();
}

std::vector<Token> Lexer::buildTokens()
{
	std::vector<Token> tokens;

	while (!isEof())
	{
		skipComments();
		skipWhitespaces();

		if (current() == ';')
		{
			tokens.push_back({SEMICOLIN, ";", _line});
			consume();
		}
		else if (current() == '{')
		{
			tokens.push_back({LBRACE, "{", _line});
			consume();
		}
		else if (current() == '}')
		{
			tokens.push_back({RBRACE, "}", _line});
			consume();
		}
		// else
			// consumeWord();

		consume();
	}


	return (tokens);
}