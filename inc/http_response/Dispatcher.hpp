#pragma once

#include <iostream>
#include <string>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

enum ConnectionMode
{
	CON_CLOSE,
	CON_KEEP_ALIVE
};

class Dispatcher
{
	private:
		// handler (baut response)
		HttpResponse handleRedirect(LocationConfig* lc);
		HttpResponse handleStatic(const HttpRequest& request, LocationConfig* lc);
		// handleCgi
		// handleUpload

		// helper
		LocationConfig*	findLocation(const std::string& path, ServerConfig* sc) const;
		void			checkMethodAllowed(std::string method, std::vector<std::string> allowed_methods) const;
		std::string		readFile(std::string& filepath) const;
		std::string		cwd() const;
		std::string		getDefaultBody(int code) const;
		std::string		getFullRootPath(LocationConfig* lc) const;

	public:
		// OCF
		Dispatcher() = default;
		Dispatcher(const Dispatcher& other) = default;
		Dispatcher& operator=(const Dispatcher& other) = default;
		~Dispatcher() = default;

		// member functions
		HttpResponse	dispatch(const HttpRequest& request, ServerConfig* sc); // sucht richtigen handler und passende location_config
		HttpResponse 	buildErrorResponse(int code, ServerConfig* sc, ConnectionMode cm);
};
