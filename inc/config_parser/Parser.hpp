#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>

#include "Token.hpp"
#include "Config.hpp"

class Parser
{
	private:
		std::vector<Token> 	_tokens;	// tokens build by lexer
		size_t				_pos;		// current pos in tokens

		Token	current() const;
		Token	consume();
		Token	peek() const;
		Token	expectType(TokenType type, const std::string& expected);
		Token	expectTypeValue(TokenType type, const std::string& value);

		ServerConfig	parseServerBlock();
		LocationConfig	parseLocationBlock();
		
		// parse server settings
		void	parseServerSetting(ServerConfig& sc);
		void	pssListen(ServerConfig& sc);
		void	pssServername(ServerConfig& sc);
		void	pssClientMaxBodySize(ServerConfig& sc);
		void	pssErrorPages(ServerConfig& sc);
		void	pssIsDefaultServer(ServerConfig& sc);
		
		// prase location settings
		void	parseLocationSetting(LocationConfig& lc);

	public:
		// OCF
		Parser() = delete;
		Parser(const std::vector<Token>& tokens);
		Parser(const Parser& other) = delete;
		Parser& operator=(const Parser& other) = delete;
		~Parser() = default;

		// methods
		Config parseConfig();
};
