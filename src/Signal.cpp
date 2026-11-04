#include "Signal.hpp"
#include <cstddef>
#include <signal.h>

volatile sig_atomic_t Signal::stopRequested = 0;

void Signal::handleSignal(int signalNumber)
{
	if (signalNumber == SIGINT || signalNumber == SIGTERM)
		stopRequested = 1;
}

bool Signal::installHandlers()
{
	struct sigaction action = {};
	action.sa_handler = handleSignal;
	if (sigemptyset(&action.sa_mask) == -1)
		return false;
	action.sa_flags = 0;

	if (sigaction(SIGINT, &action, NULL) == -1)
		return false;
	if (sigaction(SIGTERM, &action, NULL) == -1)
		return false;
	return true;
}
