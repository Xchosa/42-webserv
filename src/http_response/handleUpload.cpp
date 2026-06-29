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
	
	std::string normalizedUploadDir = resolvePath(rootPath);
	

	return normalizedUploadDir;
}

std::string Dispatcher::buildFileName(const HttpRequest& request)
{
	std::string filename;
	// handle decoded and encoded URLs 
	// firs URL decode and regex check 
	//std::regex traversalPattern("(^|/)\\.\\.(/|$)"); // 

	size_t filenameStart = request._path.find_last_of('/');
	if (filenameStart == std::string::npos )
		throw HttpException(500);
	filename = request._path.substr(filenameStart + 1);

	
	return filename;
}



bool Dispatcher::fileExists(const std::string& target) const
{
	struct stat statbuf;
	if (stat(target.c_str(), &statbuf) == -1)
		return false;

	return S_ISREG(statbuf.st_mode); // chekc regularfile
}

std::string resolvePath(std::string NewPath)
{
	std::string path = std::filesystem::path(NewPath).lexically_normal();
	return path;
}

//bool isWithin(std::filesystem::path Base, std::filesystem::path targetPath)
//{
//	// durch itereiren 
//	std::filesystem::path normalizedBase = std::filesystem::absolute(Base).lexically_normal();
//	std::filesystem::path normalizedTarget = std::filesystem::absolute(targetPath).lexically_normal();

//	auto baseIter = normalizedBase.begin();
//	auto TargetIter = normalizedBase.begin();

//	for (; baseIter != normalizedBase.end(); ++baseIter, ++TargetIter)
//	{
//		if (TargetIter == normalizedTarget.end())
//	}
//}

// string rootpath , 





// spaeter in Dispatcher.cpp
bool isWithin(const std::string& base_path, std::string& full_path)
{
	std::cout << "base_path: " << base_path<< std::endl;
	std::cout << "full_path: " << full_path<< std::endl;

	return true;
}

// funktion die prueft ob pfad ausserhalb oder innerhalb ist
// strings als referenz uebergeben und dann gleich normalisiert "zurueckgeben", nur full pfad normalisiert zurueck benoetigt

// parameter 1: root pfad MIT location (bei upload noch den upload pfad zwischen root und location) 
// home/poverbec/projects/42-webserv/danceserv/uploads/maus
// home/poverbec/projects/42-webserv/danceserv/uploads/maus 

// parameter 2:
// parameter 1 + path
// parameter 1 + /maus/a/random/test.txt






bool Dispatcher::createDirAndFile(const HttpRequest& request, std::string uploadpath)
{
	// optional do not allow creating files through symlink (std::filesystem::is_symlink())
	bool fileExisted = false; // overrites a exiting file 
	bool allowed =false; // traveling behind root/uploads/maus +> only until maus 

	std::filesystem::path uploadRoot = std::filesystem::absolute(uploadpath).lexically_normal();
	std::cout << "uploadRoot " << uploadRoot << std::endl;

	
	//std::string requestPath = std::filesystem::relative(request._path);
	std::string requestPath = request._path;
	std::cout << "requestPath  " << request._path  << std::endl;

	allowed = isWithin(uploadRoot, requestPath);

	if(allowed)
		std::cout << " is within " << "true" <<std::endl;
	else
		std::cout << " is within " << "false" <<std::endl;


	std::string target = resolvePath(uploadpath + request._path);
	std::cout << "target: " << target<< std::endl;
	fileExisted = fileExists(target); // 

	std::string RequestDir = request._path;
	std::string Filename = buildFileName(request);
	std::string subDir = request._path.substr(0, request._path.find(Filename));
	std::string TargetDir = uploadpath + subDir;

	std::cout<< "TargetDir: " <<TargetDir << std::endl;

	std::string checkedTargetDir = resolvePath(TargetDir);
	// TragetDir muss innerhalb vom uploadpath sein  



	//bool filetravel = isWithin(request.)



	std::filesystem::create_directories(checkedTargetDir);
	std::ofstream NewFile(target, std::ios::binary);
	if (!NewFile.is_open())
	{
		std::cout << "createdDir" << checkedTargetDir << std::endl ;
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
																			// POST /maus/a/a/test.txt
																			// request._path = /maus/a/a/test.txt 
	std::string uploadpath;
	bool fileExisted;
	//std::cout<< "rootPath:  " << lc->_root << std::endl; 

	
	std::string rootPath = getFullRootPath(lc); // anhaegen 				// Root webserv/danceserv
	
	std::cout<< "rootPath to danceserv:  " << rootPath << std::endl;
	if (lc->_upload_store.has_value())
		uploadpath = getFullUploadPath(lc, rootPath); 						// webserv/danceserv/upload
	else
	{
		std::cout << "now upload path" << std::endl; // or value_or
		throw HttpException(404);
	}
	std::cout << "uploadPath: " << uploadpath<< std::endl;   /// 




	std::string filename = buildFileName(request);							// test.txt
	std::cout << "requst path: " << request._path << std::endl;
	std::cout<< "filename: " <<filename << std::endl;
	


	fileExisted = createDirAndFile(request, uploadpath);
	
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
	// differ body content 
	std::cout << "body size: " << request._body.size() << std::endl;
	std::cout << "body content: " << request._body << std::endl;
	respond._headers["Content-Length"] = "0";
	respond._headers["Connection"] = getConnectionMode(request._headers);
	
	return (respond);
}
