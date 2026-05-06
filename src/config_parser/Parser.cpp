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
		throw std::runtime_error("[Exception:expectType] Unexpected value '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: '" + expected + "'");
	return (t);
}

Token Parser::expectTypeValue(TokenType type, const std::string& value)
{
	Token t = consume();
	if (t.type != type || t.value != value)
		throw std::runtime_error("[Exception:expectTypeValue] Unexpected value '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: '" + value + "'");
	return (t);
}

void Parser::parseServerSetting(ServerConfig& sc)
{
	Token setting_name = consume();

	if (setting_name.value == "listen")
		pssListen(sc);
	else if (setting_name.value == "server_name")
		pssServername(sc);
	else if (setting_name.value == "root")
		pssRoot(sc);
	else if (setting_name.value == "index")
		pssIndex(sc);
	else if (setting_name.value == "client_max_body_size")
		pssClientMaxBodySize(sc);
	else if (setting_name.value == "error_page")
		pssErrorPage(sc);
	else if (setting_name.value == "is_default_server")
		pssIsDefaultServer(sc);
	else
		throw std::runtime_error("[Exception:parseServerSetting] Unexpected setting key '" + setting_name.value + "' in line " + std::to_string(setting_name.line));

	expectType(SEMICOLIN, ";");
}

void Parser::parseLocationSetting(LocationConfig& lc)
{
	Token setting_name = consume();
	(void)lc;
	if (setting_name.value == "root")
		plsRoot(lc);
	else if (setting_name.value == "index")
		plsIndex(lc);
	else if (setting_name.value == "methods")
		plsMethods(lc);
	else if (setting_name.value == "autoindex")
		plsAutoindex(lc);
	else if (setting_name.value == "return")
		plsReturn(lc);
	else if (setting_name.value == "upload_store")
		plsUploadStore(lc);
	else if (setting_name.value == "cgi_ext")
		plsCgi(lc);
	else
		throw std::runtime_error("[Exception:parseLocationSetting] Unexpected setting key '" + current().value + "' in line " + std::to_string(current().line));

	expectType(SEMICOLIN, ";");
}

LocationConfig Parser::parseLocationBlock()
{
	LocationConfig lc;

	expectType(LBRACE, "{");
	while (current().type != RBRACE)
	{
		if (current().type == WORD && peek().type == WORD)
			parseLocationSetting(lc);
		else
			throw std::runtime_error("[Exception:parseLocationBlock] Unexpected value '" + current().value + "' in line " + std::to_string(current().line));
	}
	expectType(RBRACE, "}");
	return (lc);
}

// TODO all
void Parser::validateLocationPath(const Token& t)
{
	(void)t;
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
			expectTypeValue(WORD, "location");
			Token location_path = expectType(WORD, "<location_path");
			validateLocationPath(location_path);
			
			// wenn location doppelt -> fehler
			auto [it, inserted] = sc._locations.emplace(location_path.value, parseLocationBlock());
			if (!inserted)
				throw std::runtime_error("[Exception:parseServerBlock] Doubled location '" + current().value + "' in line " + std::to_string(current().line));
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
	// TODO hier noch default values von ServerConfig in die LocationConfig schreiben (falls settings nach locations stehen)
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
