#pragma once

#include <csignal>

namespace Signal
{
	extern volatile sig_atomic_t stopRequested;

	// Request shutdown without performing cleanup inside the handler.
	void handleSignal(int signalNumber);

	// Install the SIGINT and SIGTERM handlers
	//
	// Return false on failure.
	bool installHandlers();
}
