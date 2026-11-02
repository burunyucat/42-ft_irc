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
}
