#include "Dispatcher.hpp"

std::string Dispatcher::getFullUploadPath(LocationConfig* lc, std::string rootPath)
{
	std::string uploadDir = lc->_upload_store.value();
	
	if(uploadDir.find('/') == 0)						// absolut to executable
	{
		rootPath += uploadDir;
	}
	else
	{
		rootPath += '/' + uploadDir;
	}
	std::string selectedLocationUploadDir = rootPath += lc->_name;
	std::string normalizedUploadDir = resolvePath(selectedLocationUploadDir);
	

	return normalizedUploadDir;
}

std::string Dispatcher::buildFileName(std::string user_path)
{
	size_t filename_start = user_path.find_last_of('/');
	if (filename_start == std::string::npos || filename_start == (std::string::npos -1))
		throw HttpException(400);
	std::string filename = user_path.substr(filename_start + 1);	
	return filename;
}

void Dispatcher::validateUploadTarget(const std::string& target) const
{
	struct stat statbuf;

	if(target.empty())
		throw HttpException(400);

	if(target[target.length() -1] == '/')
		throw HttpException(400);
	if(lstat(target.c_str(), &statbuf) == -1)
	{
		if (errno == ENOENT)
  			return; // pass
		throw HttpException(500);
	}
	//if (stat(target.c_str(), &statbuf) == -1) // follows symlinks
	//		return false;
	if (S_ISLNK(statbuf.st_mode)) // check for symlink 
		throw  HttpException(400);
	if (S_ISDIR(statbuf.st_mode))
		throw HttpException(400);
	if(S_ISREG(statbuf.st_mode)) // check regularfile / got permissions / overwrite
		return ;
	throw HttpException(400);
}


bool Dispatcher::fileExists(const std::string& target) const
{
	struct stat statbuf;
	if(lstat(target.c_str(), &statbuf) == -1) // does not follow symliks 
	{
		if(errno == ENOENT )
			return false;
		throw HttpException(500);
	}
	if(S_ISREG(statbuf.st_mode) == 0) // check regularfile / got permissions
		return true;
	
	return false;

}

std::string resolvePath(std::string new_path)
{
	std::string path = std::filesystem::path(new_path).lexically_normal();
	return path;
}



bool Dispatcher::createDirAndFile(const HttpRequest& request, std::string user_path)
{
	bool fileExisted;

	fileExisted = fileExists(user_path); // 
	std::string filename = buildFileName(user_path);
	std::string subDir = user_path.substr(0, user_path.find(filename));
	std::string checkedTargetDir = resolvePath(subDir);
	std::filesystem::create_directories(checkedTargetDir);

	std::ofstream NewFile(user_path, std::ios::binary);
	if (!NewFile.is_open())
	{
		std::cout << "[INFO] Permission Error " << checkedTargetDir << std::endl ;
		throw HttpException(500);
	}
	NewFile.write(request._body.data(), request._body.size());
	if(!NewFile)
		throw HttpException(500);
	NewFile.close();

	return fileExisted ;
}


HttpResponse Dispatcher::handleUpload(const HttpRequest& request, LocationConfig* lc)
{
	std::cout << "[INFO]  entered upload handler" << std::endl;

	std::string uploadpath;
	bool fileExisted;
	
	if (lc->_upload_store.has_value())
	{
		uploadpath = getFullRootPath(lc) + "/" + lc->_upload_store.value();
	}
	else
	{
		std::cout << "[INFO] upload_path not given" << std::endl;
		throw HttpException(500);
	}	
	std::string user_path = uploadpath + request._path;
	validateUploadTarget(user_path);
	isWithin(uploadpath + lc->_name, user_path);
	fileExisted = createDirAndFile(request, user_path);
	
	std::cout << "[INFO] Location of uploadfile: " << user_path << std::endl;

	HttpResponse respond;
	respond._version = "HTTP/1.1";
	if (fileExisted == true)
	{
		respond._status_code = 200;
		respond._status_text = getStatusText(respond._status_code); // overwrites file 204 No Content auch moeglich
	}
	else
	{
		respond._status_code = 201;
		respond._status_text = getStatusText(respond._status_code);
	}
	respond._headers["Content-Length"] = "0";
	respond._headers["Connection"] = getConnectionMode(request._headers);
	
	return (respond);
}
