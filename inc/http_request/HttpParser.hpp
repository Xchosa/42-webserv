#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <regex>
#include <cctype>
#include <charconv>

#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

inline const std::regex REGEX_REQUEST_LINE{R"(^(GET|POST|DELETE) \/[\x21-\x7E]{0,2047} HTTP\/1\.1$)"};
inline constexpr std::string_view FORBIDDEN_HOST_CHARS = "*?{}();\n\t#\"' \\/";
inline constexpr size_t MAX_HEADER_SIZE = 8192; // 8 kb

enum ParseStatus
{
	COMPLETE,
	HEADER_COMPLETE,
	INCOMPLETE,
	ERROR_400,
	ERROR_413
};

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY_CHUNKED,
	BODY_CONTENT_LEN,
	DONE
};

class HttpParser
{
	private:
		std::string		_raw_buffer;
		HttpRequest		_request;
		ParseStatus		_status = INCOMPLETE;
		ParseState		_state = REQUEST_LINE;
		size_t			_content_len_expected = 0;
		ServerConfig*	_client_server_config = nullptr;
		size_t			_total_header_size = 0;

		bool			extractLine(std::string& buffer, std::string &line);
		void			parseRequestLine(const std::string& line);
		void			parseHeader(const std::string& line);
		bool			decodePath(std::string& path);							// true on success, false on error

		// validation
		void			validateRequest();
		void			validateHeaderHost(const std::string& value);
		void			validateHeaderContentLen(const std::string& value);

	public:
		// OCF
		HttpParser() = default;
		HttpParser(const HttpParser& other) = default;
		HttpParser& operator=(const HttpParser& other) = default;
		~HttpParser() = default;

		// member functions
		ParseStatus			getStatus() const;
		ParseStatus 		parseChunkedBody();
		void		 		feedBuffer(const char* data, size_t n);		// call after each recv()
		ParseStatus			parseBuffer();
		void				setServerConfig(ServerConfig* conf);
		const HttpRequest&	getRequest() const;
		void				reset();									// clean attributs so parser is ready for new request on the same connection
	
		// for debugging
		void	printRawBuffer() const;
		void	printRequest() const;
};
