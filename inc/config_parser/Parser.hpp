#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>
#include <set>
#include <string>

#include "Token.hpp"
#include "Config.hpp"

inline constexpr std::string_view FORBIDDEN_PATH_CHARS = "*?[]{}():;\n#\"' \\";
inline constexpr std::string_view FORBIDDEN_INDEX_CHARS = "*?[]{}():;\n#\"' \\/";

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

		// parse tokens
		ServerConfig	parseServerBlock();
		LocationConfig	parseLocationBlock();
		void			validateLocationPath(const Token& t);
		void			setLocationDefaultSettings(ServerConfig& sc);
		
		// parse doubled settings (server and location)
		void	psRoot(const Token &t);
		void	psIndex(const Token &t);

		// parse server settings
		void	parseServerSetting(ServerConfig& sc);
		void	pssRoot(ServerConfig& sc);
		void	pssIndex(ServerConfig& sc);
		void	pssListen(ServerConfig& sc);
		void	pssServername(ServerConfig& sc);
		void	pssClientMaxBodySize(ServerConfig& sc);
		void	pssErrorPage(ServerConfig& sc);
		void	pssIsDefaultServer(ServerConfig& sc);
		
		// prase location settings
		void	parseLocationSetting(LocationConfig& lc);
		void	plsRoot(LocationConfig& lc);
		void	plsIndex(LocationConfig& lc);
		void	plsMethods(LocationConfig& lc);
		void	plsAutoindex(LocationConfig& lc);
		void	plsReturn(LocationConfig& lc);
		void	plsUploadStore(LocationConfig& lc);
		void	plsCgi(LocationConfig& lc);

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
