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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "language.h"

#define L_APP 53

/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light",	ITEM_TAKE		},
    {	ITEM_SCROLL,	"scroll",	ITEM_TAKE		},
    {	ITEM_WAND,	"wand",		ITEM_TAKE|ITEM_HOLD	},
    {   ITEM_STAFF,	"staff",	ITEM_TAKE|ITEM_HOLD	},
    {   ITEM_WEAPON,	"weapon",	ITEM_TAKE|ITEM_WIELD	},
    {   ITEM_TREASURE,	"treasure",	0			},
    {   ITEM_ARMOR,	"armor",	ITEM_TAKE		},
    {	ITEM_POTION,	"potion",	ITEM_TAKE		},
    {	ITEM_CLOTHING,	"clothing",	0			},
    {   ITEM_FURNITURE,	"furniture",	0			},
    {	ITEM_TRASH,	"trash",	0			},
    {	ITEM_CONTAINER,	"container",	0			},
    {	ITEM_DRINK_CON, "drink",	0			},
    {	ITEM_KEY,	"key",		ITEM_TAKE		},
    {	ITEM_FOOD,	"food",		ITEM_TAKE		},
    {	ITEM_MONEY,	"money",	ITEM_TAKE		},
    {	ITEM_BOAT,	"boat",		0			},
    {	ITEM_CORPSE_NPC,"npc_corpse",	0			},
    {	ITEM_CORPSE_PC,	"pc_corpse",	0			},
    {   ITEM_FOUNTAIN,	"fountain",	0			},
    {	ITEM_PILL,	"pill",		ITEM_TAKE		},
    {	ITEM_PROTECT,	"protect",	0			},
    {	ITEM_MAP,	"map",		0			},
    {	ITEM_PORTAL,	"portal",	0			},
    {	ITEM_WARP_STONE,"warp_stone",	ITEM_TAKE		},
    {	ITEM_ROOM_KEY,	"room_key",	ITEM_TAKE		},
    {	ITEM_GEM,	"gem",		ITEM_TAKE		},
    {	ITEM_JEWELRY,	"jewelry",	ITEM_TAKE		},
    {   ITEM_JUKEBOX,	"jukebox",	0			},
    {	ITEM_PEDERNAL,	"pedernal",	ITEM_TAKE		},
    {	ITEM_FUMABLE,	"fumable",	ITEM_TAKE		},
    {   0,		NULL,		0			}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	"a",	"espada",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
   { "mace",	"",	"mazo",		OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
   { "dagger",	"a",	"daga",		OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger	},
   { "axe",	"a",	"hacha",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
   { "staff",	"a",	"vara",		OBJ_VNUM_SCHOOL_STAFF,	WEAPON_SPEAR,	&gsn_spear	},
   { "flail",	"",	"chicote",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
   { "whip",	"",	"latigo",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
   { "polearm",	"a",	"lanza",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
   { NULL,	NULL,	NULL,		0,			0,		NULL		}
};


 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L5 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	"bugs",		WIZ_BUGS,	IM },
   {	"xp",		WIZ_XP,		IM },
   {	"timestamp",	WIZ_TIMESTAMP,	IM },
   {	"plog",		WIZ_PLOG,	L1 },
   {	NULL,		0,		0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    { 	"none",		"golpe",	-1		},  /*  0 */
    {	"slice",	"slice", 	DAM_SLASH	},
    {   "stab",		"punalada",	DAM_PIERCE	},
    {	"slash",	"slash",	DAM_SLASH	},
    {	"whip",		"latigazo",	DAM_SLASH	},
    {   "claw",		"claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	"blast",	DAM_BASH	},
    {   "pound",	"mazaso",	DAM_BASH	},
    {	"crush",	"crush",	DAM_BASH	},
    {   "grep",		"grep",		DAM_SLASH	},
    {	"bite",		"mordida",	DAM_PIERCE	},  /* 10 */
    {   "pierce",	"estocada",	DAM_PIERCE	},
    {   "suction",	"succion",	DAM_BASH	},
    {	"beating",	"beating",	DAM_BASH	},
    {   "digestion",	"digestion",	DAM_ACID	},
    {	"charge",	"carga",	DAM_BASH	},  /* 15 */
    { 	"slap",		"cachetada",	DAM_BASH	},
    {	"punch",	"punetazo",	DAM_BASH	},
    {	"wrath",	"ira",		DAM_ENERGY	},
    {	"magic",	"magia",	DAM_ENERGY	},
    {   "divine",	"poder divino",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"cleave",	DAM_SLASH	},
    {	"scratch",	"aranazo",	DAM_PIERCE	},
    {   "peck",		"peck",		DAM_PIERCE	},
    {   "peckb",	"peck",		DAM_BASH	},
    {   "chop",		"chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	"sting",	DAM_PIERCE	},
    {   "smash",	"smash",	DAM_BASH	},
    {   "shbite",	"shocking bite",DAM_LIGHTNING	},
    {	"flbite",	"flaming bite", DAM_FIRE	},
    {	"frbite",	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acbite",	"mordida acida", DAM_ACID	},
    {	"chomp",	"chomp",	DAM_PIERCE	},
    {  	"drain",	"life drain",	DAM_NEGATIVE	},
    {   "thrust",	"thrust",	DAM_PIERCE	},
    {   "slime",	"slime",	DAM_ACID	},
    {	"shock",	"shock",	DAM_LIGHTNING	},
    {   "thwack",	"thwack",	DAM_BASH	},
    {   "flame",	"llamarada",	DAM_FIRE	},
    {   "chill",	"chill",	DAM_COLD	},
    {   "caida",	"caida",	DAM_BASH	},
    {	"cadenazo",	"cadenazo",	DAM_BASH	},
    {   NULL,		NULL,		0		}
};
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
/*  {
	nombre, who, stat principal, arma inicial,
	vnum guilds, max nivel skills, thac0_00, thac0_32, hpmin, hpmax, fMana,
	grupo base, grupo default, clase remort, multiclase,
	max num clases, noalign, noclase { mag cle thi war psi ran }
    } */
    {
	"mage", "Mag",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  75,  20, 6,  6,  8, TRUE,
	"mage basics", "mage default", FALSE, TRUE,
	2, 0, { FALSE, TRUE, FALSE, FALSE, FALSE, TRUE }
    },

    {
	"cleric", "Cle",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  75,  20, 2,  7, 10, TRUE,
	"cleric basics", "cleric default", FALSE, TRUE,
	2, 0, { TRUE, FALSE, FALSE, FALSE, FALSE, TRUE }
    },

    {
	"thief", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 },  75,  20,  -4,  8, 13, FALSE,
	"thief basics", "thief default", FALSE, TRUE,
	2, 0, { FALSE, FALSE, FALSE, FALSE, FALSE, TRUE }
    },

    {
	"warrior", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  75,  20,  -10,  11, 15, FALSE,
	"warrior basics", "warrior default", FALSE, TRUE,
	2, 0, { FALSE, FALSE, FALSE, FALSE, FALSE, TRUE }
    },

    {
        "psicionist", "Psi", STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
        { 3151, 3151 },  85,  18,  5,   6,  9, TRUE,
        "psi basics", "psi default", FALSE, TRUE,
        2, 0, { FALSE, FALSE, FALSE, FALSE, FALSE, TRUE }
    },
    
    {
    	"ranger", "Ran", STAT_CON,	OBJ_VNUM_SCHOOL_DAGGER,
    	{ 3585, 3585 }, 85, 20, 0, 9, 12, TRUE,
    	"ranger basics", "ranger default", FALSE, FALSE,
    	1, NOALIGN_NEUTRAL | NOALIGN_EVIL, { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE }
    }
};


/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const	struct	con_app_type	con_app		[26]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 }    /* 25 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "pisco",			"clear",	{ 151, 1,  3, 0,  2 }   },
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};

const   struct  lang_type       lang_table      [ MAX_LANGUAGE ] =
{
        { "comun"       },
        { "humano"      },
        { "enano"       },
        { "elfo"        },
        { "gnomo"       },
        { "goblin"      },
        { "orco"        },
        { "ogro"        },
        { "drow"        },
        { "kobold"      },
        { "troll"       },
        { "hobbit"      }
};
