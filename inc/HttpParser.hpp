#pragma once

#include <string>
#include <iostream>
#include <sstream>

#include "HttpRequest.hpp"

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
		ParseStatus		_status;
		ParseState		_state = REQUEST_LINE;

		void			parseBuffer();
		bool			extractLine(std::string& buffer, std::string &line);
		void			parseRequestLine(const std::string& line);
		void			parseHeader(const std::string& line);

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
		std::string			TMP_getRawBuffer() const;

};
