#include <stddef.h>

#include "parser.h"

size_t
parser_exec (struct parser *parser, const char *buff, size_t len)
{
	const char *buff_pos, *buff_evt_pos;
	int eol;

	eol = 0;

	for ( buff_evt_pos = buff, buff_pos = buff; buff_pos < (buff + len); buff_pos++ ){

		if ( *buff_pos == parser->word_delim ){

			if ( parser->on_word (parser, buff_evt_pos, (buff_pos - buff_evt_pos)) < 1 )
				return buff_pos + 1 - buff;

			buff_evt_pos = (const char*) (buff_pos + 1);

		} else if ( *buff_pos == CR ){
			eol += 0x01;
		} else if ( *buff_pos == LF ){
			eol += 0x02;
		}

		if ( ((eol & 0x01) != 0) && ((eol & 0x02) != 0) ){

			if ( parser->on_word (parser, buff_evt_pos, (buff_pos - buff_evt_pos) - 1) < 1 )
				return buff_pos + 1 - buff;

			buff_evt_pos = (const char*) (buff_pos + 1);

			if ( parser->on_eol (parser) < 1 )
				return buff_pos + 1 - buff;
		}
	}

	return buff_pos - buff;
}

