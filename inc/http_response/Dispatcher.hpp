#pragma once

#include <iostream>
#include <string>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Dispatcher
{
	private:
		// handler
		HttpResponse handleStatic(HttpRequest& request, LocationConfig* lc); // baut response
		// handleStatic
		// handleCgi
		// handleUpload

		// helper
		LocationConfig* findLocation(const std::string& path, ServerConfig* sc) const;


	public:
		// OCF
		Dispatcher() = default;
		Dispatcher(const Dispatcher& other) = default;
		Dispatcher& operator=(const Dispatcher& other) = default;
		~Dispatcher() = default;

		// member functions
		HttpResponse dispatch(const HttpRequest& request, ServerConfig* sc); // sucht richtigen handler und passende location_config
};
