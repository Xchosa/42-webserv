#include "Lexer.hpp"

Lexer::Lexer(std::string& conf_file_path)
{
	std::ifstream file(conf_file_path);
	if (!file.is_open())
		throw std::runtime_error("Unable to open config file: " + conf_file_path);

	std::ostringstream ss;
	ss << file.rdbuf();
	std::cout << ss.str();
}
