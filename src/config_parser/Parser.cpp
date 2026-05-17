#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens)
	: _tokens(tokens), _pos(0)
{
	if (_tokens[0].type == END_OF_FILE)
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
		throw std::runtime_error("[Exception:parseServerSetting] Unknown setting key '" + setting_name.value + "' in line " + std::to_string(setting_name.line));

	expectType(SEMICOLIN, ";");
}

void Parser::parseLocationSetting(LocationConfig& lc)
{
	Token setting_name = consume();

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
		throw std::runtime_error("[Exception:parseLocationSetting] Unknown setting key '" + setting_name.value + "' in line " + std::to_string(setting_name.line));

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
			throw std::runtime_error("[Exception:parseLocationBlock] Unexpected value '" + current().value + "' in line " + std::to_string(current().line) + "! Missing parameter");
	}
	expectType(RBRACE, "}");
	return (lc);
}

void Parser::validateLocationPath(const Token& t)
{
	if (t.value[0] != '/')
		throw std::runtime_error("[Exception:validateLocationPath] Invalid location path '" + t.value + "' in line " + std::to_string(t.line) + "! Path has to start with '/'");

	const std::string forbidden_chars = "*?[]{}():;\n#\"' \\";
	auto pos = t.value.find_first_of(forbidden_chars);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:validateLocationPath] Invalid location path '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}
}

void Parser::setLocationDefaultSettings(ServerConfig& sc)
{
	bool 		has_default_root =	sc._tmp_root.has_value();
	bool 		has_default_index = sc._tmp_index.has_value();

	if (has_default_root == true || has_default_index == true)
	{
		// go through locations
		for (auto& [key, val] : sc._locations)
		{
			// set default root
			if (has_default_root == true)
			{
				if (val._root.length() == 0)
					val._root = sc._tmp_root.value();
			}
			// set default index
			if (has_default_index == true)
			{
				if (val._index.length() == 0)
					val._index = sc._tmp_index.value();
			}
		}
	}
}

ServerConfig Parser::parseServerBlock()
{
	ServerConfig sc;

	Token block_name = expectTypeValue(WORD, "server");
	expectType(LBRACE, "{");

	while (current().type != RBRACE)
	{
		if (current().value == "location")
		{
			expectTypeValue(WORD, "location");
			Token location_path = expectType(WORD, "<location_path>");
			validateLocationPath(location_path);
			
			// if location double -> error
			auto [it, inserted] = sc._locations.emplace(location_path.value, parseLocationBlock());
			if (!inserted)
				throw std::runtime_error("[Exception:parseServerBlock] Doubled location '" + location_path.value + "' in line " + std::to_string(location_path.line));
		}
		else if (current().type == WORD && peek().type == WORD)
			parseServerSetting(sc);
		else
			throw std::runtime_error("[Exception:parseServerBlock] Unexpected value '" + current().value + "' in line " + std::to_string(current().line) + "! Missing parameter");
	}
	expectType(RBRACE, "}");
	setLocationDefaultSettings(sc);
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
