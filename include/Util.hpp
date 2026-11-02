#pragma once

#include <string>
#include <stdexcept>

namespace Util
{
	// -----------------------------------
	// ------- TERMINAL COLORS -----------
	// -----------------------------------

	// ANSI reset sequence
	const char* const RESET = "\033[0m";

	// Standard text colors
	const char* const BLACK = "\033[30m";
	const char* const RED = "\033[31m";
	const char* const GREEN = "\033[32m";
	const char* const YELLOW = "\033[33m";
	const char* const BLUE = "\033[34m";
	const char* const MAGENTA = "\033[35m";
	const char* const CYAN = "\033[36m";
	const char* const WHITE = "\033[37m";

	// Bright text colors
	const char* const B_BLACK = "\033[90m";
	const char* const B_RED = "\033[91m";
	const char* const B_GREEN = "\033[92m";
	const char* const B_YELLOW = "\033[93m";
	const char* const B_BLUE = "\033[94m";
	const char* const B_MAGENTA = "\033[95m";
	const char* const B_CYAN = "\033[96m";
	const char* const B_WHITE = "\033[97m";

	// Background colors
	const char* const BG_BLACK = "\033[40m";
	const char* const BG_RED = "\033[41m";
	const char* const BG_GREEN = "\033[42m";
	const char* const BG_YELLOW = "\033[43m";
	const char* const BG_BLUE = "\033[44m";
	const char* const BG_MAGENTA = "\033[45m";
	const char* const BG_CYAN = "\033[46m";
	const char* const BG_WHITE = "\033[47m";

	// Text styles
	const char* const BOLD = "\033[1m";
	const char* const UNDERLINE = "\033[4m";

    // -----------------------------------
	// ------------ METHODS --------------
	// -----------------------------------

	// Returns a string with each type of whitespace character
	const std::string& getWhiteSpaces();

	// Send a msg to std::cerr
	//
	// Add a new line
	void sendErrorMsg(const std::string& msg);

	// -----------------------------------
	// ------------TEMPLATES -------------
	// -----------------------------------

	// Iterates an iterator by step
	//
	// If iterator equals sourceEnd before or during the process, stops the iteration and throw an exception if throwException is true
	//
	// Returns the iterator
	template <typename TIterator>
	TIterator safeAdvanceIterator(TIterator& ref_iterator, TIterator sourceEnd, size_t step, bool throwException)
	{
		do
		{
			if (ref_iterator == sourceEnd)
			{
				if (throwException)
					throw std::out_of_range("safeAdvanceIterator: end reached");
				break;
			}
			ref_iterator++;
		}
		while (--step > 0);
		return ref_iterator;
	}

	// Creates a copy of iterator and iterates it by step
	//
	// If this copy equals sourceEnd before or during the process, stops the iteration and throw an exception if throwException is true
	//
	// Returns the copy iterator
	template <typename TIterator>
	TIterator safeCheckIterator(TIterator iterator, TIterator sourceEnd, size_t step, bool throwException)
	{
		do
		{
			if (iterator == sourceEnd)
			{
				if (throwException)
					throw std::out_of_range("safeCheckIterator: end reached");
				break;
			}
			iterator++;
		}
		while (--step > 0);
		return iterator;
	}
}
