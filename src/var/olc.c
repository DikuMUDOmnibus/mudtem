/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "lookup.h"


/*
 * Local functions.
 */
AREA_DATA *get_area_data args ((int vnum));

bool procesar_comando_olc( CHAR_DATA *, char * argument );

MOB_INDEX_DATA xMob;
OBJ_INDEX_DATA xObj;
static const MOB_INDEX_DATA cMob;
static const OBJ_INDEX_DATA cObj;

const	struct	olc_comm_type	olc_comm_table	[]	=
{
 { "name",	ED_MOBILE,	(void *) &xMob.player_name,	ed_line_string,		NULL		},
 { "short",	ED_MOBILE,	(void *) &xMob.short_descr,	ed_line_string,		NULL		},
 { "long",	ED_MOBILE,	(void *) &xMob.long_descr,	ed_line_string,		NULL		},
 { "material",	ED_MOBILE,	(void *) &xMob.material,	ed_line_string,		NULL		},
 { "desc",	ED_MOBILE,	NULL,				ed_desc_mob,		NULL		},
 { "norecalc",	ED_MOBILE,	(void *) &xMob.norecalc,	ed_bool,		NULL		},
 { "level",	ED_MOBILE,	(void *) &xMob.level,		ed_numero_niv,		NULL		},
 { "align",	ED_MOBILE,	(void *) &xMob.alignment,	ed_numero_align,	NULL		},
 { "group",	ED_MOBILE,	(void *) &xMob.group,		ed_numero_s_pos,	NULL		},
 { "imm",	ED_MOBILE,	(void *) &xMob.imm_flags,	ed_flag_toggle,		imm_flags	},
 { "res",	ED_MOBILE,	(void *) &xMob.res_flags,	ed_flag_toggle,		res_flags	},
 { "vuln",	ED_MOBILE,	(void *) &xMob.vuln_flags,	ed_flag_toggle,		vuln_flags	},
 { "act",	ED_MOBILE,	(void *) &xMob.act,		ed_flag_toggle,		act_flags	},
 { "affect",	ED_MOBILE,	(void *) &xMob.affected_by,	ed_flag_toggle,		affect_flags	},
 { "off",	ED_MOBILE,	(void *) &xMob.off_flags,	ed_flag_toggle,		off_flags	},
 { "form",	ED_MOBILE,	(void *) &xMob.form,		ed_flag_toggle,		form_flags	},
 { "parts",	ED_MOBILE,	(void *) &xMob.parts,		ed_flag_toggle,		part_flags	},
 { "aff2",	ED_MOBILE,	(void *) &xMob.affected2_by,	ed_flag_toggle,		affect2_flags	},
 { "shop",	ED_MOBILE,	(void *) &xMob.pShop,		ed_shop,		NULL		},
 { "repair",	ED_MOBILE,	(void *) &xMob.pRepair,		ed_repair,		NULL		},
 { "create",	ED_MOBILE,	NULL,				ed_new_mob,		NULL		},
 { "spec",	ED_MOBILE,	(void *) &xMob.spec_fun,	ed_gamespec,		NULL		},
 { "game",	ED_MOBILE,	(void *) &xMob.game_fun,	ed_gamespec,		(void *) 1	},
 { "recval",	ED_MOBILE,	(void *) &xMob,			ed_recval,		NULL		},
 { "sex",	ED_MOBILE,	(void *) &xMob.sex,		ed_shintlookup,		sex_lookup	},
 { "size",	ED_MOBILE,	(void *) &xMob.size,		ed_shintlookup,		size_lookup	},
 { "startpos",	ED_MOBILE,	(void *) &xMob.start_pos,	ed_shintlookup,		position_lookup	},
 { "defaultpos",ED_MOBILE,	(void *) &xMob.default_pos,	ed_shintlookup,		position_lookup	},
 { "clan",	ED_MOBILE,	(void *) &xMob.clan,		ed_shintposlookup,	clan_lookup	},
 { "damtype",	ED_MOBILE,	(void *) &xMob.dam_type,	ed_shintposlookup,	attack_lookup	},
 { "race",	ED_MOBILE,	(void *) &xMob,			ed_race,		NULL		},
 { "armor",	ED_MOBILE,	(void *) &xMob,			ed_ac,			NULL		},
 { "hit",	ED_MOBILE,	(void *) &xMob.hit[0],		ed_dice,		NULL		},
 { "mana",	ED_MOBILE,	(void *) &xMob.mana[0],		ed_dice,		NULL		},
 { "dam",	ED_MOBILE,	(void *) &xMob.damage[0],	ed_dice,		NULL		},
 { "hitroll",	ED_MOBILE,	(void *) &xMob.hitroll,		ed_numero_s_pos,	NULL		},
 { "wealth",	ED_MOBILE,	(void *) &xMob.wealth,		ed_numero_l_pos,	NULL		},
 { "addprog",	ED_MOBILE,	(void *) &xMob.mprogs,		ed_addprog,		NULL		},
 { "delprog",	ED_MOBILE,	(void *) &xMob.mprogs,		ed_delprog,		NULL		},
 { "script",	ED_MOBILE,	(void *) &xMob.script,		ed_script,		NULL		},
 { "name",	ED_OBJECT,	(void *) &xObj.name,		ed_line_string,		NULL		},
 { "short",	ED_OBJECT,	(void *) &xObj.short_descr,	ed_line_string,		NULL		},
 { "long",	ED_OBJECT,	(void *) &xObj.description,	ed_line_string,		NULL		},
 { "material",	ED_OBJECT,	(void *) &xObj.material,	ed_line_string,		NULL		},
 { "cost",	ED_OBJECT,	(void *) &xObj.cost,		ed_numero_pos,		NULL		},
 { "level",	ED_OBJECT,	(void *) &xObj.level,		ed_numero_niv,		NULL		},
 { "condition",	ED_OBJECT,	(void *) &xObj.condition,	ed_numero_s_pos,	NULL		},
 { "weight",	ED_OBJECT,	(void *) &xObj.weight,		ed_numero_s_pos,	NULL		},
 { "clan",	ED_OBJECT,	(void *) &xObj.clan,		ed_shintposlookup,	clan_lookup	},
 { "extra",	ED_OBJECT,	(void *) &xObj.extra_flags,	ed_flag_toggle,		extra_flags	},
 { "wear",	ED_OBJECT,	(void *) &xObj.wear_flags,	ed_flag_toggle,		wear_flags	},
 { "ed",	ED_OBJECT,	(void *) &xObj.extra_descr,	ed_ed,			NULL		},
 { "type",	ED_OBJECT,	(void *) &xObj.item_type,	ed_flag_set_sh,		type_flags	},
 { "addprog",	ED_OBJECT,	(void *) &xObj.mprogs,		ed_addprog,		NULL		},
 { "delprog",	ED_OBJECT,	(void *) &xObj.mprogs,		ed_delprog,		NULL		},
 { "addaffect",	ED_OBJECT,	(void *) &xObj,			ed_addaffect,		NULL		},
 { "delaffect",	ED_OBJECT,	(void *) &xObj.affected,	ed_delaffect,		NULL		},
 { "addapply",	ED_OBJECT,	(void *) &xObj,			ed_addapply,		NULL		},
 { "v0",	ED_OBJECT,	NULL,				ed_value,		0		},
 { "v1",	ED_OBJECT,	NULL,				ed_value,		(void *) 1	},
 { "v2",	ED_OBJECT,	NULL,				ed_value,		(void *) 2	},
 { "v3",	ED_OBJECT,	NULL,				ed_value,		(void *) 3	},
 { "v4",	ED_OBJECT,	NULL,				ed_value,		(void *) 4	},
 { "create",	ED_OBJECT,	NULL,				ed_new_obj,		NULL		},
 { "trap",	ED_OBJECT,	(void *) &xObj.trap,		ed_trap,		NULL		},
 { "commands",	0,		NULL,				ed_commands,		NULL		},
 { "?",		0,		NULL,				ed_help,		NULL		},
 { "version",	0,		NULL,				ed_version,		NULL		},
 { NULL,	0,		NULL,				NULL,			NULL		}
};

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool    run_olc_editor (DESCRIPTOR_DATA * d)
{
	switch (d->editor)
	{
		case ED_AREA:
			aedit (d->character, d->incomm);
			break;
		case ED_ROOM:
			redit (d->character, d->incomm);
			break;
		case ED_OBJECT:
			oedit (d->character, d->incomm);
			break;
		case ED_MOBILE:
			medit (d->character, d->incomm);
			break;
		case ED_MPCODE:
			mpedit (d->character, d->incomm);
			break;
		case ED_CLAN:
			cedit (d->character, d->incomm);
			break;
		case ED_RACE:
			raedit (d->character, d->incomm);
			break;
		case ED_SOCIAL:
			sedit (d->character, d->incomm);
			break;
		case ED_SKILL:
			skedit (d->character, d->incomm);
			break;
		case ED_CMD:
			cmdedit (d->character, d->incomm);
			break;
		case ED_GROUP:
			gedit (d->character, d->incomm);
			break;
		case ED_SCRIPT:
			scedit (d->character, d->incomm);
			break;
		case ED_HELP:
			hedit (d->character, d->incomm);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}



char   *olc_ed_name (CHAR_DATA * ch)
{
	static char buf[10];

	buf[0] = '\0';
	switch (ch->desc->editor)
	{
		case ED_AREA:
			sprintf (buf, "AEdit");
			break;
		case ED_ROOM:
			sprintf (buf, "REdit");
			break;
		case ED_OBJECT:
			sprintf (buf, "OEdit");
			break;
		case ED_MOBILE:
			sprintf (buf, "MEdit");
			break;
		case ED_MPCODE:
			sprintf (buf, "MPEdit");
			break;
		case ED_CLAN:
			sprintf (buf, "CEdit");
			break;
		case ED_RACE:
			sprintf (buf, "RAEdit");
			break;
		case ED_SOCIAL:
			sprintf (buf, "SEdit");
			break;
		case ED_SKILL:
			sprintf (buf, "SKEdit");
			break;
		case ED_CMD:
			sprintf (buf, "CMDEdit");
			break;
		case ED_GROUP:
			sprintf (buf, "GEdit");
			break;
		case ED_SCRIPT:
			sprintf (buf, "SCEdit");
			break;
		case ED_HELP:
			sprintf (buf, "HEdit");
			break;
		default:
			sprintf (buf, " ");
			break;
	}
	return buf;
}



char   *olc_ed_vnum (CHAR_DATA * ch)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	OBJ_INDEX_DATA *pObj;
	MOB_INDEX_DATA *pMob;
	MPROG_CODE *pMcode;
	HELP_DATA *pHelp;
	struct clan_type *pClan;
	struct race_type *pRace;
	struct social_type *pSocial;
	struct skill_type *pSkill;
	struct cmd_type *pCmd;
	struct group_type *pGrp;
	static char buf[10];

	buf[0] = '\0';
	switch (ch->desc->editor)
	{
		case ED_AREA:
			pArea = (AREA_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pArea ? pArea->vnum : 0);
			break;
		case ED_ROOM:
			pRoom = ch->in_room;
			sprintf (buf, "%d", pRoom ? pRoom->vnum : 0);
			break;
		case ED_OBJECT:
			pObj = (OBJ_INDEX_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pObj ? pObj->vnum : 0);
			break;
		case ED_MOBILE:
			pMob = (MOB_INDEX_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pMob ? pMob->vnum : 0);
			break;
		case ED_MPCODE:
			pMcode = (MPROG_CODE *) ch->desc->pEdit;
			sprintf (buf, "%d", pMcode ? pMcode->vnum : 0);
			break;
		case ED_CLAN:
			pClan = (struct clan_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pClan ? pClan->name : "");
			break;
		case ED_RACE:
			pRace = (struct race_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pRace ? pRace->name : "");
			break;
		case ED_SOCIAL:
			pSocial = (struct social_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pSocial ? pSocial->name : "");
			break;
		case ED_SKILL:
			pSkill = (struct skill_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pSkill ? pSkill->name : "" );
			break;
		case ED_CMD:
			pCmd = (struct cmd_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pCmd ? pCmd->name : "" );
			break;
		case ED_GROUP:
			pGrp = (struct group_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pGrp ? pGrp->name : "" );
			break;
		case ED_SCRIPT:
			buf[0] = '\0';
			break;
		case ED_HELP:
			pHelp = (HELP_DATA *) ch->desc->pEdit;
			sprintf (buf, "%s", pHelp ? pHelp->keyword : "" );
			break;
		default:
			sprintf (buf, " ");
			break;
	}

	return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
/* void    show_olc_cmds (CHAR_DATA * ch, const struct olc_cmd_type *olc_table) */
void show_olc_cmds( CHAR_DATA *ch )
{
	char    buf[MAX_STRING_LENGTH];
	char    buf1[MAX_STRING_LENGTH];
	int     cmd;
	int     col;

	buf1[0] = '\0';
	col = 0;
/*	for (cmd = 0; olc_table[cmd].name != NULL; cmd++) */
	for (cmd = 0; olc_comm_table[cmd].nombre != NULL; cmd++)
	{
		if (olc_comm_table[cmd].editor == 0
		||  olc_comm_table[cmd].editor == ch->desc->editor )
		{
			sprintf (buf, "%-15.15s", olc_comm_table[cmd].nombre);
			strcat (buf1, buf);
			if (++col % 5 == 0)
				strcat (buf1, "\n\r");
		}
	}

	if (col % 5 != 0)
		strcat (buf1, "\n\r");

	send_to_char (buf1, ch);
	return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool    show_commands (CHAR_DATA * ch, char *argument)
{
	show_olc_cmds(ch);
/*	switch (ch->desc->editor)
	{
		case ED_AREA:
			show_olc_cmds (ch, aedit_table);
			break;
		case ED_ROOM:
			show_olc_cmds (ch, redit_table);
			break;
		case ED_OBJECT:
			show_olc_cmds (ch, oedit_table);
			break;
		case ED_MOBILE:
			show_olc_cmds (ch, medit_table);
			break;
		case ED_MPCODE:
			show_olc_cmds (ch, mpedit_table);
			break;
		case ED_CLAN:
			show_olc_cmds (ch, cedit_table);
			break;
		case ED_RACE:
			show_olc_cmds (ch, raedit_table);
			break;
		case ED_SOCIAL:
			show_olc_cmds (ch, sedit_table);
			break;
		case ED_SKILL:
			show_olc_cmds (ch, skedit_table);
			break;
		case ED_CMD:
			show_olc_cmds (ch, cmdedit_table);
			break;
		case ED_GROUP:
			show_olc_cmds (ch, gedit_table);
			break;
		case ED_SCRIPT:
			show_olc_cmds (ch, scedit_table);
			break;
		case ED_HELP:
			show_olc_cmds (ch, hedit_table);
			break;
	} */

	return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
/* {	command		function	}, */

   {	"age", 		aedit_age	},
   {	"builder", 	aedit_builder	},	/* s removed -- Hugin */
   {	"commands", 	show_commands	},
   {	"create", 	aedit_create	},
   {	"filename", 	aedit_file	},
   {	"name", 	aedit_name	},
/* {	"recall",	aedit_recall    },   ROM OLC */
   {	"reset", 	aedit_reset	},
   {	"security", 	aedit_security	},
   {	"show", 	aedit_show	},
   {	"vnum", 	aedit_vnum	},
   {	"lvnum", 	aedit_lvnum	},
   {	"uvnum", 	aedit_uvnum	},
   {	"credits", 	aedit_credits	},
   {	"lowrange", 	aedit_lowrange	},
   {	"highrange", 	aedit_highrange	},

   {	"?",		show_help	},
   {	"version", 	show_version	},

   {	NULL, 		0		}
};



const struct olc_cmd_type redit_table[] =
{
/* {	command		function	}, */

   {	"commands", 	show_commands	},
   {	"create", 	redit_create	},
   {	"desc", 	redit_desc	},
   {	"ed", 		redit_ed	},
   {	"format", 	redit_format	},
   {	"name", 	redit_name	},
   {	"show", 	redit_show	},
   {	"heal", 	redit_heal	},
   {	"mana", 	redit_mana	},
   {	"clan", 	redit_clan	},

   {	"north", 	redit_north	},
   {	"south", 	redit_south	},
   {	"east", 	redit_east	},
   {	"west", 	redit_west	},
   {	"up",	 	redit_up	},
   {	"down", 	redit_down	},
   {	"dentro", 	redit_inside	},
   {	"fuera", 	redit_outside	},

	/* New reset commands. */
   {	"mreset", 	redit_mreset	},
   {	"oreset", 	redit_oreset	},
   {	"mlist", 	redit_mlist	},
   {	"rlist", 	redit_rlist	},
   {	"olist", 	redit_olist	},
   {	"mshow", 	redit_mshow	},
   {	"oshow", 	redit_oshow	},
   {	"teleport", 	redit_teleport	},
   {	"owner", 	redit_owner	},
   {	"addrprog",	redit_addrprog	},
   {	"delrprog",	redit_delrprog	},
   {	"listreset",	redit_listreset },
   {	"checkobj",	redit_checkobj	},
   {	"checkmob",	redit_checkmob	},
   {	"copy",		redit_copy	},

   {	"?",	 	show_help	},
   {	"version", 	show_version	},

   {	NULL,	 	0		}
};



const struct olc_cmd_type oedit_table[] =
{
/* {	command		function	}, */

   {	"addaffect",	oedit_addaffect	}, /**/
   {	"addapply",	oedit_addapply	}, /**/
   {	"commands",	show_commands	}, /**/
   {	"cost",		oedit_cost	}, /**/
   {	"create",	oedit_create	}, /**/
   {	"delaffect",	oedit_delaffect	}, /**/
   {	"ed",		oedit_ed	}, /**/
   {	"long",		oedit_long	}, /**/
   {	"name",		oedit_name	}, /**/
   {	"short",	oedit_short	}, /**/
   {	"show",		oedit_show	},
   {	"v0",		oedit_value0	}, /**/
   {	"v1",		oedit_value1	}, /**/
   {	"v2",		oedit_value2	}, /**/
   {	"v3",		oedit_value3	}, /**/
   {	"v4",		oedit_value4	}, /**/
   {	"weight",	oedit_weight	}, /**/

   {	"extra",	oedit_extra	}, /**/
   {	"wear",		oedit_wear	}, /**/
   {	"type",		oedit_type	}, /**/
   {	"material",	oedit_material	}, /**/
   {	"level",	oedit_level	}, /**/
   {	"condition",	oedit_condition	}, /**/
   {	"trapset",	oedit_trapset	}, /**/
   {	"trapremove",	oedit_trapremove}, /**/
   {	"trapdam",	oedit_trapdam	}, /**/
   {	"trapeff",	oedit_trapeff	}, /**/
   {	"charges",	oedit_charges	}, /**/
   {	"clan",		oedit_clan	}, /**/
   {	"addoprog",	oedit_addoprog	}, /**/
   {	"deloprog",	oedit_deloprog	}, /**/

   {	"?",		show_help	}, /**/
   {	"version",	show_version	}, /**/

   {NULL, 0,}
};



const struct olc_cmd_type medit_table[] =
{
/* {	command		function    	}, */

   {	"alignment",	medit_align	}, /**/
   {	"commands",	show_commands	}, /**/
   {	"create",	medit_create	}, /**/
   {	"desc",		medit_desc	}, /**/
   {	"level",	medit_level	}, /**/
   {	"long",		medit_long	}, /**/
   {	"name",		medit_name	}, /**/
   {	"shop",		medit_shop	}, /**/
   {	"repair",	medit_repair	}, /**/
   {	"short",	medit_short	}, /**/
   {	"show",		medit_show	},
   {	"spec",		medit_spec	}, /**/
   {	"game",		medit_game	}, /**/

   {	"sex",		medit_sex	}, /**/
   {	"act",		medit_act	}, /**/
   {	"affect",	medit_affect	}, /**/
   {	"affect2",	medit_affect2	}, /**/
   {	"armor",	medit_ac	}, /**/
   {	"form",		medit_form	}, /**/
   {	"part",		medit_part	}, /**/
   {	"imm",		medit_imm	}, /**/
   {	"res",		medit_res	}, /**/
   {	"vuln",		medit_vuln	}, /**/
   {	"material",	medit_material	}, /**/
   {	"off",		medit_off	}, /**/
   {	"size",		medit_size	}, /**/
   {	"hitdice",	medit_hitdice	}, /**/
   {	"manadice",	medit_manadice	}, /**/
   {	"damdice",	medit_damdice	}, /**/
   {	"race",		medit_race	}, /**/
   {	"position",	medit_position	}, /**/
   {	"wealth",	medit_gold	}, /**/
   {	"hitroll",	medit_hitroll	}, /**/
   {	"addmprog",	medit_addmprog	}, /**/
   {	"delmprog",	medit_delmprog	}, /**/
   {	"damtype",	medit_damtype	}, /**/
   {	"group",	medit_group	}, /**/
   {	"clan",		medit_clan	}, /**/
   {	"recval",	medit_recval	}, /**/
   {	"script",	medit_script	}, /**/
   {	"norecalc",	medit_norecalc	}, /**/

   {	"?",		show_help	}, /**/
   {	"version",	show_version	},

   {	NULL, 		0		}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data (int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		if (pArea->vnum == vnum)
			return pArea;
	}

	return 0;
}



/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool    edit_done (CHAR_DATA * ch)
{
	ch->desc->pEdit = NULL;
	ch->desc->editor = 0;
	return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void    aedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	char    command[MAX_INPUT_LENGTH];
	char    arg[MAX_INPUT_LENGTH];
	int     cmd;
	int     value;

	EDIT_AREA (ch, pArea);
	smash_tilde (argument);
	strcpy (arg, argument);
	argument = one_argument (argument, command);

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("AEdit:  Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (command, "done"))
	{
		edit_done (ch);
		return;
	}

	if (command[0] == '\0')
	{
		aedit_show (ch, argument);
		return;
	}

	if ((value = flag_value (area_flags, command)) != NO_FLAG)
	{
		TOGGLE_BIT (pArea->area_flags, value);

		send_to_char ("Flag toggled.\n\r", ch);
		return;
	}

	/* Search Table and Dispatch Command. */
	for (cmd = 0; aedit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix (command, aedit_table[cmd].name))
		{
			if ((*aedit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				return;
			}
			else
				return;
		}
	}

	/* Default to Standard Interpreter. */
	interpret (ch, arg);
	return;
}



/* Room Interpreter, called by do_redit. */
void    redit (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	char    arg[MAX_STRING_LENGTH];
	char    command[MAX_INPUT_LENGTH];
	int     cmd;
	int     value;

	EDIT_ROOM (ch, pRoom);
	pArea = pRoom->area;

	smash_tilde (argument);
	strcpy (arg, argument);
	argument = one_argument (argument, command);

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("REdit:  Insufficient security to modify room.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (command, "done"))
	{
		edit_done (ch);
		return;
	}

	if (command[0] == '\0')
	{
		redit_show (ch, argument);
		return;
	}

	if ((value = flag_value (room_flags, command)) != NO_FLAG)
	{
		TOGGLE_BIT (pRoom->room_flags, value);

		SET_BIT (pArea->area_flags, AREA_CHANGED);
		send_to_char ("Room flag toggled.\n\r", ch);
		return;
	}

	if ((value = flag_value (sector_flags, command)) != NO_FLAG)
	{
		pRoom->sector_type = value;

		SET_BIT (pArea->area_flags, AREA_CHANGED);
		send_to_char ("Sector type set.\n\r", ch);
		return;
	}

	/* Search Table and Dispatch Command. */
	for (cmd = 0; redit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix (command, redit_table[cmd].name))
		{
			if ((*redit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				return;
			}
			else
				return;
		}
	}

	/* Default to Standard Interpreter. */
	interpret (ch, arg);
	return;
}

/* Object Interpreter, called by do_oedit. */
void    oedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	OBJ_INDEX_DATA *pObj;

	smash_tilde (argument);

	EDIT_OBJ (ch, pObj);
	pArea = pObj->area;

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("OEdit: Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if (emptystring(argument))
	{
		oedit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if ( !procesar_comando_olc(ch, argument) )
		interpret(ch, argument);

	return;
}

/* Mobile Interpreter, called by do_medit. */
void    medit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	MOB_INDEX_DATA *pMob;

	EDIT_MOB (ch, pMob);
	pArea = pMob->area;

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("MEdit: Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if (emptystring(argument))
	{
		medit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if (!procesar_comando_olc(ch, argument))
		interpret (ch, argument);

	return;
}




const struct editor_cmd_type editor_table[] =
{
/* {	command		function	}, */

   {	"area",		do_aedit	},
   {	"room",		do_redit	},
   {	"object",	do_oedit	},
   {	"mobile",	do_medit	},
   {	"mp",		do_mpedit	},
   {	"clan",		do_cedit	},
   {	"race",		do_raedit	},
   {	"social",	do_sedit	},
   {	"skill",	do_skedit	},
   {	"command",	do_cmdedit	},
   {	"group",	do_gedit	},
   {	"script",	do_scedit	},
   {	"help",		do_hedit	},

   {	NULL,		0		}
};


/* Entry point for all editors. */
void    do_olc (CHAR_DATA * ch, char *argument)
{
	char    command[MAX_INPUT_LENGTH];
	int     cmd;

	argument = one_argument (argument, command);

	if (command[0] == '\0')
	{
		do_help (ch, "olc");
		return;
	}

	/* Search Table and Dispatch Command. */
	for (cmd = 0; editor_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix (command, editor_table[cmd].name))
		{
			(*editor_table[cmd].do_fun) (ch, argument);
			return;
		}
	}

	/* Invalid command, send help. */
	do_help (ch, "olc");
	return;
}



/* Entry point for editing area_data. */
void    do_aedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	int     value;
	char    arg[MAX_STRING_LENGTH];

	pArea = ch->in_room->area;

	argument = one_argument (argument, arg);
	if (is_number (arg))
	{
		value = atoi (arg);
		if (!(pArea = get_area_data (value)))
		{
			send_to_char ("That area vnum does not exist.\n\r", ch);
			return;
		}
	}
	else if (!str_cmp (arg, "create"))
	{
		if (!aedit_create (ch, argument))
			return;
		else
			pArea = area_last;
	}

	if (!IS_BUILDER (ch, pArea) || ch->pcdata->security < 9)
	{
		send_to_char ("Insuficiente seguridad para editar areas.\n\r", ch);
		return;
	}

	ch->desc->pEdit = (void *) pArea;
	ch->desc->editor = ED_AREA;
	return;
}



/* Entry point for editing room_index_data. */
void    do_redit (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *pRoom, *pRoom2;
	char    arg1[MAX_STRING_LENGTH];

	if ( !IS_IMMORTAL(ch) && difftime(current_time, ch->lastfight) < MINUTOS(1) )
	{
		send_to_char( "Espera algunos segundos y vuelve a intentarlo.\n\r", ch );
		return;
	}

	argument = one_argument (argument, arg1);

	pRoom = ch->in_room;

	if (!str_cmp (arg1, "reset"))
	{
		if (!IS_BUILDER (ch, pRoom->area))
		{
			send_to_char ("Insuficiente seguridad para modificar cuartos.\n\r", ch);
			return;
		}

		reset_room (pRoom);
		send_to_char ("Room reset.\n\r", ch);
		return;
	}
	else if (!str_cmp (arg1, "create"))
	{
		if (argument[0] == '\0' || atoi (argument) == 0)
		{
			send_to_char ("Syntax:  edit room create [vnum]\n\r", ch);
			return;
		}

		if (redit_create (ch, argument))
		{
			char_from_room (ch);
			char_to_room (ch, ch->desc->pEdit);
			SET_BIT (pRoom->area->area_flags, AREA_CHANGED);
			pRoom = ch->in_room;
		}
	}
	else
	{
		pRoom2 = get_room_index (atoi (arg1));

		if ((pRoom2 != NULL) && IS_BUILDER (ch, pRoom2->area))
		{
			char_from_room (ch);
			char_to_room (ch, pRoom2);
			pRoom = ch->in_room;
		}
		else if (atoi (arg1) != 0)
		{
			send_to_char ("Insuficiente seguridad para editar cuarto, o cuarto inexistente.\n\r", ch);
			return;
		}
	}

	if (!IS_BUILDER (ch, pRoom->area))
	{
		send_to_char ("Insuficiente seguridad para modificar cuartos.\n\r", ch);
		return;
	}

	ch->desc->editor = ED_ROOM;
	return;
}



/* Entry point for editing obj_index_data. */
void    do_oedit (CHAR_DATA * ch, char *argument)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	char    arg1[MAX_STRING_LENGTH];
	int     value;

	if (IS_NPC (ch))
		return;

	argument = one_argument (argument, arg1);

	if (is_number (arg1))
	{
		value = atoi (arg1);
		if (!(pObj = get_obj_index (value)))
		{
			send_to_char ("OEdit:  That vnum does not exist.\n\r", ch);
			return;
		}

		if (!IS_BUILDER (ch, pObj->area))
		{
			send_to_char ("Insuficiente seguridad para modificar objetos.\n\r", ch);
			return;
		}

		ch->desc->pEdit = (void *) pObj;
		ch->desc->editor = ED_OBJECT;
		return;
	}
	else
	{
		if (!str_cmp (arg1, "create"))
		{
			value = atoi (argument);
			if (argument[0] == '\0' || value == 0)
			{
				send_to_char ("Syntax:  edit object create [vnum]\n\r", ch);
				return;
			}

			pArea = get_vnum_area (value);

			if (!pArea)
			{
				send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
				return;
			}

			if (!IS_BUILDER (ch, pArea))
			{
				send_to_char ("Insuficiente seguridad para modificar objetos.\n\r", ch);
				return;
			}

			if (oedit_create (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				ch->desc->editor = ED_OBJECT;
			}
			return;
		}
	}

	send_to_char ("OEdit:  There is no default object to edit.\n\r", ch);
	return;
}



/* Entry point for editing mob_index_data. */
void    do_medit (CHAR_DATA * ch, char *argument)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int     value;
	char    arg1[MAX_STRING_LENGTH];

	argument = one_argument (argument, arg1);

	if (is_number (arg1))
	{
		value = atoi (arg1);
		if (!(pMob = get_mob_index (value)))
		{
			send_to_char ("MEdit:  That vnum does not exist.\n\r", ch);
			return;
		}

		if (!IS_BUILDER (ch, pMob->area))
		{
			send_to_char ("Insuficiente seguridad para modificar mobs.\n\r", ch);
			return;
		}

		ch->desc->pEdit = (void *) pMob;
		ch->desc->editor = ED_MOBILE;
		return;
	}
	else
	{
		if (!str_cmp (arg1, "create"))
		{
			value = atoi (argument);
			if (arg1[0] == '\0' || value == 0)
			{
				send_to_char ("Syntax:  edit mobile create [vnum]\n\r", ch);
				return;
			}

			pArea = get_vnum_area (value);

			if (!pArea)
			{
				send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
				return;
			}

			if (!IS_BUILDER (ch, pArea))
			{
				send_to_char ("Insuficiente seguridad para modificar mobs.\n\r", ch);
				return;
			}

			if (medit_create (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				ch->desc->editor = ED_MOBILE;
			}
			return;
		}
	}

	send_to_char ("MEdit:  There is no default mobile to edit.\n\r", ch);
	return;
}



void    display_resets (CHAR_DATA * ch)
{
	ROOM_INDEX_DATA *pRoom;
	RESET_DATA *pReset;
	MOB_INDEX_DATA *pMob = NULL;
	char    buf[MAX_STRING_LENGTH];
	char    final[MAX_STRING_LENGTH];
	int     iReset = 0;

	EDIT_ROOM (ch, pRoom);
	final[0] = '\0';

	send_to_char (
	   " No.  Loads    Description       Location         Vnum   Mx Mn Description"
	   "\n\r"
	   "==== ======== ============= =================== ======== ===== ==========="
	   "\n\r", ch);

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
	{
		OBJ_INDEX_DATA *pObj;
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_INDEX_DATA *pObjToIndex;
		ROOM_INDEX_DATA *pRoomIndex;

		final[0] = '\0';
		sprintf (final, "[%2d] ", ++iReset);

		switch (pReset->command)
		{
			default:
				sprintf (buf, "Bad reset command: %c.", pReset->command);
				strcat (final, buf);
				break;

			case 'M':
				if (!(pMobIndex = get_mob_index (pReset->arg1)))
				{
					sprintf (buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1);
					strcat (final, buf);
					continue;
				}

				if (!(pRoomIndex = get_room_index (pReset->arg3)))
				{
					sprintf (buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3);
					strcat (final, buf);
					continue;
				}

				pMob = pMobIndex;
				sprintf (buf, "M[%5d] %-13.13s en el suelo         R[%5d] %2d-%2d %-15.15s\n\r",
				   pReset->arg1, pMob->short_descr, pReset->arg3,
				   pReset->arg2, pReset->arg4, pRoomIndex->name);
				strcat (final, buf);

				/*
				 * Check for pet shop.
				 * -------------------
				 */
				{
					ROOM_INDEX_DATA *pRoomIndexPrev;

					pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
					if (pRoomIndexPrev
					   && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
						final[5] = 'P';
				}

				break;

			case 'O':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Load Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!(pRoomIndex = get_room_index (pReset->arg3)))
				{
					sprintf (buf, "Load Object - Bad Room %d\n\r", pReset->arg3);
					strcat (final, buf);
					continue;
				}

				sprintf (buf, "O[%5d] %-13.13s en el suelo         "
				   "R[%5d]       %-15.15s\n\r",
				   pReset->arg1, pObj->short_descr,
				   pReset->arg3, pRoomIndex->name);
				strcat (final, buf);

				break;

			case 'P':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Put Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!(pObjToIndex = get_obj_index (pReset->arg3)))
				{
					sprintf (buf, "Put Object - Bad To Object %d\n\r",
					   pReset->arg3);
					strcat (final, buf);
					continue;
				}

				sprintf (buf,
				   "O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
				   pReset->arg1,
				   pObj->short_descr,
				   pReset->arg3,
				   pReset->arg2,
				   pReset->arg4,
				   pObjToIndex->short_descr);
				strcat (final, buf);

				break;

			case 'G':
			case 'E':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Give/Equip Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!pMob)
				{
					sprintf (buf, "Give/Equip Object - No Previous Mobile\n\r");
					strcat (final, buf);
					break;
				}

				if (pMob->pShop)
				{
					sprintf (buf,
					   "O[%5d] %-13.13s en el inventario de S[%5d]       %-15.15s\n\r",
					   pReset->arg1,
					   pObj->short_descr,
					   pMob->vnum,
					   pMob->short_descr);
				}
				else
					sprintf (buf,
					   "O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
					   pReset->arg1,
					   pObj->short_descr,
					   (pReset->command == 'G') ?
					   flag_string (wear_loc_strings, WEAR_NONE)
					   : flag_string (wear_loc_strings, pReset->arg3),
					   pMob->vnum,
					   pMob->short_descr);
				strcat (final, buf);

				break;

				/*
				 * Doors are set in rs_flags don't need to be displayed.
				 * If you want to display them then uncomment the new_reset
				 * line in the case 'D' in load_resets in db.c and here.
				 */
			case 'D':
				pRoomIndex = get_room_index (pReset->arg1);
				sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
				   pReset->arg1,
				   capitalize (dir_name[pReset->arg2]),
				   pRoomIndex->name,
				   flag_string (door_resets, pReset->arg3));
				strcat (final, buf);

				break;
				/*
				 * End Doors Comment.
				 */
			case 'R':
				if (!(pRoomIndex = get_room_index (pReset->arg1)))
				{
					sprintf (buf, "Randomize Exits - Bad Room %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				sprintf (buf, "R[%5d] Exits are randomized in %s\n\r",
				   pReset->arg1, pRoomIndex->name);
				strcat (final, buf);

				break;
		}
		send_to_char (final, ch);
	}

	return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void    add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index)
{
	RESET_DATA *reset;
	int     iReset = 0;

	if (!room->reset_first)
	{
		room->reset_first = pReset;
		room->reset_last = pReset;
		pReset->next = NULL;
		return;
	}

	index--;

	if (index == 0)				/* First slot (1) selected. */
	{
		pReset->next = room->reset_first;
		room->reset_first = pReset;
		return;
	}

	/*
	 * If negative slot( <= 0 selected) then this will find the last.
	 */
	for (reset = room->reset_first; reset->next; reset = reset->next)
	{
		if (++iReset == index)
			break;
	}

	pReset->next = reset->next;
	reset->next = pReset;
	if (!pReset->next)
		room->reset_last = pReset;
	return;
}



void    do_resets (CHAR_DATA * ch, char *argument)
{
	char    arg1[MAX_INPUT_LENGTH];
	char    arg2[MAX_INPUT_LENGTH];
	char    arg3[MAX_INPUT_LENGTH];
	char    arg4[MAX_INPUT_LENGTH];
	char    arg5[MAX_INPUT_LENGTH];
	char    arg6[MAX_INPUT_LENGTH];
	char    arg7[MAX_INPUT_LENGTH];
	RESET_DATA *pReset = NULL;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	argument = one_argument (argument, arg3);
	argument = one_argument (argument, arg4);
	argument = one_argument (argument, arg5);
	argument = one_argument (argument, arg6);
	argument = one_argument (argument, arg7);

	if (!IS_BUILDER (ch, ch->in_room->area))
	{
		send_to_char ("Resets: Invalid security for editing this area.\n\r",
		   ch);
		return;
	}

	/*
	 * Display resets in current room.
	 * -------------------------------
	 */
	if (arg1[0] == '\0')
	{
		if (ch->in_room->reset_first)
		{
			send_to_char (
			   "Resets: M = mobile, R = room, O = object, "
			   "P = pet, S = shopkeeper\n\r", ch);
			display_resets (ch);
		}
		else
			send_to_char ("No resets in this room.\n\r", ch);
	}


	/*
	 * Take index number and search for commands.
	 * ------------------------------------------
	 */
	if (is_number (arg1))
	{
		ROOM_INDEX_DATA *pRoom = ch->in_room;

		/*
		 * Delete a reset.
		 * ---------------
		 */
		if (!str_cmp (arg2, "delete"))
		{
			int     insert_loc = atoi (arg1);

			if (!ch->in_room->reset_first)
			{
				send_to_char ("No resets in this area.\n\r", ch);
				return;
			}

			if (insert_loc - 1 <= 0)
			{
				pReset = pRoom->reset_first;
				pRoom->reset_first = pRoom->reset_first->next;
				if (!pRoom->reset_first)
					pRoom->reset_last = NULL;
			}
			else
			{
				int     iReset = 0;
				RESET_DATA *prev = NULL;

				for (pReset = pRoom->reset_first;
				   pReset;
				   pReset = pReset->next)
				{
					if (++iReset == insert_loc)
						break;
					prev = pReset;
				}

				if (!pReset)
				{
					send_to_char ("Reset not found.\n\r", ch);
					return;
				}

				if (prev)
					prev->next = prev->next->next;
				else
					pRoom->reset_first = pRoom->reset_first->next;

				for (pRoom->reset_last = pRoom->reset_first;
				   pRoom->reset_last->next;
				   pRoom->reset_last = pRoom->reset_last->next) ;
			}

			free_reset_data (pReset);
			send_to_char ("Reset deleted.\n\r", ch);
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
		}
		else
			/*
			 * Add a reset.
			 * ------------
			 */
			if ((!str_cmp (arg2, "mob") && is_number (arg3))
		   || (!str_cmp (arg2, "obj") && is_number (arg3)))
		{
			/*
			 * Check for Mobile reset.
			 * -----------------------
			 */
			if (!str_cmp (arg2, "mob"))
			{
				if (get_mob_index (is_number (arg3) ? atoi (arg3) : 1) == NULL)
				{
					send_to_char ("Mob no existe.\n\r", ch);
					return;
				}
				pReset = new_reset_data ();
				pReset->command = 'M';
				pReset->arg1 = atoi (arg3);
				pReset->arg2 = is_number (arg4) ? atoi (arg4) : 1;	/* Max # */
				pReset->arg3 = ch->in_room->vnum;
				pReset->arg4 = is_number (arg5) ? atoi (arg5) : 1;	/* Min # */
			}
			else
				/*
				 * Check for Object reset.
				 * -----------------------
				 */
			if (!str_cmp (arg2, "obj"))
			{
				/*
				 * Inside another object.
				 * ----------------------
				 */
				if (!str_prefix (arg4, "inside"))
				{
					OBJ_INDEX_DATA *temp;

					temp = get_obj_index (is_number (arg5) ? atoi (arg5) : 1);
					if ((temp->item_type != ITEM_CONTAINER) &&
					   (temp->item_type != ITEM_CORPSE_NPC))
					{
						send_to_char ("Objeto 2 no es container.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->command = 'P';
					pReset->arg2 = is_number (arg6) ? atoi (arg6) : 1;
					pReset->arg3 = is_number (arg5) ? atoi (arg5) : 1;
					pReset->arg4 = is_number (arg7) ? atoi (arg7) : 1;
				}
				else
					/*
					 * Inside the room.
					 * ----------------
					 */
				if (!str_cmp (arg4, "room"))
				{
					if (get_obj_index (atoi (arg3)) == NULL)
					{
						send_to_char ("Vnum no existe.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->command = 'O';
					pReset->arg2 = 0;
					pReset->arg3 = ch->in_room->vnum;
					pReset->arg4 = 0;
				}
				else
					/*
					 * Into a Mobile's inventory.
					 * --------------------------
					 */
				{
					if (flag_value (wear_loc_flags, arg4) == NO_FLAG)
					{
						send_to_char ("Resets: '? wear-loc'\n\r", ch);
						return;
					}
					if (get_obj_index (atoi (arg3)) == NULL)
					{
						send_to_char ("Vnum no existe.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->arg3 = flag_value (wear_loc_flags, arg4);
					if (pReset->arg3 == WEAR_NONE)
						pReset->command = 'G';
					else
						pReset->command = 'E';
				}
			}
			add_reset (ch->in_room, pReset, atoi (arg1));
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
			send_to_char ("Reset added.\n\r", ch);
		}
		else if (!str_cmp (arg2, "random") && is_number (arg3))
		{
			if (atoi (arg3) < 1 || atoi (arg3) > 6)
			{
				send_to_char ("Invalid argument.\n\r", ch);
				return;
			}
			pReset = new_reset_data ();
			pReset->command = 'R';
			pReset->arg1 = ch->in_room->vnum;
			pReset->arg2 = atoi (arg3);
			add_reset (ch->in_room, pReset, atoi (arg1));
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
			send_to_char ("Random exits reset added.\n\r", ch);
		}
		else
		{
			send_to_char ("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
			send_to_char ("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
			send_to_char ("        RESET <number> OBJ <vnum> room\n\r", ch);
			send_to_char ("        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch);
			send_to_char ("        RESET <number> DELETE\n\r", ch);
			send_to_char ("        RESET <number> RANDOM [#x exits]\n\r", ch);
		}
	}

	return;
}



/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void    do_alist (CHAR_DATA * ch, char *argument)
{
	char    buf[MAX_STRING_LENGTH];
	char    result[MAX_STRING_LENGTH * 2];	/* May need tweaking. */
	AREA_DATA *pArea;

	sprintf (result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
	   "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders");

	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		sprintf (buf, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
		   pArea->vnum,
		   pArea->name,
		   pArea->min_vnum,
		   pArea->max_vnum,
		   pArea->file_name,
		   pArea->security,
		   pArea->builders);
		strcat (result, buf);
	}

	send_to_char (result, ch);
	return;
}

bool procesar_comando_olc( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];
	MOB_INDEX_DATA *pMob;
	OBJ_INDEX_DATA *pObj;
	int temp;

	argument = one_argument( argument, arg );

	for ( temp = 0; olc_comm_table[temp].nombre; temp++ )
	{
		if ( ( olc_comm_table[temp].editor == 0
		  ||   ch->desc->editor == olc_comm_table[temp].editor )
		&&   LOWER(arg[0]) == LOWER(olc_comm_table[temp].nombre[0])
		&&  !str_prefix(arg, olc_comm_table[temp].nombre) )
		{
			switch(ch->desc->editor)
			{
				case ED_MOBILE:
				EDIT_MOB(ch, pMob);
				xMob = cMob;	/* limpiar xMob */
				if ( olc_comm_table[temp].argumento )
					xMob = *pMob;
				if ( (*olc_comm_table[temp].funcion) (temp, ch, argument, olc_comm_table[temp].argumento, olc_comm_table[temp].parametro )
				     && xMob.area != NULL )
					SET_BIT(xMob.area->area_flags, AREA_CHANGED);
				if ( olc_comm_table[temp].argumento )
					*pMob = xMob;
				return TRUE;
				break;

				case ED_OBJECT:
				EDIT_OBJ(ch, pObj);
				xObj = cObj;
				if ( olc_comm_table[temp].argumento )
					xObj = *pObj;
				if ( (*olc_comm_table[temp].funcion) (temp, ch, argument, olc_comm_table[temp].argumento, olc_comm_table[temp].parametro )
				     && xObj.area != NULL )
					SET_BIT(xObj.area->area_flags, AREA_CHANGED);
				if ( olc_comm_table[temp].argumento )
					*pObj = xObj;
				return TRUE;
				break;
			}
		}
	}

	return FALSE;
}
