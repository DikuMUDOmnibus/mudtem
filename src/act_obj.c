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
#include <stdlib.h>
#include <unistd.h>	// para unlink()
#include "merc.h"
#include "events.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "auction.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_wake		);
DECLARE_SPELL_FUN(spell_imprint		);
DECLARE_SPELL_FUN(spell_acid_blast	);
DECLARE_SPELL_FUN(spell_fireball	);
DECLARE_SPELL_FUN(spell_identify	);
DECLARE_SPELL_FUN(spell_null		);
DECLARE_SPEC_FUN(spec_taxidermist	);

extern	AFFECT_DATA *new_affect(void);

void fwrite_room(ROOM_INDEX_DATA *room);

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
CD *	find_reparador	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));

#undef OD
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
/*  CHAR_DATA *owner, *wch; */

    if (IS_IMMORTAL(ch))
	return TRUE;

    if ( IS_NULLSTR(obj->owner) )
	return TRUE;

    if ( !str_cmp( obj->owner, ch->name ) )
    	return TRUE;

/*  owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE; */

    return FALSE;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "No puedes tomar eso.\n\r", ch );
	return;
    }

    if (checkgetput(ch, obj) )
        return;

    if (!IS_IMMORTAL(ch) && IS_SET(obj->extra_flags, ITEM_PROTOTIPO))
    {
        send_to_char( "No puedes tomar prototipos.\n\r", ch);
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: no puedes acarrear tantos items.",
	    ch, NULL, strToEnt(obj->name,ch->in_room), TO_CHAR );
	return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
    &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
	act( "$d: no puedes acarrear tanto peso.",
	    ch, NULL, strToEnt(obj->name,ch->in_room), TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	if (obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC )
		act("Ese cadaver no es tuyo.",ch,NULL,NULL,TO_CHAR );
	else
		act("Eso no te pertenece.", ch, NULL, NULL, TO_CHAR );
	return;
    }

    if ( obj->in_obj )
    {
	OBJ_DATA *contenedor;

	for ( contenedor = obj; contenedor->in_obj; contenedor = contenedor->in_obj )
		;

	if ( ES_CORPSE(contenedor) && !can_loot(ch,contenedor) )
	{
		send_to_char( "Saqueo de cuerpos no esta permitido.\n\r", ch );
		return;
	}
    }

    if ( !IS_IMMORTAL(ch)
    &&    obj->clan
    &&    obj->clan != ch->clan )
    {
    	send_to_char( "No es de tu clan.\n\r", ch );
    	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N parece estar usando $p.",
		    ch,objToEnt(obj),chToEnt(gch),TO_CHAR);
		return;
	    }
    }

    if ( container != NULL )
    {
    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level)
	{
	    send_to_char("No eres lo suficientemente poderoso para poder usarlo.\n\r",ch);
	    return;
	}

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;	
	act( "Sacas $p desde $P.", ch, objToEnt(obj), objToEnt(container), TO_CHAR );
	act( "$n saca $p desde $P.", ch, objToEnt(obj), objToEnt(container), TO_ROOM );
	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else
    {
	act( "Tomas $p.", ch, objToEnt(obj), objToEnt(container), TO_CHAR );
	act( "$n toma $p.", ch, objToEnt(obj), objToEnt(container), TO_ROOM );
	obj_from_room( obj );
	if (IS_SET( obj->extra_flags, ITEM_HIDDEN ))
		REMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_split(ch,buffer);
	  }
        }

	extract_obj( obj, TRUE );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    if ( HAS_TRIGGER( obj, TRIG_GET ) )
    	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_GET );

    return;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Tomar que?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "No veo ningun $T aqui.", ch, NULL, strToEnt(arg1,ch->in_room), TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	    if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
	    {
		fwrite_room(ch->in_room);
		save_char_obj( ch );
	    }
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;

	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "No veo nada aqui.\n\r", ch );
		else
		    act( "No veo ningun $T aqui.", ch, NULL, strToEnt(&arg1[4],ch->in_room), TO_CHAR );
	    }
	    else
	    if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
	    {
		fwrite_room(ch->in_room);
		save_char_obj( ch );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "No puedes hacer eso.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "No veo ningun $T aqui.", ch, NULL, strToEnt(arg2,ch->in_room), TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "Eso no es un contenedor.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {
		if (!can_loot(ch,container))
		{
		    send_to_char( "No puedes hacer eso.\n\r", ch );
		    return;
		}
	    }
	}

	if ( !ES_CORPSE(container) && IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "El $d esta cerrado.", ch, NULL, strToEnt(container->name,ch->in_room), TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "No veo nada de eso en el $T.",
		    ch, NULL, strToEnt(arg2,ch->in_room), TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	    if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
	    {
		fwrite_room(ch->in_room);
		save_char_obj( ch );
	    }
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("No seas tan ambicioso!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "No veo nada en el $T.",
			ch, NULL, strToEnt(arg2,ch->in_room), TO_CHAR );
		else
		    act( "No veo nada de eso en el $T.",
			ch, NULL, strToEnt(arg2,ch->in_room), TO_CHAR );
	    }
	    else
	    if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
	    {
		fwrite_room(ch->in_room);
		save_char_obj( ch );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Poner que donde?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "No puedes hacer eso.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "No veo ningun $T aqui.", ch, NULL, strToEnt(arg2,ch->in_room), TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "Eso no es un contenedor.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "El $d esta cerrado.", ch, NULL, strToEnt(container->name,ch->in_room), TO_CHAR );
	return;
    }

    if (checkgetput( ch, container) )
       return;

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
	{
	    send_to_char( "No tienes ese item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "No lo puedes poner dentro de si mismo.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "No lo puedes soltar.\n\r", ch );
	    SET_BIT(obj->detected, DETECTED_CURSE);
	    return;
	}

    	if (WEIGHT_MULT(obj) != 100)
    	{
           send_to_char("Sientes como que fuera una mala idea.\n\r",ch);
            return;
        }

	if (get_obj_weight( obj ) + get_true_weight( container )
	     > (container->value[0] * 10) 
	||  get_obj_weight(obj) > (container->value[3] * 10))
	{
	    send_to_char( "No cabe.\n\r", ch );
	    return;
	}
	
	if (container->pIndexData->vnum == OBJ_VNUM_PIT 
	&& !CAN_WEAR(container,ITEM_TAKE))
	{
		if (obj->timer)
			SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
		else
			obj->timer = number_range(100,200);
	}

	obj_from_char( obj );
	obj_to_obj( obj, container );

	if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	    act("$n pone $p sobre $P.",ch,objToEnt(obj),objToEnt(container), TO_ROOM);
	    act("Pones $p sobre $P.",ch,objToEnt(obj),objToEnt(container), TO_CHAR);
	}
	else
	{
	    act( "$n pone $p dentro de $P.", ch, objToEnt(obj), objToEnt(container), TO_ROOM );
	    act( "Pones $p dentro de $P.", ch, objToEnt(obj), objToEnt(container), TO_CHAR );
	}

	if ( HAS_TRIGGER( obj, TRIG_PUT ) )
		mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_PUT );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   WEIGHT_MULT(obj) == 100
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_true_weight( container )
		 <= (container->value[0] * 10) 
	    &&   get_obj_weight(obj) < (container->value[3] * 10))
	    {
	    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
		&& !CAN_WEAR(obj, ITEM_TAKE) )
		{
			if (obj->timer)
				SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
			else
				obj->timer = number_range(100,200);
		}

		obj_from_char( obj );
		obj_to_obj( obj, container );

        	if (IS_SET(container->value[1],CONT_PUT_ON))
        	{
            	    act("$n pone $p sobre $P.",ch,objToEnt(obj),objToEnt(container), TO_ROOM);
            	    act("Pones $p sobre $P.",ch,objToEnt(obj),objToEnt(container), TO_CHAR);
        	}
		else
		{
		    act( "$n pone $p dentro de $P.", ch, objToEnt(obj), objToEnt(container), TO_ROOM );
		    act( "Pones $p dentro de $P.", ch, objToEnt(obj), objToEnt(container), TO_CHAR );
		}

		if ( HAS_TRIGGER( obj, TRIG_PUT ) )
			mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_PUT );
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MIL];
    char shd[MIL];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *repobj;
    OBJ_DATA *repobj_next;
    bool found;
    int cnt;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Soltar que?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount, gold = 0, silver = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) )
	{
	    send_to_char( "Lamentablemente, no puedes hacer eso.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	||   !str_cmp( arg, "silver"))
	{
	    if (ch->silver < amount)
	    {
		send_to_char("No tienes tantas monedas de plata.\n\r",ch);
		return;
	    }

	    ch->silver -= amount;
	    silver = amount;
	}

	else
	{
	    if (ch->gold < amount)
	    {
		send_to_char("No tienes tantas monedas de oro.\n\r",ch);
		return;
	    }

	    ch->gold -= amount;
  	    gold = amount;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_SILVER_ONE:
		silver += 1;
		extract_obj(obj, TRUE);
		break;

	    case OBJ_VNUM_GOLD_ONE:
		gold += 1;
		extract_obj( obj, TRUE );
		break;

	    case OBJ_VNUM_SILVER_SOME:
		silver += obj->value[0];
		extract_obj(obj, TRUE);
		break;

	    case OBJ_VNUM_GOLD_SOME:
		gold += obj->value[1];
		extract_obj( obj, TRUE );
		break;

	    case OBJ_VNUM_COINS:
		silver += obj->value[0];
		gold += obj->value[1];
		extract_obj(obj, TRUE);
		break;
	    }
	}

	obj_to_room( create_money( gold, silver ), ch->in_room );
	act( "$n bota unas cuantas monedas.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
	{
		fwrite_room(ch->in_room);
		save_char_obj( ch );
	}
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "No tienes ese item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "No puedes soltarlo.\n\r", ch );
	    SET_BIT(obj->detected, DETECTED_CURSE);
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n bota $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Tu botas $p.", ch, objToEnt(obj), NULL, TO_CHAR );
	if (!IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
		if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
		{
			fwrite_room(ch->in_room);
			save_char_obj( ch );
		}
	}
	else
	{
	    act("$p se disuelve en una nube de humo.",ch,objToEnt(obj),NULL,TO_ROOM);
	    act("$p se disuelve en una nube de humo.",ch,objToEnt(obj),NULL,TO_CHAR);
	    extract_obj(obj, TRUE);
	}
    }
    else
    {
	found = FALSE;
	/* 'drop all' or 'drop all.obj' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		cnt = 0;

		if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
		&&   can_see_obj( ch, obj )
		&&   obj->wear_loc == WEAR_NONE
		&&   can_drop_obj( ch, obj ) )
		{
			found = TRUE;
			strcpy(shd,obj->short_descr);
			for ( repobj = obj; repobj; repobj = repobj_next )
			{
				repobj_next = repobj->next_content;

				if ( !can_see_obj( ch, repobj )
				||    repobj->wear_loc != WEAR_NONE
				||   !can_drop_obj( ch, repobj )
				||    obj->pIndexData != repobj->pIndexData )
					continue;

				if ( repobj == obj_next )
					obj_next = repobj_next;

				obj_from_char( repobj );
				cnt++;
				obj_to_room( repobj, ch->in_room );
		        	if (IS_OBJ_STAT(repobj,ITEM_MELT_DROP))
		        	{
					act("$p se disuelve en una nube de humo.",ch,objToEnt(repobj),NULL,TO_ROOM);
					act("$p se disuelve en una nube de humo.",ch,objToEnt(repobj),NULL,TO_CHAR);
					extract_obj(repobj, TRUE);
				}
			}

			if ( cnt > 1 )
				sprintf( buf, "$n bota %d*%s.", cnt, shd );
			else
				sprintf( buf, "$n bota %s.", shd );
			act( buf, ch, NULL, NULL, TO_ROOM );
			if ( cnt > 1 )
				sprintf( buf, "Botas %d*%s.", cnt, shd );
			else
				sprintf( buf, "Botas %s.", shd );
			act( buf, ch, NULL, NULL, TO_CHAR );
			if ( IS_SET(ch->in_room->room_flags, ROOM_GRABADO) )
			{
				fwrite_room(ch->in_room);
				save_char_obj( ch );
			}
		}
	}
	if ( !found )
	{
		if ( arg[3] == '\0' )
			act( "No estas llevando nada.",
				ch, NULL, strToEnt(arg,ch->in_room), TO_CHAR );
		else
			act( "No estas llevando ningun $T.",
				ch, NULL, strToEnt(&arg[4],ch->in_room), TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Darle que a quien?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;
	bool silver;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
	{
	    send_to_char( "Lamentablemente, no puedes hacer eso.\n\r", ch );
	    return;
	}

	silver = str_cmp(arg2,"gold");

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Darle que a quien?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
	}

	if ( ch == victim )
	{
		send_to_char( "Muy gracioso.\n\r", ch );
		return;
	}

	if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) )
	{
	    send_to_char( "No tienes esa cantidad.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim)
	&& get_carry_weight(victim) + (silver ? silver_weight(amount) : gold_weight(amount)) > can_carry_w(victim) )
	{
	    act( "$N no puede llevar tanto peso.", ch, NULL, chToEnt(victim), TO_CHAR );
	    return;
	}

	if ( IS_NPC(victim)
	&&   IS_SET(victim->act, ACT_IS_CHANGER))
	{
	    if ( get_carry_weight(ch) + (silver ? silver_weight(amount) : gold_weight(amount)) > can_carry_w(ch) )
	    {
	    	act( "$n te dice 'Lo lamento, estas llevando demasiado peso.'", ch, NULL, chToEnt(victim), TO_VICT );
	    	return;
	    }

	    if ( !can_see(victim, ch) )
	    {
		act( "$N no puede verte.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	    }
	}

	if (silver)
	{
	    ch->silver		-= amount;
	    victim->silver 	+= amount;
	}
	else
	{
	    ch->gold		-= amount;
	    victim->gold	+= amount;
	}

	sprintf(buf,"$n te da %d monedas de %s.",amount, silver ? "plata" : "oro");
	act( buf, ch, NULL, chToEnt(victim), TO_VICT    );
	act( "$n le da a $N unas monedas.",  ch, NULL, chToEnt(victim), TO_NOTVICT );
	sprintf(buf,"Le das a $N %d monedas de %s.",amount, silver ? "plata" : "oro");
	act( buf, ch, NULL, chToEnt(victim), TO_CHAR    );

 	/*
 	 * Bribe trigger
 	 */
 	if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_BRIBE ) )
 	    mp_bribe_trigger( victim, ch, silver ? amount : amount * 100 );
 
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
	{
	    int change;

	    change = (silver ? 95 * amount / 100 / 100 
		 	     : 95 * amount);

	    if (!silver && change > victim->silver)
	    	victim->silver += change;

	    if (silver && change > victim->gold)
		victim->gold += change;

	    if (change < 1)
	    {
		act( "$n te dice 'Lo lamento, no me diste demasiadas monedas para poder cambiartelas.'",victim,NULL,chToEnt(ch),TO_VICT);
		ch->reply = victim;
		sprintf(buf,"%d %s %s", 
			amount, silver ? "silver" : "gold",ch->name);
		do_give(victim,buf);
	    }
	    else
	    {
		sprintf(buf,"%d %s %s", 
			change, silver ? "gold" : "silver",ch->name);
		do_give(victim,buf);
		if (silver)
		{
		    sprintf(buf,"%d silver %s", 
			(95 * amount / 100 - change * 100),ch->name);
		    do_give(victim,buf);
		}
		act("$n te dice 'Gracias, vuelve cuando quieras.'",
		    victim,NULL,chToEnt(ch),TO_VICT);
		ch->reply = victim;
	    }
	}
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "No tienes ese item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "Debes dejar de usarlo primero.\n\r", ch );
	return;
    }

    if ( IS_SET(obj->extra_flags, ITEM_PROTOTIPO) )
    {
    	send_to_char( "No puedes dar prototipos.\n\r", ch );
    	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Muy gracioso.\n\r", ch );
	return;
    }

    if (IS_NPC(victim)
    && victim->pIndexData->pShop != NULL
    && victim->pIndexData->spec_fun != spec_taxidermist )
    {
	act("$N te dice 'Lo lamento, deberas vender eso.'",
	    ch,NULL,chToEnt(victim),TO_CHAR);
	ch->reply = victim;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "No puedes soltarlo.\n\r", ch );
	SET_BIT(obj->detected, DETECTED_CURSE);
	return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && victim->race == RACE_VAMPIRE )
    {
	act( "$N rechaza $p.", ch, objToEnt(obj), chToEnt(victim), TO_CHAR );
	act( "$n intenta darle $p a $N pero $E lo rechaza.",
	    ch, objToEnt(obj), chToEnt(victim), TO_ROOM );
	act( "Rechazas $p que $n queria darte.",
	    ch, objToEnt(obj), chToEnt(victim), TO_VICT );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N tiene $S manos llenas.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ))
    {
	act( "$N no puede llevar tanto peso.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N no puede verlo.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if ( obj->clan && victim->clan != obj->clan )
    {
    	act( "$N no puede tener ese objeto.", ch, NULL, chToEnt(victim), TO_CHAR );
    	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    MOBtrigger = FALSE;
    act( "$n le da $p a $N.", ch, objToEnt(obj), chToEnt(victim), TO_NOTVICT );
    act( "$n te da $p.",   ch, objToEnt(obj), chToEnt(victim), TO_VICT    );
    act( "Le das $p a $N.", ch, objToEnt(obj), chToEnt(victim), TO_CHAR    );
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_GIVE ) )
	mp_give_trigger( victim, ch, obj );

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    /* find out what */
    if (argument == '\0')
    {
	send_to_char("Envenenar que item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
	send_to_char("No tienes ese item.\n\r",ch);
	return;
    }

    if ((skill = get_skill(ch,gsn_envenom)) < 1)
    {
	send_to_char("Estas loco? Podrias envenenarte!\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("Fallas en envenenar $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)  /* success! */
	{
	    act("$n mezcla $p con un veneno mortal.",ch,objToEnt(obj),NULL,TO_ROOM);
	    act("Mezclas $p con un veneno mortal.",ch,objToEnt(obj),NULL,TO_CHAR);

	    if ( !IS_SET(obj->value[3], FOOD_POISON) )
	    {
	    	SET_BIT(obj->value[3], FOOD_POISON);
		check_improve(ch,gsn_envenom,TRUE,4);
	    }

	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}

	act("Fallas en envenenar $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	if (!IS_SET(obj->value[3], FOOD_POISON))
	    check_improve(ch,gsn_envenom,FALSE,4);
	WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act("Pareciera que no puedes envenenar $p.",ch,objToEnt(obj),NULL,TO_CHAR);
            return;
        }

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("Solo puedes envenenar armas con filo.\n\r",ch);
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p ya fue envenenada.",ch,objToEnt(obj),NULL,TO_CHAR);
            return;
        }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = getNivelPr(ch) * percent / 100;
            af.duration  = getNivelPr(ch)/2 * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n cubre $p con un veneno mortal.",ch,objToEnt(obj),NULL,TO_ROOM);
	    act("Cubres $p con un veneno mortal.",ch,objToEnt(obj),NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
            return;
        }
	else
	{
	    act("Fallas en envenenar $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
    }
 
    act("No puedes envenenar $p.",ch,objToEnt(obj),NULL,TO_CHAR);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Llenar que?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "No tienes ese item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "No hay ninguna fuente aqui!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "No puedes llenar eso.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "Ya hay otro liquido en eso.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Tu contenedor esta lleno.\n\r", ch );
	return;
    }

    sprintf(buf,"Llenas $p con %s de $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, objToEnt(obj), objToEnt(fountain), TO_CHAR );
    sprintf(buf,"$n llena $p con %s de $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,objToEnt(obj),objToEnt(fountain),TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Verter que en que?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("No tienes ese item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("Ese no es un contenedor para liquidos.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("Esta vacio.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"Inviertes $p, derramando %s por todo el suelo.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,objToEnt(out),NULL,TO_CHAR);
	
	sprintf(buf,"$n invierte $p, derramando %s por todo el suelo.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,objToEnt(out),NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
	vch = get_char_room(ch,argument);

	if (vch == NULL)
	{
	    send_to_char("Verter a quien?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("No esta sosteniendo nada.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("Solo puedes vertir en otros contenedores de liquidos.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("No puedes cambiar las leyes de la fisica!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("No tienen el mismo liquido.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("No hay nada en $p para verter.",ch,objToEnt(out),NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p esta lleno hasta el tope.",ch,objToEnt(in),NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"Viertes %s de $p en $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,objToEnt(out),objToEnt(in),TO_CHAR);
    	sprintf(buf,"$n vierte %s de $p en $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,objToEnt(out),objToEnt(in),TO_ROOM);
    }
    else
    {
        sprintf(buf,"Viertes un poco de %s para $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,chToEnt(vch),TO_CHAR);
	sprintf(buf,"$n vierte en tu contenedor un poco de %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,chToEnt(vch),TO_VICT);
        sprintf(buf,"$n vierte %s en el contenedor de $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,chToEnt(vch),TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Tomar que?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "No puedes encontrarlo.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "Fallas en alcanzar tu boca.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "No puedes tomar de eso.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
	liquid = obj->value[2];

	if ( (ch->race == RACE_VAMPIRE) && (liquid != liq_lookup( "blood" )) )
	{
	    send_to_char( "No puedes tomar de eso.\n\r", ch );
	    return;
	}

	if ( (obj->pIndexData->vnum == OBJ_VNUM_CHARCO) && !IS_NPC(ch) && (ch->pcdata->condition[COND_THIRST] > 10) )
	{
	    send_to_char( "No estas TAN desesperado como para pasarle la lengua al suelo!\n\r", ch );
	    return;
	}

        if ( liquid < 0 )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }

	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "Esta vacio.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, obj->value[1]);
	break;
    }

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->condition[COND_FULL] > 45)
    {
	send_to_char("Estas demasiado lleno para poder tomar mas.\n\r",ch);
	return;
    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_CHARCO )
    {
	act( "$n se agacha y toma $T de $p.", ch, objToEnt(obj), strToEnt(liq_table[liquid].liq_name,ch->in_room), TO_ROOM );
	act( "Tomas $T de $p.", ch, objToEnt(obj), strToEnt(liq_table[liquid].liq_name,ch->in_room), TO_CHAR );
    }
    else
    {
	act( "$n toma $T de $p.",
		ch, objToEnt(obj), strToEnt(liq_table[liquid].liq_name,ch->in_room), TO_ROOM );
	act( "Tomas $T de $p.",
		ch, objToEnt(obj), strToEnt(liq_table[liquid].liq_name,ch->in_room), TO_CHAR );
    }

    gain_condition( ch, COND_DRUNK,
		amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );

    if ( ch->race != RACE_VAMPIRE )
    {
	gain_condition( ch, COND_FULL,
		amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
	if ( gain_condition(ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 ) == victdead )
		return;
	if ( gain_condition(ch, COND_HUNGER, amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 ) == victdead )
		return;
    }
    else
    /* If blood */
    if ( liquid == liq_lookup("blood") )
    {
	gain_condition( ch, COND_FULL, amount * 2 );
	if ( gain_condition( ch, COND_THIRST, amount ) == victdead )
		return;
	if ( gain_condition( ch, COND_HUNGER, amount ) == victdead )
		return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "Te sientes ebrio.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	send_to_char( "Estas lleno.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	send_to_char( "Tu sed fue saciada.\n\r", ch );

    if ( obj->value[3] != 0 && !IS_FORM(ch, FORM_UNDEAD) )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n se ahoga y empieza a toser.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "Te ahogas y toses.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level     = number_fuzzy(amount);
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }
	
    if (obj->value[0] > 0)
    {
        obj->value[1] -= amount;
        if (obj->item_type == ITEM_DRINK_CON
        &&  obj->value[1] == 0 )
		act( "$p quedo vacio.", ch, objToEnt(obj), NULL, TO_CHAR );
    }
        	
    return;
}

void eat_msg( EVENT *ev )
{
	switch((int) ev->param)
	{
		case 0:
		send_to_char( "Tu hambre fue saciada.\n\r", ev->item.ch );
		break;
		case 1:
		send_to_char( "Estas lleno.\n\r", ev->item.ch );
		break;
		case 2:
		act( "$n se ahoga y empieza a toser.", ev->item.ch, NULL, NULL, TO_ROOM );
		act( "Te ahogas y toses.", ev->item.ch, NULL, NULL, TO_CHAR );
		break;
		case 3:
		act( "$n termina de comer.", ev->item.ch, NULL, NULL, TO_ROOM );
		act( "Terminas de comer.", ev->item.ch, NULL, NULL, TO_CHAR );
		break;
		case 4:
		send_to_char( "Hmmm...sientes un sabor raro.\n\r", ev->item.ch );
	}
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Comer que?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "No tienes ese item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "No es comestible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "Estas demasiado lleno para poder comer mas.\n\r", ch );
	    return;
	}
    }

    act( "$n empieza a comer $p.", ch, objToEnt(obj), NULL, TO_ROOM );
    act( "Empiezas a comer $p.", ch, objToEnt(obj), NULL, TO_CHAR );

    WAIT_STATE( ch, 3*PULSE_PER_SECOND );

    switch ( obj->item_type )
    {
    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, obj->value[0] / 2 );
	    if ( gain_condition( ch, COND_HUNGER, obj->value[1]) == victdead )
	    	return;
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
		char_event_add( ch, PULSE_PER_SECOND, 0, eat_msg );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		char_event_add( ch, PULSE_PER_SECOND, (void *) 1, eat_msg );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    if (CHANCE(50))
	    	char_event_add( ch, PULSE_PER_SECOND, (void *) 4, eat_msg );

	    char_event_add( ch, 2*PULSE_PER_SECOND, (void *) 2, eat_msg );

	    if ( IS_SET(obj->value[3], FOOD_POISON) )
	    {
		af.where     = TO_AFFECTS;
		af.type      = gsn_poison;
		af.level     = number_fuzzy(obj->value[0]);
		af.duration  = 2 * obj->value[0];
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_POISON;
		affect_join( ch, &af );
	    }

	    if ( IS_SET(obj->value[3], FOOD_PLAGUE) )
	    {
		af.where     = TO_AFFECTS;
		af.type      = gsn_plague;
		af.level     = number_fuzzy(obj->value[0]);
		af.duration  = 2 * obj->value[0];
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_PLAGUE;
		affect_join( ch, &af );
	    }
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
	obj_cast_spell( obj->value[2], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
	obj_cast_spell( obj->value[3], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
	obj_cast_spell( obj->value[4], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );

	gain_condition( ch, COND_FULL, 8 );
	if ( gain_condition( ch, COND_HUNGER, 4 ) == victdead )
		return;
	break;
    }

    char_event_add( ch, 3*PULSE_PER_SECOND, (void *) 3, eat_msg );

    extract_obj( obj, TRUE );

    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj, *tObj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "No puedes remover $p.", ch, objToEnt(obj), NULL, TO_CHAR );
	SET_BIT(obj->detected, DETECTED_CURSE);
	return FALSE;
    }

    if (obj->wear_loc == WEAR_WIELD && (tObj = get_eq_char(ch, WEAR_SECONDARY)))
    	remove_obj( ch, WEAR_SECONDARY, TRUE );

    unequip_char( ch, obj );
    act( "$n deja de usar $p.", ch, objToEnt(obj), NULL, TO_ROOM );
    act( "Dejas de usar $p.", ch, objToEnt(obj), NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    if ( getNivelPr(ch) < obj->level )
    {
	printf_to_char( ch, "Debes ser de nivel %d para poder usar %s.\n\r",
		obj->level, obj->short_descr );
	act( "$n intenta usar $p, pero es demasiado inexperto.",
	    ch, objToEnt(obj), NULL, TO_ROOM );
	return;
    }

    if ( obj->clan && obj->clan != ch->clan )
    {
    	send_to_char( "Ese objeto no es de tu clan.\n\r", ch );
    	return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n enciende $p y lo sostiene.", ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Enciendes $p y lo sostienes.",  ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_FINGERS) && !IS_SET(ch->parts, PART_CLAWS) )
	{
		extern bool fBootDb;

		send_to_char( "Necesitas dedos para poder usar anillos.\n\r", ch );

		if (fBootDb)
			bugf("wear_obj : mob %d no pudo ponerse anillo vnum %d",
				CHARVNUM(ch), obj->pIndexData->vnum );

		return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n se pone $p en su dedo izquierdo.",    ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p en tu dedo izquierdo.",  ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n se pone $p en su dedo derecho.",   ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p en tu dedo derecho.", ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "Ya tienes tus dedos ocupados.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n se pone $p alrededor del cuello.",   ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p alrededor del cuello.", ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n se pone $p alrededor del cuello.",   ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p alrededor del cuello.", ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "Ya estas usando dos objetos en el cuello.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n se pone $p en el torso.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en el torso.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_HEAD) )
	{
		send_to_char( "Necesitas tener cabeza para poder hacer eso.\n\r", ch );
		return;
	}

	act( "$n se pone $p en su cabeza.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en tu cabeza.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_LEGS) )
	{
		send_to_char( "Necesitas tener piernas para poder hacer eso.\n\r", ch );
		return;
	}

	act( "$n se pone $p en sus piernas.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en tus piernas.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_FEET) )
	{
		send_to_char( "Necesitas pies para poder hacer eso.\n\r", ch );
		return;
	}

	act( "$n se pone $p en sus pies.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en tus pies.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_HANDS) )
	{
		send_to_char( "Necesitas manos para poder hacer eso.\n\r", ch );
		return;
	}

	act( "$n se pone $p en las manos.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en las manos.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_ARMS) )
	{
		send_to_char( "Necesitas brazos para poder hacer eso.\n\r", ch );
		return;
	}

	act( "$n se pone $p en los brazos.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p en los brazos.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n se pone $p alrededor del torso.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p alrededor del torso.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n se pone $p alrededor de la cintura.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Te pones $p alrededor de la cintura.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n se pone $p en su muneca izquierda.",
		ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p en tu muneca izquierda.",
		ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n se pone $p en su muneca derecha.",
		ch, objToEnt(obj), NULL, TO_ROOM );
	    act( "Te pones $p en tu muneca derecha.",
		ch, objToEnt(obj), NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "Ya estas usando dos munequeras.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);

	if (weapon != NULL
	&&  ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	{
	    send_to_char("Tus manos estan atadas con tu arma!\n\r",ch);
	    return;
	}

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("No puedes usar un escudo mientras usas 2 armas.\n\r",ch);
            return;
        }

	act( "$n usa $p como escudo.", ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Usas $p como escudo.", ch, objToEnt(obj), NULL, TO_CHAR );

	equip_char( ch, obj, WEAR_SHIELD );

	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill = 0;
	int wear_pos;

	if ( !IS_NPC(ch)
	&&    get_eq_char(ch, WEAR_WIELD) != NULL
	&&    get_eq_char(ch, WEAR_SHIELD) == NULL
	&&    get_eq_char(ch, WEAR_HOLD) == NULL
	&&   !IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
	&&   (skill = get_skill(ch, gsn_ambidiestro)) > 0 )
		wear_pos = WEAR_SECONDARY;
	else
		wear_pos = WEAR_WIELD;

	if ( !remove_obj( ch, wear_pos, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&&    get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield * 10))
	{
	    send_to_char( "Es demasiado pesado como para esgrimirlo.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch)
	&&   ch->size < SIZE_LARGE 
	&&   IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (get_eq_char(ch,WEAR_SHIELD)
	  || get_eq_char(ch,WEAR_WIELD)
	  || get_eq_char(ch,WEAR_SECONDARY)) )
	{
		send_to_char("Necesitas dos manos libres para esa arma.\n\r",ch);
		return;
	}

	if ( wear_pos == WEAR_SECONDARY
	&&   number_percent() > skill )
	{
		act( "Whoops! Intentas esgrimir $p pero se te cae!", ch, objToEnt(obj), NULL, TO_CHAR );
 		act( "$n intenta esgrimir $p, pero se le cae, torpemente.", ch, objToEnt(obj), NULL, TO_ROOM );
 		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		return;
    	}

	act( "$n esgrime $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Esgrimes $p.", ch, objToEnt(obj), NULL, TO_CHAR );

	equip_char( ch, obj, wear_pos );

        sn = get_weapon_sn(ch,FALSE);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p se siente como una parte de ti!",ch,objToEnt(obj),NULL,TO_CHAR);
        else if (skill > 85)
            act("Te sientes seguro con $p.",ch,objToEnt(obj),NULL,TO_CHAR);
        else if (skill > 70)
            act("Eres habil con $p.",ch,objToEnt(obj),NULL,TO_CHAR);
        else if (skill > 50)
            act("Tu pericia con $p es adecuada.",ch,objToEnt(obj),NULL,TO_CHAR);
        else if (skill > 25)
            act("$p se siente un poco aparatosa en tus manos.",ch,objToEnt(obj),NULL,TO_CHAR);
        else if (skill > 1)
            act("Te enredas y casi se te cae $p.",ch,objToEnt(obj),NULL,TO_CHAR);
        else
            act("No sabes siquiera cual es la punta de $p.",
                ch,objToEnt(obj),NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("No puedes sostener un item mientras usas 2 armas.\n\r",ch);
            return;
        }

	act( "$n sostiene $p en su mano.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Sostienes $p en tu mano.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act("$n suelta $p para que flote junto a $m.",ch,objToEnt(obj),NULL,TO_ROOM);
	act("Sueltas $p y empieza a flotar a tu lado.",ch,objToEnt(obj),NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LENTES ) )
    {
	if ( !remove_obj( ch, WEAR_LENTES, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_EYE) )
	{
		send_to_char( "Tu no tienes ojos.\n\r", ch );
		return;
	}

	act( "$n usa $p en sus ojos.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Usas $p para proteger tus ojos.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LENTES );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_OREJAS ) )
    {
	if ( !remove_obj( ch, WEAR_OREJAS, fReplace ) )
	    return;

	if ( !IS_SET(ch->parts, PART_EAR) )
	{
		send_to_char( "Tu no tienes ojos.\n\r", ch );
		return;
	}

	act( "$n se pone $p en los oidos.",   ch, objToEnt(obj), NULL, TO_ROOM );
	act( "Usas $p en tus orejas.", ch, objToEnt(obj), NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_OREJAS );
	return;
    }

    if ( fReplace )
	send_to_char( "No puedes esgrimir, ponerte, o sostener eso.\n\r", ch );

    return;
}

void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if ( !IS_NPC( ch ) && IS_AFFECTED2( ch, AFF_GHOUL ) )
    {
	send_to_char(
	   "No puedes ponerte, esgrimir o sostener nada mientras seas un ghoul.\n\r",
		     ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Ponerte, esgrimir o sostener que?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "No tienes ese item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sacarte que?\n\r", ch );
	return;
    }

     if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
            if ( obj->wear_loc != WEAR_NONE ) remove_obj( ch, obj->wear_loc, TRUE );
	}
     	return;
       }
    else
        if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "No tienes ese item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int silver;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n se ofrece a si mismo a $t, que graciosamente declina.",
	    ch, strToEnt(ch->clan ? get_clan_table(ch->clan)->god : "Mota", ch->in_room), NULL, TO_ROOM );
	sprintf( buf, "%s aprecia tu ofrecimiento y quizas lo acepte despues.\n\r",
		(ch->clan ? get_clan_table(ch->clan)->god : "Mota") );
	send_to_char(buf,ch);
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "No puedes encontrarlo.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
	   sprintf(buf,"A %s no le gustaria eso.\n\r",(ch->clan ? get_clan_table(ch->clan)->god : "Mota") );
	   send_to_char(buf,ch);
	   return;
        }
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC))
    {
	act( "$p no es un sacrificio aceptable.", ch, objToEnt(obj), NULL, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N parece estar usando $p.",
		    ch,objToEnt(obj),chToEnt(gch),TO_CHAR);
		return;
	    }
    }
		
    silver = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	silver = UMIN(silver,obj->cost);

    if (silver == 1) {
	sprintf(buf,"%s te da una moneda de plata por tu sacrificio.\n\r",
			(ch->clan ? get_clan_table(ch->clan)->god : "Mota") );
	send_to_char(buf,ch);
	}
    else
    {
	sprintf(buf,"%s te da %d monedas de plata por tu sacrificio.\n\r",
		(ch->clan ? get_clan_table(ch->clan)->god : "Mota"),silver);
	send_to_char(buf,ch);
    }
    
    ch->silver += silver;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && silver > 1)
	{
	    sprintf(buffer,"%d",silver);
	    do_split(ch,buffer);	
	}
    }

    if ( HAS_TRIGGER( obj, TRIG_SAC ) )
    	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_SAC );

    act( "$n sacrifica $p a $T.", ch, objToEnt(obj),strToEnt(ch->clan ? get_clan_table(ch->clan)->god : "Mota",ch->in_room) , TO_ROOM );
    wiznet("$N envia $p como un ofrecimiento consumido por el fuego.",
	   ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj, TRUE );

    return;
}

void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Beber que?\n\r", ch );
	return;
    }

    if ( EN_ARENA(ch) )
    {
    	send_to_char( "No puedes beber pociones en la Arena.\n\r", ch );
    	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "No tienes esa pocion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "Solo puedes beber pociones.\n\r", ch );
	return;
    }

    if (getNivelPr(ch) < obj->level)
    {
	send_to_char("Ese liquido es demasiado poderoso para que lo puedas beber.\n\r",ch);
	return;
    }

    act( "$n bebe $p.", ch, objToEnt(obj), NULL, TO_ROOM );
    act( "Bebes $p.", ch, objToEnt(obj), NULL ,TO_CHAR );

    if ( IS_AFFECTED2(ch, AFF_DOLOR_GUATA) )
    {
    	act( "Tomas un sorbo de $p...pero...", ch, objToEnt(obj), NULL, TO_CHAR );
    	act( "$n empieza a vomitar.", ch, NULL, NULL, TO_ROOM );
	ch->move = UMAX( 0, ch->move - 50 );
	ch->hit = UMAX( 1, ch->hit - 50 );
    	return;
    }

    if ( ch->position == POS_FIGHTING
    &&   ( get_eq_char( ch, WEAR_HOLD )
	|| get_eq_char( ch, WEAR_SECONDARY ) ) )
	WAIT_STATE(ch, 2*PULSE_PER_SECOND);
    else
	WAIT_STATE(ch, PULSE_PER_SECOND);

    DAZE_STATE(ch, PULSE_VIOLENCE);

    obj_cast_spell( obj->value[1], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
    obj_cast_spell( obj->value[2], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
    obj_cast_spell( obj->value[3], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );
    obj_cast_spell( obj->value[4], obj->value[0], objToEnt(obj), ch, chToEnt(ch) );

    extract_obj( obj, TRUE );

    return;
}

void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    if ( EN_ARENA(ch) )
    {
    	send_to_char( "No puedes recitar mientras estes en la Arena.\n\r", ch );
    	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "No tienes ese scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "Puedes recitar scrolls solamente.\n\r", ch );
	return;
    }

    if ( getNivelPr(ch) < scroll->level)
    {
	send_to_char(
		"Este scroll es demasiado complejo como para que lo puedas comprender.\n\r",ch);
	return;
    }

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
        	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
	        return;
	}

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No puedes encontrarlo.\n\r", ch );
	    return;
	}
    }

    act( "$n recita $p.", ch, objToEnt(scroll), NULL, TO_ROOM );
    act( "Recitas $p.", ch, objToEnt(scroll), NULL, TO_CHAR );

    WAIT_STATE(ch, skill_table[gsn_scrolls].beats);

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("Pronuncias mal una silaba.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], objToEnt(scroll), ch, obj ? objToEnt(obj) : chToEnt(victim) );
    	obj_cast_spell( scroll->value[2], scroll->value[0], objToEnt(scroll), ch, obj ? objToEnt(obj) : chToEnt(victim) );
    	obj_cast_spell( scroll->value[3], scroll->value[0], objToEnt(scroll), ch, obj ? objToEnt(obj) : chToEnt(victim) );
    	obj_cast_spell( scroll->value[4], scroll->value[0], objToEnt(scroll), ch, obj ? objToEnt(obj) : chToEnt(victim) );

	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll, TRUE );
    return;
}

void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    char buf[MIL];
    int sn;

    if ( EN_ARENA(ch) )
    {
    	send_to_char( "No puedes hacerlo mientras estes en la Arena.\n\r", ch );
    	return;
    }

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "No estas sosteniendo nada.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "Solo puedes blandir una vara.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	sprintf( buf, "Do_brandish: bad sn %d, vnum %d, jugador %s.",
		sn, staff->pIndexData->vnum, ch->name );
	bug( buf, 0 );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n blande $p.", ch, objToEnt(staff), NULL, TO_ROOM );
	act( "Blandes $p.",  ch, objToEnt(staff), NULL, TO_CHAR );
	if ( getNivelPr(ch) < staff->level 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act ("Fallas al invocar $p.",ch,objToEnt(staff),NULL,TO_CHAR);
	    act ("...y no paso nada.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		sprintf( buf, "Do_brandish: bad target for sn %d, vnum %d, jugador %s.",
		sn, staff->pIndexData->vnum, ch->name );
		bug( buf, 0 );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], objToEnt(staff), ch, chToEnt(vch) );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$p de $n brilla fuertemente y desaparece.", ch, objToEnt(staff), NULL, TO_ROOM );
	act( "Tu $p brilla fuertemente y desaparece.", ch, objToEnt(staff), NULL, TO_CHAR );
	extract_obj( staff, TRUE );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    if ( EN_ARENA(ch) )
    {
	send_to_char( "No puedes hacerlo mientras estes en la Arena.\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap que o quien?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "No estas sosteniendo nada.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "Solo puedes zap con una varita.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap que o quien?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "No puedes encontrarlo.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2*PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N con $p.", ch, objToEnt(wand), chToEnt(victim), TO_NOTVICT );
	    act( "Tu zap $N con $p.", ch, objToEnt(wand), chToEnt(victim), TO_CHAR );
	    act( "$n zaps you con $p.",ch, objToEnt(wand), chToEnt(victim), TO_VICT );
	}
	else
	{
	    act( "$n zaps $P con $p.", ch, objToEnt(wand), objToEnt(obj), TO_ROOM );
	    act( "Tu zap $P con $p.", ch, objToEnt(wand), objToEnt(obj), TO_CHAR );
	}

 	if (getNivelPr(ch) < wand->level 
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( "Tus esfuerzos con $p solo produjeron chispas y humo.",
		 ch,objToEnt(wand),NULL,TO_CHAR);
	    act( "Los esfuerzos de $n con $p solo produjeron chispas y humo.",
		 ch,objToEnt(wand),NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], objToEnt(wand), ch, obj ? objToEnt(obj) : chToEnt(victim) );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$p de $n explota en mil pedazos.", ch, objToEnt(wand), NULL, TO_ROOM );
	act( "$p explota en mil pedazos.", ch, objToEnt(wand), NULL, TO_CHAR );
	extract_obj( wand, TRUE );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Robar que de quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Eso es estupido.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( IS_NPC(victim) 
	  && victim->position == POS_FIGHTING)
    {
	send_to_char(  "Kill stealing no esta permitido.\n\r"
		       "Mejor no lo hagas -- podrias salir herido.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );

    /* porcentaje de ser descubierto */
    percent  = number_percent();

    if (!IS_AWAKE(victim))
    	percent -= 20;
    else if (!can_see(victim,ch))
    	percent -= 25;

    percent -= get_skill(ch, gsn_steal) / 4;
    percent -= get_curr_stat(ch, STAT_DEX) / 2;
    percent += get_curr_stat(victim, STAT_DEX) / 2;
    percent += (getNivelPr(victim) - getNivelPr(ch)) * 10;

/*  if ( ((getNivelPr(ch) + 7 < getNivelPr(victim) || getNivelPr(ch) -7 > getNivelPr(victim)) 
    && !IS_NPC(victim) && !IS_NPC(ch) )
    || ( !IS_NPC(ch) && percent > get_skill(ch,gsn_steal))
    || ( !IS_NPC(ch) && !is_clan(ch)) ) */

    if ( (is_clan(ch) && (!is_clan(victim) || is_same_clan(ch, victim)))
    ||    CHANCE(percent)
    ||   !ENTRE_I( getNivelPr(victim) - 7, getNivelPr(ch), getNivelPr(victim) + 7 ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	affect_strip(ch,gsn_sneak);
	REMOVE_BIT(ch->affected_by,AFF_SNEAK);

	act( "$n intento robarte.", ch, NULL, chToEnt(victim), TO_VICT    );
	act( "$n intento robarle a $N.",  ch, NULL, chToEnt(victim), TO_NOTVICT );

	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s es un maldito ladron!", PERS(ch, victim) );
	   break;
        case 1 :
	   sprintf( buf, "%s no podria robarle un dulce a un bebe!",
		    PERS(ch, victim) );
	   break;
	case 2 :
	    sprintf( buf,"%s intento robarme!", PERS(ch, victim) );
	    break;
	case 3 :
	    sprintf(buf,"Quita tus manos de ahi, %s!", PERS(ch, victim) );
	    break;
        }
	if ( IS_NPC(victim) )
		REMOVE_BIT(victim->comm, COMM_NOSHOUT);
        if (!IS_AWAKE(victim))
            do_wake(victim,"");
	if (IS_AWAKE(victim))
	    do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		sprintf(buf,"$N intento robarle a %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
		if ( !IS_SET(ch->act, PLR_THIEF) )
		{
		    SET_BIT(ch->act, PLR_THIEF);
		    send_to_char( "*** #BEres ahora un #FLADRON!!#f#b ***\n\r", ch );
		    save_char_obj( ch );
		}
	    }
	}
	else
		multi_hit( victim, ch, TYPE_UNDEFINED );

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) 
    ||	 !str_cmp( arg1, "silver"))
    {
	int gold, silver;

	gold = victim->gold * number_range(1, getNivelPr(ch)) / 60;
	silver = victim->silver * number_range(1,getNivelPr(ch)) / 60;
	if ( gold <= 0 && silver <= 0 )
	{
	    send_to_char( "No pudiste sacar ninguna moneda.\n\r", ch );
	    return;
	}

	ch->gold     	+= gold;
	ch->silver   	+= silver;
	victim->silver 	-= silver;
	victim->gold 	-= gold;
	if (silver <= 0)
	    sprintf( buf, "Bingo!  Obtuviste %d monedas de oro.\n\r", gold );
	else if (gold <= 0)
	    sprintf( buf, "Bingo!  Obtuviste %d monedas de plata.\n\r",silver);
	else
	    sprintf(buf, "Bingo!  Obtuviste %d monedas de plata y %d de oro.\n\r",
		    silver,gold);

	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
	send_to_char( "No puedes encontrarlo.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > getNivelPr(ch) )
    {
	send_to_char( "No pudiste sacarselo.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "Tienes tus manos llenas.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "No puedes llevar tanto peso.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("Robas $p.",ch,objToEnt(obj),NULL,TO_CHAR);
    check_improve(ch,gsn_steal,TRUE,2);
    send_to_char( "Lo tienes!\n\r", ch );
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "No puedes hacer eso aqui.\n\r", ch );
	return NULL;
    }

    REMOVE_BIT(keeper->comm, COMM_NOCHANNELS);

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_say( keeper, "Asesinos no son bienvenidos!" );
	sprintf( buf, "%s el #BASESINO#b esta aqui!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_say( keeper, "Ladrones no son bienvenidos!" );
	sprintf( buf, "%s el #BTHIEF#b esta aqui!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }
	*/
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Lo lamento, esta cerrado. Vuelve mas tarde." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Lo lamento, esta cerrado. Vuelve manana." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "No trato con gente que no puedo ver." );
	return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj, TRUE);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
	
	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
	    	if ( obj->pIndexData == obj2->pIndexData
		&&  !str_cmp(obj->short_descr,obj2->short_descr) )
		{
			if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
				cost /= 2;
			else
				cost = cost * 3 / 4;
		}
	    }
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Comprar que?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Lo lamento, no puedes comprar eso aqui.\n\r", ch );
	    return;
	}

	pet         = alt_get_char_room( pRoomIndexNext, arg );

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Lo lamento, no vendo de ese tipo de mascotas.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("Ya tienes una mascota.\n\r",ch);
	    return;
	}

 	cost = 10 * getNivelPr(pet) * getNivelPr(pet);

	if ( (ch->silver + 100 * ch->gold) < cost )
	{
	    send_to_char( "No te alcanza el dinero.\n\r", ch );
	    return;
	}

	if ( getNivelPr(ch) < getNivelPr(pet) )
	{
	    send_to_char(
		"No eres lo suficientemente poderoso para poder dominar esta mascota.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf2,"Regateas, y logras que el precio baje a %d monedas.\n\r",cost);
	    send_to_char(buf2,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf2, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf2 );
	}

	sprintf( buf2, "%sUna etiqueta en el cuello dice 'Pertenezco a %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf2 );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Disfruta tu mascota.\n\r", ch );
	act( "$n compro $N como mascota.", ch, NULL, chToEnt(pet), TO_ROOM );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1 || number > 99)
	{
	    act("$n te dice 'Se realista!'",keeper,NULL,chToEnt(ch),TO_VICT);
	    return;
	}

	if ( obj != NULL && IS_SET( obj->extra_flags, ITEM_PROTOTIPO ) )
	{
	    act("$n te dice 'Ese es un objeto prototipo.'", keeper, NULL, chToEnt(ch),
	    	TO_VICT );
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n te dice 'No vendo eso -- intenta 'list''.",
		keeper, NULL, chToEnt(ch), TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n te dice 'No tengo tantos en stock.",
		    keeper,NULL,chToEnt(ch),TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->silver + ch->gold * 100) < cost * number )
	{
	    if (number > 1)
		act("$n te dice 'No tienes tanto dinero como para comprar esa cantidad.",
		    keeper,objToEnt(obj),chToEnt(ch),TO_VICT);
	    else
	    	act( "$n te dice 'No tienes suficiente dinero para poder comprar $p'.",
		    keeper, objToEnt(obj), chToEnt(ch), TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( obj->level > getNivelPr(ch) )
	{
	    act( "$n te dice 'No puedes usar $p todavia'.",
		keeper, objToEnt(obj), chToEnt(ch), TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "No puedes acarrear tantos items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "No puedes llevar tanto peso.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_skill(ch,gsn_haggle))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("Regateas con $N.",ch,NULL,chToEnt(keeper),TO_CHAR);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if (number > 1)
	{
	    sprintf(buf,"$n compra $p[%d].",number);
	    act(buf,ch,objToEnt(obj),NULL,TO_ROOM);
	    sprintf(buf,"Compras $p[%d] por %d monedas de plata.",number,cost * number);
	    act(buf,ch,objToEnt(obj),NULL,TO_CHAR);
	}
	else
	{
	    act( "$n compra $p.", ch, objToEnt(obj), NULL, TO_ROOM );
	    sprintf(buf,"Compras $p por %d monedas de plata.",cost);
	    act( buf, ch, objToEnt(obj), NULL, TO_CHAR );
	}
	deduct_cost(ch,cost * number);
	keeper->gold += cost * number/100;
	keeper->silver += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
	    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
	}
    }
}

void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "No puedes hacer eso aqui.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Mascotas en venta:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    getNivelPr(pet),
		    10 * getNivelPr(pet) * getNivelPr(pet),
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Lo lamento, se nos acabaron las mascotas.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Ni Preci Can] Item\n\r", ch );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
		    sprintf(buf,"[%2d %5d -- ] %s\n\r",
			obj->level,cost,obj->short_descr);
		else
		{
		    count = 1;

		    while (obj->next_content != NULL 
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
		    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "No puedes comprar nada aqui.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Vender que?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n te dice 'No tienes ese item'.",
	    keeper, NULL, chToEnt(ch), TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "No puedes soltarlo.\n\r", ch );
	SET_BIT(obj->detected, DETECTED_CURSE);
	return;
    }

    if ( IS_OBJ_STAT(obj, ITEM_FLAMING) )
    {
    	send_to_char( "No puedes vender eso.\n\r", ch );
    	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n no ve lo que le estas ofreciendo.",keeper,NULL,chToEnt(ch),TO_VICT);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n no se ve interesado en $p.", keeper, objToEnt(obj), chToEnt(ch), TO_VICT );
	return;
    }
    if ( cost > (keeper-> silver + 100 * keeper->gold) )
    {
	act("$n te dice 'Lo lamento, pero no tengo suficiente dinero para comprarte $p'.",
	    keeper,objToEnt(obj),chToEnt(ch),TO_VICT);
	return;
    }

    act( "$n vende $p.", ch, objToEnt(obj), NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("Regateas con el vendedor.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "Vendes $p por %d moneda%s de plata y %d moneda%s de oro.",
	cost - (cost/100) * 100, ( ( cost - (cost/100) * 100 ) == 1 ) ? "" : "s",
        cost/100, cost == 1 ? "" : "s" );
    act( buf, ch, objToEnt(obj), NULL, TO_CHAR );
    ch->gold     += cost/100;
    ch->silver 	 += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->gold < 0 )
	keeper->gold = 0;
    if ( keeper->silver< 0)
	keeper->silver = 0;

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj, TRUE );
    }
    else
    {
	obj_from_char( obj );
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range(50,100);
	obj_to_keeper( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Avaluar que?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n te dice 'No tienes ese item'.",
	    keeper, NULL, chToEnt(ch), TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n no ve lo que le estas ofreciendo.",keeper,NULL,chToEnt(ch),TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "No puedes soltarlo.\n\r", ch );
	SET_BIT(obj->detected, DETECTED_CURSE);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n no se ve interesado en $p.", keeper, objToEnt(obj), chToEnt(ch), TO_VICT );
	return;
    }

    sprintf( buf, 
	"$n te dice 'Te daria %d monedas de plata y %d monedas de oro por $p'.", 
	cost - (cost/100) * 100, cost/100 );
    act( buf, keeper, objToEnt(obj), chToEnt(ch), TO_VICT );
    ch->reply = keeper;

    return;
}

/*
 * Hide objects... by Maniac!
 */
void do_hide_obj(CHAR_DATA *ch, char *argument)
{
    char	arg[MAX_INPUT_LENGTH];
    int		chance;
    OBJ_DATA	*obj;

    one_argument(argument,arg);
    if ( arg[0] == '\0' )
    {
        send_to_char("Que quieres esconder?\n\r", ch);
        return;
    }

    obj = get_obj_carry(ch, arg, ch);
    if ( obj == NULL )
    {
        send_to_char("No puedes encontrarlo.\n\r", ch);
        return;
    }

    if (IS_SET(obj->extra_flags, ITEM_HIDDEN))	/* no use in hiding it again */
	    return;

    chance = number_range(1, 5);

    if (es_clase(ch, CLASS_RANGER))
	chance += 2;
    else
    if (es_clase(ch, CLASS_THIEF))
	chance++;

    if (chance > 5)			/* Let's not push it... */
	chance = 5;
    if ( IS_IMMORTAL(ch) )
	chance = 5;
    switch (chance)			/* Let's see what we've got */
    {
	case 1:
		act("$n esta a cuatro patas tratando de esconder $p.", ch, objToEnt(obj), NULL, TO_ROOM );
		act("Intentas esconder $p, pero fallas miserablemente.", ch, objToEnt(obj), NULL, TO_CHAR );
		break;
	case 2:
		act("$n esta a cuatro patas cavando en el suelo.", ch, NULL, NULL, TO_ROOM );
		act("Escondes $p, pero lo hiciste en forma obvia.", ch, objToEnt(obj), NULL, TO_CHAR );
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		SET_BIT(obj->extra_flags, ITEM_HIDDEN);
		break;
	case 3:
		act( "Fallas en esconder $p.", ch, objToEnt(obj), NULL, TO_CHAR );
		break;
	case 4:
	case 5:
		act( "Escondes $p.", ch, objToEnt(obj), NULL, TO_CHAR );
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		SET_BIT(obj->extra_flags, ITEM_HIDDEN);
		break;
    }
}

void do_search(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int found = FALSE;
    int	chance;

    buf[0] = buf2[0] = '\0';

    for ( obj = ch->in_room->contents;obj && (!found); obj = obj->next_content )
    {
        if ( IS_SET(obj->extra_flags, ITEM_HIDDEN) )
        {
		chance = number_range( 1, 5);
		if (es_clase(ch, CLASS_THIEF)
		||  es_clase(ch, CLASS_RANGER))
			chance++; 
		if (IS_IMMORTAL(ch))
		    chance = 5;
		if (chance > 3)
		{
			sprintf(buf, "Encuentras %s.\n\r", obj->short_descr);
			strcat(buf2, buf);
			REMOVE_BIT(obj->extra_flags, ITEM_HIDDEN);
			found = TRUE;
		}
        }
    }
    if (!found)
	sprintf(buf2, "No encontraste nada.\n\t");

    send_to_char(buf2, ch);
    buf2[0] = '\0';
}

/* Original Code by Todd Lair.                                        */
/* Improvements and Modification by Jason Huang (huangjac@netcom.com).*/
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void do_brew ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn, skill;

    skill = get_skill(ch, gsn_brew);

    if ( skill < 1 )
    {                                          
	send_to_char( "No sabes como preparar pociones.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Hacer una pocion de que spell?\n\r", ch );
	return;
    }

    obj = get_eq_char( ch, WEAR_HOLD );

    /* Interesting ... Most scrolls/potions in the mud have no hold
       flag; so, the problem with players running around making scrolls 
       with 3 heals or 3 gas breath from pre-existing scrolls has been 
       severely reduced. Still, I like the idea of 80% success rate for  
       first spell imprint, 25% for 2nd, and 10% for 3rd. I don't like the
       idea of a scroll with 3 ultrablast spells; although, I have limited
       its applicability when I reduced the spell->level to 1/3 and 1/4 of 
       getNivelPr(ch) for scrolls and potions respectively. --- JH */

    /* I will just then make two items, an empty vial and a parchment available
       in midgaard shops with holdable flags and -1 for each of the 3 spell
       slots. Need to update the midgaard.are files --- JH */

    if ( !obj || obj->item_type != ITEM_POTION )
    {
	send_to_char( "No estas sosteniendo un vial.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
	send_to_char( "No conoces ningun spell que se llame asi.\n\r", ch );
	return;
    }

    /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
       when you quaff a gas breath potion, and then the mobs in the room are
       hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */
  
    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
	send_to_char( "No puedes preparar ese spell.\n\r", ch );
	return;
    }

    act( "$n empieza a preparar una pocion.", ch, objToEnt(obj), NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );

    /* Check the skill percentage, fcn(wis,int,skill) */
    if ( !CHANCE(skill)
    ||   !CHANCE((get_curr_stat(ch, STAT_WIS)+get_curr_stat(ch, STAT_INT))*2) )
    {
	act( "$p explota violentamente!", ch, objToEnt(obj), NULL, TO_ALL );

        spell_acid_blast(gsn_acid_blast, LEVEL_HERO - 1, chToEnt(ch), chToEnt(ch), TARGET_CHAR );

	extract_obj( obj, TRUE );
	return;
    }

    /* took this outside of imprint codes, so I can make do_brew differs from
       do_scribe; basically, setting potion level and spell level --- JH */

    obj->level		= getNivelPr(ch)/2;
    obj->value[0]	= getNivelPr(ch)/4;

    spell_imprint(sn, getNivelPr(ch), chToEnt(ch), objToEnt(obj), TARGET_OBJ ); 
}

void do_scribe ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn, skill;

    skill = get_skill(ch, gsn_scribe);

    if ( skill < 1 )
    {                                          
	send_to_char( "No sabes escribir scrolls.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Escribir cual spell?\n\r", ch );
	return;
    }

    /* Do we have a parchment to scribe spells? */
    obj = get_eq_char( ch, WEAR_HOLD );

    if ( !obj || obj->item_type != ITEM_SCROLL )
    {
	send_to_char( "No estas sosteniendo un papel.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
	send_to_char( "No conoces ningun spell con ese nombre.\n\r", ch );
	return;
    }
    
    act( "$n empieza a escribir un scroll.", ch, objToEnt(obj), NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );

    /* Check the skill percentage, fcn(int,wis,skill) */
    if ( !CHANCE(skill)
    ||   !CHANCE((get_curr_stat(ch, STAT_WIS)+get_curr_stat(ch, STAT_INT))*2) )
    {
	act( "$p se convierte en fuego!", ch, objToEnt(obj), NULL, TO_ALL );
        spell_fireball(gsn_fireball, LEVEL_HERO - 1, chToEnt(ch), chToEnt(ch), TARGET_CHAR ); 
	extract_obj( obj, TRUE );
	return;
    }

    /* basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant weapon
       has no analogs in potion forms --- JH */

    obj->level		= getNivelPr(ch)*2/3;
    obj->value[0]	= getNivelPr(ch)/3;

    spell_imprint(sn, getNivelPr(ch), chToEnt(ch), objToEnt(obj), TARGET_OBJ ); 
}

/* Contributed by BoneCrusher of EnvyMud. */
void do_donate( CHAR_DATA *ch, char *arg )
{
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *repobj;
    OBJ_DATA *repobj_next;
    ROOM_INDEX_DATA *antes, *pit;
    int	roomvnum;
    char      arg1[MAX_INPUT_LENGTH];
    char      buf[MSL];

    arg = one_argument( arg, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Donar que?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) || !is_clan(ch) )
    	roomvnum = ROOM_VNUM_ALTAR;
    else
	roomvnum = get_clan_table(ch->clan)->pit;

    pit		= get_room_index( roomvnum );
    antes	= ch->in_room; /* donde estaba antes */

    if ( !pit || !antes )
    {
    	send_to_char( "Algo salio mal.\n\r", ch );
    	return;
    }

    char_from_room( ch );
    char_to_room( ch, pit );

    if ( ( container = get_obj_here( ch, "donation pit" ) ) == NULL )
    {
	send_to_char( "El donation pit no esta en esta realidad.\n\r", ch );
	char_from_room( ch );
	char_to_room( ch, antes );
	return;
    }

    char_from_room( ch );
    char_to_room( ch, antes );

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	if ( ( obj = get_obj_carry( ch, arg1 , ch ) ) == NULL )
	{
	    send_to_char( "No tienes ese item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "No puedes soltarlo.\n\r", ch );
	    SET_BIT(obj->detected, DETECTED_CURSE);
	    return;
	}
	
	if ( obj->item_type == ITEM_TRASH
	    || obj->item_type == ITEM_FOOD
	    || obj->item_type == ITEM_KEY
	    || obj->item_type == ITEM_PILL )
	{
	    act( "Envias $p volando hacia $P.", ch, objToEnt(obj), objToEnt(container),
		TO_CHAR );
	    extract_obj( obj, TRUE );
	    return;
	}

	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n envia $p volando hacia $P.", ch, objToEnt(obj), objToEnt(container), TO_ROOM );
	act( "Envias $p volando hacia $P.", ch, objToEnt(obj), objToEnt(container), TO_CHAR );
	send_to_room( "Un ruido metalico se escucha desde el pit!",
		     container->in_room );
    }
    else
    {
	int count = 0, glcount = 0;
	char shd[MSL];

	for ( obj = ch->carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&& can_see_obj( ch, obj )
		&& obj->wear_loc == WEAR_NONE
		&& obj != container
		&& can_drop_obj( ch, obj ) )
		{
			count	= 0;
			strcpy(shd,obj->short_descr);
			for ( repobj = obj; repobj; repobj = repobj_next )
			{
				repobj_next = repobj->next_content;

				if ( !can_see_obj( ch, repobj )
				||   repobj->wear_loc != WEAR_NONE
				||   obj->pIndexData != repobj->pIndexData
				||   !can_drop_obj( ch, repobj ) )
					continue;

				if ( repobj == obj_next )
					obj_next = repobj_next;

				count++;

				if ( repobj->item_type == ITEM_TRASH
				|| repobj->item_type == ITEM_FOOD
				|| repobj->item_type == ITEM_KEY
				|| repobj->item_type == ITEM_PILL )
				{
					extract_obj( repobj, TRUE );
					continue;
				}

				obj_from_char(repobj);
				obj_to_obj( repobj, container );
			}
			if ( count > 1 )
				sprintf( buf, "$n envia %d*%s volando hacia $P.", count, shd );
			else
				sprintf( buf, "$n envia %s volando hacia $P.", shd );
			act( buf, ch, NULL, objToEnt(container), TO_ROOM );
			if ( count > 1 )
				sprintf( buf, "Envias %d*%s volando hacia $P.", count, shd );
			else
				sprintf( buf, "Envias %s volando hacia $P.", shd );
			act( buf, ch, NULL, objToEnt(container), TO_CHAR );
			glcount += count;
		}
	}
	if (glcount > 0)
	{
		sprintf(buf, "%d ruido%s metalico%s se escucha%s desde el pit!",
		glcount, glcount == 1 ? "" : "s", glcount == 1 ? "" : "s",
		glcount == 1 ? "" : "n" );
		send_to_room( buf, container->in_room );
	}
    }

    return;
}

/* Poison weapon by Thelonius for EnvyMud */
/* Blade thirst code is a changed version of poison weapon */
/* Written by The Maniac. This skill came from the internet book */
/* The Tome of Mighty Magic */
void do_bladethirst( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *pobj;
    OBJ_DATA *wobj;
    AFFECT_DATA *paf;
    char      arg [ MAX_INPUT_LENGTH ];

    if ( get_skill(ch, gsn_bladethirst) < 1 )
    {                                          
	send_to_char( "Que crees que eres, un necromancer?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )                                              
    { send_to_char( "Que tratas de hacer...?\n\r",    ch ); return; }
    if ( ch->fighting )                                       
    { send_to_char( "Mientras peleas?  Buen intento.\n\r", ch ); return; }
    if ( !( obj = get_obj_carry( ch, arg , ch ) ) )
    { send_to_char( "No tienes esa arma.\n\r",      ch ); return; }
    if ( obj->item_type != ITEM_WEAPON )
    { send_to_char( "Eso no es un arma.\n\r",        ch ); return; }
    if ( IS_OBJ_STAT( obj, ITEM_BLADE_THIRST ) )
    { send_to_char( "Esa arma ya esta sedienta.\n\r",  ch ); return; }

 /* Now we have a valid weapon...check to see if we have the bar of mithril. */
    for ( pobj = ch->carrying; pobj; pobj = pobj->next_content )
    {
	if ( pobj->pIndexData->vnum == OBJ_VNUM_MITHRIL )
	    break;
    }
    if ( !pobj )
    {
	send_to_char( "No tienes el mithril.\n\r", ch );
	return;
    }

    /* Okay, we have the mithril...do we have blood? */
    for ( wobj = ch->carrying; wobj; wobj = wobj->next_content )
    {
	if ( wobj->item_type == ITEM_DRINK_CON
	    && wobj->value[1]  >  0
	    && wobj->value[2]  == liq_lookup( "blood" ) )
	    break;
    }
    if ( !wobj )
    {
	send_to_char( "Necesitas un poco de sangre para este skill.\n\r", ch );
	return;
    }

    /* Great, we have the ingredients...but is the ch smart enough? */
    if ( !IS_NPC( ch ) && get_curr_stat( ch , STAT_WIS ) < 17 )
    {
	send_to_char( "No recuerdas como hacerlo...\n\r", ch );
	return;
    }
    /* And does he have steady enough hands? */
    if ( !IS_NPC( ch )
	&& ( get_curr_stat( ch , STAT_DEX ) < 17
	    || ch->pcdata->condition[COND_DRUNK] > 0 ) )
    {
	send_to_char(
	"Tus manos son muy torpes como para poder mezclar los ingredientes.\n\r",
								ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_bladethirst].beats );

    /* Check the skill percentage */
    if ( !IS_NPC( ch )
	&& number_percent( ) > ch->pcdata->learned[gsn_bladethirst] )
    {
	send_to_char( "Fallas y derramas un poco sobre ti.  Ouch!\n\r",
		     ch );
	damage_old( ch, ch, getNivelPr(ch) * 2, gsn_bladethirst, DAM_ACID, TRUE );
	act( "$n derrama la mezcla en el suelo!", ch, NULL, NULL, TO_ROOM );
	extract_obj( pobj, TRUE );
	extract_obj( wobj, TRUE );
	return;
    }

    /* Well, I'm tired of waiting.  Are you? */
    act( "Mezclas $p en $P, creando una pocion maldita!",
	ch, objToEnt(pobj), objToEnt(wobj), TO_CHAR );
    act( "$n mezcla $p en $P, creando una pocion maldita!",
	ch, objToEnt(pobj), objToEnt(wobj), TO_ROOM );
    act( "Derramas la pocion sobre $p, que brilla malignamente!",
	ch, objToEnt(obj), NULL, TO_CHAR  );
    act( "$n derrama la pocion sobre $p, que brilla malignamente!",
	ch, objToEnt(obj), NULL, TO_ROOM  );
    SET_BIT( obj->extra_flags, ITEM_BLADE_THIRST );
    obj->cost *= getNivelPr(ch);

    /* Set an object timer.  Dont want proliferation of thirsty weapons */
    obj->timer = 10 + getNivelPr(ch);

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
	obj->timer *= 2;

    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
	obj->timer *= 2;

    /* WHAT?  All of that, just for that one bit?  How lame. ;) */
    act( "El resto de la pocion corroe $p, que se desvanece.",
	ch, objToEnt(wobj), NULL, TO_CHAR );
    act( "El resto de la pocion corroe $p, que se desvanece.",
	ch, objToEnt(wobj), NULL, TO_ROOM );
    extract_obj( pobj, TRUE );
    extract_obj( wobj, TRUE );

    paf			= new_affect();
    paf->type           = -1;
    paf->duration       = -1;
    paf->location       = APPLY_HITROLL;
    paf->modifier       = 3;
    paf->bitvector      = 0;
    paf->next           = obj->affected;
    obj->affected       = paf;

    return;
}

/* Study skill... converted from Rom by Maniac */
void do_study( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *scroll;

    argument = one_argument(argument, arg1);

    if (( scroll = get_obj_carry(ch,arg1,ch)) == NULL)
    {
        send_to_char("No tienes ese scroll.\n\r",ch);
        return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
        send_to_char("Solo puedes estudiar scrolls.\n\r",ch);
        return;
    }

    if ( getNivelPr(ch) < scroll->level )
    {
        send_to_char("No eres de nivel lo suficientemente alto para poder estudiar este scroll.\n\r", ch );
        return;
    }

    if ( get_skill(ch, scroll->value[1]) < 1 )
    {
        send_to_char("Tu clase no puede aprender este spell.\n\r",ch);
        return;
    }

    if ( ch->pcdata->learned[scroll->value[1]] > 0 )
    {
        send_to_char("Tu ya conoces ese spell!\n\r",ch);
        return;
    }

    act("$n estudia $p.",ch,objToEnt(scroll),NULL,TO_ROOM);
    act("Estudias $p.",ch,objToEnt(scroll),NULL,TO_CHAR);

    if (number_percent() >= (20 + ch->pcdata->learned[gsn_scrolls]) * 4/5)
    {
        send_to_char("Te equivocas y el scroll se desvanece!\n\r",ch);
        act("$n lanza un grito de furia.",ch,NULL,NULL,TO_ROOM);
        check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
        act("Aprendiste el spell!",ch,NULL,NULL,TO_CHAR);
        act("$n aprendio el spell!",ch,NULL,NULL,TO_ROOM);
        ch->pcdata->learned[scroll->value[1]] = 5;
        ch->pcdata->points += 3; 
        check_improve(ch,gsn_scrolls,TRUE,2);
    }
    extract_obj(scroll, TRUE);
    return;
}

/* put an item on auction, or see the stats on the current item or bet */
void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

    if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
	return;

    if (arg1[0] == '\0')
    {
	if (auction->item != NULL)
        {
            /* show item data here */
            if (auction->bet > 0)
                sprintf (buf, "La apuesta actual en este item es de #B%d#b MP.\n\r",auction->bet);
            else
                sprintf (buf, "No se han recibido apuestas para este item.\n\r");
            send_to_char (buf,ch);
            spell_identify (0, LEVEL_HERO - 1, chToEnt(ch), objToEnt(auction->item), TARGET_OBJ ); /* uuuh! */
            return;
        }
        else
        {
            send_to_char ("Rematar que?\n\r",ch);
            return;
        }
    }

    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    {
	if (auction->item == NULL)
	{
		send_to_char ("No hay remate para detener.\n\r",ch);
		return;
	}
	else /* stop the auction */
	{
		sprintf (buf,"La venta de %s ha sido detenida por Dios. Item confiscado.",
                        auction->item->short_descr);
		talk_auction (buf);
		obj_to_char (auction->item, ch);
		auction->item = NULL;
		if (auction->buyer != NULL) /* return money to the buyer */
		{
			auction->buyer->silver += auction->bet;
			send_to_char ("Tu dinero ha sido devuelto.\n\r",auction->buyer);
		}
		return;
	}
    }

    if (!str_cmp(arg1,"bet") )
    {
        if (auction->item != NULL)
        {
            int newbet;

            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("Apostar cuanto?\n\r",ch);
                return;
            }

            if ( ch == auction->seller )
            {
                send_to_char ("No puedes apostar en este remate.\n\r", ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
/*	    sprintf( buf, "Bet: %d", newbet);
            log_string (buf); */

            if (newbet < (auction->bet + 100))
            {
                send_to_char ("Debes subir la apuesta en al menos 100 MP.\n\r",ch);
                return;
            }

            if (newbet > DINERO(ch))
            {
                send_to_char ("No tienes esa cantidad de dinero!\n\r",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the silver to the last buyer, if one exists */
            if (auction->buyer != NULL)
                auction->buyer->silver += auction->bet;

            deduct_cost( ch, newbet ); /* substract the silver - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"Una apuesta de %d MP ha sido puesta en %s.",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;
        }
        else
        {
            send_to_char ("No hay ningun objeto en remate.\n\r",ch);
            return;
        }
    }

/* finally... */

    if ( !str_cmp(arg1, "on") )
    {
    	send_to_char( "Canal de remates ahora esta #BON#b.\n\r", ch );
    	REMOVE_BIT(ch->comm, COMM_NOAUCTION);
    	return;
    }
    
    if ( !str_cmp(arg1, "off") )
    {
    	send_to_char( "Canal de remates ahora esta #BOFF#b.\n\r", ch );
    	SET_BIT(ch->comm, COMM_NOAUCTION);
    	return;
    }

/*    obj = get_obj_list (ch, arg1, ch->carrying); */ /* does char have the item ? */

    if ( IS_SET(ch->act, PLR_NOAUCTION) )
    {
    	send_to_char( "No puedes rematar objetos todavia. Espera unos minutos.\n\r", ch );
    	return;
    }

    obj = get_obj_carry( ch, arg1, ch );

    if (obj == NULL)
    {
        send_to_char ("No estas llevando eso.\n\r",ch);
        return;
    }

    if (obj->clan)
    {
    	send_to_char ("No puedes rematar un objeto de clan.\n\r", ch );
    	return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act ("No puedes rematar $Ts.",ch, NULL, strToEnt(item_name (obj->item_type),ch->in_room), TO_CHAR);
        return;

    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_SCROLL:
        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0;
        auction->buyer = NULL;
        auction->seller = ch;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf (buf, "Un nuevo item ha sido recibido: %s.", obj->short_descr);
        talk_auction (buf);

	SET_BIT(ch->act, PLR_NOAUCTION);

	char_event_add(ch,MINUTOS(10)*PULSE_PER_SECOND,(void *) PLR_NOAUCTION,
		actremove );

        return;
    } /* switch */
    else
    {
        act ("Intentalo despues - $p esta siendo rematado justo ahora!",ch,objToEnt(auction->item),NULL,TO_CHAR);
        return;
    }
}

/*
 * Reparador.
 */
CHAR_DATA *find_reparador( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *reparador;
    REPAIR_DATA *pRepair;

    pRepair = NULL;
    for ( reparador = ch->in_room->people; reparador; reparador = reparador->next_in_room )
    {
	if ( IS_NPC(reparador) && (pRepair = reparador->pIndexData->pRepair) != NULL )
	    break;
    }

    if ( pRepair == NULL )
    {
	send_to_char( "No puedes hacer eso aqui.\n\r", ch );
	return NULL;
    }

    REMOVE_BIT(reparador->comm, COMM_NOCHANNELS);

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_say( reparador, "Asesinos no son bienvenidos!" );
	sprintf( buf, "%s el #BASESINO#b esta aqui!\n\r", ch->name );
	do_yell( reparador, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_say( reparador, "Ladrones no son bienvenidos!" );
	sprintf( buf, "%s el #BLADRON#b esta aqui!\n\r", ch->name );
	do_yell( reparador, buf );
	return NULL;
    }
	*/
    /*
     * Repair hours.
     */
    if ( time_info.hour < pRepair->open_hour )
    {
	do_say( reparador, "Lo lamento, esta cerrado. Vuelve mas tarde." );
	return NULL;
    }
    
    if ( time_info.hour > pRepair->close_hour )
    {
	do_say( reparador, "Lo lamento, esta cerrado. Vuelve manana." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( reparador, ch ) )
    {
	do_say( reparador, "No trato con gente que no puedo ver." );
	return NULL;
    }

    return reparador;
}

void reparar_obj(CHAR_DATA *ch, CHAR_DATA *reparador, OBJ_DATA *obj, bool fAll)
{
	int i;

	if ( !obj )
		return;

	if ( fAll )
	{
		if ( obj->contains )
			reparar_obj(ch, reparador, obj->contains, TRUE);
		if ( obj->next_content )
			reparar_obj(ch, reparador, obj->next_content, TRUE);
	}

	if ( obj->condition > 89 )
	{
		if ( !fAll )
			send_to_char( "Ese objeto esta en excelente condicion!\n\r", ch );
		return;
	}

	for ( i = 0; i < MAX_TRADE; ++i )
		if (reparador->pIndexData->pRepair->repair_type[i] == obj->item_type)
			break;

	if ( i == MAX_TRADE )
	{
		if ( !fAll )
			send_to_char( "Lo siento, no se como reparar eso.\n\r", ch );
		return;
	}

	i = (obj->cost * ( 100 - obj->condition ) / 100) * 1.2;

	if ( i < 50 )
		i = obj->level * 25;

	if ( i > DINERO(ch))
	{
		if ( !fAll )
			send_to_char( "Lo lamento, no tienes suficiente dinero.\n\r", ch );
		return;
	}

	deduct_cost( ch, i );

	obj->condition = UMIN( 90 + (int) ( ( getNivelPr(reparador) / 60.0 ) * 10 )
		+ number_range(0, 3), 100 );

	act( "$n repara $p y te lo entrega.", reparador, objToEnt(obj), chToEnt(ch), TO_VICT );
	act( "$n repara $p y se lo entrega a $N.", reparador, objToEnt(obj), chToEnt(ch), TO_NOTVICT );
}

void do_reparar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *reparador;
	OBJ_DATA *obj;
	
	if ( ch->in_room == NULL )
		return;
	
	if ( !argument || (argument && argument[0] == '\0') )
	{
		send_to_char( "Que objeto quieres reparar?\n\r", ch );
		return;
	}

	if ( (reparador = find_reparador(ch)) == NULL )
		return;

	if ( !ch->carrying )
	{
		act( "$n te dice 'No tienes nada para reparar.", reparador, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	if ( str_cmp(argument, "all") && str_cmp(argument, "todo") )
	{
		if ( (obj = get_obj_carry( ch, argument, ch )) == NULL )
		{
			send_to_char( "No estas llevando eso!\n\r", ch );
			return;
		}

		reparar_obj(ch, reparador, obj, FALSE);
	}
	else
		reparar_obj(ch, reparador, ch->carrying, TRUE);
}

void do_cotizar( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char buf[MSL];
	CHAR_DATA *reparador;
	REPAIR_DATA *pRepair;
	int i;
	
	if ( !ch->in_room )
		return;
	
	if ( !argument || (argument && argument[0] == '\0') )
	{
		send_to_char( "Que objeto quieres cotizar?\n\r", ch );
		return;
	}

	if ( ( reparador = find_reparador( ch ) ) == NULL )
		return;
	
	pRepair = reparador->pIndexData->pRepair;
	
	if ( (obj = get_obj_carry( ch, argument, ch )) == NULL )
	{
		send_to_char( "No estas llevando eso!\n\r", ch );
		return;
	}
	
	for ( i = 0; i < MAX_TRADE; ++i )
	{
		if (pRepair->repair_type[i] == obj->item_type)
			break;
	}
	
	if ( i == MAX_TRADE )
	{
		act( "$n te dice 'Lo siento, no se como reparar eso'.", reparador, NULL, chToEnt(ch), TO_VICT );
		return;
	}
	
	if ( obj->condition > 89 )
	{
		act( "$n te dice 'Ese objeto esta en excelente condicion!'.", reparador, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	i = (obj->cost * ( 100 - obj->condition ) / 100) * 1.2;

	if ( i < 50 )
		i = obj->level * 25;

	sprintf( buf, "$n te dice 'Reparar $p te costara %d monedas de plata'.", i );
	act( buf, reparador, objToEnt(obj), chToEnt(ch), TO_VICT );

	return;
}

void afilar_arma( EVENT *ev )
{
	OBJ_DATA *arma = ev->item.obj;
	CHAR_DATA *ch;
	AFFECT_DATA af;
	
	switch( (int) ev->param )
	{
		case 0:
		if ( (ch = arma->carried_by) )
		{
			act( "Terminas de afilar $p.", ch, objToEnt(arma), NULL, TO_CHAR );
			act( "$n termina de afilar $p.", ch, objToEnt(arma), NULL, TO_ROOM );
		}
		af.where	= TO_WEAPON;
		af.type		= gsn_afilar;
		af.duration	= 6 + (getNivelPr(ch) / 6);
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= WEAPON_SHARP;
		af.level	= getNivelPr(ch);
		affect_to_obj( arma, &af );
		break;

		case 1:
		if ( (ch = arma->carried_by) )
		{
			act( "Terminas de afilar $p, pero no notas ningun cambio.", ch, objToEnt(arma), NULL, TO_CHAR );
			act( "$n termina de afilar $p.", ch, objToEnt(arma), NULL, TO_ROOM );
		}
		arma->condition = UMAX(1, arma->condition - 5);
		break;
		
		case 2:
		if ( (ch = arma->carried_by) )
		{
			act( "Whoops! al estar afilando $p, se rompio!", ch, objToEnt(arma), NULL, TO_CHAR );
			act( "Whoops! $p se quebro mientras $p estaba afilando.", ch, objToEnt(arma), NULL, TO_ROOM );
		}
		extract_obj( arma, TRUE );
		break;
	}
	return;
}

void do_afilar( CHAR_DATA *ch, char *argument )
{
	int chance;
	OBJ_DATA *obj, *arma;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Que quieres afilar?\n\r", ch );
		return;
	}
	
	if ( (chance = get_skill(ch,gsn_afilar)) < 1 )
	{
		send_to_char( "No sabes como afilar.\n\r", ch );
		return;
	}
	
	if ( (arma = get_obj_carry(ch, argument, ch)) == NULL )
	{
		send_to_char( "No estas llevando eso.\n\r", ch );
		return;
	}
	
	if ( arma->item_type != ITEM_WEAPON )
	{
		send_to_char( "Eso no es un arma!\n\r", ch );
		return;
	}

	if ( IS_WEAPON_STAT(arma, WEAPON_SHARP) )
	{
		send_to_char( "Esa arma ya esta afilada.\n\r", ch );
		return;
	}

	for ( obj = ch->carrying; obj; obj = obj->next_content )
		if ( obj->item_type == ITEM_PEDERNAL )
			break;
	
	if ( !obj )
	{
		send_to_char( "Necesitas un pedernal para poder afilar.\n\r", ch );
		return;
	}
	
	WAIT_STATE(ch,skill_table[gsn_afilar].beats);
	act( "Empiezas a afilar $p con $P.", ch, objToEnt(arma), objToEnt(obj), TO_CHAR );
	act( "$n empieza a afilar $p con $P.", ch, objToEnt(arma), objToEnt(obj), TO_ROOM );
	extract_obj( obj, TRUE );

	if ( chance > number_percent() ) /* exito */
	{
		SET_BIT(obj->detected, DETECTED_SHARP);
		obj_event_add( arma, skill_table[gsn_afilar].beats, 0, afilar_arma );
		check_improve( ch, gsn_afilar, TRUE, 1 );
	}
	else /* fracaso */
	{
		if ( CHANCE(chance*2) ) /* no pasa nada */
			obj_event_add( arma, skill_table[gsn_afilar].beats, (int *)1, afilar_arma );
		else /* se pierde el arma */
			obj_event_add( arma, skill_table[gsn_afilar].beats, (int *)2, afilar_arma );
		check_improve( ch, gsn_afilar, FALSE, 1 );
	}

	return;
}

void do_identify( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	int costo;

	if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
	{
		send_to_char( "No estas llevando eso.\n\r", ch );
		return;
	}

	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
		if ( IS_NPC(rch)
		&&   IS_MOB_MAGIC(rch)
		&&   rch->position == POS_STANDING
		&&   IS_FORM(rch, FORM_SENTIENT)
		&&  !HATES(rch, ch)
		&&   getNivelPr(rch) >= obj->level )
			break;

	if (!rch)
	{
		send_to_char("Parece que nadie sabe mucho acerca de eso.\n\r", ch);
		return;
	}

	if ( getNivelPr(rch) < obj->level )
	{
		act( "$n te dice 'Mis poderes son insuficientes para poder identificar $p'.",
			rch, objToEnt(obj), chToEnt(ch), TO_VICT );
		return;
	}

	costo = getNivelPr(ch) * getNivelPr(ch) * 10 + obj->level * 100 + 50;

	if (IS_IMMORTAL(ch))
		act( "$n te mira y sus ojos brillan!", rch, objToEnt(obj), chToEnt(ch), TO_VICT );
	else if (DINERO(ch) < costo)
	{
		act( "$n sigue su camino sin mirar a $p.",
			rch, objToEnt(obj), 0, TO_ROOM );
		return;
	}
	else
	{
		deduct_cost(ch, costo);
		send_to_char("Tu bolsillo se siente mas liviano.\n\r", ch);
	}

	act( "$n toma $p y bota algunos huesos al suelo.",
		rch, objToEnt(obj), 0, TO_ALL );
	act( "En el humo, alcanzas a leer:", ch, NULL, NULL, TO_CHAR );
	spell_identify( 0, LEVEL_HERO - 1, chToEnt(ch), objToEnt(obj), TARGET_OBJ );
}

void do_rezar( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *lugar;
	OBJ_DATA *obj;

	if ( IS_NPC(ch) || !is_clan(ch) )
	{
		send_to_char( "Debes estar en un clan para poder rezar a tu dios.\n\r", ch );
		return;
	}

	if ( ES_INDEP(ch->clan) )
	{
		send_to_char( "No crees en Dios, como piensas rezarle?\n\r", ch );
		return;
	}

	if ( (IS_GOOD(ch)    && IS_CLAN_NOGOOD(ch->clan))
	||   (IS_EVIL(ch)    && IS_CLAN_NOEVIL(ch->clan))
	||   (IS_NEUTRAL(ch) && IS_CLAN_NONEUTRAL(ch->clan)) )
	{
		act( "$T te ignora.", ch, NULL, strToEnt(CLAN_GOD(ch),ch->in_room), TO_CHAR );
		return;
	}

	if ( !IS_SET(ch->in_room->room_flags, ROOM_SANTUARIO ) )
	{
		send_to_char( "Este lugar no es lo suficientemente puro.\n\r", ch );
		return;
	}

	if ( (obj = ch->pcdata->corpse) == NULL )
	{
		send_to_char( "No tienes porque rezar.\n\r", ch );
		return;
	}

	act( "$n empieza a rezarle a su dios $T.", ch, NULL, strToEnt(CLAN_GOD(ch),ch->in_room), TO_ROOM );
	act( "Empiezas a rezarle a tu dios $T.", ch, NULL, strToEnt(CLAN_GOD(ch),ch->in_room), TO_CHAR );

	WAIT_STATE(ch,12);

	if (obj->carried_by)
		obj_from_char( obj );
	else
	if ( obj->in_obj )
		obj_from_obj( obj );
	else
	if ( obj->in_room )
		obj_from_room( obj );

	obj_to_room( obj, (lugar = get_room_index(MORGUE(ch))) );

	if ( lugar->people )
		act( "$p aparece en el cuarto.", lugar->people, objToEnt(obj), NULL, TO_ALL );

	return;
}

void do_trozar( CHAR_DATA *ch, char *argument )
{
	int cnt = 0, chance;
	OBJ_DATA *corpse, *obj;
	char cnts[MIL];

	if ( (chance = get_skill(ch, gsn_trozar)) < 1 )
	{
		send_to_char( "Trozar? Que es eso?\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Que quieres trozar?\n\r", ch );
		return;
	}

	corpse = get_obj_here( ch, argument );

	if ( corpse == NULL )
	{
		act( "No encuentras ningun $T por aqui.", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
		return;
	}

	if ( corpse->item_type != ITEM_CORPSE_NPC )
	{
		act( "No puedes trozar $T.", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
		return;
	}

	if ( corpse->contains )
	{
		act( "No creo que quieras comer bistecs con trozos de $P!",
			ch, NULL, objToEnt(corpse->contains), TO_CHAR );
		return;
	}

	if ( !CHANCE(chance) )
	{
		act( "$n intenta trozar $P, pero hace mal el corte, destrozando el cuerpo.", ch, NULL, objToEnt(corpse), TO_ROOM );
		act( "Whoops! Haces mal un corte y $P se despedaza.", ch, NULL, objToEnt(corpse), TO_CHAR );
		extract_obj( corpse, TRUE );
		return;
	}

	if ( IS_SET(corpse->value[2], PART_HEAD) )
		cnt++;
	if ( IS_SET(corpse->value[2], PART_ARMS) )
		cnt += 2;
	if ( IS_SET(corpse->value[2], PART_LEGS) )
		cnt += 2;
	if ( IS_SET(corpse->value[2], PART_HEART) )
		cnt++;
	if ( IS_SET(corpse->value[2], PART_WINGS) )
		cnt++;
	if ( IS_SET(corpse->value[2], PART_TAIL) )
		cnt++;

	if ( !CHANCE(chance) )
		cnt /= number_fuzzy(2);

	cnt = UMAX(1,cnt);

	sprintf( cnts, "%d", cnt );
	act( "$n troza $P, obteniendo $t bistecs.", ch, strToEnt(cnts,ch->in_room), objToEnt(corpse), TO_ROOM );
	act( "Trozas $P, obteniendo $t bistecs.", ch, strToEnt(cnts,ch->in_room), objToEnt(corpse), TO_CHAR );

	while (cnt-- > 0)
	{
		obj		= create_object( get_obj_index(OBJ_VNUM_BISTEC), 0 );
		obj->timer	= 10;
		obj->cost	= 0;
		if (IS_SET(corpse->value[1], CONT_CUERPO_POISON))
			SET_BIT(obj->value[3], FOOD_POISON);
		obj_to_char( obj, ch );
	}

	WAIT_STATE(ch, skill_table[gsn_trozar].beats );

	extract_obj( corpse, TRUE );

	return;
}

void do_beber( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Que quieres beber?\n\r", ch );
		return;
	}

	if ( (obj = get_obj_carry( ch, argument, ch )) == NULL
	&&   (obj = get_obj_type_list(ITEM_FOUNTAIN, ch->in_room->contents)) == NULL )
	{
		act( "No estas llevando ningun $t.", ch, strToEnt(argument,ch->in_room), NULL, TO_CHAR );
		return;
	}

	if ( obj->item_type == ITEM_POTION )
		do_quaff( ch, argument );
	else
	if ( obj->item_type == ITEM_DRINK_CON
	||   obj->item_type == ITEM_FOUNTAIN )
		do_drink( ch, argument );
	else
		act( "No puedes beber $t.", ch, strToEnt(argument,ch->in_room), NULL, TO_CHAR );

	return;
}

void nuke_corpse( char * name, bool bak )
{
	FILE *fp;
	char buf[MIL];

	sprintf( buf, "%s%s", CORPSE_DIR, name );

	if ( (fp = fopen(buf,"r")) )
	{
		fclose(fp);
		unlink(buf);
	}

	if ( !bak )
		return;

	sprintf( buf, "%s%s", CORPSE_BAK_DIR, name );

	if ( (fp = fopen( buf, "r")) )
	{
		fclose(fp);
		unlink(buf);
	}
}
