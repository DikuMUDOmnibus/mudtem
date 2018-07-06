typedef	char *	gotofunc( char *, int, int );

#define MAX_TERM	4
#define TERM_VT100	2

#define TIPO_NONE	0
#define	TIPO_IBM3151	A
#define TIPO_VT100	B
#define TIPO_COLOR	C

struct term_type
{
	char *		name;
	int		tipo;
	char *		blink;
	char *		reverse;
	char *		bold;
	char *		underline;
	gotofunc *	gotoxy;
	char *		home;
	char *		clear;
	char *		clearattr;
	char *		barrain;
	char *		barraout;
	char *		reset;
	int		colormod;
	char *		fgcolor;
};

extern	const	struct	term_type	term_table[];
