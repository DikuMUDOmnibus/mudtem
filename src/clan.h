#ifndef _CLAN_H
#define _CLAN_H

#define CLAN_GOMA	0
#define	CLAN_PERKINS	1
#define	CLAN_CONSCRIPTO	2
#define	CLAN_CABO	3
#define CLAN_CONCEJAL	4
#define CLAN_TENIENTE	5
#define	CLAN_COMANDANTE	6
#define CLAN_CORONEL	7
#define CLAN_MINISTRO	8
#define CLAN_SENADOR	9
#define CLAN_DICTADOR	10

#define CLAN_FILENAME DATA_DIR "clanes"

#define PET_INDEF	0
#define PET_ACEPTADA	1
#define PET_RECHAZADA	2

typedef struct petition_type	PETITION_DATA;
typedef	struct clan_type	CLAN_TYPE;

struct clan_type {
	char	*name;
	char	*who_name;
	char	*god;
	int	hall;
	int	recall;
	int	death;
	int	pit;
	int	flags;
	int	pkills;		/* Pkills a favor del clan	*/
	int	pdeaths;	/* Pkills en contra del clan	*/
	int	mkills;		/* Mkills a favor del clan	*/
	int	mdeaths;	/* Mkills en contra del clan	*/
	int	illpkills;	/* Pkills ilegales		*/
};

struct petition_type
{
	PETITION_DATA *	next;
	char *		name;
	int		nivel;
	sh_int		oldclan;
	long		id;
	sh_int		clan;
	sh_int		status;
	time_t		when;
};

extern	const char *		lookup_clan_status(int);
extern	struct clan_type *	get_clan_table(int);
extern	int			get_clan_index(int);
extern	int			parse_clan_status(const char *);
extern	bool			is_independent(CHAR_DATA *);
extern	bool			check_peticiones(void);
extern	void			check_new_peticiones(CHAR_DATA *);
extern	void			borrar_clanes(void);
extern	void			cargar_clanes(void);
extern	void			grabar_clanes(void);
extern	void			grabar_peticiones(void);
extern	void			cargar_peticiones(void);

#define CEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

#define UPPER_MAX_CLAN 50

#endif // _CLAN_H
