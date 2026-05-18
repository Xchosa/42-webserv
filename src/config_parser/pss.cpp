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
			throw std::runtime_error(getFileLine(t) + "Invalid ip '" + host_str + "'");
		sc._listen_host = host_str;
	}

	// validate port
	try
	{
		size_t idx;
		size_t port = std::stoull(port_str, &idx);
		if (idx != port_str.length() || port < 1 || port > 65535)
			throw std::runtime_error(getFileLine(t) + "Invalid port '" + port_str + "'");
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error(getFileLine(t) + "Invalid port '" + port_str + "'");
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
			throw std::runtime_error(getFileLine(t) + "Invalid server_name '" + t.value + "'! Invalid char: '" + invalid_char + "'");
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
		throw std::runtime_error(getFileLine(t) + "Invalid data size '" + t.value + "'");

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
		throw std::runtime_error(getFileLine(t) + "Overflow when converting data size '" + nbr_str + "'");
	}
	if (idx != nbr_str.length())
		throw std::runtime_error(getFileLine(t) + "Invalid data size '" + nbr_str + "'");

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
		throw std::runtime_error(getFileLine(t) + "Invalid data size suffix '" + std::string(1, suffix) + "'! Expected: 'k', 'm' or 'g'");

	sc._client_max_body_size = nbr * multiplier;
}

void Parser::pssErrorPage(ServerConfig& sc)
{
	// error if not enough params
	if (current().type != WORD || peek().type != WORD)
	{
		Token cur = current();
		throw std::runtime_error(getFileLine(cur) + "Expected 'error_page <code> </page>'");
	}

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
		throw std::runtime_error(getFileLine(t) + "Invalid error code '" + t.value + "'! Failed to convert value");
	}
	if (idx != t.value.length())
		throw std::runtime_error(getFileLine(t) + "Invalid error code '" + t.value + "'! Code not a number");
	if (errcode < 300 || errcode >= 600)
		throw std::runtime_error(getFileLine(t) + "Invalid error code '" + t.value + "'! Code out of range");

	// validate error file
	t = consume();
	if (t.value.length() < 3)
		throw std::runtime_error(getFileLine(t) + "Invalid error page '" + t.value + "'! Too short");
	if (t.value.at(0) != '/')
		throw std::runtime_error(getFileLine(t) + "Invalid error page '" + t.value + "'! Path has to start with '/'");

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
		throw std::runtime_error(getFileLine(t) + "Invalid is_default_server '" + t.value + "'! Expected: 'true' or 'false'");
	sc._is_default_server = flag;
}
