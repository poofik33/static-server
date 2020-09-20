#include <misc.h>

std::string errorStringCode(std::string str, int code)
{
	if (str.back() != ' ')
		str.append(" ");
	return str + "with error [" + std::string(strerror(code)) + "]";
}

int hexDigit(char c)
{
	if ('0' <= c && c <= 'f') {
		if (c <= '9') return c - '0';
		if ('A' <= c) {
			if (c <= 'F') return c - 'A' + 10;
			if ('a' <= c) return c - 'a' + 10;
		}
	}
	return -1;
}

std::string urlUnescape(const std::string &str)
{
	std::string result;

	if (str.empty()) return result;

	result.reserve(str.size());

	auto ptr = str.data();
	while(*ptr)
	{
		int h, l;
		if (*ptr == '%' &&
			(h = hexDigit(*(ptr+1))) >= 0 &&
			(l = hexDigit(*(ptr+2))) >= 0)
		{
			char c = (h << 4) + l;
			ptr += 3;

			result.push_back(c);
			continue;
		}
		else if (*ptr == '+')
		{
			result.push_back(' ');
			ptr++;
			continue;
		}

		result.push_back(*ptr);
		ptr++;
	}

	return result;
}
