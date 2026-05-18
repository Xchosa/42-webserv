#include "Parser.hpp"

void Parser::psRoot(const Token& t)
{
	auto pos = t.value.find_first_of(FORBIDDEN_PATH_CHARS);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:psRoot] Invalid root path '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}
}

void Parser::psIndex(const Token& t)
{
	auto pos = t.value.find_first_of(FORBIDDEN_INDEX_CHARS);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:psIndex] Invalid index '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}
}
