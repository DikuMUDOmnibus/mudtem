#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_look );

/* Trap.c written by Helix of Twilight and Vego Mud */

/* local function */
void  trapdamage args( ( CHAR_DATA *ch, OBJ_DATA *obj) );

void do_traplist(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found;

	found = FALSE;
	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
		if ( !can_see_obj( ch, obj ) || !IS_TRAP(obj) )
			continue;

		found = TRUE;

		for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
			;

		if ( in_obj->carried_by != NULL )
		{
			sprintf( buf, "%s carried by %s.\n\r",
			    obj->short_descr, PERS(in_obj->carried_by, ch) );
		}
		else
		{
			sprintf( buf, "%s in %s.\n\r",
			    obj->short_descr, in_obj->in_room == NULL
			    ? "somewhere" : in_obj->in_room->name );
		}

		buf[0] = UPPER(buf[0]);
		send_to_char( buf, ch );
	}

	if ( !found )
		send_to_char( "No traps found.\n\r", ch );

	return;
}

bool checkmovetrap(CHAR_DATA *ch, int dir)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	bool found;


	if (IS_NPC(ch) )
		return FALSE;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;

		if( IS_TRAP(obj)
		    && IS_SET(obj->trap->trap_eff, TRAP_EFF_MOVE)
		    && obj->trap->trap_charge > 0)
			found = TRUE;
		else
			found = FALSE;

		if (found == TRUE)
		{
			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_NORTH)
			    && dir == 0)
			{
				trapdamage(ch, obj);
				return TRUE;
			}

			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_EAST)
			    && dir == 1)
			{
				trapdamage(ch, obj);
				return TRUE;
			}

			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_SOUTH)
			    && dir == 2)
			{
				trapdamage(ch, obj);
				return TRUE;
			}

			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_WEST)
			    && dir == 3)
			{
				trapdamage(ch, obj);
				return TRUE;
			}

			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_UP)
			    && dir == 4)
			{
				trapdamage(ch, obj);
				return TRUE;
			}

			if (IS_SET(obj->trap->trap_eff, TRAP_EFF_DOWN)
			    && dir == 5)
			{
				trapdamage(ch, obj);
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool checkgetput(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (IS_NPC(ch) )
		return FALSE;

	if (!IS_TRAP(obj))
		return FALSE;

	if (IS_SET(obj->trap->trap_eff, TRAP_EFF_OBJECT)
	    && obj->trap->trap_charge > 0)
	{
		trapdamage(ch, obj);
		return TRUE;
	}

	return FALSE;
}

bool checkopen(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (IS_NPC(ch))
		return FALSE;

	if (!IS_TRAP(obj))
		return FALSE;

	if (IS_SET(obj->trap->trap_eff, TRAP_EFF_OPEN)
	    && obj->trap->trap_charge > 0)
	{
		trapdamage(ch, obj);
		return TRUE;
	}
	return FALSE;
}

void trapdamage(CHAR_DATA *ch, OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *pRoomIndex;
	AFFECT_DATA af;
	CHAR_DATA *wch, *wch_next;
	int dam = 0;
	int typedam = 0,ac = 0;

	if (obj->trap->trap_charge <= 0)
		return;

	act("Escuchas un ruido extrano......", ch, NULL, NULL, TO_ROOM);
	act("Escuchas un ruido extrano......", ch, NULL, NULL, TO_CHAR);
	obj->trap->trap_charge--;

	switch(obj->trap->trap_dam)
	{
	case TRAP_DAM_SLEEP:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			if ( IS_AFFECTED(ch, AFF_SLEEP) )
				return;

			af.where	= TO_AFFECTS;
			af.type		= 0;
			af.duration	= 4 + obj->level;
			af.level	= obj->level;
			af.location	= APPLY_NONE;
			af.modifier	= 0;
			af.bitvector	= AFF_SLEEP;
			affect_join( ch, &af );

			if ( IS_AWAKE(ch) )
			{
				send_to_char( "De repente te dio mucho sueno ..... zzzzzz.\n\r", ch );
				act( "$n va a dormir.", ch, NULL, NULL, TO_ROOM );
				ch->position = POS_SLEEPING;
			}
		}
		else
		{
			for (wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room)
			{
				if ( IS_AFFECTED(wch, AFF_SLEEP) )
					continue;

				af.where	= TO_AFFECTS;
				af.type		= 0;
				af.duration	= 4 + obj->level;
				af.level	= obj->level;
				af.location	= APPLY_NONE;
				af.modifier	= 0;
				af.bitvector	= AFF_SLEEP;
				affect_join( wch, &af );

				if ( IS_AWAKE(wch) )
				{
					send_to_char( "De repente te dio mucho sueno ..... zzzzzz.\n\r", wch );
					act( "$n va a dormir.", wch, NULL, NULL, TO_ROOM );
					wch->position = POS_SLEEPING;
				}
			}
		}
		break;

	case TRAP_DAM_TELEPORT:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			if ( ch->in_room == NULL
			    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
			    || ( !IS_NPC(ch) && ch->fighting != NULL ) )
			{
				send_to_char( "Wow! eso estuvo cerca...activaste una trampa y no funciono!\n\r", ch );
				return;
			}

			pRoomIndex = get_random_room( chToEnt(ch) );
			act( "$n lentamente se desvanece de esta realidad.", ch, NULL, NULL, TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, pRoomIndex );
			act( "$n lentamente aparece en esta realidad.", ch, NULL, NULL, TO_ROOM );
			do_look( ch, "auto" );
			return;
		}
		else
		{
			for (wch = ch->in_room->people; wch != NULL; wch = wch_next)
			{
				wch_next = wch->next_in_room;

				if ( wch->in_room == NULL
				    ||   IS_SET(wch->in_room->room_flags, ROOM_NO_RECALL)
				    || ( !IS_NPC(wch) && wch->fighting != NULL ) )
				{
					send_to_char( "Wow! eso estuvo cerca...Activaste una trampa y no funciono!\n\r",
					    wch);
					continue;
				}

				pRoomIndex = get_random_room( chToEnt(ch) );
				act( "$n lentamente se desvanece de esta realidad.", wch, NULL, NULL, TO_ROOM );
				char_from_room( wch );
				char_to_room( wch, pRoomIndex );
				act( "$n lentamente aparece en esta realidad.", wch, NULL, NULL, TO_ROOM );
				do_look( ch, "auto" );
			}
		}
		break;


	case TRAP_DAM_FIRE:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("Una bola de fuego sale disparada desde $p y alcanza a $n!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("Una bola de fuego sale disparada desde $p y te alcanza!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		else
		{
			act("Una bola de fuego sale disparada desde $p y golpea a todos en el cuarto!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("Una bola de fuego sale disparada desde $p y golpea a todos en el cuarto!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam	= obj->level * 4;
		typedam = DAM_FIRE;
		ac	= AC_EXOTIC;
		break;


	case TRAP_DAM_COLD:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("A blast of frost from $p hits $n!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("A blast of frost from $p hits you!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		else
		{
			act("A blast of frost from $p fills the room freezing you!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("A blast of frost from $p fills the room freezing you!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam	= obj->level * 5;
		typedam = DAM_COLD;
		ac	= AC_EXOTIC;
		break;


	case TRAP_DAM_ACID:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("A blast of acid erupts from $p, burning your skin!", ch, objToEnt(obj), NULL, TO_CHAR);
			act("A blast of acid erupts from $p, burning $n's skin!", ch, objToEnt(obj), NULL, TO_ROOM);
		}
		else
		{
			act("A blast of acid erupts from $p, burning your skin!", ch, objToEnt(obj), NULL,TO_ROOM);
			act("A blast of acid erupts from $p, burning your skin!", ch, objToEnt(obj), NULL,TO_CHAR);
		}
		dam	= obj->level * 6;
		typedam = DAM_ACID;
		ac	= AC_EXOTIC;
		break;


	case TRAP_DAM_ENERGY:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("A pulse of energy from $p zaps $n!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("A pulse of energy from $p zaps you!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		else
		{
			act("A pulse of energy from $p zaps you!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("A pulse of energy from $p zaps you!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam = obj->level * 3;
		typedam = DAM_ENERGY;
		ac = AC_EXOTIC;
		break;


	case TRAP_DAM_BLUNT:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("$n sets off a trap on $p and is hit by a blunt object!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You are hit by a blunt object from $p!", ch, objToEnt(obj), NULL, TO_CHAR);       
		}
		else
		{
			act("$n sets off a trap on $p and you are hit by a flying object!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You are hit by a blunt object from $p!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam = (10* obj->level) + UMAX(0, GET_AC(ch,AC_BASH));
		typedam = DAM_BASH;
		ac = AC_BASH;
		break;


	case TRAP_DAM_PIERCE:
		if (!IS_SET( obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("$n sets of a trap on $p and is pierced in the chest!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You set off a trap on $p and are pierced through the chest!", ch,objToEnt(obj), NULL, TO_CHAR);
		}
		else
		{
			act("$n sets off a trap on $p and you are hit by a piercing object!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You set off a trap on $p and are pierced through the chest!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam = (10* obj->level) + UMAX(0, GET_AC(ch,AC_PIERCE));
		typedam = DAM_PIERCE;
		ac = AC_PIERCE;
		break;


	case TRAP_DAM_SLASH:
		if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM) )
		{
			act("$n just got slashed by a trap on $p.", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You just got slashed by a trap on $p!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		else
		{
			act("$n set off a trap releasing a blade that slashes you!", ch, objToEnt(obj), NULL, TO_ROOM);
			act("You set off a trap releasing blades around the room..", ch, objToEnt(obj), NULL, TO_CHAR);
			act("One of the blades slashes you in the chest!", ch, objToEnt(obj), NULL, TO_CHAR);
		}
		dam = (10* obj->level) + UMAX(0, GET_AC(ch,AC_SLASH));
		typedam = DAM_SLASH;
		ac = AC_SLASH;
		break;
	}

	/*
	 * Do the damage
	 */

	dam = UMIN( 50, dam );

	if (!IS_SET(obj->trap->trap_eff, TRAP_EFF_ROOM))
		damage(ch,ch,dam,TYPE_HIT,typedam,FALSE);
	else
	{
		for (wch = ch->in_room->people; wch != NULL; wch = wch_next)
		{
			wch_next = wch->next_in_room;

			if (wch == NULL)
				break;

			if (obj->trap->trap_dam == TRAP_DAM_BLUNT
			    || obj->trap->trap_dam == TRAP_DAM_PIERCE
			    || obj->trap->trap_dam == TRAP_DAM_SLASH)
				dam = (10* obj->level) + GET_AC(wch,ac);

			damage(wch,wch,dam,TYPE_HIT,typedam,FALSE);
		}
	}

	return;
}
