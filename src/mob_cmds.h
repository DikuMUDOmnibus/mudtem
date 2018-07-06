/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *                                                                         *
 ***************************************************************************/


struct	mob_cmd_type
{
    char * const	name;
    NEW_DO_FUN *	do_fun;
};

/* the command table itself */
extern	const	struct	mob_cmd_type	mob_cmd_table	[];

/*
 * Command functions.
 * Defined in mob_cmds.c
 */
DECLARE_NEW_DO_FUN(	do_mpasound	);
DECLARE_NEW_DO_FUN(	do_mpgecho	);
DECLARE_NEW_DO_FUN(	do_mpzecho	);
DECLARE_NEW_DO_FUN(	do_mpkill	);
DECLARE_NEW_DO_FUN(	do_mpassist	);
DECLARE_NEW_DO_FUN(	do_mpjunk	);
DECLARE_NEW_DO_FUN(	do_mpechoaround	);
DECLARE_NEW_DO_FUN(	do_mpecho	);
DECLARE_NEW_DO_FUN(	do_mpechoat	);
DECLARE_NEW_DO_FUN(	do_mpmload	);
DECLARE_NEW_DO_FUN(	do_mpoload	);
DECLARE_NEW_DO_FUN(	do_mppurge	);
DECLARE_NEW_DO_FUN(	do_mpgoto	);
DECLARE_NEW_DO_FUN(	do_mpat		);
DECLARE_NEW_DO_FUN(	do_mptransfer	);
DECLARE_NEW_DO_FUN(	do_mpgtransfer	);
DECLARE_NEW_DO_FUN(	do_mpforce	);
DECLARE_NEW_DO_FUN(	do_mpgforce	);
DECLARE_NEW_DO_FUN(	do_mpvforce	);
DECLARE_NEW_DO_FUN(	do_mpcast	);
DECLARE_NEW_DO_FUN(	do_mpdamage	);
DECLARE_NEW_DO_FUN(	do_mpremember	);
DECLARE_NEW_DO_FUN(	do_mpforget	);
DECLARE_NEW_DO_FUN(	do_mpdelay	);
DECLARE_NEW_DO_FUN(	do_mpcancel	);
DECLARE_NEW_DO_FUN(	do_mpcall	);
DECLARE_NEW_DO_FUN(	do_mpflee	);
DECLARE_NEW_DO_FUN(	do_mpotransfer	);
DECLARE_NEW_DO_FUN(	do_mpremove	);
DECLARE_NEW_DO_FUN( do_mpfollow		);
DECLARE_NEW_DO_FUN( do_mpyell		);
DECLARE_NEW_DO_FUN( do_mpsay		);
DECLARE_NEW_DO_FUN( do_mptimer		);
DECLARE_NEW_DO_FUN( do_mproomhunt	);
DECLARE_NEW_DO_FUN( do_mpslay		);
DECLARE_NEW_DO_FUN( do_mpwithdraw	);
DECLARE_NEW_DO_FUN( do_mphunt		);
DECLARE_NEW_DO_FUN( do_oremove		);
DECLARE_NEW_DO_FUN( do_odrop		);
DECLARE_NEW_DO_FUN( do_opurge		);
DECLARE_NEW_DO_FUN( do_mptell		);
