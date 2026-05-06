#include "Parser.hpp"

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

// TODO alles
void Parser::pssServername(ServerConfig& sc)
{
	sc._server_names.push_back(consume().value);
}

// TODO alles
void Parser::pssRoot(ServerConfig& sc)
{
	sc._root = consume().value;
}

// TODO alles
void Parser::pssIndex(ServerConfig& sc)
{
	sc._index = consume().value;
}

// TODO alles
void Parser::pssClientMaxBodySize(ServerConfig& sc)
{
	sc._client_max_body_size = 1024 * 1024;
	consume();
}

// TODO alles
void Parser::pssErrorPage(ServerConfig& sc)
{
	int key = std::stoi(consume().value);
	std::string val = consume().value;

	sc._error_pages.emplace(key, val);
	// TODO klaeren: was soll bei doppelten error pages passieren? -> rueckgabewert emplace klaert auf
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
