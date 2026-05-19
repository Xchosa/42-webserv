#pragma once

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Dispatcher
{
	private:
		// handler
		HttpResponse handleStatic(HttpRequest& request, LocationConfig& location_config); // baut response
		// handleStatic
		// handleCgi
		// handleUpload

	public:
		// OCF
		Dispatcher() = default;
		Dispatcher(const Dispatcher& other) = default;
		Dispatcher& operator=(const Dispatcher& other) = default;
		~Dispatcher() = default;

		// member functions
		HttpResponse dispatch(const HttpRequest& request, ServerConfig* server_config); // sucht richtigen handler und passende location_config
};
