/*
	Copyright 2024 Andrew Todd

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
	the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GENTOOLS_CONSOLE_UI_TOOLKIT_PAGE_COLLECTION_H
#define	GENTOOLS_CONSOLE_UI_TOOLKIT_PAGE_COLLECTION_H

#include <string>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <vector>
#include <stack>
#include <unordered_map>
#include <functional>
#include <stdint.h>

#include <ConsolePage.h>

namespace GenTools::ConsoleUIToolkit
{
	/// <summary>
	/// Represents a meaningful collection of <seealso cref="ConsolePage"/> objects that the <seealso cref="ConsoleUIManager"/> can digest to create the UI
	/// </summary>
	class PageCollection
	{
	private:
		mutable std::shared_mutex _sharedMutex;
		const std::string _collectionName;
		const std::vector<ConsolePage> _pages;
		mutable std::stack<const ConsolePage*> _pageStack;
		mutable std::unordered_map<ConsoleKey, const std::function<bool()>> _pageActions;

	public:
		PageCollection(std::string&& collectionName, std::vector<ConsolePage>&& pages) noexcept;

		/// <summary>
		/// Returns the name of the collection
		/// </summary>
		/// <returns>immutable reference to collections name string object</returns>
		const std::string& CollectionName() const noexcept;

		/// <summary>
		/// Returns the index of the current page
		/// </summary>
		/// <returns>index of page</returns>
		size_t CurrentPageIndex() const noexcept;

		/// <summary>
		/// Returns the current page
		/// </summary>
		/// <returns>immutable reference to the page object</returns>
		const ConsolePage& CurrentPage() const noexcept;

		/// <summary>
		/// Returns the page actions collection
		/// </summary>
		/// <returns>immutable reference to the page actions map</returns>
		const std::unordered_map<ConsoleKey, const std::function<bool()>>& PageActions() const noexcept;

		/// <summary>
		/// Reads the page stack and rolls back to the last page before the current one
		/// </summary>
		/// <returns>true if success, false otherwise</returns>
		bool RollBackToPreviousPage() const
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// Sets the current page to the index specified if its within the range of the collection
		/// </summary>
		/// <remarks>
		/// This should be used cautiously in an environment where multiple threads may call it. Prefer the overload with string param in such cases
		/// </remarks>
		/// <param name="newPageIndex">index of the page to set</param>
		/// <returns>true if set, false otherwise</returns>
		bool SetCurrentPage(const size_t& newPageIndex) const noexcept;

		/// <summary>
		/// Sets the current page to the page with pageName argument if it exists
		/// </summary>
		/// <remarks>
		/// This is the preferred method when multiple threads may be trying to change the page
		/// </remarks>
		/// <param name="pageName">name of the page to switch to</param>
		/// <returns>true if set, false otherwise</returns>
		bool SetCurrentPage(const std::string& pageName) const noexcept;

		/// <summary>
		/// Used to query the List of ConsolePages to get an index
		/// </summary>
		/// <param name="pageName"></param>
		/// <returns>Return the index of the ConsolePage with the name pageName if one exists, -1 otherwise</returns>
		int64_t GetPageIndex(const std::string& pageName) const noexcept;
	};
}
#endif // !GENTOOLS_CONSOLE_UI_TOOLKIT_PAGE_COLLECTION_H