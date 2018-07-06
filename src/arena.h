#if !defined(_ARENA_H)
#define _ARENA_H

#define ARENA_LIBRE	0
#define ARENA_OCUPADA	1

#define ARENA_NONE	0
#define ARENA_PERSONAL	A
#define ARENA_ETERNA	B

typedef	struct	arena_list_type		ALIST;

struct arena_type
{
	char *		name;
	const int	minjug;
	const int	maxjug;
	const int	desde;
	const int	hasta;
	sh_int		status;
	long		flags;
	ALIST *		jugadores;
	long		pozo;
};

struct arena_list_type
{
	ALIST *		next;
	CHAR_DATA *	ch;
	sh_int		frags;
	sh_int		team;
	int		apuestas;
	int		curhit;
	int		curmana;
	int		curmove;
};

void	check_arena( CHAR_DATA *, CHAR_DATA * );
ALIST *	extract_alist( ALIST *, ALIST * );
void	arena_quit_handler( CHAR_DATA *, int );
int	esta_inscrito( CHAR_DATA * );
int	arena_room_lookup( int );
#endif // _ARENA_H
