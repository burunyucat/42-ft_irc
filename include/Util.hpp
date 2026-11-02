#pragma once

#include <string>
#include <stdexcept>

namespace Util
{
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

	// -----------------------------------
	// ------------- COLORS --------------
	// -----------------------------------

	#define RESET "\033[0m"

	// Standard test colors
	#define BLACK "\033[30m"
	#define RED "\033[31m"
	#define GREEN "\033[32m"
	#define YELLOW "\033[33m"
	#define BLUE "\033[34m"
	#define MAGENTA "\033[35m"
	#define CYAN "\033[36m"
	#define WHITE "\033[37m"

	// Bright text colors
	#define B_BLACK "\033[90m"
	#define B_RED "\033[91m"
	#define B_GREEN "\033[92m"
	#define B_YELLOW "\033[93m"
	#define B_BLUE "\033[94m"
	#define B_MAGENTA "\033[95m"
	#define B_CYAN "\033[96m"
	#define B_WHITE "\033[97m"

	// Background colors
	#define BG_BLACK "\033[40m"
	#define BG_RED "\033[41m"
	#define BG_GREEN "\033[42m"
	#define BG_YELLOW "\033[43m"
	#define BG_BLUE "\033[44m"
	#define BG_MAGENTA "\033[45m"
	#define BG_CYAN "\033[46m"
	#define BG_WHITE "\033[47m"

	// Text styles
	#define BOLD "\033[1m"
	#define UNDERLINE "\033[4m"
}
