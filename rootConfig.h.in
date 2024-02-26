//rootConfig.h.in, used for configuration defintions in project scope

#ifndef ROOT_CONFIG_CMDLINEPARSER_H
#define ROOT_CONFIG_CMDLINEPARSER_H

#include <iostream>

#define GREEN_TERMINAL std::cout << GREEN
#define RED_TERMINAL std::cout << RED
#define YELLOW_TERMINAL std::cout << YELLOW
#define BLUE_TERMINAL std::cout << BLUE
#define RESET_TERMINAL std::cout << RESET
#define DIMB_TEXT std::cout << DIMB
#define NORM_TEXT std::cout << NORM_TXT

#if defined(_LINUX_TARGET) || defined(_MAC_TARGET)
#include <cstdlib>
#define CLEAR_TERMINAL std::system("clear");

#define PRINT_WARNING(message) \
	do { \
		YELLOW_TERMINAL; \
		std::cerr << message << std::endl; \
		RESET_TERMINAL; \
	} while(0)

#define PRINT_ERROR(message) \
	do { \
		RED_TERMINAL; \
		std::cerr << message << std::endl; \
		RESET_TERMINAL; \
	} while(0)
#endif

#if defined(_WIN_TARGET)
#include <windows.h>
#define CLEAR_TERMINAL std::system("cls");

void init_windows_terminal();

#define PRINT_WARNING(message) \
	do { \
		init_windows_terminal(); \
		YELLOW_TERMINAL; \
		std::cerr << message << std::endl; \
		RESET_TERMINAL; \
	} while(0)

#define PRINT_ERROR(message) \
	do { \
		init_windows_terminal(); \
		RED_TERMINAL; \
		std::cerr << message << std::endl; \
		RESET_TERMINAL; \
	} while(0)
#endif

#endif //!ROOT_CONFIG_CMDLINEPARSER_H
