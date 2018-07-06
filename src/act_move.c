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
#include <time.h>
#else
#include <sys/types.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "lookup.h"
#include "special.h" /* taxis */

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_hide_obj	);
DECLARE_DO_FUN(do_clan_recall   );



char *	const	dir_name	[MAX_DIR]	=
{
    "north", "east", "south", "west", "up", "down", "inside", "outside"
};

char *	const	dir_nombre	[]		=
{
    "el norte", "el este", "el sur", "el oeste", "arriba", "abajo", "dentro",
    "fuera"
};

char *	const	dir_nom		[]		=
{
    "norte", "este", "sur", "oeste", "arriba", "abajo", "dentro", "fuera"
};

const	sh_int	rev_dir		[]		=
{
    DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP, DIR_OUTSIDE,
    DIR_INSIDE
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 0
};



/*
 * Local functions.
 */
int	find_door	args( ( Entity * ent, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
int	get_destino	args( ( char * ) );


FRetVal move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    if ( door < 0 || door > (MAX_DIR - 1) )
    {
	bug( "Do_move: bad door %d.", door );
	return fFAIL;
    }

    if ( checkmovetrap(ch, door) )
	return fFAIL;

     /*
      * Exit trigger, if activated, bail out. Only PCs are triggered.
      */
     if ( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
	return fFAIL;
 
    if ( IS_AFFECTED2( ch, AFF_HOLD ) ) 
    {
	act( "Estas atrapad$o en una red!  No puedes moverte!", ch,
		NULL, NULL, TO_CHAR );
	return fFAIL;
    }

    in_room = ch->in_room;

    if ( ( pexit   = exit_lookup(ch->in_room, door) ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch, pexit->u1.to_room))
    {
	send_to_char( "Lamentablemente, no puedes ir en esa direccion.\n\r", ch );
	return fFAIL;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
    &&   !IS_TRUSTED(ch,ANGEL))
    {
	act( "$d esta cerrado.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_CHAR );
	return fFAIL;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "Como?  Y dejar a tu amado maestro?\n\r", ch );
	return fFAIL;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ) )
    {
	send_to_char( "Ese cuarto es privado en este momento.\n\r", ch );
	return fFAIL;
    }

    if ( !IS_NPC(ch) )
    {
	int iClass, iGuild;
	int move;

	if ( !IS_IMMORTAL(ch) )
	{
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
			for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
			{
				if ( !es_clase(ch, iClass)
	    			&&   to_room->vnum == class_table[iClass].guild[iGuild] )
	    			{
					send_to_char( "No puedes entrar alli.\n\r", ch );
					return fFAIL;
				}
			}
		}

		if (( in_room->sector_type == SECT_WATER_NOSWIM
		||    to_room->sector_type == SECT_WATER_NOSWIM )
  		&&    !IS_AFFECTED(ch,AFF_FLYING) )
		{
		    OBJ_DATA *obj;
		    bool found;

		    /*
		     * Look for a boat.
		     */
		    found = FALSE;

		    if ( IS_AFFECTED(ch, AFF_FLYING) )
			found = TRUE;

		    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		    {
			if ( obj->item_type == ITEM_BOAT )
			{
			    found = TRUE;
			    break;
			}
		    }
		    if ( !found )
		    {
			send_to_char( "Necesitas un bote para entrar alli.\n\r", ch );
			return fFAIL;
		    }
		}
	} // !IS_IMMORTAL(ch)

	if ( (in_room->sector_type == SECT_AIR
	  ||  to_room->sector_type == SECT_AIR)
	&&   !IS_AFFECTED(ch, AFF_FLYING) )
	{
		send_to_char( "No puedes volar.\n\r", ch );
		return fFAIL;
	}

	if ( in_room->sector_type != SECT_UNDERWATER
	&&   to_room->sector_type == SECT_UNDERWATER )
	{
		if ( !IS_PART(ch, PART_AGALLAS)
		&&    ch->aire < 1 )
		{
			send_to_char( "Contienes tu respiracion.\n\r", ch );
			ch->aire = get_curr_stat(ch, STAT_CON) / 5;
		}
	}

	if ( in_room->sector_type == SECT_UNDERWATER
	&&   to_room->sector_type != SECT_UNDERWATER )
	{
		if ( ch->aire > 0 )
		{
			send_to_char( "AHHHH...aire puro.\n\r", ch );
			ch->aire = 0;
		}
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */

	if ( get_carry_weight(ch) > can_carry_w(ch) * 2 )
	{
		send_to_char( "No puedes moverte con tanto peso encima!\n\r", ch );
		return fFAIL;
	}
	else
	if ( get_carry_weight(ch) > can_carry_w(ch) )
	{
		send_to_char( "Es dificil moverte con tanto peso encima!\n\r", ch );
		move *= 3;
	}

	/* conditional effects */
	if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
	    move /= 2;

	if (IS_AFFECTED(ch,AFF_SLOW))
	    move *= 2;
	
	if ( ch->master != NULL
	&& IS_NPC(ch->master)
	&& SPEC_FUN(ch->master) == spec_taxi )
	    move = 0;

	if ( ch->move < move )
	{
		send_to_char( "Estas demasiado cansado.\n\r", ch );
		return fFAIL;
	}

	WAIT_STATE( ch, 1 );
	ch->move -= move;
    } // !IS_NPC(ch)

    if ( !IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO )
    {
	if ( IS_NULLSTR(race_table[ch->race].msg_salir) )
		act( "$n va hacia $T.", ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_ROOM );
	else
		act( race_table[ch->race].msg_salir, ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_ROOM );
    }

    if ( ch->hit < ch->max_hit / 8
    &&   CHANCE(25)
    &&   get_obj_type_list( ITEM_FOUNTAIN, ch->in_room->contents ) == NULL )
    {
	OBJ_DATA *obj;

	act( "$n deja un rastro de sangre al caminar.", ch, NULL, NULL, TO_ROOM );
	obj		= create_object( get_obj_index(OBJ_VNUM_CHARCO), 0 );
    	obj->timer	= number_fuzzy(2);
	obj_to_room( obj, ch->in_room );
    }

    char_from_room( ch );
    char_to_room( ch, to_room );

    if ( !IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO )
	act( "$n ha llegado.", ch, NULL, NULL, TO_ROOM );

    do_look( ch, "auto" );

    if (in_room == to_room) /* no circular follows */
	return fOK;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch
	&&   IS_AFFECTED(fch,AFF_CHARM)
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch
	&&   fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room))
	{
		if (!IS_NPC(ch)
		&&   IS_SET(ch->in_room->room_flags,ROOM_LAW)
		&&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
		{
			act("No puedes traer a $N a la ciudad.",
				ch,NULL,chToEnt(fch),TO_CHAR);
			act("Tu entrada a la ciudad esta prohibida.",
				fch,NULL,NULL,TO_CHAR);
			continue;
		}

		act( "Sigues a $N.", fch, NULL, chToEnt(ch), TO_CHAR );
		move_char( fch, door, TRUE );
	}
    }

     /* 
      * If someone is following the char, these triggers get activated
      * for the followers before the char, but it's safer this way...
      */
     if ( IS_NPC( ch ) )
     {
	if ( HAS_TRIGGER( ch, TRIG_ENTRY ) )
		mp_percent_trigger( chToEnt(ch), NULL, NULL, NULL, TRIG_ENTRY );
	if ( HAS_TRIGGER( ch, TRIG_ENTRYALL ) )
		mp_entryall_trigger( ch );
     }
 
     if ( !char_died(ch) )
	mp_greet_trigger( ch );

     if ( HAS_ROOM_TRIGGER( ch->in_room, RTRIG_ENTER ) )
     {
	mp_percent_trigger( roomToEnt(ch->in_room), chToEnt(ch), NULL, NULL, RTRIG_ENTER );
	if (char_died(ch))
		return chdead;
     }

     if ( ch->in_room->sector_type == SECT_HOYO
     &&  !IS_AFFECTED(ch, AFF_FLYING)
     &&   exit_lookup(ch->in_room, DIR_DOWN) )
     {
	FRetVal rval;

	ch->fall++;
	rval = move_char( ch, DIR_DOWN, TRUE );

	if ( rval == chdead )
		return chdead;

	if ( ch->fall > 80 )
	{
		bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
		char_from_room( ch );
		char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
		ch->fall = 0;
		return fOK;
	}

	if (exit_lookup(ch->in_room, DIR_DOWN) == NULL && ch->fall)
		damage(ch,ch,ch->fall * number_range(ch->fall * 20, ch->fall * 50), 1040, DAM_BASH, TRUE);

	if (char_died(ch))
		return chdead;
	else
		ch->fall = 0;
    }

    return fOK;
}

void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}

void do_dentro( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_INSIDE, FALSE );
    return;
}

void do_fuera( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_OUTSIDE, FALSE );
    return;
}

int find_door( Entity * ent, char *arg )
{
    EXIT_DATA *pexit;
    int door;
    ROOM_INDEX_DATA * room = entWhereIs(ent);

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) || !str_cmp(arg, "norte"  ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) || !str_cmp(arg, "este"   ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) || !str_cmp(arg, "sur"    ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) || !str_cmp(arg, "oeste"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) || !str_cmp(arg, "arriba" ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) || !str_cmp(arg, "abajo"  ) ) door = 5;
    else if ( !str_cmp( arg, "i" ) || !str_cmp( arg, "inside") || !str_cmp(arg, "adentro") ) door = 6;
    else if ( !str_cmp( arg, "o" ) || !str_cmp( arg, "outside")|| !str_cmp(arg, "afuera" ) ) door = 7;
    else
    {
	for ( pexit = room->exits; pexit; pexit = pexit->next )
	{
	    if ( IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return pexit->direccion;
	}

	nact( "No veo $T aqui.", ent, NULL, strToEnt(arg,room), TO_CHAR );

	return -1;
    }

    if ( ( pexit = exit_lookup(room, door) ) == NULL )
    {
	nact( "No veo una puerta $T aqui.", ent, NULL, strToEnt(arg,room), TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_ent( "No puedes hacer eso.\n\r", ent );
	return -1;
    }

    return door;
}

int find_exit( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) || !str_cmp(arg, "norte"   ) ) door = DIR_NORTH;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) || !str_cmp(arg, "este"    ) ) door = DIR_EAST;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) || !str_cmp(arg, "sur"     ) ) door = DIR_SOUTH;
    else if ( !str_cmp( arg, "o" ) || !str_cmp( arg, "w" )     || !str_cmp(arg, "west"    ) || !str_cmp(arg, "oeste"   ) ) door = DIR_WEST;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) || !str_cmp(arg, "arriba"  ) ) door = DIR_UP;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) || !str_cmp(arg, "abajo"   ) ) door = DIR_DOWN;
    else if ( !str_cmp( arg, "i" ) || !str_cmp( arg, "dentro") || !str_cmp(arg, "inside"  ) ) door = DIR_INSIDE;
    else if ( !str_cmp( arg, "f" ) || !str_cmp( arg, "fuera" ) || !str_cmp(arg, "outside" ) ) door = DIR_OUTSIDE;
    else
    {
	act( "No veo $T aqui.", ch, NULL, strToEnt(arg,ch->in_room), TO_CHAR );
	return -1;
    }

    if ( ( pexit = exit_lookup(ch->in_room, door) ) == NULL )
    {
	act( "No veo una salida $T aqui.", ch, NULL, strToEnt(arg,ch->in_room), TO_CHAR );
	return -1;
    }

    return door;
}


void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Abrir que?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("No puedes hacer eso.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("Ya estaba abierta.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("Esta con llave.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("Abres $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n abre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    return;
 	}

	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER)
	    { send_to_char( "No es un contenedor.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "Ya estaba abierto.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "Esta con llave.\n\r",            ch ); return; }

	if (checkopen(ch, obj) )
	         return;

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("Abres $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	act( "$n abre $p.", ch, objToEnt(obj), NULL, TO_ROOM );

	if ( HAS_TRIGGER( obj, TRIG_HACER ) )
		mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_HACER );

	return;
    }

    if ( ( door = find_door( chToEnt(ch), arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = exit_lookup(ch->in_room, door);

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "Ya estaba abierta.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "Esta con llave.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n abre la $d.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = exit_lookup(to_room, rev_dir[door]) ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "La $d se abre.", rch, NULL, strToEnt(pexit_rev->keyword,ch->in_room), TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Cerrar que?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("No puedes hacer eso.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("Ya estaba cerrada.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("Cierras $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n cierra $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "No es un contenedor.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "Ya estaba cerrado.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("Cierras $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	act( "$n cierra $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( chToEnt(ch), arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= exit_lookup(ch->in_room, door);

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "Ya estaba cerrada.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n cierra la $d.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = exit_lookup(to_room, rev_dir[door]) ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "La $d se cierra.", rch, NULL, strToEnt(pexit_rev->keyword,ch->in_room), TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Poner llave a que?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("No puedes hacer eso.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("No esta cerrado.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("No se le puede poner llave.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("No tienes la llave.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("Ya estaba con llave.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("Pones llave a $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n pone llave a $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "No es un contenedor.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "No se le puede poner llave.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "No tienes la llave.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "Ya estaba con llave.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("Pones llave a $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	act( "$n pone llave a $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( chToEnt(ch), arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= exit_lookup(ch->in_room, door);

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "No esta cerrada.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "No se le puede poner llave.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "No tienes la llave.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "Ya estaba con llave.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n pone llave a $d.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = exit_lookup(to_room, rev_dir[door]) ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.", ch, objToEnt(obj),NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( chToEnt(ch), arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = exit_lookup(ch->in_room, door);
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = exit_lookup(to_room, rev_dir[door]) ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Forzar que?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && getNivelPr(ch) + 5 < getNivelPr(gch) )
	{
	    act( "$N esta parado demasiado cerca del seguro.",
		ch, NULL, chToEnt(gch), TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,gsn_pick_lock))
    {
	send_to_char( "Fallaste.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("No puedes hacer eso.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("No esta cerrado.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("No puede ser forzado.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("Fallaste.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("Fuerzas el seguro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n fuerza el seguro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	    


	
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "No es un contenedor.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "No puede ser forzado.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "El seguro ya esta abierto.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "Fallaste.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("Fuerzas el seguro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
        act("$n fuerza el seguro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( chToEnt(ch), arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = exit_lookup(ch->in_room, door);
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "No puede ser forzado.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "El seguro ya esta abierto.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "Fallaste.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n fuerza $d.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = exit_lookup(to_room, rev_dir[door]) ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Que tal si dejas de pelear primero?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("No encuentras un lugar donde pararte.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("No hay espacio para pararse en $p.",
		ch,objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	    return;
	}
 	ch->on = obj;
    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "No puedes despertar!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "Despiertas y te levantas.\n\r", ch );
	    act( "$n se despierta y se levanta.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   new_act("Despiertas y te paras en $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	   act("$n se despierta y se para en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    new_act("Despiertas y te paras sobre $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	    act("$n se despierta y se para sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	else 
	{
	    new_act("Despiertas y te paras dentro de $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	    act("$n se despierta y se para dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_look(ch,"auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "Te levantas.\n\r", ch );
	    act( "$n se levanta.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("Te paras en $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n se para en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("Te paras sobre $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n se para sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	else
	{
	    act("Te paras dentro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n se para dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	if ( !ch->fighting )
		ch->position = POS_STANDING;
	else
		ch->position = POS_FIGHTING;
	break;

    case POS_STANDING:
	send_to_char( "Ya estas de pie.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "Estas peleando!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Estas peleando!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("No puedes descansar en eso.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act_new("No hay mas espacio en $p.",ch,objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch,AFF_SLEEP))
	{
	    send_to_char("No puedes despertar!\n\r",ch);
	    return;
	}

	if (obj == NULL)
	{
	    send_to_char( "Te levantas y empiezas a descansar.\n\r", ch );
	    act ("$n se levanta y empieza a descansar.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    new_act("Te levantas y empiezas a descansar en $p.",
		    chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_SLEEPING);
	    act("$n se levanta y empieza a descansar en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            new_act("Te levantas y empiezas a descansar sobre $p.",
                    chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_SLEEPING);
            act("$n se levanta y empieza a descansar sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
        else
        {
            new_act("Te levantas y empiezas a descansar dentro de $p.",
                    chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_SLEEPING);
            act("$n se levanta y empieza a descansar dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "Ya estas descansando.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "Descansas.\n\r", ch );
	    act( "$n se sienta y descansa.", ch, NULL, NULL, TO_ROOM );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("Te sientas en $p y descansas.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n se sienta en $p y descansa.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("Te sientas sobre $p y descansas.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n se sienta sobre $p y descansa.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
        else
        {
	    act("Descansa dentro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n descansa dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("Descansas.\n\r",ch);
	    act("$n descansa.",ch,NULL,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("Descansas en $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n descansa en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("Descansas sobre $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n descansa sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
        }
        else
        {
	    act("Descansa dentro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    act("$n descansa dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Que tal si terminas esta pelea primero?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (!IS_SET(obj->item_type,ITEM_FURNITURE)
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("No puedes sentarte en eso.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    new_act("No hay mas espacio en $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
    }
    switch (ch->position)
    {
	case POS_SLEEPING:
	    if (IS_AFFECTED(ch,AFF_SLEEP))
	    {
		send_to_char("No puedes despertar!\n\r",ch);
		return;
	    }

            if (obj == NULL)
            {
            	send_to_char( "Despiertas y te sientas.\n\r", ch );
            	act( "$n despierta y se sienta.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	new_act("Despiertas y te sientas en $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
            	act("$n despierta y se sienta en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	new_act("Despiertas y te sientas sobre $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
            	act("$n despierta y se sienta sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
            }
            else
            {
            	new_act("Despiertas y te sientas dentro de $p.",chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
            	act("$n despierta y se sienta dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("Dejas de descansar.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("Te sientas en $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se sienta en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("Te sientas sobre $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se sienta sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("Ya estas sentado.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("Te sientas.\n\r",ch);
    	        act("$n se sienta en el suelo.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("Te sientas en $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se sienta en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("Te sientas sobre $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se sienta sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    else
	    {
		act("Te sientas dentro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se sienta dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }

    if ( obj && HAS_TRIGGER( obj, TRIG_HACER ) )
	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_HACER );

    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "Ya estas durmiendo.\n\r", ch );
	break;
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "Duermes.\n\r", ch );
	    act( "$n va a dormir.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("No ves eso aqui.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("No puedes dormir en eso!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		new_act("No hay suficiente espacio en $p para ti.",
		    chToEnt(ch),objToEnt(obj),NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("Te acuestas en $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se acuesta en $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("Te acuestas sobre $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	        act("$n se acuesta sobre $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    else
	    {
		act("Te acuestas dentro de $p.",ch,objToEnt(obj),NULL,TO_CHAR);
		act("$n se acuesta dentro de $p.",ch,objToEnt(obj),NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;

	    if ( HAS_TRIGGER( obj, TRIG_HACER ) )
	    	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_HACER );
	}
	break;

    case POS_FIGHTING:
	send_to_char( "Estas peleando!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "Pero tu estas durmiendo!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "No esta aqui.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N ya estaba despierto.", ch, NULL, chToEnt(victim), TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "No lo puedes despertar!",   ch, NULL, chToEnt(victim), TO_CHAR );  return; }

    act_new( "$n te despierta.", ch, NULL, chToEnt(victim), TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "Intentas moverte en forma silenciosa.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if (IS_AFFECTED(ch,AFF_SNEAK))
	return;

    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = getNivelPr(ch); 
	af.duration  = getNivelPr(ch);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	af.caster_id = ch->id;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}

void do_hide( CHAR_DATA *ch, char *argument )
{
    /* Maniac: Hide <object> tries to hide an object from players */
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] != '\0' )
    {
        do_hide_obj(ch, argument);
        return;
    }

    send_to_char( "Intentas esconderte.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( number_percent( ) < get_skill(ch,gsn_hide))
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    affect_strip ( ch, gsn_predinvis			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}

void entrar_laberinto(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA * temp;
	OBJ_DATA * tobj, *tobj_next;
	int vnum;
	CHAR_DATA *tch, *tch_next;
	MOB_INDEX_DATA *mob;
	char buf[MIL];
extern	AREA_DATA * laberinto;

	for ( vnum = laberinto->min_vnum; vnum <= laberinto->max_vnum; vnum++ )
		if ( (temp = get_room_index(vnum)) )
		{
			REMOVE_BIT(temp->room_flags, ROOM_PROTOTIPO);
			for ( tch = temp->people; tch; tch = tch_next )
			{
				tch_next = tch->next_in_room;
				extract_char(tch,TRUE);
			}
			for (tobj = temp->contents; tobj; tobj = tobj_next )
			{
				tobj_next = tobj->next_content;
				if ( tobj->item_type == ITEM_PORTAL )
					extract_obj(tobj,TRUE);
			}
		}

	vnum = 5 + number_range(1,1+((ch->luck < 0) ? -(ch->luck/10) : 1));

	mob		= get_mob_index(MOB_VNUM_RATA);
	mob->level	= number_fuzzy(getNivelPr(ch));
	set_mob_raza(mob, number_range(1,maxrace-1));
	sprintf(buf,"guerrero laberinto %s", race_table[mob->race].name );
	free_string(mob->player_name);
	mob->player_name = str_dup(buf);
	sprintf(buf,"un guerrero %s", race_table[mob->race].name );
	free_string(mob->short_descr);
	mob->short_descr = str_dup(buf);
	sprintf(buf,"Un guerrero %s da vueltas por el lugar, sediento de sangre.\n\r",
		race_table[mob->race].name );
	free_string(mob->long_descr);
	mob->long_descr = str_dup(buf);
	recalc(mob);

	while(vnum > 0)
	{
		tch = create_mobile(mob);

		char_to_room(tch,
			get_room_index(number_range(31000,31048)));

		set_mob_raza(mob, number_range(1,maxrace-1));
		sprintf(buf,"guerrero laberinto %s", race_table[mob->race].name );
		free_string(mob->player_name);
		mob->player_name = str_dup(buf);
		sprintf(buf,"un guerrero %s", race_table[mob->race].name );
		free_string(mob->short_descr);
		mob->short_descr = str_dup(buf);
		sprintf(buf,"Un guerrero %s da vueltas por el lugar, sediento de sangre.\n\r",
			race_table[mob->race].name );
		free_string(mob->long_descr);
		mob->long_descr = str_dup(buf);
		recalc(mob);

		vnum--;
	}

	tobj = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
	tobj->value[0] = 100;
	tobj->value[1] = 0;
	tobj->value[2] = 0;
	tobj->value[3] = ROOM_VNUM_ALTAR;
	obj_to_room(tobj, get_room_index(31048));
}

void recall_event( EVENT * ev )
{
	CHAR_DATA * ch = ev->item.ch;
	ROOM_INDEX_DATA * target = (ROOM_INDEX_DATA *) ev->param;
extern	AREA_DATA * laberinto;
extern	void recalc( MOB_INDEX_DATA * );

	if (ch->in_room == target)
		return;

	act( "$n desaparece.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "Sientes que algo te tira hacia arriba...\n\r", ch );

	if (ch->position == POS_FIGHTING)
	{
		act( "Huyes del combate! Pierdes $t puntos de experiencia.",
			ch, intToEnt(25, ch->in_room), NULL, TO_CHAR);
		gain_exp(ch, -25);

		mob_odio_check(ch->in_room, ch);
	}

	if (laberinto && target == get_room_index(laberinto->min_vnum))
		entrar_laberinto(ch);

	char_from_room(ch);
	char_to_room(ch, target);

	act( "$n aparece en el cuarto.", ch, NULL, NULL, TO_ROOM );

	ch->move /= 2;
	do_look(ch, "auto");
}

ROOM_INDEX_DATA * get_recall_room( CHAR_DATA * ch, int clanrecall )
{
extern AREA_DATA * laberinto;

	if ((CHANCE(1) || clanrecall == 2)
	&&  laberinto
	&&  laberinto->nplayer == 0)
		return get_room_index(laberinto->min_vnum);

	return (clanrecall == 1? get_room_index(get_clan_table(ch->clan)->recall) : get_room_index(ROOM_VNUM_TEMPLE));
}

void recall(CHAR_DATA *, int, char *);

DO_FUN_DEC(do_recall)
{
	recall(ch, 0, argument);
}

DO_FUN_DEC(do_clan_recall)
{
	recall(ch, 1, argument);
}

void recall( CHAR_DATA *ch, int clan, char *argument )
{
	ROOM_INDEX_DATA * target, * realtarget;
	int chance;
	extern bool order_mob;

	if ( IS_NPC(ch)
	&&   IS_PET(ch)
	&&   order_mob == TRUE )
		return;

	realtarget = get_recall_room(ch, clan);

	act( "$n ruega a su dios $t por transporte!",
		ch, strToEnt(CLAN_GOD(ch),ch->in_room), NULL, TO_ROOM);
	act( "Ruegas a tu dios $t por transporte!",
		ch, strToEnt(CLAN_GOD(ch),ch->in_room), NULL, TO_CHAR);

	if ( event_pending(ch->events, recall_event)
	||   IS_AFFECTED(ch, AFF_CURSE)
	||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   realtarget == ch->in_room)
	{
		act( "$t te ignora.", ch, strToEnt(CLAN_GOD(ch), ch->in_room),
			NULL, TO_CHAR );
		return;
	}

	chance = get_skill(ch, gsn_recall);

	chance *= UMAX(1, (300 - getNivelPr(ch) * 10)/100);

	if (ch->position == POS_FIGHTING)
	{
		chance /= 2;

		if (!CHANCE(chance))
		{
			send_to_char( "No puedes concentrarte lo suficiente.\n\r", ch );
			return;
		}
	}

	target = realtarget;

	if (ch->pet
	&&  ch->pet->in_room->area == ch->in_room->area )
		recall( ch->pet, clan, argument );

	char_event_add( ch, PULSE_PER_SECOND * 3, (void *) target, recall_event );
}

void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "No puedes hacer eso aqui.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "Tienes %d sesiones de entrenamiento.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 1;

    if ( !str_cmp( argument, "str" )
    ||   !str_cmp( argument, "fuerza" ) )
    {
	if ( class_table[getClasePr(ch)].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "fuerza";
    }

    else if ( !str_cmp( argument, "int" )
    ||	      !str_cmp( argument, "inteligencia" ) )
    {
	if ( class_table[getClasePr(ch)].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "inteligencia";
    }

    else if ( !str_cmp( argument, "wis" )
    ||	      !str_cmp( argument, "sabiduria" ) )
    {
	if ( class_table[getClasePr(ch)].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "sabiduria";
    }

    else if ( !str_cmp( argument, "dex" )
    ||	      !str_cmp( argument, "destreza " ) )
    {
	if ( class_table[getClasePr(ch)].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "destreza";
    }

    else if ( !str_cmp( argument, "con" )
    ||	      !str_cmp( argument, "contextura" ) )
    {
	if ( class_table[getClasePr(ch)].attr_prime == STAT_CON )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitucion";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else
    {
	strcpy( buf, "Puedes entrenar:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " fuerza" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " inteligencia" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " sabiduria" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " destreza" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " constitucion" );
	strcat( buf, " hp mana.\n\r");

	send_to_char( buf, ch );
	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "No tienes suficientes sesiones de entrenamiento.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Tu durabilidad aumenta!",ch,NULL,NULL,TO_CHAR);
        act( "La durabilidad de $n aumenta!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "No tienes suficientes sesiones de entrenamiento.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act( "Tu poder aumenta!",ch,NULL,NULL,TO_CHAR);
        act( "El poder de $n aumenta!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Tu $T esta al maximo.", ch, NULL, strToEnt(pOutput,ch->in_room), TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "No tienes suficientes sesiones de entrenamiento.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Tu $T aumenta!", ch, NULL, strToEnt(pOutput,ch->in_room), TO_CHAR );
    act( "La $T de $n aumenta!", ch, NULL, strToEnt(pOutput,ch->in_room), TO_ROOM );
    return;
}

void do_taxi(CHAR_DATA *ch, char * argument )
{
    CHAR_DATA *taxista;
    int dest = -1, i;
    char buf[MIL];
    extern const struct taxi_dest_type taxi_dest[];
    
    for ( taxista = ch->in_room->people ; taxista != NULL ; taxista = taxista->next_in_room )
    {
	if (!IS_NPC(taxista))
		continue;
	if (SPEC_FUN(taxista) == spec_taxi)
		break;
    }
    
    if (taxista == NULL || SPEC_FUN(taxista) != spec_taxi)
    {
       send_to_char("No puedes hacer eso aqui.\n\r",ch);
       return;
    }
    
    if (taxista->fighting != NULL)
    {
       send_to_char("Espera a que la pelea termine.\n\r",ch);
       return;
    }
    
    if ( IS_NULLSTR(argument) )
    {
    	act( "$n te dice 'Hacia donde quieres que te lleve?'", taxista, NULL, chToEnt(ch), TO_VICT );
    	act( "$n te dice 'Para ver la lista de destinos pon \"#Btaxi lista#b\"'", taxista, NULL, chToEnt(ch), TO_VICT );
    	return;
    }

    if (!str_cmp(argument,"lista"))
    {
    	act( "$n te dice 'Estos son los lugares donde te puedo llevar:'", taxista, NULL, chToEnt(ch), TO_VICT );
    	for ( i = 0; taxi_dest[i].vnum; ++i )
    	{
    		sprintf( buf, "%2d. #B%s#b\n\r", i, taxi_dest[i].lugar );
    		send_to_char( buf, ch );
    	}
	return;
    }

    if ( (dest = get_destino(argument)) == -1 )
    {
    	act("$n te dice 'No se como llegar alli'.", taxista, NULL, chToEnt(ch), TO_VICT );
    	return;
    }

    if ( DINERO(ch) < 50 )
    {
    	act("$n te dice 'Tienes muy poco dinero'.", taxista, NULL, chToEnt(ch), TO_VICT );
    	return;
    }

    if ( is_hunting(taxista) )
    {
    	act( "$n te dice 'Estoy ocupado en este momento'.", taxista, NULL, chToEnt(ch), TO_VICT );
    	return;
    }

    if (ch->master != NULL && ch->master != taxista)
    {
       act("$n te dice 'Debes estar solo'.",taxista,NULL,chToEnt(ch),TO_VICT);
       return;
    }

    if (!ch->master)
	add_follower(ch,taxista);

    send_to_char("Ok.\n\r",ch);

    set_hunt_room( taxista, get_room_index(taxi_dest[dest].vnum) );

    return;
}
    
void do_shadow ( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( get_skill(ch, gsn_shadow) < 1 )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    send_to_char( "Intentas moverte en las sombras.\n\r", ch );
    affect_strip( ch, gsn_shadow );

    if ( IS_NPC( ch ) || number_percent( ) < ch->pcdata->learned[gsn_shadow] )
    {
	af.where     = TO_AFFECTS;
	af.level     = getNivelPr(ch);
	af.type      = gsn_shadow;
	af.duration  = getNivelPr(ch);
	af.modifier  = APPLY_NONE;
	af.location  = 0;
	af.bitvector = AFF_SNEAK;
	af.caster_id = ch->id;
	affect_to_char( ch, &af );
    }
    return;

}

void do_chameleon ( CHAR_DATA *ch, char *argument )
{
    if ( get_skill(ch, gsn_chameleon) < 1 )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    send_to_char( "Intentas fundirte con tus alrededores.\n\r", ch);

    if ( IS_AFFECTED( ch, AFF_HIDE ) )
        REMOVE_BIT( ch->affected_by, AFF_HIDE );

    if ( IS_NPC( ch ) || number_percent( ) < ch->pcdata->learned[gsn_chameleon] )
        SET_BIT( ch->affected_by, AFF_HIDE );

    return;
}

void do_heighten ( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( get_skill(ch, gsn_heighten) < 1 )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( is_affected( ch, gsn_heighten ) )
        return;

    if ( IS_NPC( ch ) || number_percent( ) < ch->pcdata->learned[gsn_heighten] )
    {
	af.where     = TO_AFFECTS;
	af.level     = getNivelPr(ch);
	af.type      = gsn_heighten;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	af.caster_id = ch->id;
	affect_to_char( ch, &af );

	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char( ch, &af );
	
	af.bitvector = AFF_INFRARED;
	affect_to_char( ch, &af );
	
	send_to_char( "Tus sentidos se han realzado.\n\r", ch );
    }
    return;

}

/* Untangle by Thelonius for EnvyMud */
void do_untangle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *victim;
    char         arg [ MAX_INPUT_LENGTH ];

    if ( get_skill(ch, gsn_untangle) < 1 )
    {
	send_to_char( "No eres lo suficientemente diestro.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	victim = ch;
    else if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( !IS_AFFECTED2(victim, AFF_HOLD) )
    {
    	if ( victim == ch )
    		send_to_char( "No estas enredado.\n\r", ch );
    	else
    		act( "$N no esta enredado.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_untangle].beats);

    if ( ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )
	|| ( !IS_NPC( ch ) && CHANCE(get_skill(ch, gsn_untangle)) ) )
    {
	affect_strip( victim, gsn_snare );
	affect_strip( victim, skill_lookup("web") );
	check_improve( ch, gsn_untangle, TRUE, 1 );

	if ( victim != ch )
	{
	    act( "Liberas a $N.",  ch, NULL, chToEnt(victim), TO_CHAR    );
	    act( "$n te libera.", ch, NULL, chToEnt(victim), TO_VICT    );
	    act( "$n libera a $N.",  ch, NULL, chToEnt(victim), TO_NOTVICT );
        }
	else
        {
	    send_to_char( "Te liberas.\n\r", ch );
	    act( "$n se libera.", ch, NULL, NULL, TO_ROOM );
	}

	return;
    }

    send_to_char( "Fallaste.\n\r", ch );
    check_improve( ch, gsn_untangle, FALSE, 1 );
}

void do_snare( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim = NULL;
	int skill = get_skill( ch, gsn_snare );

	if ( skill < 2 )
	{
		send_to_char( "Enredar? Que es eso?\n\r", ch );
		return;
	}

	victim = ch->fighting;

	if ( IS_NULLSTR(argument) && (victim == NULL) )
	{
		send_to_char( "Enredar a quien?\n\r", ch );
		return;
	}

	if ( !victim )
	{
		victim = get_char_room( ch, argument );

		if ( !victim )
		{
			send_to_char( "No esta aqui.\n\r", ch );
			return;
		}
	}

	if ( IS_AFFECTED2(victim, AFF_HOLD) )
	{
		act( "$N ya esta enredado.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	if ( is_safe(ch, victim) )
		return;

	check_killer( ch, victim );

	WAIT_STATE( ch, skill_table[gsn_snare].beats );

	if ( CHANCE(skill) )
	{
		AFFECT_DATA af;

		af.where	= TO_AFFECTS2;
		af.type		= gsn_snare;
		af.duration	= 1 + getNivelPr(ch) / 8;
		af.location	= APPLY_HITROLL;
		af.modifier	= - getNivelPr(ch) / 3;
		af.bitvector	= AFF_HOLD;
		af.level	= getNivelPr(ch);
		af.caster_id	= ch->id;

		affect_to_char( victim, &af );

		act( "Tu red l$O atrapo!", ch, NULL, chToEnt(victim), TO_CHAR    );
		act( "$n te atrapo con su red!",  ch, NULL, chToEnt(victim), TO_VICT    );
		act( "$n atrapo con su red a $N.",   ch, NULL, chToEnt(victim), TO_NOTVICT );

		check_improve( ch, gsn_snare, TRUE, 1 );
		multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
	{
		act( "Tu red no atrapo a $N.  Uh oh!",
			ch, NULL, chToEnt(victim), TO_CHAR    );
		act( "$n te lanzo una red!  Atacal$O!",
			ch, NULL, chToEnt(victim), TO_VICT    );
		act( "$n le lanzo una red a $N, pero fallo!",
			ch, NULL, chToEnt(victim), TO_NOTVICT );

		check_improve( ch, gsn_snare, FALSE, 1 );
		multi_hit( victim, ch, TYPE_UNDEFINED );
	}

	return;
}
