#include <ConsolePage.h>

namespace GenTools::ConsoleUIToolkit
{
	ConsolePage::ConsolePage(NavigationInteraction&& goBack, std::string&& pageName, std::string&& pagePrompts,
		std::vector<NavigationInteraction>&& navigationInteractions, std::vector<IDisplayField>&& objectsToDisplay, std::unique_ptr<ConsoleUIToolkit::InputInteraction>&& inputInteraction) noexcept 
		: _pageName(std::move(pageName)), 
		_pagePrompts(std::move(pagePrompts)), 
		_navigationInteractions(std::move(navigationInteractions)), 
		_goBackInteraction(std::move(goBack)), 
		_objectsToDisplay(std::move(objectsToDisplay)), 
		_inputInteraction(std::move(inputInteraction))
	{}

	const std::string& ConsolePage::PageName() const noexcept
	{
		return _pageName;
	}

	const std::string& ConsolePage::PagePrompts() const noexcept
	{
		return _pagePrompts;
	}

	const std::vector<NavigationInteraction>& ConsolePage::NavigationInteractions() const noexcept
	{
		return _navigationInteractions;
	}

	const NavigationInteraction& ConsolePage::GoBackInteraction() const noexcept
	{
		return _goBackInteraction;
	}

	const std::vector<IDisplayField>& ConsolePage::ObjectsToDisplay() const noexcept
	{
		return _objectsToDisplay;
	}

	const std::unique_ptr<ConsoleUIToolkit::InputInteraction>& ConsolePage::InputInteraction() const noexcept
	{
		return _inputInteraction;
	}
}