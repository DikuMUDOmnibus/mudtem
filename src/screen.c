#include "include.h"
#include "screen.h"

char * goto3151 ( char *buf, int x, int y )
{
	sprintf( buf, "\eY%c%c", 32 + x, 32 + y );

	return buf;
}

char * gotovt100 ( char *buf, int x, int y )
{
	sprintf( buf, "\e[%d;%dH", x, y );

	return buf;
}

const struct term_type	term_table	[]	=
{
	{
		"ninguno",
		TIPO_NONE,
		"",
		"",
		"",
		"",
		NULL,
		"",
		"",
		"",
		"",
		"",
		"",
		0,
		""
	},

	{
		"ibm3151",
		TIPO_IBM3151,
		"\e4D",
		"\e4A",
		"\e4H",
		"\e4B",
		goto3151,
		"\eH",
		"\eH\eJ",
		"\e4@",
		"\e=",
		"\e=",
		"\e#:",
		0,
		""
	},

	{
		"vt100",
		TIPO_VT100,
		"\e[5m",
		"\e[7m",
		"\e[1m",
		"\e[4m",
		gotovt100,
		"\e[H",
		"\e[2J\e[H",
		"\e[0m",
		"\e7\e[0;%dr\e[%d;%1H",
		"\e8",
		"\e[r",
		0,
		""
	},

	{
		"vt100_color",
		TIPO_VT100 | TIPO_COLOR,
		"\e[5m",
		"\e[7m",
		"\e[1m",
		"\e[4m",
		gotovt100,
		"\e[H",
		"\e[2J\e[H",
		"\e[0m",
		"\e7\e[%d;%dH",
//		"\e7\e[0;%dr\e[%d;%1H",
		"\e8",
		"\e[r",
		30,
		"\e[%dm"
	},

	{
		NULL,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		NULL
	}
};
