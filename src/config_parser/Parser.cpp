#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens, const std::string& conf_file_path)
	: _tokens(tokens), _pos(0), _conf_file_path(conf_file_path)
{
	if (_tokens[0].type == END_OF_FILE)
		throw std::runtime_error(getFileLine(_tokens[0]) + "Empty config file!");
}

std::string Parser::getFileLine(const Token& t) const
{
	return ("[" + _conf_file_path + ":" + std::to_string(t.line) + "] ");
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
		throw std::runtime_error(getFileLine(t) + "Unexpected value '" + t.value + "'! Expected: '" + expected + "'");
	return (t);
}

Token Parser::expectTypeValue(TokenType type, const std::string& value)
{
	Token t = consume();
	if (t.type != type || t.value != value)
		throw std::runtime_error(getFileLine(t) + "Unexpected value '" + t.value + "'! Expected: '" + value + "'");
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
		throw std::runtime_error(getFileLine(setting_name) + "Unknown setting key '" + setting_name.value + "'");

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
	else if (setting_name.value == "upload")
		plsUpload(lc);
	else if (setting_name.value == "cgi_ext")
		plsCgi(lc);
	else
		throw std::runtime_error(getFileLine(setting_name) + "Unknown setting key '" + setting_name.value + "'");

	expectType(SEMICOLIN, ";");
}

LocationConfig Parser::parseLocationBlock(std::string& location_name)
{
	LocationConfig lc;

	lc._name = location_name;
	expectType(LBRACE, "{");
	while (current().type != RBRACE)
	{
		if (current().type == WORD && peek().type == WORD)
			parseLocationSetting(lc);
		else
		{
			Token t = current();
			throw std::runtime_error(getFileLine(t) + "Unexpected value '" + t.value + "'! Missing parameter");
		}
	}
	expectType(RBRACE, "}");
	return (lc);
}

void Parser::validateLocationPath(const Token& t)
{
	if (t.value[0] != '/')
		throw std::runtime_error(getFileLine(t) + "Invalid location path '" + t.value + "'! Path has to start with '/'");

	auto pos = t.value.find_first_of(FORBIDDEN_PATH_CHARS);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error(getFileLine(t) + "Invalid location path '" + t.value + "'! Invalid char: '" + invalid_char + "'");
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
			auto [it, inserted] = sc._locations.emplace(location_path.value, parseLocationBlock(location_path.value));
			if (!inserted)
				throw std::runtime_error(getFileLine(location_path) + "Doubled location '" + location_path.value + "'");
		}
		else if (current().type == WORD && peek().type == WORD)
			parseServerSetting(sc);
		else
		{
			Token t = current();
			throw std::runtime_error(getFileLine(t) + "Unexpected value '" + t.value + "'! Missing parameter");
		}
	}
	expectType(RBRACE, "}");
	setLocationDefaultSettings(sc);
	return (sc);
}

void Parser::validateConfig(Config& c)
{
	// pro server block
	for (ServerConfig& sc : c._servers)
	{
		size_t count_default_server = 0;

		if (sc._listen_port == 0)
			throw std::runtime_error("ServerConfig: Detected Server without listen port!");
		
		if (sc._is_default_server == true)
			count_default_server++;
		if (count_default_server > 1)
			throw std::runtime_error("ServerConfig: Detected more than one default server!");

		if (sc._locations.size() == 0)
			throw std::runtime_error("ServerConfig: No location found");
		
		// pro location block
		for (auto& it : sc._locations)
		{
			auto& lc = it.second;

			if (lc._redirect_code.has_value()) // redirect locations -> no need for methods
				continue;

			if (lc._root.length() <= 0)
				throw std::runtime_error("LocationConfig: Detected Location without a root directory!");

			if (lc._methods.size() == 0)
				throw std::runtime_error("LocationConfig: Detected a non return location with no method allowed!");

			// post only allowed when cgi or upload exist
			if (std::find(lc._methods.begin(), lc._methods.end(), "POST") != lc._methods.end())
			{
				if (lc._upload == false && lc._cgi_map.size() == 0)
					throw std::runtime_error("LocationConfig: Detected allowed 'POST' method without active upload or cgi settings!");
			}
		}
	}
}

Config Parser::parseConfig()
{
	Config config;

	while (_tokens.at(_pos).type != END_OF_FILE)
	{
		config._servers.push_back(parseServerBlock());
	}
	validateConfig(config);
	return (config);
}
