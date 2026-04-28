#include "webserv.hpp"

#define PORT 8082

ServerConfig dummyParser()
{
	ServerConfig Config;

	Config._listen_host = "127.0.0.1";
	Config._listen_port = PORT;
	Config._server_names.push_back("localhost");
	Config._error_pages[404] = "/404.html";

	LocationConfig default_location;
	default_location._path = "/";
	default_location._root = "/usr/share/nginx/html";
	default_location._index = "index.html";

	// try_files $uri $uri/ =404; is complex logic not directly mapped to a simple variable.
	// It implies checking for a file, then a directory, and finally returning a 404.
	Config._locations.insert({"/", default_location}); // / als key -> returns config 

	return Config;
}


Config hardcodedConfigParser()
{
    Config config;

    // Server 1: listen 8081
    {
        ServerConfig sc;
        sc._listen_port = 8081;
        sc._listen_host = "0.0.0.0";
		// sc._root = "./html/static"; 
    	//sc._index = "index.html";
   		//sc._autoindex = false; 
        LocationConfig lc;
        lc._path = "/";
        lc._methods = {"GET", "POST", "DELETE"};
        sc._locations["/"] = lc;
        config._servers.push_back(sc);
    }

    // Server 2: listen 8081, server_name example.com
    {
        ServerConfig sc;
        sc._listen_port = 8081;
        sc._listen_host = "0.0.0.0";
        sc._server_names.push_back("example.com");
        //sc._root = "./html/static";
        //sc._index = "static_index.html";
        //sc._autoindex = false;
        LocationConfig lc;
        lc._path = "/";
        lc._methods = {"GET", "POST", "DELETE"};
        sc._locations["/"] = lc;
        config._servers.push_back(sc);
    }

    // Server 3: listen 8082
    {
        ServerConfig sc;
        sc._listen_port = 8082;
        //sc._autoindex = true;
        //sc._root = "./html/directory";

        LocationConfig lc_root;
        lc_root._path = "/";
        lc_root._autoindex = true;
        sc._locations["/"] = lc_root;

        LocationConfig lc_wrong;
        lc_wrong._path = "/wrong/url";
        lc_wrong._redirect_code = 444;
        sc._locations["/wrong/url"] = lc_wrong;

		
        LocationConfig lc_moved;
        lc_moved._path = "/permanently/moved/url";
        lc_moved._redirect_code = 301;
        lc_moved._redirect_url = "http://www.example.com/moved/here";
        sc._locations["/permanently/moved/url"] = lc_moved;
        config._servers.push_back(sc);
    }


    return config;
}