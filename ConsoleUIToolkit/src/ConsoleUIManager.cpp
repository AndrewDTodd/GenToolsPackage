#include <ConsoleUIManager.h>
#include <rootConfig.h>

#include <sstream>

namespace GenTools::ConsoleUIToolkit
{
	const NavigationInteraction ConsoleUIManager::DefaultGoBackCollection("Press Q or esc to go back to the previous page",
		{ ConsoleKey::Escape, ConsoleKey::Q },
		[]() {return ConsoleUIManager::GetInstance()->RollBackToPreviousCollection(); },
		[]() {return true; });

	const NavigationInteraction ConsoleUIManager::DefaultGoPackPage("Press esc to go back to the previous page",
		{ ConsoleKey::Escape },
		[]() {return ConsoleUIManager::GetInstance()->ActiveCollectionStack().top()->RollBackToPreviousPage(); },
		[]() {return true; });

	ConsoleUIManager::ConsoleUIManager() noexcept
	{}

	ConsoleUIManager::~ConsoleUIManager() noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

		if (_instance)
		{
			readLock.unlock();

			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			if (_instance)
			{
				_running.store(0);

				if (_operationThread && _operationThread->joinable())
					_operationThread->join();

				delete _instance;
				_instance = nullptr;

				TERMINAL::SetCanonicalMode();
			}
		}
		return;
	}

	ConsoleUIManager* ConsoleUIManager::GetInstance() noexcept
	{
		//Acquire concurrent read lock for read access
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

		if (!_instance)
		{
			//Release the read lock to prevent deadlock
			readLock.unlock();
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			if (!_instance)
			{
				_instance = new ConsoleUIManager();
			}
		}
		return _instance;
	}

	const std::stack<const PageCollection*>& ConsoleUIManager::ActiveCollectionStack() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

		return _pageCollectionStack;
	}

	void ConsoleUIManager::Initialize(std::string&& programTitle, std::vector<PageCollection>&& pageCollection, std::vector<std::string>&& programHeaders) noexcept
	{
		uint8_t expected = 0;
		if (_initialized.compare_exchange_strong(expected, 1))
		{
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

#if defined(_WIN_TARGET)
			TERMINAL::init_windows_terminal();
#endif

			TERMINAL::SetNonCanonicalMode();

			const_cast<std::string&>(_programTitle) = std::move(programTitle);
			const_cast<std::vector<PageCollection>&>(_pageCollections) = std::move(pageCollection);
			const_cast<std::vector<std::string>&>(_programHeaders) = std::move(programHeaders);

			_pageCollectionStack.push(&_pageCollections[0]);
		}
	}

	const std::thread* ConsoleUIManager::Run() noexcept
	{
		uint8_t expected = 0;
		if (_running.compare_exchange_strong(expected, 1))
		{
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			_operationThread = std::make_unique<std::thread>(std::bind(&ConsoleUIManager::InternalRun, this));
		}
		return _operationThread.get();
	}

	void ConsoleUIManager::ShutDown() noexcept
	{
		uint8_t expected = 1;
		if (_running.compare_exchange_strong(expected, 0))
		{
			if (_operationThread && _operationThread->joinable())
			{
				_operationThread->join();
			}

			TERMINAL::SetCanonicalMode();
		}
	}

	bool ConsoleUIManager::SetActiveCollection(const std::string& collectionName)
#if defined(_RELEASE)
		noexcept
#endif
	{
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_pageCollections.empty())
			throw std::logic_error("Must Initialize the instance and set the PageCollection!\nNo items in the PageCollection");
#endif
		readLock.unlock();
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		size_t newCollectionIndex = 0;
		for (const auto& collection : _pageCollections)
		{
			if (collection.CollectionName() == collectionName)
				break;

			newCollectionIndex++;
		}

		if (newCollectionIndex == 0)
		{
			if (_pageCollections.at(0).CollectionName() != collectionName)
			{
				LogError("Could not find any PageCollection with the name " + collectionName);
				return false;
			}
		}

		_pageCollectionStack.push(&_pageCollections.at(newCollectionIndex));

		return true;
	}

	const PageCollection& ConsoleUIManager::GetActiveCollection(bool& success)
#if defined(_RELEASE)
		noexcept
#endif
	{
		std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_pageCollectionStack.empty())
			throw std::logic_error("Must Initialize the instance and set the PageCollection!\nNo items in the PageCollection! No Active Collection");
#endif

		success = true;

		return *_pageCollectionStack.top();
	}

	bool ConsoleUIManager::RollBackToPreviousCollection()
#if defined(_RELEASE)
		noexcept
#endif
	{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_pageCollectionStack.empty())
			throw std::logic_error("Must Initialize the instance and set the PageCollection!\nNo items in the PageCollection");
#endif

		if (_pageCollectionStack.size() > 1)
		{
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			_pageCollectionStack.pop();
			return true;
		}
		return false;
	}

	void ConsoleUIManager::Print(const std::string& output) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		std::cout << '\n' << output << std::endl;
	}

	void ConsoleUIManager::LogError(const std::string& message) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		std::cout << '\n';
		TERMINAL::PRINT_ERROR(message);
	}

	void ConsoleUIManager::LogWarning(const std::string& message) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		std::cout << '\n';

		TERMINAL::PRINT_WARNING(message);
	}

	std::string ConsoleUIManager::GetInput(const std::string& prompt) const noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

		if (!prompt.empty())
			std::cout << prompt << std::endl;

		std::string input;
		std::cin >> input;

		return input;
	}

	void ConsoleUIManager::InternalRun() const noexcept
	{
		while (_running != 0)
		{
			try
			{
				Render();

				if (!HandleInput())
				{
					Print("Press any Key to continue...");
					TERMINAL::ReadKey();
				}
			}
			catch (const std::exception e)
			{
				LogError(e.what());
			}
		}
	}

	void ConsoleUIManager::Render() const
#if defined(_RELEASE)
		noexcept
#endif
	{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_initialized == 0)
			throw std::logic_error("Must Initialize the instance before it can render and start IO");
#endif

		TERMINAL::CLEAR_TERMINAL();

		// Title
		std::cout << _programTitle << std::endl;

		// Headers
		if (!_programHeaders.empty())
		{
			for (const std::string& s : _programHeaders)
			{
				std::cout << s << std::endl;
			}
		}

		std::cout << std::endl;

		const ConsolePage& page = _pageCollectionStack.top()->CurrentPage();

		// Page Title and headers/prompts
		std::cout << page.PageName() << std::endl;
		if (!page.PagePrompts().empty())
		{
			std::cout << page.PagePrompts() << std::endl;
		}
		std::cout << std::endl;

		// Page navigation options
		if (!page.NavigationInteractions().empty())
		{
			for (const NavigationInteraction& nav : page.NavigationInteractions())
			{
				if (nav.InteractionVisible())
				{
					if (!nav.InteractionPrompt().empty())
					{
						std::cout << nav.InteractionPrompt() << std::endl;
					}
					std::cout << nav.InteractionText() << std::endl;
				}
			}
		}

		// Page go back navigation
		if (page.GoBackInteraction().InteractionVisible())
		{
			std::cout << page.GoBackInteraction().InteractionPrompt() << std::endl;
			std::cout << page.GoBackInteraction().InteractionText() << std::endl;
		}

		// Page objects to display
		if (!page.ObjectsToDisplay().empty())
		{
			for (const IDisplayField& display : page.ObjectsToDisplay())
			{
				if (display.InteractionVisible())
				{
					std::cout << std::endl;
					if (!display.FieldMessage().empty())
						std::cout << display.FieldMessage() << std::endl;
					display.Display(std::cout);
					std::cout << std::endl;
				}
			}
		}

		// Page input option
		if (page.InputInteraction())
		{
			if (page.InputInteraction()->InteractionVisible())
			{
				std::cout << std::endl;
				std::cout << page.InputInteraction()->InputPrompt() << std::endl;
			}
		}

		std::cout << std::endl;
	}

	bool ConsoleUIManager::HandleInput() const
#if defined(_RELEASE)
		noexcept
#endif
	{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
		if (_initialized == 0)
			throw std::logic_error("Must Initialize the instance before it can render and start IO");
#endif

		const ConsolePage& page = _pageCollectionStack.top()->CurrentPage();

		if (page.InputInteraction())
		{
			char keyInput = TERMINAL::ReadKey();
			
			const auto& pageActions = _pageCollectionStack.top()->PageActions();
			auto pageActionsItr = pageActions.find(static_cast<ConsoleKey>(keyInput));
			if (pageActionsItr != pageActions.end())
			{
				return pageActionsItr->second();
			}

			LogError(std::string("Was unable to handle input. \'") + keyInput + "\' was not recognized as a valid command in this context");
			return false;
		}
		std::string input;
		if (!CancelableReadLine(input, page.GoBackInteraction().InteractionKeys()))
		{
			return page.GoBackInteraction().InteractionAction()();
		}
		input = ConvertToLowercase(input);
		return page.InputInteraction()->InputAction(input);
	}

	static FORCE_INLINE bool contains(const std::vector<ConsoleKey>& keys, ConsoleKey key)
	{
		return std::find(keys.begin(), keys.end(), key) != keys.end();
	}

	bool ConsoleUIManager::CancelableReadLine(std::string& value, const std::vector<ConsoleKey>& escapeKeys) const noexcept
	{
		value.clear();
		std::stringstream buffer;
		ConsoleKey key = static_cast<ConsoleKey>(TERMINAL::ReadKey());
		while (contains(escapeKeys, key) && key != ConsoleKey::Enter)
		{
			if (key == ConsoleKey::Backspace && TERMINAL::GetCursorPosition().column > 0)
			{
				auto cursorPos = TERMINAL::GetCursorPosition();
				TERMINAL::_CURSOR_LEFT("1");
				TERMINAL::_ERASE_TO_END_LINE;

				if (buffer.tellp() > 0)
				{
					buffer << to_char(ConsoleKey::Backspace);
				}

				std::cout << buffer.str();

			}
			else if (ConsoleKeyMethods::IsLetterOrDigit(key) || ConsoleKeyMethods::IsWhiteSpace(key) || ConsoleKeyMethods::IsPunctuation(key))
			{
				char ch = TERMINAL::ReadKey();
				buffer << ch;
				std::cout << ch;
				key = static_cast<ConsoleKey>(ch);
			}
			else if (key == ConsoleKey::ArrowLeft && std::cout.tellp() > 0)
			{
				std::cout << to_char(ConsoleKey::Backspace);
				key = static_cast<ConsoleKey>(TERMINAL::ReadKey());
			}
			else if (key == ConsoleKey::ArrowRight && std::cout.tellp() < buffer.str().length())
			{
				TERMINAL::_CURSOR_RIGHT("1");
				key = static_cast<ConsoleKey>(TERMINAL::ReadKey());
			}
			else
			{
				key = static_cast<ConsoleKey>(TERMINAL::ReadKey());
			}
		}

		if (key == ConsoleKey::Enter)
		{
			std::cout << std::endl;
			value = buffer.str();
			return true;
		}
		return false;
	}
}