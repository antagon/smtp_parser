#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

#include "smtp_parser.h"

#define CMD_MATCHES(str, cmd, str_len) (strncasecmp (str, cmd, str_len) == 0)
#define CMD_ENABLED(mask, cmd) (((mask) & (cmd)) != 0)

static int
_parser_on_word_cb (struct parser *parser, const char *buff, size_t len)
{
	struct smtp_arg *arg;

	if ( len == 0 )
		return 1;

	arg = (struct smtp_arg*) parser->user_data;

	if ( arg->type == 0 ){
		if ( CMD_MATCHES (buff, "HELO", len) )
			arg->type = SMTP_C_HELO;
		else if ( CMD_MATCHES (buff, "EHLO", len) )
			arg->type = SMTP_C_EHLO;
		else if ( CMD_MATCHES (buff, "MAIL", len) )
			arg->type = SMTP_C_MAILFROM;
		else if ( CMD_MATCHES (buff, "RCPT", len) )
			arg->type = SMTP_C_RCPTTO;
		else if ( CMD_MATCHES (buff, "DATA", len) )
			arg->type = SMTP_C_DATA;
		else if ( CMD_MATCHES (buff, ".", len) )
			arg->type = SMTP_C_EOF;
		else if ( CMD_MATCHES (buff, "RSET", len) )
			arg->type = SMTP_C_RSET;
		else if ( CMD_MATCHES (buff, "VRFY", len) )
			arg->type = SMTP_C_VRFY;
		else if ( CMD_MATCHES (buff, "NOOP", len) )
			arg->type = SMTP_C_NOOP;
		else if ( CMD_MATCHES (buff, "QUIT", len) )
			arg->type = SMTP_C_QUIT;
		else
			arg->type = SMTP_C_UNKNOWN;

		return 1;
	}

	switch ( arg->type ){
		case SMTP_C_HELO:
		case SMTP_C_EHLO:
		case SMTP_C_MAILFROM:
		case SMTP_C_RCPTTO:
		case SMTP_C_VRFY:
		case SMTP_C_UNKNOWN:
			arg->argv[arg->argc].val = buff;
			arg->argv[arg->argc].len = len;
			arg->argc++;
			break;
		case SMTP_C_DATA:
		case SMTP_C_RSET:
		case SMTP_C_NOOP:
		case SMTP_C_QUIT:
			// no additional parameters are expected for these commands
			break;
	}

	return 1;
}

static int
_parser_on_eol_cb (struct parser *parser)
{
	struct smtp_arg *arg;

	arg = (struct smtp_arg*) parser->user_data;

	arg->complete = 1;

	return 0;
}

void
smtp_parser_init (struct smtp_parser *smtp_parser, const struct smtp_cmd *smtp_cmd, size_t smtp_cmd_cnt)
{
	memset (smtp_parser, 0, sizeof (struct smtp_parser));

	smtp_parser->smtp_cmd = smtp_cmd;
	smtp_parser->smtp_cmd_cnt = smtp_cmd_cnt;
	smtp_parser->parser.word_delim = ' ';
	smtp_parser->parser.on_word = _parser_on_word_cb;
	smtp_parser->parser.on_eol = _parser_on_eol_cb;
}

size_t
smtp_parser_exec (struct smtp_parser *smtp_parser, int smtp_cmd_enabled, const char *buff, size_t len)
{
	size_t parsed_len, i;

	smtp_parser->parser.user_data = &(smtp_parser->arg);

	parsed_len = parser_exec (&(smtp_parser->parser), buff, len);

	if ( smtp_parser->arg.complete ){
		for ( i = 0; i < smtp_parser->smtp_cmd_cnt; i++ ){
			if ( (smtp_parser->smtp_cmd[i].type == smtp_parser->arg.type) && CMD_ENABLED (smtp_cmd_enabled, smtp_parser->arg.type) ){
				smtp_parser->smtp_cmd[i].cb (&(smtp_parser->arg), smtp_parser->user_data);
				smtp_parser->arg.type = 0;
				break;
			}
		}
	}

	return parsed_len;
}

int
smtp_strresponse (int code, char *buff, size_t len)
{
	const char *str;

	switch ( code ){
		case SMTP_READY:
			str = "Service ready";
			break;
		case SMTP_BYE:
			str = "Bye";
			break;
		case SMTP_MAILOK:
			str = "OK";
			break;
		case SMTP_STARTMAIL:
			str = "End data with '.'";
			break;
		case SMTP_ENOTAVAIL:
			str = "Service not available";
			break;
		case SMTP_ELOCAL:
			str = "Local error";
			break;
		case SMTP_ESTORAGE:
			str = "Not enough storage available";
			break;
		case SMTP_ESYNTAX:
			str = "Syntax error";
			break;
		case SMTP_EARGSYNTAX:
			str = "Syntax error in parameter";
			break;
		case SMTP_ECMDNIMPL:
			str = "Command not implemented";
			break;
		case SMTP_EBADSEQ:
			str = "Bad command sequence";
			break;
		case SMTP_EARGNIMPL:
			str = "Command parameter not implemented";
			break;
		default:
			return -1;
	}

	return snprintf (buff, len, "%d %s\r\n", code, str);
}

