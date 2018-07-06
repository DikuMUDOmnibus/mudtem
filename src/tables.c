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
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merc.h"
#include "events.h"
#include "tables.h"
#include "olc.h"

/* for position */
const struct position_type position_table[] =
{
    {   "dead",                 "dead"  },
    {   "mortally wounded",     "mort"  },
    {   "incapacitated",        "incap" },
    {   "stunned",              "stun"  },
    {   "sleeping",             "sleep" },
    {   "resting",              "rest"  },
    {   "sitting",              "sit"   },
    {   "fighting",             "fight" },
    {   "standing",             "stand" },
    {   "mounted",              "mount" },
    {   NULL,                   NULL    }
};

/* for sex */
const struct sex_type sex_table[] =
{
   {    "none"          },
   {    "male"          },
   {    "female"        },
   {    "either"        },
   {    NULL            }
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {   "tiny"          },
    {   "small"         },
    {   "medium"        },
    {   "large"         },
    {   "huge",         },
    {   "giant"         },
    {   NULL            }
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {   "npc",                  ACT_IS_NPC,     FALSE   },
    {   "sentinel",             ACT_SENTINEL,   TRUE    },
    {   "scavenger",            ACT_SCAVENGER,  TRUE    },
    {	"psi",			ACT_PSI,	TRUE	},
    {   "aggressive",           ACT_AGGRESSIVE, TRUE    },
    {   "stay_area",            ACT_STAY_AREA,  TRUE    },
    {   "wimpy",                ACT_WIMPY,      TRUE    },
    {   "pet",                  ACT_PET,        TRUE    },
    {   "train",                ACT_TRAIN,      TRUE    },
    {   "practice",             ACT_PRACTICE,   TRUE    },
    {   "undead",               ACT_UNDEAD,     TRUE    },
    {	"group",		ACT_GROUP,	TRUE	},
    {   "cleric",               ACT_CLERIC,     TRUE    },
    {   "mage",                 ACT_MAGE,       TRUE    },
    {   "thief",                ACT_THIEF,      TRUE    },
    {   "warrior",              ACT_WARRIOR,    TRUE    },
    {   "noalign",              ACT_NOALIGN,    TRUE    },
    {   "nopurge",              ACT_NOPURGE,    TRUE    },
    {   "outdoors",             ACT_OUTDOORS,   TRUE    },
    {	"prototipo",		ACT_PROTOTIPO,	FALSE	},
    {   "indoors",              ACT_INDOORS,    TRUE    },
    {   "teacher",              ACT_TEACHER,    TRUE    },
    {   "healer",               ACT_IS_HEALER,  TRUE    },
    {   "gain",                 ACT_GAIN,       TRUE    },
    {   "update_always",        ACT_UPDATE_ALWAYS,      TRUE    },
    {   "changer",              ACT_IS_CHANGER, TRUE    },
    {   "banker",               ACT_BANKER,     TRUE    },
    {   NULL,                   0,              FALSE   }
};

const struct flag_type plr_flags[] =
{
    {   "npc",                  PLR_IS_NPC,		FALSE   },
    {	"noauction",		PLR_NOAUCTION,		TRUE	},
    {   "autoassist",           PLR_AUTOASSIST,		FALSE   },
    {   "autoexit",             PLR_AUTOEXIT,		FALSE   },
    {   "autoloot",             PLR_AUTOLOOT,		FALSE   },
    {   "autosac",              PLR_AUTOSAC,		FALSE   },
    {   "autogold",             PLR_AUTOGOLD,		FALSE   },
    {   "autosplit",            PLR_AUTOSPLIT,		FALSE   },
    {	"nosave",		PLR_NOSAVE,		FALSE	},
    {   "holylight",            PLR_HOLYLIGHT,		FALSE   },
    {   "can_loot",             PLR_CANLOOT,		FALSE   },
    {   "nosummon",             PLR_NOSUMMON,		FALSE   },
    {   "nofollow",             PLR_NOFOLLOW,		FALSE   },
    {	"mal_align",		PLR_MAL_ALIGN,		TRUE	},
    {   "permit",               PLR_PERMIT,		TRUE    },
    {	"nopk",			PLR_NOPK,		FALSE	},
    {   "log",                  PLR_LOG,		FALSE   },
    {   "deny",                 PLR_DENY,		FALSE   },
    {   "freeze",               PLR_FREEZE,		FALSE   },
    {   "thief",                PLR_THIEF,		FALSE   },
    {   "killer",               PLR_KILLER,		FALSE   },
    {   "remort",               PLR_REMORT,		FALSE   },
    {   NULL,                   0,			0       }
};

const struct flag_type affect_flags[] =
{
	{	"blind",		AFF_BLIND,		TRUE	},
	{	"invisible",		AFF_INVISIBLE,		TRUE	},
	{	"detect_evil",		AFF_DETECT_EVIL,	TRUE	},
	{	"detect_invis",		AFF_DETECT_INVIS,	TRUE	},
	{	"detect_magic",		AFF_DETECT_MAGIC,	TRUE	},
	{	"detect_hidden",	AFF_DETECT_HIDDEN,	TRUE	},
	{	"detect_good",		AFF_DETECT_GOOD,	TRUE	},
	{	"sanctuary",		AFF_SANCTUARY,		TRUE	},
	{	"faerie_fire",		AFF_FAERIE_FIRE,	TRUE	},
	{	"infrared",		AFF_INFRARED,		TRUE	},
	{	"curse",		AFF_CURSE,		TRUE	},
	{	"unused",		AFF_UNUSED_FLAG,	FALSE	},
	{	"poison",		AFF_POISON,		TRUE	},
	{	"protect_evil",		AFF_PROTECT_EVIL,	TRUE	},
	{	"protect_good",		AFF_PROTECT_GOOD,	TRUE	},
	{	"sneak",		AFF_SNEAK,		TRUE	},
	{	"hide",			AFF_HIDE,		TRUE	},
	{	"sleep",		AFF_SLEEP,		TRUE	},
	{	"charm",		AFF_CHARM,		TRUE	},
	{	"flying",		AFF_FLYING,		TRUE	},
	{	"pass_door",		AFF_PASS_DOOR,		TRUE	},
	{	"haste",		AFF_HASTE,		TRUE	},
	{	"calm",			AFF_CALM,		TRUE	},
	{	"plague",		AFF_PLAGUE,		TRUE	},
	{	"weaken",		AFF_WEAKEN,		TRUE	},
	{	"dark_vision",		AFF_DARK_VISION,	TRUE	},
	{	"berserk",		AFF_BERSERK,		TRUE	},
	{	"swim",			AFF_SWIM,		TRUE	},
	{	"regeneration",		AFF_REGENERATION,	TRUE	},
	{	"slow",			AFF_SLOW,		TRUE	},
	{	"polymorph",		AFF_POLYMORPH,		FALSE	},
	{	NULL,			0,			0	}
};

const   struct  flag_type       affect2_flags   []      =
{
	{	"vampiric_bite",	AFF_VAMP_BITE,		FALSE	},
	{	"flaming_shield",	AFF_FLAMING_SHIELD,	TRUE	},
	{	"ghoul",		AFF_GHOUL,		FALSE	},
	{	"mute",			AFF_MUTE,       	TRUE    },
	{       "amnesia",              AFF_AMNESIA,    	TRUE    },
	{	"hold",			AFF_HOLD,		TRUE	},
	{	"estupidez",		AFF_ESTUPIDEZ,		TRUE	},
	{	"dolor_guata",		AFF_DOLOR_GUATA,	TRUE	},
	{	"respirar_agua",	AFF_RESPIRAR_AGUA,	TRUE	},
	{       NULL,                   0,              	0       }
};

const struct flag_type off_flags[] =
{
    {   "area_attack",          OFF_AREA_ATTACK,	TRUE    },
    {   "backstab",             OFF_BACKSTAB,		TRUE    },
    {   "bash",                 OFF_BASH,		TRUE    },
    {   "berserk",              OFF_BERSERK,		TRUE    },
    {   "disarm",               OFF_DISARM,		TRUE    },
    {   "dodge",                OFF_DODGE,		TRUE    },
    {   "fade",                 OFF_FADE,		TRUE    },
    {   "fast",                 OFF_FAST,		TRUE    },
    {   "kick",                 OFF_KICK,		TRUE    },
    {   "dirt_kick",            OFF_KICK_DIRT,		TRUE    },
    {   "parry",                OFF_PARRY,		TRUE    },
    {   "rescue",               OFF_RESCUE,		TRUE    },
    {   "tail",                 OFF_TAIL,		TRUE    },
    {   "trip",                 OFF_TRIP,		TRUE    },
    {   "crush",                OFF_CRUSH,		TRUE    },
    {   "assist_all",           ASSIST_ALL,		TRUE    },
    {   "assist_align",         ASSIST_ALIGN,		TRUE    },
    {   "assist_race",          ASSIST_RACE,		TRUE    },
    {   "assist_players",       ASSIST_PLAYERS,		TRUE    },
    {   "assist_guard",         ASSIST_GUARD,		TRUE    },
    {   "assist_vnum",          ASSIST_VNUM,		TRUE    },
    {   "circle",               OFF_CIRCLE,		TRUE    },
    {   NULL,                   0,			0       }
};

const struct flag_type imm_flags[] =
{
    {   "summon",               A,      TRUE    },
    {   "charm",                B,      TRUE    },
    {   "magic",                C,      TRUE    },
    {   "weapon",               D,      TRUE    },
    {   "bash",                 E,      TRUE    },
    {   "pierce",               F,      TRUE    },
    {   "slash",                G,      TRUE    },
    {   "fire",                 H,      TRUE    },
    {   "cold",                 I,      TRUE    },
    {   "lightning",            J,      TRUE    },
    {   "acid",                 K,      TRUE    },
    {   "poison",               L,      TRUE    },
    {   "negative",             M,      TRUE    },
    {   "holy",                 N,      TRUE    },
    {   "energy",               O,      TRUE    },
    {   "mental",               P,      TRUE    },
    {   "disease",              Q,      TRUE    },
    {   "drowning",             R,      TRUE    },
    {   "light",                S,      TRUE    },
    {   "sound",                T,      TRUE    },
    {   "wood",                 X,      TRUE    },
    {   "silver",               Y,      TRUE    },
    {   "iron",                 Z,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type form_flags[] =
{
    {   "edible",               FORM_EDIBLE,            TRUE    },
    {   "poison",               FORM_POISON,            TRUE    },
    {   "magical",              FORM_MAGICAL,           TRUE    },
    {   "instant_decay",        FORM_INSTANT_DECAY,     TRUE    },
    {   "other",                FORM_OTHER,             TRUE    },
    {   "animal",               FORM_ANIMAL,            TRUE    },
    {   "sentient",             FORM_SENTIENT,          TRUE    },
    {   "undead",               FORM_UNDEAD,            TRUE    },
    {   "construct",            FORM_CONSTRUCT,         TRUE    },
    {   "mist",                 FORM_MIST,              TRUE    },
    {   "intangible",           FORM_INTANGIBLE,        TRUE    },
    {   "biped",                FORM_BIPED,             TRUE    },
    {   "centaur",              FORM_CENTAUR,           TRUE    },
    {   "insect",               FORM_INSECT,            TRUE    },
    {   "spider",               FORM_SPIDER,            TRUE    },
    {   "crustacean",           FORM_CRUSTACEAN,        TRUE    },
    {   "worm",                 FORM_WORM,              TRUE    },
    {   "blob",                 FORM_BLOB,              TRUE    },
    {   "mammal",               FORM_MAMMAL,            TRUE    },
    {   "bird",                 FORM_BIRD,              TRUE    },
    {   "reptile",              FORM_REPTILE,           TRUE    },
    {   "snake",                FORM_SNAKE,             TRUE    },
    {   "dragon",               FORM_DRAGON,            TRUE    },
    {   "amphibian",            FORM_AMPHIBIAN,         TRUE    },
    {   "fish",                 FORM_FISH ,             TRUE    },
    {   "cold_blood",           FORM_COLD_BLOOD,        TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type part_flags[] =
{
    {   "head",                 PART_HEAD,              TRUE    },
    {   "arms",                 PART_ARMS,              TRUE    },
    {   "legs",                 PART_LEGS,              TRUE    },
    {   "heart",                PART_HEART,             TRUE    },
    {   "brains",               PART_BRAINS,            TRUE    },
    {   "guts",                 PART_GUTS,              TRUE    },
    {   "hands",                PART_HANDS,             TRUE    },
    {   "feet",                 PART_FEET,              TRUE    },
    {   "fingers",              PART_FINGERS,           TRUE    },
    {   "ear",                  PART_EAR,               TRUE    },
    {   "eye",                  PART_EYE,               TRUE    },
    {   "long_tongue",          PART_LONG_TONGUE,       TRUE    },
    {   "eyestalks",            PART_EYESTALKS,         TRUE    },
    {   "tentacles",            PART_TENTACLES,         TRUE    },
    {   "fins",                 PART_FINS,              TRUE    },
    {   "wings",                PART_WINGS,             TRUE    },
    {   "tail",                 PART_TAIL,              TRUE    },
    {   "claws",                PART_CLAWS,             TRUE    },
    {   "fangs",                PART_FANGS,             TRUE    },
    {   "horns",                PART_HORNS,             TRUE    },
    {   "scales",               PART_SCALES,            TRUE    },
    {   "tusks",                PART_TUSKS,             TRUE    },
    {	"agallas",		PART_AGALLAS,		TRUE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type comm_flags[] =
{
    {   "quiet",                COMM_QUIET,             TRUE    },
    {   "deaf",                 COMM_DEAF,              TRUE    },
    {   "nowiz",                COMM_NOWIZ,             TRUE    },
    {   "noclangossip",         COMM_NOAUCTION,         TRUE    },
    {   "nogossip",             COMM_NOGOSSIP,          TRUE    },
    {   "noquestion",           COMM_NOQUESTION,        TRUE    },
    {   "nomusic",              COMM_NOMUSIC,           TRUE    },
    {   "noclan",               COMM_NOCLAN,            TRUE    },
    {   "noquote",              COMM_NOQUOTE,           TRUE    },
    {   "shoutsoff",            COMM_SHOUTSOFF,         TRUE    },
    {	"olc_pausa",		COMM_OLC_PAUSA,		TRUE	},
    {   "compact",              COMM_COMPACT,           TRUE    },
    {   "brief",                COMM_BRIEF,             TRUE    },
    {   "prompt",               COMM_PROMPT,            TRUE    },
    {   "combine",              COMM_COMBINE,           TRUE    },
    {   "telnet_ga",            COMM_TELNET_GA,         TRUE    },
    {   "show_affects",         COMM_SHOW_AFFECTS,      TRUE    },
    {   "nograts",              COMM_NOGRATS,           TRUE    },
    {	"stat",			COMM_STAT,		TRUE	},
    {   "noemote",              COMM_NOEMOTE,           FALSE   },
    {   "noshout",              COMM_NOSHOUT,           FALSE   },
    {   "notell",               COMM_NOTELL,            FALSE   },
    {   "nochannels",           COMM_NOCHANNELS,        FALSE   },
    {	"olcx",			COMM_OLCX,		TRUE	},
    {   "snoop_proof",          COMM_SNOOP_PROOF,       FALSE   },
    {   "afk",                  COMM_AFK,               TRUE    },
    {	"noinfo",		COMM_NOINFO,		TRUE	},
    {	"condicion",		COMM_CONDICION,		TRUE	},
    {	"nospam",		COMM_NOSPAM,		TRUE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type mprog_flags[] =
{
    {   "act",                  TRIG_ACT,               TRUE    },
    {   "bribe",                TRIG_BRIBE,             TRUE    },
    {   "death",                TRIG_DEATH,             TRUE    },
    {   "entry",                TRIG_ENTRY,             TRUE    },
    {   "fight",                TRIG_FIGHT,             TRUE    },
    {   "give",                 TRIG_GIVE,              TRUE    },
    {   "greet",                TRIG_GREET,             TRUE    },
    {   "grall",                TRIG_GRALL,             TRUE    },
    {   "kill",                 TRIG_KILL,              TRUE    },
    {   "hpcnt",                TRIG_HPCNT,             TRUE    },
    {   "random",               TRIG_RANDOM,            TRUE    },
    {   "nspeech",              TRIG_NSPEECH,           TRUE    },
    {   "speech",               TRIG_SPEECH,            TRUE    },
    {   "exit",                 TRIG_EXIT,              TRUE    },
    {   "exall",                TRIG_EXALL,             TRUE    },
    {   "delay",                TRIG_DELAY,             TRUE    },
    {   "surr",                 TRIG_SURR,              TRUE    },
    {	"entryall",		TRIG_ENTRYALL,		TRUE	},
    {   "time",			TRIG_TIME,		TRUE	},
    {	"oneshot",		TRIG_ONESHOT,		TRUE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type area_flags[] =
{
    {   "none",                 AREA_NONE,              FALSE   },
    {   "changed",              AREA_CHANGED,           TRUE    },
    {   "added",                AREA_ADDED,             TRUE    },
    {   "loading",              AREA_LOADING,           FALSE   },
    {   "envy",                 AREA_ENVY,              FALSE   },
    {   "prototipo",            AREA_PROTOTIPO,         FALSE   },
    {	"rom_old",		AREA_ROM_OLD,		FALSE	},
    {	"recalc",		AREA_RECALC,		FALSE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type exit_flags[] =
{
    {   "door",                 EX_ISDOOR,              TRUE    },
    {   "closed",               EX_CLOSED,              TRUE    },
    {   "locked",               EX_LOCKED,              TRUE    },
    {   "pickproof",            EX_PICKPROOF,           TRUE    },
    {   "nopass",               EX_NOPASS,              TRUE    },
    {   "easy",                 EX_EASY,                TRUE    },
    {   "hard",                 EX_HARD,                TRUE    },
    {   "infuriating",          EX_INFURIATING,         TRUE    },
    {   "noclose",              EX_NOCLOSE,             TRUE    },
    {   "nolock",               EX_NOLOCK,              TRUE    },
    {   NULL,                   0,                      0       }
};



const struct flag_type door_resets[] =
{
    {   "open and unlocked",    0,              TRUE    },
    {   "closed and unlocked",  1,              TRUE    },
    {   "closed and locked",    2,              TRUE    },
    {   NULL,                   0,              0       }
};



const struct flag_type room_flags[] =
{
    {   "dark",                 ROOM_DARK,              TRUE    },
    {   "cone_silence",         ROOM_CONE_OF_SILENCE,   TRUE    },
    {   "no_mob",               ROOM_NO_MOB,            TRUE    },
    {   "indoors",              ROOM_INDOORS,           TRUE    },
    {	"flaming",		ROOM_FLAMING,		TRUE	},
    {	"grabado",		ROOM_GRABADO,		TRUE	},
    {	"nomagic",		ROOM_NOMAGIC,		TRUE	},
    {   "private",              ROOM_PRIVATE,           TRUE    },
    {   "safe",                 ROOM_SAFE,              TRUE    },
    {   "solitary",             ROOM_SOLITARY,          TRUE    },
    {   "pet_shop",             ROOM_PET_SHOP,          TRUE    },
    {   "no_recall",            ROOM_NO_RECALL,         TRUE    },
    {   "imp_only",             ROOM_IMP_ONLY,          TRUE    },
    {   "gods_only",            ROOM_GODS_ONLY,         TRUE    },
    {   "heroes_only",          ROOM_HEROES_ONLY,       TRUE    },
    {   "newbies_only",         ROOM_NEWBIES_ONLY,      TRUE    },
    {   "law",                  ROOM_LAW,               TRUE    },
    {   "nowhere",              ROOM_NOWHERE,           TRUE    },
    {   "teleport",             ROOM_TELEPORT,          TRUE    },
    {	"arena",		ROOM_ARENA,		TRUE	},
    {   "prototipo",            ROOM_PROTOTIPO,         FALSE   },
    {	"santuario",		ROOM_SANTUARIO,		TRUE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type sector_flags[] =
{
    {   "inside",       SECT_INSIDE,            TRUE    },
    {   "city",         SECT_CITY,              TRUE    },
    {   "field",        SECT_FIELD,             TRUE    },
    {   "forest",       SECT_FOREST,            TRUE    },
    {   "hills",        SECT_HILLS,             TRUE    },
    {   "mountain",     SECT_MOUNTAIN,          TRUE    },
    {   "swim",         SECT_WATER_SWIM,        TRUE    },
    {   "noswim",       SECT_WATER_NOSWIM,      TRUE    },
    {   "unused",       SECT_UNUSED,            TRUE    },
    {   "air",          SECT_AIR,               TRUE    },
    {   "desert",       SECT_DESERT,            TRUE    },
    {   "hoyo",         SECT_HOYO,              TRUE    },
    {	"underwater",	SECT_UNDERWATER,	TRUE	},
    {   NULL,           0,                      0       }
};

const struct flag_type type_flags[] =
{
    {   "light",                ITEM_LIGHT,             TRUE    },
    {   "scroll",               ITEM_SCROLL,            TRUE    },
    {   "wand",                 ITEM_WAND,              TRUE    },
    {   "staff",                ITEM_STAFF,             TRUE    },
    {   "weapon",               ITEM_WEAPON,            TRUE    },
    {   "treasure",             ITEM_TREASURE,          TRUE    },
    {   "armor",                ITEM_ARMOR,             TRUE    },
    {   "potion",               ITEM_POTION,            TRUE    },
    {	"clothing",		ITEM_CLOTHING,		TRUE	},
    {   "furniture",            ITEM_FURNITURE,         TRUE    },
    {   "trash",                ITEM_TRASH,             TRUE    },
    {   "container",            ITEM_CONTAINER,         TRUE    },
    {   "drinkcontainer",       ITEM_DRINK_CON,         TRUE    },
    {   "key",                  ITEM_KEY,               TRUE    },
    {   "food",                 ITEM_FOOD,              TRUE    },
    {   "money",                ITEM_MONEY,             TRUE    },
    {   "boat",                 ITEM_BOAT,              TRUE    },
    {   "npccorpse",            ITEM_CORPSE_NPC,        TRUE    },
    {   "pccorpse",		ITEM_CORPSE_PC,         FALSE   },
    {   "fountain",             ITEM_FOUNTAIN,          TRUE    },
    {   "pill",                 ITEM_PILL,              TRUE    },
    {   "protect",              ITEM_PROTECT,           TRUE    },
    {   "map",                  ITEM_MAP,               TRUE    },
    {   "portal",               ITEM_PORTAL,            TRUE    },
    {   "warpstone",            ITEM_WARP_STONE,        TRUE    },
    {   "roomkey",              ITEM_ROOM_KEY,          TRUE    },
    {   "gem",                  ITEM_GEM,               TRUE    },
    {   "jewelry",              ITEM_JEWELRY,           TRUE    },
    {   "jukebox",              ITEM_JUKEBOX,           TRUE    },
    {	"pedernal",		ITEM_PEDERNAL,		TRUE	},
    {	"fumable",		ITEM_FUMABLE,		TRUE	},
    {   NULL,                   0,                      0       }
};


const struct flag_type extra_flags[] =
{
    {   "glow",                 ITEM_GLOW,              TRUE    },
    {   "hum",                  ITEM_HUM,               TRUE    },
    {   "dark",                 ITEM_DARK,              TRUE    },
    {   "lock",                 ITEM_LOCK,              TRUE    },
    {   "evil",                 ITEM_EVIL,              TRUE    },
    {   "invis",                ITEM_INVIS,             TRUE    },
    {   "magic",                ITEM_MAGIC,             TRUE    },
    {   "nodrop",               ITEM_NODROP,            TRUE    },
    {   "bless",                ITEM_BLESS,             TRUE    },
    {   "antigood",             ITEM_ANTI_GOOD,         TRUE    },
    {   "antievil",             ITEM_ANTI_EVIL,         TRUE    },
    {   "antineutral",          ITEM_ANTI_NEUTRAL,      TRUE    },
    {   "noremove",             ITEM_NOREMOVE,          TRUE    },
    {   "inventory",            ITEM_INVENTORY,         FALSE	},
    {   "nopurge",              ITEM_NOPURGE,           TRUE    },
    {   "rotdeath",             ITEM_ROT_DEATH,         TRUE    },
    {   "visdeath",             ITEM_VIS_DEATH,         TRUE    },
    {	"flaming",		ITEM_FLAMING,		TRUE	},
    {   "nonmetal",             ITEM_NONMETAL,          TRUE    },
    {   "nolocate",             ITEM_NOLOCATE,          TRUE    },
    {   "meltdrop",             ITEM_MELT_DROP,         TRUE    },
    {   "hadtimer",             ITEM_HAD_TIMER,         FALSE	},
    {   "sellextract",          ITEM_SELL_EXTRACT,      TRUE    },
    {	"atm",			ITEM_ATM,		TRUE	},
    {   "burnproof",            ITEM_BURN_PROOF,        TRUE    },
    {   "nouncurse",            ITEM_NOUNCURSE,         TRUE    },
    {   "hidden",               ITEM_HIDDEN,            TRUE    },
    {   "bladethirst",          ITEM_BLADE_THIRST,      FALSE   },
    {	"vampire_bane",		ITEM_VAMPIRE_BANE,	TRUE	},
    {   "prototipo",            ITEM_PROTOTIPO,         FALSE   },
    {   NULL,                   0,                      0       }
};



const struct flag_type wear_flags[] =
{
    {   "take",                 ITEM_TAKE,              TRUE    },
    {   "finger",               ITEM_WEAR_FINGER,       TRUE    },
    {   "neck",                 ITEM_WEAR_NECK,         TRUE    },
    {   "body",                 ITEM_WEAR_BODY,         TRUE    },
    {   "head",                 ITEM_WEAR_HEAD,         TRUE    },
    {   "legs",                 ITEM_WEAR_LEGS,         TRUE    },
    {   "feet",                 ITEM_WEAR_FEET,         TRUE    },
    {   "hands",                ITEM_WEAR_HANDS,        TRUE    },
    {   "arms",                 ITEM_WEAR_ARMS,         TRUE    },
    {   "shield",               ITEM_WEAR_SHIELD,       TRUE    },
    {   "about",                ITEM_WEAR_ABOUT,        TRUE    },
    {   "waist",                ITEM_WEAR_WAIST,        TRUE    },
    {   "wrist",                ITEM_WEAR_WRIST,        TRUE    },
    {   "wield",                ITEM_WIELD,             TRUE    },
    {   "hold",                 ITEM_HOLD,              TRUE    },
    {   "nosac",                ITEM_NO_SAC,            TRUE    },
    {   "wearfloat",            ITEM_WEAR_FLOAT,        TRUE    },
    {   "lentes",               ITEM_WEAR_LENTES,       TRUE    },
    {	"orejas",		ITEM_WEAR_OREJAS,	TRUE	},
/*    {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
    {   NULL,                   0,                      0       }
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {   "none",                 APPLY_NONE,             TRUE    },
    {   "strength",             APPLY_STR,              TRUE    },
    {   "dexterity",            APPLY_DEX,              TRUE    },
    {   "intelligence",         APPLY_INT,              TRUE    },
    {   "wisdom",               APPLY_WIS,              TRUE    },
    {   "constitution",         APPLY_CON,              TRUE    },
    {   "sex",                  APPLY_SEX,              TRUE    },
    {   "class",                APPLY_CLASS,            TRUE    },
    {   "level",                APPLY_LEVEL,            TRUE    },
    {   "age",                  APPLY_AGE,              TRUE    },
    {   "height",               APPLY_HEIGHT,           TRUE    },
    {   "weight",               APPLY_WEIGHT,           TRUE    },
    {   "mana",                 APPLY_MANA,             TRUE    },
    {   "hp",                   APPLY_HIT,              TRUE    },
    {   "move",                 APPLY_MOVE,             TRUE    },
    {   "gold",                 APPLY_GOLD,             TRUE    },
    {   "experience",           APPLY_EXP,              TRUE    },
    {   "ac",                   APPLY_AC,               TRUE    },
    {   "hitroll",              APPLY_HITROLL,          TRUE    },
    {   "damroll",              APPLY_DAMROLL,          TRUE    },
    {   "saves",                APPLY_SAVES,            TRUE    },
    {   "savingpara",           APPLY_SAVING_PARA,      TRUE    },
    {   "savingrod",            APPLY_SAVING_ROD,       TRUE    },
    {   "savingpetri",          APPLY_SAVING_PETRI,     TRUE    },
    {   "savingbreath",         APPLY_SAVING_BREATH,    TRUE    },
    {   "savingspell",          APPLY_SAVING_SPELL,     TRUE    },
    {   "spellaffect",          APPLY_SPELL_AFFECT,     FALSE   },
    {   NULL,                   0,                      0       }
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {   "in the inventory",     WEAR_NONE,      TRUE    },
    {   "as a light",           WEAR_LIGHT,     TRUE    },
    {   "on the left finger",   WEAR_FINGER_L,  TRUE    },
    {   "on the right finger",  WEAR_FINGER_R,  TRUE    },
    {   "around the neck (1)",  WEAR_NECK_1,    TRUE    },
    {   "around the neck (2)",  WEAR_NECK_2,    TRUE    },
    {   "on the body",          WEAR_BODY,      TRUE    },
    {   "over the head",        WEAR_HEAD,      TRUE    },
    {   "on the legs",          WEAR_LEGS,      TRUE    },
    {   "on the feet",          WEAR_FEET,      TRUE    },
    {   "on the hands",         WEAR_HANDS,     TRUE    },
    {   "on the arms",          WEAR_ARMS,      TRUE    },
    {   "as a shield",          WEAR_SHIELD,    TRUE    },
    {   "about the shoulders",  WEAR_ABOUT,     TRUE    },
    {   "around the waist",     WEAR_WAIST,     TRUE    },
    {   "on the left wrist",    WEAR_WRIST_L,   TRUE    },
    {   "on the right wrist",   WEAR_WRIST_R,   TRUE    },
    {   "wielded",              WEAR_WIELD,     TRUE    },
    {   "held in the hands",    WEAR_HOLD,      TRUE    },
    {   "floating nearby",      WEAR_FLOAT,     TRUE    },
    {   "secondary weapon",     WEAR_SECONDARY, TRUE    },
    {   "como lentes",          WEAR_LENTES,    TRUE    },
    {	"como aros",		WEAR_OREJAS,	TRUE	},
    {   NULL,                   0             , 0       }
};


const struct flag_type wear_loc_flags[] =
{
    {   "none",         WEAR_NONE,      TRUE    },
    {   "light",        WEAR_LIGHT,     TRUE    },
    {   "lfinger",      WEAR_FINGER_L,  TRUE    },
    {   "rfinger",      WEAR_FINGER_R,  TRUE    },
    {   "neck1",        WEAR_NECK_1,    TRUE    },
    {   "neck2",        WEAR_NECK_2,    TRUE    },
    {   "body",         WEAR_BODY,      TRUE    },
    {   "head",         WEAR_HEAD,      TRUE    },
    {   "legs",         WEAR_LEGS,      TRUE    },
    {   "feet",         WEAR_FEET,      TRUE    },
    {   "hands",        WEAR_HANDS,     TRUE    },
    {   "arms",         WEAR_ARMS,      TRUE    },
    {   "shield",       WEAR_SHIELD,    TRUE    },
    {   "about",        WEAR_ABOUT,     TRUE    },
    {   "waist",        WEAR_WAIST,     TRUE    },
    {   "lwrist",       WEAR_WRIST_L,   TRUE    },
    {   "rwrist",       WEAR_WRIST_R,   TRUE    },
    {   "wielded",      WEAR_WIELD,     TRUE    },
    {   "hold",         WEAR_HOLD,      TRUE    },
    {   "floating",     WEAR_FLOAT,     TRUE    },
    {   "secondary",    WEAR_SECONDARY, TRUE    },
    {   "lentes",       WEAR_LENTES,    TRUE    },
    {	"aros",		WEAR_OREJAS,	TRUE	},
    {   NULL,           0,              0       }
};

const struct flag_type container_flags[] =
{
	{	"closeable",	CONT_CLOSEABLE,		TRUE	},
	{	"pickproof",	CONT_PICKPROOF,		TRUE	},
	{	"closed",	CONT_CLOSED,		TRUE	},
	{	"locked",	CONT_LOCKED,		TRUE	},
	{	"puton",	CONT_PUT_ON,		TRUE	},
	{	"poison",	CONT_CUERPO_POISON,	FALSE	},
	{	NULL,		0,			0	}
};

/*****************************************************************************
		      ROM - specific tables:
 ****************************************************************************/

const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type weapon_class[] =
{
    {   "exotic",       WEAPON_EXOTIC,          TRUE    },
    {   "sword",        WEAPON_SWORD,           TRUE    },
    {   "dagger",       WEAPON_DAGGER,          TRUE    },
    {   "spear",        WEAPON_SPEAR,           TRUE    },
    {   "mace",         WEAPON_MACE,            TRUE    },
    {   "axe",          WEAPON_AXE,             TRUE    },
    {   "flail",        WEAPON_FLAIL,           TRUE    },
    {   "whip",         WEAPON_WHIP,            TRUE    },
    {   "polearm",      WEAPON_POLEARM,         TRUE    },
    {   NULL,           0,                      0       }
};


const struct flag_type weapon_type2[] =
{
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",     WEAPON_TWO_HANDS,     TRUE    },
    {   "shocking",      WEAPON_SHOCKING,      TRUE    },
    {   "poison",       WEAPON_POISON,          TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type res_flags[] =
{
    {   "summon",        RES_SUMMON,            TRUE    },
    {   "charm",         RES_CHARM,            TRUE    },
    {   "magic",         RES_MAGIC,            TRUE    },
    {   "weapon",        RES_WEAPON,           TRUE    },
    {   "bash",          RES_BASH,             TRUE    },
    {   "pierce",        RES_PIERCE,           TRUE    },
    {   "slash",         RES_SLASH,            TRUE    },
    {   "fire",          RES_FIRE,             TRUE    },
    {   "cold",          RES_COLD,             TRUE    },
    {   "lightning",     RES_LIGHTNING,        TRUE    },
    {   "acid",          RES_ACID,             TRUE    },
    {   "poison",        RES_POISON,           TRUE    },
    {   "negative",      RES_NEGATIVE,         TRUE    },
    {   "holy",          RES_HOLY,             TRUE    },
    {   "energy",        RES_ENERGY,           TRUE    },
    {   "mental",        RES_MENTAL,           TRUE    },
    {   "disease",       RES_DISEASE,          TRUE    },
    {   "drowning",      RES_DROWNING,         TRUE    },
    {   "light",         RES_LIGHT,            TRUE    },
    {   "sound",        RES_SOUND,              TRUE    },
    {   "wood",         RES_WOOD,               TRUE    },
    {   "silver",       RES_SILVER,             TRUE    },
    {   "iron",         RES_IRON,               TRUE    },
    {   NULL,          0,            0    }
};


const struct flag_type vuln_flags[] =
{
    {   "summon",       VULN_SUMMON,            TRUE    },
    {   "charm",        VULN_CHARM,             TRUE    },
    {   "magic",        VULN_MAGIC,             TRUE    },
    {   "weapon",       VULN_WEAPON,            TRUE    },
    {   "bash",         VULN_BASH,              TRUE    },
    {   "pierce",       VULN_PIERCE,            TRUE    },
    {   "slash",        VULN_SLASH,             TRUE    },
    {   "fire",         VULN_FIRE,              TRUE    },
    {   "cold",         VULN_COLD,              TRUE    },
    {   "lightning",    VULN_LIGHTNING,         TRUE    },
    {   "acid",         VULN_ACID,              TRUE    },
    {   "poison",       VULN_POISON,            TRUE    },
    {   "negative",     VULN_NEGATIVE,          TRUE    },
    {   "holy",         VULN_HOLY,              TRUE    },
    {   "energy",       VULN_ENERGY,            TRUE    },
    {   "mental",       VULN_MENTAL,            TRUE    },
    {   "disease",      VULN_DISEASE,           TRUE    },
    {   "drowning",     VULN_DROWNING,          TRUE    },
    {   "light",        VULN_LIGHT,             TRUE    },
    {   "sound",        VULN_SOUND,             TRUE    },
    {   "wood",         VULN_WOOD,              TRUE    },
    {   "silver",       VULN_SILVER,            TRUE    },
    {   "iron",         VULN_IRON,              TRUE    },
    {   NULL,           0,                      0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",  GATE_NORMAL_EXIT,       TRUE    },
    {   "no_curse",     GATE_NOCURSE,           TRUE    },
    {   "go_with",      GATE_GOWITH,            TRUE    },
    {   "buggy",        GATE_BUGGY,             TRUE    },
    {   "random",       GATE_RANDOM,            TRUE    },
    {   NULL,           0,                      0       }
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",     STAND_AT,       TRUE    },
    {   "stand_on",     STAND_ON,       TRUE    },
    {   "stand_in",     STAND_IN,       TRUE    },
    {   "sit_at",       SIT_AT,         TRUE    },
    {   "sit_on",       SIT_ON,         TRUE    },
    {   "sit_in",       SIT_IN,         TRUE    },
    {   "rest_at",      REST_AT,        TRUE    },
    {   "rest_on",      REST_ON,        TRUE    },
    {   "rest_in",      REST_IN,        TRUE    },
    {   "sleep_at",     SLEEP_AT,       TRUE    },
    {   "sleep_on",     SLEEP_ON,       TRUE    },
    {   "sleep_in",     SLEEP_IN,       TRUE    },
    {   "put_at",       PUT_AT,         TRUE    },
    {   "put_on",       PUT_ON,         TRUE    },
    {   "put_in",       PUT_IN,         TRUE    },
    {   "put_inside",   PUT_INSIDE,     TRUE    },
    {   NULL,           0,              0       }
};

const struct flag_type trap_flags[]=
{
    {   "move",         TRAP_EFF_MOVE,  TRUE    },
    {   "object",       TRAP_EFF_OBJECT,TRUE    },
    {   "room",         TRAP_EFF_ROOM,  TRUE    },
    {   "north",        TRAP_EFF_NORTH, TRUE    },
    {   "east",         TRAP_EFF_EAST,  TRUE    },
    {   "south",        TRAP_EFF_SOUTH, TRUE    },
    {   "west",         TRAP_EFF_WEST,  TRUE    },
    {   "up",           TRAP_EFF_UP,    TRUE    },
    {   "down",         TRAP_EFF_DOWN,  TRUE    },
    {   "open",         TRAP_EFF_OPEN,  TRUE    },
    {   NULL,           0,              0       }
};

const struct flag_type trapd_flags[]=
{
    {	"sleep",	TRAP_DAM_SLEEP,		TRUE	},
    {	"teleport",	TRAP_DAM_TELEPORT,	TRUE	},
    {	"fire",		TRAP_DAM_FIRE,		TRUE	},
    {	"cold",		TRAP_DAM_COLD,		TRUE	},
    {	"acid",		TRAP_DAM_ACID,		TRUE	},
    {	"energy",	TRAP_DAM_ENERGY,	TRUE	},
    {	"blunt",	TRAP_DAM_BLUNT,		TRUE	},
    {	"pierce",	TRAP_DAM_PIERCE,	TRUE	},
    {	"slash",	TRAP_DAM_SLASH,		TRUE	},
    {	NULL,		0,			0	}
};
  
const struct flag_type con_table[]=
{
	{ "PLAYING",		CON_PLAYING,		TRUE	},
	{ "GET_NAME",		CON_GET_NAME,		TRUE	},
	{ "GET_OLD_PASS",	CON_GET_OLD_PASSWORD,	TRUE	},
	{ "CONF_NEW_NAME",	CON_CONFIRM_NEW_NAME,	TRUE	},
	{ "GET_NEW_PASS",	CON_GET_NEW_PASSWORD,	TRUE	},
	{ "CONF_NEW_PASS",	CON_CONFIRM_NEW_PASSWORD,	TRUE	},
	{ "GET_NEW_RACE",	CON_GET_NEW_RACE,	TRUE	},
	{ "GET_NEW_SEX",	CON_GET_NEW_SEX,	TRUE	},
	{ "GET_NEW_CLASS",	CON_GET_NEW_CLASS,	TRUE	},
	{ "GET_ALIGNMENT",	CON_GET_ALIGNMENT,	TRUE	},
	{ "PICK_WEAPON",	CON_PICK_WEAPON,	TRUE	},
	{ "READ_IMOTD",		CON_READ_IMOTD,		TRUE	},
	{ "READ_MOTD",		CON_READ_MOTD,		TRUE	},
	{ "BREAK_CONNECT",	CON_BREAK_CONNECT,	TRUE	},
	{ "BEGIN_REMORT",	CON_BEGIN_REMORT,	TRUE	},
	{ "GET_STATS",		CON_GET_STATS,		TRUE	},
	{ "COPYOVER_REC",	CON_COPYOVER_RECOVER,	TRUE	},
	{ "READ_NMOTD",		CON_READ_NMOTD,		TRUE	},
	{ "GET_NEWANSI",	CON_GET_NEWANSI,	TRUE	},
	{ "CONFIRMAR_RAZA",	CON_CONFIRMAR_RAZA,	TRUE	},
	{ "CONFIRMAR_CLASE",	CON_CONFIRMAR_CLASE,	TRUE	},
	{ NULL,			0,			FALSE	}
};

const   struct  flag_type       apply_types     []      =
{
	{       "affects",      TO_AFFECTS,     TRUE    },
	{       "object",       TO_OBJECT,      TRUE    },
	{       "immune",       TO_IMMUNE,      TRUE    },
	{       "resist",       TO_RESIST,      TRUE    },
	{       "vuln",         TO_VULN,        TRUE    },
	{       "weapon",       TO_WEAPON,      TRUE    },
	{	"parts",	TO_PARTS,	TRUE	},
	{       NULL,           0,              TRUE    }
};

const   struct  bit_type        bitvector_type  []      =
{
	{       affect_flags,   "affect"        },
	{       apply_flags,    "apply"         },
	{       imm_flags,      "imm"           },
	{       res_flags,      "res"           },
	{       vuln_flags,     "vuln"          },
	{       weapon_type2,   "weapon"        },
	{       affect2_flags,  "affect2"       }
};

const	struct	flag_type	mem_types	[]	=
{
	{	"customer",	MEM_CUSTOMER,	TRUE	},
	{	"seller",	MEM_SELLER,	TRUE	},
	{	"hostile",	MEM_HOSTILE,	TRUE	},
	{	"afraid",	MEM_AFRAID,	TRUE	},
	{	"hunting",	MEM_HUNTING,	TRUE	},
	{	"vampire",	MEM_VAMPIRE,	FALSE	},
	{	"quest",	MEM_QUEST,	FALSE	},
	{	"room",		MEM_ROOM,	FALSE	},
	{	"quest_comp",	MEM_QUEST_COMPLETO, FALSE	},
	{	NULL,		0,		TRUE	}
};

const	struct	flag_type	target_table	[]	=
{
	{	"tar_ignore",		TAR_IGNORE,		TRUE	},
	{	"tar_char_offensive",	TAR_CHAR_OFFENSIVE,	TRUE	},
	{	"tar_char_defensive",	TAR_CHAR_DEFENSIVE,	TRUE	},
	{	"tar_char_self",	TAR_CHAR_SELF,		TRUE	},
	{	"tar_obj_inv",		TAR_OBJ_INV,		TRUE	},
	{	"tar_obj_char_def",	TAR_OBJ_CHAR_DEF,	TRUE	},
	{	"tar_obj_char_off",	TAR_OBJ_CHAR_OFF,	TRUE	},
	{	NULL,			0,			TRUE	}
};

const	struct	recval_type	recval_table	[]	=
{
	/*      2d6	    +   10,     AC,     dam			}, */
	{	2,	6,	10,	9,	1,	4,	0	},
	{	2,	7,	21,	8,	1,	5,	0	},
	{	2,	6,	35,	7,	1,	6,	0	},
	{	2,	7,	46,	6,	1,	5,	1	},
	{	2,	6,	60,	5,	1,	6,	1	},
	{	2,	7,	71,	4,	1,	7,	1	},
	{	2,	6,	85,	4,	1,	8,	1	},
	{	2,	7,	96,	3,	1,	7,	2	},
	{	2,	6,	110,	2,	1,	8,	2	},
	{	2,	7,	121,	1,	2,	4,	2	}, /* 10 */
	{	2,	8,	134,	1,	1,	10,	2	},
	{	2,	10,	150,	0,	1,	10,	3	},
	{	2,	10,	170,	-1,	2,	5,	3	},
	{	2,	10,	190,	-1,	1,	12,	3	},
	{	3,	9,	208,	-2,	2,	6,	3	},
	{	3,	9,	233,	-2,	2,	6,	4	},
	{	3,	9,	258,	-3,	3,	4,	4	},
	{	3,	9,	283,	-3,	2,	7,	4	},
	{	3,	9,	308,	-4,	2,	7,	5	},
	{	3,	9,	333,	-4,	2,	8,	5	}, /* 20 */
	{	4,	10,	360,	-5,	4,	4,	5	},
	{	5,	10,	400,	-5,	4,	4,	6	},
	{	5,	10,	450,	-6,	3,	6,	6	},
	{	5,	10,	500,	-6,	2,	10,	6	},
	{	5,	10,	550,	-7,	2,	10,	7	},
	{	5,	10,	600,	-7,	3,	7,	7	},
	{	5,	10,	650,	-8,	5,	4,	7	},
	{	6,	12,	703,	-8,	2,	12,	8	},
	{	6,	12,	778,	-9,	2,	12,	8	},
	{	6,	12,	853,	-9,	4,	6,	8	}, /* 30 */
	{	6,	12,	928,	-10,	6,	4,	9	},
	{	10,	10,	1000,	-10,	4,	7,	9	},
	{	10,	10,	1100,	-11,	7,	4,	10	},
	{	10,	10,	1200,	-11,	5,	6,	10	},
	{	10,	10,	1300,	-11,	6,	5,	11	},
	{	10,	10,	1400,	-12,	4,	8,	11	},
	{	10,	10,	1500,	-12,	8,	4,	12	},
	{	10,	10,	1600,	-13,	16,	2,	12	},
	{	15,	10,	1700,	-13,	17,	2,	13	},
	{	15,	10,	1850,	-13,	6,	6,	13	}, /* 40 */
	{	25,	10,	2000,	-14,	12,	3,	14	},
	{	25,	10,	2250,	-14,	5,	8,	14	},
	{	25,	10,	2500,	-15,	10,	4,	15	},
	{	25,	10,	2750,	-15,	5,	9,	15	},
	{	25,	10,	3000,	-15,	9,	5,	16	},
	{	25,	10,	3250,	-16,	7,	7,	16	},
	{	25,	10,	3500,	-17,	13,	4,	17	},
	{	25,	10,	3750,	-18,	5,	11,	18	},
	{	50,	10,	4000,	-19,	11,	5,	18	},
	{	50,	10,	4500,	-20,	10,	6,	19	}, /* 50 */
	{	50,	10,	5000,	-21,	5,	13,	20	},
	{	50,	10,	5500,	-22,	15,	5,	20	},
	{	50,	10,	6000,	-23,	15,	6,	21	},
	{	50,	10,	6500,	-24,	15,	7,	22	},
	{	50,	10,	7000,	-25,	15,	8,	23	},
	{	50,	10,	7500,	-26,	15,	9,	24	},
	{	50,	10,	8000,	-27,	15,	10,	24	},
	{	50,	10,	8500,	-28,	20,	10,	25	},
	{	50,	10,	9000,	-29,	30,	10,	26	},
	{	50,	10,	9500,	-30,	50,	10,	28	}  /* 60 */
};

const	struct	flag_type	dam_classes	[]	=
{
	{	"dam_bash",	DAM_BASH,	TRUE	},
	{	"dam_pierce",	DAM_PIERCE,	TRUE	},
	{	"dam_slash",	DAM_SLASH,	TRUE	},
	{	"dam_fire",	DAM_FIRE,	TRUE	},
	{	"dam_cold",	DAM_COLD,	TRUE	},
	{	"dam_lightning",DAM_LIGHTNING,	TRUE	},
	{	"dam_acid",	DAM_ACID,	TRUE	},
	{	"dam_poison",	DAM_POISON,	TRUE	},
	{	"dam_negative",	DAM_NEGATIVE,	TRUE	},
	{	"dam_holy",	DAM_HOLY,	TRUE	},
	{	"dam_energy",	DAM_ENERGY,	TRUE	},
	{	"dam_mental",	DAM_MENTAL,	TRUE	},
	{	"dam_disease",	DAM_DISEASE,	TRUE	},
	{	"dam_drowning",	DAM_DROWNING,	TRUE	},
	{	"dam_light",	DAM_LIGHT,	TRUE	},
	{	"dam_other",	DAM_OTHER,	TRUE	},
	{	"dam_harm",	DAM_HARM,	TRUE	},
	{	"dam_charm",	DAM_CHARM,	TRUE	},
	{	"dam_sound",	DAM_SOUND,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	flag_type	log_flags	[]	=
{
	{	"log_normal",	LOG_NORMAL,	TRUE	},
	{	"log_always",	LOG_ALWAYS,	TRUE	},
	{	"log_never",	LOG_NEVER,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	flag_type	show_flags	[]	=
{
	{	"comunicacion",	TYP_CMM,	TRUE	},
	{	"combate",	TYP_CBT,	TRUE	},
	{	"especiales",	TYP_ESP,	TRUE	},
	{	"grupo",	TYP_GRP,	TRUE	},
	{	"objetos",	TYP_OBJ,	TRUE	},
	{	"informacion",	TYP_INF,	TRUE	},
	{	"otros",	TYP_OTH,	TRUE	},
	{	"movimiento",	TYP_MVT,	TRUE	},
	{	"configuracion",TYP_CNF,	TRUE	},
	{	"lenguajes",	TYP_LNG,	TRUE	},
	{	"manejo",	TYP_PLR,	TRUE	},
	{	"olc",		TYP_OLC,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	taxi_dest_type	taxi_dest	[]	=
{
	{	"midgaard",			3001	},
	{	"new thalos",			9506	},
	{	"torre de la brujeria",		15700	},
	{	"hell",				10401	},
	{	"megacity",			8001 	},
	{	"miden'nir",			3505	},
	{	"wyvern",			1701	},
	{	"valley of the elves",		7805	},
	{	"drow city",			5100	},
	{	"yggdrasil",			9900	},
	{	"king castle",			15201	},
	{	"castle reinhold",		8200	},
	{	"temple of white lotus",	10701	},
	{	"goblin stronghold",		29001	},
	{	"gangland",			2101	},
	{	"darathorn's pit",		15400	},
	{	NULL,				0	}
};

const	struct	flag_type	clan_flags	[]	=
{
	{	"independent",		CLAN_INDEP,	TRUE	},
	{	"nogood",		CLAN_NOGOOD,	TRUE	},
	{	"noevil",		CLAN_NOEVIL,	TRUE	},
	{	"noneutral",		CLAN_NONEUTRAL,	TRUE	},
	{	NULL,			0,		TRUE	}
};

const struct flag_type oprog_flags[] =
{
    {	"get",			TRIG_GET,		TRUE	},
    {	"put",			TRIG_PUT,		TRUE	},
    {	"sac",			TRIG_SAC,		TRUE	},
    {	"wear",			TRIG_WEAR,		TRUE	},
    {	"remove",		TRIG_REMOVE,		TRUE	},
    {	"hacer",		TRIG_HACER,		TRUE	},
    {	"delay",		OTRIG_DELAY,		TRUE	},
    {   NULL,                   0,                      0       }
};

const struct flag_type rprog_flags[] =
{
    {	"enter",		RTRIG_ENTER,		TRUE	},
    {	"comm",			RTRIG_COMM,		TRUE	},
    {	"excomm",		RTRIG_EXCOMM,		TRUE	},
    {	"delay",		RTRIG_DELAY,		TRUE	},
    {	"speech",		RTRIG_SPEECH,		TRUE	},
    {	NULL,			0,			TRUE	}
};

const struct flag_type stat_table[] =
{
	{	"str",		STAT_STR,	TRUE	},
	{	"int",		STAT_INT,	TRUE	},
	{	"wis",		STAT_WIS,	TRUE	},
	{	"dex",		STAT_DEX,	TRUE	},
	{	"con",		STAT_CON,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const struct flag_type	petstat_table[] =
{
	{	"indef",	PET_INDEF,	TRUE	},
	{	"aceptada",	PET_ACEPTADA,	TRUE	},
	{	"rechazada",	PET_RECHAZADA,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const struct flag_type	detect_table[] =
{
	{	"curse",	DETECTED_CURSE,	TRUE	},
	{	"evil",		DETECTED_EVIL,	TRUE	},
	{	"sharp",	DETECTED_SHARP,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const struct flag_type	skill_flags[] =
{
	{	"cleric",	SKILL_CLERIC,	TRUE	},
	{	"mage",		SKILL_MAGE,	TRUE	},
	{	"warrior",	SKILL_WARRIOR,	TRUE	},
	{	"thief",	SKILL_THIEF,	TRUE	},
	{	"psi",		SKILL_PSI,	TRUE	},
	{	"ranger",	SKILL_RANGER,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const struct flag_type	noalign_table [] =
{
	{	"good",		NOALIGN_GOOD,		TRUE	},
	{	"neutral",	NOALIGN_NEUTRAL,	TRUE	},
	{	"evil",		NOALIGN_EVIL,		TRUE	},
	{	NULL,		0,			TRUE	}
};

const struct event_type	event_table [] =
{
	{	"event_get_obj",	event_get_obj,		TRUE	},
	{	"event_wield",		event_wield,		TRUE	},
	{	"stc_event",		stc_event,		TRUE	},
	{	"to_room_act_event",	to_room_act_event,	TRUE	},
	{	"room_update_event",	room_update_event,	TRUE	},
	{	"char_update_event",	char_update_event,	FALSE	},
	{	"obj_update_event",	obj_update_event,	TRUE	},
	{	"autosave_event",	autosave_event,		TRUE	},
	{	"mob_fight",		mob_fight,		TRUE	},
	{	"mob_cast",		mob_cast,		TRUE	},
	{	"smart_event",		smart_event,		TRUE	},
	{	"hunt_event",		hunt_event,		TRUE	},
	{	"quest_update",		quest_update,		FALSE	},
	{	"mem_check_event",	mem_check_event,	TRUE	},
	{	"save_plist_event",	save_plist_event,	TRUE	},
	{	"eat_msg",		eat_msg,		TRUE	},
	{	"afilar_arma",		afilar_arma,		TRUE	},
	{	"obj_quemar",		obj_quemar,		TRUE	},
	{	"reboot_event",		reboot_event,		TRUE	},
	{	"recall_event",		recall_event,		TRUE	},
	{	"ent_timer",		ent_timer,		TRUE	},
	{	"ent_stc_event",	ent_stc_event,		TRUE	},
	{	"ent_free_event",	ent_free_event,		TRUE	},
	{	"actremove",		actremove,		TRUE	},
	{	"actset",		actset,			TRUE	},
	{	"room_aggress_event",	room_aggress_event,	TRUE	},
	{	"check_strength",	check_strength,		TRUE	},
	{	"obj_extract_event",	obj_extract_event,	TRUE	},
	{	NULL,			NULL,			FALSE	}
};

char * areaname( void * point )
{
	AREA_DATA * area = *(AREA_DATA **) point;

	return area->name;
}

char * value2str( void * point )
{
extern	char * show_obj_values( OBJ_INDEX_DATA * );

	OBJ_INDEX_DATA * obj = *(OBJ_INDEX_DATA **) point;

	return show_obj_values(obj);
}

char * clan2str( void * point )
{
	sh_int clan = *(sh_int *) point;

	return get_clan_table(clan)->name;
}

char * extradescr2str( void * point )
{
static	char buf[MIL];
	EXTRA_DESCR_DATA *ed = *(EXTRA_DESCR_DATA **) point;

	buf[0] = '\0';
	for ( ; ed; ed = ed->next )
	{
	    strcat( buf, ed->keyword );
	    if ( ed->next )
		strcat( buf, " " );
	}
	buf[67] = '\0'; // para lineas de 80 carac
	return buf;
}

char * progs( void * point )
{
static	char buf[MSL];
	char tmpbuf[MIL];
	int cnt;
	MPROG_LIST * list = *(MPROG_LIST **) point;

	buf[0] = '\0';
	strcat(buf, "Programas:\n\r");

	for (cnt=0; list; list=list->next)
        {
              if (cnt ==0)
                      strcat ( buf, "#UN  Vnum  Trigger Frase     #u\n\r" );

              sprintf(tmpbuf, "%2d %5d %7.7s %s\n\r", cnt,
                     list->vnum, rprog_type_to_name(list->trig_type),
                     list->trig_phrase);
              strcat( buf, tmpbuf );
              cnt++;
        }

	return buf;
}

char * sex2str( void * point )
{
	sh_int sex = *(sh_int *) point;
static	char tmpch[2];

	if ( sex == SEX_FEMALE )
		tmpch[0] = 'F';
	else
	if ( sex == SEX_MALE )
		tmpch[0] = 'M';
	else
		tmpch[0] = 'O';

	return tmpch;
}

char * damtype2str( void * point )
{
	sh_int dtype = *(sh_int *) point;

	return attack_table[dtype].name;
}

char * size2str( void * point )
{
	sh_int siz = *(sh_int *) point;

	return size_table[siz].name;
}

char * ac2str( void * point )
{
	sh_int * ac = (sh_int *) point;
static	char buf[40];

	sprintf(buf,"Pi:%d Ba:%d Sl:%d Ma:%d",
		ac[AC_PIERCE], ac[AC_BASH],
		ac[AC_SLASH], ac[AC_EXOTIC]);

	return buf;
}

char * dice2str( void * point )
{
	sh_int * dado = (sh_int *) point;
static	char buf[30];

	sprintf(buf, "%dd%d+%d", dado[DICE_NUMBER], dado[DICE_TYPE], dado[DICE_BONUS] );

	return buf;
}

char * race2str( void * point )
{
	sh_int raza = *(sh_int *) point;

	return race_table[raza].name;
}

char * pos2str( void * point )
{
	sh_int posic = *(sh_int *) point;

	return position_table[posic].short_name;
}

char * exits2str( void * point )
{
    EXIT_DATA *pexit = *(EXIT_DATA **) point;
static char buf[MSL];
    char word[MIL], reset_state[MIL], tmpbuf[MIL];
    char *state;
    int i, length;

    buf[0] = '\0';

    for ( ; pexit; pexit = pexit->next )
    {
	    sprintf( tmpbuf, "-%-5.5s hacia [%5d] ",
		capitalize(dir_nom[pexit->direccion]),
		pexit->u1.to_room ? pexit->u1.to_room->vnum : 0 );
	    strcat( buf, tmpbuf );

	    if (pexit->key > 0)
	    {
	    	sprintf(tmpbuf, "Llave:%d ", pexit->key);
	    	strcat(buf, tmpbuf);
	    }

	    /*
	     * Format up the exit info.
	     * Capitalize all flags that are not part of the reset info.
	     */
	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    strcat( buf, "Flags: [" );
	    for (; ;)
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    int end;

		    end = strlen(buf) - 1;
		    buf[end] = ']';
		    if (pexit->keyword)
		    	strcat(buf, "(K)");
		    if (pexit->description)
		    	strcat(buf, "(D)");
		    strcat( buf, "\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = UPPER(word[i]);
		}

		strcat( buf, word );
		strcat( buf, " " );
	    }

/*	    if ( pexit->keyword && pexit->keyword[0] != '\0' )
	    {
		sprintf( tmpbuf, "Kwds: [%s]\n\r", pexit->keyword );
		strcat( buf, tmpbuf );
	    }
	    if ( pexit->description && pexit->description[0] != '\0' )
	    {
		sprintf( tmpbuf, "%s", pexit->description );
		strcat( buf, tmpbuf );
	    } */
    }

    return buf;
}

char * spec2str( void * point )
{
	SPEC_FUN * spec = *(SPEC_FUN **) point;

	return spec_name(spec);
}

char * game2str( void * point )
{
	GAME_FUN * game = *(GAME_FUN **) point;

	return game_name(game);
}

char * shop2str( void * point )
{
	SHOP_DATA *pShop = *(SHOP_DATA **) point;
	int iTrade;
static	char buf[MSL];
	char tmpbuf[MIL];

	sprintf( buf,
	  "Ganancia al vender : %d%%\n\r"
	  "Ganancia al comprar: %d%%\n\r"
	  "Horas : %d hasta %d\n\r",
		pShop->profit_buy, pShop->profit_sell,
		pShop->open_hour, pShop->close_hour );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    if ( pShop->buy_type[iTrade] != 0 )
	    {
		if ( iTrade == 0 )
		    strcat( buf, "N Tipo\n\r" );
		sprintf( tmpbuf, "%d %s\n\r", iTrade,
			flag_string( type_flags, pShop->buy_type[iTrade] ) );
		strcat(buf,tmpbuf);
	    }
	}

	return buf;
}

const struct olc_show_table_type redit_olc_show_table [] =
{
	{
		"name",	&xRoom.name, "Nombre:", OLCS_STRING,
		1, 1, 40, 1, 1, NULL
	},
	{
		"vnum",	&xRoom.vnum, "Vnum:", OLCS_SHINT,
		49, 1, 5, 1, 1, NULL
	},
	{
		"area", &xRoom.area, "Area:", OLCS_STRFUNC,
		60, 1, 15, 1, 1, areaname
	},
	{
		"heal", &xRoom.heal_rate, "Heal:", OLCS_SHINT,
		1, 2, 3, 1, 1, NULL
	},
	{
		"mana", &xRoom.mana_rate, "Mana:", OLCS_SHINT,
		10, 2, 3, 1, 1, NULL
	},
	{
		"clan", &xRoom.clan, "Clan:", OLCS_STRFUNC,
		19, 2, 10, 1, 1, clan2str
	},
	{
		"sector", &xRoom.sector_type, "Sector:", OLCS_FLAGSTR_SHINT,
		35, 2, 10, 1, 1, sector_flags
	},
	{
		"roomflags", &xRoom.room_flags, "Flags:", OLCS_FLAGSTR_INT,
		1, 3, 73, 1, 1, room_flags
	},
	{
		"extradesc", &xRoom.extra_descr, "Extra desc:", OLCS_STRFUNC,
		1, 4, 67, 1, 1, extradescr2str
	},
	{
		"desctag", NULL, "Descripcion:", OLCS_TAG,
		1, 5, -1, -1, 1, NULL
	},
	{
		"desc", &xRoom.description, "", OLCS_STRING,
		1, 6, -1, 6, 1, NULL
	},
	{
		"exitstag", NULL, "Salidas:", OLCS_TAG,
		1, 12, -1, -1, 1, NULL
	},
	{
		"exits", &xRoom.exits, "", OLCS_STRFUNC,
		1, 13, -1, -1, 1, exits2str
	},
	// pagina 2
	{
		"owner", &xRoom.owner, "Due~no:", OLCS_STRING,
		1, 1, 10, 1, 2, NULL
	},
	{
		"teleport", &xRoom.tele_dest, "Teleport:", OLCS_SHINT,
		19, 1, 5, 1, 2, NULL
	},
	{
		"progs", &xRoom.mprogs, "", OLCS_STRFUNC,
		1, 2, -1, -1, 2, progs
	},
	{
		NULL, NULL, NULL, 0, 0, 0, 0, 0
	}
};

const struct olc_show_table_type medit_olc_show_table [] =
{
	{
		"name", &xMob.player_name, "Nombre:", OLCS_STRING,
		1, 1, 31, 1, 1, NULL
	},
	{
		"area", &xMob.area, "Area:", OLCS_STRFUNC,
		40, 1, 12, 1, 1, areaname
	},
	{
		"norecalc", &xMob.norecalc, "NoRec  :", OLCS_BOOL,
		58, 1, 1, 1, 1, NULL
	},
	{
		"vnum",	&xMob.vnum, "Vnum :", OLCS_SHINT,
		70, 1, 5, 1, 1, NULL
	},
	{
		"level", &xMob.level, "Nivel :", OLCS_SHINT,
		1, 2, 2, 1, 1, NULL
	},
	{
		"align", &xMob.alignment, "Align:", OLCS_SHINT,
		11, 2, 5, 1, 1, NULL
	},
	{
		"damtype", &xMob.dam_type, "Damtype:", OLCS_STRFUNC,
		37, 2, 12, 1, 1, damtype2str
	},
	{
		"sex", &xMob.sex, "Sexo   :", OLCS_STRFUNC,
		58, 2, 1, 1, 1, sex2str
	},
	{
		"group", &xMob.group, "Grupo:", OLCS_SHINT,
		70, 2, 5, 1, 1, NULL
	},
	{
		"race", &xMob.race, "Raza:", OLCS_STRFUNC,
		1, 3, 10, 1, 1, race2str
	},
	{
		"size", &xMob.size, "Tam:", OLCS_STRFUNC,
		17, 3, 5, 1, 1, size2str
	},
	{
		"material", &xMob.material, "Mat:", OLCS_STRING,
		27, 3, 10, 1, 1, NULL
	},
	{
		"wealth", &xMob.wealth, "$:", OLCS_INT,
		43, 3, 6, 1, 1, NULL
	},
	{
		"start_pos", &xMob.start_pos, "Pos ini:", OLCS_STRFUNC,
		58, 3, 4, 1, 1, pos2str
	},
	{
		"default_pos", &xMob.default_pos, "def:", OLCS_STRFUNC,
		72, 3, 4, 1, 1, pos2str
	},
	{
		"hit", &xMob.hit, "HitD:", OLCS_STRFUNC,
		1, 4, 12, 1, 1, dice2str
	},
	{
		"damage", &xMob.damage, "DamD:", OLCS_STRFUNC,
		22, 4, 12, 1, 1, dice2str
	},
	{
		"mana", &xMob.mana, "ManaD:", OLCS_STRFUNC,
		39, 4, 12, 1, 1, dice2str
	},
	{
		"hitroll", &xMob.hitroll, "Hitr:", OLCS_SHINT,
		71, 4, 2, 1, 1, NULL
	},
	{
		"spec_fun", &xMob.spec_fun, "Spec:", OLCS_STRFUNC,
		1, 5, 15, 1, 1, spec2str
	},
	{
		"game_fun", &xMob.game_fun, "Game:", OLCS_STRFUNC,
		22, 5, 16, 1, 1, game2str
	},
	{
		"ac", &xMob.ac, "AC:", OLCS_STRFUNC,
		39, 5, 38, 1, 1, ac2str
	},
	{
		"act", &xMob.act, "Act :", OLCS_FLAGSTR_INT,
		1, 6, 74, 1, 1, act_flags
	},
	{
		"aff", &xMob.affected_by, "Aff :", OLCS_FLAGSTR_INT,
		1, 7, 74, 1, 1, affect_flags
	},
	{
		"aff2", &xMob.affected2_by, "Aff2:", OLCS_FLAGSTR_INT,
		1, 8, 74, 1, 1, affect2_flags
	},
	{
		"form", &xMob.form, "Form:", OLCS_FLAGSTR_INT,
		1, 9, 74, 1, 1, form_flags
	},
	{
		"parts", &xMob.parts, "Part:", OLCS_FLAGSTR_INT,
		1, 10, 74, 1, 1, part_flags
	},
	{
		"imm", &xMob.imm_flags, "Imm :", OLCS_FLAGSTR_INT,
		1, 11, 74, 1, 1, imm_flags
	},
	{
		"res", &xMob.res_flags, "Res :", OLCS_FLAGSTR_INT,
		1, 12, 74, 1, 1, res_flags
	},
	{
		"vuln", &xMob.vuln_flags, "Vuln:", OLCS_FLAGSTR_INT,
		1, 13, 74, 1, 1, vuln_flags
	},
	{
		"off", &xMob.off_flags, "Off :", OLCS_FLAGSTR_INT,
		1, 14, 74, 1, 1, off_flags
	},
	{
		"short_descr", &xMob.short_descr, "ShDe:", OLCS_STRING,
		1, 15, 74, 1, 1, NULL
	},
	{
		"long_descr", &xMob.long_descr, "LoDe:", OLCS_STRING,
		1, 16, 74, 1, 1, NULL
	},
	// pagina 2
	{
		"shop", &xMob.pShop, "", OLCS_STRFUNC,
		1, 2, 25, 4 + MAX_TRADE, 2, shop2str
	},
	{
		NULL, NULL, NULL, 0,
		0, 0, 0, 0, 0, NULL
	}
};

const struct olc_show_table_type oedit_olc_show_table [] =
{
	{
		"name", &xObj.name, "Nombre:", OLCS_STRING,
		1, 1, 32, 1, 1, NULL
	},
	{
		"area", &xObj.area, "Area:", OLCS_STRFUNC,
		40, 1, 14, 1, 1, areaname
	},
	{
		"vnum", &xObj.vnum, "Vnum :", OLCS_SHINT,
		68, 1, 5, 1, 1, NULL
	},
	{
		"type", &xObj.item_type, "Tipo  :", OLCS_FLAGSTR_SHINT,
		1, 2, 11, 1, 1, type_flags
	},
	{
		"level", &xObj.level, "Nivel:", OLCS_SHINT,
		20, 2, 2, 1, 1, NULL
	},
	{
		"condition", &xObj.condition, "Cond:", OLCS_SHINT,
		29, 2, 3, 1, 1, NULL
	},
	{
		"clan", &xObj.clan, "Clan:", OLCS_STRFUNC,
		40, 2, 10, 1, 1, clan2str
	},
	{
		"weight", &xObj.weight, "Peso:", OLCS_SHINT,
		56, 2, 5, 1, 1, NULL
	},
	{
		"cost",	&xObj.cost, "Costo:", OLCS_INT,
		68, 2, -1, 1, 1, NULL
	},
	{
		"wear", &xObj.wear_flags, "Wear F:", OLCS_FLAGSTR_INT,
		1, 3, 72, 1, 1, wear_flags
	},
	{
		"extra", &xObj.extra_flags, "Extra :", OLCS_FLAGSTR_INT,
		1, 4, 72, 1, 1, extra_flags
	},
	{
		"short_descr", &xObj.short_descr, "ShDesc:", OLCS_STRING,
		1, 5, 72, 1, 1, NULL
	},
	{
		"extra_descr", &xObj.extra_descr, "ExDesc:", OLCS_STRFUNC,
		1, 6, 72, 1, 1, extradescr2str
	},
	{
		"description", &xObj.description, "Desc  :", OLCS_STRING,
		1, 7, 72, 6, 1, NULL
	},
	{
		"value", &xObj, "", OLCS_STRFUNC,
		1, 13, -1, 6, 1, value2str
	},
	{
		NULL, NULL, NULL, 0,
		0, 0, 0, 0, 0, NULL
	}
};

const struct flag_type	mat_table [] =
{
	{	"indef",	MAT_INDEF,		TRUE	},
	{	"metal",	MAT_METAL,		TRUE	},
	{	"madera",	MAT_MADERA,		TRUE	},
	{	"oro",		MAT_ORO,		TRUE	},
	{	"plata",	MAT_PLATA,		TRUE	},
	{	"cobre",	MAT_COBRE,		TRUE	},
	{	"hierro",	MAT_HIERRO,		TRUE	},
	{	"acero",	MAT_ACERO,		TRUE	},
	{	"cemento",	MAT_CEMENTO,		TRUE	},
	{	"marmol",	MAT_MARMOL,		TRUE	},
	{	"mithril",	MAT_MITHRIL,		TRUE	},
	{	"vidrio",	MAT_VIDRIO,		TRUE	},
	{	"papel",	MAT_PAPEL,		TRUE	},
	{	"cristal",	MAT_CRISTAL,		TRUE	},
	{	"comida",	MAT_COMIDA,		TRUE	},
	{	"marfil",	MAT_MARFIL,		TRUE	},
	{	"carne",	MAT_CARNE,		TRUE	},
	{	"cuero",	MAT_CUERO,		TRUE	},
	{	"bronce",	MAT_BRONCE,		TRUE	},
	{	"hielo",	MAT_HIELO,		TRUE	},
	{	"piedra",	MAT_PIEDRA,		TRUE	},
	{	"hueso",	MAT_HUESO,		TRUE	},
	{	"tela",		MAT_TELA,		TRUE	},
	{	"plastico",	MAT_PLASTICO,		TRUE	},
	{	"seda",		MAT_SEDA,		TRUE	},
	{	"lana",		MAT_LANA,		TRUE	},
	{	"terciopelo",	MAT_TERCIOPELO,		TRUE	},
	{	"agua",		MAT_AGUA,		TRUE	},
	{	"liquido",	MAT_LIQUIDO,		TRUE	},
	{	"ebano",	MAT_EBANO,		TRUE	},
	{	"adamantio",	MAT_ADAMANTIO,		TRUE	},
	{	"diamante",	MAT_DIAMANTE,		TRUE	},
	{	"aluminio",	MAT_ALUMINIO,		TRUE	},
	{	"goma",		MAT_GOMA,		TRUE	},
	{	"titanio",	MAT_TITANIO,		TRUE	},
	{	"greda",	MAT_GREDA,		TRUE	},
	{	"cuarzo",	MAT_CUARZO,		TRUE	},
	{	"gema",		MAT_GEMA,		TRUE	},
	{	"perla",	MAT_PERLA,		TRUE	},
	{	"platino",	MAT_PLATINO,		TRUE	},
	{	"uranio",	MAT_URANIO,		TRUE	},
	{	"plutonio",	MAT_PLUTONIO,		TRUE	},
	{	"tierra",	MAT_TIERRA,		TRUE	},
	{	"tabaco",	MAT_TABACO,		TRUE	},
	{	"pan",		MAT_PAN,		TRUE	},
	{	NULL,		0,			0	}
};
