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

	// listen
	// HOST:PORT (host und : sind optional)
	if (setting_name.value == "listen")
	{
		Token t = consume();
		
		// get values
		size_t dp_pos = t.value.find(":");
		std::string host_str;
		std::string port_str;
		if (dp_pos == std::string::npos)
		{
			port_str = t.value;
		}
		else
		{
			host_str = t.value.substr(0, dp_pos);
			port_str = t.value.substr(dp_pos + 1, t.value.length() - dp_pos);

			// validate ip
			struct in_addr addr;
			if (inet_pton(AF_INET, host_str.c_str(), &addr) != 1)
				throw std::runtime_error("[Exception:parseServerSetting] Invalid ip '" + host_str + "' in line " + std::to_string(t.line));
			sc._listen_host = host_str;
		}

		// validate port
		try
		{
			size_t idx;
			size_t port = std::stoull(port_str, &idx);
			if (idx != port_str.length() || port < 1 || port > 65535)
				throw std::runtime_error("[Exception:parseServerSetting] Invalid port '" + port_str + "' in line " + std::to_string(t.line));
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error("[Exception:parseServerSetting] Invalid port '" + port_str + "' in line " + std::to_string(t.line));
		}
		sc._listen_port = std::stoull(port_str);
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
