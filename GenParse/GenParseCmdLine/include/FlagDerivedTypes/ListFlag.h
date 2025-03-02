#ifndef GENTOOLS_GENPARSECMDLINE_LIST_FLAG_H
#define GENTOOLS_GENPARSECMDLINE_LIST_FLAG_H

#include <Flag.h>
#include <flag_argument.h>
#include <flag_argument_specializations.h>

namespace GenTools::GenParse
{
	template<IsFlagArgument BaseArg, bool Enclosed = false, bool Seperated = false>
	class ListFlag : virtual public Flag<Arg_List<BaseArg, Enclosed, Seperated>>
	{
	private:
		char m_leftDelimiter;
		char m_rightDelimiter;
		char m_separator;

		static bool TokenStartsWithFlag(const std::string_view& token)
		{
			return !token.empty() && token.front() == '-';
		}

	public:
		using list_argument_type = Arg_List<BaseArg, Enclosed, Seperated>;
		using containter_type = std::vector<typename BaseArg::value_type>;

		ListFlag(Tokens&& tokens, std::string&& flagDesc, 
			const char leftDelimiter = '[', const char rightDelimiter = ']', const char separator = ',')
			: Flag<list_argument_type>(std::move(tokens), std::move(flagDesc), list_argument_type(leftDelimiter, rightDelimiter, separator), true, false, false),
			m_leftDelimiter(leftDelimiter), m_rightDelimiter(rightDelimiter), m_separator(separator)
		{}

		ListFlag(Tokens&& tokens, std::string&& flagDesc, 
			bool flagRequired, bool posParsable, 
			const char leftDelimiter = '[', const char rightDelimiter = ']', const char separator = ',')
			: Flag<list_argument_type>(std::move(tokens), std::move(flagDesc), list_argument_type(leftDelimiter, rightDelimiter, separator), true, flagRequired, posParsable),
			m_leftDelimiter(leftDelimiter), m_rightDelimiter(rightDelimiter), m_separator(separator)
		{}

		// Gather tokens to form the complete list
		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			if (itr == end)
			{
				if (this->ArgRequired)
					throw std::invalid_argument("ListFlag missing argument tokens");

				return;
			}

			std::string collected;

			if constexpr (Enclosed)
			{
				// The first token must start with with the open delimiter
				std::string token(itr->data(), itr->size());
				if (token.empty() || token.front() != m_leftDelimiter)
					throw std::invalid_argument(std::string("Expected list to be enclosed in '") + m_leftDelimiter + "' and '" + m_rightDelimiter + "'");

				// Check if this token also ends with the closing delimiter
				if (token.back() == m_rightDelimiter)
				{
					// Remove both delimiters
					collected = token;
					++itr;
				}
				else
				{
					// Remove the opening delimiter from the first token
					collected = token;
					++itr;
					bool foundClose = false;
					
					// Continue collecting tokens until a token ending with m_rightDelimiter is found
					while (itr != end)
					{
						std::string next(itr->data(), itr->size());
						if (!next.empty() && next.back() == m_rightDelimiter)
						{
							collected += " " + next;
							++itr;
							foundClose = true;
							break;
						}
						else
						{
							collected += " " + next;
							++itr;
						}
					}

					if (!foundClose)
						throw std::invalid_argument(std::string("Expected closing '") + m_rightDelimiter + "' for list");
				}
			}
			else
			{
				// Not enclosed: gather tokens until a token starting with '-' is found, or end of tokens is reached
				while (itr != end && !TokenStartsWithFlag(*itr))
				{
					std::string token(itr->data(), itr->size());
					if (!collected.empty())
						collected += " ";

					collected += token;
					++itr;
				}
			}

			// Parse the concatenated string into the underlying list
			this->_flagArg.Parse(collected.c_str());
		}

		// Override TryRaise: similar logic as Raise, but using try/catch
		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			try
			{
				if (itr == end)
				{
					if (this->ArgRequired)
					{
						if (errorMsg)
							*errorMsg = "ListFlag missing argument tokens";

						return false;
					}

					return true;
				}

				std::string collected;
				if constexpr (Enclosed)
				{
					std::string token(itr->data(), itr->size());
					if (token.empty() || token.front() != m_leftDelimiter)
					{
						if (errorMsg)
							*errorMsg = std::string("Expected list to be enclosed in '") + m_leftDelimiter + "' and '" + m_rightDelimiter + "'";

						return false;
					}

					if (token.back() == m_rightDelimiter)
					{
						collected = token.substr(1, token.size() - 2);
						++itr;
					}
					else
					{
						collected = token;
						++itr;
						bool foundClose = false;
						while (itr != end)
						{
							std::string next(itr->data(), itr->size());
							if (!next.empty() && next.back() == m_rightDelimiter)
							{
								collected += " " + next;
								++itr;
								foundClose = true;
								break;
							}
							else
							{
								collected += " " + next;
								++itr;
							}
						}

						if (!foundClose)
						{
							if (errorMsg)
								*errorMsg = std::string("Expected closing '") + m_rightDelimiter + "' for list";

							return false;
						}
					}
				}
				else
				{
					while (itr != end && !TokenStartsWithFlag(*itr))
					{
						std::string token(itr->data(), itr->size());
						if (!collected.empty())
							collected += " ";

						collected += token;
						++itr;
					}
				}

				if (!this->_flagArg.TryParse(collected.c_str(), errorMsg))
					return false;

				return true;
			}
			catch (const std::exception& e)
			{
				if (errorMsg)
					*errorMsg = e.what();

				return false;
			}
		}
	};
}

#endif // !GENTOOLS_GENPARSECMDLINE_LIST_FLAG_H
