#include "stdio.h"
#include "ctype.h"
#include "string.h"

int vsprintf(char *str, const char *format, va_list ap) {

	if (!str)
		return 0;
	if (!format)
		return 0;
	size_t loc=0;
	size_t i;

	for (i=0 ; i<=strlen(format);i++, loc++)
	{
		if(format[i] != '%') {
			str[loc] = format[i];
			continue;
		}
		switch (format[i+1])
		{
		/*** characters ***/
		case 'c': {
			char c = va_arg (ap, char);
			str[loc] = c;
			i++;
			break;
		}

		/*** integers ***/
		case 'd':
		case 'i': {
			int c = va_arg (ap, int);
			char s[32]={0};
			itoa_s (c, 10, s);
			strcpy (&str[loc], s);
			loc+= strlen(s) - 2;
			i++;		// go to next character
			break;
		}

		/*** display in hex ***/
		case 'X':
		case 'x': {
			int c = va_arg (ap, int);
			char s[32]={0};
			itoa_s (c,16,s);
			strcpy (&str[loc], s);
			i++;		// go to next character
			loc+=strlen(s) - 2;
			break;
		}

		/*** strings ***/
		case 's': {
			int c = (int&) va_arg (ap, char);
			char s[32]={0};
			strcpy (s,(const char*)c);						
			strcpy (&str[loc], s);
			i++;		// go to next character
			loc+=strlen(s) - 2;
			break;
		}
		default:
			break;
		}
	}

	return loc - 2;
}