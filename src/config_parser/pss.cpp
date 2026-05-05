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

// TODO alles
void Parser::pssServername(ServerConfig& sc)
{
	sc._server_names.push_back(consume().value);
}
