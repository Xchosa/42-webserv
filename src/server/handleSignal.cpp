#include "Server.hpp"

volatile std::sig_atomic_t gSignalStatus = true;

void signalHandler(int sig)
{
	std::cout << "Interrupt handle " << sig << std::endl;

	if ((sig == SIGINT) || (sig == SIGTERM))
		gSignalStatus = false;
}

void initSignal(void)
{
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGPIPE, SIG_IGN);
}
