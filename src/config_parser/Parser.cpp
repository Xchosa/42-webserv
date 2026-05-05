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

Token Parser::expectType(TokenType type, const std::string& expected)
{
	Token t = consume();
	if (t.type != type)
		throw std::runtime_error("[Exception:expectType] Unexpected value '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: " + expected);
	return (t);
}

Token Parser::expectTypeValue(TokenType type, const std::string& value)
{
	Token t = consume();
	if (t.type != type || t.value != value)
		throw std::runtime_error("[Exception:expectTypeValue] Unexpected value '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: '" + value + "'");
	return (t);
}

LocationConfig Parser::parseLocationBlock()
{
	LocationConfig lc;

	return (lc);
}

void Parser::parseServerSetting(ServerConfig& sc)
{
	(void)sc;

	Token setting_name = consume();

	if (setting_name.value == "listen")
	{
		Token token = consume();
		// alle pruefungen hier noch einbauen + unterscheidung ob nur port oder port + host angegeben
		
		sc._listen_port = std::stoull(token.value);
		
	}

	expectType(SEMICOLIN, ";");

}

ServerConfig Parser::parseServerBlock()
{
	ServerConfig sc;

	Token block_name = expectTypeValue(WORD, "server");
	expectType(LBRACE, "{");

	while (current().type != RBRACE)
	{
		if (current().value == "location" && peek().type == WORD)
		{
			// sc._locations.push_back(parseLocationBlock());
			consume();
		}
		else if (current().type == WORD && peek().type == WORD)
		{
			parseServerSetting(sc);
		}
		else
		{
			throw std::runtime_error("[Exception:parseServerBlock] Unexpected value '" + current().value + "' in line " + std::to_string(current().line));
		}





	}
	expectType(RBRACE, "}");

	return (sc);
}








Config Parser::parseConfig()
{
	Config config;

	while (_tokens.at(_pos).type != END_OF_FILE)
	{
		config._servers.push_back(parseServerBlock());
	}
	return (config);
}
