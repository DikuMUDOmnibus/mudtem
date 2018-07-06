
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
#include <string.h>
#include <time.h>
#include <limits.h>
#include "merc.h"
#include "music.h"
#include "recycle.h"
#include "tables.h"
#include <ctype.h>
#include "events.h"
#include "math.h"
#include "smart.h"
#include "lookup.h"
#include "special.h" /* taxi */
#include "auction.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit          );
DECLARE_DO_FUN(do_echo          );
DECLARE_DO_FUN(do_info          );
DECLARE_DO_FUN(do_emote         );
DECLARE_DO_FUN(do_open          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_informar	);
DECLARE_DO_FUN(do_cast		);
DECLARE_DO_FUN(do_sleep		);
DECLARE_SPELL_FUN( spell_null   );
DECLARE_SPELL_FUN( spell_fireball );
DECLARE_SPELL_FUN( spell_acid_blast );
void event_update( void );
int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
	       int depth, int in_zone );
void dead_update( void );
void shutdown_mud_now( CHAR_DATA * );


/*
 * Local functions.
 */
int     hit_gain        args( ( CHAR_DATA *ch ) );
int     mana_gain       args( ( CHAR_DATA *ch ) );
int     move_gain       args( ( CHAR_DATA *ch ) );
void    mobile_update   args( ( void ) );
void    weather_update  args( ( void ) );
void    tele_update     args( ( void ) );
void	mf_update	args( ( void ) ); /* smart.c */
bool	puede_deteriorarse	args( ( OBJ_DATA * ) );

/* used for saving */

int     save_number = 0;

extern  const   sh_int  rev_dir[];
extern  char *  const dir_name[];

extern	int	RACE_ZOMBIE;

extern  AFFECT_DATA     *new_affect( void );

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    if (!IS_NPC(ch)) 
	     ch->pcdata->last_level = ( ch->played + (int) (current_time - ch->logon) ) / 3600;

    add_hp      = con_app[get_curr_stat(ch,STAT_CON)].hitp + number_range(
		    class_table[getClasePr(ch)].hp_min,
		    class_table[getClasePr(ch)].hp_max );
    add_mana    = number_range(2,(2*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/5);
    if (!class_table[getClasePr(ch)].fMana)
	add_mana /= 2;
    add_move    = number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac    = wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp      = UMAX(  2, add_hp   );
    add_mana    = UMAX(  2, add_mana );
    add_move    = UMAX(  6, add_move );

    ch->max_hit         += add_hp;
    ch->max_mana        += add_mana;
    ch->max_move        += add_move;
    ch->practice        += add_prac;
    ch->train           += 1 + (get_curr_stat(ch,STAT_INT) > 21) + (get_curr_stat(ch,STAT_INT) > 24);
    ch->exp		 = EXP_NIVEL(ch, getNivelPr(ch));

    if (!IS_NPC(ch))
    {
	ch->pcdata->learn   += 1;
	free_string(ch->pcdata->who_text);
	ch->pcdata->who_text = str_dup( "@" );

	ch->pcdata->perm_hit        += add_hp;
	ch->pcdata->perm_mana       += add_mana;
	ch->pcdata->perm_move       += add_move;

	strip_mem_char( ch, MEM_QUEST );
	strip_mem_char( ch, MEM_QUEST_COMPLETO );
    }

    if (!hide)
    {
	sprintf(buf,
	    "Ganas %d hit point%s, %d mana, %d move, y %d practica%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
	    add_prac, add_prac == 1 ? "" : "s");
	send_to_char( buf, ch );
    }
    return;
}   

void level_up( CHAR_DATA * ch )
{
	char buf[MIL];
	LEVEL_DATA *lev;

	send_to_char( "#B#F#USubiste de nivel!!!#n\n\r", ch );
	sprintf (buf,"%s subio de nivel, felicitaciones\n\r", SELFPERS(ch) );
	do_info( NULL , buf );
	for ( lev = ch->level_data; lev; lev = lev->next )
		lev->nivel++;
	sprintf(buf,"%s subio a nivel %d", ch->name, getNivelPr(ch));
	log_string(buf);
	sprintf(buf,"$N ha alcanzado el nivel %d!",getNivelPr(ch));
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level(ch,FALSE);
	if ( !IS_NPC(ch) )
		save_char_obj(ch);
}

void gain_exp( CHAR_DATA *ch, int gain )
{
    extern bool destruccion;
    bool temp = TRUE;
    int minexp;

    if (getNivelPr(ch) >= LEVEL_HERO)
	return;

    if (destruccion)
    	return;

    minexp = EXP_NIVEL(ch,1);

    ch->exp = UMAX( minexp, ch->exp + gain );

    while ( getNivelPr(ch) < LEVEL_HERO
    &&      ch->exp >= EXP_NIVEL(ch, getNivelPr(ch)+1)
    &&      temp )
    {
#if defined(OLD_LEVEL)
	char buf[MAX_STRING_LENGTH];

	send_to_char( "#B#F#USubiste de nivel!!!#n\n\r", ch );
	sprintf (buf,"%s subio de nivel, felicitaciones\n\r", PERS(ch, ch) );
	do_info( NULL , buf );
	getNivelPr(ch) += 1;
	sprintf(buf,"%s subio a nivel %d", ch->name, getNivelPr(ch));
	log_string(buf);
	sprintf(buf,"$N ha alcanzado el nivel %d!",getNivelPr(ch));
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level(ch,FALSE);
	save_char_obj(ch);
#else
	if ( !IS_NPC(ch) )
	{
		send_to_char( "Alcanzaste la cantidad de puntos para poder subir de nivel!!!\n\r", ch );
		if ( getNivelPr(ch) < 6 )
			send_to_char( "Debes ir a tu culto y usar el comando #BNIVEL#b para subir.\n\r", ch );
		temp = FALSE;
	}
	else
		level_up( ch );
#endif
    }

    return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
	int gain;
	int number;

	if (ch->in_room == NULL)
		return 0;

	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + getNivelPr(ch)/2);
	gain += class_table[IS_NPC(ch) ? CLASS_WARRIOR : getClasePr(ch)].hp_max - 10;
	number = number_percent();

	if (number < get_skill(ch,gsn_fast_healing))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	if ( IS_AFFECTED(ch, AFF_REGENERATION) )
		gain *= 2;

	switch ( ch->position )
	{
	    default:            gain /= 3;                      break;
	    case POS_SLEEPING:                                  break;
	    case POS_RESTING:   gain /= 2;                      break;
	    case POS_FIGHTING:  gain /= 5;			break;
	}

	if ( !IS_NPC(ch) )
	{
		if (ch->pcdata->condition[COND_HUNGER]	== 0 )
			gain /= 2;

		if ( ch->pcdata->condition[COND_THIRST]	== 0 )
			gain /= 2;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO))
		gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL
	&& (ch->on->item_type == ITEM_FURNITURE)
	&& !IS_OBJ_STAT(ch->on, ITEM_PROTOTIPO) )
		gain = gain * ch->on->value[3] / 100;

	if ( IS_AFFECTED(ch, AFF_POISON) )
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
		gain /= 2;

	if ( ch->position != POS_FIGHTING )
		gain *= 3;

	return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
	int gain;
	int number;

	if (ch->in_room == NULL || (!IS_NPC(ch) && IS_SET(ch->act, PLR_MAL_ALIGN)) )
		return 0;

	gain = (get_curr_stat(ch,STAT_WIS) + get_curr_stat(ch,STAT_INT) + getNivelPr(ch)) / 2;
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
		check_improve(ch,gsn_meditation,TRUE,8);
	}

	if ( !IS_NPC(ch) && !class_table[getClasePr(ch)].fMana)
	    gain /= 2;

	switch ( ch->position )
	{
	    default:            gain /= 4;                      break;
	    case POS_SLEEPING:                                  break;
	    case POS_RESTING:   gain /= 2;                      break;
	    case POS_FIGHTING:  gain /= 6;                      break;
	}

	if ( !IS_NPC(ch) )
	{
		if ( ch->pcdata->condition[COND_HUNGER] == 0 )
			gain /= 2;
		if ( ch->pcdata->condition[COND_THIRST] == 0 )
			gain /= 2;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO))
		gain = gain * ch->in_room->mana_rate / 100;

	if (ch->on != NULL
	&& (ch->on->item_type == ITEM_FURNITURE)
	&& !IS_SET(ch->on->extra_flags, ITEM_PROTOTIPO) )
		gain = gain * ch->on->value[4] / 100;

	if ( IS_AFFECTED( ch, AFF_POISON ) )
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
		gain /= 2;

	if ( ch->position != POS_FIGHTING )
		gain *= 3;

	return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = getNivelPr(ch) + 2;
    }
    else
    {
	gain = UMAX( 15, getNivelPr(ch) );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);          break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;      break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if (!IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO))
	gain = gain * ch->in_room->heal_rate/100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE
	&& !IS_SET(ch->on->extra_flags, ITEM_PROTOTIPO) )
		gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    if ( !ch->fighting )
	gain *= 1.5;

    return URANGE( 2, gain, ch->max_move - ch->move );

/*  return UMIN(gain, ch->max_move - ch->move); */
}



FRetVal gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;
    FRetVal rval = fOK;

    if ( value == 0
    ||	 IS_NPC(ch)
    ||	 getNivelPr(ch) >= LEVEL_IMMORTAL
    ||   EN_ARENA(ch)
    ||   (ch->desc && ch->desc->editor) )
	return fFAIL;

    condition                           = ch->pcdata->condition[iCond];

    if (condition == -1)
	return fOK;

    ch->pcdata->condition[iCond]        = URANGE( 0, condition + value, 48 );

    if ( ch->desc == NULL ) /* linkdead */
	return fOK;

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
		case COND_HUNGER:
		send_to_char( "Te estas muriendo de #BINANICION#b!!!\n\r", ch );
		act( "El estomago de $n cruje como puerta vieja.", ch, NULL, NULL, TO_ROOM );
		rval = damage(ch, ch, dice(getNivelPr(ch) / 2, 4), TYPE_HIT, DAM_OTHER, FALSE );
		break;

		case COND_THIRST:
		if (CHANCE(50))
			send_to_char( "Te estas muriendo de #BDESHIDRATACION#b!!!\n\r", ch );
		else
			send_to_char( "Te estas #BSECANDO#b como #B#FPASA#n!!!\n\r", ch );
		act( "$n se esta muriendo de deshidratacion.", ch, NULL, NULL, TO_ROOM );
		rval = damage(ch, ch, number_range(getNivelPr(ch) / 2, getNivelPr(ch)), TYPE_HIT, DAM_OTHER, FALSE );
		break;

		case COND_DRUNK:
		if ( condition != 0 )
			send_to_char( "Estas sobrio.\n\r", ch );
		break;
	}
    }
    else
    if ( ch->pcdata->condition[iCond] < 3 )
    {
    	switch( iCond )
    	{
    		case COND_HUNGER:
		act_new( "Estas hambrient$o.", ch, NULL, NULL, TO_CHAR, POS_DEAD );
    		break;

		case COND_THIRST:
		act_new( "Estas sedient$o.", ch, NULL, NULL, TO_CHAR, POS_DEAD );
		break;
	}
    }

    return rval;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) )
		continue;

	if ( ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
	    continue;

	if (ch->in_room->area->empty
	&& !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	if ( IS_SWITCHED(ch) && IS_IMMORTAL(ch->desc->original) )
		send_to_char( "MTICK!\n\r", ch );

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	if ( ch->pIndexData->script )
	{
		SCRIPT_DATA *script = ch->pIndexData->script;

		if ( script->timer == 0 )
		{
			interpret( ch, script->comandos[script->posicion++] );
			if ( script->comandos[script->posicion] == NULL )
				script->posicion = 0;
		}
		else if ( script->timer > 0 )
			script->timer--;
	}

	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	{
	    if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
	    {
		if(mp_percent_trigger( chToEnt(ch), NULL, NULL, NULL, TRIG_RANDOM ) > 0)
			continue;
	    }
	}

	if ( !IS_SET(ch->act, ACT_PET)
	&&   !is_mob_safe(ch) )
	{
		if ( IS_MOB_CASTER(ch)
		&&   ch->mana > ch->max_mana / 4
		&&   ch->position == POS_STANDING
		&&  !event_pending(ch->events, mob_cast) )
			char_event_add( ch, UMAX(ch->wait,2*PULSE_PER_SECOND), 0, mob_cast );

		if ( IS_SET(ch->form, FORM_SENTIENT)
		&&  !is_hunting(ch)
		&&  !event_pending(ch->events, smart_event) )
			char_event_add( ch, 2 * PULSE_PER_SECOND, 0, smart_event );
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 4 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE)
		 &&  can_loot(ch, obj)
		 && !IS_TRAP(obj)
		 && obj->cost > max
		 && can_see_obj(ch, obj) )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n toma $p.", ch, objToEnt(obj_best), NULL, TO_ROOM );
	    }
	}

	if (char_died(ch))
		continue;

	if (ch->in_room == NULL)
	{
		bugf( "Char %s, vnum %d, id %d en cuarto NULL",
			ch->name, CHARVNUM(ch), ch->id );
		continue;
	}

	if (is_hunting(ch)
	&& !event_pending(ch->events, hunt_event))
	{
		if ( ch->hunt_data->id > 0 )
		{
			CHAR_DATA * victim = get_char_from_id( ch->hunt_data->id );

			if ( victim
			&&   can_hunt(ch, victim, ch->hunt_data->especial) )
			{
				set_hunt(ch, victim, ch->hunt_data->especial);
				continue;
			}
		}
		else
		if (huntroom(ch) != NULL) /* buscando cuartos */
		{
			ROOM_INDEX_DATA *room = huntroom(ch);

			if (!room)
			{
				bugf( "mobile_update : roomhunt vnum %d, id %ld",
					ch->pIndexData->vnum, ch->id );
				stop_hunting( ch, FALSE, FALSE, TRUE );
				continue;
			}

			if (can_hunt_room(ch, room))
				set_hunt_room( ch, room );

			continue;
		}
		else
		{
			bugf( "mobile_update : mob %d no caza, id %ld",
				ch->pIndexData->vnum,
				ch->id );
			stop_hunting( ch, TRUE, TRUE, TRUE );
			continue;
		}
	}

	if ( IS_SET(ch->act, ACT_GROUP)
	&&   ch->master == NULL
	&&   ch->leader == NULL )
	{
		CHAR_DATA *gch;
		CHAR_DATA *best = NULL;

		for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
			if ( gch != ch
			&&   IS_NPC(gch)
			&&  !IS_AFFECTED(gch, AFF_CHARM)
			&&   gch->master == NULL
			&&   gch->leader == NULL
			&&   is_friend(ch, gch) )
			{
				if ( ch->pIndexData->group > 0 )
				{
					if ( ch->pIndexData->group == gch->pIndexData->group
					&&  (best == NULL || getNivelPr(best) < getNivelPr(gch)) )
						best = gch;
				}
				else
				if ( best == NULL || getNivelPr(best) < getNivelPr(gch) )
					best = gch;
			}

		if (best != NULL)
		{
			add_follower(ch, best);
			ch->leader = best;
			act( "$n se une al grupo de $N.", ch, NULL, chToEnt(best), TO_ROOM );
		}
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL)
	&& ch->master == NULL
	&& ch->leader == NULL
	&& number_bits(2) == 0
	&& ( door = number_bits( 5 ) ) <= (MAX_DIR - 1)
	&& ( pexit = exit_lookup(ch->in_room, door) ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    move_char( ch, door, FALSE );
	}

	if (CHANCE(1))
	{
		extern int maxSocial;
		bool check_social(CHAR_DATA *, char *, char *);

		int blah = number_range(0,maxSocial-1);

		check_social(ch,social_table[blah].name,"");
	}
    }

    return;
}

/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MIL];
    char buf2[MIL];
    int diff;

    buf[0] = buf2[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	sprintf( buf, "El dia ha comenzado." );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	sprintf( buf, "El sol sale por el este." );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	sprintf( buf, "El sol lentamente desaparece por el oeste." );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	sprintf( buf, "La noche ha empezado." );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    sprintf( buf2, "El cielo se esta nublando." );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    sprintf( buf2, "Ha empezado a llover." );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    sprintf( buf2, "Las nubes desaparecen." );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    sprintf( buf2, "El Trueno relampaguea en el cielo." );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    sprintf( buf2, "La lluvia ha cesado." );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    sprintf( buf2, "El Trueno se ha desvanecido." );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( !IS_NULLSTR(buf) || !IS_NULLSTR(buf2) )
    {
	CHAR_DATA *ch, *ch_next;

	for ( ch = char_list; ch; ch = ch_next )
	{
	    ch_next = ch->next;

	    if ( ch->in_room == NULL )
	    	continue;

	    if ( IS_OUTSIDE(ch)
	    &&   IS_AWAKE(ch) )
	    {
	    	if ( buf[0] != '\0' )
			act( buf, ch, NULL, NULL, TO_CHAR );
		if ( buf2[0] != '\0' )
			act( buf2, ch, NULL, NULL, TO_CHAR );
	    }

	    if ( IS_NPC(ch) )
	    {
	    	if (HAS_TRIGGER(ch, TRIG_TIME))
			mp_time_trigger( ch );
	    }
	    else
	    if (weather_info.sky >= SKY_RAINING)
		water_effect(ch);
	}
    }

    return;
}

/*
 * Update the bank system
 * (C) 1996 The Maniac from Mythran Mud
 *
 * This updates the shares value (I hope)
 */
void bank_update(void)
{
	int     value = 0;
	FILE    *fp;

	if ( time_info.hour < 9
	||   time_info.hour > 17 )
		return; /* el banco esta cerrado */

	value = number_range ( 0, 200);
	value -= 100;
	value /= 10;

	share_value += value;

	if ( share_value > 150 && CHANCE(80) )
		share_value *= 0.8;

	if ( share_value < 50 && CHANCE(80) )
		share_value *= 1.2;

	share_value = URANGE( 25, share_value, 200 );

	if ( !( fp = fopen ( BANK_FILE, "w" ) ) )
	{
		bug( "bank_update:  fopen of BANK_FILE failed", 0 );
		return;
	}
	fprintf (fp, "SHARE_VALUE %d\n\r", share_value);
	fclose(fp);
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static	int	pulse_area;
    static	int	pulse_mobile;
    static	int	pulse_violence;
    static	int	pulse_point;
    static	int	pulse_music;
    static	int	pulse_tele;
    static	int	pulse_auction;

    if ( --pulse_area     <= 0 )
    {
	pulse_area      = PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	bank_update     ( );
	area_update     ( );
    }

    if ( --pulse_music    <= 0 )
    {
	pulse_music     = PULSE_MUSIC;
	song_update();
    }
    
    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile    = PULSE_MOBILE;
	mobile_update   ( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence  = PULSE_VIOLENCE;
	violence_update ( );
	limpiar_ent_temp( );
    }

    if ( --pulse_point    <= 0 )
    {
	wiznet("TICK!",NULL,NULL,WIZ_TICKS,0,0);
	pulse_point	= number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 );
	weather_update  ( );
/*	obj_update      ( ); */
    }

    if ( --pulse_tele <= 0 )
    {
	pulse_tele      = PULSE_TELEPORT;
	tele_update     ( );
    }

    if ( --pulse_auction <= 0 )
    {
    	pulse_auction	= PULSE_AUCTION;
    	auction_update	( );
    }

    dead_update();
    event_update    ( );
    mf_update( );
    tail_chain( );

    return;
}

void tele_update ( void ) 
{
	CHAR_DATA       *ch;
	ROOM_INDEX_DATA *pRoomIndex;
	DESCRIPTOR_DATA	*d, *d_next;

	for ( d = descriptor_list; d; d = d_next )
	{
		d_next = d->next;

		if ( d->connected != CON_PLAYING )
			continue;

		ch = CH(d);

		if ( ch->in_room
		&&   IS_SET(ch->in_room->room_flags, ROOM_TELEPORT )
		&&  !IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO)
		&&  !IS_IMMORTAL(ch))
		{
			do_look ( ch, "tele" );

			if ( ch->in_room->tele_dest == 0 )
				pRoomIndex = get_random_room (chToEnt(ch));
			else
				pRoomIndex = get_room_index(ch->in_room->tele_dest);

			if ( pRoomIndex == NULL )
			{
				char buf[MSL];
				
				sprintf(buf, "tele_update : room %d tele_dest %d NULL",
					ch->in_room->vnum, ch->in_room->tele_dest );
				bug( buf, 0 );
				continue;
			}

			act("$n se desvanece!!!", ch, NULL, NULL, TO_ROOM);
			char_from_room(ch);
			char_to_room(ch, pRoomIndex);
			act("$n lentamente aparece en esta realidad.", ch, NULL, NULL, TO_ROOM);
			do_look(ch, "auto");
		}
	}
}

void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
    {
	    switch (++auction->going) /* increase the going state */
	    {
	    case 1 : /* going once */
	    case 2 : /* going twice */
	    if (auction->bet > 0)
		sprintf (buf, "%s: se va a las %s en %d MP.", auction->item->short_descr,
		     ((auction->going == 1) ? "una" : "dos"), auction->bet);
	    else
		sprintf (buf, "%s: se va a las %s (sin apuestas).", auction->item->short_descr,
		     ((auction->going == 1) ? "una" : "dos"));

	    talk_auction (buf);
	    break;

	    case 3 : /* SOLD! */

	    if (auction->bet > 0)
	    {
		sprintf (buf, "%s vendido a %s por %d MP.",
		    auction->item->short_descr,
		    IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
		    auction->bet);
		talk_auction(buf);
		obj_to_char (auction->item,auction->buyer);
		act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
		     auction->buyer,objToEnt(auction->item),NULL,TO_CHAR);
		act ("The auctioneer appears before $n, and hands $m $p",
		     auction->buyer,objToEnt(auction->item),NULL,TO_ROOM);

		auction->seller->silver += auction->bet; /* give him the money */

		auction->item = NULL; /* reset item */

	    }
	    else /* not sold */
	    {
		sprintf (buf, "No se recibieron apuestas por %s - objeto ha sido removido.",auction->item->short_descr);
		talk_auction(buf);
		act ("El encargado aparece frente a ti y te devuelve $p.",
		      auction->seller,objToEnt(auction->item),NULL,TO_CHAR);
		act ("El encargado aparece frente a $n y le devuelve $p.",
		      auction->seller,objToEnt(auction->item),NULL,TO_ROOM);
		obj_to_char (auction->item,auction->seller);
		auction->item = NULL; /* clear auction */

	    } /* else */

	    } /* switch */
    } /* if */
} /* func */

void reboot_event( EVENT *ev )
{
	int r_timer = (int) ev->param;
	extern int shutdown_type;
	extern void hacer_copyover( CHAR_DATA * );
	char * stype = NULL;
	char buf[MIL];

	switch(shutdown_type)
	{
		case SHUTDOWN_REBOOT:
		stype = "Reboot";
		break;

		case SHUTDOWN_COPYOVER:
		stype = "Copyover";
		break;

		case SHUTDOWN_NORMAL:
		stype = "Shutdown";
		break;

		case SHUTDOWN_NONE:
		return;

		default:
		bugf( "reboot_event : shutdown_type = %d", shutdown_type );
		shutdown_type = SHUTDOWN_NONE;
		return;
	}

	if ( r_timer )
		if ( r_timer % 60 == 0
		|| ( r_timer < 60 && r_timer % 15 == 0 )
		|| ( r_timer < 15 && r_timer % 5  == 0 ) )
		{
			sprintf( buf, "%s automatico en #B%d#b %s%s.\n\r",
				stype,
				r_timer >= 60 ? r_timer / 60 : r_timer,
				r_timer >= 60 ? "minuto" : "segundo",
				r_timer == 60 ? "" : "s" );
			do_info( NULL, buf );
		}

	if ( r_timer < 1 )
	{
		if ( shutdown_type != SHUTDOWN_COPYOVER )
			shutdown_mud_now(NULL);
		else
			hacer_copyover(NULL);
	}
	else
		generic_event_add(PULSE_PER_SECOND, (void *) (r_timer - 1), reboot_event );
}

bool update_char( CHAR_DATA * );

void char_update_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	bool temp = FALSE;

	if ( ch->in_room == NULL )
		temp = TRUE;
	else
		temp = update_char( ch );

	if ( temp == TRUE
	&&  !char_died(ch) )
		char_event_add( ch,
			number_range( PULSE_TICK * 3 / 4, PULSE_TICK * 5 / 4 ),
			(void *) ch->id,
			char_update_event );
}

/*
 * Update all chars, including mobs.
*/
bool update_char( CHAR_DATA *ch )
{   
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	FRetVal rval;
	char buf[MIL];
	bool es_npc = IS_NPC(ch);

	if ( EDITANDO(ch) )
		return TRUE;

	if ( ch->max_hit < 0 )
	{
		sprintf( buf, "Char '%s' con max_hit '%d', in_room '%d'",
			ch->name,
			ch->max_hit,
			ch->in_room->vnum );
		bug( buf, 0 );
		return TRUE;
	}

	if ( IS_NPC(ch) && ch->timer > 0 && --ch->timer == 0 )
	{
		if ( ch->race == RACE_ZOMBIE && ch->in_room )
		{
			OBJ_DATA *obj;
			obj = create_object( get_obj_index(OBJ_VNUM_PLASTA), 0 );
			obj->timer = number_fuzzy(5);
			obj_to_room( obj, ch->in_room );
			act( "$n se pudre.", ch, NULL, NULL, TO_ROOM );
		}

		extract_char(ch, TRUE);
		return FALSE;
	}

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit < ch->max_hit )
		change_health(chToEnt(ch), chToEnt(ch), hit_gain(ch));
	    else
		ch->hit = ch->max_hit;

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( IS_NPC(ch) && ch->position >= POS_STUNNED )
	{
		if ( ch->ent == NULL
		&&   difftime(current_time, ch->logon) > MINUTOS(10) )
			ch->ent = chToEntidad(ch, TRUE);

		/* check to see if we need to go home */
		if ( ch->zone
		&&   (ch->zone != ch->in_room->area)
		&&   IS_SET(ch->act, ACT_STAY_AREA)
		&&   ch->desc == NULL
		&&   ch->fighting == NULL
		&&  !IS_AFFECTED(ch,AFF_CHARM)
		&&   SPEC_FUN(ch) != spec_taxi
		&&   number_percent() < 5)
		{
			act("$n vuelve a su hogar.",ch,NULL,NULL,TO_ROOM);
			extract_char(ch,TRUE);
			return FALSE;
		}

		if ( IS_SET(ch->act, ACT_PROTOTIPO)
		&&  !IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO) )
		{
			act( "$n se desvanece lentamente.", ch, NULL, NULL, TO_ROOM );
			extract_char( ch, TRUE );
			return FALSE;
		}

		if ( ch->pIndexData->pShop && (ch->gold * 100 + ch->silver) < ch->pIndexData->wealth )
		{
			ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
			ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
		}

		if ( ch->fighting == NULL )
			ch->wait = ch->daze = 0;

		if ( !IS_PET(ch)
		&&    ch->random < getNivelPr(ch) / 10 + 1 )
		{
			ch->random++;
			if (CHANCE(getNivelPr(ch)))
				crear_armadura_random(ch);
		}

		if ( ch->memory ) /* codigo para olvidar */
		{
			MEM_DATA *mem, *mem_next;
			double difer;
			double maxdifer;

			if (IS_SET(ch->form, FORM_SENTIENT))
				maxdifer = MINUTOS(360);
			else
				maxdifer = MINUTOS(60);

			for ( mem = ch->memory; mem; mem = mem_next )
			{
				mem_next = mem->next;

				difer = difftime(current_time, mem->when);

				if ( difer > maxdifer )
					extract_mem( ch, mem );
			}
		} /* fin codigo olvidar */
	} /* fin seccion NPC */

	if ( !IS_NPC(ch) && !event_pending(ch->events, autosave_event) )
	    char_event_add( ch,
		MINUTOS(5) * PULSE_PER_SECOND,
		(void *) ch->id,
		autosave_event );

	if ( IS_IMMORTAL(ch) && IS_SET(ch->wiznet, WIZ_TICKS) )
		send_to_char( "TAC!\n\r", ch );

	ch->luck = number_range( -100, 100 );

	if ( !IS_NPC(ch) && getNivelPr(ch) < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ch->timer > 20 )
	    {
	    	do_quit( ch, "" );
	    	return FALSE;
	    }

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p se apaga.", ch, objToEnt(obj), NULL, TO_ROOM );
		    act( "$p parpadea y se apaga.", ch, objToEnt(obj), NULL, TO_CHAR );
		    extract_obj( obj, TRUE );
		}
		else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p parpadea.",ch,objToEnt(obj),NULL,TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if ( ch->pcdata->bank )
	    {
	    	BANK_DATA *bank;

		for ( bank = ch->pcdata->bank; bank; bank = bank->next )
			if ( difftime(current_time, bank->when) > HORAS(24)
			&&  (bank->valor * bank->interes) < LONG_MAX )
			{
//				bank->valor	*= bank->interes;
				bank->when 	+= HORAS(24);
			}
	    }

	    if (es_clase(ch, CLASS_RANGER) && !IS_SET(ch->act, PLR_MAL_ALIGN))
	    {
	    	if (IS_EVIL(ch))
		{
		    	act( "La ira de $T cae sobre ti!", ch, NULL, strToEnt(CLAN_GOD(ch),ch->in_room), TO_CHAR );
		    	act( "Sientes que tu poder magico se desvanece.", ch, NULL, NULL, TO_CHAR );
			change_health( chToEnt(ch), chToEnt(ch), - (ch->hit/2) );
		    	ch->mana = 0;
			SET_BIT(ch->act, PLR_MAL_ALIGN);
		}
		else if ( (ch->alignment < 0) && !IS_EVIL(ch) )
			act( "Cuidado! Tu dios $T se va a enojar por tu comportamiento.", ch, NULL, strToEnt(CLAN_GOD(ch),ch->in_room), TO_CHAR );
	    }

	    if ( ch->pcdata->quaff > 0 )
		ch->pcdata->quaff--;

#if defined(POCIONES_LIMITADAS)
	    if ( !EN_ARENA(ch)
	    &&    getNivelPr(ch) > 15
	    &&    ch->pcdata->quaff > number_range(2,3) )
	    {
		send_to_char("Sientes un extrano dolor de estomago!\n\r",ch);
		send_to_char("La combinacion de pociones en tu estomago #BEXPLOTARON#b!\n\r",ch);
		change_health( chToEnt(ch), chToEnt(ch), - (ch->hit/2) );
		affect_strip(ch, gsn_fireproof);
		if (CHANCE(30))
			spell_fireball(skill_lookup("fireball"),LEVEL_HERO + (ch->pcdata->quaff), ch, ch, TARGET_CHAR);
		else
			spell_acid_blast(skill_lookup("acid blast"),LEVEL_HERO + (ch->pcdata->quaff), ch, ch, TARGET_CHAR);
		if (char_died(ch))
			return TRUE;
	    }
#endif

	    if ( ++ch->timer >= 12 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    act( "$n desaparece en el Vacio.",
			ch, NULL, NULL, TO_ROOM );
		    send_to_char( "Apareces en el Vacio.\n\r", ch );
		    if (getNivelPr(ch) > 1)
			save_char_obj( ch );
		    if ( ch->in_room )
		    {
			char_from_room( ch );
			char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		    }
		    else
		    	bugf( "update_char : NULL ch->in_room, char %s", ch->name );
		}
	    }

	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	    if ( gain_condition( ch, COND_THIRST, -1 ) == victdead )
		return TRUE;
	    if ( gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1) == victdead )
		return TRUE;
	}

	/* Update de los affects */
	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    if ( paf->type >= MAX_SKILL )
		bugf( "Char_update_event : val %d,where %d,type %d,lev %d,dur %d,loc %d,mod %d,player %s",
			paf->valid, paf->where, paf->type,
			paf->level, paf->duration, paf->location,
			paf->modifier, ch->name );
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && paf->type < MAX_SKILL )
		    {
		    	if (!IS_NULLSTR(skill_table[paf->type].msg_off) )
			{
				send_to_char( skill_table[paf->type].msg_off, ch );
				send_to_char( "\n\r", ch );
			}
			if (!IS_NULLSTR(skill_table[paf->type].msg_room) )
				act( skill_table[paf->type].msg_room, ch, NULL, NULL, TO_ROOM );
		    }
		}

		if ( skill_table[paf->type].spell_callback != NULL )
			(*skill_table[paf->type].spell_callback) ( chToEnt(ch) );

		affect_remove( ch, paf );
	    }
	}

	if ( !EN_ARENA(ch) )
	{
		/* Enfermedades */
		if (is_affected(ch, gsn_plague) || IS_AFFECTED(ch, AFF_PLAGUE))
		{
			AFFECT_DATA *af, plague;
			CHAR_DATA *vch;
			int dam;

			act("$n se retuerce en agonia mientras la plaga hace estragos en su piel.",
				ch,NULL,NULL,TO_ROOM);
			send_to_char("Te retuerces en agonia por culpa de la plaga.\n\r",ch);

			if ((af = affect_find(ch->affected, gsn_plague)) == NULL)
			{
				REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
				return TRUE;
			}

			if (af->level == 1)
				return TRUE;

			plague.where		= TO_AFFECTS;
			plague.type		= gsn_plague;
			plague.level		= af->level - 1; 
			plague.duration		= number_range(1,2 * plague.level);
			plague.location		= APPLY_STR;
			plague.modifier		= -5;
			plague.bitvector	= AFF_PLAGUE;

			for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
			{
				if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
				&&  !IS_IMMORTAL(vch)
				&&  !IS_AFFECTED(vch,AFF_PLAGUE)
				&& (!IS_NPC(ch) || !IS_SET(ch->act, ACT_IS_HEALER))
				&&   number_bits(2) == 0)
				{
					send_to_char("Te sientes ahogado y febril.\n\r",vch);
					act("$n tirita y se ve muy enfermo.",vch,NULL,NULL,TO_ROOM);
					affect_join(vch,&plague);
				}
			}

			dam = UMIN(getNivelPr(ch), af->level / 4 + 1);

			ch->mana = UMAX(0, ch->mana - dam);
			ch->move = UMAX(0, ch->move - dam);

			rval = damage_old( ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE);

			if ( rval == victdead )
				return ( es_npc ? FALSE : TRUE );
		}

		if (IS_AFFECTED(ch, AFF_POISON))
		{
		    AFFECT_DATA *poison;

		    poison = affect_find(ch->affected,gsn_poison);

		    if (poison != NULL)
		    {
			act( "$n tirita y sufre.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "Tiritas y sufres.\n\r", ch );

			rval = damage_old(ch,ch,poison->level/10 + 1,gsn_poison,
			    DAM_POISON,FALSE);

			if ( rval == victdead )
				return ( es_npc ? FALSE : TRUE );
		    }
		}

		if ( weather_info.sunlight != SUN_DARK
		&&   ch->race == RACE_VAMPIRE
		&&   get_eq_char( ch, WEAR_LENTES ) == NULL
		&&  !room_is_dark(ch->in_room) )
		{
		    int dmg = 0;

		    if ( ch->in_room->sector_type == SECT_INSIDE )
			dmg = 10;
		    else
		    if ( ch->in_room->sector_type == SECT_FOREST )
			dmg = 25;
		    else
			dmg = 50;

		    if ( weather_info.sky == SKY_CLOUDY )
			dmg /= 2;

		    if ( weather_info.sky == SKY_RAINING )
			dmg *= 0.75;

		    act( "#BAGGH#b! La #BLUZ#b!!!", ch, NULL, NULL, TO_CHAR );

		    rval = damage( ch, ch, dmg, gsn_poison, DAM_LIGHT, FALSE );

		    if ( rval == victdead )
			return ( es_npc ? FALSE : TRUE );
		}

		if ( !IS_IMMORTAL(ch)
		&&    ch->in_room->sector_type == SECT_UNDERWATER )
		{
			ch->aire--;
			if ( ch->aire <= 0 )
			{
				if ( ch->position == POS_STUNNED )
				{
					rval = newdamage( chToEnt(ch), ch, 1200, TYPE_HIT, DAM_DROWNING, TRUE );
					
					if ( rval == victdead )
						return ( es_npc ? FALSE : TRUE );
				}
				else
				{
					send_to_char( "Glub glub glub glub...\n\r", ch );
					update_pos(ch);
				}
			}
			else
			{
				switch(ch->aire)
				{
					case 1:
					send_to_char( "Necesitas AIRE! RAPIDO!\n\r", ch );
					break;

					case 2:
					send_to_char( "Te pones morado.\n\r", ch );
					break;
				}
			} // aire <= 0
		} // underwater

		if ( ch->position == POS_INCAP && number_range(0,1) == 0)
		{
		    rval = damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);

		    if ( rval == victdead )
		    	return ( es_npc ? FALSE : TRUE );
		}

		if ( ch->position == POS_MORTAL )
		{
		    rval = damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);

		    if ( rval == victdead )
		    	return ( es_npc ? FALSE : TRUE );
		}
	}

	return TRUE;
}

bool update_obj( OBJ_DATA * );

void obj_update_event( EVENT *ev )
{
	OBJ_DATA *obj = ev->item.obj;
	bool temp = FALSE;

	temp = update_obj( obj );

	if ( temp == TRUE
	&&  !event_pending(obj->events, obj_update_event) )
		obj_event_add( obj,
			number_range( PULSE_TICK * 3 / 4, PULSE_TICK * 5 / 4 ),
			NULL,
			obj_update_event );
}

bool update_obj( OBJ_DATA * obj )
{
	CHAR_DATA *rch;
	AFFECT_DATA *paf, *paf_next;
	char *message;
	char *mensaje = NULL;

	/* go through affects and decrement */
	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && !IS_NULLSTR(skill_table[paf->type].msg_obj) )
		    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,objToEnt(obj),NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,objToEnt(obj),NULL,TO_ALL);
			}
		    }
		}

		affect_remove_obj( obj, paf );
	    }
	}

	if ( obj->carried_by )
	{
		obj->carry_timer++;

		if ( (get_skill(obj->carried_by, gsn_lore) + obj->carry_timer * 2) > (100 + number_percent()*2) )
		{
			char * msg = NULL;
			long bit = 0;

			if ( !DETECTED(obj, DETECTED_CURSE)
			&& ( IS_OBJ_STAT(obj, ITEM_NODROP)
			  || IS_OBJ_STAT(obj, ITEM_NOREMOVE) ) )
			{
				msg = "Hmmm...sientes que $p esta maldito.";
				bit = DETECTED_CURSE;
			}
			else
			if ( !DETECTED(obj, DETECTED_EVIL)
			&&   IS_OBJ_STAT(obj, ITEM_EVIL) )
			{
				msg = "Hmmm...sientes que $p es maligno.";
				bit = DETECTED_EVIL;
			}
			else
			if ( obj->item_type == ITEM_WEAPON
			&&   IS_WEAPON_STAT(obj, WEAPON_SHARP)
			&&  !DETECTED(obj, DETECTED_SHARP) )
			{
				msg = "Hmmm...sientes que $p esta afilada.";
				bit = DETECTED_SHARP;
			}

			if ( msg != NULL && bit > 0 )
			{
				act( msg, obj->carried_by, objToEnt(obj), NULL, TO_CHAR );
				SET_BIT(obj->detected, bit);
				check_improve(obj->carried_by,gsn_lore,TRUE,5);
			}
		}
	} // detect

	if ( puede_deteriorarse(obj)
	&&   number_range(0,15) == 0 )
	{
		--obj->condition;

		switch( obj->condition )
		{
			default: mensaje = NULL;				break;
			case 75: mensaje = "$p esta en buena condicion.";	break;
			case 50: mensaje = "$p esta en regular condicion.";	break;
			case 25: mensaje = "$p esta en mala condicion.";	break;
		}

		switch(obj->item_type)
		{
			case ITEM_FOOD:
			if ( obj->condition < 50 )
			{
				if ( CHANCE(20)
				&&  !IS_SET(obj->value[3], FOOD_POISON) )
				{
					if (obj->carried_by)
						act( "$p empieza a descomponerse.",
							obj->carried_by, objToEnt(obj), NULL, TO_CHAR );
					SET_BIT(obj->value[3], FOOD_POISON);
				}
				else
				if ( CHANCE(20)
				&&  !IS_SET(obj->value[3], FOOD_PLAGUE) )
				{
					if (obj->carried_by)
						act( "$p toma un color oscuro.",
							obj->carried_by, objToEnt(obj), NULL, TO_CHAR );
					SET_BIT(obj->value[3], FOOD_PLAGUE);
				}
			}
			break;
		}

		if ( mensaje != NULL
		&&   obj->carried_by != NULL
		&&  !IS_SET(obj->carried_by->comm, COMM_CONDICION ) )
			act( mensaje, obj->carried_by, objToEnt(obj), NULL, TO_CHAR );
	}

	if ( (obj->timer <= 0 || --obj->timer > 0) && obj->condition > 0 )
	{
		EXIT_DATA *pExit;

		if (obj->in_room
		 && obj->in_room->sector_type == SECT_AIR
		 && IS_SET(obj->wear_flags, ITEM_TAKE)
		 && (pExit = exit_lookup(obj->in_room, DIR_DOWN))
		 && pExit->u1.to_room)
		{
			ROOM_INDEX_DATA *new_room = pExit->u1.to_room;

			if (( rch = obj->in_room->people ) != NULL )
			{
				act( "$p cae hacia abajo.", rch, objToEnt(obj), NULL, TO_ROOM );
				act( "$p cae hacia abajo.", rch, objToEnt(obj), NULL, TO_CHAR );
			}

			obj_from_room(obj);
			obj_to_room(obj, new_room);

			if (( rch = obj->in_room->people ) != NULL )
			{
				act( "$p cae desde el cielo.", rch, objToEnt(obj), NULL, TO_ROOM );
				act( "$p cae desde el cielo.", rch, objToEnt(obj), NULL, TO_CHAR );
			}
		}

		return TRUE;
	}

	switch ( obj->item_type )
	{
	default:              message = "$p se deshace en polvo.";  break;
	case ITEM_FOUNTAIN:   message = "$p se seca.";         break;
	case ITEM_CORPSE_NPC: message = "$p se deshace en polvo."; break;
	case ITEM_CORPSE_PC:  message = "$p se deshace en polvo."; break;
	case ITEM_FOOD:       message = "$p se pudre.";        break;
	case ITEM_POTION:     message = "$p se ha evaporado.";  
								break;
	case ITEM_PORTAL:     message = "$p se desvanece."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
	    {
		if (obj->contains)
		    message = "$p parpadea y desaparece, derramando sus contenidos por el suelo.";
		else
		    message = "$p parpadea y desaparece.";
	    }
	    else
		message = "$p se deshace en polvo.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
		act( message, obj->carried_by, objToEnt(obj), NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,objToEnt(obj),NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
		   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
		act( message, rch, objToEnt(obj), NULL, TO_ROOM );
		act( message, rch, objToEnt(obj), NULL, TO_CHAR );
	    }
	}

	if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		{
		    if (obj->wear_loc == WEAR_FLOAT)
		    {
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj, TRUE);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room);
		    }
		    else
			obj_to_char(t_obj,obj->carried_by);
		}

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj, TRUE);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj, TRUE );

	return FALSE;
}

bool puede_deteriorarse( OBJ_DATA * obj )
{
	ROOM_INDEX_DATA *room = obj_whereis(obj);

	if ( room == NULL
	||  !CAN_WEAR(obj, ITEM_TAKE)
	||   obj->condition <= 0
	||   obj->carried_by == NULL
	||   IS_NPC(obj->carried_by)
	||   IS_IMMORTAL(obj->carried_by)
	||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
	||   IS_SET(room->room_flags, ROOM_PROTOTIPO) )
		return FALSE;

	return TRUE;
}

void mem_check_event( EVENT * ev )
{
	CHAR_DATA * ch = ev->item.ch;
	MEM_DATA *mem;
	bool hecho = FALSE;

	if ( ch->position != POS_STANDING
	||   IS_AFFECTED(ch, AFF_CHARM) )
	{
		char_event_add( ch, PULSE_PER_SECOND, 0, mem_check_event );
		return;
	}

	for ( mem = ch->memory; mem; mem = mem->next )
	{
		switch(mem->reaction)
		{
			case MEM_AFRAID:
			{
				int door = mob_best_door(ch);

				if ( is_mob_safe(ch) )
					continue;

				if ( door > -1 )
				{
					act( "$n huye aterrorizad$o!", ch, NULL, NULL, TO_ROOM );
					move_char( ch, door, FALSE );
				}
				else
				{
					CHAR_DATA *victim = get_char_id_room(ch, mem->id);
	
					if (victim && can_see(ch, victim))
						multi_hit( ch, victim, TYPE_UNDEFINED );
				}

				hecho = TRUE;
			} /* afraid */
			break;

			case MEM_HOSTILE:
			{
				CHAR_DATA *victim = get_char_id_room(ch, mem->id);
				double difer;
				char *msg;

				if (!victim
				||  !can_see(ch, victim)
				||   is_mob_safe(ch))
					continue;

				difer = difftime(current_time, mem->when);

				if ( difer < MINUTOS(3) )
					msg = "Tan pronto?";
				else
				if ( difer < MINUTOS(10) )
					msg = "Volviste por mas, eh?";
				else
				if ( difer < MINUTOS(30) )
					msg = "Parece que no tuviste suficiente la ultima vez!";
				else
					msg = "Tanto tiempo sin verte!";

				act( "$n mira a $N y dice, '$t'",
					ch, strToEnt(msg,ch->in_room), chToEnt(victim), TO_NOTVICT );
				act( "$n te mira y dice, '$t'",
					ch, strToEnt(msg,ch->in_room), chToEnt(victim), TO_VICT );
				act( "Miras a $N y dices, '$t",
					ch, strToEnt(msg,ch->in_room), chToEnt(victim), TO_CHAR);
				multi_hit( ch, victim, TYPE_UNDEFINED );

				hecho = TRUE;
			} /* hostile */
			break;
		} /* switch */

		if ( hecho == TRUE )
			break;
	} /* for */

	if ( !char_died(ch) )
		char_event_add( ch, PULSE_PER_SECOND, 0, mem_check_event );
}

void aggress( CHAR_DATA *mch, bool odio )
{
    int count = 0;
    CHAR_DATA *victim = NULL, *vch = NULL;

    if (mch->position == POS_FIGHTING)
    	return;

    for ( vch = mch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ((!IS_NPC(vch) || IS_PET(vch))
	&&   getNivelPr(vch) < LEVEL_IMMORTAL
	&&   getNivelPr(mch) >= getNivelPr(vch) - 5 
	&&  (odio == FALSE || (HATES(mch, vch) && ENTRE(getNivelPr(vch) - 2, getNivelPr(mch), getNivelPr(vch) + 2)))
	&&  (!is_clan(mch) || !is_clan(vch) || !is_same_clan(mch, vch))
	&&  (!IS_SET(mch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
	&&   vch->master != mch		/* mascotas no */
	&&   can_see( mch, vch ) )
	{
	    if ( number_range( 0, count ) == 0 )
		victim = vch;
	    count++;
	}
    }

    if ( !victim )
	return;

    if ( HATES(mch, victim) )
    {
	char *point;
	char mensaje[MIL];

	if ( IS_FORM(mch, FORM_ANIMAL) )
	{
		act( "$n grune y se lanza sobre $N!", mch, NULL, chToEnt(victim), TO_ROOM );
		act( "$n grune y se lanza sobre ti!", mch, NULL, chToEnt(victim), TO_VICT );
	}
	else
	{
		if ( victim->sex == SEX_FEMALE )
			point = race_table[victim->race].hembra;
		else
			point = race_table[victim->race].macho;
		if ( IS_NULLSTR(point) )
			point = race_table[victim->race].name;

 	   	sprintf( mensaje, "Muere, asqueros%c %s!!!",
    			(victim->sex == SEX_FEMALE) ? 'a' : 'o',
    			point );

		act( "$n te mira con odio y grita '$t'", mch, strToEnt(mensaje,mch->in_room), chToEnt(victim), TO_VICT );
		act( "$n mira con odio a $N y grita '$t'", mch, strToEnt(mensaje,mch->in_room), chToEnt(victim), TO_NOTVICT );
	}
    }

    multi_hit( mch, victim, TYPE_UNDEFINED );
}

void room_aggress_event( EVENT * ev )
{
    int cnt = 0;
    bool odio = FALSE;
    ROOM_INDEX_DATA *pRoom = ev->item.room;
    CHAR_DATA *ch;

    for ( ch = pRoom->people; ch; ch = ch->next_in_room )
    {
	if ( IS_NPC(ch)
	&&   IS_AWAKE(ch)
	&&   ch->fighting == NULL
	&&  !IS_AFFECTED(ch, AFF_CALM)
	&&  !IS_AFFECTED(ch, AFF_CHARM)
	&&   ch->pIndexData->pShop == NULL
	&&   ch->pIndexData->pRepair == NULL
	&&  !IS_SET(ch->act, ACT_TRAIN)
	&&  !IS_SET(ch->act, ACT_PRACTICE)
	&&  !IS_SET(ch->act, ACT_PROTOTIPO)
	&&  !IS_SET(ch->act, ACT_TEACHER)
	&&  !IS_SET(ch->act, ACT_IS_HEALER)
	&&  !IS_SET(ch->act, ACT_GAIN)
	&&  !IS_SET(ch->act, ACT_IS_CHANGER)
	&&  !IS_SET(ch->act, ACT_BANKER)
	&&  (IS_SET(ch->act, ACT_AGGRESSIVE) || (odio = odia_alguien_cuarto(ch))) )
	{
		cnt++;
		aggress(ch, odio);
	}
    }

    if (cnt > 0)
    	room_event_add(pRoom, 1, NULL, room_aggress_event);

    return;
}
