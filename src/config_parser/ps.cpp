#include "Parser.hpp"

void Parser::psRoot(const Token& t)
{
	const std::string forbidden_chars = "*?[]{};\n#\"' \\";
	auto pos = t.value.find_first_of(forbidden_chars);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:psRoot] Invalid root path '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}
}

void Parser::psIndex(const Token& t)
{
	const std::string forbidden_chars = "*?[]{};\n#\"' \\/";
	auto pos = t.value.find_first_of(forbidden_chars);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:psIndex] Invalid index '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}
}
