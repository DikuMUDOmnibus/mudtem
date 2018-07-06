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
#include <stdlib.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(Entity *ent)
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *ch = entEsCh(ent) ? entGetCh(ent) : NULL;

    for ( ; ; )
    {
        room = get_room_index( number_range( 0, 65535 ) );
        if ( room
        &&   (ch == NULL || can_see_room(ch,room))
	&&   !room_is_private(room)
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE) 
	&&   !IS_SET(room->room_flags, ROOM_PROTOTIPO)
	&&   !IS_SET(room->room_flags, ROOM_ARENA)
	&&   (room->clan == 0 || ch == NULL || (is_clan(ch) && ch->clan == room->clan) )
	&&   (ch == NULL || !IS_SET(room->room_flags, ROOM_NO_RECALL) || CHANCE(getNivelPr(ch)))
	&&   (ch == NULL
	||   !IS_NPC(ch)
	||   !IS_SET(ch->act,ACT_AGGRESSIVE)
	||   !IS_SET(room->room_flags,ROOM_LAW)) )
		break;
    }

    return room;
}

/* RT Enter portals */
void do_enter( CHAR_DATA *ch, char *argument)
{    
    ROOM_INDEX_DATA *location; 

    if ( ch->fighting != NULL ) 
	return;

    if ( !IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO) )
    {
    	send_to_char("No puedes usar portales desde un cuarto prototipo.\n\r", ch );
    	return;
    }

    /* nifty portal stuff */
    if (argument[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

	portal = get_obj_list( ch, argument,  ch->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_PORTAL 
        ||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,ANGEL)))
	{
	    send_to_char("No encuentras una manera de entrar.\n\r",ch);
	    return;
	}

	if (!IS_TRUSTED(ch,ANGEL) && IS_SET(portal->value[2],GATE_NOCURSE) /* ! */
	&&  (IS_AFFECTED(ch,AFF_CURSE) 
	||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
	{
	    send_to_char("Algo te impide irte...\n\r",ch);
	    return;
	}

	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
	{
	    location = get_random_room(chToEnt(ch));
	    portal->value[3] = location->vnum; /* for record keeping :) */
	}
	else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
	    location = get_random_room(chToEnt(ch));
	else
	    location = get_room_index(portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	   act("$p parece ir a ninguna parte.",ch,objToEnt(portal),NULL,TO_CHAR);
	   return;
	}

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags,ROOM_LAW))
        {
            send_to_char("Algo te impide irte...\n\r",ch);
            return;
        }

	act("$n entra en $p.",ch,objToEnt(portal),NULL,TO_ROOM);

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("Entras en $p.",ch,objToEnt(portal),NULL,TO_CHAR);
	else
	    act("Caminas a traves de $p y apareces en algun otro lugar...",
	        ch,objToEnt(portal),NULL,TO_CHAR); 

	char_from_room(ch);
	char_to_room(ch, location);

	if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
	{
	    obj_from_room(portal);
	    obj_to_room(portal,location);
	}

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("$n aparece.",ch,objToEnt(portal),NULL,TO_ROOM);
	else
	    act("$n aparece desde $p.",ch,objToEnt(portal),NULL,TO_ROOM);

	do_look(ch,"auto");

	/* charges */
	if (portal->value[0] > 0)
	    portal->value[0]--;

	/* protect against circular follows */
	if (old_room == location)
	    return;

    	for ( fch = old_room->people; fch != NULL; fch = fch_next )
    	{
            fch_next = fch->next_in_room;

            if (portal == NULL || portal->value[0] == 0) 
	    /* no following through dead portals */
                continue;
 
            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
            	do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING)
            {
 
                if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
                &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
                {
                    act("No puedes traer a $N a la ciudad.",
                    	ch,NULL,chToEnt(fch),TO_CHAR);
                    act("Esta prohibida tu entrada a la ciudad.",
                    	fch,NULL,NULL,TO_CHAR);
                    continue;
            	}
 
            	act( "Sigues a $N.", fch, NULL, chToEnt(ch), TO_CHAR );
		do_enter(fch,argument);
            }
    	}

 	if (portal != NULL && portal->value[0] == 0)
	{
	    act("$p se aleja de esta realidad.",ch,objToEnt(portal),NULL,TO_CHAR);
	    if (ch->in_room == old_room)
		act("$p se aleja de esta realidad.",ch,objToEnt(portal),NULL,TO_ROOM);
	    else if (old_room->people != NULL)
	    {
		act("$p se aleja de esta realidad.", 
		    old_room->people,objToEnt(portal),NULL,TO_CHAR);
		act("$p se aleja de esta realidad.",
		    old_room->people,objToEnt(portal),NULL,TO_ROOM);
	    }
	    extract_obj(portal, TRUE);
	}

 	/* 
 	 * If someone is following the char, these triggers get activated
 	 * for the followers before the char, but it's safer this way...
 	 */
 	if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
 	    mp_percent_trigger( chToEnt(ch), NULL, NULL, NULL, TRIG_ENTRY );
 	if ( !IS_NPC( ch ) )
 	    mp_greet_trigger( ch );

	return;
    }

    send_to_char("Nope, imposible hacerlo.\n\r",ch);
    return;
}
