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

#ifndef GENTOOLS_CONSOLE_UI_TOOLKIT_DISPLAY_FIELD_H
#define	GENTOOLS_CONSOLE_UI_TOOLKIT_DISPLAY_FIELD_H

#include <string>
#include <type_traits>
#include <ostream>
#include <functional>

namespace GenTools::ConsoleUIToolkit
{
	/// <summary>
	/// Interface used to reference <seealso cref="DisplayField"/> specializations polymorphically
	/// </summary>
	class IDisplayField
	{
	public:
		virtual const std::string& FieldMessage() const noexcept = 0;
		virtual void Display(std::ostream& os) const noexcept = 0;
		virtual bool InteractionVisible() const noexcept = 0;
	};

	template<typename T>
	concept Streamable = requires(T t, std::ostream& os)
	{
		{ os << t } -> std::same_as<std::ostream&>;
	};

	class ConsolePage;

	/// <summary>
	/// Used by the <seealso cref="ConsolePage"/> to link data from the an object to the UI
	/// </summary>
	template<Streamable DispObj>
	class DisplayField : public IDisplayField
	{
	private:
		const std::string _fieldMessage;
		const DispObj& _objectToDisplay;
		const std::function<bool()> _interactionVisible;

	public:
		DisplayField(std::string&& message, const DispObj& toDisplay, std::function<bool()> interactionVisible = []() {return true; }) noexcept
			: _fieldMessage(std::move(message)), _objectToDisplay(toDisplay), _interactionVisible(interactionVisible)
		{}

		const std::string& FieldMessage() const noexcept final
		{
			return _fieldMessage;
		}

		void Display(std::ostream& os) const noexcept final
		{
			os << _objectToDisplay;
		}

		bool InteractionVisible() const noexcept final
		{
			return _interactionVisible();
		}
	};
}
#endif // !GENTOOLS_CONSOLE_UI_TOOLKIT_DISPLAY_FIELD_H