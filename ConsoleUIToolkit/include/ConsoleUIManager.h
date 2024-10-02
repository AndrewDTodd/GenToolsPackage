/*
	ConsoleUIManager
	<summary>
	This singleton class is used to manage IO and provide a framework for console UI
	</summary>

	PageCollection
	<summary>
	Represents a meaningful collection of <seealso cref="ConsolePage"/> objects that the <seealso cref="ConsoleUIManager"/> can digest to create the UI
	</summary>

	Copyright 2024 Andrew Todd

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
	the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_UI_MANAGER_H
#define	GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_UI_MANAGER_H

#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <stack>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <atomic>

#include <ConsolePage.h>
#include <NavigationInteraction.h>
#include <InputInteraction.h>
#include <DisplayField.h>
#include <PageCollection.h>

namespace GenTools::ConsoleUIToolkit
{
	/// <summary>
	/// This singleton class is used to manage IO and provide a framework for console UI
	/// </summary>
	class ConsoleUIManager
	{
	private:
		inline static std::shared_mutex _sharedMutex;
		inline static ConsoleUIManager* _instance = nullptr;
		std::atomic<uint8_t> _initialized{ 0 };
		std::atomic<uint8_t> _running{ 0 };
		std::unique_ptr<std::thread> _operationThread;
		const std::string _programTitle;
		const std::vector<std::string> _programHeaders;
		const std::vector<PageCollection> _pageCollections;
		std::stack<const PageCollection*> _pageCollectionStack;

		/// <summary>
		/// <para>Handles the UI loop</para>
		/// <para>Called by the public method Run</para>
		/// </summary>
		void InternalRun() const noexcept;

		/// <summary>
		/// <para>Used internally to draw ConsolePages to the console</para>
		/// <para>Will throw a <seealso cref="std::logic_error"/> if called before instance initialization. Should only be seen by developers as this method can only be thrown in Debug or RelWithDebInfo build modes</para>
		/// </summary>
		/// <exception cref="std::logic_error"></exception>
		void Render() const
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// <para>Used internally to process the actions defined by the ConsolePage objects</para>
		/// <para>Will throw a <seealso cref="std::logic_error"/> if called before instance initialization. Should only be seen by developers as this method can only be thrown in Debug or RelWithDebInfo build modes</para>
		/// </summary>
		/// <returns>return true if the user input was handled successfully, false otherwise</returns>
		/// <exception cref="System.InvalidOperationException"></exception>
		bool HandleInput() const
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// Default constructor is private, ConsoleUIManager is a singleton. Internal use only
		/// </summary>
		ConsoleUIManager() noexcept;

	public:
		/// <summary>
		/// A <seealso cref="NavigationInteraction"/> that can be used by <seealso cref="PageCollection"/>s as a means to direct to the previous collection
		/// </summary>
		static const NavigationInteraction DefaultGoBackCollection;

		/// <summary>
		/// A <seealso cref="NavigationInteraction"/> that can be used by <seealso cref="ConsolePage"/>s as a means to direct to the previous page
		/// </summary>
		static const NavigationInteraction DefaultGoPackPage;

		/// <summary>
		/// Destructor, will shut down UI thread and dispose of resources. Prefer calling ShutDown first, but not necessary
		/// </summary>
		~ConsoleUIManager() noexcept;

		/// <summary>
		/// Get a pointer to the singletons active instance, or makes an instance if there isn't one already
		/// </summary>
		/// <returns>Pointer to the ConsoleUIManager instance</returns>
		static ConsoleUIManager* GetInstance() noexcept;

		/// <summary>
		/// Returns a immutable reference to the managers current stack
		/// </summary>
		/// /// <returns>Immutable reference to stack</returns>
		const std::stack<const PageCollection*>& ActiveCollectionStack() const noexcept;

		/// <summary>
		/// <para>Used to initialize the singleton, can only be called once.</para>
		/// <para>Sets the _programTile, _ProgramHeaders and _pageCollection to the respective arguments</para>
		/// </summary>
		/// <param name="programTitle">Name of the program</param>
		/// <param name="programHeaders">Headers to be applied to the UI</param>
		/// <param name="pageCollections">The pages of the program UI</param>
		void Initialize(std::string&& programTitle, std::vector<PageCollection>&& pageCollection, std::vector<std::string>&& programHeaders) noexcept;

		/// <summary>
		/// Spawns the UI loop on a new thread if the instance is not already running
		/// </summary>
		/// <returns>The thread handle to the thread representing the UI operation</returns>
		const std::thread* Run() noexcept;

		/// <summary>
		/// Called to tell Manager that shutdown is requested
		/// </summary>
		/// <remarks>Should only be called after the UI has been started. Will do nothing if this condition is not met</remarks>
		void ShutDown() noexcept;

		/// <summary>
		/// If the Manager contains a <seealso cref="PageCollection"/> with the entered name it will switch the context to that collection and return true. Returns false otherwise.
		/// </summary>
		/// <param name="collectionName">name of the <seealso cref="PageCollection"/> to switch to</param>
		/// <returns>true if the collection is found and switch succeeds, false otherwise</returns>
		bool SetActiveCollection(const std::string& collectionName)
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// Gets the active collection
		/// </summary>
		/// <param name="success">Set to true if there is an active collection, false otherwise</param>
		/// <returns>Immutable reference to the active <seealso cref="PageCollection"/></returns>
		/// <exception cref="std::logic_error">can be thrown in Debug or RelWithDebInfo build modes</exception>
		const PageCollection& GetActiveCollection(bool& success)
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// Switches the managers context to the previous <seealso cref="PageCollection"/> by popping current context off the stack
		/// </summary>
		/// <returns>true if operation succeeds, false otherwise</returns>
		bool RollBackToPreviousCollection()
#if defined(_RELEASE)
			noexcept
#endif
			;

		/// <summary>
		/// Prints the message specified in output to the terminal
		/// </summary>
		/// <param name="output">message to print</param>
		void Print(const std::string& output) const noexcept;

		/// <summary>
		/// Prints an error message to the terminal
		/// </summary>
		/// <param name="message">the error message to print</param>
		void LogError(const std::string& message) const noexcept;

		/// <summary>
		/// Prints a warning message to the terminal
		/// </summary>
		/// <param name="message">the warning message to print</param>
		void LogWarning(const std::string& message) const noexcept;

		/// <summary>
		/// Gets input from the user
		/// </summary>
		/// <param name="prompt">a prompt to show with the input line</param>
		/// <returns>the entry from the user as an immutable <seealso cref="std::string"/> reference</returns>
		std::string GetInput(const std::string& prompt) const noexcept;

		/// <summary>
		/// Get input from the user through a special input process that allows the user to exit the input prompt by pressing any of specified keys in 'escapeKeys'
		/// </summary>
		/// <param name="value">reference to a std::String where the output will be saved to when completed</param>
		/// <param name="escapeKeys">collection of <seealso cref="ConsoleKeys"/> that if encountered will cancel the operation</param>
		/// <returns>true if input was gathered, false otherwise</returns>
		bool CancelableReadLine(std::string& value, const std::vector<ConsoleKey>& escapeKeys) const noexcept;
	};
}
#endif // !GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_UI_MANAGER_H
