#include "Dispatcher.hpp"




HttpResponse Dispatcher::handleDelete(const HttpRequest& request, LocationConfig* lc)
{

	bool file_existed; 

	std::string full_request_path = getFullRootPath(lc)+ request._path;
	//bool is_deletable = isWithin(lc->_name,);
	bool is_deletable = false;
	// config files, source files, or arbitrary filesystem paths are forbidden 


	HttpResponse respond;
	respond._version = "HTTP/1.1";
	if (file_existed && is_deletable== true)
	{
		respond._status_code = 204;
		respond._status_text = getStatusText(respond._status_code); // overwrites file 204 No Content auch moeglich
	}
	else if (!is_deletable) 
	{
		respond._status_code = 403;
		respond._status_text = getStatusText(respond._status_code);
	}

	respond._headers["Content-Length"] = "0";
	respond._headers["Connection"] = getConnectionMode(request._headers);
	
	return (respond);
}