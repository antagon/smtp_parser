#ifndef _PARSER_H
#define _PARSER_H

#include <stddef.h>

#define CR '\r'
#define LF '\n'

struct parser
{
	int (*on_word) (struct parser *parser, const char *buff, size_t len);
	int (*on_eol) (struct parser *parser);
	void *user_data;
	char word_delim;
};

extern size_t parser_exec (struct parser *parser, const char *buff, size_t len);

#endif

