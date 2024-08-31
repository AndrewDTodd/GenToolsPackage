#ifndef CMD_LINE_PARSER_H
#define CMD_LINE_PARSER_H

#include <unordered_map>
#include <unordered_set>
#include <string_view>

#include "Flags.h"

namespace TokenValueParser
{
	class Parser
	{
	private:
		std::unordered_set<const Flag&> requiredFlags;
		std::unordered_set<const Trigger&> triggerFlags;
		std::unordered_set<const NestedFlags&> nestedFlags;
		std::unordered_set<const Flag&> generalFlags;

		std::unordered_multimap<std::string_view, Flag&> parsedArgsMap;

	public:
		Parser(std::initializer_list<Flag> flags)
		{
			for (const auto& flag : flags)
			{

			}
		}
	};
}
#endif // !CMD_LINE_PARSER_H
