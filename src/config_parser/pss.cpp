#include "Parser.hpp"

void Parser::pssRoot(ServerConfig& sc)
{
	psRoot(current());
	sc._tmp_root = consume().value;
}

void Parser::pssIndex(ServerConfig& sc)
{
	psIndex(current());
	sc._tmp_index = consume().value;
}

void Parser::pssListen(ServerConfig& sc)
{
	Token t = consume();
	
	// get values
	size_t dp_pos = t.value.find(":");
	std::string host_str;
	std::string port_str;
	if (dp_pos == std::string::npos)
		port_str = t.value;
	else
	{
		host_str = t.value.substr(0, dp_pos);
		port_str = t.value.substr(dp_pos + 1, t.value.length() - dp_pos);

		// validate ip
		struct in_addr addr;
		if (inet_pton(AF_INET, host_str.c_str(), &addr) != 1)
			throw std::runtime_error("[Exception:pssListen] Invalid ip '" + host_str + "' in line " + std::to_string(t.line));
		sc._listen_host = host_str;
	}

	// validate port
	try
	{
		size_t idx;
		size_t port = std::stoull(port_str, &idx);
		if (idx != port_str.length() || port < 1 || port > 65535)
			throw std::runtime_error("[Exception:pssListen] Invalid port '" + port_str + "' in line " + std::to_string(t.line));
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("[Exception:pssListen] Invalid port '" + port_str + "' in line " + std::to_string(t.line));
	}
	sc._listen_port = std::stoull(port_str);
}

void Parser::pssServername(ServerConfig& sc)
{
	while (current().type != SEMICOLIN)
	{
		Token t = consume();
		auto pos = t.value.find_first_of(FORBIDDEN_INDEX_CHARS);
		if (pos != std::string::npos)
		{
			char invalid_char = t.value[pos];
			throw std::runtime_error("[Exception:pssServername] Invalid server_name '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
		}
		sc._server_names.push_back(t.value);
	}
}

void Parser::pssClientMaxBodySize(ServerConfig& sc)
{
	Token t = consume();

	if (t.value == "0") // unlimited
	{
		sc._client_max_body_size = 0;
		return ;
	}
	if (t.value.length() <= 1)
		throw std::runtime_error("[Exception:pssClientMaxBodySize] Invalid data size '" + std::string(1, t.value.back()) + "' in line " + std::to_string(t.line));

	// get number
	std::string	nbr_str = t.value.substr(0, t.value.length() - 1);
	size_t		idx;
	size_t		nbr;
	try
	{
		nbr = std::stoull(nbr_str, &idx);
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("[Exception:pssClientMaxBodySize] Overflow when converting data size in bytes '" + nbr_str + "' in line " + std::to_string(t.line));
	}
	if (idx != nbr_str.length())
		throw std::runtime_error("[Exception:pssClientMaxBodySize] Invalid data size '" + nbr_str + "' in line " + std::to_string(t.line));

	// get suffix (kb, mb, gb, etc)
	char	suffix = t.value.back();
	suffix = std::tolower(suffix);
	size_t	multiplier;
	if (suffix == 'k')
		multiplier = 1024ULL;
	else if (suffix == 'm')
		multiplier = 1024ULL * 1024;
	else if (suffix == 'g')
		multiplier = 1024ULL * 1024 * 1024;
	else
		throw std::runtime_error("[Exception:pssClientMaxBodySize] Invalid data size '" + std::string(1, t.value.back()) + "' in line " + std::to_string(t.line) + "! Expected: 'k', 'm' or 'g'");

	sc._client_max_body_size = nbr * multiplier;
}

void Parser::pssErrorPage(ServerConfig& sc)
{
	// error if not enough params
	if (current().type != WORD || peek().type != WORD)
		throw std::runtime_error("[Exception:pssErrorPage] Excpected 'error_page <code> </page>' in line " + std::to_string(peek().line));

	// validate error code
	Token	t = consume();
	size_t	idx;
	int		errcode;
	try
	{
		errcode = std::stoi(t.value, &idx);
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("[Exception:pssErrorPage] Invalid error code '" + t.value + "' in line " + std::to_string(t.line) + "! Failed to convert value");
	}
	if (idx != t.value.length())
		throw std::runtime_error("[Exception:pssErrorPage] Invalid error code '" + t.value + "' in line " + std::to_string(t.line) + "! Code not a number");
	if (errcode < 300 || errcode >= 600)
		throw std::runtime_error("[Exception:pssErrorPage] Invalid error code '" + t.value + "' in line " + std::to_string(t.line) + "! Code out of range");

	// validate error file
	t = consume();
	if (t.value.length() < 3)
		throw std::runtime_error("[Exception:pssErrorPage] Invalid error page '" + t.value + "' in line " + std::to_string(t.line) + "! Too short to be the correct file");
	if (t.value.at(0) != '/')
		throw std::runtime_error("[Exception:pssErrorPage] Invalid error page '" + t.value + "' in line " + std::to_string(t.line) + "! Path has to start with a '/'");

	// write in map
	sc._error_pages[errcode] = t.value;
}

void Parser::pssIsDefaultServer(ServerConfig& sc)
{
	Token t = consume();
	bool flag;

	if (t.value == "true" || t.value == "1")
		flag = true;
	else if (t.value == "false" || t.value == "0")
		flag = false;
	else
		throw std::runtime_error("[Exception:pssIsDefaultServer] Unexpected value for setting is_default_server '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: 'true' or 'false'");
	sc._is_default_server = flag;
}
