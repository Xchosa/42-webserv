#include "webserv.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

#define MAX_EVENTS 10 // @PAUL; was ist das hier?

Config startParseConfig(const std::string& conf_file_path)
{
	Config config;

	Lexer l(conf_file_path);
	std::vector<Token> tokens = l.buildTokens();
	// l.printTokens(tokens);

	Parser p(tokens);
	config = p.parseConfig();

	return (config);
}

int main(int argc, char **argv)
{
	Config config;
	if (argc != 2)
	{
		std::cerr << "Error: Usage: ./webserv <config_file>\n";
		return (1);
	}
	try
	{
		std::string config_file = argv[1];
		config = startParseConfig(config_file);

		Server server(config);
		server.run();
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error: " << e.what() <<std::endl;
		return 1;
	}
	return 0;	
}
