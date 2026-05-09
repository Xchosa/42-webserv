  #pragma once

  #include <string>


  std::string buildHelloWorldResponse()
  {
  	std::string body = "Hello World\n";

  	return "HTTP/1.1 200 OK\r\n"
  		   "Content-Type: text/plain\r\n"
  		   "Content-Length: " + std::to_string(body.size()) + "\r\n"
  		   "Connection: close\r\n"
  		   "\r\n" +
  		   body;
  }
