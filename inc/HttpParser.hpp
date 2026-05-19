#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <regex>

#include "HttpRequest.hpp"

inline const std::regex REGEX_REQUEST_LINE{R"(^(GET|POST|DELETE) [\x21-\x7E]{1,2048} HTTP/1\.1$)"};

enum ParseStatus
{
	COMPLETE,
	INCOMPLETE,
	ERROR
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

		void			parseBuffer();
		bool			extractLine(std::string& buffer, std::string &line);
		void			parseRequestLine(const std::string& line);
		void			parseHeader(const std::string& line);

		void			validateRequest();

	public:
		// OCF
		HttpParser() = default;
		HttpParser(const HttpParser& other) = default;
		HttpParser& operator=(const HttpParser& other) = default;
		~HttpParser() = default;

		// member functions
		ParseStatus			getStatus() const;
		ParseStatus	 		feed(const char* data, size_t n);	// feed nach jedem recv() callen und dann status returnen (INCOMPLETE, COMPLETE, ...)

		// const HttpRequest&	getRequest() const;					// damit client auf request struct zugreifen kann, vorher auch pruefen ob status = COMPLETE!
		// void				reset();							// _raw_buffer, _request, _status leeren, damit parser bereit fuer naechsten request auf derselben verbindung
		
		// for debugging
		void	printRawBuffer() const;
		void	printRequest() const;
};
