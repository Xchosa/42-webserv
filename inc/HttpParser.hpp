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
		// getter fuer request + status
		void parse(int fd); // recv(), buffern in _raw_buffer, parsen in _request und _status setzen, vielleicht ParseStatus zurueckgeben?
};
