#pragma once

#include "HttpRequest.hpp"

enum ParseStatus
{
	COMPLETE,
	INCOMPLETE,
	ERROR
};

class HttpParser
{
	private:
		std::string		_raw_buffer;
		HttpRequest		_request;
		ParseStatus		_status;

	public:
		// OCF
		HttpParser() = default;
		HttpParser(const HttpParser& other) = default;
		HttpParser& operator=(const HttpParser& other) = default;
		~HttpParser() = default;

		// member functions
		ParseStatus feed(const char* data, size_t n);	// feed nach jedem recv() callen und dann status returnen (INCOMPLETE, COMPLETE, ...)
		const HttpRequest& getRequest() const;			// damit client auf request struct zugreifen kann, vorher auch pruefen ob status = COMPLETE!
		void reset();									// _raw_buffer, _request, _status leeren, damit parser bereit fuer naechsten request auf derselben verbindung
};
