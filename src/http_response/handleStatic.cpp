#include "Dispatcher.hpp"

HttpResponse Dispatcher::handleStatic(const HttpRequest &request, LocationConfig* lc)
{
	HttpResponse r;

	if (lc == nullptr)
		throw HttpException(500);

	std::string full_path = getFullRootPath(lc) + request._path;

	// pfad vorhanden?
	struct stat statbuf;
	if (stat(full_path.c_str(), &statbuf) == -1)
		throw HttpException(404);

	std::string body;
	if (S_ISREG(statbuf.st_mode)) // path is a file
	{
		try
		{
			body = readFile(full_path);
			r._headers["Content-Type"] = getMimeTypeFromFile(full_path);
		}
		catch(const std::exception& e)
		{
			throw HttpException(404);
		}
	}
	else if (S_ISDIR(statbuf.st_mode)) // path is a directory
	{
		if (lc->_index.length() > 0) // index file exist
		{
			std::string index_path = full_path + "/" + lc->_index;

			try
			{
				body = readFile(index_path);
				r._headers["Content-Type"] = getMimeTypeFromFile(index_path);
			}
			catch(const std::exception& e)
			{
				if (lc->_autoindex == true)
				{
					body = "TODO autoindex\n";
					r._headers["Content-Type"] = "text/html";
				}
				else
					throw HttpException(404);
			}
		}
		else // no index file
		{
			if (lc->_autoindex == true)
			{
				body = "TODO autoindex\n";
				r._headers["Content-Type"] = "text/html";
			}
			else
				throw HttpException(403);
		}
	}
	else
	{
		throw HttpException(502);
	}

	r._version = "HTTP/1.1";
	r._status_code = 200;
	r._status_text = getStatusText(200);
	r._body = body;
	r._headers["Connection"] = getConnectionMode(request._headers);
	r._headers["Content-Length"] = std::to_string(r._body.length());

	return (r);
}
