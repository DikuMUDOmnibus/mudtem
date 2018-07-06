#include "include.h"
#include "tables.h"

/*
	VNUM	BIT			DESC
	31	ITEM_HOLD		lampara
	32	ITEM_WEAR_FINGER	anillo
	33	ITEM_WEAR_FINGER	anillo
	34	ITEM_WEAR_NECK		collar
	35	ITEM_WEAR_NECK		capa
	36	ITEM_WEAR_BODY		armadura
	37	ITEM_WEAR_HEAD		casco
	38	ITEM_WEAR_LEGS		polainas
	39	ITEM_WEAR_FEET		botas
	40	ITEM_WEAR_HANDS		guanteletes
	41	ITEM_WEAR_ARMS		mangas
	42	ITEM_WEAR_SHIELD	escudo
	43	ITEM_WEAR_ABOUT		toga
	44	ITEM_WEAR_WAIST		cinturon
	45	ITEM_WEAR_WRIST		brazalete
	46	ITEM_WEAR_WRIST		cinto
	47	ITEM_WIELD		arma
*/

struct random_armadura_type
{
	int vnum;
	int peso;
	int wear_loc;
	char *prefix;
	char *desc;
};

struct random_armadura_type r_arm_table [] =
{
	{	31,	50,	WEAR_HOLD,	"a ",	"luz"		}, // 0
	{	32,	10,	WEAR_FINGER_L,	" ",	"anillo"	}, // 1
	{	33,	10,	WEAR_FINGER_R,	" ",	"anillo"	}, // 2
	{	34,	10,	WEAR_NECK_1,	" ",	"collar"	}, // 3
	{	35,	10,	WEAR_NECK_2,	"a ",	"capa"		}, // 4
	{	36,	100,	WEAR_BODY,	"a ",	"armadura"	}, // 5
	{	37,	50,	WEAR_HEAD,	" ",	"casco"		}, // 6
	{	38,	50,	WEAR_LEGS,	"as ",	"polainas"	}, // 7
	{	39,	30,	WEAR_FEET,	"as ",	"botas"		}, // 8
	{	40,	30,	WEAR_HANDS,	"os ",	"guanteletes"	}, // 9
	{	41,	40,	WEAR_ARMS,	"as ",	"mangas"	}, // 10
	{	42,	100,	WEAR_SHIELD,	" ",	"escudo"	}, // 11
	{	43,	40,	WEAR_ABOUT,	"a ",	"toga"		}, // 12
	{	44,	20,	WEAR_WAIST,	" ",	"cinturon"	}, // 13
	{	45,	20,	WEAR_WRIST_L,	" ",	"brazalete"	}, // 14
	{	46,	20,	WEAR_WRIST_R,	" ",	"cinto"		}, // 15
	{	47,	100,	WEAR_WIELD,	" ",	"arma"		}, // 16
	{	47,	100,	WEAR_SECONDARY,	" ",	"arma"		}, // 17
	{	0,	0,	0,		NULL,	NULL		}
};

typedef char *	string;
typedef string	r_w_t[5];
typedef string	armas_type[3];
typedef int	ar_p_type[3];

const r_w_t r_w_t_table [MAX_WEAPON_CLASS] =
{
	{ "divine",	"shbite",	"flbite",	"frbite",	"acbite"	}, // exotic
	{ "slash",	"slice",	"pierce",	"cleave",	"chop"		}, // sword
	{ "pierce",	"stab",		"peck",		"chop",		"scratch"	}, // dagger
	{ "pierce",	"stab",		"chop",		"sting",	"thrust"	}, // spear
	{ "pound",	"blast",	"crush",	"beating",	"charge"	}, // mace
	{ "slash",	"smash",	"crush",	"grep",		"cleave"	}, // axe
	{ "scratch",	"thwack",	"charge",	"pound",	"peck"		}, // flail
	{ "whip",	"blast",	"charge",	"slap",		"peckb"		}, // whip
	{ "blast",	"crush",	"charge",	"beating",	"pound"		} // polearm
};

const armas_type armas_table [MAX_WEAPON_CLASS] =
{
	{ "sierra electrica",	"barra",		"estaca"	},
	{ "espada",		"espada corta",		"espada larga"	},
	{ "daga",		"cortapluma",		"cuchillo"	},
	{ "lanza",		"tridente",		"lanza"		},
	{ "mazo",		"garrote",		"martillo"	},
	{ "hacha",		"machete",		"hacha"		},
	{ "mayal",		"mayal",		"mayal"		},
	{ "latigo",		"latigo",		"latigo"	},
	{ "vara",		"palo",			"palo con clavos" }
};

const armas_type a_pref_table [MAX_WEAPON_CLASS] =
{
	{ "a ",			"a ",			"a "		},
	{ "a ",			"a ",			"a "		},
	{ "a ",			"a ",			" "		},
	{ "a ",			" ",			"a "		},
	{ " ",			" ",			" "		},
	{ " ",			" ",			" "		},
	{ " ",			" ",			" "		},
	{ " ",			" ",			" "		},
	{ "a ",			" ",			" "		}
};

const ar_p_type a_weig_table [MAX_WEAPON_CLASS] =
{
	{ 200,			100,			100		},
	{ 200,			100,			300		},
	{ 80,			50,			50		},
	{ 100,			150,			100		},
	{ 150,			200,			200		},
	{ 150,			100,			150		},
	{ 100,			100,			100		},
	{ 50,			50,			50		},
	{ 100,			100,			110		}
};

sh_int ran_mat_table [] =
{
	MAT_MADERA, MAT_CEMENTO, MAT_ACERO, MAT_HIERRO, MAT_PLATA, MAT_ORO,
	MAT_MARMOL
};

int mat_weig_table [] =
{
	50,	100,	80,	80,	50,	50,
	100
};

int mat_cost_table [] =
{
	50,	50,	100,	100,	150,	200,
	200
};

int afloc[] = {	APPLY_HITROLL,	APPLY_DAMROLL,	APPLY_MANA,	APPLY_HIT,
		APPLY_STR,	APPLY_INT,	APPLY_DEX,	APPLY_CON,
		APPLY_WIS,	APPLY_SAVES	};
int afmod[] = {	5,		5,		100,		50,
		3,		3,		3,		3,
		3,		10		};

void give_random_affect( OBJ_DATA * obj )
{
	AFFECT_DATA af;
	int i;

	for ( i = 0; i < UMAX(1, obj->level / 10); i++ )
		if (CHANCE(30 + obj->level/2))
		{
			int num = number_range(0,9);
			int mod = number_range(-1*afmod[num],afmod[num]);

			af.type		= 0;
			af.where	= TO_AFFECTS;
			af.level	= obj->level;
			af.duration	= -1;
			af.location	= afloc[num];
			af.modifier	= (mod == 0) ? 1 : mod;
			af.bitvector	= 0;

			affect_to_obj(obj, &af);
			obj->enchanted = TRUE;
		}
}

void crear_armadura_random( CHAR_DATA * ch )
{
	int posicion = 16, cnt = 0;
	OBJ_DATA * obj;
	bool found = FALSE;
	int material = number_range(0,6);
	int weight = 0;
	char *nombre, *prefix;
	char buf[MIL];
	int nivel = war ? WAR_LEVEL : getNivelPr(ch);

	if (war == FALSE)
	{
		while(found == FALSE)
		{
			posicion = number_range( 0, 17 );

			cnt++;

			if (cnt > 25)
				return;

			if (r_arm_table[posicion].wear_loc == WEAR_SHIELD
			&&  get_eq_char(ch, WEAR_SECONDARY) != NULL )
				continue;

			if (r_arm_table[posicion].wear_loc == WEAR_SECONDARY
			&& (get_eq_char(ch, WEAR_WIELD) == NULL
			 || get_eq_char(ch, WEAR_SHIELD) != NULL
			 || get_eq_char(ch, WEAR_HOLD) != NULL) )
				continue;

			if (IS_SET(ch->act, ACT_WARRIOR)
			||  IS_SET(ch->act, ACT_THIEF))
			{
				if (get_eq_char(ch, WEAR_WIELD) == NULL)
				{
					posicion = 16;
					break;
				}

				if (get_eq_char(ch, WEAR_SHIELD) == NULL)
				{
					posicion = 11;
					break;
				}

				if (get_eq_char(ch, WEAR_BODY) == NULL)
				{
					posicion = 5;
					break;
				}
			}

			if (get_eq_char(ch, r_arm_table[posicion].wear_loc) == NULL)
				found = TRUE;
		} // while
	} // war

	obj = create_object( get_obj_index(r_arm_table[posicion].vnum), nivel );

	obj->level = nivel;

	switch(obj->item_type)
	{	
		case ITEM_ARMOR:	// nivel 35 : 10
		obj->value[0] = UMAX(1, number_range(nivel * .2, nivel*.4));
		obj->value[1] = UMAX(1, number_range(nivel * .2, nivel*.4));
		obj->value[2] = UMAX(1, number_range(nivel * .2, nivel*.4));
		obj->value[3] = number_range(nivel * .1, nivel*.3);
		obj->value[4] = 0;
		nombre = r_arm_table[posicion].desc;
		prefix = r_arm_table[posicion].prefix;
		weight = r_arm_table[posicion].peso;
		break;

		case ITEM_WEAPON:
		obj->value[0] = number_range(0, MAX_WEAPON_CLASS - 1);
		obj->value[1] = number_fuzzy(UMIN(obj->level / 4 + 2, 5));
		obj->value[2] = number_fuzzy((obj->level + 7) / obj->value[1]);
		obj->value[3] = attack_lookup(r_w_t_table[obj->value[0]][number_range(0,4)]);
		obj->value[4] = 0;
		cnt = number_range(0,2);
		nombre = armas_table[obj->value[0]][cnt];
		prefix = a_pref_table[obj->value[0]][cnt];
		weight = a_weig_table[obj->value[0]][cnt];
		break;

		case ITEM_LIGHT:
		obj->value[2] = number_range(1, obj->level * 200);
		nombre = r_arm_table[posicion].desc;
		prefix = r_arm_table[posicion].prefix;
		weight = number_range(10,50);
		break;

		default:
		bugf( "crear_armadura_random : tipo de obj %d invalido, vnum %d",
			obj->item_type,
			r_arm_table[posicion].vnum );
		nombre = r_arm_table[posicion].desc;
		prefix = r_arm_table[posicion].prefix;
		weight = number_range(10,100);
		break;
	}

	sprintf( buf, "random %s %s", nombre,
		flag_string(mat_table, ran_mat_table[material]) );
	free_string(obj->name);
	obj->name = str_dup(buf);

	sprintf( buf, "un%s%s de %s", prefix,
		nombre, flag_string(mat_table, ran_mat_table[material]) );
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);

	sprintf( buf, "Un%s%s de %s esta en el suelo.",
		prefix,
		nombre,
		flag_string(mat_table, ran_mat_table[material]) );
	free_string(obj->description);
	obj->description = str_dup(buf);

	obj->material = ran_mat_table[material];

	obj->weight = weight + mat_weig_table[material];
	obj->cost = mat_cost_table[material];

	obj->timer = number_range(1,1000);

	if (CHANCE(10 + obj->level*2))
		give_random_affect(obj);

	if (war == FALSE)
	{
		obj_to_char(obj, ch);
		equip_char(ch, obj, r_arm_table[posicion].wear_loc );
	}
	else
	{
		ROOM_INDEX_DATA * room;

		if (ch->was_in_room == NULL)
		{
			bugf("crear_armadura_random : ch %s cuarto NULL",	
				ch->name );
			return;
		}

		room = get_room_index( number_range( ch->was_in_room->area->min_vnum,
				      		     ch->was_in_room->area->max_vnum ) );

		while(room == NULL)
			room = get_room_index(number_range(ch->was_in_room->area->min_vnum,
					    ch->was_in_room->area->max_vnum));

		obj_to_room(obj, room);
	}
}

bool es_obj_random(OBJ_DATA *obj)
{
	if (ENTRE_I(31,obj->pIndexData->vnum,47))
		return TRUE;
	else
		return FALSE;
}
