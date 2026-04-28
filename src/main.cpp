#include "webserv.hpp"




#define MAX_EVENTS 10

ServerConfig dummyParser();
Config hardcodedConfigParser();



int main()
{
	try{
		Config config;
		config._servers.push_back(dummyParser());
		
		Server server(config);
		//server.run();
	}
	catch( const std::exception &e){
		std::cerr << "Error" << e.what() <<std::endl;
		return 1;
	}
	return 0;
}

// check with debugger what is filled and what is not