#ifndef _SMTP_PARSER_H
#define _SMTP_PARSER_H

#include <stddef.h>

#include "parser.h"

enum
{
	SMTP_C_HELO = 0x01,
	SMTP_C_EHLO = 0x02,
	SMTP_C_MAILFROM = 0x04,
	SMTP_C_RCPTTO = 0x08,
	SMTP_C_DATA = 0x10,
	SMTP_C_EOF = 0x20,
	SMTP_C_RSET = 0x40,
	SMTP_C_VRFY = 0x80,
	SMTP_C_NOOP = 0x100,
	SMTP_C_QUIT = 0x200,
	SMTP_C_UNKNOWN = 0x400
};

#define SMTP_C_ALL (SMTP_C_HELO | SMTP_C_EHLO | SMTP_C_MAILFROM \
								| SMTP_C_RCPTTO | SMTP_C_DATA \
								| SMTP_C_EOF | SMTP_C_RSET \
								| SMTP_C_VRFY | SMTP_C_NOOP \
								| SMTP_C_QUIT | SMTP_C_UNKNOWN)

enum
{
	SMTP_READY = 220,
	SMTP_BYE = 221,
	SMTP_MAILOK = 250,
	SMTP_STARTMAIL = 354,
	SMTP_ENOTAVAIL = 421,
	SMTP_ELOCAL = 451,
	SMTP_ESTORAGE = 452,
	SMTP_ESYNTAX = 500,
	SMTP_EARGSYNTAX = 501,
	SMTP_ECMDNIMPL = 502,
	SMTP_EBADSEQ = 503,
	SMTP_EARGNIMPL = 504
};

struct smtp_req_arg
{
	const char *val;
	size_t len;
};

// The structure is passed as a parameter to callback function.
// It holds parsed out arguments (fields) found after SMTP command.
struct smtp_arg
{
	int type;
	int complete;
	size_t argc;
#define SMTP_ARGS_MAX 4
	struct smtp_req_arg argv[SMTP_ARGS_MAX];
};

// The structure is available to programmer as a mean to create bond between particular SMTP command and his callback function.
struct smtp_cmd
{
	int type;
	int (*cb) (const struct smtp_arg *arg, void *user_data);
};

struct smtp_parser
{
	void *user_data;
	const struct smtp_cmd *smtp_cmd;
	size_t smtp_cmd_cnt;
	struct parser parser;
	struct smtp_arg arg;
};

extern void smtp_parser_init (struct smtp_parser *smtp_parser, const struct smtp_cmd *smtp_cmd, size_t smtp_cmd_cnt);

extern size_t smtp_parser_exec (struct smtp_parser *smtp_parser, int smtp_cmd_enabled, const char *buff, size_t len);

extern int smtp_strresponse (int code, char *buff, size_t len);

#endif

