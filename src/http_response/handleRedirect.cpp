#include "Dispatcher.hpp"

HttpResponse Dispatcher::handleRedirect(LocationConfig* lc, const HttpRequest& request)
{
	HttpResponse r;
	int redirect_code = lc->_redirect_code.value();

	r._version = "HTTP/1.1";
	r._status_code = redirect_code;
	r._status_text = getStatusText(redirect_code);
	r._headers["Connection"] = getConnectionMode(request._headers);

	if (redirect_code >= 300 && redirect_code < 400)
	{
		r._headers["Content-Length"] = "0";
		r._headers["Location"] = lc->_redirect_url.value();
	}
	else if (lc->_redirect_url.has_value())
	{
		r._headers["Content-Type"] = "text/plain";
		r._headers["Content-Length"] = std::to_string(lc->_redirect_url.value().length());
		r._body = lc->_redirect_url.value();
	}
	else
	{
		r._body = getDefaultErrorBody(redirect_code);
		r._headers["Content-Type"] = "text/html";
		r._headers["Content-Length"] = std::to_string(r._body.length());
	}

	return (r);
}
