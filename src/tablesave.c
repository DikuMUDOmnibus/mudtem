#include "include.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "plist.h"
#include "clan.h"
#include "recycle.h"
#include "puntos.h"
#include "language.h"

struct savetable_type
{
	char *	campo;
	sh_int	tipo_campo;
	void *	puntero_campo;
	const void *	argumento;
	const void *	argumento2;
};

struct cmd_type		cmd;
struct race_type	race;
PLIST			pl;
struct social_type	soc;
struct clan_type	clan;
PETITION_DATA		petic;
struct skill_type	sk;
MPROG_CODE		pcode;
struct rec_muerte	ppunt;

char * cmd_func_name( DO_FUN * );
DO_FUN * cmd_func_lookup( char * );

char * gsn_name( sh_int * );
sh_int * gsn_lookup( char * );

char * spell_name( SPELL_FUN * );
SPELL_FUN * spell_function( char * );

char * spcb_name_lookup( SPELL_CB * );
SPELL_CB * spcb_lookup( char * );

typedef char * STR_FUNC ( void * );
typedef bool   STR_READ_FUNC ( void *, char * );

char * do_fun_str( void * temp )
{
	DO_FUN ** fun = (DO_FUN **) temp;

	return cmd_func_name( *fun );
}

char * noclase_str( void * temp )
{
	bool * bleh = (bool *) temp;
static	char buf[MIL];
	int i;

	buf[0] = '\0';

	for ( i = 0; i < MAX_CLASS; i++ )
		if ( bleh[i] == TRUE )
		{
			strcat(buf, class_table[i].name);
			strcat(buf, " ");
		}

	if ( (i = strlen(buf)) > 0 )
		buf[i - 1] = '\0';

	return buf;
}

char * odio_str( void * temp )
{
	bool * bleh = *(bool **) temp;
static	char buf[MIL];
	char buf2[64];
	int i;

	buf[0] = '\0';

	for ( i = 0; i < maxrace; i++ )
		if ( bleh[i] == TRUE )
		{
			sprintf( buf2, "%s ", race_table[i].name );
			strcat( buf, buf2 );
		}

	if ((i = strlen(buf)) > 0)
		buf[i-1] = '\0';

	return buf;
}

char * position_str( void * temp )
{
	sh_int *flags = (sh_int *) temp;

	return position_table[*flags].name;
}

char * size_str( void * temp )
{
	sh_int *size = (sh_int *) temp;

	return size_table[UMAX(0, *size)].name;
}

char * lang_str( void * temp )
{
	sh_int *lang = (sh_int *) temp;

	return lang_table[*lang].name;
}

char * race_str( void * temp )
{
	sh_int *raza = (sh_int *) temp;

	return race_table[*raza].name;
}

char * clan_str( void * temp )
{
	sh_int *klan = (sh_int *) temp;

	return get_clan_table(*klan)->name;
}

char * class_str( void * temp )
{
	sh_int *class = (sh_int *) temp;

	return class_table[*class].name;
}

char * pgsn_str( void * temp )
{
	sh_int **pgsn = (sh_int **) temp;

	return gsn_name(*pgsn);
}

char * spell_fun_str( void * temp )
{
	SPELL_FUN ** spfun = (SPELL_FUN **) temp;

	return spell_name(*spfun);
}

char * spell_cb_str( void * temp )
{
	SPELL_CB ** spcb = (SPELL_CB **) temp;

	return spcb_name_lookup(*spcb);
}

bool odio_read( void * temp, char * arg )
{
	char *** odio = (char ***) temp;
	int cnt = 0;
	char raza[32];

	while( !emptystring(arg) )
	{
		arg = one_argument(arg, raza);
		(*odio)[cnt++] = str_dup(raza);
	}

	(*odio)[cnt] = str_dup("");

	return TRUE;
}

bool noclase_read( void * temp, char * arg )
{
	bool * noclase = (bool *) temp;
	int i = 0;
	char clase[32];

	while(!emptystring(arg))
	{
		arg = one_argument(arg, clase);
		i = class_lookup(clase);
		if ( i == -1 )
			bugf("noclase_read : clase %s invalida", clase);
		else
			noclase[i] = TRUE;
	}

	return TRUE;
}

bool race_read( void * temp, char * arg )
{
	sh_int * raza = (sh_int *) temp;

	*raza = race_lookup(arg);

	return (*raza == 0) ? FALSE : TRUE;
}

bool clan_read( void * temp, char * arg )
{
	sh_int * klan = (sh_int *) temp;

	*klan = clan_lookup(arg);

	return TRUE;
}

bool class_read( void * temp, char * arg )
{
	sh_int * class = (sh_int *) temp;

	*class = class_lookup(arg);

	if ( *class == -1 )
	{
		*class = 0;
		return FALSE;
	}

	return TRUE;
}

bool do_fun_read( void * temp, char * arg )
{
	DO_FUN ** fun = (DO_FUN **) temp;

	*fun = cmd_func_lookup(arg);

	return TRUE;
}

bool position_read( void * temp, char * arg )
{
	sh_int * posic = (sh_int *) temp;
	sh_int ffg = position_lookup(arg);

	*posic = UMAX(0, ffg);

	if ( ffg == -1 )
		return FALSE;
	else
		return TRUE;
}

bool size_read( void * temp, char * arg )
{
	sh_int * size = (sh_int *) temp;
	int ffg = size_lookup(arg);

	*size = UMAX(0, ffg);

	if ( ffg == -1 )
		return FALSE;
	else
		return TRUE;
}

bool lang_read( void * temp, char * arg )
{
	sh_int * lang = (sh_int *) temp;
	int blah = lang_lookup(arg);

	*lang = UMAX(0, blah);

	if (blah == -1)
		return FALSE;
	else
		return TRUE;
}

bool pgsn_read( void * temp, char * arg )
{
	sh_int ** pgsn = (sh_int **) temp;
	sh_int * blah = gsn_lookup(arg);

	*pgsn = blah;

	return !str_cmp(arg, "") || blah != NULL;
}

bool spell_fun_read( void * temp, char * arg )
{
	SPELL_FUN ** spfun = (SPELL_FUN **) temp;
	SPELL_FUN * blah = spell_function(arg);

	*spfun = blah;

	return !str_cmp(arg, "") || blah != NULL;
}

bool spell_cb_read( void * temp, char * arg )
{
	SPELL_CB ** spcb = (SPELL_CB **) temp;
	SPELL_CB * blah = spcb_lookup(arg);

	*spcb = blah;

	return !str_cmp(arg, "") || blah != NULL;
}

#define CAMPO_STRING			0
#define CAMPO_FUNCION_INT_TO_STR	1
#define CAMPO_SHINT			2
#define CAMPO_FLAGSTRING		3
#define CAMPO_INT			4
#define CAMPO_FLAGVECTOR		5
#define CAMPO_BOOL			6
#define CAMPO_SHINT_ARRAY		7
#define CAMPO_STRING_ARRAY		8
#define CAMPO_FUNCION_SHINT_TO_STR	9
#define CAMPO_SHINT_FLAGSTRING		10
#define CAMPO_BOOL_ARRAY		11
#define CAMPO_INUTIL			12

const struct savetable_type puntajesavetable [] =
{
	{	"muertes",	CAMPO_INT,	(void *) &ppunt.muertes,	NULL	},
	{	"activo",	CAMPO_INT,	(void *) &ppunt.activo,		NULL	},
	{	"nivel",	CAMPO_INT,	(void *) &ppunt.nivel,		NULL	},
	{	"name",		CAMPO_STRING,	(void *) &ppunt.name,		NULL	},
	{	"who_name",	CAMPO_STRING,	(void *) &ppunt.who_name,	NULL	},
	{	NULL,		0,		NULL,				NULL	}
};

const struct savetable_type progcodesavetable [] =
{
	{	"vnum",		CAMPO_SHINT,			(void *) &pcode.vnum,	NULL,	NULL	},
	{	"descripcion",	CAMPO_STRING,			(void *) &pcode.descripcion, NULL, NULL	},
	{	"code",		CAMPO_STRING,			(void *) &pcode.code,	NULL,	NULL	},
	{	NULL,		0,				NULL,			NULL,	NULL	}
};

const struct savetable_type skillsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &sk.name,		NULL,			NULL	},
	{	"nombre",	CAMPO_STRING,			(void *) &sk.nombre,		NULL,			NULL	},
	{	"skill_level",	CAMPO_SHINT_ARRAY,		(void *) &sk.skill_level,	(void *) MAX_CLASS,	NULL	},
	{	"rating",	CAMPO_SHINT_ARRAY,		(void *) &sk.rating,		(void *) MAX_CLASS,	NULL	},
	{	"spell_fun",	CAMPO_FUNCION_INT_TO_STR,	(void *) &sk.spell_fun,		spell_fun_str,		spell_fun_read	},
	{	"spell_callback", CAMPO_FUNCION_INT_TO_STR,	(void *) &sk.spell_callback,	spell_cb_str,		spell_cb_read	},
	{	"target",	CAMPO_SHINT_FLAGSTRING,		(void *) &sk.target,		target_table,		NULL	},
	{	"minimum_position", CAMPO_FUNCION_SHINT_TO_STR,	(void *) &sk.minimum_position,	position_str,		position_read	},
	{	"pgsn",		CAMPO_FUNCION_INT_TO_STR,	(void *) &sk.pgsn,		pgsn_str,		pgsn_read	},
	{	"slot",		CAMPO_SHINT,			(void *) &sk.slot,		NULL,			NULL	},
	{	"min_mana",	CAMPO_SHINT,			(void *) &sk.min_mana,		NULL,			NULL	},
	{	"beats",	CAMPO_SHINT,			(void *) &sk.beats,		NULL,			NULL	},
	{	"noun_damage",	CAMPO_STRING,			(void *) &sk.noun_damage,	NULL,			NULL	},
	{	"msg_off",	CAMPO_STRING,			(void *) &sk.msg_off,		NULL,			NULL	},
	{	"msg_obj",	CAMPO_STRING,			(void *) &sk.msg_obj,		NULL,			NULL	},
	{	"msg_room",	CAMPO_STRING,			(void *) &sk.msg_room,		NULL,			NULL	},
	{	"flags",	CAMPO_FLAGVECTOR,		(void *) &sk.flags,		NULL,			NULL	},
#if defined(MUD_SLANG)
	{	"slangscript",	CAMPO_STRING,			(void *) &sk.slangscript,	NULL,			NULL	},
#endif
	{	NULL,		0,				NULL,				NULL,			NULL	}
};

const struct savetable_type peticionsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &petic.name,	NULL,		NULL		},
	{	"nivel",	CAMPO_INT,			(void *) &petic.nivel,	NULL,		NULL		},
	{	"id",		CAMPO_INT,			(void *) &petic.id,	NULL,		NULL		},
	{	"oldclan",	CAMPO_FUNCION_SHINT_TO_STR,	(void *) &petic.oldclan,clan_str,	clan_read	},
	{	"clan",		CAMPO_FUNCION_SHINT_TO_STR,	(void *) &petic.clan,	clan_str,	clan_read	},
	{	"status",	CAMPO_SHINT,			(void *) &petic.status,	NULL,		NULL		},
	{	"when",		CAMPO_INT,			(void *) &petic.when,	NULL,		NULL		},
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

const struct savetable_type clansavetable [] =
{
	{	"name",		CAMPO_STRING,	(void *) &clan.name,	NULL,	NULL	},
	{	"who_name",	CAMPO_STRING,	(void *) &clan.who_name,NULL,	NULL	},
	{	"god",		CAMPO_STRING,	(void *) &clan.god,	NULL,	NULL	},
	{	"hall",		CAMPO_INT,	(void *) &clan.hall,	NULL,	NULL	},
	{	"recall",	CAMPO_INT,	(void *) &clan.recall,	NULL,	NULL	},
	{	"death",	CAMPO_INT,	(void *) &clan.death,	NULL,	NULL	},
	{	"pit",		CAMPO_INT,	(void *) &clan.pit,	NULL,	NULL	},
	{	"flags",	CAMPO_FLAGVECTOR, (void *) &clan.flags,	NULL,	NULL	},
	{	"pkills",	CAMPO_INT,	(void *) &clan.pkills,	NULL,	NULL	},
	{	"pdeaths",	CAMPO_INT,	(void *) &clan.pdeaths,	NULL,	NULL	},
	{	"mkills",	CAMPO_INT,	(void *) &clan.mkills,	NULL,	NULL	},
	{	"mdeaths",	CAMPO_INT,	(void *) &clan.mdeaths,	NULL,	NULL	},
	{	"illpkills",	CAMPO_INT,	(void *) &clan.illpkills, NULL,	NULL	},
	{	NULL,		0,		NULL,			NULL,	NULL	}
};

const struct savetable_type socialsavetable [] =
{
	{	"name",		CAMPO_STRING,	(void *) &soc.name,		NULL,	NULL	},
	{	"char_no_arg",	CAMPO_STRING,	(void *) &soc.char_no_arg, 	NULL,	NULL	},
	{	"others_no_arg",CAMPO_STRING,	(void *) &soc.others_no_arg,	NULL,	NULL	},
	{	"char_found",	CAMPO_STRING,	(void *) &soc.char_found,	NULL,	NULL	},
	{	"others_found",	CAMPO_STRING,	(void *) &soc.others_found,	NULL,	NULL	},
	{	"vict_found",	CAMPO_STRING,	(void *) &soc.vict_found,	NULL,	NULL	},
	{	"char_auto",	CAMPO_STRING,	(void *) &soc.char_auto,	NULL,	NULL	},
	{	"others_auto",	CAMPO_STRING,	(void *) &soc.others_auto,	NULL,	NULL	},
	{	NULL,		0,		NULL,				NULL,	NULL	}
};

const struct savetable_type plistsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &pl.name,	NULL,		NULL		},
	{	"id",		CAMPO_INT,			(void *) &pl.id,	NULL,		NULL		},
	{	"nivel",	CAMPO_INT,			(void *) &pl.nivel,	NULL,		NULL		},
	{	"clan",		CAMPO_FUNCION_SHINT_TO_STR,	(void *) &pl.clan,	clan_str,	clan_read	},
	{	"clan_status",	CAMPO_SHINT,			(void *) &pl.clan_status,NULL,		NULL		},
	{	"sex",		CAMPO_SHINT,			(void *) &pl.sex,	NULL,		NULL		},
	{	"class",	CAMPO_FUNCION_SHINT_TO_STR,	(void *) &pl.class,	class_str,	class_read	},
	{	"race",		CAMPO_FUNCION_SHINT_TO_STR,	(void *) &pl.race,	race_str,	race_read	},
	{	"lastlog",	CAMPO_INT,			(void *) &pl.lastlog,	NULL,		NULL		},
	{	"hostname",	CAMPO_STRING,			(void *) &pl.host,	NULL,		NULL		},
#if defined(CRC)
	{	"crc",		CAMPO_SHINT,			(void *) &pl.crc,	NULL,		NULL		},
#else
	{	"crc",		CAMPO_INUTIL,			NULL,			NULL,		NULL		},
#endif
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

const struct savetable_type racesavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &race.name,	NULL,		NULL		},
	{	"pc",		CAMPO_BOOL,			(void *) &race.pc_race,	NULL,		NULL		},
	{	"selec",	CAMPO_BOOL,			(void *) &race.seleccionable,	NULL,	NULL		},
	{	"act",		CAMPO_FLAGVECTOR,		(void *) &race.act,	NULL,		NULL		},
	{	"aff",		CAMPO_FLAGVECTOR,		(void *) &race.aff,	NULL,		NULL		},
	{	"aff2",		CAMPO_FLAGVECTOR,		(void *) &race.aff2,	NULL,		NULL		},
	{	"off",		CAMPO_FLAGVECTOR,		(void *) &race.off,	NULL,		NULL		},
	{	"imm",		CAMPO_FLAGVECTOR,		(void *) &race.imm,	NULL,		NULL		},
	{	"res",		CAMPO_FLAGVECTOR,		(void *) &race.res,	NULL,		NULL		},
	{	"vuln",		CAMPO_FLAGVECTOR,		(void *) &race.vuln,	NULL,		NULL		},
	{	"form",		CAMPO_FLAGVECTOR,		(void *) &race.form,	NULL,		NULL		},
	{	"parts",	CAMPO_FLAGVECTOR,		(void *) &race.parts,	NULL,		NULL		},
	{	"lang",		CAMPO_FUNCION_INT_TO_STR,	(void *) &race.race_lang,	lang_str,	lang_read		},
	{	"hate",		CAMPO_STRING,			(void *) &race.hate,	NULL,		NULL		},
	{	"salir",	CAMPO_STRING,			(void *) &race.msg_salir,	NULL,	NULL		},
	{	"macho",	CAMPO_STRING,			(void *) &race.macho,	NULL,		NULL		},
	{	"hembra",	CAMPO_STRING,			(void *) &race.hembra,	NULL,		NULL		},
	{	"who",		CAMPO_STRING,			(void *) &race.who_name,NULL,		NULL		},
	{	"points",	CAMPO_SHINT,			(void *) &race.points,	NULL,		NULL		},
	{	"cmult",	CAMPO_SHINT_ARRAY,		(void *) &race.class_mult,	(void *) MAX_CLASS,	NULL	},
	{	"skills",	CAMPO_STRING_ARRAY,		(void *) &race.skills,	(void *) MAX_RACE_SKILL,	NULL		},
	{	"stats",	CAMPO_SHINT_ARRAY,		(void *) &race.stats,	(void *) MAX_STATS,	NULL		},
	{	"maxstats",	CAMPO_SHINT_ARRAY,		(void *) &race.max_stats,(void *) MAX_STATS,	NULL		},
	{	"size",		CAMPO_FUNCION_INT_TO_STR,	(void *) &race.size,	size_str,	size_read	},
	{	"noalign",	CAMPO_FLAGVECTOR,		(void *) &race.noalign,	NULL,		NULL		},
	{	"noclase",	CAMPO_INUTIL,			(void *) &race.noclase,	NULL,		NULL		},
	{	"no_clase",	CAMPO_FUNCION_INT_TO_STR,	(void *) &race.noclase,	noclase_str,	noclase_read	},
	{	"remort",	CAMPO_BOOL,			(void *) &race.remort_race,	NULL,	NULL		},
	{	"odio",		CAMPO_INUTIL,			(void *) &race.odio,	NULL,		NULL		},
	{	"odio_r",	CAMPO_FUNCION_INT_TO_STR,	(void *) &race.odio,	odio_str,	odio_read	},
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

const struct savetable_type cmdsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &cmd.name,	NULL,		NULL		},
	{	"do_fun",	CAMPO_FUNCION_INT_TO_STR,	(void *) &cmd.do_fun,	do_fun_str,	do_fun_read	},
	{	"position",	CAMPO_FUNCION_SHINT_TO_STR,	(void *) &cmd.position,	position_str,	position_read	},
	{	"level",	CAMPO_SHINT,			(void *) &cmd.level,	NULL,		NULL		},
	{	"log",		CAMPO_FLAGSTRING,		(void *) &cmd.log,	log_flags,	NULL		},
	{	"show",		CAMPO_FLAGSTRING,		(void *) &cmd.show,	show_flags,	NULL		},
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

void load_struct( FILE *fp, void * tipobase, const struct savetable_type * tabla, void * puntero )
{
	char * word;
	const struct savetable_type * temp;
	sh_int * pshint;
	int * pentero;
	char ** pcadena;
	char * cadena;
	int * pint;
	STR_READ_FUNC * funcion;
	struct flag_type * flagtable;
	bool found = FALSE;
	bool * pbool;
	int cnt = 0, i;

	while ( str_cmp((word = fread_word(fp)), "#END") )
	{
		for ( temp = tabla; !IS_NULLSTR(temp->campo); temp++ )
		{
			if ( !str_cmp( word, temp->campo ) )
			{
				// lo encontramos!
				switch(temp->tipo_campo)
				{
					case CAMPO_STRING:
					pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pcadena = fread_string(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT:
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pshint = (sh_int) fread_number(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_INT:
					pint = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pint = fread_number(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_FUNCION_INT_TO_STR:
					funcion = temp->argumento2;
					cadena = fread_string(fp);
					pint = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					if ((*funcion) (pint, cadena) == FALSE)
						bugf( "load_struct : campo %s invalido, cadena %s",
							temp->campo, cadena );
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FUNCION_SHINT_TO_STR:
					funcion = temp->argumento2;
					cadena = fread_string(fp);
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					if ((*funcion) (pshint, cadena) == FALSE)
						bugf( "load_struct : campo %s invalido, cadena %s",
							temp->campo, cadena );
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FLAGSTRING:
					flagtable = (struct flag_type *) temp->argumento;
					pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_string(fp);
					if ( (*pentero = flag_value(flagtable, cadena)) == NO_FLAG )
						*pentero = 0;
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT_FLAGSTRING:
					flagtable = (struct flag_type *) temp->argumento;
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_string(fp);
					if ( (*pshint = flag_value(flagtable, cadena)) == NO_FLAG )
						*pshint = 0;
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FLAGVECTOR:
					pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pentero = fread_flag(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_BOOL:
					pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_word(fp);
					*pbool = str_cmp( cadena, "false" ) ? TRUE : FALSE;
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT_ARRAY:
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_word(fp)), "@") )
					{
						if ( i == (int) temp->argumento )
							bugf( "load_struct : campo_shint_array %s con exceso de elementos",
								temp->campo );
						else
							pshint[i++] = (sh_int) atoi(cadena);
					}
					found = TRUE, cnt++;
					break;

					case CAMPO_STRING_ARRAY:
					pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_string(fp)), "@") )
					{
						if ( i == (int) temp->argumento )
							bugf( "load_struct : campo_string_array %s con exceso de elementos",
								temp->campo);
						else
							pcadena[i++] = cadena;
					}
					found = TRUE, cnt++;
					break;

					case CAMPO_INUTIL:
					fread_to_eol(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_BOOL_ARRAY:
					pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_word(fp)), "@") )
					{
						if ( (temp->argumento != NULL
						  && i == (int) temp->argumento)
						||   (temp->argumento == NULL
						  && temp->argumento2 != NULL
						  && i == *( (int *) temp->argumento2)) )
							bugf( "load_struct : campo_bool_array %s con exceso de elementos",
								temp->campo );
						else
							pbool[i++] = (bool) atoi(cadena);
					}
					found = TRUE, cnt++;
					break;
				} // switch
				if (found == TRUE)
					break;
			} // if
		} // for

		if (found == FALSE)
		{
			bugf( "load_struct : clave %s no encontrada", word );
			fread_to_eol(fp);
		}
		else
			found = FALSE;
	} // while
}

void save_struct( FILE *fp, void * tipobase, const struct savetable_type * tabla, void * puntero )
{
	const struct savetable_type * temp;
	char ** pcadena;
	sh_int * pshint;
	STR_FUNC * funcion;
	char * cadena;
	int * pentero;
	bool * pbool;
	const struct flag_type * flagtable;
	int cnt = 0, i;

	for ( temp = tabla; !IS_NULLSTR(temp->campo); temp++ )
	{
		switch(temp->tipo_campo)
		{
			default:
			bugf( "save_struct : tipo_campo %d invalido, campo %s",
				temp->tipo_campo, temp->campo );
			break;

			case CAMPO_STRING:
			pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, !IS_NULLSTR(*pcadena) ? *pcadena : "" );
			break;

			case CAMPO_SHINT:
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %d\n", temp->campo, *pshint );
			break;

			case CAMPO_INT:
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %d\n", temp->campo, *pentero );
			break;

			case CAMPO_FUNCION_INT_TO_STR:
			funcion = temp->argumento;
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			cadena = (*funcion) ((void *) pentero);
			fprintf( fp, "%s %s~\n", temp->campo, cadena );
			break;

			case CAMPO_FUNCION_SHINT_TO_STR:
			funcion = temp->argumento;
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			cadena = (*funcion) ((void *) pshint);
			fprintf( fp, "%s %s~\n", temp->campo, cadena );
			break;

			case CAMPO_FLAGSTRING:
			flagtable = (struct flag_type *) temp->argumento;
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, flag_string(flagtable, *pentero) );
			break;

			case CAMPO_SHINT_FLAGSTRING:
			flagtable = (struct flag_type *) temp->argumento;
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, flag_string(flagtable, *pshint) );
			break;

			case CAMPO_FLAGVECTOR:
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s\n", temp->campo, print_flags(*pentero) );
			break;

			case CAMPO_BOOL:
			pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s\n", temp->campo,
						(*pbool == TRUE) ? "true" : "false" );
			break;

			case CAMPO_SHINT_ARRAY:
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (int) temp->argumento; i++ )
				fprintf( fp, "%d ", pshint[i] );
			fprintf( fp, "@\n" );
			break;

			case CAMPO_STRING_ARRAY:
			pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (int) temp->argumento; i++ )
				fprintf( fp, "%s~ ", !IS_NULLSTR(pcadena[i]) ? pcadena[i] : "" );
			fprintf( fp, "@~\n" );
			break;

			case CAMPO_BOOL_ARRAY:
			pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (temp->argumento ? (int) temp->argumento : *(int *) temp->argumento2); i++ )
				fprintf( fp, "%d ", pbool[i] == TRUE ? 1 : 0 );
			fprintf( fp, "@\n" );
			break;

			case CAMPO_INUTIL:
			break;
		}

		cnt++;
	}
};

void grabar_tabla_comandos( void )
{
	FILE * fp;
	struct cmd_type * temp;
extern	struct cmd_type * cmd_table;
	int cnt = 0;

	fp = fopen( DATA_DIR "comandos", "w" );

	if ( !fp )
	{
		perror( "grabar_tabla_comandos" );
		return;
	}

	for ( temp = cmd_table; !IS_NULLSTR(temp->name); temp = temp++ )
		cnt++;

	fprintf( fp, "%d\n\n", cnt );

	for ( temp = cmd_table; !IS_NULLSTR(temp->name); temp = temp++ )
	{
		fprintf( fp, "#COMANDO\n" );
		save_struct( fp, &cmd, cmdsavetable, temp );
		fprintf( fp, "#END\n\n" );
	}

	fclose(fp);
}

void cargar_comandos( void )
{
	FILE * fp;
extern	struct cmd_type *	cmd_table;
extern	int			MAX_CMD;
static	struct cmd_type		emptycmd;
	int i = 0, largo;
	char * word;

	fp = fopen( DATA_DIR "comandos", "r" );

	if ( fp == NULL )
	{
		perror( "load_tabla_comandos ");
		return;
	}

	largo = fread_number(fp);

	MAX_CMD = largo;

	flog( "Creando cmd_table de largo %d, tama~no %d", largo + 1,
		sizeof(struct cmd_type) * (largo + 1) );
	cmd_table = mud_calloc( sizeof(struct cmd_type), largo + 1);

	for ( i = 0; i <= largo; i++ )
		cmd_table[i] = emptycmd; // limpiar

	i = 0;

	while(TRUE)
	{
		word = fread_word(fp);

		if ( str_cmp(word, "#COMANDO") )
		{
			bugf( "load_tabla_comandos : word %s", word );
			fclose(fp);
			return;
		}

		load_struct( fp, &cmd, cmdsavetable, &cmd_table[i++] );

		if ( i == largo )
		{
			flog( "Tabla de comandos cargada." );
			fclose(fp);
			cmd_table[i].name = str_dup( "" );
			return;
		}
	}
}

void grabar_razas( void )
{
	FILE * fp;
	struct race_type * temp;
extern	struct race_type * race_table;
	int cnt = 0;

	fp = fopen( DATA_DIR "temprazas", "w" );

	if ( !fp )
	{
		perror( "grabar_razas : fopen" );
		return;
	}

	for ( temp = race_table; !IS_NULLSTR(temp->name); temp = temp++ )
		cnt++;

	fprintf( fp, "%d\n\n", cnt );

	for ( temp = race_table; !IS_NULLSTR(temp->name); temp = temp++ )
	{
		fprintf( fp, "#RAZA\n" );
		save_struct( fp, &race, racesavetable, temp );
		fprintf( fp, "#END\n\n" );
	}

	fclose(fp);

	if (rename(DATA_DIR "temprazas", DATA_DIR "razas") == -1)
		perror("grabar_razas : rename");
}

void cargar_razas( void )
{
	FILE * fp;
	extern int maxrace;
static	struct race_type cRace;
	char * word;
	char ** temp;
	bool * tempb;
	int i, j;

	fp = fopen( DATA_DIR "razas", "r" );

	if ( !fp )
	{
		perror( "cargar_tabla_razas" );
		return;
	}

	maxrace = fread_number(fp);

	flog( "Creando race_table de largo %d, tama~no %d", maxrace + 1,
		sizeof(struct race_type) * (maxrace + 1) );

	race_table = mud_calloc( sizeof(struct race_type), maxrace + 1 );

	// limpiar razas
	for ( i = 0; i <= maxrace; i++ )
	{
		race_table[i] = cRace;
		temp = mud_calloc(sizeof(char *), maxrace);
		if (temp == NULL)
		{
			bugf("cargar_razas : mud_calloc");
			exit(1);
		}
		race_table[i].odio = temp;
		race_table[i].race_id = i;
	}

	i = 0;

	while(TRUE)
	{
		word = fread_word(fp);

		if ( str_cmp(word, "#RAZA") )
		{
			bugf( "cargar_tabla_razas : word %s", word );
			fclose(fp);
			return;
		}

		load_struct( fp, &race, racesavetable, &race_table[i++] );

		if ( i == maxrace )
		{
			flog( "Tabla de razas cargada." );
			fclose(fp);
			race_table[i].name = NULL;
			// consolidar odio racial
			for ( i = 0; i < maxrace; i++ )
			{
				temp = (char **) race_table[i].odio;
				tempb = mud_calloc(sizeof(bool), maxrace);
				for ( j = 0; j < maxrace; j++ )
					tempb[j] = FALSE;
				j = 0;
				while(str_cmp(temp[j],""))
				{
					int r = race_lookup(temp[j]);
					if ( r == 0 )
						bugf("cargar_razas : odio %s en raza %s",
							temp[j], race_table[i].name );
					else
						tempb[r] = TRUE;
					free_string(temp[j++]);
				}
				free(race_table[i].odio);
				race_table[i].odio = tempb;
			}
			return;
		}
	}
}

void grabar_plist( void )
{
	FILE *  fp;
	PLIST * plt;
extern	PLIST * player_list[26];
	int i;

	fp = fopen( DATA_DIR "plist", "w" );

	if ( !fp )
	{
		perror ( "save_plist" );
		return;
	}

	for ( i = 0; i < 26; ++i )
	{
		for ( plt = player_list[i]; plt; plt = plt->next )
		{
			fprintf( fp, "#PLAYER\n" );
			save_struct( fp, &pl, plistsavetable, plt );
			fprintf( fp, "#END\n\n" );
		}
	}

	fprintf( fp, "@\n" );

	fclose ( fp );

	return;
}

void cargar_plist( void )
{
	FILE *fp;
	char * word;
	PLIST * plx, * plz;
extern	PLIST * player_list[26];
extern	PLIST * new_plist( void );

	fp = fopen( DATA_DIR "plist", "r" );

	if ( !fp )
	{
		perror( "cargar_plist" );
		return;
	}

	for (;;)
	{
		word = fread_word(fp);

		if ( !str_cmp(word, "@") || feof(fp) )
		{
			fclose(fp);
			flog( "Lista de jugadores cargada." );
			return;
		}

		if ( str_cmp( word, "#PLAYER" ) )
		{
			bugf( "cargar_plist : clave %s invalida",
				word );
			fclose(fp);
			return;
		}

		plx = new_plist();
		load_struct( fp, &pl, plistsavetable, plx );

		if ( (plz = plist_lookup_id( plx->id )) == NULL )
		{
			plx->next			  = player_list[HASHKEY(plx->name[0])];
			player_list[HASHKEY(plx->name[0])] = plx;
		}
		else
		{
			bugf( "cargar_plist : jugador %s == %s duplicado", plx->name, plz->name );
			free_plist( plx );
		}
	}
}

#define SOCIAL_FILE	DATA_DIR "socials"

void grabar_socials(void)
{
	FILE *fp;
	int i;
	extern int maxSocial;
	
	fp = fopen (SOCIAL_FILE, "w");
	
	if (!fp)
	{
		perror( SOCIAL_FILE );
		return;
	}

	fprintf (fp, "%d\n", maxSocial);

	for ( i = 0 ; i < maxSocial ; i++)
	{
		fprintf( fp, "#SOCIAL\n" );
		save_struct( fp, &soc, socialsavetable, &social_table[i] );
		fprintf( fp, "#END\n\n" );
	}

	fclose (fp);
}

void cargar_socials (void)
{
	FILE *fp;
	int i;
	extern int maxSocial;
	char * clave;
	
	fp = fopen (SOCIAL_FILE, "r");

	if (!fp)
	{
		perror(SOCIAL_FILE);
		exit(1);
	}

	fscanf (fp, "%d\n", &maxSocial);

	flog( "Creando social_table de largo %d, tama~no %d", maxSocial + 1,
		sizeof(struct social_type) * (maxSocial + 1) );
	/* IMPORTANT to use malloc so we can realloc later on */
	social_table = mud_malloc (sizeof(struct social_type) * (maxSocial+1));
	
	for (i = 0; i < maxSocial; i++)
	{
		if ( str_cmp((clave = fread_word(fp)), "#SOCIAL") )
		{
			bugf( "cargar_socials : clave %s inexistente",
				clave );
			exit(1);
		}
		load_struct (fp, &soc, socialsavetable, &social_table[i]);
	}

	/* For backwards compatibility */
	social_table[maxSocial].name = str_dup(""); /* empty! */		

	fclose (fp);

	flog( "Tabla de socials cargada." );
}

#define CLAN_FILE DATA_DIR "clanes"

void cargar_clanes (void)
{
	FILE *fp;
	int i;
	extern sh_int clan_count;
	extern struct clan_type * clan_table;
	char * clave;
	
	fp = fopen (CLAN_FILE, "r");

	if (!fp)
	{
		perror(CLAN_FILE);
		exit(1);
	}

	fscanf (fp, "%hd\n", &clan_count);

	flog( "Creando clan_table de largo %d, tama~no %d", clan_count + 1,
		sizeof(struct clan_type) * (clan_count + 1) );
	/* IMPORTANT to use malloc so we can realloc later on */
	clan_table = mud_malloc (sizeof(struct clan_type) * (clan_count+1));
	
	for (i = 0; i < clan_count; i++)
	{
		if ( str_cmp((clave = fread_word(fp)), "#CLAN") )
		{
			bugf( "cargar_clanes : clave %s inexistente",
				clave );
			exit(1);
		}
		load_struct (fp, &clan, clansavetable, &clan_table[i]);
	}

	clan_table[i].name = NULL;

	fclose (fp);

	flog( "Tabla de clanes cargada." );
}

void grabar_clanes(void)
{
	FILE *fp;
	int i;
	extern sh_int clan_count;
	extern struct clan_type * clan_table;
	
	fp = fopen (CLAN_FILE, "w");
	
	if (!fp)
	{
		perror( CLAN_FILE );
		return;
	}

	fprintf (fp, "%d\n", clan_count);

	for ( i = 0 ; i < clan_count ; i++)
	{
		fprintf( fp, "#CLAN\n" );
		save_struct( fp, &clan, clansavetable, &clan_table[i] );
		fprintf( fp, "#END\n\n" );
	}

	fclose (fp);
}

void cargar_peticiones(void)
{
	FILE *fp;
	PETITION_DATA *pet;
extern	PETITION_DATA *petition_list;
	char *word;

	petition_list = NULL;

	fp = fopen( DATA_DIR "peticiones", "r" );

	if ( !fp )
	{
		perror( "cargar_peticiones" );
		return;
	}

	while (TRUE)
	{
		word = fread_word( fp );

		if ( feof(fp) || !str_cmp( word, "#!" ) )
		{
			fclose( fp );
			flog( "Tabla de peticiones cargada." );
			return;
		}

		if ( !str_cmp( word, "#PETICION" ) )
		{
			pet		= new_petition();
			load_struct( fp, &petic, peticionsavetable, pet );
			pet->next	= petition_list;
			petition_list	= pet;
			continue;
		}

		bugf( "cargar_peticiones : clave %s invalida", word );
		fclose(fp);
		return;
	}
}

void grabar_peticiones(void)
{
	FILE * fp;
	PETITION_DATA * pet;
extern	PETITION_DATA * petition_list;

	fp = fopen( DATA_DIR "peticiones", "w" );

	if ( !fp )
	{
		perror( "grabar_peticiones" );
		return;
	}

	for ( pet = petition_list; pet; pet = pet->next )
	{
		fprintf( fp, "#PETICION\n" );
		save_struct( fp, &petic, peticionsavetable, pet );
		fprintf( fp, "#END\n\n" );
	}

	fprintf( fp, "#!\n" );
	fclose(fp);
}

#define SKILL_FILE DATA_DIR "newskills"

void grabar_skills( void )
{
	FILE *fpn;
	int i;

	fpn = fopen( SKILL_FILE, "w" );

	if ( !fpn )
	{
		bug( "Save_skills : NULL fpn", 0 );
		fclose( fpn );
		return;
	}

	fprintf( fpn, "%d\n", MAX_SKILL );

	for ( i = 0; i < MAX_SKILL; ++i )
	{
		fprintf( fpn, "#SKILL\n" );
		save_struct( fpn, &sk, skillsavetable, &skill_table[i] );
		fprintf( fpn, "#END\n\n" );
	}

	fprintf( fpn, "#!\n" );

	fclose( fpn );
}

void	cargar_skills( void )
{
	FILE *fp;
static	struct skill_type skzero;
	int i = 0;
	char *word;

	fp = fopen( SKILL_FILE, "r" );

	if ( !fp )
	{
		bug( "No se pudo leer " SKILL_FILE " para cargar skills.", 0 );
		exit(1);
	}

	fscanf( fp, "%d\n", &MAX_SKILL );

	flog( "Creando tabla de skills de largo %d, tama~no %d",
		MAX_SKILL + 1, sizeof(struct skill_type) * (MAX_SKILL + 1) );
	skill_table = mud_malloc( sizeof(struct skill_type) * (MAX_SKILL + 1) );

	if ( !skill_table )
	{
		bug( "Error! Skill_table == NULL, MAX_SKILL : %d", MAX_SKILL );
		exit(1);
	}

	for ( ; ; )
	{
		word = fread_word( fp );

		if ( !str_cmp( word, "#!" ) )
			break;

		if ( str_cmp( word, "#SKILL" ) )
		{
			bugf( "Cargar_skills : clave inexistente (%s)", word );
			exit(1);
		}

		if ( i >= MAX_SKILL )
		{
			bug( "Cargar_skills : numero de skills mayor que MAX_SKILL", 0 );
			exit(1);
		}

		skill_table[i] = skzero;
		load_struct( fp, &sk, skillsavetable, &skill_table[i++] );
	}

	skill_table[MAX_SKILL].name = NULL;

	fclose(fp);
}

void grabar_progs( int minvnum, int maxvnum )
{
	FILE * fp;
	MPROG_CODE * pMprog;
	char buf[64];

	for ( pMprog = mprog_list; pMprog; pMprog = pMprog->next )
		if ( pMprog->changed == TRUE
		&&   ENTRE_I(minvnum, pMprog->vnum, maxvnum) )
		{
			sprintf(buf, PROG_DIR "%d.prg", pMprog->vnum );
			fp = fopen( buf, "w" );

			if ( !fp )
			{
				perror(buf);
				return;
			}

			fprintf( fp, "#PROG\n" );
			save_struct( fp, &pcode, progcodesavetable, pMprog );
			fprintf( fp, "#END\n\n" );
			fclose(fp);

			pMprog->changed = FALSE;
		}
}

void cargar_prog( FILE * fp, MPROG_CODE ** prog )
{
extern	MPROG_CODE * mprog_list;
static	MPROG_CODE mprog_zero;
	char * word = fread_word(fp);

	if (str_cmp(word, "#PROG"))
	{
		bugf("cargar_prog : clave %s invalida", word);
		*prog = NULL;
		return;
	}

	*prog = alloc_perm(sizeof(MPROG_CODE));

	// blanquear
	**prog = mprog_zero;

	load_struct( fp, &pcode, progcodesavetable, *prog );

	// a la lista
	if (mprog_list == NULL)
		mprog_list = *prog;
	else
	{
		// al comienzo o al final? VNUM decide
		if ((*prog)->vnum < mprog_list->vnum)
		{
			(*prog)->next	= mprog_list;
			mprog_list	= *prog;
		}
		else
		{
			MPROG_CODE * temp, * prev = mprog_list;

			for ( temp = mprog_list->next; temp; temp = temp->next )
			{
				if ( temp->vnum > (*prog)->vnum )
					break;
				prev = temp;
			}
			prev->next = *prog;
			(*prog)->next = temp;
		}
	}
}

MPROG_CODE * pedir_prog( int vnum )
{
	FILE * fp;
	MPROG_CODE * prog;
	char buf[128];
	extern bool fBootDb;

	prog = get_mprog_index(vnum);

	if (prog != NULL)
		return prog;

	sprintf(buf, PROG_DIR "%d.prg", vnum );

	fp = fopen(buf,"r");

	if ( !fp )
	{
		if ( fBootDb == TRUE )
			perror("pedir_prog");
		return NULL;
	}

	cargar_prog(fp, &prog);

	fclose(fp);

	return prog;
}

sh_int contador[MAX_LEVEL/10 + 1];

void cargar_puntajes( void )
{
	FILE * fp;
static	struct rec_muerte temp;
	char * word;
	int i,j;

	temp.name	= str_dup( "" );
	temp.who_name	= str_dup( "" );

	for ( i = 0; i < MAX_LEVEL/10 + 1; i++ )
		for ( j = 0; j < LARGO_TABLA_PUNTOS; j++ )
			puntajes[i][j] = temp;

	fp = fopen( PUNTOS_FILENAME, "r" );

	if ( !fp )
	{
		perror( "leer_puntajes" );
		return;
	}

	for (;;)
	{
		word = fread_word(fp);

		if ( !str_cmp(word,"#!") )
		{
			fclose(fp);
			return;
		}

		if ( str_cmp(word,"#PUNTAJE") )
		{
			bugf("leer_puntajes : clave %s invalida", word );
			fclose(fp);
			return;
		}

		load_struct( fp, &ppunt, puntajesavetable, &temp );

		if ( contador[temp.nivel/10] < LARGO_TABLA_PUNTOS )
			puntajes[temp.nivel/10][contador[temp.nivel/10]++] = temp;
	}
}

void grabar_puntajes( void )
{
	FILE * fp;
	int i,j;

	fp = fopen( PUNTOS_FILENAME, "w" );

	if ( !fp )
	{
		perror( "grabar_puntajes" );
		return;
	}

	for ( i = 0; i < MAX_LEVEL/10 + 1; i++ )
		for ( j = 0; j < LARGO_TABLA_PUNTOS; j++ )
		{
			fprintf( fp, "#PUNTAJE\n" );
			save_struct( fp, &ppunt, puntajesavetable, &puntajes[i][j] );
			fprintf( fp, "#END\n\n" );
		}

	fprintf( fp, "#!\n" );
	fclose(fp);
}
