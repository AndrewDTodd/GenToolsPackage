#include <PageCollection.h>
#include <ConsoleUIManager.h>

#include <exception>

namespace GenTools::ConsoleUIToolkit
{
	PageCollection::PageCollection(std::string&& collectionName, std::vector<ConsolePage>&& pages) noexcept
		: _collectionName(std::move(collectionName)), _pages(std::move(pages))
	{
		const ConsolePage& page = _pages[0];

		_pageStack.push(&page);

		if (!page.NavigationInteractions().empty())
		{
			for (const NavigationInteraction& interaction : page.NavigationInteractions())
			{
				if (interaction.InteractionVisible())
				{
					for (ConsoleKey key : interaction.InteractionKeys())
					{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
						auto itrActions = _pageActions.find(key);
						if (itrActions != _pageActions.end())
						{
							_pageActions.emplace(key, interaction.InteractionAction());
						}
						else
							ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
						_pageActions.emplace(key, interaction.InteractionAction());
#endif
					}
				}
			}
		}

		auto& interaction = page.GoBackInteraction();
		for (ConsoleKey key : interaction.InteractionKeys())
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			auto itrActions = _pageActions.find(key);
			if (itrActions != _pageActions.end())
			{
				_pageActions.emplace(key, interaction.InteractionAction());
			}
			else
				ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
			_pageActions.emplace(key, interaction.InteractionAction());
#endif
		}
	}

	const std::string& PageCollection::CollectionName() const noexcept
	{
		return _collectionName;
	}

	size_t PageCollection::CurrentPageIndex() const noexcept
	{
		return _pageStack.size();
	}

	const ConsolePage& PageCollection::CurrentPage() const noexcept
	{
		return *_pageStack.top();
	}

	const std::unordered_map<ConsoleKey, const std::function<bool()>>& PageCollection::PageActions() const noexcept
	{
		return _pageActions;
	}

	bool PageCollection::RollBackToPreviousPage() const
#if defined(_RELEASE)
		noexcept
#endif
	{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_pageStack.empty())
			throw std::logic_error("Error: From PageCollection\n >>>Must initialize the instance and set the PageCollection. No items in the PageCollection");
#endif

		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		_pageStack.pop();

		_pageActions.clear();

		const ConsolePage& page = *_pageStack.top();

		if (!page.NavigationInteractions().empty())
		{
			for (const NavigationInteraction& interaction : page.NavigationInteractions())
			{
				if (interaction.InteractionVisible())
				{
					for (ConsoleKey key : interaction.InteractionKeys())
					{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
						auto itrActions = _pageActions.find(key);
						if (itrActions != _pageActions.end())
						{
							_pageActions.emplace(key, interaction.InteractionAction());
						}
						else
							ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
						_pageActions.emplace(key, interaction.InteractionAction());
#endif
					}
				}
			}
		}

		auto& interaction = page.GoBackInteraction();
		for (ConsoleKey key : interaction.InteractionKeys())
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			auto itrActions = _pageActions.find(key);
			if (itrActions != _pageActions.end())
			{
				_pageActions.emplace(key, interaction.InteractionAction());
			}
			else
				ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
			_pageActions.emplace(key, interaction.InteractionAction());
#endif
		}

		return true;
	}

	bool PageCollection::SetCurrentPage(const size_t& newPageIndex) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		if (newPageIndex < _pages.size())
		{
			_pageActions.clear();

			const ConsolePage& page = _pages.at(newPageIndex);

			_pageStack.push(&page);

			if (!page.NavigationInteractions().empty())
			{
				for (const NavigationInteraction& interaction : page.NavigationInteractions())
				{
					if (interaction.InteractionVisible())
					{
						for (ConsoleKey key : interaction.InteractionKeys())
						{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
							auto itrActions = _pageActions.find(key);
							if (itrActions != _pageActions.end())
							{
								_pageActions.emplace(key, interaction.InteractionAction());
							}
							else
								ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
							_pageActions.emplace(key, interaction.InteractionAction());
#endif
						}
					}
				}
			}

			auto& interaction = page.GoBackInteraction();
			for (ConsoleKey key : interaction.InteractionKeys())
			{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
				auto itrActions = _pageActions.find(key);
				if (itrActions != _pageActions.end())
				{
					_pageActions.emplace(key, interaction.InteractionAction());
				}
				else
					ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
				_pageActions.emplace(key, interaction.InteractionAction());
#endif
			}

			return true;
		}

		return false;
	}

	bool PageCollection::SetCurrentPage(const std::string& pageName) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		size_t newPageIndex = 0;
		for (const auto& page : _pages)
		{
			if (page.PageName() == pageName)
				break;

			newPageIndex++;
		}

		if (newPageIndex == 0)
		{
			if (_pages.at(0).PageName() != pageName)
				return false;
		}

		_pageActions.clear();

		const ConsolePage& page = _pages.at(newPageIndex);

		_pageStack.push(&page);

		if (!page.NavigationInteractions().empty())
		{
			for (const NavigationInteraction& interaction : page.NavigationInteractions())
			{
				if (interaction.InteractionVisible())
				{
					for (ConsoleKey key : interaction.InteractionKeys())
					{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
						auto itrActions = _pageActions.find(key);
						if (itrActions != _pageActions.end())
						{
							_pageActions.emplace(key, interaction.InteractionAction());
						}
						else
							ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
						_pageActions.emplace(key, interaction.InteractionAction());
#endif
					}
				}
			}
		}

		auto& interaction = page.GoBackInteraction();
		for (ConsoleKey key : interaction.InteractionKeys())
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			auto itrActions = _pageActions.find(key);
			if (itrActions != _pageActions.end())
			{
				_pageActions.emplace(key, interaction.InteractionAction());
			}
			else
				ConsoleUIManager::GetInstance()->LogError(std::string("Cannot add duplicate keys to page's action list. Offending key ") + static_cast<char>(key) + " already has an associated action on this page\nContinuing without adding");
#elif defined(_RELEASE)
			_pageActions.emplace(key, interaction.InteractionAction());
#endif
		}

		return true;
	}

	int64_t PageCollection::GetPageIndex(const std::string& pageName) const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

		size_t newPageIndex = 0;
		for (const auto& page : _pages)
		{
			if (page.PageName() == pageName)
				break;

			newPageIndex++;
		}

		if (newPageIndex == 0)
		{
			if (_pages.at(0).PageName() != pageName)
				return -1;
		}

		return newPageIndex;
	}
}