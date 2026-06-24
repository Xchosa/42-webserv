#include "Dispatcher.hpp"

std::string Dispatcher::getFullUploadPath(LocationConfig* lc, std::string rootPath)
{
	std::string uploadDir;
	uploadDir= lc->_upload_store.value();
	
	if (uploadDir.find("./") == 0 )
	{
		uploadDir.erase(0,1);
		rootPath += uploadDir; 
	}
	else if(uploadDir.find('/') == 0)						// absolut to executable
	{
		rootPath += uploadDir;
	}
	else
	{
		rootPath += '/' + uploadDir;
	}
	return (rootPath);
}

std::string Dispatcher::buildFileName(const HttpRequest& request)
{
	std::string filename;
	// handle decoded and encoded URLs 
	// firs URL decode and regex check 
	std::regex traversalPattern("(^|/)\\.\\.(/|$)"); // 
	
	/*
		(^|/)    start of string OR slash
	\\.\\.   exactly two dots
	(/|$)    slash OR end of string

	So it matches:

	../x
	/a/../b
	/a/..
	..
	*/
	
	size_t filenameStart = request._path.find_last_of('/');
	if (filenameStart == std::string::npos )
		throw HttpException(500);
	filename = request._path.substr(filenameStart + 1);
	return filename;
}

void Dispatcher::createDirAndFile(const HttpRequest& request, std::string uploadpath, std::string target)
{
	std::filesystem::create_directories(uploadpath);
	std::ofstream NewFile(target, std::ios::binary);
	//std::ofstream NewFile(target);
	if (!NewFile.is_open())
		throw HttpException(500);
	NewFile.write(request._body.data(), request._body.size());
	//NewFile << request._body;
	if(!NewFile)
		throw HttpException(500);
	NewFile.close();
}

bool Dispatcher::fileExists(const std::string& target) const
{
	struct stat statbuf;
	if (stat(target.c_str(), &statbuf) == -1)
		return false;

	return S_ISREG(statbuf.st_mode); // chekc regularfile

}

HttpResponse Dispatcher::handleUpload(const HttpRequest& request, LocationConfig* lc)
{
	std::string uploadpath;
	bool fileExisted;
	std::cout<< "rootPath:  " << lc->_root << std::endl;

	std::string rootPath = getFullRootPath(lc); // anhaegen 
	std::cout<< "rootPath to danceserv:  " << rootPath << std::endl;
	if (lc->_upload_store.has_value())
		uploadpath = getFullUploadPath(lc, rootPath);
	else
	{
		std::cout << "now upload path" << std::endl; // or value_or
		throw HttpException(404);
	}
	std::string filename = buildFileName(request);
	
	std::cout << "requst path: " << request._path << std::endl;
	std::cout<< "filename: " <<filename << std::endl;
	std::cout << "uploadPath: " << uploadpath<< std::endl;

	
	std::string target = uploadpath + "/" + filename;

	fileExisted = fileExists(target);
	createDirAndFile(request, uploadpath,target);

	std::cout << "body size: " << request._body.size() << std::endl;
	std::cout << "body content: " << request._body << std::endl;
	
	HttpResponse respond;
	respond._version = "HTTP/1.1";
	if (fileExisted == true)
	{
		respond._status_code = 200;
		respond._status_text = "OK"; // overwrites file 204 No Content auch moeglich
	}
	else
	{
		respond._status_code = 201;
		respond._status_text = "Created";
	}
	// differ body content 
	respond._headers["Content-Length"] = "0";
	respond._headers["Connection"] = getConnectionMode(request._headers);
	
	return (respond);
}
