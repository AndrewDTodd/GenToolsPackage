#include <NavigationInteraction.h>

namespace GenTools::ConsoleUIToolkit
{
	NavigationInteraction::NavigationInteraction(std::string&& interactionText,
		std::vector<ConsoleKey>&& interactionKeys,
		std::function<bool()> interactionAction,
		std::function<bool()> interactionVisible,
		std::string&& interactionPrompt) noexcept 
		: _interactionPrompt(std::move(interactionPrompt)), 
		_interactionText(std::move(interactionText)), 
		_interactionKeys(std::move(interactionKeys)), 
		_interactionAction(interactionAction), 
		_interactionVisible(interactionVisible)
	{}

	const std::string& NavigationInteraction::InteractionPrompt() const noexcept
	{
		return _interactionPrompt;
	}

	const std::string& NavigationInteraction::InteractionText() const noexcept
	{
		return _interactionText;
	}

	const std::vector<ConsoleKey>& NavigationInteraction::InteractionKeys() const noexcept
	{
		return _interactionKeys;
	}

	const std::function<bool()> NavigationInteraction::InteractionAction() const noexcept
	{
		return _interactionAction;
	}

	bool NavigationInteraction::InteractionVisible() const noexcept
	{
		return _interactionVisible();
	}
}