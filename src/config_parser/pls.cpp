#include "Parser.hpp"

void Parser::plsRoot(LocationConfig& lc)
{
	psRoot(current());
	lc._root = consume().value;
}

void Parser::plsIndex(LocationConfig& lc)
{
	psIndex(current());
	lc._index = consume().value;
}

void Parser::plsMethods(LocationConfig& lc)
{
	std::set<std::string> valid_methods = {"GET", "POST", "DELETE"};

	while (current().type != SEMICOLIN)
	{
		Token t = consume();
		if (!valid_methods.count(t.value))
			throw std::runtime_error("[Exception:plsMethods] Unexpected method '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: 'GET', 'POST' or 'DELETE'");
		lc._methods.push_back(t.value);
	}
}

void Parser::plsAutoindex(LocationConfig& lc)
{
	Token t = consume();
	bool flag;

	if (t.value == "on")
		flag = true;
	else if (t.value == "off")
		flag = false;
	else
		throw std::runtime_error("[Exception:plsAutoindex] Unexpected value for setting autoindex '" + t.value + "' in line " + std::to_string(t.line) + "! Expected: 'on' or 'off'");
	lc._autoindex = flag;
}

void Parser::plsReturn(LocationConfig& lc)
{
	// validate redirect code
	Token	t = consume();
	size_t	idx;
	int		code = std::stoi(t.value, &idx);
	if (idx != t.value.length())
		throw std::runtime_error("[Exception:plsReturn] Invalid redirect code '" + t.value + "' in line " + std::to_string(t.line) + "! Code not a number");
	if (code < 200 || code >= 600)
		throw std::runtime_error("[Exception:plsReturn] Invalid redirect code '" + t.value + "' in line " + std::to_string(t.line) + "! Code out of range");
	lc._redirect_code = code;

	// validate optional redirect url
	// 2xx, 4xx, 5xx -> body text, everyting allowed
	// 3xx -> validate url or path
	if (current().type == WORD)
	{
		t = consume();
		if (code >= 300 && code <= 399)
		{
			if (t.value[0] != '/' 
				&& t.value.substr(0,7) != "http://" 
				&& t.value.substr(0,8) != "https://")
				throw std::runtime_error("[Exception:plsReturn] Invalid redirect URL/path '" + t.value + "' in line " + std::to_string(t.line));
		}
		lc._redirect_url = t.value;
	}

	// if code 3xx, redirect url is neccesary
	if (code >= 300 && code <= 399 && !lc._redirect_url.has_value())
		throw std::runtime_error("[Exception:plsReturn] Redirect URL/path necceccary on code '" + t.value + "' in line " + std::to_string(t.line));
}

void Parser::plsUploadStore(LocationConfig& lc)
{
	Token t = consume();
	const std::string forbidden_chars = "*?[]{};\n#\"' \\";
	auto pos = t.value.find_first_of(forbidden_chars);
	if (pos != std::string::npos)
	{
		char invalid_char = t.value[pos];
		throw std::runtime_error("[Exception:plsUploadStore] Invalid upload_store path '" + t.value + "' in line " + std::to_string(t.line) + "! Invalid char: '" + invalid_char + "'");
	}

	lc._upload_store = t.value;
}

// TODO alles
void Parser::plsCgi(LocationConfig& lc)
{
	auto key = consume().value;
	auto val = consume().value;
	lc._cgi_map[key] = val;
}
