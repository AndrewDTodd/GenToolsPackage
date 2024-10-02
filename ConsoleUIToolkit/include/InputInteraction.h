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

#ifndef GENTOOLS_CONSOLE_UI_TOOLKIT_INPUT_INTERACTION_H
#define	GENTOOLS_CONSOLE_UI_TOOLKIT_INPUT_INTERACTION_H

#include <string>
#include <functional>

namespace GenTools::ConsoleUIToolkit
{
	class ConsolePage;

	/// <summary>
	/// Used by the <seealso cref="ConsolePage"/> to handle input
	/// </summary>
	class InputInteraction
	{
	private:
		const std::string _inputPrompt;
		const std::function<bool(std::string&)> _inputAction;
		const std::function<bool()> _interactionVisible;

	public:
		InputInteraction(std::string&& inputPrompt, std::function<bool(std::string&)> inputAction, std::function<bool()> interactionVisible = []() {return true; }) noexcept;

		const std::string& InputPrompt() const noexcept;
		bool InputAction(std::string& input) const noexcept;
		bool InteractionVisible() const noexcept;
	};
}
#endif // !GENTOOLS_CONSOLE_UI_TOOLKIT_INPUT_INTERACTION_H