/*
	ConsolePage
	<summary>
	This class defines the page structure used by the "ConsoleUIManager" instance
	Represents a meaningful collection of prompts to allow the user to navigate and direct the applications state/setting
	</summary>

	NavigationInteraction
	<summary>
	Used by the "ConsolePage" to define the navigation items of the page
	</summary>

	InputInteraction
	<summary>
	Used by the <seealso "ConsolePage" to handle input
	</summary>

	/// <summary>
	/// Used by the <seealso cref="ConsolePage"/> to link data from the an object to the UI
	/// </summary>

	Copyright 2024 Andrew Todd

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
	the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_PAGE_H
#define	GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_PAGE_H

#include <string>
#include <vector>
#include <memory>

#include <NavigationInteraction.h>
#include <DisplayField.h>
#include <InputInteraction.h>

namespace GenTools::ConsoleUIToolkit
{
	/// <summary>
	/// <para>This class defines the page structure used by the <seealso cref="ConsoleUIManager"/> instance</para>
	/// <para>Represents a meaningful collection of prompts to allow the user to navigate and direct the applications state/setting</para>
	/// <para>Allows for the handling of user-input requests by the program logic</para>
	/// </summary>
	class ConsolePage
	{
	private:
		const std::string _pageName;
		const std::string _pagePrompts;
		const std::vector<NavigationInteraction> _navigationInteractions;
		const NavigationInteraction _goBackInteraction;
		const std::vector<IDisplayField> _objectsToDisplay;
		const std::unique_ptr<ConsoleUIToolkit::InputInteraction> _inputInteraction;

	public:
		ConsolePage(NavigationInteraction&& goBack, std::string&& pageName, std::string&& pagePrompts,
			std::vector<NavigationInteraction>&& navigationInteractions, std::vector<IDisplayField>&& objectsToDisplay, std::unique_ptr<ConsoleUIToolkit::InputInteraction>&& inputInteraction = nullptr) noexcept;

		const std::string& PageName() const noexcept;
		const std::string& PagePrompts() const noexcept;
		const std::vector<NavigationInteraction>& NavigationInteractions() const noexcept;
		const NavigationInteraction& GoBackInteraction() const noexcept;
		const std::vector<IDisplayField>& ObjectsToDisplay() const noexcept;
		const std::unique_ptr<ConsoleUIToolkit::InputInteraction>& InputInteraction() const noexcept;
	};
}
#endif // !GENTOOLS_CONSOLE_UI_TOOLKIT_CONSOLE_PAGE_H