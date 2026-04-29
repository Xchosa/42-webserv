#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

struct LocationConfig
{
	std::string							_path;

	std::string 						_root;
	std::string							_index;
	std::vector<std::string> 			_methods; 
	bool								_autoindex		= false;

	std::optional<int>					_redirect_code;
	std::optional<std::string>			_redirect_url;
	
	std::optional<std::string>			_upload_store;
	std::optional<size_t>				_max_body_size;
	std::map<std::string, std::string> 	_cgi_map;

	// helper methods
	bool allows_method(const std::string& method) const;
};
// welche Pfad variablen sind absolut oder relative oder 
// / = absolut zum pfad der executable 
// alle anderen pfade sind alle relative zu dieser root
// ausser cgi die ruft exectuable cgi auf...

// html 
// root ./html/upload_files -> rel. pfad zur root 
// root ./ => relativ zur executable
// root /etc => absolut 
// index index.html		=> relativ zur config 
// index /index.html 	=>reliv 