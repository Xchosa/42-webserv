#include "Dispatcher.hpp"

HttpResponse Dispatcher::handleDelete(const HttpRequest& request, LocationConfig* lc)
{
	std::cout << "[INFO]  entered delete handler" << std::endl;

	std::string user_path = getFullRootPath(lc)+ request._path;
	isWithin(getFullRootPath(lc)+ lc->_name, user_path);
	
	std::cout << "[INFO]  Traget for deleting file; "<< user_path << std::endl;

	struct stat statbuf;

	if(lstat(user_path.c_str(), &statbuf) == -1)
	{
		if (errno == ENOENT)
  			throw HttpException (404);
		throw HttpException(500);
	}

	if (S_ISLNK(statbuf.st_mode)) // check for symlink 
		throw  HttpException(403);
	if (S_ISDIR(statbuf.st_mode))
		throw HttpException(403); // target directory
	if(!S_ISREG(statbuf.st_mode)) // check regularfile / got permissions / overwrite
		throw HttpException(403);

	if(unlink(user_path.c_str()) == -1)
	{
		if(errno == ENOENT)
			throw HttpException(404); // path des not exist
		if(errno == EACCES || errno == EPERM)
			throw HttpException(403); // permission
		if(errno == EBUSY)
			throw HttpException(409); // used by another process
		throw HttpException(500);
	}

	HttpResponse response;
	response._version = "HTTP/1.1";
  	response._status_code = 204;
  	response._status_text = getStatusText(204);
  	response._headers["Content-Length"] = "0";
  	response._headers["Connection"] = getConnectionMode(request._headers);

	return (response);
}
