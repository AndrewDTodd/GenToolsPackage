#include <InputInteraction.h>

namespace GenTools::ConsoleUIToolkit
{
	InputInteraction::InputInteraction(std::string&& inputPrompt, std::function<bool(std::string&)> inputAction, std::function<bool()> interactionVisible) noexcept 
		: _inputPrompt(std::move(inputPrompt)), _inputAction(inputAction), _interactionVisible(interactionVisible)
	{}

	const std::string& InputInteraction::InputPrompt() const noexcept
	{
		return _inputPrompt;
	}

	bool InputInteraction::InputAction(std::string& input) const noexcept
	{
		return _inputAction(input);
	}

	bool InputInteraction::InteractionVisible() const noexcept
	{
		return _interactionVisible();
	}
}