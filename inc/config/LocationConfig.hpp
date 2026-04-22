#include <string>
#include <vector>
#include <optional>
#include <map>

struct LocationConfig
{
	std::string							path;
	std::string 						root;
	std::string							index;
	std::vector<std::string> 			methods;
	bool								autoindex; // standard auf false
	std::optional<int>					redirect_code;
	std::optional<std::string>			redirect_url;
	std::optional<std::string>			upload_store;
	std::optional<size_t>				max_body_size;
	std::map<std::string, std::string> 	cgi_map;

	// helper methods
	bool allows_method(const std::string& method) const;
};
