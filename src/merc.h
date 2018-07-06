/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if !defined(_MERC_H)
#define _MERC_H

#include "sniptype.h"

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#define DECLARE_LOOKUP_FUN( fun )	int fun ( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_GAME_FUN( fun )		GAME_FUN  fun
#define DECLARE_LOOKUP_FUN( fun )	LOOKUP_F  fun
#define DECLARE_NEW_DO_FUN( fun )	NEW_DO_FUN	fun
#endif

#define SPELL(spell)		DECLARE_SPELL_FUN(spell);
#define SPELL_FUN_DEC(spell)	FRetVal spell (int sn, int level, Entity *caster, Entity * ent, int target)
#define COMMAND(cmd)		DECLARE_DO_FUN(cmd);
#define DO_FUN_DEC(x)		void x (CHAR_DATA *ch, char *argument)
#define NEW_DO_FUN_DEC(x)	FRetVal x (Entity *ent, char *argument)
#define DECLARE_SPELL_CB(x)	FRetVal x (Entity *ent)

/* system calls */
#if defined(WIN32)
#define pid_t long
#endif

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
#define unix
#endif

typedef short   int			sh_int;

#if defined(WIN32)
typedef char bool;
#else
typedef unsigned char			bool;
#endif

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct 	buf_type	 	BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef	struct	help_area_data		HELP_AREA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mem_data		MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	spelldesc		SpellDesc;
typedef	struct	repair_data		REPAIR_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  mprog_list		MPROG_LIST;
typedef struct  mprog_code		MPROG_CODE;
typedef	struct	_event			EVENT;		// *
typedef struct	bank_data		BANK_DATA;	// *
typedef	struct	plist_type		PLIST;		// *
typedef struct	fight_data		FIGHT_DATA;	// *
typedef struct	hunt_data		HUNT_DATA;	// *
typedef struct	quest_data		QUEST_DATA;	// *
typedef	struct	entity			Entity;		// *
typedef struct	limit_data		LIMIT_DATA;	// *
typedef	struct	level_data		LEVEL_DATA;	// *
typedef	struct	trap_data		TRAP_DATA;	// *
typedef struct	script_data		SCRIPT_DATA;	// *

/*
 * Function types.
 */
typedef	enum	{ fFAIL, fOK, fERROR, chdead, victdead, bothdead }	FRetVal;
typedef	void	DO_FUN		args( ( CHAR_DATA *ch, char *argument ) );
typedef FRetVal	NEW_DO_FUN	args( ( Entity *ent, char *argument ) );
typedef bool	SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef FRetVal SPELL_FUN	args( ( int sn, int level, Entity *caster, Entity * ent,
					int target ) );
typedef void	GAME_FUN	args( ( CHAR_DATA *ch, CHAR_DATA *croupier,
					char *argument ) );
typedef int	LOOKUP_F	args( ( const char * ) );
typedef FRetVal	SPELL_CB	args( ( Entity * ) );

/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4608
#define MAX_INPUT_LENGTH	  256
#define PAGELEN			   22

/* I am lazy :) */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_LANGUAGE               12 /* Number of languages */
#define MAX_SOCIALS		  256
/* #define MAX_GROUP		   34 */
#define MAX_IN_GROUP		   16
#define MAX_ALIAS		   10
#define MAX_CLASS		    6
#define MAX_IGNORE		    3

extern	int	MAX_SKILL;
extern	int	MAX_GROUP;
extern	int	maxrace;

#define MAX_DIR				8
#define NO_FLAG				-99	/* Must not be used in flags or stats. */
#define MAX_DAMAGE_MESSAGE		43
#define MAX_LEVEL			60
#define WAR_LEVEL			30
#define MAX_RACE_SKILL			5
#define LEVEL_HERO			(MAX_LEVEL - 9)
#define LEVEL_IMMORTAL			(MAX_LEVEL - 8)

#define PULSE_PER_SECOND	    6
#define PULSE_VIOLENCE		(  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		(  3 * PULSE_PER_SECOND)
#define PULSE_MUSIC		(  6 * PULSE_PER_SECOND)
#define PULSE_TICK		( 40 * PULSE_PER_SECOND)
#define PULSE_AREA		(100 * PULSE_PER_SECOND)
#define PULSE_TELEPORT		( 20 * PULSE_PER_SECOND) /* Froth */
#define PULSE_AUCTION		( 10 * PULSE_PER_SECOND) /* 10 seconds */
#define PULSE_RANGER		(  3 * PULSE_PER_SECOND)
#define PULSE_SAVEPLIST		( MINUTOS(30) * PULSE_PER_SECOND )
#define PULSE_SEGUNDOS		( MINUTOS(4) * PULSE_PER_SECOND )

#define IMPLEMENTOR		MAX_LEVEL		// 60
#define	CREATOR			(MAX_LEVEL - 1)		// 59
#define SUPREME			(MAX_LEVEL - 2)		// 58
#define DEITY			(MAX_LEVEL - 3)		// 57
#define GOD			(MAX_LEVEL - 4)		// 56
#define IMMORTAL		(MAX_LEVEL - 5)		// 55
#define DEMI			(MAX_LEVEL - 6)		// 54
#define ANGEL			(MAX_LEVEL - 7)		// 53
#define AVATAR			(MAX_LEVEL - 8)		// 52
#define HERO			LEVEL_HERO

#define PROMPT_ALL		"#B#4<#2%hhp #1%mm #6%vmv#4>#n "
#define PROMPT_FULL		"#B#4<#2%h/%Hhp #1%m/%Mm #6%v/%Vmv #5%xxp#4>#n "
#define PROMPT_FULL2		"#B#4<#2%h/%Hhp #1%m/%Mm #6%v/%Vmv #5%Xxp#4>#n "
#define PROMPT_WARRIOR		"#B#4<#2%h/%Hhp #6%v/%Vmv #5%xxp#4>#n "
#define PROMPT_OLC		"#B#4<#2%o #6%O#4>#n "

#define CLASS_MAGE		0
#define CLASS_CLERIC		1
#define CLASS_THIEF		2
#define CLASS_WARRIOR		3
#define CLASS_PSICIONIST	4
#define CLASS_PSI		4
#define CLASS_RANGER		5

extern	short			MAX_CLAN;
extern	int			RACE_VAMPIRE;
extern	int			RACE_HUMAN;
extern	int			RACE_DRAGON;
extern	int			RACE_TROLL;

#include "clan.h"
#include "mudconf.h"

/*
 * Site ban structure.
 */
#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL			D	
#define BAN_PERMIT		E
#define BAN_PERMANENT		F

struct	ban_data
{
    BAN_DATA *	next;
    bool	valid;
    sh_int	ban_flags;
    sh_int	level;
    char *	name;
};

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    int		size;   /* size in k */
    char *      string; /* buffer's string */
};

struct skhash
{
	struct skhash *	next;
	int		sn;
};

struct	script_data
{
	SCRIPT_DATA *	next;
	int		timer;
	char **		comandos;
	int		posicion;
	int		vnum;
};

struct plist_type
{
	struct plist_type *	next;
	char *			name;
	char *			host;
	long			id;
	int			nivel;
	sh_int			clan;
	sh_int			clan_status;
	sh_int			sex;
	sh_int			class;
	sh_int			race;
	time_t			lastlog;
#if defined(CRC)
	WORD			crc;
#endif
};

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX			 7
#define CON_GET_NEW_CLASS		 8
#define CON_GET_ALIGNMENT		 9
#define CON_PICK_WEAPON			10
#define CON_READ_IMOTD			11
#define CON_READ_MOTD			12
#define CON_BREAK_CONNECT		13
#define CON_BEGIN_REMORT		14
#define CON_GET_STATS			15
#define CON_COPYOVER_RECOVER		16
#define CON_READ_NMOTD			17
#define CON_GET_NEWANSI			18
#define CON_CONFIRMAR_RAZA		19
#define CON_CONFIRMAR_CLASE		20
#define CON_GET_MULTICLASE		21
#define CON_GET_SEGUNDA_CLASE		22
#define CON_FTP_COMMAND			23
#define CON_FTP_DATA			24
#define CON_FTP_AUTH			25

typedef enum { FTP_NORMAL, FTP_PUSH, FTP_PUSH_WAIT } ftp_mode;

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    EVENT *		events;
    bool		valid;
    bool		fcommand;
    sh_int		term;
    char *		host;
#if defined(WIN32)
	SOCKET		descriptor;
#else
	sh_int		descriptor;
#endif
    sh_int		connected;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    char *		showstr_head;
    char *		showstr_point;
    int			ifd;
    pid_t		ipid;
    char *		ident;
    int			port;
    int			ip;
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
    sh_int		editor;		/* OLC */
    sh_int		pagina;		/* OLC */
    char *		screenmap;
    char *		oldscreenmap;
    char *		username;
    struct
    {
        char *		filename;   /* Filename being written to */
        char *		data;       /* Data being written 	 */
        short int 	lines_left; /* Lines left 		 */
        short int 	lines_sent; /* Lines sent so far 	 */
        ftp_mode 	mode;       /* FTP_xxx 		 	 */
    } ftp;
};

struct	trap_data
{
	TRAP_DATA *	next;
	sh_int		trap_eff;
	sh_int		trap_dam;
	sh_int		trap_charge;
};

struct	bank_data
{
	BANK_DATA *	next;
	long		valor;
	sh_int		tipo;
	time_t		when;
	time_t		start;
	float		interes;
};
	
#define	BANK_DEPOSITO	1
#define BANK_PRESTAMO	2
#define BANK_ACCIONES	3

/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * next_area;
    sh_int	level;
    char *	keyword;
    char *	text;
};

struct help_area_data
{
	HELP_AREA *	next;
	HELP_DATA *	first;
	HELP_DATA *	last;
	AREA_DATA *	area;
	char *		filename;
	bool		changed;
};

/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	profit_buy_actual;
    sh_int	profit_sell_actual;
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

/*
 * Repair types;
 */
struct	repair_data
{
    REPAIR_DATA *next;			/* Proxima reparadora en lista	*/
    sh_int	keeper;			/* Vnum del mob reparador	*/
    sh_int	repair_type [MAX_TRADE];/* Items que se repararan	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

/*
 * Per-class stuff.
 */
#define MAX_GUILD 	2
#define MAX_STATS 	5
#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4

struct	class_type
{
    char *	name;			/* the full name of the class */
    char 	who_name	[4];	/* Three-letter name for 'who'	*/
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	weapon;			/* First weapon			*/
    sh_int	guild[MAX_GUILD];	/* Vnum of guild rooms		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    char *	base_group;		/* base skills gained		*/
    char *	default_group;		/* default skills gained	*/
    bool	remort_class;		/* is a remort class		*/
    bool	multiclase;
    sh_int	max_num_clases;
    int		noalign;		/* alineaciones no permitidas	*/
    bool	noclase[MAX_CLASS];
};

struct item_type
{
    int		type;
    char *	name;
    long	min_wear_flags;
};

struct weapon_type
{
    char *	name;
    char *	prefix;
    char *	nombre;
    sh_int	vnum;
    sh_int	type;
    sh_int	*gsn;
};

struct wiznet_type
{
    char *	name;
    long 	flag;
    int		level;
};

struct attack_type
{
    char *	name;			/* name */
    char *	noun;			/* message */
    int   	damage;			/* damage class */
};

struct race_type
{
    char *	name;			/* call name of the race */
    bool	pc_race;		/* can be chosen by pcs .. 1 */
    bool	seleccionable;		/* se puede elegir .. 2*/
    sh_int	dam_type;		/* ..4 */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	aff2;			/* aff2 */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long        res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
    char *	hate;                   /* razas odiadas */
    char *	msg_salir;		/* mensaje al salir de un cuarto */
    char *	macho;
    char *	hembra;
    char *	who_name;
    sh_int	points;			/* puntos de creacion */
    sh_int	race_lang;              /* race language */
    sh_int	size;			/* tama~no */
    sh_int	race_id;
    sh_int	class_mult[MAX_CLASS];
    char *	skills[MAX_RACE_SKILL];
    sh_int	stats[MAX_STATS];
    sh_int	max_stats[MAX_STATS];
    char *	no_clase;
    bool	remort_race;		/* is a remort_race */
    int		noalign;
    void *	odio;
    bool	noclase[MAX_CLASS];
};

struct spec_type
{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};

struct game_type
{
    char * 	name;			/* special function name */
    GAME_FUN *	function;		/* the function */
};

/*
 * Data structure for notes.
 */
#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_PENALTY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4

struct	note_data
{
    NOTE_DATA *	next;
    bool 	valid;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
};

/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
    int			caster_id;
    bool		valid;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN		4
#define TO_WEAPON	5
#define TO_AFFECTS2	6
#define TO_ROOM_AFF	7
#define TO_PARTS	8

/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MAX_VNUM		   32767

#define MOB_VNUM_FIDO		   3062
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   3404
#define MOB_VNUM_ZOMBIE		   1
#define MOB_VNUM_STALKER	   2
#define MOB_VNUM_RATA		   31000

#define MOB_VNUM_PATROLMAN	   2106
#define GROUP_VNUM_TROLLS	   2100
#define GROUP_VNUM_OGRES	   2101


/* RT ASCII conversions -- used so we can have letters in this file */
#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
#define ACT_PSI			(D)		/* Es Psionico		*/
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_UNDEAD		(O)
#define ACT_GROUP		(P)
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)
#define ACT_PROTOTIPO		(X)		/* Prototipo OLC	*/
#define ACT_INDOORS		(Y)
#define ACT_TEACHER		(Z)		/* Es Profesor Idiomas	*/
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_IS_CHANGER		(dd)
#define ACT_BANKER		(ee)		/* Es banquero		*/

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH		(O)
#define ASSIST_ALL       	(P)
#define ASSIST_ALIGN	        (Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)
#define OFF_CIRCLE		(V)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
 
/* RES bits for mobs */
#define RES_SUMMON		(A)
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT		(S)
#define RES_SOUND		(T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
 
/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_CHARM		(B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
 
/* tipos de comida */
#define FOOD_POISON		(A)
#define FOOD_PLAGUE		(B)
#define FOOD_MAGIC		(C)

/* detectables */
#define DETECTED_CURSE		(A)
#define DETECTED_EVIL		(B)
#define DETECTED_SHARP		(C)

/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)	
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)
#define PART_AGALLAS		(Z)

/* materiales */
#define MAT_INDEF		0
#define MAT_METAL		1
#define MAT_MADERA		2
#define MAT_ORO			3
#define MAT_PLATA		4
#define MAT_COBRE		5
#define MAT_HIERRO		6
#define MAT_ACERO		7
#define MAT_CEMENTO		8
#define MAT_MARMOL		9
#define MAT_MITHRIL		10
#define MAT_VIDRIO		11
#define MAT_PAPEL		12
#define MAT_CRISTAL		13
#define MAT_COMIDA		14
#define MAT_MARFIL		15
#define MAT_CARNE		16
#define MAT_CUERO		17
#define MAT_BRONCE		18
#define MAT_HIELO		19
#define MAT_PIEDRA		20
#define MAT_HUESO		21
#define MAT_TELA		22
#define MAT_PLASTICO		23
#define MAT_SEDA		24
#define MAT_LANA		25
#define MAT_TERCIOPELO		26
#define MAT_AGUA		27
#define MAT_LIQUIDO		28
#define MAT_EBANO		29
#define MAT_ADAMANTIO		30
#define MAT_DIAMANTE		31
#define MAT_ALUMINIO		32
#define MAT_GOMA		33
#define MAT_TITANIO		34
#define MAT_GREDA		35
#define MAT_CUARZO		36
#define MAT_GEMA		37
#define MAT_PERLA		38
#define MAT_PLATINO		39
#define MAT_URANIO		40
#define MAT_PLUTONIO		41
#define MAT_TIERRA		42
#define MAT_TABACO		43
#define MAT_PAN			44

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_UNUSED_FLAG		(L)	/* unused */
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW		(dd)
#define AFF_POLYMORPH		(ee)

/*
 * Bits for 'affected2_by'.
 * Used in #MOBILES.
 */
#define AFF_VAMP_BITE		(A)
#define AFF_FLAMING_SHIELD	(B)
#define AFF_GHOUL		(C)
#define AFF_MUTE		(D)
#define AFF_AMNESIA		(E)
#define AFF_HOLD		(G)
#define AFF_ESTUPIDEZ		(I)
#define AFF_DOLOR_GUATA		(J)
#define AFF_RESPIRAR_AGUA	(K)

/*
 * Poker.
 */
#define POKER_NADA    		(0)
#define POKER_INICIO  		(1)
#define POKER_JUGANDO 		(2)
#define POKER_APUESTAS		(3)
#define CAMBIAR_LISTO		(G)

/*
 * Clanes.
 */
#define CLAN_INDEP		(A)
#define CLAN_NOGOOD		(B)
#define CLAN_NOEVIL		(C)
#define CLAN_NONEUTRAL		(D)

/* All new trap stuff */
#define TRAP_DAM_SLEEP      0
#define TRAP_DAM_TELEPORT   1
#define TRAP_DAM_FIRE       2
#define TRAP_DAM_COLD       3
#define TRAP_DAM_ACID       4
#define TRAP_DAM_ENERGY     5
#define TRAP_DAM_BLUNT      6
#define TRAP_DAM_PIERCE     7
#define TRAP_DAM_SLASH      8

#define TRAP_EFF_MOVE       1 /* trigger on movement */
#define TRAP_EFF_OBJECT     2 /* trigger on get or put */
#define TRAP_EFF_ROOM       4 /* affect all in room */
#define TRAP_EFF_NORTH      8 /* movement in this direction */
#define TRAP_EFF_EAST       16
#define TRAP_EFF_SOUTH      32
#define TRAP_EFF_WEST       64
#define TRAP_EFF_UP         128
#define TRAP_EFF_DOWN       256
#define TRAP_EFF_OPEN       512 /* trigger on open */

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Command types.
 */
#define TYP_NUL 0
#define TYP_CMM	10
#define TYP_CBT	2
#define TYP_ESP 3
#define TYP_GRP 4
#define TYP_OBJ 5
#define TYP_INF 6
#define TYP_OTH 7
#define TYP_MVT 8
#define TYP_CNF 9
#define TYP_LNG 11
#define TYP_PLR 12
#define TYP_OLC 13

/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char *	name;
    DO_FUN *	do_fun;
    sh_int	position;
    sh_int	level;
    int		log;
    int		show;
};

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL			0
#define SEX_MALE			1
#define SEX_FEMALE			2

/*
 * Alineacion
 */
#define ALIGN_NEUTRAL			0
#define ALIGN_GOOD			1
#define ALIGN_EVIL			2

#define NOALIGN_NEUTRAL			A
#define NOALIGN_GOOD			B
#define NOALIGN_EVIL			C

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE	1
#define OBJ_VNUM_GOLD_ONE	2
#define OBJ_VNUM_GOLD_SOME	3
#define OBJ_VNUM_SILVER_SOME	4
#define OBJ_VNUM_COINS		5

#define OBJ_VNUM_CORPSE_NPC	10
#define OBJ_VNUM_CORPSE_PC	11
#define OBJ_VNUM_SEVERED_HEAD	12
#define OBJ_VNUM_TORN_HEART	13
#define OBJ_VNUM_SLICED_ARM	14
#define OBJ_VNUM_SLICED_LEG	15
#define OBJ_VNUM_GUTS		16
#define OBJ_VNUM_BRAINS		17

#define OBJ_VNUM_MUSHROOM	20
#define OBJ_VNUM_LIGHT_BALL	21
#define OBJ_VNUM_SPRING		22
#define OBJ_VNUM_DISC		23
#define OBJ_VNUM_PORTAL		25
#define OBJ_VNUM_DUMMY		30

#define OBJ_VNUM_ROSE		1001

#define OBJ_VNUM_PIT		3010

#define OBJ_VNUM_SCHOOL_MACE	3700
#define OBJ_VNUM_SCHOOL_DAGGER	3701
#define OBJ_VNUM_SCHOOL_SWORD	3702
#define OBJ_VNUM_SCHOOL_SPEAR	3717
#define OBJ_VNUM_SCHOOL_STAFF	3718
#define OBJ_VNUM_SCHOOL_AXE	3719
#define OBJ_VNUM_SCHOOL_FLAIL	3720
#define OBJ_VNUM_SCHOOL_WHIP	3721
#define OBJ_VNUM_SCHOOL_POLEARM 3722

#define OBJ_VNUM_SCHOOL_VEST	3703
#define OBJ_VNUM_SCHOOL_SHIELD	3704
#define OBJ_VNUM_SCHOOL_BANNER  3716
#define OBJ_VNUM_MAP		3162

#define OBJ_VNUM_WHISTLE	2116

#define OBJ_VNUM_MITHRIL	1312
#define OBJ_VNUM_STAKE		3035
#define OBJ_VNUM_LENTES		3007
#define OBJ_VNUM_ESPADA_PERS	3008
#define OBJ_VNUM_PLASTA		  18
#define OBJ_VNUM_CHARCO		  19
#define OBJ_VNUM_BISTEC		   9
#define OBJ_VNUM_SCROLL_RECALL	3042

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		     11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_PROTECT		     27
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_WARP_STONE		     30
#define ITEM_ROOM_KEY		     31
#define ITEM_GEM		     32
#define ITEM_JEWELRY		     33
#define ITEM_JUKEBOX		     34
#define ITEM_PEDERNAL		     35
#define ITEM_FUMABLE		     36


/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_FLAMING		(R)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_ATM		(X)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_HIDDEN		(aa)
#define ITEM_BLADE_THIRST	(bb)
#define ITEM_VAMPIRE_BANE	(cc)
#define ITEM_PROTOTIPO		(dd)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_WEAR_LENTES	(R)
#define ITEM_WEAR_OREJAS	(S)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7	
#define WEAPON_POLEARM		8
#define MAX_WEAPON_CLASS	9

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SPELL_AFFECT	     25
#define APPLY_RACE		     26

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16
#define CONT_CUERPO_POISON	     64


/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_MORGUE           3036
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   3001
#define ROOM_VNUM_ALTAR		   3054
#define ROOM_VNUM_SCHOOL	   3700
#define ROOM_VNUM_BALANCE	   4500
#define ROOM_VNUM_CIRCLE	   4400
#define ROOM_VNUM_DEMISE	   4201
#define ROOM_VNUM_HONOR		   4300
#define ROOM_BANK		   3054
#define ROOM_VNUM_ARENA_MIN	   3360
#define ROOM_VNUM_ARENA_MAX	   3368
#define ROOM_VNUM_ALOSER	   3038
#define ROOM_VNUM_AWINNER	   3039

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_CONE_OF_SILENCE	(B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)

#define ROOM_FLAMING		(E)
#define ROOM_GRABADO		(F)
#define ROOM_NOMAGIC		(G)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_TELEPORT		(U)
#define ROOM_ARENA		(X)
#define ROOM_PROTOTIPO          (Y)
#define ROOM_SANTUARIO		(Z)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5
#define DIR_INSIDE		      6
#define DIR_OUTSIDE		      7


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE			0
#define SECT_CITY			1
#define SECT_FIELD			2
#define SECT_FOREST			3
#define SECT_HILLS			4
#define SECT_MOUNTAIN			5
#define SECT_WATER_SWIM			6
#define SECT_WATER_NOSWIM		7
#define SECT_UNUSED			8
#define SECT_AIR			9
#define SECT_DESERT			10
#define SECT_HOYO			11
#define SECT_UNDERWATER			12
#define SECT_MAX			13

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_FLOAT		     18
#define WEAR_SECONDARY               19
#define WEAR_LENTES		     20
#define WEAR_OREJAS		     21
#define MAX_WEAR                     22



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3


/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/

#define PLR_NOAUCTION		(B)		/* no puede rematar	*/

/* RT auto flags */
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_NOSAVE		(I)

#define PLR_MUDFTP		(J)

/* RT personal flags */
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)

#define PLR_MAL_ALIGN		(S)

/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_NOPK		(V)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_THIEF		(Z)
#define PLR_KILLER		(aa)
#define PLR_REMORT		(bb)

/*
 * Color codes
 */
#define ANSI_attrib(buf,attr)		sprintf(buf,"\033[%dm",(int)(attr))
#define ANSI_fgcolor(buf,fc)		sprintf(buf,"\033[%dm",(int)(fc)+30)
#define ANSI_attrcolor(buf,fc,bc,attr)	sprintf(buf,"\033[%d;%d;%dm",(int)(fc)+30,(int)(bc)+40,(int)(attr))
#define VT_SAVECURSOR			"\0337"  /* Save cursor and attrib */
#define VT_RESTORECURSOR		"\0338"  /* Restore cursor pos and attribs */
#define VT_SETWIN_CLEAR			"\033[r" /* Clear scrollable window size */
#define VT_CLEAR_SCREEN			"\033[2J" /* Clear screen */
#define VT_CLEAR_LINE			"\033[2K" /* Clear this whole line */
#define VT_RESET_TERMINAL		"\033c"
#define VT_INITSEQ    			"\033[1;24r"
#define VT_CURSPOS    			"\033[%d;%dH"
#define VT_CURSRIG    			"\033[%dC"
#define VT_CURSLEF    			"\033[%dD"
#define VT_HOMECLR    			"\033[2J\033[0;0H"
#define VT_CTEOTCR    			"\033[K"
#define VT_CLENSEQ    			"\033[r\033[2J"
#define VT_INDUPSC    			"\033M"
#define VT_INDDOSC    			"\033D"
#define VT_SETSCRL    			"\033[%d;24r"
#define VT_INVERTT    			"\033[0;1;7m"
#define VT_BOLDTEX    			"\033[0;1m"
#define VT_NORMALT    			"\033[0m"
#define VT_MARGSET    			"\033[%d;%dr"
#define VT_CURSAVE    			"\0337"
#define VT_CURREST    			"\0338"

enum {ANSI_BLACK=0,    ANSI_RED=1,      ANSI_GREEN=2,
      ANSI_YELLOW=3,   ANSI_BROWN=3,    ANSI_BLUE=4,
      ANSI_MAGENTA=5,  ANSI_CYAN=6,     ANSI_WHITE=7};

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN		(H)
#define COMM_NOQUOTE		(I)
#define COMM_SHOUTSOFF		(J)

/* display flags */
#define COMM_OLC_PAUSA		(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)
#define COMM_STAT		(S)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W) 
#define COMM_OLCX		(X)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)
#define COMM_NOINFO		(aa)
#define COMM_CONDICION		(bb)
#define COMM_NOSPAM		(dd)

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)
#define WIZ_BUGS		(U)
#define WIZ_XP			(V)
#define WIZ_TIMESTAMP		(W)
#define WIZ_PLOG		(X)

/* Tipos de shutdown */
#define SHUTDOWN_NONE		0
#define SHUTDOWN_REBOOT		1
#define SHUTDOWN_COPYOVER	2
#define SHUTDOWN_NORMAL		3

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    GAME_FUN *		game_fun;
    SHOP_DATA *		pShop;
    REPAIR_DATA *	pRepair;
    MPROG_LIST *        mprogs;
    AREA_DATA *		area;		/* OLC */
    SCRIPT_DATA *	script;
    MEM_DATA *		memoria;
    sh_int		vnum;
    sh_int		group;
    bool		new_format;	/* 1.. */
    bool		norecalc;	/* 2.. */
    sh_int		count;		/* ..4 */
    sh_int		killed;
    sh_int		reset_num;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		act;
    long		affected_by;
    long		affected2_by;
    sh_int		alignment;
    sh_int		level;
    sh_int		hitroll;
    sh_int		hit[3];
    sh_int		mana[3];
    sh_int		damage[3];
    sh_int		ac[4];
    sh_int 		dam_type;
    sh_int		size;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		wealth;
    long		form;
    long		parts;
    char *		material;
    long		mprog_flags;
    sh_int		clan;
    sh_int		clase;
};

struct prob_data
{
	struct prob_data *	next;
	char *			comando;
};

/* memory settings */
#define MEM_CUSTOMER	A
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D
#define MEM_HUNTING	E
#define MEM_VAMPIRE	F
#define MEM_QUEST	G
#define MEM_ROOM	H
#define MEM_QUEST_COMPLETO I

/* memory for mobs */
struct mem_data
{
    MEM_DATA 	*next;
    bool	valid;
    int		id; 	
    int 	reaction;
    time_t 	when;
};

struct fight_data
{
	FIGHT_DATA *	next;
	CHAR_DATA *	atacante;
	long		id;
	int		dam;
	time_t		when;
};

struct hunt_data
{
	HUNT_DATA *		next;
	CHAR_DATA *		victima;
	long			id;
	ROOM_INDEX_DATA *	room;
	bool			especial;
};

#define ENT_CH		0
#define ENT_OBJ		1
#define ENT_STRING	2
#define ENT_INT		3
#define ENT_ROOM	4

struct entity
{
	Entity *		next;
	bool			valid;
	sh_int			tipo;
	ROOM_INDEX_DATA *	whereis;
	union
	{
		CHAR_DATA *		ch;
		OBJ_DATA *		obj;
		char *			string;
		int			entero;
		ROOM_INDEX_DATA *	room;
	} u;
};

struct level_data
{
	LEVEL_DATA * next;
	sh_int	clase;
	sh_int	nivel;
};

/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		pet;
    Entity *		mprog_target;
    Entity *		ent;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    GAME_FUN *		game_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;	/* cuarto de inicio */
    ROOM_INDEX_DATA *	ultimo_cuarto;	/* cuarto en el que estaba */
    AREA_DATA *		zone;
    PC_DATA *		pcdata;
    GEN_DATA *		gen_data;
    HUNT_DATA *		hunt_data;
    EVENT *		events;
    FIGHT_DATA *	fdata;
    LEVEL_DATA *	level_data;
    char *		name;
    long		id;
    sh_int		version;		/* 2.. */
    sh_int		group;			/* ..4 */
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		clan;
    sh_int		sex;
    sh_int		race;
    sh_int		random;
    int			played;
    time_t		logon;
    sh_int		timer;
    sh_int		wait;
    sh_int		daze;
    sh_int		hit;
    sh_int		max_hit;
    sh_int		mana;
    sh_int		max_mana;
    sh_int		move;
    sh_int		max_move;
    sh_int		size;
    long		gold;
    long		silver;
    int			exp;
    long		act;
    long		comm;   /* RT added to pad the vector */
    long		wiznet; /* wiz stuff */
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    sh_int		incog_level;
    long		affected_by;
    long		affected2_by;
    sh_int		position;
    sh_int		practice;
    sh_int		train;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		wimpy;
    sh_int		armor[4];
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    char *		material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		fall;
    sh_int		true_race;
    sh_int		luck;
    sh_int		aire;
    sh_int		trust;
    bool		valid;
};

struct quest_data
{
	QUEST_DATA *	next;
	int		qpoints;
	sh_int		timer;
	sh_int		type;
	long		id;
	sh_int		estado;
};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    BUFFER * 		buffer;
    OBJ_DATA *		corpse;
    BANK_DATA *		bank;
    PLIST *		pdata;
    CHAR_DATA *		retando;
    CHAR_DATA *		apostando;
    QUEST_DATA *	quest;
    struct prob_data *	prohibido;
    bool		valid;		/* 1.. */
    bool		confirm_remort; /* 2.. */
    sh_int		quaff;		/* ..4 */
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		prompt;
    char *		prefix;
    time_t              last_note;
    time_t              last_idea;
    time_t              last_penalty;
    time_t              last_news;
    time_t              last_changes;
    int			last_level;
    sh_int		condition	[4];
    sh_int *		learned;
    bool *		group_known;
    sh_int		points;
    sh_int		clan_status;
    sh_int		max_clan_status;
    sh_int		min_clan_status;
    sh_int		color;
    char *		alias[MAX_ALIAS];
    char * 		alias_sub[MAX_ALIAS];
    int			incarnations;
    int			muertes;
    int                 language        [ MAX_LANGUAGE ];
    int                 speaking;
    int                 learn;
    int 		security;	/* OLC */ /* Builder security */
    char *              who_text;
    char *		mensaje;
    char *		ignore[MAX_IGNORE];
    int			lines;  /* for the pager */
    int			apuesta;
    int			victorias;
    int			derrotas;
    sh_int		perm_hit;
    sh_int		perm_mana;
    sh_int		perm_move;
    sh_int		true_sex;
    sh_int		true_align;
    bool              	confirm_delete;
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    int		points_chosen;
    bool	valid;
    bool	*skill_chosen;
    bool	*group_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};

struct limit_data
{
	LIMIT_DATA *	next;
	long		carrier_id;
	sh_int		id;
};

/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area;		/* OLC */
    TRAP_DATA *		trap;
    MPROG_LIST *        mprogs;
    LIMIT_DATA *	limit;
    long		mprog_flags;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		material_string;
    int			extra_flags;
    int			wear_flags;
    int			cost;
    int			value[5];
    sh_int		vnum;
    sh_int		reset_num;
    sh_int		item_type;
    sh_int		level;
    sh_int 		condition;
    sh_int		count;
    sh_int		weight;
    sh_int		clan;
    sh_int		max_count;
    sh_int		material;
    bool		new_format;
};

/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    OBJ_DATA *		on;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    EVENT *		events;
    TRAP_DATA *		trap;
    bool		valid;		/* 1.. */
    bool		enchanted;	/* 2.. */
    sh_int		item_type;	/* ..4 */
    char *	        owner;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		material_string;
    int			extra_flags;
    int			wear_flags;
    int			cost;
    sh_int		wear_loc;
    sh_int		weight;
    sh_int		level;
    sh_int 		condition;
    sh_int		timer;
    sh_int		clan;
    int			value	[5];
    int			detected;
    sh_int		carry_timer;
    sh_int		id;
    sh_int		humedad;
    sh_int		material;
};

/*
 * Exit data.
 */
struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	sh_int			vnum;
    } u1;
    char *		keyword;
    char *		description;
    EXIT_DATA *		next;		/* OLC */
    sh_int		direccion;
    sh_int		exit_info;
    sh_int		key;
    sh_int		rs_flags;	/* OLC */
    sh_int		orig_door;	/* OLC */
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
    sh_int		arg4;
    char		command;
};

/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    HELP_AREA *		helps;
    char *		file_name;
    char *		name;
    char *		credits;
    char *		clan;
    sh_int		age;
    sh_int		nplayer;
    sh_int		low_range;
    sh_int		high_range;
    sh_int 		min_vnum;
    sh_int		max_vnum;
    char *		builders;	/* OLC */ /* Listing of */
    int			area_flags;	/* OLC */
    sh_int		vnum;		/* OLC */ /* Area vnum  */
    sh_int		security;	/* OLC */ /* Value 1-9  */
    sh_int		version;
    bool		empty;
};

/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exits;
    RESET_DATA *	reset_first;	/* OLC */
    RESET_DATA *	reset_last;	/* OLC */
    EVENT *		events;
    AFFECT_DATA *	affected;
    MPROG_LIST *        mprogs;
    long		mprog_flags;
    char *		name;
    char *		description;
    char *		owner;
    int			room_flags;
    sh_int		vnum;
    sh_int		light;
    sh_int		sector_type;
    sh_int		heal_rate;
    sh_int 		mana_rate;
    sh_int		clan;
    sh_int		tele_dest;
    sh_int		reset_num;
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3

/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    char *	nombre;			/* Nombre traducido		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/	
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    SPELL_CB *	spell_callback;		/* callback para el spell	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for objects	*/
    char *	msg_room;		/* Mensaje para los cuartos	*/
    int		flags;			/* Flags especiales para skills */
#if defined(MUD_SLANG)
    char *	slangscript;		/* SLang Script			*/
#endif
};

struct spelldesc
{
	sh_int sn;
	sh_int nivel;
	sh_int clase;
};

#define	SKILL_CLERIC	A
#define SKILL_MAGE	B
#define SKILL_WARRIOR	C
#define SKILL_THIEF	D
#define SKILL_PSI	E
#define SKILL_RANGER	F
#define SKILL_NOSOUND	G // caster no necesita producir sonido alguno

struct  group_type
{
    char *	name;
    char *	spells[MAX_IN_GROUP];
    sh_int	rating[MAX_CLASS];
};

/*
 * MOBprog definitions
 */                   
#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_SURR	(P)
#define TRIG_NSPEECH	(Q)
#define TRIG_ENTRYALL	(R)
#define TRIG_TIME	(S)
#define TRIG_ONESHOT	(T)

/*
 * Triggers para objetos
 */
#define TRIG_GET	(A)
#define TRIG_PUT	(B)
#define TRIG_SAC	(C)
#define TRIG_WEAR	(D)
#define TRIG_REMOVE	(E)
#define TRIG_HACER	(F)
#define OTRIG_DELAY	(G)

/*
 * Triggers para cuartos
 */
#define RTRIG_ENTER	(A)
#define RTRIG_COMM	(B)
#define RTRIG_EXCOMM	(C)
#define RTRIG_DELAY	(D)
#define RTRIG_SPEECH	(E)

struct mprog_list
{
    int			trig_type;
    char *		trig_phrase;
    sh_int		vnum;
    char *  		code;
    MPROG_LIST * 	next;
    bool                valid;
};

struct mprog_code
{
    sh_int		vnum;
    bool		changed;
    char *		descripcion;
    char *		code;
    MPROG_CODE *	next;
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
#define GSN(x)	extern sh_int x;
#include "gsn.h"
#undef GSN

/*
 * Utility macros.
 */
#define IS_SWITCHED( ch )       ( (ch)->desc && (ch)->desc->original )
#define IS_COLOR(ch)		(!IS_NPC(ch) && IS_SET((ch)->comm, COMM_COLOR))
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define replace_string( pstr, nstr ) \
          { free_string( (pstr) ); pstr=str_dup( (nstr) ); }
#define IS_NULLSTR(str)	((str)==NULL || (str)[0]=='\0')
#define CHECKNULLSTR(str) ( (str) == NULL ? "" : (str) )
#define CH(d)		((d)->original ? (d)->original : (d)->character )
#define CHANCE(temp)	(number_percent() <= (temp))
#define MINUTOS(temp)	((temp) * 60)
#define HORAS(temp)	(MINUTOS(60) * (temp))
#define DIAS(blah)	((blah) * HORAS(24))
#define ENTRE(x,y,z)	(((x) < (y)) && ((y) < (z)))
#define ENTRE_I(x,y,z)	(((x) <= (y)) && ((y) <= (z)))
#define IS_CLOSED(exit) (IS_SET(exit->exit_info,EX_ISDOOR) && IS_SET(exit->exit_info,EX_CLOSED))
#define NOMBRE_SKILL(sn) (IS_NULLSTR(skill_table[sn].nombre) ? skill_table[sn].name : skill_table[sn].nombre)

/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)	(IS_SET((ch)->affected2_by, (sn)))

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_T_GOOD(ch)		(ch->pcdata->true_align == ALIGN_GOOD)
#define IS_T_NEUTRAL(ch)	(ch->pcdata->true_align == ALIGN_NEUTRAL)
#define IS_T_EVIL(ch)		(ch->pcdata->true_align == ALIGN_EVIL)
#define IS_SAME_ALIGN(ch,gch)	( ( ch == gch ) || (IS_GOOD(ch) && IS_GOOD(gch)) || (IS_NEUTRAL(ch) && IS_NEUTRAL(gch)) || (IS_EVIL(ch) && IS_EVIL(gch)) )
#define IS_CLAN_NOEVIL(clan)	(IS_SET(get_clan_table(clan)->flags, CLAN_NOEVIL))
#define IS_CLAN_NOGOOD(clan)	(IS_SET(get_clan_table(clan)->flags, CLAN_NOGOOD))
#define IS_CLAN_NONEUTRAL(clan)	(IS_SET(get_clan_table(clan)->flags, CLAN_NONEUTRAL))
#define ES_ALIGN_OPUESTA(ch,tch)	( (IS_GOOD(ch) && IS_EVIL(tch)) || (IS_EVIL(ch) && IS_GOOD(tch)) )
#define CLAN_GOD(ch)		( is_clan(ch) ? get_clan_table((ch)->clan)->god : "Mota" )
#define CLAN_STATUS(ch)		( IS_NPC(ch) ? CLAN_GOMA : (ch)->pcdata->clan_status )
#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)		((ch)->armor[type]			    \
		        + ( IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  
#define GET_HITROLL(ch)	\
		(((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit))
#define GET_DAMROLL(ch) \
		(((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam))

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver / 50 + \
						      (ch)->gold / 10)
#define gold_weight(gold)	((gold) / 10)
#define silver_weight(silver)	((silver) / 50)
#define HAS_TRIGGER(ch,trig)	(IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_ROOM_TRIGGER(room,trig)	(IS_SET((room)->mprog_flags,(trig)))
#define IS_MUTILATED(ch)	(IS_NPC(ch) ? (ch->parts != ch->pIndexData->parts) : \
				(ch->parts != race_table[ch->race].parts) )
#define U_LT(msg)	(msg[strlen(msg) - 1])
#define TIPO(msg)	(U_LT(msg) == 'a' || U_LT(msg) == 'n' ? "la" : "el")
#define TIPO2(msg)	(U_LT(msg) == 'a' || U_LT(msg) == 'n' ? "a" : "")
#define MORGUE(ch)	(ch->clan ? get_clan_table(ch->clan)->death : ROOM_VNUM_ALTAR)
#define EN_ARENA(ch)	(IS_SET(ch->in_room->room_flags, ROOM_ARENA))
#define ES_SPELL(dt)	((dt)>0 && (dt)<MAX_SKILL && skill_table[(dt)].spell_fun != spell_null && skill_table[(dt)].spell_fun != NULL)
#define DINERO(ch)	((ch)->silver + 100 * (ch)->gold)
#define EN_AREA(ch)	((ch)->in_room->area)
#define ES_VULN(ch,tipo)	(IS_SET(ch->vuln_flags,tipo))
#define ES_RES(ch,tipo)		(IS_SET(ch->res_flags,tipo))
#define ES_IMMUNE(ch,tipo)	(IS_SET(ch->imm_flags,tipo))
#define ES_MAT(obj,mat)		(!str_cmp(obj->material,mat))
#define NOM_RAZA(raza)		(race_table[raza].name)
// #define HATES(ch,victim)	(!str_cmp(race_table[ch->race].name, "all") || !str_infix(race_table[victim->race].name,race_table[ch->race].hate))
#define HATES(ch,victim)	(((bool *) (race_table[(ch)->race].odio))[(victim)->race] == TRUE)
#define CAN_SAY(ch)		( (IS_NPC(ch) && IS_SET((ch)->act, ACT_PSI)) || (!IS_AFFECTED2((ch),AFF_MUTE) && !IS_SET((ch)->in_room->room_flags, ROOM_CONE_OF_SILENCE)) )
#define es_caster(ch)		(class_table[getClasePr(ch)].fMana)
#define ES_WARRIOR(ch)		(getClasePr(ch) == CLASS_WARRIOR)
#define ES_THIEF(ch)		(getClasePr(ch) == CLASS_THIEF)
#define ES_PSI(ch)		(getClasePr(ch) == CLASS_PSI)
#define ES_MAGE(ch)		(getClasePr(ch) == CLASS_MAGE)
#define ES_CLERIC(ch)		(getClasePr(ch) == CLASS_CLERIC)
#define IS_MOB_CASTER(mob)	es_caster(mob)
#define ES_PMOB_CASTER(pmob)	(class_table[pmob->clase].fMana)
#define IS_MOB_MAGIC(mob)	(class_table[getClasePr(mob)].fMana && getClasePr(mob) != CLASS_PSI)
#define PROMEDIO_CH(ch)		((1 + ch->damage[DICE_NUMBER]) * ch->damage[DICE_TYPE] / 2 )
#define PROMEDIO_OBJ(obj)	((1 + obj->value[2]) * obj->value[1] / 2 )
#define ES_PKILL(ch)		(IS_NPC(ch) || IS_SET(ch->act, PLR_KILLER) || is_clan(ch))
#define IS_FORM(ch,bit)		(IS_SET((ch)->form,bit))
#define IS_PART(ch,bit)		(IS_SET((ch)->parts,bit))
#define IS_WIZNET(ch,bit)	(IS_SET((ch)->wiznet,bit))
#define SPEC_FUN(ch)		((ch)->pIndexData->spec_fun)
#define EDITANDO(ch)		((ch)->desc && ((ch)->desc->editor || IS_SET((ch)->in_room->room_flags, ROOM_PROTOTIPO)))
#define IS_PET(ch)		(IS_SET((ch)->act, ACT_PET))
#define WHEREIS(ch)		(ch->in_room ? ch->in_room->vnum : 0)
#define CHARVNUM(ch)		(IS_NPC(ch) ? ch->pIndexData->vnum : 0)
#define CHAR_SEXO(ch)		((ch)->sex == SEX_FEMALE ? 'a' : 'o')
#define CHAR_SEXO_UPPER(ch)	((ch)->sex == SEX_FEMALE ? 'A' : 'O')
#define STR_SEXO(ch)		((ch)->sex == SEX_FEMALE ? "a" : "o")
#define STR_SEXO_UPPER(ch)	((ch)->sex == SEX_FEMALE ? "A" : "O")
#define CLEAR_OFF(pmob)	{							\
				REMOVE_BIT(pmob->off_flags, OFF_BACKSTAB);	\
				REMOVE_BIT(pmob->off_flags, OFF_BASH);		\
				REMOVE_BIT(pmob->off_flags, OFF_BERSERK);	\
				REMOVE_BIT(pmob->off_flags, OFF_DISARM);	\
				REMOVE_BIT(pmob->off_flags, OFF_DODGE);		\
				REMOVE_BIT(pmob->off_flags, OFF_KICK);		\
				REMOVE_BIT(pmob->off_flags, OFF_KICK_DIRT);	\
				REMOVE_BIT(pmob->off_flags, OFF_PARRY);		\
				REMOVE_BIT(pmob->off_flags, OFF_RESCUE);	\
				REMOVE_BIT(pmob->off_flags, OFF_TAIL);		\
				REMOVE_BIT(pmob->off_flags, OFF_TRIP);		\
				REMOVE_BIT(pmob->off_flags, OFF_CRUSH);		\
				REMOVE_BIT(pmob->off_flags, OFF_CIRCLE);	\
			}
#define IS_BUILDER(ch, Area)	( !IS_NPC(ch) && !IS_SWITCHED(ch) && \
				( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	  \
				|| strstr( Area->builders, "All" ) ) )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)
#define ES_OWNER(ch,obj)	((obj)->owner && !str_cmp((ch)->name,(obj)->owner))
#define IS_TRAP(obj)		(obj->trap)
#define ES_ARMA(obj)		(obj->item_type == ITEM_WEAPON)
#define EXP_NIVEL(ch,nivel)	((exp_per_level((ch),IS_NPC(ch) ? 10 : (ch)->pcdata->points))*(nivel))
#define ES_INDEP(clan)		(IS_SET(get_clan_table(clan)->flags, CLAN_INDEP))
#define ES_CORPSE(obj)		(((obj)->item_type == ITEM_CORPSE_NPC) || ((obj)->item_type == ITEM_CORPSE_PC))
#define DETECTED(obj, bit)	(IS_SET((obj)->detected, (bit)))

/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: chcolor(ch) ) : "alguien" )
#define SELFPERS(ch)		( IS_NPC(ch) ? (ch)->short_descr : chcolor(ch) )
#define USTRSEX(ch)		( (ch)->sex == SEX_FEMALE ? "A" : "O" )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char *	name;
    char *	char_no_arg;
    char *	others_no_arg;
    char *	char_found;
    char *	others_found;
    char *	vict_found;
    char *	char_not_found;
    char *	char_auto;
    char *	others_auto;
};

/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];

extern	const	struct	class_type	class_table	[MAX_CLASS];
extern	const	struct	weapon_type	weapon_table	[];
extern  const   struct  item_type	item_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	attack_type	attack_table	[];
extern		struct	race_type	*race_table;
extern		struct	skill_type	*skill_table;
extern  const	struct	spec_type	spec_table	[];
extern	const	struct	game_type	game_table	[];
extern	const	struct	liq_type	liq_table	[];
extern		struct	group_type	*group_table;
extern		struct	social_type	*social_table;


extern char last_command [MAX_STRING_LENGTH];

/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;
extern		REPAIR_DATA	  *	repair_first;

extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;

extern		MPROG_CODE	  *	mprog_list;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		bool			MOBtrigger;
extern 		int		        share_value;
extern		bool			war;


/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
siz_t	fread		args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#else
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""			/* Player files	*/
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"proto.are"		/* To reserve one stream */
#endif

#if defined(MSDOS)
#define PLAYER_DIR	"../player/"			/* Player files */
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"nul"			/* To reserve one stream */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"        	/* Player files */
#define GOD_DIR         "../gods/"  		/* list of gods */
#define TEMP_FILE	"../player/romtmp"
#if defined(WIN32)
#define NULL_FILE	"nul"
#else
#define NULL_FILE	"/dev/null"		/* To reserve one stream */
#endif // WIN32
#endif // unix

#define CORPSE_DIR	"../corpses/"		/* Directorio de los cuerpos	*/
#define CORPSE_BAK_DIR	CORPSE_DIR "bak/"	/* Directorio esp de cuerpos	*/
#define DATA_DIR	"../data/"		/* Directorio de datos		*/
#define DUMP_DIR	DATA_DIR "dump/"	/* Directorio del dump		*/
#define PROG_DIR	DATA_DIR "progs/"
#define ROOM_DIR	"../rooms/"		/* Directorio de cuartos	*/
#define AREA_DIR	"../area/"
#define SCRIPT_DIR	AREA_DIR "scripts"	/* Directorio de scripts	*/
#define BIN_DIR		"../bin/"
#define LOG_DIR		"../log/"
#define CLANLOG_DIR	LOG_DIR "clan/"		/* Log de los clanes		*/
#define PLAYER_LOG_DIR	LOG_DIR "plog/"		/* Directorio de logs personales */

#define AREA_LIST       AREA_DIR "area.lst"	/* Lista de areas */
#define EXE_FILE	BIN_DIR "rom"
#define NEW_EXE_FILE	BIN_DIR "rom.new"
#define CONFIG_FILE	DATA_DIR "config"
#define BANK_FILE       DATA_DIR "banco"	/* Banco */
#define BUG_FILE        DATA_DIR "bugs"		/* For 'bug' and bug()*/
#define TYPO_FILE       DATA_DIR "typos"	/* For 'typo'*/
#define NOTE_FILE	DATA_DIR "notes.not"	/* For 'notes'*/
#define IDEA_FILE	DATA_DIR "ideas.not"
#define PENALTY_FILE	DATA_DIR "penal.not"
#define NEWS_FILE	DATA_DIR "news.not"
#define CHANGES_FILE	DATA_DIR "chang.not"
#define SHUTDOWN_FILE   DATA_DIR "shutdown.txt"	/* For 'shutdown'*/
#define BAN_FILE	DATA_DIR "ban.txt"
#define MUSIC_FILE	DATA_DIR "musica"
#define DISABLED_FILE	DATA_DIR "disabled.txt"	/* disabled commands */
#define LAST_COMMAND_FILE	DATA_DIR "last_command.txt"
#define LOG_NIVELES	LOG_DIR "niveles"	/* Log de los niveles		*/

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define GF	GAME_FUN
#define AD	AFFECT_DATA
#define MPC	MPROG_CODE
#define MD	MEM_DATA

/* act_comm.c */
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch, bool pets ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void    flog            args( ( char * fmt, ...) );
void	bugf		args( ( char * fmt, ...) );
void	clanlog		args( ( int, char * fmt, ...) );
void	talk_auction	args( ( char *argument ) );
void	mostrar_mensaje args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* act_enter.c */
RID  *get_random_room   args ( ( Entity * ) );
int	find_exit	args ( (CHAR_DATA *ch, char *argument) );

/* act_move.c */
FRetVal	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );

/* act_obj.c */
bool	can_loot	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );
void	nuke_corpse	args( ( char *, bool ) );

/* act_wiz.c */
void wiznet		args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
void copyover_recover	args( (void) );

/* alias.c */
void 	substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* comm.c */
#define act(format,ch,arg1,arg2,type)\
	new_act((format),chToEntidad((ch),FALSE),(arg1),(arg2),(type),POS_RESTING)
#define act_new(format,ch,arg1,arg2,type,pos)\
	new_act((format),chToEnt(ch),(arg1),(arg2),(type),(pos))
#define nact(format,ch,arg1,arg2,type)\
	new_act((format),(ch),(arg1),(arg2),(type),POS_RESTING)
void	new_act		args( ( const char *, Entity *, Entity *, Entity *, int, int ) );
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_room	args( ( const char *txt, ROOM_INDEX_DATA *room ) );

/* db.c */
char *	print_flags	args( ( int flag ));
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	new_create_object		args( ( OBJ_INDEX_DATA *pObjIndex, int level, bool giveid ) );
#define	create_object(obj,level)	new_create_object(obj,level,TRUE)
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
MPC *	get_mprog_index args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	log_char	args( ( CHAR_DATA *, const char * ) );
void	tail_chain	args( ( void ) );
void *	mud_malloc	args( ( size_t ) );
void *	mud_calloc	args( ( size_t, size_t ) );

/* db2.c */
void	recalc		args( ( MID *pMob ) );

/* effect.c */
void	acid_effect	args( (void *vo, int level, int dam, int target) );
void	cold_effect	args( (void *vo, int level, int dam, int target) );
void	fire_effect	args( (void *vo, int level, int dam, int target) );
void	poison_effect	args( (void *vo, int level, int dam, int target) );
void	shock_effect	args( (void *vo, int level, int dam, int target) );
void	water_effect	args( (CHAR_DATA *ch) );

/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
FRetVal newdamage	args( ( Entity *, CHAR_DATA *, int, int, int, bool ) );
#define	damage_old(ch,victim,dam,dt,class,show)		damage(ch,victim,dam,dt,class,show)
#define damage(ch,victim,dam,dt,class,show)		newdamage(chToEntidad(ch, FALSE),victim,dam,dt,class,show)
#define obj_damage(obj,victim,dam,dt,class,show)	newdamage(objToEntidad(obj, FALSE),victim,dam,dt,class,show)
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );

/* ftp.c */
bool	ftp_push	args( ( DESCRIPTOR_DATA * d ) );

/* handler.c */
int	get_count	args( ( OBJ_INDEX_DATA * ) );
bool	emptystring	args( ( const char * ) );
bool	es_kill_steal	args( ( CHAR_DATA *, CHAR_DATA *, bool ) );
void	nuke_mem	args( ( CHAR_DATA *, long, bool ) );
void	nuke_id		args( ( long, bool ) );
bool	is_friend	args( ( CHAR_DATA *, CHAR_DATA * ) );
AD  	*affect_find args( (AFFECT_DATA *paf, int sn));
char *	chcolor		args( ( CHAR_DATA *ch ) );
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void 	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int	liq_lookup	args( ( const char *name) );
int	weapon_lookup	args( ( const char *name) );
int	weapon_type	args( ( const char *name) );
char 	*weapon_name	args( ( int weapon_Type) );
char	*weapon_nombre	args( ( int weapon_Type) );
int	item_lookup	args( ( const char *name) );
char	*item_name	args( ( int item_type) ); 
int	attack_lookup	args( ( const char *name) );
int	race_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
#define	is_clan(ch)	( (ch)->clan != 0 )
bool	is_same_clan	args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch, bool secondary ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *, int ) );
int 	get_max_train	args( ( CHAR_DATA *, int ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool	is_exact_name	args( ( char *str, char *namelist ) );
bool	is_name_order	args( ( char *, char * ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_to_room	args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove_room args( (ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
bool	is_obj_affected	args( ( OBJ_DATA *obj, int sn ) );
bool	is_room_affected	args( ( ROOM_INDEX_DATA *, int ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_join_alt	args( ( CHAR_DATA *, AFFECT_DATA * ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	new_obj_from_char	args( ( OBJ_DATA *obj, bool ) );
#define obj_from_char(obj)	new_obj_from_char(obj, TRUE)
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
FRetVal	extract_obj	args( ( OBJ_DATA *obj, bool limit ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	alt_get_char_room	args( ( ROOM_INDEX_DATA *, char * ) );
CD *	get_char_world		args( ( CHAR_DATA *ch, char *argument ) );
CD *	alt_get_char_world	args( ( char * ) );
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer ) );
OD *	alt_get_obj_carry	args( ( CHAR_DATA *, char * ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type_list	args( ( int, OBJ_DATA * ) );
OD *	create_money	args( ( int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	color_value_string args( ( int term, int color, bool bold, bool flash , bool reverse, bool underline ) );
int 	strlen_color	args( ( char *argument ) );
int	roll_stat	args( ( CHAR_DATA *, int ) );
char *	capitalizar	args( ( const char *str ) );
int	parsebet	args( ( const int currentbet, const char *argument ) );
void	extract_mem	args( ( CHAR_DATA *ch, MEM_DATA *mem ) );
CD *	get_char_id	args( ( long id ) );
CD *	get_char_from_id args( ( long id ) );
CD *	get_char_id_room	args( ( CHAR_DATA *, long ) );
int	itemtablepos	args( ( int ) );
bool	es_vuln_mat	args( ( CHAR_DATA *, int ) );
bool	es_res_mat	args( ( CHAR_DATA *, int ) );
CD *	obj_carried_by	args( ( OBJ_DATA * ) );
void	strip_mem_char	args( ( CHAR_DATA *ch, int type ) );
void	give_mem_when	args( ( CHAR_DATA *, time_t, int, long ) );
#define give_mem(ch,reaction,id)	give_mem_when( (ch), current_time, (reaction), (id) )
CD *	get_random_char_world	args( ( CHAR_DATA *, CHAR_DATA * ) );
#define	char_died(ch)	ent_died(chToEnt(ch))
int	mob_best_door	args( ( CHAR_DATA * ) );
bool	mob_can_move	args( ( CHAR_DATA *, EXIT_DATA * ) );
char *	tformat		args( ( time_t when ) );
void	set_char	args( ( CHAR_DATA *, int ) );
int	obj_whereis_vnum	args( ( OBJ_DATA * ) );
ROOM_INDEX_DATA *	obj_whereis	args( ( OBJ_DATA * ) );
char *	itos		args( ( int ) );
#if !defined(USAR_MACROS)
bool	can_recall	args( ( CHAR_DATA * ) );
#else
#define	can_recall(ch)	(!IS_AFFECTED((ch),AFF_CURSE) && !IS_SET((ch)->in_room->room_flags, ROOM_NO_RECALL))
#endif
MD *	mem_lookup	args( ( MEM_DATA *, int ) );
MD *	mem_lookup_id	args( ( MEM_DATA *, long ) );
void	exit_set	args( ( ROOM_INDEX_DATA *, EXIT_DATA * ) );
char *	parentesis	args( ( const char * ) );
void	polymorph	args( ( CHAR_DATA *, int ) );
FIGHT_DATA *	give_fd	args( ( CHAR_DATA *, CHAR_DATA * ) );
void	extract_fd	args( ( CHAR_DATA *, FIGHT_DATA * ) );
bool	is_in_room	args( ( CHAR_DATA *, ROOM_INDEX_DATA * ) );
sh_int	get_obj_id	args( ( OBJ_DATA * ) );
void	extract_limit	args( ( OBJ_DATA * ) );
void	give_limit	args( ( OBJ_DATA *, CHAR_DATA * ) );
char *	affect_list	args( ( AFFECT_DATA * ) );
MEM_DATA *	mem_lookup_react_id	args( ( MEM_DATA *, int, int ) );
bool	es_spell_clase	args( ( int, int ) );
void	listar_razas	args( ( DESCRIPTOR_DATA *, CHAR_DATA * ) );
OD *	get_obj_vnum_list	args( ( OBJ_DATA *list, int vnum ) );
#if defined(USAR_MACROS)
#define getClasePr(ch)	(((ch) && (ch)->level_data) ? (ch)->level_data->clase : 0)
#define getNivelPr(ch)	(((ch) && (ch)->level_data) ? (ch)->level_data->nivel : 0)
#else
sh_int	getNivelPr	args( ( CHAR_DATA * ) );
sh_int	getClasePr	args( ( CHAR_DATA * ) );
#endif
void	setNivelPr	args( ( CHAR_DATA *, int ) );
void	setClasePr	args( ( CHAR_DATA *, int, int ) );
int	get_vnum_mob_name_area	args( ( char *, AREA_DATA * ) );
int	get_vnum_obj_name_area	args( ( char *, AREA_DATA * ) );
int	num_clases	args( ( CHAR_DATA * ) );
bool	puede_ser_clase	args( ( int, int, int ) );
bool	odia_alguien_cuarto	args( ( CHAR_DATA * ) );
void	addClase	args( ( CHAR_DATA *, int, int ) );
bool	es_clase	args( ( CHAR_DATA *, int ) );
char *	colorstrip	args( ( char * ) );
int	mat_lookup	args( ( char * material ) );
void	set_mob_raza	args( ( MOB_INDEX_DATA *, int ) );
void	mob_odio_check	args( ( ROOM_INDEX_DATA *, CHAR_DATA * ) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );

/* magic.c */
SpellDesc * find_spell	args( ( SpellDesc * spell, CHAR_DATA *ch, char *name) );
/* int 	mana_cost 	(CHAR_DATA *ch, int min_mana, int level); */
int	mana_cost	args( ( int, int, int ) );
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim, int dam_type ) );
void	obj_cast_spell	args( ( int sn, int level, Entity *,
				    CHAR_DATA *, Entity * ) );

/* mob_prog.c */
void	program_flow	args( ( sh_int vnum, char *source, Entity * ent, Entity * actor,
				Entity * arg1, Entity * arg2 ) );
bool	mp_act_trigger	args( ( char *argument, Entity *ent, Entity * actor,
				Entity *arg1, Entity *arg2, int type ) );
int	mp_percent_trigger args( ( Entity *ent, Entity *actor,
				Entity *arg1, Entity *arg2, int type ) );
void	mp_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool	mp_exit_trigger   args( ( CHAR_DATA *ch, int dir ) );
void	mp_give_trigger   args( ( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj ) );
void 	mp_greet_trigger  args( ( CHAR_DATA *ch ) );
void	mp_entryall_trigger	args( ( CHAR_DATA * ) );
void	mp_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
bool	mp_time_trigger		args( (CHAR_DATA *) );
bool	has_item		args( (CHAR_DATA *, sh_int, sh_int, bool) );

/* mob_cmds.c */
void	mob_interpret	args( ( Entity *, char * ) );
#if defined(USAR_MACROS)
#define	mprog_type_to_name(type)	flag_string(mprog_flags, (type))
#define oprog_type_to_name(type)	flag_string(oprog_flags, (type))
#define rprog_type_to_name(type)	flag_string(rprog_flags, (type))
#endif

/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
void	save_corpse	args( ( CHAR_DATA *, bool ) );

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
int	race_exp_per_level	args( ( int, int, int ) );
int	get_points	args( ( int, int ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );
bool	es_skill_racial	args( ( int, int ) );
bool	can_prac	args( ( CHAR_DATA *, int ) );
int	charMinLevelSn	args( ( CHAR_DATA *, int ) );
int	maxNivel	args( ( CHAR_DATA * ) );
SpellDesc * charTieneSn	args( ( SpellDesc *, CHAR_DATA *, int ) );
SpellDesc * blanquear_spelldesc args( ( SpellDesc * ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/* update.c */
void	level_up	args( ( CHAR_DATA * ) );
void	advance_level	args( ( CHAR_DATA *ch, bool hide ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
FRetVal	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void    bank_update     args( ( void ) );
void	camino_update	args( ( void ) );
void	auction_update	args( ( void ) );

/* string.c */
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );

/* hunt.c */
int	hunt_victim     args( ( CHAR_DATA *ch ) );
bool	can_hunt	args( ( CHAR_DATA *, CHAR_DATA *, bool ) );
bool	can_hunt_room	args( ( CHAR_DATA *, ROOM_INDEX_DATA * ) );
void	set_hunt	args( ( CHAR_DATA *, CHAR_DATA *, bool ) );
void	set_hunt_room	args( ( CHAR_DATA *, ROOM_INDEX_DATA * ) );
#define is_hunting(ch)	((ch)->hunt_data != NULL)
#define huntvictim(ch)	((ch)->hunt_data->victima)
#define huntroom(ch)	((ch)->hunt_data->room)
void	stop_hunting	args( ( CHAR_DATA *, bool, bool, bool ) );

/* trap.c */
bool  checkmovetrap  args( ( CHAR_DATA *ch, int dir) );
bool  checkgetput    args( ( CHAR_DATA *ch, OBJ_DATA *obj) );
bool  checkopen      args( ( CHAR_DATA *ch, OBJ_DATA *obj) );

/* games.c */
GF *    game_lookup     args( ( const char *name ) );
char *	game_name	args( ( GF *function) );

/* economy.c */
BANK_DATA * get_bank	args( ( CHAR_DATA *, int ) );

/* quest.c */
#include "quest.h"

/* entity.h */
#include "entity.h"

void printf_to_char (CHAR_DATA *ch, char *fmt, ...);

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef  AD
#undef  GF
#undef	MD


/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */
#define		AREA_ENVY	8	/* Envy area */
#define         AREA_PROTOTIPO  16      /* Area prototipo */
#define		AREA_ROM_OLD	32	/* Area estandar Rom 2.4 */
#define		AREA_RECALC	64	/* Recalculado de stats */


/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	char *	const	dir_nombre	[];
extern	char *	const	dir_nom		[];
extern	const	sh_int	rev_dir         [];
extern	const	struct	spec_type	spec_table	[];

/*
 * Global variables
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *             shop_last;
extern		REPAIR_DATA *		repair_last;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern		int			top_repair;

extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;
extern		int			top_mprog_index;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};

/* db.c */
void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );

/* string.c */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char * new ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *, char * ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );
char	*flag_string	args ( ( const struct flag_type *flag_table, long bits ) );

/* prototypes from db.c */
void  load_disabled   args( ( void ) );
void  save_disabled   args( ( void ) );

/* events.c */
void	add_event	args( ( CHAR_DATA *ch, int timer, int tipo, void *arg1, void *arg2, void *arg3, void *arg4 ) );
void	remove_events	args( ( CHAR_DATA *ch ) );

/* random.c */
void	crear_armadura_random	args( ( CHAR_DATA * ) );
bool	es_obj_random		args( ( OBJ_DATA * ) );

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )			\
		if ( !str_cmp( word, literal ) )	\
		{					\
			field  = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( string, field, value )			\
		if ( !str_cmp( word, string ) )		\
		{					\
			free_string( field );		\
			field = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#define KEY_IGNORE( string, value )			\
		if ( !str_cmp( word, string ) )		\
		{					\
			value;				\
			fMatch = TRUE;			\
			break;				\
		}

#define KEY_DO( string, dothis )			\
		if ( !str_cmp( word, string ) )		\
		{					\
			dothis;				\
			fMatch = TRUE;			\
			break;				\
		}

#define ARRAY_COPY( array1, array2, largo )				\
		{							\
			int _xxx_;					\
			for ( _xxx_ = 0; _xxx_ < largo; _xxx_++ )	\
				array1[_xxx_] = array2[_xxx_];		\
		}

#endif // _MERC_H
