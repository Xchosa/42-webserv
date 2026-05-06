#include "Parser.hpp"

// TODO alles
void Parser::plsRoot(LocationConfig& lc)
{
	psRoot(current());
	lc._root = consume().value;
}

// TODO alles
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

// TODO alles
void Parser::plsReturn(LocationConfig& lc)
{
	Token code = consume();
	Token to = consume();

	if (code.type != WORD || to.type != WORD) // hier noch pruefung ob url/pfad stimmt
		throw std::runtime_error("[Exception:plsReturn] Unexpected value for redirect url '" + to.value + "' in line " + std::to_string(to.line) + "! Expected: a path or url");

	lc._redirect_code = std::stoi(code.value);
	lc._redirect_url = to.value;
}

// TODO alles
void Parser::plsUploadStore(LocationConfig& lc)
{
	lc._upload_store = consume().value;
}

// TODO alles
void Parser::plsCgi(LocationConfig& lc)
{
	auto key = consume().value;
	auto val = consume().value;
	lc._cgi_map[key] = val;
}
