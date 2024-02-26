#include <rootConfig.h>

#if defined(_WIN_TARGET)
void init_windows_terminal()
{
	static bool windows_terminal_initialized = false;

	if (!windows_terminal_initialized)
	{
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
		windows_terminal_initialized = true;
	}
}
#endif