#include "utils.hpp"

std::string lowercase(std::string str)
{
	for (auto& c : str)
		c = std::tolower(static_cast<unsigned char>(c));
	return (str);
}
