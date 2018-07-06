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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );

void acid_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
     {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ) /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	 if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		msg = "$p se pudre y se disuelve.";
		break;
	    case ITEM_ARMOR:
		msg = "$p se corroe y se quiebra.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p se disuelve.";
	 	break;
	    case ITEM_STAFF:
	    case ITEM_WAND:
		chance -= 10;
		msg = "$p se corroe y se quiebra.";
		break;
	    case ITEM_SCROLL:
		chance += 10;
		msg = "$p se disuelve y se desvanece.";
		break; 
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,objToEnt(obj),NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,objToEnt(obj),NULL,TO_ALL);

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    AFFECT_DATA *paf;
	    bool af_found = FALSE;
	    int i;

 	    affect_enchant(obj);

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
            {
                if ( paf->location == APPLY_AC)
                {
                    af_found = TRUE;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX(paf->level,level);
		    break;
                }
            }
 
            if (!af_found)
            /* needs a new affect */
            {
		paf = new_affect();
 
                paf->type       = -1;
                paf->level      = level;
                paf->duration   = -1;
                paf->location   = APPLY_AC;
                paf->modifier   =  1;
                paf->bitvector  = 0;
                paf->next       = obj->affected;
                obj->affected   = paf;
            }
 
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
	}

	/* get rid of the object */
	if (obj->contains)  /* dump contents */
	{
	    for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)
		    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj, TRUE);
		    continue;
		}

		acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
	    }
 	}

	extract_obj(obj, TRUE);
	return;
    }
}


void cold_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
 
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR) /* whack a character */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* chill touch effect */
	if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
	{
	    AFFECT_DATA af;

            act("$n se pone azul y empieza a tiritar.",victim,NULL,NULL,TO_ROOM);
	    act("Un frio profundo penetra hasta tus huesos.",victim,NULL,NULL,TO_CHAR);
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("chill touch");
            af.level     = level;
            af.duration  = 6;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = 0;
            affect_join( victim, &af );
	}

	/* hunger! (warmth sucked out */
	if (!IS_NPC(victim))
	    if (gain_condition(victim,COND_HUNGER,dam/20) == victdead)
	    	return;

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    cold_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
   }

   if (target == TARGET_OBJ) /* toast an object */
   {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	    case ITEM_POTION:
		msg = "$p se congela y se quiebra!";
		chance += 25;
		break;
	    case ITEM_DRINK_CON:
		msg = "$p se congela y se rompe!";
		chance += 5;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,objToEnt(obj),NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,objToEnt(obj),NULL,TO_ALL);

	extract_obj(obj, TRUE);
	return;
    }
}
   


void fire_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    fire_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }
 
    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* chance of blindness */
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
	{
            AFFECT_DATA af;
            act("El humo enceguece a $n!",victim,NULL,NULL,TO_ROOM);
            act("Tus ojos se irritan por el humo...no ves nada!",
		victim,NULL,NULL,TO_CHAR);
	 
            af.where        = TO_AFFECTS;
            af.type         = gsn_fire_breath;
            af.level        = level;
            af.duration     = number_range(0,level/10);
            af.location     = APPLY_HITROLL;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;
            af.caster_id    = victim->id; // FIXME
 
            affect_to_char(victim,&af);
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    if (gain_condition(victim,COND_THIRST,dam/20) == victdead)
	    	return;

	/* let's toast some gear! */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    fire_effect(obj,level,dam,TARGET_OBJ);
        }
	return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

    	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
        ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
            return;
 
        chance = level / 4 + dam / 10;
 
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch ( obj->item_type )
        {
        default:             
	    return;
        case ITEM_CONTAINER:
            msg = "$p se incinera y se quema!";
            break;
        case ITEM_POTION:
            chance += 25;
            msg = "$p hierve y explota!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p se enciende y se quema!";
            break;
        case ITEM_STAFF:
            chance += 10;
            msg = "$p se hace humo!";
            break;
        case ITEM_WAND:
            msg = "$p se calienta y se quiebra!";
            break;
        case ITEM_FOOD:
            msg = "$p se quema!";
            break;
        case ITEM_PILL:
            msg = "$p se derrite!";
            break;
        }

        chance = URANGE(5,chance,95);

        if (number_percent() > chance)
            return;
 
	if (obj->carried_by != NULL)
            act( msg, obj->carried_by, objToEnt(obj), NULL, TO_ALL );
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,objToEnt(obj),NULL,TO_ALL);

        if (obj->contains)
        {
            /* dump the contents */
 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
		if (obj->in_room != NULL)
                    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj, TRUE);
		    continue;
		}
		fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
            }
        }
 
        extract_obj( obj, TRUE );
	return;
    }
}

void poison_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
 
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }
 
    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of poisoning */
        if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON))
        {
	    AFFECT_DATA af;

            send_to_char("Sientes el veneno corriendo por tus venas.\n\r",
                victim);
            act("$n se ve muy enfermo.",victim,NULL,NULL,TO_ROOM);

            af.where     = TO_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = level / 2;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    poison_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)  /* do some poisoning */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	
	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
  	||  IS_OBJ_STAT(obj,ITEM_BLESS)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;
	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_FOOD:
		break;
	    case ITEM_DRINK_CON:
		if (obj->value[0] == obj->value[1])
		    return;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	SET_BIT(obj->value[3], FOOD_POISON); /* sirve para liqcont tb */
	return;
    }
}

void shock_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
	{
	    send_to_char("Tus musculos dejan de responder.\n\r",victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	/* toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) /2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	   case ITEM_WAND:
	   case ITEM_STAFF:
		chance += 10;
		msg = "$p se sobrecarga y explota!";
		break;
	   case ITEM_JEWELRY:
		chance -= 10;
		msg = "$p se funde en escoria.";
	}
	
	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,objToEnt(obj),NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,objToEnt(obj),NULL,TO_ALL);

	extract_obj(obj, TRUE);
	return;
    }
}

void water_effect(CHAR_DATA *ch)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE)
			switch(obj->material)
			{
				case MAT_METAL:
				obj->humedad++;
				break;
			}
}
