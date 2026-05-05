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

bool Lexer::isSpecialChar() const
{
	char c = current();

	if (c == ';' || c == '{' || c == '}' || c == '\n')
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

void Lexer::skipCommentsAndWhitespaces()
{
	while (!isEof() && (current() == '#' || std::isspace(current())))
	{
		skipComments();
		skipWhitespaces();
	}
}

std::string Lexer::consumeWord()
{
	std::string word;

	while (!isEof() && !isSpecialChar() && !std::isspace(current()))
	{
		word += consume();
	}
	return (word);
}

std::vector<Token> Lexer::buildTokens()
{
	std::vector<Token> tokens;

	while (!isEof())
	{
		skipCommentsAndWhitespaces();

		if (isEof())
			break ;

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
		else
			tokens.push_back({WORD, consumeWord(), _line});
	}
	tokens.push_back({END_OF_FILE, "", _line});
	return (tokens);
}
