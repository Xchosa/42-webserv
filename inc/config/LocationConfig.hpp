#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

struct LocationConfig
{
	// std::string							_path;

	std::string 						_root;
	std::string							_index;
	std::vector<std::string> 			_methods; 
	bool								_autoindex		= false;

	std::optional<int>					_redirect_code;
	std::optional<std::string>			_redirect_url;
	
	std::optional<std::string>			_upload_store;
	// std::optional<size_t>				_max_body_size;		// nur auf server ebene
	std::map<std::string, std::string> 	_cgi_map;

	// helper methods
	// bool allows_method(const std::string& method) const;
};





// RETURN CODES
// 301 - Moved Permanently      (Browser cached den neuen Ort)
// 302 - Found / Temporary      (häufigster Redirect)
// 303 - See Other              (nach POST → GET umleiten)
// 307 - Temporary Redirect     (wie 302, aber Methode bleibt erhalten)
// 308 - Permanent Redirect     (wie 301, aber Methode bleibt erhalten)
