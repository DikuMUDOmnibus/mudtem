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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include "merc.h"
#include "mob_cmds.h"
#include "events.h"
#include "lookup.h"
#include "olc.h"
#include "tables.h"

DECLARE_DO_FUN( do_look 	);
DECLARE_DO_FUN( do_say		);
DECLARE_DO_FUN( do_yell		);
DECLARE_DO_FUN( do_tell		);
extern ROOM_INDEX_DATA *find_location( CHAR_DATA *, char * );

#define CHECK_ENT(ch)			\
	if ( !entidadEsCh(ent) )	\
		return fFAIL;		\
	else				\
		ch = entidadGetCh(ent);

/*
 * Command table.
 */
const	struct	mob_cmd_type	mob_cmd_table	[] =
{
	{	"asound", 	do_mpasound	},
	{	"gecho",	do_mpgecho	},
	{	"zecho",	do_mpzecho	},
	{	"kill",		do_mpkill	},
	{	"assist",	do_mpassist	},
	{	"junk",		do_mpjunk	},
	{	"echo",		do_mpecho	},
	{	"echoaround",	do_mpechoaround	},
	{	"echoat",	do_mpechoat	},
	{	"mload",	do_mpmload	},
	{	"oload",	do_mpoload	},
	{	"purge",	do_mppurge	},
	{	"goto",		do_mpgoto	},
	{	"at",		do_mpat		},
	{	"transfer",	do_mptransfer	},
	{	"gtransfer",	do_mpgtransfer	},
	{	"otransfer",	do_mpotransfer	},
	{	"force",	do_mpforce	},
	{	"gforce",	do_mpgforce	},
	{	"vforce",	do_mpvforce	},
	{	"cast",		do_mpcast	},
	{	"damage",	do_mpdamage	},
	{	"remember",	do_mpremember	},
	{	"forget",	do_mpforget	},
	{	"delay",	do_mpdelay	},
	{	"cancel",	do_mpcancel	},
	{	"call",		do_mpcall	},
	{	"flee",		do_mpflee	},
	{	"remove",	do_mpremove	},
	{	"follow",	do_mpfollow	},
	{	"yell",		do_mpyell	},
	{	"say",		do_mpsay	},
	{	"timer",	do_mptimer	},
	{	"hunt",		do_mphunt	},
	{	"roomhunt",	do_mproomhunt	},
	{	"slay",		do_mpslay	},
	{	"withdraw",	do_mpwithdraw	},
	{	"oremove",	do_oremove	},
	{	"odrop",	do_odrop	},
	{	"opurge",	do_opurge	},
	{	"tell",		do_mptell	},
	{	"",		0		}
};

void do_mob( CHAR_DATA *ch, char *argument )
{
    /*
     * Security check!
     */
    if ( ch->desc != NULL && get_trust(ch) < MAX_LEVEL )
	return;
    mob_interpret( chToEnt(ch), argument );
}

void mpbug( Entity * ent, char * fmt, ...)
{
	char buf[2*MSL];
	va_list args;

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	do_mpsay( ent, buf );
}

/*
 * Mob command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void mob_interpret( Entity * ent, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument( argument, command );

    /*
     * Look for command in command table.
     */
    for ( cmd = 0; mob_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == mob_cmd_table[cmd].name[0]
	&&   !str_prefix( command, mob_cmd_table[cmd].name ) )
	{
	    (*mob_cmd_table[cmd].do_fun) ( ent, argument );
	    tail_chain( );
	    return;
	}
    }

    bugf( "Mob_interpret: comando invalido (%s), entidad %s",
    	command, entidadToStringExt(ent) );
}

#if !defined(USAR_MACROS)
char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case TRIG_ACT:             	return "ACT";
    case TRIG_SPEECH:          	return "SPEECH";
    case TRIG_NSPEECH:		return "NSPEECH";
    case TRIG_RANDOM:          	return "RANDOM";
    case TRIG_FIGHT:           	return "FIGHT";
    case TRIG_HPCNT:           	return "HPCNT";
    case TRIG_DEATH:           	return "DEATH";
    case TRIG_ENTRY:           	return "ENTRY";
    case TRIG_GREET:           	return "GREET";
    case TRIG_GRALL:        	return "GRALL";
    case TRIG_GIVE:            	return "GIVE";
    case TRIG_BRIBE:           	return "BRIBE";
    case TRIG_KILL:	      	return "KILL";
    case TRIG_DELAY:           	return "DELAY";
    case TRIG_SURR:	      	return "SURRENDER";
    case TRIG_EXIT:	      	return "EXIT";
    case TRIG_EXALL:	      	return "EXALL";
    case TRIG_ENTRYALL:		return "ENTRYALL";
    case TRIG_TIME:		return "TIME";
    default:                  	return "ERROR";
    }
}

char *oprog_type_to_name( int type )
{
    switch ( type )
    {
	case TRIG_GET:		return "GET";
	case TRIG_PUT:		return "PUT";
	case TRIG_SAC:		return "SAC";
	case TRIG_WEAR:		return "WEAR";
	case TRIG_REMOVE:	return "REMOVE";
	case TRIG_HACER:	return "HACER";
	case OTRIG_DELAY:	return "DELAY";
	default:		return "ERROR";
    }
}

char *rprog_type_to_name( int type )
{
    switch ( type )
    {
    	case RTRIG_ENTER:	return "ENTER";
    	case RTRIG_COMM:	return "COMM";
    	case RTRIG_EXCOMM:	return "EXCOMM";
    	case RTRIG_DELAY:	return "DELAY";
    	case RTRIG_SPEECH:	return "SPEECH";
    	default:		return "ERROR";
    }
}
#endif // USAR_MACROS

/* 
 * Displays MOBprogram triggers of a mobile
 *
 * Syntax: mpstat [name]
 */
void do_mpstat( CHAR_DATA *ch, char *argument )
{
    MPROG_LIST *progs;
    CHAR_DATA*	victim;
    OBJ_DATA *	obj = NULL;
    int i;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Mpstat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	if ( (obj = get_obj_world(ch, argument)) == NULL )
	{
		send_to_char( "Mob/objeto inexistente.\n\r", ch );
		return;
	}
	else
		progs = obj->pIndexData->mprogs;
    }
    else
    	progs = victim->pIndexData->mprogs;

    if ( victim != NULL && !IS_NPC( victim ) )
    {
	send_to_char( "That is not a mobile.\n\r", ch);
	return;
    }

    printf_to_char( ch, "%s #x%-6d [%s]\n\r",
	victim ? "Mob" : "Obj",
	victim ? victim->pIndexData->vnum : obj->pIndexData->vnum,
	victim ? victim->short_descr : obj->short_descr );

    if ( victim )
	printf_to_char( ch, "Target [%s]\n\r",
		victim->mprog_target == NULL 
			? "No target" : entGetName(victim->mprog_target) );

    if ( progs == NULL )
    {
	send_to_char( "[No programs set]\n\r", ch);
	return;
    }

    for ( i = 0; progs; progs = progs->next )
    	printf_to_char( ch, "[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n\r",
	      ++i,
	      victim ? mprog_type_to_name( progs->trig_type ) : oprog_type_to_name( progs->trig_type ),
	      progs->vnum,
	      progs->trig_phrase );

    return;
}

/*
 * Displays the source code of a given MOBprogram
 *
 * Syntax: mpdump [vnum]
 */
void do_mpdump( CHAR_DATA *ch, char *argument )
{
   char buf[ MAX_INPUT_LENGTH ];
   MPROG_CODE *mprg;

   one_argument( argument, buf );
   if ( ( mprg = get_mprog_index( atoi(buf) ) ) == NULL )
   {
	send_to_char( "No such MOBprogram.\n\r", ch );
	return;
   }
   page_to_char( mprg->code, ch );
}

/*
 * Prints the argument to all active players in the game
 *
 * Syntax: mob gecho [string]
 */
NEW_DO_FUN_DEC(do_mpgecho)
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	bugf( "MpGEcho: argumento invalido de entidad %s",
		entidadToStringExt(ent) );
	return fFAIL;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
 	{
	    if ( IS_IMMORTAL(d->character) )
		send_to_char( "Mob echo> ", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r", d->character );
	}
    }

    return fOK;
}

/*
 * Prints the argument to all players in the same area as the mob
 *
 * Syntax: mob zecho [string]
 */
NEW_DO_FUN_DEC(do_mpzecho)
{
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *room;

    if ( argument[0] == '\0' )
    {
	bugf( "MpZEcho: argumento invalido de entidad %s",
	    entidadToStringExt(ent) );
	return fFAIL;
    }

    if ( (room = entWhereIs(ent)) == NULL )
	return fFAIL;

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   d->character->in_room->area == room->area )
 	{
	    if ( IS_IMMORTAL(d->character) )
		send_to_char( "Mob echo> ", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r", d->character );
	}
    }

    return fOK;
}

/*
 * Prints the argument to all the rooms aroud the mobile
 *
 * Syntax: mob asound [string]
 */
NEW_DO_FUN_DEC(do_mpasound)
{
    ROOM_INDEX_DATA *room;
    EXIT_DATA *pexit;

    if ( argument[0] == '\0' )
	return fFAIL;

    room = entWhereIs(ent);

    for ( pexit = room->exits; pexit; pexit = pexit->next )
    	if ( pexit->u1.to_room != NULL
    	&&   pexit->u1.to_room != room )
    	{
	    MOBtrigger  = FALSE;
	    new_act( argument, roomToEnt(pexit->u1.to_room), NULL, NULL, TO_ROOM, POS_RESTING );
	    MOBtrigger  = TRUE;
	}

    return fOK;
}

/*
 * Lets the mobile kill any player or mobile without murder
 *
 * Syntax: mob kill [victim]
 */
NEW_DO_FUN_DEC(do_mpkill)
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return fFAIL;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	return fFAIL;

    if ( victim == ch
    || ch->position == POS_FIGHTING )
	return fFAIL;

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpKill - Charmed mob attacking master from vnum %d.",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );

    return fOK;
}

/*
 * Lets the mobile assist another mob or player
 *
 * Syntax: mob assist [character]
 */
NEW_DO_FUN_DEC(do_mpassist)
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return fFAIL;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	return fFAIL;

    if ( victim == ch || ch->fighting != NULL || victim->fighting == NULL )
	return fFAIL;

    multi_hit( ch, victim->fighting, TYPE_UNDEFINED );

    return fOK;
}

/*
 * Lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them 
 *
 * Syntax: mob junk [item]
 */
NEW_DO_FUN_DEC(do_mpjunk)
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    one_argument( argument, arg );

    if ( arg[0] == '\0')
	return fFAIL;

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
    	if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
      	{
      	    unequip_char( ch, obj );
	    extract_obj( obj, TRUE );
    	    return fOK;
      	}
      	if ( ( obj = get_obj_carry( ch, arg ,ch) ) == NULL )
	    return fFAIL;
	extract_obj( obj, TRUE );
    }
    else
      	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      	{
            obj_next = obj->next_content;
	    if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
          	if ( obj->wear_loc != WEAR_NONE)
			unequip_char( ch, obj );
          	extract_obj( obj, TRUE );
            } 
      	}

    return fOK;
}

/*
 * Prints the message to everyone in the room other than the mob and victim
 *
 * Syntax: mob echoaround [victim] [string]
 */
NEW_DO_FUN_DEC(do_mpechoaround)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return fFAIL;

    if ( entidadEsCh(ent) )
    	victim = get_char_room( entidadGetCh(ent), arg );
    else
	victim = alt_get_char_room( entWhereIs(ent), arg );

    if ( victim == NULL )
	return fFAIL;

    new_act( argument, ent, NULL, chToEnt(victim), TO_NOTVICT, POS_RESTING );

    return fOK;
}

/*
 * Prints the message to only the victim
 *
 * Syntax: mob echoat [victim] [string]
 */
NEW_DO_FUN_DEC(do_mpechoat)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
	return fFAIL;

    if ( entidadEsCh(ent) )
    	victim = get_char_world( entidadGetCh(ent), arg );
    else
	victim = alt_get_char_world( arg );

    if ( victim == NULL )
    	return fFAIL;
 
    new_act( argument, ent, NULL, chToEnt(victim), TO_VICT, POS_RESTING );

    return fOK;
}

/*
 * Prints the message to the room at large
 *
 * Syntax: mpecho [string]
 */
NEW_DO_FUN_DEC(do_mpecho)
{
    if ( argument[0] == '\0' )
	return fFAIL;
    new_act( argument, ent, NULL, NULL, TO_ROOM, POS_RESTING );
    return fOK;
}

/*
 * Lets the mobile load another mobile.
 *
 * Syntax: mob mload [vnum]
 */
NEW_DO_FUN_DEC(do_mpmload)
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;
    ROOM_INDEX_DATA *room;

    room = entWhereIs(ent);

    one_argument( argument, arg );

    if ( room == NULL || arg[0] == '\0' || !is_number(arg) )
	return fFAIL;

    vnum = atoi(arg);

    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	bugf( "Mpmload: indice inexistente (%d), entidad %s",
	    vnum, entidadToStringExt(ent) );
	return fFAIL;
    }

    victim = create_mobile( pMobIndex );

    char_to_room( victim, room );

    return fOK;
}

/*
 * Lets the mobile load an object
 *
 * Syntax: mob oload [vnum] [level] {R}
 */
NEW_DO_FUN_DEC(do_mpoload)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;
    bool            fToroom = FALSE, fWear = FALSE;
    ROOM_INDEX_DATA *room;

    if ( (room = entWhereIs(ent)) == NULL )
	return fFAIL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bugf( "Mpoload - Sintaxis invalida, entidad %s.",
	    entidadToStringExt(ent) );
        return fFAIL;
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bugf( "Mpoload - vnum invalido, entidad %s.", 
		entidadToStringExt(ent) );
	return fFAIL;
    }

    if ( arg2[0] == '\0' )
    {
	if ( entidadEsCh(ent) )
		level = get_trust( entidadGetCh(ent) );
	else
		level = pObjIndex->level;
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    bugf( "Mpoload - Sintaxis invalida, entidad %s.", 
		entidadToStringExt(ent) );
	    return fFAIL;
        }
	level = atoi( arg2 );
	if ( level < 0 || (entidadEsCh(ent) && level > get_trust(entidadGetCh(ent))) )
	{
	    bugf( "Mpoload - Nivel invalido, entidad %s.", 
		entidadToStringExt(ent) );
	    return fFAIL;
	}
    }

    /*
     * Added 3rd argument
     * omitted - load to mobile's inventory
     * 'R'     - load to room
     * 'W'     - load to mobile and force wear
     */
    if ( arg3[0] == 'R' || arg3[0] == 'r' )
	fToroom = TRUE;
    else if ( arg3[0] == 'W' || arg3[0] == 'w' )
	fWear = TRUE;

    obj = create_object( pObjIndex, level );

    if ( (fWear || !fToroom) && CAN_WEAR(obj, ITEM_TAKE) )
    {
	if ( entidadEsCh(ent) )
	{
		obj_to_char( obj, entidadGetCh(ent) );
		if ( fWear )
			wear_obj( entidadGetCh(ent), obj, TRUE );
	}
	else
	if ( entidadEsObj(ent) )
		obj_to_obj( obj, entidadGetObj(ent) );
	else
	if ( entidadEsRoom(ent) )
		obj_to_room( obj, entidadGetRoom(ent) );
	else
	{
		bugf( "do_mpoload : entidad %s invalida",
			entidadGetTipo(ent) );
		return fFAIL;
	}
    }
    else
    {
	obj_to_room( obj, room );
    }

    return fOK;
}

/*
 * Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room. The mobile cannot
 * purge itself for safety reasons.
 *
 * syntax mob purge {target}
 */
NEW_DO_FUN_DEC(do_mppurge)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;
    CHAR_DATA *victim = NULL;
    OBJ_DATA  *obj;
    ROOM_INDEX_DATA *room;

    room = entWhereIs(ent);

    if ( room == NULL )
    	return fFAIL;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

	for ( victim = room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC( victim )
	    &&    victim != ch 
	    &&   !IS_SET(victim->act, ACT_NOPURGE) )
		extract_char( victim, TRUE );
	}

	for ( obj = room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( !IS_SET(obj->extra_flags, ITEM_NOPURGE) )
		extract_obj( obj, TRUE );
	}

	return fOK;
    }

    if ( ch && ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	if ( ( obj = get_obj_here( ch, arg ) ) )
	{
	    extract_obj( obj, TRUE );
	    return fOK;
	}
	bugf( "Mppurge - Argumento invalido, entidad %s.",
		entidadToStringExt(ent) );
	return fFAIL;
    }

    if ( victim == NULL )
    	return fFAIL;

    if ( !IS_NPC( victim ) )
    {
	bugf( "Mppurge - Purgeando PC, entidad %s.", 
		entidadToStringExt(ent) );
	return fFAIL;
    }

    extract_char( victim, TRUE );

    return fOK;
}


/*
 * Lets the mobile goto any location it wishes that is not private.
 *
 * Syntax: mob goto [location]
 */
NEW_DO_FUN_DEC(do_mpgoto)
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Mpgoto - No argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpgoto - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, location );

    return fOK;
}

/* 
 * Lets the mobile do a command at another location.
 *
 * Syntax: mob at [location] [commands]
 */
NEW_DO_FUN_DEC( do_mpat )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;
    OBJ_DATA 	    *on;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpat - Bad argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpat - No such location from vnum %d.",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return fOK;
}
 
/*
 * Lets the mobile transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: mob transfer [target|'all'] [location]
 */
NEW_DO_FUN_DEC(do_mptransfer)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bugf( "Mptransfer - Bad syntax from entidad %s.", 
		entToStringExt(ent) );
	return fFAIL;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	CHAR_DATA *victim_next;

	for ( victim = entGetPeopleRoom(ent); victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( !IS_NPC(victim) )
	    {
		sprintf( buf, "%s %s", victim->name, arg2 );
		do_mptransfer( ent, buf );
	    }
	}
	return fOK;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = entWhereIs(ent);
    }
    else
    {
	if ( !str_cmp( arg2, "random" ) )
		location = get_random_room( ent );
	else
	if ( ( location = ent_find_location( ent, arg2 ) ) == NULL )
	{
	    bugf( "Mptransfer - No such location from vnum %d.",
	    	entToStringExt(ent) );
	    return fFAIL;
	}

	if ( room_is_private( location ) )
	    return fFAIL;
    }

    if ( ( victim = ent_get_char_world( ent, arg1 ) ) == NULL )
	return fFAIL;

    if ( victim->in_room == NULL )
	return fFAIL;

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    char_from_room( victim );
    char_to_room( victim, location );
    do_look( victim, "auto" );

    return fOK;
}

/*
 * Lets the mobile transfer all chars in same group as the victim.
 *
 * Syntax: mob gtransfer [victim] [location]
 */
NEW_DO_FUN_DEC(do_mpgtransfer)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    CHAR_DATA       *who, *victim, *victim_next;
    CHAR_DATA	*ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bug( "Mpgtransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( (who = get_char_room( ch, arg1 )) == NULL )
	return fFAIL;

    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
    	victim_next = victim->next_in_room;
    	if( is_same_group( who,victim ) )
    	{
	    sprintf( buf, "%s %s", victim->name, arg2 );
	    do_mptransfer( ent, buf );
    	}
    }
    return fOK;
}

/*
 * Lets the mobile force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the mobile.
 *
 * Syntax: mob force [victim] [commands]
 */
NEW_DO_FUN_DEC(do_mpforce)
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( vch->in_room == ch->in_room
		&& get_trust( vch ) < get_trust( ch ) 
		&& can_see( ch, vch ) )
	    {
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	    return fFAIL;

	if ( victim == ch )
	    return fFAIL;

	interpret( victim, argument );
    }

    return fOK;
}

/*
 * Lets the mobile force a group something. Must be mortal level.
 *
 * Syntax: mob gforce [victim] [commands]
 */
NEW_DO_FUN_DEC(do_mpgforce)
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpGforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	return fFAIL;

    if ( victim == ch )
	return fFAIL;

    for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_same_group(victim,vch) )
        {
	    interpret( vch, argument );
	}
    }
    return fOK;
}

/*
 * Forces all mobiles of certain vnum to do something (except ch)
 *
 * Syntax: mob vforce [vnum] [commands]
 */
NEW_DO_FUN_DEC(do_mpvforce)
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpVforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    if ( !is_number( arg ) )
    {
	bug( "MpVforce - Non-number argument vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }

    vnum = atoi( arg );

    for ( victim = char_list; victim; victim = victim_next )
    {
	victim_next = victim->next;
	if ( IS_NPC(victim) && victim->pIndexData->vnum == vnum
	&&   ch != victim && victim->fighting == NULL )
	    interpret( victim, argument );
    }
    return fOK;
}


/*
 * Lets the mobile cast spells --
 * Beware: this does only crude checking on the target validity
 * and does not account for mana etc., so you should do all the
 * necessary checking in your mob program before issuing this cmd!
 *
 * Syntax: mob cast [spell] {target}
 */
NEW_DO_FUN_DEC(do_mpcast)
{
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    Entity * entvictim = NULL;
    char spell[ MAX_INPUT_LENGTH ],
	 target[ MAX_INPUT_LENGTH ];
    int sn;

    argument = one_argument( argument, spell );
               one_argument( argument, target );

    if ( spell[0] == '\0' )
    {
	bugf( "MpCast - Bad syntax from entidad %s.", 
		entToStringExt(ent) );
	return fFAIL;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
    {
	bugf( "MpCast - No such spell (%s) from entidad %s.", 
		spell,
		entToStringExt(ent) );
	return fFAIL;
    }

    vch = ent_get_char_room( ent, target );
    obj = ent_get_obj_here( ent, target );

    switch ( skill_table[sn].target )
    {
	default: return fFAIL;
	case TAR_IGNORE: 
	    break;
	case TAR_CHAR_OFFENSIVE: 
	    if ( vch == NULL || entComparar(ent, chToEnt(vch)) )
		return fFAIL;
	    entvictim = chToEnt(vch);
	    break;
	case TAR_CHAR_DEFENSIVE:
	    entvictim = (vch == NULL) ? ent : chToEnt(vch);
	    break;
	case TAR_CHAR_SELF:
	    entvictim = ent;
	    break;
	case TAR_OBJ_CHAR_DEF:
	case TAR_OBJ_CHAR_OFF:
	case TAR_OBJ_INV:
	    if ( obj == NULL )
		return fFAIL;
	    entvictim = objToEnt(obj);
    }
    return (*skill_table[sn].spell_fun)( sn, entGetNivel(ent), ent, entvictim,
	skill_table[sn].target );
}

/*
 * Lets mob cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: mob damage [victim] [min] [max] {kill}
 */
NEW_DO_FUN_DEC(do_mpdamage)
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;
    ROOM_INDEX_DATA *room;
    CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;

    room = entWhereIs(ent);

    if ( room == NULL )
    	return fFAIL;

    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );

    if ( target[0] == '\0' )
    {
	bugf( "MpDamage - Bad syntax from entidad %s.",
		entidadToStringExt(ent) );
	return fFAIL;
    }
    if( !str_cmp( target, "all" ) )
	fAll = TRUE;
    else if( ( victim = ent_get_char_room( ent, target ) ) == NULL )
	return fFAIL;

    if ( is_number( min ) )
	low = atoi( min );
    else
    {
	bugf( "MpDamage - Bad damage min entidad %s.", 
		entidadToStringExt(ent) );
	return fFAIL;
    }
    if ( is_number( max ) )
	high = atoi( max );
    else
    {
	bugf( "MpDamage - Bad damage max entidad %s.", 
		entidadToStringExt(ent) );
	return fFAIL;
    }
    one_argument( argument, target );

    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */

    if ( target[0] != '\0' )
	fKill = TRUE;
    if ( fAll )
    {
	for( victim = room->people; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( ch == NULL || (victim != ch) )
    		damage( victim, victim, 
		    fKill ? 
		    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
	        TYPE_UNDEFINED, DAM_NONE, FALSE );
	}
    }
    else
    	return damage( victim, victim,
	    fKill ? 
	    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
        TYPE_UNDEFINED, DAM_NONE, FALSE );

    return fOK;
}

/*
 * Lets the mobile to remember a target. The target can be referred to
 * with $q and $Q codes in MOBprograms. See also "mob forget".
 *
 * Syntax: mob remember [victim]
 */
NEW_DO_FUN_DEC(do_mpremember)
{
    CHAR_DATA *victim;

    if ( IS_NULLSTR(argument) )
    {
    	mpbug( ent, "do_mpremember : argumento nulo" );
    	return fERROR;
    }

    victim = ent_get_char_room(ent, argument);

    if (victim == NULL)
    {
    	mpbug( ent, "do_mpremember : char %s no esta en cuarto",
    		argument );
    	return fERROR;
    }

    switch(ent->tipo)
    {
	case ENT_CH:
	entSetTarget(ent, chToEnt(victim) );
	break;

	case ENT_ROOM:
	if ( mem_lookup_id( victim->memory, entGetVnum(ent) ) != NULL )
	{
		mpbug( ent, "do_mpremember : char %s ya recuerda a ent %s",
			victim->name, entToStringExt(ent) );
		return fERROR;
	}
	give_mem( victim, MEM_ROOM, entGetVnum(ent) );
	break;

	default:
	mpbug( ent, "do_mpremember : ent %s invalida", entToStringExt(ent) );
	return fERROR;
    }

    return fOK;
}

/*
 * Reverse of "mob remember".
 *
 * Syntax: mob forget
 */
NEW_DO_FUN_DEC(do_mpforget)
{
    CHAR_DATA *victim;
    MEM_DATA *mem;

    switch(ent->tipo)
    {
	case ENT_CH:
	entSetTarget(ent, NULL);
	break;

	case ENT_ROOM:
	if ( IS_NULLSTR(argument) )
	{
		mpbug( ent, "do_mpforget : argumento nulo" );
		return fERROR;
	}
	if ( (victim = ent_get_char_room(ent, argument)) == NULL )
	{
		mpbug( ent, "do_mpforget : char %s no esta en cuarto",
			argument );
		return fERROR;
	}
	if ( (mem = mem_lookup_react_id(victim->memory, MEM_ROOM, entGetVnum(ent))) == NULL )
	{
		mpbug( ent, "do_mpforget : char %s no recuerda a ent %s",
			victim->name, entToStringExt(ent) );
		return fERROR;
	}
	extract_mem( victim, mem );
	break;

	default:
	mpbug( ent, "do_mpforget : ent %s invalida", entToStringExt(ent) );
	return fERROR;
    }

    return fOK;
}

/*
 * Sets a delay for MOBprogram execution. When the delay time expires,
 * the mobile is checked for a MObprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_MOBILE
 *
 * Syntax: mob delay [pulses]
 */
NEW_DO_FUN_DEC(do_mpdelay)
{
    if ( !is_number( argument ) )
    {
	bugf( "MpDelay: invalid arg from vnum %d.", 
		entToStringExt(ent) );
	return fFAIL;
    }

    switch(ent->tipo)
    {
    	case ENT_CH:
	char_event_add( entGetCh(ent), PULSE_PER_SECOND * atoi(argument),
		(void *) ENT_CH, ent_timer );
	break;

	case ENT_ROOM:
	room_event_add( entGetRoom(ent), PULSE_PER_SECOND * atoi(argument),
		(void *) ENT_ROOM, ent_timer );
	break;

	case ENT_OBJ:
	obj_event_add( entGetObj(ent), PULSE_PER_SECOND * atoi(argument),
		(void *) ENT_OBJ, ent_timer );
	break;

	default:
	bugf( "do_mpdelay : ent tipo %d invalida",
		ent->tipo );
	return fFAIL;
    }

    return fOK;
}

/*
 * Reverse of "mob delay", deactivates the timer.
 *
 * Syntax: mob cancel
 */
NEW_DO_FUN_DEC(do_mpcancel)
{
   EVENT * ev, * evlist;

   switch(ent->tipo)
   {
	case ENT_CH:
	evlist = entGetCh(ent)->events;
	break;

	case ENT_OBJ:
	evlist = entGetObj(ent)->events;
	break;

	case ENT_ROOM:
	evlist = entGetRoom(ent)->events;
	break;

	default:
	return fFAIL;
    }

    if ( (ev = event_pending(evlist, ent_timer)) != NULL )
    {
	event_delete(ev);
	return fOK;
    }

    return fFAIL;
}

/*
 * Lets the mobile to call another MOBprogram withing a MOBprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: mob call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
NEW_DO_FUN_DEC(do_mpcall)
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *vch;
    OBJ_DATA *obj1, *obj2;
    MPROG_CODE *prg;
    extern void program_flow( sh_int, char *, Entity *, Entity *, Entity *, Entity * );

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bugf( "MpCall: missing arguments from entity %s.", 
		entToStringExt(ent) );
	return fFAIL;
    }
    if ( ( prg = get_mprog_index( atoi(arg) ) ) == NULL )
    {
	bugf( "MpCall: invalid prog from entity %s.", 
		entToStringExt(ent) );
	return fFAIL;
    }
    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = ent_get_char_room( ent, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj1 = ent_get_obj_here( ent, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj2 = ent_get_obj_here( ent, arg );
    program_flow( prg->vnum, prg->code, ent, chToEnt(vch), objToEnt(obj1), objToEnt(obj2) );
    return fOK;
}

/*
 * Forces the mobile to flee.
 *
 * Syntax: mob flee
 *
 */
NEW_DO_FUN_DEC(do_mpflee)
{
    ROOM_INDEX_DATA *was_in;
    EXIT_DATA *pexit;
    int door, attempt;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    if ( ch->fighting != NULL )
	return fFAIL;

    if ( (was_in = ch->in_room) == NULL )
	return fFAIL;

    for ( attempt = 0; attempt < MAX_DIR; attempt++ )
    {
        door = number_door( );
        if ( ( pexit = exit_lookup(was_in, door) ) == NULL
        ||   pexit->u1.to_room == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || ( IS_NPC(ch)
        &&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
            continue;

        move_char( ch, door, FALSE );
        if ( ch->in_room != was_in )
	    return fOK;
    }

    return fFAIL;
}

/*
 * Lets the mobile to transfer an object. The object must be in the same
 * room with the mobile.
 *
 * Syntax: mob otransfer [item name] [location]
 */
NEW_DO_FUN_DEC(do_mpotransfer)
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "MpOTransfer - Missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }
    one_argument( argument, buf );
    if ( ( location = find_location( ch, buf ) ) == NULL )
    {
	bug( "MpOTransfer - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }
    if ( (obj = get_obj_here( ch, arg )) == NULL )
	return fFAIL;
    if ( obj->carried_by == NULL )
	obj_from_room( obj );
    else
    {
	if ( obj->wear_loc != WEAR_NONE )
	    unequip_char( ch, obj );
	obj_from_char( obj );
    }
    obj_to_room( obj, location );
    return fOK;
}

/*
 * Lets the mobile to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: mob remove [victim] [object vnum|'all']
 */
NEW_DO_FUN_DEC(do_mpremove)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    sh_int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	return fFAIL;

    one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
	fAll = TRUE;
    else if ( !is_number( arg ) )
    {
	bug ( "MpRemove: Invalid object from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return fFAIL;
    }
    else
	vnum = atoi( arg );

    for ( obj = victim->carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( fAll || obj->pIndexData->vnum == vnum )
	{
	     unequip_char( ch, obj );
	     obj_from_char( obj );
	     extract_obj( obj, TRUE );
	}
    }

    return fOK;
}

NEW_DO_FUN_DEC(do_mpfollow)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return fFAIL;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return fFAIL;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!",
	    ch, NULL, chToEnt(ch->master), TO_CHAR );
	return fFAIL;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return fFAIL;
	}
	stop_follower( ch );
	return fOK;
    }

    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return fOK;
}

NEW_DO_FUN_DEC(do_mpyell)
{
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    REMOVE_BIT( ch->comm, COMM_NOSHOUT );
    do_yell( ch, argument );

    return fOK;
}

NEW_DO_FUN_DEC(do_mpsay)
{
    CHAR_DATA *ch;

    switch(ent->tipo)
    {
    	case ENT_CH:
    	ch = entGetCh(ent);
	REMOVE_BIT( ch->comm, COMM_NOCHANNELS );
	do_say( ch, argument );
	return fOK;

	case ENT_OBJ:
	case ENT_STRING:
	case ENT_ROOM:
	nact( "$n dice '$t'", ent, strToEnt(argument, entWhereIs(ent)), NULL, TO_ROOM );
	return fOK;
    }

    return fFAIL;
}

NEW_DO_FUN_DEC(do_mptimer)
{
    CHAR_DATA *ch;

    CHECK_ENT(ch)

    if ( ch->pIndexData->script == NULL )
    {
    	bug( "Do_mptimer : mob %d con script NULL.", ch->pIndexData->vnum );
    	return fFAIL;
    }

    ch->pIndexData->script->timer = atoi( argument );
    return fOK;
}

NEW_DO_FUN_DEC(do_mphunt)
{
	CHAR_DATA *victim;
	CHAR_DATA *ch;

    	CHECK_ENT(ch)

	if ( is_hunting(ch) )
	{
		send_to_char( "Ya estas cazando.\n\r", ch );
		return fFAIL;
	}

	if ( (victim = get_char_world(ch, argument)) == NULL )
	{
		send_to_char( "No lo encuentras.\n\r", ch );
		return fFAIL;
	}

	if ( !can_hunt(ch, victim, TRUE) )
	{
		send_to_char( "No puedes cazarlo.\n\r", ch );
		return fFAIL;
	}

	send_to_char( "Ok.\n\r", ch );
	set_hunt(ch, victim, TRUE);
	return fOK;
}

NEW_DO_FUN_DEC(do_mproomhunt)
{
	int vnum = atoi( argument );
	ROOM_INDEX_DATA *room;
	CHAR_DATA *ch;

    	CHECK_ENT(ch)

	if ( (vnum < 1) || (room = get_room_index(vnum)) == NULL )
		return fFAIL;

	if ( !can_hunt_room(ch, room) )
	{
		send_to_char( "No puedes cazar ese cuarto.\n\r", ch );
		return fFAIL;
	}

	if ( ch->pIndexData && ch->pIndexData->script )
		ch->pIndexData->script->timer = -1; /* para que se detenga */

	set_hunt_room( ch, room );

	return fOK;
}

void raw_kill( Entity *, CHAR_DATA *, int );

NEW_DO_FUN_DEC(do_mpslay)
{
	CHAR_DATA *victim;
	CHAR_DATA *ch;

    	CHECK_ENT(ch)

	if ( (victim = get_char_room( ch, argument )) == NULL )
		return fFAIL;

	if ( ch == victim )
		return fFAIL;

	raw_kill( chToEntidad(victim, FALSE), victim, DAM_SLASH );

	return fOK;
}

NEW_DO_FUN_DEC(do_mpwithdraw)
{
	CHAR_DATA *victim;
	char arg[MIL];
	CHAR_DATA *ch;

    	CHECK_ENT(ch)

	argument = one_argument( argument, arg );

	if ( (victim = get_char_room( ch, arg )) == NULL )
		return fFAIL;

	deduct_cost( victim, atoi(argument) );
	return fOK;
}

NEW_DO_FUN_DEC(do_oremove)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj = NULL;
	int posic;
	char arg[MIL];
	ROOM_INDEX_DATA *room = entWhereIs(ent);

	if ( room == NULL )
		return fFAIL;

	argument = one_argument( argument, arg );

	if ( (victim = alt_get_char_room(room, arg)) == NULL )
	{
		mpbug( ent, "No esta aqui." );
		return fFAIL;
	}

	one_argument( argument, arg ); // ahora para la posicion

	if ( is_number(arg) ) // vnum
		obj = get_obj_vnum_list(victim->carrying, atoi(arg) );

	if ( obj == NULL
	&&  (obj = alt_get_obj_carry(victim, arg)) == NULL )
	{
		if ( (posic = flag_value( wear_loc_strings, arg )) == NO_FLAG )
		{
			mpbug( ent, "Posicion inexistente." );
			return fFAIL;
		}

		if ( (obj = get_eq_char( victim, posic )) == NULL )
		{
			mpbug( ent, "No lleva nada en esa posicion." );
			return fFAIL;
		}
	}

	unequip_char( victim, obj );

	return fOK;
}

NEW_DO_FUN_DEC(do_odrop)
{
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room = entWhereIs(ent);

	if ( !room )
		return fFAIL;

	argument = one_argument( argument, arg );

	if ( (victim = alt_get_char_room(room, arg)) == NULL )
	{
		mpbug( ent, "No esta aqui." );
		return fFAIL;
	}

	if ( (obj = alt_get_obj_carry(victim, argument)) == NULL )
	{
		mpbug( ent, "No lleva eso." );
		return fFAIL;
	}

	obj_from_char(obj);
	obj_to_room(obj, victim->in_room);
	return fOK;
}

void ent_timer( EVENT * ev )
{
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;

	switch((int) ev->param)
	{
		case ENT_CH:
		ch = ev->item.ch;
		if ( HAS_TRIGGER(ch, TRIG_DELAY) )
			mp_percent_trigger( chToEnt(ch), NULL, NULL, NULL, TRIG_DELAY );
		break;

		case ENT_OBJ:
		obj = ev->item.obj;
		if ( HAS_TRIGGER(obj, OTRIG_DELAY) )
			mp_percent_trigger( objToEnt(obj), NULL, NULL, NULL, OTRIG_DELAY );
		break;

		case ENT_ROOM:
		room = ev->item.room;
		if ( HAS_ROOM_TRIGGER(room, RTRIG_DELAY) )
			mp_percent_trigger( roomToEnt(room), NULL, NULL, NULL, RTRIG_DELAY );
		break;
	}
}

void recursive_obj_delete( OBJ_DATA * lista, int vnum )
{
	if ( lista == NULL )
		return;

	if ( lista->next_content )
		recursive_obj_delete( lista->next_content, vnum );

	if ( lista->contains )
		recursive_obj_delete( lista->contains, vnum );

	if ( lista->pIndexData->vnum != vnum )
		return;

	extract_obj( lista, TRUE );
}

NEW_DO_FUN_DEC(do_opurge)
{
	ROOM_INDEX_DATA * room = entWhereIs(ent);
	char arg[MIL];
	int vnum;
	OBJ_DATA * lista;

	if ( room == NULL )
	{
		bugf( "do_opurge : room nulo, ent %s",
			entToStringExt(ent) );
		return fERROR;
	}

	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "room" ) )
		lista = room->contents;
	else
	if ( !str_cmp( arg, "char" ) )
	{
		CHAR_DATA * ch;

		argument = one_argument( argument, arg );

		ch = ent_get_char_room( ent, arg );

		if ( ch == NULL )
		{
			mpbug( ent, "do_opurge : char %s no encontrado",
				arg );
			return fERROR;
		}

		lista = ch->carrying;
	}
	else
	{
		mpbug( ent, "do_opurge : arg %s invalido",
			arg );
		return fERROR;
	}

	if ( !is_number(argument) )
	{
		mpbug( ent, "do_opurge : vnum %s invalido", argument );
		return fERROR;
	}

	vnum = atoi(argument);

	recursive_obj_delete(lista, vnum);

	return fOK;
}

NEW_DO_FUN_DEC( do_mptell )
{
	CHAR_DATA *victim;
	char arg[MIL];
	char * oldarg = argument;

	argument = one_argument( argument, arg );

	victim = ent_get_char_world( ent, arg );

	if (victim == NULL)
	{
		mpbug( ent, "do_mptell : victima %s no encontrada, ent %s",
			arg, entToStringExt(ent) );
		return fERROR;
	}

	switch(ent->tipo)
	{
		case ENT_CH:
		REMOVE_BIT(entGetCh(ent)->comm, COMM_NOTELL);
		do_tell( entGetCh(ent), oldarg );
		return fOK;

		case ENT_ROOM:
		case ENT_OBJ:
		nact( "$n te dice '$t'", ent, strToEnt(argument, victim->in_room), chToEnt(victim), TO_VICT );
		return fOK;
	}

	mpbug( ent, "do_mptell : entidad %s invalida, argumento %s",
		entToStringExt(ent), oldarg );
	return fERROR;
}

NEW_DO_FUN_DEC( do_mpclosedoor )
{
	char arg[MIL];
	ROOM_INDEX_DATA *room;
	int dir;
	EXIT_DATA * pExit;
	extern int find_door( Entity *, char * );

	if ( IS_NULLSTR(argument) )
	{
		mpbug( ent, "do_mpclosedoor : argumento nulo" );
		return fERROR;
	}

	argument = one_argument( argument, arg );

	if ( !is_number(arg) || (room = get_room_index(atoi(arg))) == NULL )
	{
		mpbug( ent, "do_mpclosedoor : arg %s no es vnum", arg );
		return fERROR;
	}

	argument = one_argument( argument, arg );

	dir = find_door( ent, arg );

	if ( dir == -1 )
	{
		mpbug( ent, "do_mpclosedoor : direccion invalida" );
		return fERROR;
	}

	pExit = exit_lookup(room, dir);

	if ( pExit == NULL )
	{
		mpbug( ent, "do_mpclosedoor : exit NULL, room %d, dir %d", room->vnum, dir );
		return fERROR;
	}

	SET_BIT(pExit->exit_info, EX_CLOSED);

	return fOK;
}
