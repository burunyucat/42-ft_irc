/**
 * @file Signal.cpp
 * @brief Install and handle termination signals to request server shutdown.
 */

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

	// Report failed sends instead of terminating on a closed connection.
	action.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &action, NULL) == -1)
		return false;
	return true;
}
