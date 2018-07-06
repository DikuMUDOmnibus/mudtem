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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "events.h"
#include "math.h"
#include "olc.h"
#include "smart.h"
#include "arena.h"
#include "vars.h"
#include "lookup.h"
#include "quest.h"

// #define RANGO_PK 7
#undef RANGO_PK

/* command procedures needed */
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_steal		);
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_echo		);
DECLARE_DO_FUN(do_info		);
DECLARE_DO_FUN(do_circle	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_tail		);
DECLARE_DO_FUN(do_cast		);
DECLARE_DO_FUN(do_snare		);
DECLARE_DO_FUN(do_recite	);
DECLARE_DO_FUN(do_brandish	);
DECLARE_DO_FUN(do_zap		);
DECLARE_DO_FUN(do_eat		);
DECLARE_DO_FUN(do_quaff		);
DECLARE_DO_FUN(do_visible	);
DECLARE_DO_FUN(do_rescue	);
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_fire_breath	);
void	set_puntaje	args( ( CHAR_DATA *ch ) );
void	revisa_muertes	args( ( CHAR_DATA *ch ) );
void	mensaje_muerte	args( ( Entity *, CHAR_DATA *victim, int dam_type ) );
void	mensaje		args( ( Entity *, CHAR_DATA *victim ) );
bool	use_magical_item	args( ( CHAR_DATA *ch ) );
bool	check_blind	args( ( CHAR_DATA *ch ) );

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	new_dam_message	args( ( Entity *, CHAR_DATA *, int, int, bool ) );

void	death_cry	args( ( CHAR_DATA *ch ) );
int	group_gain	args( ( Entity *, CHAR_DATA * ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch, int type ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt , bool secondary ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( Entity *, CHAR_DATA *victim , int type ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool second ) );
void	primer_ataque	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	destruccion;
bool    check_race_special   args( ( CHAR_DATA *ch ) );

CHAR_DATA * afighter[2] = { NULL, NULL };

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, ch->in_room == victim->in_room ? FALSE : TRUE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( chToEnt(ch), chToEnt(victim), NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{
	    if (IS_NPC(rch)
	    && (rch->pIndexData->pShop != NULL
	     || rch->pIndexData->pRepair != NULL
	     || IS_SET(rch->act, ACT_TRAIN)
	     || IS_SET(rch->act, ACT_PRACTICE)
	     || IS_SET(rch->act, ACT_TEACHER)
	     || IS_SET(rch->act, ACT_IS_HEALER)
	     || IS_SET(rch->act, ACT_IS_CHANGER)
	     || IS_SET(rch->act, ACT_BANKER)) )
	     	continue;
	     
	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  getNivelPr(rch) + 6 > getNivelPr(victim))
	    {
		do_emote(rch,"grita y ataca!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);

		continue;
	    }

	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||    is_same_group(ch, rch)

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			if ( target->fighting == ch
			&&   rch->hit > ch->hit
			&&   IS_SET(rch->off_flags, OFF_RESCUE) )
				do_rescue(rch, ch->name);
			else
			{
				do_emote(rch,"grita y ataca!");
				multi_hit(rch,target,TYPE_UNDEFINED);
			}
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;

    /* decrement the wait */
    if ( !IS_NPC(ch) && ch->desc == NULL)
    {
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);
	ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 
    }

    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	if ( (ch->fighting == NULL) && (dt == TYPE_UNDEFINED) )
	{
		primer_ataque(ch,victim);

		if ( char_died(victim) )
			return;
	}

	mob_hit(ch,victim,dt);
	return;
    }

    if ( ch != victim
    &&  !IS_NPC(victim)
    &&  !EN_ARENA(victim)
    &&   war == FALSE
    &&  !IS_SET(victim->in_room->room_flags, ROOM_PROTOTIPO)
    && (!IS_AFFECTED(ch, AFF_CURSE) || !IS_AFFECTED(victim, AFF_CURSE)))
    {
    	AFFECT_DATA af;

	af.type		= gsn_curse;
	af.where	= TO_AFFECTS;
	af.level	= MAX_LEVEL;
	af.duration	= 3;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_CURSE;
	af.caster_id	= ch->id;

	if ( !IS_AFFECTED(ch, AFF_CURSE) )
		affect_to_char( ch, &af );
	if ( !IS_AFFECTED(victim, AFF_CURSE) )
	{
		af.caster_id = victim->id;
		affect_to_char( victim, &af );
	}
    }

    if ( ch->fighting && check_race_special( ch ) )
	return;

    one_hit( ch, victim, dt , FALSE );

    if (ch->fighting != victim)
	return;

    if (get_eq_char (ch, WEAR_SECONDARY) && CHANCE(get_skill(ch, gsn_ambidiestro)) )
    {
	check_improve( ch, gsn_ambidiestro, TRUE, 5 );
	one_hit( ch, victim, dt, TRUE );
	if (ch->fighting != victim)
		return;
    }

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt,FALSE);

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW) || IS_AFFECTED2(ch,AFF_HOLD))
	chance /= 2;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt , FALSE );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
	return; /* antes se hacia chance = 0 */

    if (IS_AFFECTED2(ch,AFF_HOLD))
    	chance /= 2;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt , FALSE );
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch, gsn_fourth_attack)/5;

    if (IS_AFFECTED2(ch,AFF_HOLD))
    	return;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_fourth_attack,TRUE,6);
        if ( ch->fighting != victim )
            return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance;
    CHAR_DATA *vch, *vch_next;

    /* oh boy!  Fun stuff! */
    if ( ch->fighting == victim && !event_pending(ch->events, mob_fight) )
    	char_event_add( ch, ch->wait ? ch->wait : 1, (void *) victim->id, mob_fight );

    if ( getNivelPr(ch) > 22
    &&  !IS_AFFECTED(ch, AFF_CHARM)
    &&  !IS_SET(ch->act, ACT_PET)
    &&   IS_NPC(victim) 
    &&   ( IS_SET(victim->act, ACT_PET) || IS_AFFECTED(victim, AFF_CHARM) )
    &&   IS_SET(ch->form, FORM_SENTIENT)
    &&   victim->master
    &&  ( victim->in_room == victim->master->in_room )
    &&	 can_see(ch,victim->master)
    &&  (CHANCE(30) || ( (ch->hit < ch->max_hit / 2) && CHANCE(70) ) ) )
    {
	act( "$n mira con #BODIO#b a $N y dice '#BTU#b eres mi objetivo!'", ch, NULL, chToEnt(victim->master), TO_NOTVICT );
	act( "$n te mira con #BODIO#b y dice '#BTU#b eres mi objetivo!'", ch, NULL, chToEnt(victim->master), TO_VICT );
    	stop_fighting( ch, FALSE );
    	set_fighting( ch, victim->master );
	victim = victim->master;
    }

    one_hit(ch,victim,dt,FALSE); /* primer ataque */

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt,FALSE);
	}
    }

    if (get_eq_char (ch, WEAR_SECONDARY) && CHANCE(get_skill(ch, gsn_ambidiestro)) )
        one_hit( ch, victim, dt, TRUE );

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt,FALSE); /* ataque por haste */

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if ((IS_AFFECTED(ch,AFF_SLOW) || IS_AFFECTED2(ch,AFF_HOLD))
    &&  !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance = 0;

    if (IS_AFFECTED2(ch,AFF_HOLD))
    	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt, FALSE);
	if (ch->fighting != victim)
	    return;
    } 

    chance = get_skill(ch, gsn_fourth_attack)/5;

    if ((IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    ||   IS_AFFECTED2(ch,AFF_HOLD) )
	chance = 0;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        if ( ch->fighting != victim )
            return;
    }
}


/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt , bool secondary)
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    FRetVal result;

    sn = -1;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    {
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    }
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch,secondary);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_PSI))
	{
		thac0_00 = 18;
		thac0_32 = 5;
	}
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[getClasePr(ch)].thac0_00;
	thac0_32 = class_table[getClasePr(ch)].thac0_32;
    }
    thac0  = interpolate( getNivelPr(ch), thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab || dt == gsn_circle )
	thac0 -= 10 * (100 - get_skill(ch,dt));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 

    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;

    if ( !can_see( ch, victim ) )
    {
    	if ( CHANCE(get_skill(ch,gsn_blindfight)) )
    		check_improve(ch,gsn_blindfight,TRUE,6);
    	else
    	{
		check_improve(ch,gsn_blindfight,FALSE,6);
		victim_ac -= 4;
	}
    }

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	newdamage( chToEntidad(ch, FALSE), victim, 0, dt, dam_type, TRUE );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
    {
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( getNivelPr(ch) / 2, getNivelPr(ch) * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
    }
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * getNivelPr(ch)/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }
    }

    if (get_eq_char(ch,WEAR_SHIELD) == NULL
    &&  get_eq_char(ch,WEAR_SECONDARY) == NULL)  /* no shield = more */
	dam = dam * 11/10;

    /* sharpness! */
    if (wield && IS_WEAPON_STAT(wield,WEAPON_SHARP))
    {
	int percent;

	if ((percent = number_percent()) <= (skill / 8))
	    dam = 2 * dam + (dam * 2 * percent / 100);
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( (dt == gsn_backstab || dt == gsn_circle) && wield != NULL) 
    {
    	if ( wield->value[0] != 2 )
	    dam *= 1 + (getNivelPr(ch) / 10); 
	else 
	    dam *= 2 + (getNivelPr(ch) / 8);
    }

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( wield )
    {
	if ( es_vuln_mat(victim, wield->material) )
		dam *= 1.5;
	else
	if ( es_res_mat(victim, wield->material) )
		dam /= 2;
    }

    if ( dam <= 0 )
	dam = 1;

    result = newdamage( chToEntidad(ch, FALSE), victim, dam, dt, dam_type, TRUE );
    
    if (result == fOK && IS_AFFECTED2(victim, AFF_FLAMING_SHIELD) && CHANCE(getNivelPr(victim)))
	(*skill_table[gsn_flamestrike].spell_fun) ( gsn_flamestrike, getNivelPr(victim) / 4, chToEnt(victim), chToEnt(ch), TO_CHAR );

    /* Arma pudo haber sido extraida en la llamada a damage()! */
    if (!char_died(ch))
    {
    	if (!secondary)
		wield = get_eq_char( ch, WEAR_WIELD );
	    else
		wield = get_eq_char( ch, WEAR_SECONDARY );
    }

    /* but do we have a funky weapon? */
    if (result == fOK
    &&  wield != NULL
    && !char_died(ch)
    && !char_died(victim))
    { 
	// usamos dam (parametro) como valor temporal

	if (ch->fighting == victim && HAS_TRIGGER(wield, TRIG_HACER) )
		mp_percent_trigger( objToEnt(wield), chToEnt(ch), NULL, chToEnt(victim), TRIG_HACER );

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	{
	    int level;
	    AFFECT_DATA *poison, af;

	    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
		level = wield->level;
	    else
		level = poison->level;
	
	    if (!saves_spell(level / 2,victim,DAM_POISON)) 
	    {
		send_to_char("You feel poison coursing through your veins.\n\r",
		    victim);
		act("$n is poisoned by the venom on $p.",
		    victim,objToEnt(wield),NULL,TO_ROOM);

    		af.where     = TO_AFFECTS;
    		af.type      = gsn_poison;
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }

	    /* weaken the poison if it's temporary */
	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
		    act("The poison on $p has worn off.",ch,objToEnt(wield),NULL,TO_CHAR);
	    }
 	}

    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    act("$p draws life from $n.",victim,objToEnt(wield),NULL,TO_ROOM);
	    act("You feel $p drawing your life away.",
		victim,objToEnt(wield),NULL,TO_CHAR);
	    newdamage(chToEntidad(ch, FALSE),victim,dam,0,DAM_NEGATIVE,FALSE);
	    ch->alignment = UMAX(-1000,ch->alignment - 1);
	    ch->hit += dam/2;
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
	{
	    dam = number_range(1,wield->level / 4 + 1);
	    act("$n is burned by $p.",victim,objToEnt(wield),NULL,TO_ROOM);
	    act("$p sears your flesh.",victim,objToEnt(wield),NULL,TO_CHAR);
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	    newdamage(chToEntidad(ch, FALSE),victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	{
	    dam = number_range(1,wield->level / 6 + 2);
	    act("$p freezes $n.",victim,objToEnt(wield),NULL,TO_ROOM);
	    act("The cold touch of $p surrounds you with ice.",
		victim,objToEnt(wield),NULL,TO_CHAR);
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    newdamage(chToEntidad(ch, FALSE),victim,dam,0,DAM_COLD,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	    dam = number_range(1,wield->level/5 + 2);
	    act("$n is struck by lightning from $p.",victim,objToEnt(wield),NULL,TO_ROOM);
	    act("You are shocked by $p.",victim,objToEnt(wield),NULL,TO_CHAR);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    newdamage(chToEntidad(ch, FALSE),victim,dam,0,DAM_LIGHTNING,FALSE);
	}
    }

    tail_chain( );
    return;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (destruccion || EN_ARENA(victim) || war == TRUE)
	return FALSE;

    if (ch != victim
    && !IS_NPC(ch)
    && !IS_NPC(victim)
    &&  victim->desc
    &&  victim->desc->editor != ED_NONE )
    {
    	send_to_char( "Esa persona esta editando. Dejala tranquila.\n\r", ch );
    	return TRUE;
    }

    if ( !IS_NPC( ch ) && IS_AFFECTED2( ch, AFF_GHOUL ) )
    {
	send_to_char(
	     "You may not participate in combat while in ghoul form.\n\r",
		     ch );
	return TRUE;
    }

    if ( !IS_NPC( victim ) && IS_AFFECTED2( victim, AFF_GHOUL ) )
    {
	act( "Your attack passes through $N.", ch, NULL, chToEnt(victim), TO_CHAR );
	act( "$n's attack passes through $N.", ch, NULL, chToEnt(victim), TO_NOTVICT );
	act( "$n's attack passes through you.", ch, NULL, chToEnt(victim), TO_VICT );
	return TRUE;
    }

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && getNivelPr(ch) > LEVEL_IMMORTAL)
	return FALSE;

    /* safe room? */
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)
     || IS_SET(victim->in_room->room_flags,ROOM_PET_SHOP))
    {
	send_to_char("No en este lugar.\n\r",ch);
	return TRUE;
    }

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	if (victim->pIndexData->pShop != NULL)
	{
	    send_to_char("Al vendedor no le gustaria eso!\n\r",ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act,ACT_BANKER))
	{
	    send_to_char("No creo que Mota lo aprobaria.\n\r",ch);
	    return TRUE;
	}

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	    {
		act("But $N looks so cute and cuddly...",
		    ch,NULL,chToEnt(victim),TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	    {
		send_to_char("You don't own that monster.\n\r",ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else
    {
	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
	    {
		send_to_char("Players are your friends!\n\r",ch);
		return TRUE;
	    }

	}
	/* player doing the killing */
	else
	{
	    if (!is_clan(ch))
	    {
		send_to_char("Unete a un clan si quieres matar jugadores.\n\r",ch);
		return TRUE;
	    }

	    if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
	    {
		send_to_char("No esta en un clan, no molestes.\n\r",ch);
		return TRUE;
	    }

#if defined(RANGO_PK)
	    if ( (getNivelPr(ch) > getNivelPr(victim) + RANGO_PK)
	    ||   (getNivelPr(ch) + RANGO_PK < getNivelPr(victim)) )
	    {
		send_to_char("Busca a alguien de tu porte.\n\r",ch);
		return TRUE;
	    }
#endif

	    if (IS_SET(victim->act, PLR_NOPK))
	    {
	    	send_to_char("Ese jugador todavia se esta recuperando.\n\r", ch);
	    	return TRUE;
	    }
	}
    }
    return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (destruccion || EN_ARENA(victim) || war == TRUE)
    	return FALSE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && getNivelPr(ch) > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* safe room? */
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)
     || IS_SET(victim->in_room->room_flags,ROOM_PET_SHOP))
	return TRUE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && getNivelPr(victim) > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}

	/* player doing the killing */
	else
	{
	    if (!is_clan(ch))
		return TRUE;

	    if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
		return TRUE;

	    if (is_same_clan(ch,victim) && area)
	    	return TRUE;

	    if (is_same_group(ch,victim) && area)
	    	return TRUE;

#if defined(RANGO_PK)
	    if ( (getNivelPr(ch) > getNivelPr(victim) + RANGO_PK)
	    ||   (getNivelPr(ch) + RANGO_PK < getNivelPr(victim)) )
		return TRUE;
#endif

	    if (IS_SET(victim->act, PLR_NOPK))
	    	return TRUE;
	}
    }
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    if (destruccion || EN_ARENA(victim) || war == TRUE)
    	return;

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_IMMORTAL(victim)
    ||   (victim->race == RACE_VAMPIRE)
    ||   IS_SET(victim->act, PLR_THIEF))
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf2[MAX_STRING_LENGTH];

	    sprintf( buf2, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf2, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   getNivelPr(ch) >= LEVEL_IMMORTAL
    ||  (is_clan(ch) && is_clan(victim))
    ||   IS_SET(ch->act, PLR_KILLER) 
    ||	 ch->fighting  == victim)
	return;

    send_to_char( "*** Ahora eres un #BKILLER#b!! ***\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    sprintf(buf,"$N is attempting to murder %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + getNivelPr(victim) - getNivelPr(ch) )
	return FALSE;

    act( "Detienes con tu arma el ataque de $n.",  ch, NULL, chToEnt(victim), TO_VICT    );
    act( "$N detiene tu ataque con su arma.", ch, NULL, chToEnt(victim), TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    OBJ_DATA *shield;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    chance = get_skill(victim,gsn_shield_block) / 5 + 3;

    if ( ( shield = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + getNivelPr(victim) - getNivelPr(ch) )
        return FALSE;

    act( "Bloqueas el ataque de $n con tu escudo.",  ch, NULL, chToEnt(victim), TO_VICT );
    act( "$N bloquea tu ataque con su escudo.", ch, NULL, chToEnt(victim), TO_CHAR );
    act( "$n bloquea el ataque de $N con su escudo.", ch, NULL, chToEnt(victim), TO_NOTVICT );
    check_improve(victim,gsn_shield_block,TRUE,6);

    if (number_bits(2) == 0)
	shield->condition--;

    if (shield->condition <= 0)
    	{
    		act( "Tu escudo se quebro!", ch, NULL, chToEnt(victim), TO_VICT );
    		act( "El escudo de $N se quebro!", ch, NULL, chToEnt(victim), TO_CHAR );
    		act( "El escudo de $N se quebro!", ch, NULL, chToEnt(victim), TO_NOTVICT );
    		extract_obj( shield, TRUE );
	}

    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + getNivelPr(victim) - getNivelPr(ch) )
    {
    	check_improve(victim, gsn_dodge, FALSE, 6);
        return FALSE;
    }

    act( "Esquivas el ataque de $n.", ch, NULL, chToEnt(victim), TO_VICT    );
    act( "$N esquiva tu ataque.", ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n esquiva el ataque de $N.", ch, NULL, chToEnt(victim), TO_NOTVICT );

    check_improve(victim,gsn_dodge,TRUE,6);

    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
	if ( victim->in_room->sector_type == SECT_UNDERWATER
	&&  !IS_IMMORTAL(victim)
	&&   victim->aire == 0 )
	{
		victim->position = POS_STUNNED;
		return;
	}
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( (IS_NPC(victim) || EN_ARENA(victim)) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}

/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    if ( IS_NPC(ch) )
    {
	EVENT *ev;

    	if ( (ev = event_pending( ch->events, mob_fight )) )
    		event_delete( ev );

	if ( is_mob_fighting(ch) )
	{
		bug( "Set_fighting : mob %d ya estaba en lista", ch->pIndexData->vnum );
		stop_mob_fight(ch);
	}

	set_mob_fight(ch);
    }

    /* fightdata */
    if ( fdata_lookup( ch, victim->id ) == NULL )
	give_fd( ch, victim );

    ch->fighting	= victim;
    ch->position	= POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    if (fBoth == FALSE)
    {
	if (IS_NPC(ch) && is_mob_fighting(ch))
		stop_mob_fight(ch);
    	ch->fighting = NULL;
    	ch->position = IS_NPC(ch) ? ch->default_pos : POS_STANDING;
    	update_pos(ch);
    	return;
    }

    // si se modifica ARREGLAR extract_char en handler.c
    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( (fBoth && fch == ch) || fch->fighting == ch )
	{
	    if ( IS_NPC(fch) && is_mob_fighting(fch) )
		stop_mob_fight(fch);

	    fch->fighting = NULL;
	    fch->position = IS_NPC(fch) ? fch->default_pos : POS_STANDING;

	    update_pos( fch );
	}
    }

    return;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, int type )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    ROOM_INDEX_DATA *location = NULL;

    if ( !IS_NPC(ch) )
    {
	if ( getNivelPr(ch) > 20 )
	{
		if ( is_clan(ch) && ES_INDEP(ch->clan) )
			location = get_room_index( MORGUE(ch) );
		else
			location = ch->in_room;
	}
	else
		location = get_room_index ( MORGUE(ch) );
    }

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( DINERO(ch) > 0
	&&  !IS_SET(ch->act, ACT_PROTOTIPO) )
	{
	    obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->value[1] = IS_SET(ch->form, FORM_POISON) ? 1 : 0;
	corpse->value[2] = ch->parts;
	corpse->value[3] = ch->pIndexData->vnum;
    }
    else /* PC */
    {
	name			= ch->name;
	corpse			= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer		= number_range( 25, 40 );
	ch->pcdata->corpse	= corpse;
	REMOVE_BIT(ch->act,PLR_CANLOOT);

	free_string(corpse->owner);
	corpse->owner = str_dup(ch->name);

	if (is_clan(ch) && (ch->gold > 2 || ch->silver > 2))
	{
		obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);
		ch->gold	-= ch->gold/2;
		ch->silver	-= ch->silver/2;
	}
    }

    corpse->cost  = 0;
    corpse->level = getNivelPr(ch);

    sprintf( buf, "corpse cuerpo %s", name );
    free_string( corpse->name );
    corpse->name = str_dup( buf );

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    switch(type)
    {
    	case DAM_FIRE:
    		sprintf(buf, "El cuerpo carbonizado de %s esta aqui.", name );
    		break;
    	case DAM_COLD:
    		sprintf(buf, "El cuerpo congelado de %s esta aqui.", name );
    		break;
    	case DAM_ACID:
    		sprintf(buf, "El cuerpo de %s esta aqui, corroido por el acido.", name );
    		break;
    	case DAM_PIERCE:
    		sprintf(buf, "El cuerpo de %s esta aqui, todavia desangrandose.", name );
    		break;
    	case DAM_SLASH:
    		sprintf(buf, "El cuerpo mutilado de %s esta aqui.", name );
    		break;
    	case DAM_BASH:
    		sprintf(buf, "El cuerpo destrozado de %s esta aqui.", name );
    		break;
    	default:
    		sprintf(buf, corpse->description, name );
    		break;
    }

    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_FLAMING);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj, TRUE );

	else if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch,objToEnt(obj),NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,objToEnt(obj),NULL,TO_ROOM);
		 extract_obj(obj, TRUE);
	    }
	    else
	    {
		act("$p falls to the floor.",ch,objToEnt(obj),NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	    }
	}
	else
	    obj_to_obj( obj, corpse );
    }

    if ( IS_NPC(ch) )
	obj_to_room( corpse,ch->in_room );
    else
	obj_to_room( corpse,location );

    return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "Escuchas el grito de agonia de $n.";

    switch ( number_bits(4) )
    {
    case  0:
		msg  = "$n cae al suelo ... MUERTO.";
	break;
    case  1: 
	if (ch->material == 0)
	{
		if (CHANCE(50))
			msg = "$n se deshace en un mar de sangre y visceras.";
		else
			msg  = "Un chorro de sangre de $n ensucia tu armadura.";
	}
	break;
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    if (CHANCE(50))
	    	msg = "$n se deshace en un mar de sangre y visceras.";
	    else
	    	msg = "$n reparte sus visceras por todo el suelo.";
	    vnum = OBJ_VNUM_GUTS;
	    REMOVE_BIT(ch->parts, PART_GUTS);
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    if ( CHANCE(50) )
	    	msg = "El cuello de $n cede, y su cabeza cae al suelo.";
	    else
	    	msg = "La cabeza ensangrentada de $n cae al suelo.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	    REMOVE_BIT(ch->parts, PART_HEAD);
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    if (CHANCE(50))
	    	msg = "El corazon de $n cae desde su pecho.";
	    else
	    	msg = "El corazon todavia palpitante de $n cae al suelo.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	    REMOVE_BIT(ch->parts, PART_HEART);
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "El brazo de $n cae de su cuerpo mutilado.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	    REMOVE_BIT(ch->parts, PART_ARMS);
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "La pierna de $n cae de su cuerpo mutilado.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	    REMOVE_BIT(ch->parts, PART_LEGS);
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    if (CHANCE(50))
	    	msg = "Tu ultimo golpe #BDESTROZA#b la cabeza de $n, y sus sesos quedan repartidos por el suelo.";
	    else
	    	msg = "La cabeza de $n se desarma, y sus sesos quedan repartidos por el suelo.";
	    vnum = OBJ_VNUM_BRAINS;
	    REMOVE_BIT(ch->parts, PART_BRAINS);
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		SET_BIT(obj->value[3], FOOD_POISON);
	    if (IS_SET(ch->form,FORM_MAGICAL))
	    	SET_BIT(obj->value[3], FOOD_MAGIC);
	    if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type	= ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "Escuchas el grito de agonia de algo.";
    else
	msg = "Escuchas el grito de agonia de alguien.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= (MAX_DIR - 1); door++ )
    {
	EXIT_DATA *pexit;

	if ( (pexit = exit_lookup(was_in_room, door)) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

void raw_kill( Entity * ent, CHAR_DATA *victim, int type )
{
    int i;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    bool temp = FALSE;

    if (ent)
    {
    	if ( entComparar(ent, chToEntidad(victim, FALSE)) == FALSE )
    		mensaje( ent, victim );
    	if ( !IS_NPC(victim) )
    		mensaje_muerte( ent, victim, type );
    }

    stop_fighting( victim, TRUE );
    death_cry( victim );
    make_corpse( victim, type );
    poner_ent_lista( chToEntidad(victim, TRUE) );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, getNivelPr(victim), MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );

    if ( IS_AFFECTED2( victim, AFF_VAMP_BITE )
    &&  !IS_AFFECTED(victim, AFF_POLYMORPH)
    &&   victim->race != RACE_VAMPIRE )
    	polymorph( victim, RACE_VAMPIRE );

    for ( paf = victim->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->where == TO_AFFECTS2 && paf->bitvector == AFF_GHOUL )
	{
		temp = TRUE;
		continue;
	}

        affect_remove( victim, paf );
    }

    victim->pcdata->quaff	= 1;
    victim->fall		= 0;
    victim->affected_by		= race_table[victim->race].aff;
    victim->affected2_by	= race_table[victim->race].aff2;

    if (temp)
    	SET_BIT(victim->affected2_by, AFF_GHOUL );

    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;

    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    victim->parts	= race_table[victim->race].parts;
/*  save_char_obj( victim ); we're stable enough to not need this :) */

    victim->pcdata->condition[COND_FULL  ] = 0;
    victim->pcdata->condition[COND_THIRST] = 25;
    victim->pcdata->condition[COND_HUNGER] = 25;

    if ( !IS_NPC(victim) )
    {
	SET_BIT(victim->act, PLR_NOPK);
	char_event_add( victim, 180*PULSE_PER_SECOND, (void *) PLR_NOPK, actremove );
	char_event_add( victim, 180*PULSE_PER_SECOND, "Te sientes menos seguro.\n\r", stc_event );
    }

    return;
}

int group_gain( Entity *ent, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch = entEsCh(ent) ? entGetCh(ent) : NULL;
    int xp, nxp = 0;
    int members;
    int group_levels;
    int highestlevel;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( entEsCh(ent)
    &&   entGetCh(ent) == victim )
	return 0;

    members		= 0;
    group_levels	= 0;
    highestlevel	= 0;

    for ( gch = entGetPeopleRoom(ent); gch != NULL; gch = gch->next_in_room )
    {
	if ( ent_is_same_group( chToEnt(gch), ent ) )
        {
	    members++;
//	    group_levels += IS_NPC(gch) ? getNivelPr(gch) / 2 : getNivelPr(gch);
	    group_levels += getNivelPr(gch);
	    if (getNivelPr(gch) > highestlevel)
	    {
	    	lch = gch;
	    	highestlevel = getNivelPr(gch);
	    }
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = entGetNivel(ent);
    }

    for ( gch = entGetPeopleRoom(ent); gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !ent_is_same_group( chToEnt(gch), ent ) )
	    continue;

	if ( getNivelPr(gch) <= (highestlevel - 8) )
	{
		send_to_char( "Tus poderes son inutiles para un grupo tan avanzado de aventureros.\n\r", gch );
		continue;
	}

	if ( esta_en_quest(gch)
	&&  !quest_completo(gch)
	&&   es_quest_mob(gch, victim) )
	{
		send_to_char( "Has completado tu #BQUEST#b!\n\r", gch );
		send_to_char( "Vuelve donde tu questmaster antes de que el tiempo se agote!\n\r", gch );
		completar_quest(gch);
	}

	if ( !IS_NPC(gch)
	&&   !IS_AFFECTED(gch, AFF_POLYMORPH)
	&&   (gch->race == RACE_VAMPIRE || IS_AFFECTED2(gch, AFF_VAMP_BITE))
	&&    gch->true_race != RACE_VAMPIRE )
	{
		MEM_DATA * mem = mem_lookup( gch->memory, MEM_VAMPIRE );

		if ( mem
		&&   mem->id == (IS_NPC(victim) ? victim->pIndexData->vnum : victim->id) )
		{
			send_to_char( "Tu maldicion ha sido cancelada!\n\r", gch );
			if (gch->race == RACE_VAMPIRE)
			{
				send_to_char( "Tu cuerpo vuelve a su forma original.\n\r", gch );
				flog( "%s vuelve a ser %s.", gch->name, race_table[gch->true_race].name );
				polymorph(gch, gch->true_race);
			}
			strip_mem_char(gch, MEM_VAMPIRE);
			affect_strip(gch, gsn_vampiric_bite);
		}
	}

	xp = xp_compute( gch, victim, group_levels );

	if ( members > 1 && lch && HATES( gch, lch ) )
	{
		send_to_char( "Pierdes un tercio de tu exp por agruparte con basura.\n\r", gch );
		xp *= 0.66;
	}

	sprintf( buf, "Recibes #B%d#b puntos de experiencia.\n\r", xp );
	send_to_char( buf, gch );

        if (!IS_NPC(gch) && (xp > 0))
        {
        	++gch->pcdata->muertes;
          	revisa_muertes(gch);
        }

	if ( entEsCh(ent) && gch == entGetCh(ent) )
		nxp = xp;

	gain_exp( gch, xp );

	for ( obj = gch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch) ) )
	    {
		act( "Sientes que $p se separa de tu cuerpo y cae.", gch, objToEnt(obj), NULL, TO_CHAR );
		act( "$p se separa de $n y cae.",   gch, objToEnt(obj), NULL, TO_ROOM );
		if ( IS_OBJ_STAT(obj, ITEM_MELT_DROP) )
		{
			act( "$p se deshace en el suelo.", gch, objToEnt(obj), NULL, TO_ALL );
			extract_obj(obj, TRUE);
		}
		else
		{
			obj_from_char( obj );
			if ( IS_OBJ_STAT(obj, ITEM_NODROP) )
				obj_to_char( obj, gch );
			else
				obj_to_room( obj, gch->in_room );
		}
	    }
	}
    }

    return nxp;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range, aldif;
    int change;
    float xpmult, xpm = 0;
    char buf[MSL];
    FIGHT_DATA * fd;

    if (IS_NPC(victim)
    &&  (IS_SET(victim->act, ACT_PROTOTIPO)
      || IS_SET(victim->in_room->room_flags, ROOM_PROTOTIPO)) )
       return 0;

    if ( !IS_NPC(gch)
    &&   !IS_NPC(victim)
    &&    is_clan(gch)
    &&    is_clan(victim)
    &&    is_same_clan(gch,victim) )
    	return 0;

    level_range = getNivelPr(victim) - getNivelPr(gch);
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    }
    
    if (level_range > 4)
	base_exp = 160 + 20 * (level_range - 4);

    /* do alignment computations */
    align = victim->alignment - gch->alignment;

    if (IS_NPC(victim) && IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * getNivelPr(gch)/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * getNivelPr(gch)/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * getNivelPr(gch)/total_levels;  
	gch->alignment -= change;
    }

    aldif = abs(gch->alignment - victim->alignment);

    if (aldif < 250)
    	xpm = 0.7;
    else
    if (aldif < 500)
    	xpm = 0.8;
    else
    if (aldif < 750)
    	xpm = 0.9;
    else
    if (aldif < 1250)
    	xpm = 1.0;
    else
    if (aldif < 1500)
    	xpm = 1.05;
    else
    if (aldif < 1750)
    	xpm = 1.1;
    else
    	xpm = 1.2;

    if (IS_NEUTRAL(gch))
    	xpm *= 1.1;

    /* calculate exp multiplier */
    if (IS_NPC(victim) && IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;
    else
    	xp = (int) ((float) base_exp * (float) xpm);

    if (IS_NPC(victim))
    {
	AFFECT_DATA * paf;

	if (IS_AFFECTED2(victim, AFF_MUTE)
	&& (paf = affect_find(victim->affected, gsn_mute)) != NULL
	&&  paf->caster_id != gch->id)
	{
		if (IS_MOB_MAGIC(victim))
			xp /= 3;
		else
			xp *= 0.8;
	}

    	if ( !IS_SET(victim->pIndexData->affected_by, AFF_SANCTUARY)
    	&&   (IS_SET(victim->pIndexData->act, ACT_WARRIOR)
    	||    IS_SET(victim->pIndexData->act, ACT_THIEF)) )
		xp *= 0.95 - ((float) (getNivelPr(victim))/160.0);

	if ( !IS_SET(victim->pIndexData->affected_by, AFF_HASTE)
	&&   !IS_SET(victim->off_flags, OFF_FAST)
	&&   (IS_SET(victim->pIndexData->act, ACT_WARRIOR)
	 ||   IS_SET(victim->pIndexData->act, ACT_THIEF)) )
	 	xp *= 0.95 - ((float) (getNivelPr(victim))/200.0);

	if ( IS_SET(victim->pIndexData->affected_by, AFF_SLOW) )
		xp *= 0.7;
	if ( IS_SET(victim->pIndexData->affected_by, AFF_PLAGUE) )
		xp *= 0.7;
	if ( IS_SET(victim->pIndexData->affected_by, AFF_POISON) )
		xp *= 0.7;
	if ( IS_MOB_MAGIC(victim)
	&&   IS_SET(victim->pIndexData->affected2_by, AFF_MUTE) )
		xp *= 0.5;
    }

    /* hate */
    if ( HATES(gch, victim) )
	xp += xp / 10;

    if ( victim->race == gch->race )
	xp -= xp / 10;

    if ( IS_NPC(victim) )
    {
	if (!IS_SET(victim->act,ACT_CLERIC)
	&&  !IS_SET(victim->act,ACT_MAGE)
	&&  !IS_SET(victim->act,ACT_THIEF)
	&&  !IS_SET(victim->act,ACT_WARRIOR)
	&&  !IS_SET(victim->act,ACT_PSI))
		xp *= 0.85;
    }

    /* randomize the rewards */
    xp = number_range (xp * 5/6, xp * 7/6);

    /* adjust for grouping */
//  xp = xp * getNivelPr(gch)/( UMAX(1,total_levels - 1) );
    xp = xp * getNivelPr(gch)/UMAX(1, URANGE(getNivelPr(gch), total_levels, getNivelPr(gch)*2));

    /* fightdata */
    fd		 = fdata_lookup( gch, victim->id );
    xpmult	 = fd ? UMIN( fd->dam, victim->max_hit ) : 0;
    xpmult	/= victim->max_hit;
    xp		*= xpmult;

    if ( !IS_NPC(gch)
    && (getNivelPr(gch) > 25)
    && ((xp > 150) || ((getNivelPr(victim) - getNivelPr(gch)) > 3)) )
    {
    	sprintf( buf, "%s gano %d xp matando a %s, vnum %d, nivel %d, room %d",
    		gch->name,
    		xp,
    		victim->name,
    		CHARVNUM(victim),
    		getNivelPr(victim),
    		victim->in_room->vnum );
    	log_string( buf );
    	wiznet( buf, gch, NULL, WIZ_XP, 0, 0 );
    }

    return xp;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim, bool second )
{
    OBJ_DATA *obj;
    int wear_pos = second ? WEAR_SECONDARY : WEAR_WIELD;

    if ( ( obj = get_eq_char( victim, wear_pos ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("Su arma no se solto!",ch,NULL,chToEnt(victim),TO_CHAR);
	act("$n intento desarmarte, pero tu arma no se solto!",
	    ch,NULL,chToEnt(victim),TO_VICT);
	act("$n intento desarmar a $N, pero fallo.",ch,NULL,chToEnt(victim),TO_NOTVICT);
	return;
    }

    act( "$n te #B#FDESARMO#n y lanzo tu arma lejos!",
	 ch, NULL, chToEnt(victim), TO_VICT    );
    act( "Desarmas a $N!",  ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n desarmo a $N!",  ch, NULL, chToEnt(victim), TO_NOTVICT );

    DAZE_STATE(victim, 2 * PULSE_VIOLENCE );
    WAIT_STATE(victim, skill_table[gsn_disarm].beats );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(ch)
	&& !IS_NPC(victim)
	&& can_see_obj(ch,obj)
	&& IS_SET(ch->form, FORM_SENTIENT)
	&& CHANCE(getNivelPr(ch) * 2) )
	{
		char_event_add( ch, ch->wait + 1, (int *) ITEM_WEAPON, event_get_obj );
		char_event_add( ch, ch->wait + 2, 0, event_wield );
	}
	if (IS_NPC(victim) && can_see_obj(victim,obj))
	{
		if (victim->wait == 0)
		{
			get_obj(victim,obj,NULL);
			wear_obj( victim, obj, FALSE );
		}
		else
		{
			char_event_add( victim, victim->wait, (int *) ITEM_WEAPON, event_get_obj );
			char_event_add( victim, victim->wait + 1, 0, event_wield );
		}
	}
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ( !can_prac(ch, gsn_berserk) )
    {
	send_to_char("Tu cara se pone roja, pero nada sucede.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK)
    ||  is_affected(ch,gsn_berserk)
    ||  is_affected(ch,gsn_frenzy))
    {
	send_to_char("Te pones un poco mas loco.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("Estas demasiado tranquilo como para enfurecerte.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("No tienes demasiada energia.\n\r",ch);
	return;
    }

    chance = get_skill(ch, gsn_berserk);

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += getNivelPr(ch) * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Tu pulso se acelera mientras eres consumido por la #BRABIA#b!\n\r",ch);

	if (IS_SET(ch->parts, PART_EYE))
		act("Los ojos de $n se tornan salvajes.",ch,NULL,NULL,TO_ROOM);

	check_improve(ch,gsn_berserk,TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= gsn_berserk;
	af.level	= getNivelPr(ch);
	af.duration	= number_fuzzy(getNivelPr(ch) / 8);
	af.modifier	= UMAX(1,getNivelPr(ch)/5);
	af.bitvector 	= AFF_BERSERK;
	af.caster_id	= ch->id;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (getNivelPr(ch)/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Tu pulso se acelera, pero nada sucede.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    FRetVal rval;

    one_argument(argument,arg);

    if ( !can_prac(ch, gsn_bash) )
    {
	send_to_char("Empujar? Que es eso?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("Pero no estas peleando!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("No esta aqui.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("Mejor espera a que se levante primero.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    chance = get_skill(ch, gsn_bash);

    if (is_safe(ch,victim))
	return;

    if ( IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("Pero $N es tu amigo!",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 

    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20; /* 30 */

    /* level */
    chance += (getNivelPr(ch) - getNivelPr(victim));

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    check_killer(ch,victim);

    /* now the attack */
    if (number_percent() < chance )
    {
    	act("$n te envia lejos con un poderoso empujon!",
		ch,NULL,chToEnt(victim),TO_VICT);
	act("Empujas a $N, y l$O haces volar!", ch, NULL, chToEnt(victim), TO_CHAR);
	act("$n hace volar a $N con un poderoso empujon.",
		ch,NULL,chToEnt(victim),TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	WAIT_STATE(victim, skill_table[gsn_bash].beats / 4);
	DAZE_STATE(victim, skill_table[gsn_bash].beats * (IS_NPC(victim) ? 2 : 3) / 3);
	rval = damage(ch, victim,
		getNivelPr(ch)/3 + number_range( 1, 1 + 2 * ch->size + chance/20 ),
		gsn_bash, DAM_BASH,FALSE);
	if (rval == fOK)
		victim->position = POS_RESTING;
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
	act( "Te vas de hocico al suelo!",
	    ch,NULL,chToEnt(victim),TO_CHAR);
	act( "$n se va de hocico al suelo.",
	    ch,NULL,chToEnt(victim),TO_NOTVICT);
	act( "Evades el empujon de $n, logrando que se vaya de hocico al suelo.",
	    ch,NULL,chToEnt(victim),TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	WAIT_STATE(ch,skill_table[gsn_bash].beats );
	ch->position = POS_RESTING;
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    FRetVal rval;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) < 1
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT)) )
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("No esta aqui.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
    	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
/*  chance -= 2 * get_curr_stat(victim,STAT_DEX); */
    chance -= get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
/*  chance += (getNivelPr(ch) - getNivelPr(victim)) * 2; */
    chance += (getNivelPr(ch) - getNivelPr(victim)) * 4;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
	case(SECT_UNDERWATER):		chance  =  0;	break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
	act("$n kicks dirt in your eyes!",ch,NULL,chToEnt(victim),TO_VICT);
	rval = damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	if ( rval == victdead )
		return;
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.level 	= getNivelPr(ch);
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;
	af.caster_id	= ch->id;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
	check_killer(ch,victim);
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    FRetVal rval;

    one_argument(argument,arg);

    if ( !can_prac(ch, gsn_trip) )
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("No esta aqui.\n\r",ch);
	return;
    }

    chance = get_skill(ch, gsn_trip);

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
    	return;
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    check_killer(ch,victim);

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (getNivelPr(ch) - getNivelPr(victim)) * 2;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,chToEnt(victim),TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,chToEnt(victim),TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,chToEnt(victim),TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,		skill_table[gsn_trip].beats);
	WAIT_STATE(victim,	skill_table[gsn_trip].beats / 2);
	rval = damage(ch,victim,number_range(2, 2 + getNivelPr(ch)/3 + 3 * victim->size),gsn_trip,
		DAM_BASH,FALSE);
	if (rval == fOK)
		victim->position = POS_RESTING;
   }
   else
   {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
   } 
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Atacar a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }
/*  Allow player killing
    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
        &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }
*/
    if ( victim == ch )
    {
	send_to_char( "Te golpeas.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( es_kill_steal(ch, victim, TRUE) )
    	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N es tu adorado maestro.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if ( !IS_NPC(ch)
    &&   !IS_NPC(victim)
    &&   !EN_ARENA(victim)
    &&    is_clan(ch)
    &&    is_clan(victim)
    &&    is_same_clan(ch, victim) )
    {
    	act( "Debes ASESINAR a $N.", ch, NULL, chToEnt(victim), TO_CHAR );
    	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "Haces lo mejor que puedes!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if (IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
    	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "Help! I am being attacked by %s!",ch->short_descr);
    else
    	sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );

    if ( !IS_NPC(ch) && !IS_NPC(victim) )
    {
	AFFECT_DATA af;

	af.where	= TO_AFFECTS;
	af.type		= gsn_curse;
	af.level	= LEVEL_HERO;
	af.duration	= 1;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_CURSE;
	af.caster_id	= ch->id;

	if ( !IS_AFFECTED(ch, AFF_CURSE) )
		affect_to_char( ch, &af );
	if ( !IS_AFFECTED(victim, AFF_CURSE) )
	{
		af.caster_id = victim->id;
		affect_to_char( victim, &af );
	}
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Backstab whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("No esta aqui.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if (IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
	return;

    if ( (!IS_NPC(ch) || IS_PET(ch))
    &&   (obj = get_eq_char( ch, WEAR_WIELD )) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 3)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    check_killer( ch, victim );

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );

    if ( number_percent( ) < get_skill(ch,gsn_backstab)
    || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE);
    }

    return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
 
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }
 
    if ( !can_circle(ch, victim) )
    {
    	act( "No puedes distraer a $N lo suficiente.", ch, NULL, chToEnt(victim), TO_CHAR );
    	return;
    }

    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
        damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE);
    }
 
    return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
		ch->position = POS_STANDING;
	send_to_char( "No estas peleando.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_HOLD ) ) 
    {
	send_to_char( "Estas atascado en una red!  No puedes moverte!\n\r", ch );
	act( "$n intento huir, pero esta atrapado en una red!",
	    ch, NULL, NULL, TO_ROOM );
	return;
    }

    was_in = ch->in_room;

    for ( attempt = 0; attempt < 7; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );

	if ( (pexit = exit_lookup(was_in, door)) == NULL
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0, ch->daze/2) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	if (move_char( ch, door, FALSE ) == fFAIL)
	{
		send_to_char( "Algo paso.\n\r", ch );
		return;
	}

	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n huyo!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch)
	&&   !EN_ARENA(ch)
	&&   !IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO) )
	{
		send_to_char( "Huyes del combate!\n\r", ch );

		if (es_clase(ch, CLASS_THIEF)
		&& (number_percent() < 3*(getNivelPr(ch)/2) ) )
			send_to_char( "Huyes en forma segura.\n\r", ch);
		else
		{
			send_to_char( "Pierdes 10 puntos de exp.\n\r", ch); 
			gain_exp( ch, -10 );
		}
	}

	mob_odio_check(was_in, ch);

	stop_fighting(ch, TRUE);

	if ( IS_NPC(ch)
	&&  (IS_SET(ch->act, ACT_WIMPY)
	 ||  getNivelPr(victim) > getNivelPr(ch) + 10
	 || (IS_MOB_CASTER(ch) && !CAN_SAY(ch))
	 || (IS_SET(ch->act, ACT_SENTINEL) && ch->hit < ch->max_hit / 4) )
	&&   mem_lookup_id(ch->memory, victim->id) == NULL )
		give_mem( ch, MEM_AFRAID, victim->id );

	return;
    }

    send_to_char( "#BPANICO#b! No pudiste huir!\n\r", ch );

    return;
}

void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    if ( IS_NPC(fch) && es_kill_steal(ch, fch, TRUE) )
    	return;

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n rescues you!", ch, NULL, chToEnt(victim), TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, chToEnt(victim), TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    if (ch->fighting)
    	stop_fighting( ch, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( (chance = get_skill(ch, gsn_kick)) < 1 )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "No estas peleando con nadie.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    check_killer(ch,victim);

    chance = get_skill(ch, gsn_kick);

    if ( !can_see(ch, victim) )
    {
    	if ( CHANCE(get_skill(ch, gsn_blindfight)) )
    		chance = chance * 2 / 3;
    	else
		chance /= 2;
    }

    if ( CHANCE(chance) )
    {
	int lorange = get_curr_stat(ch, STAT_STR)/4 + getNivelPr(ch)*3/4 + chance/8 + IS_NPC(ch) ? getNivelPr(ch) / 4 : 0;
	int hirange = lorange + 2*getNivelPr(ch);

	damage(ch,victim,number_range(lorange, hirange), gsn_kick, DAM_BASH, TRUE );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	int blah = number_range(0,2);
	char * charmsg = "", * victmsg = charmsg, *roommsg = charmsg;

	switch(blah)
	{
		case 0:
		charmsg = "Tu patada no le pega a $N.";
		victmsg = "La debil patada de $n ni siquiera paso cerca tuyo.";
		roommsg = "La debil patada de $n ni siquiera pasa cerca de $N.";
		break;

		case 1:
		charmsg = "Tu patada yerra a $N.";
		victmsg = "La patada de $n te yerra.";
		roommsg = "La patada de $n yerra a $N.";
		break;

		case 2:
		charmsg = "Tu pie va directo hacia la cara de $N, pero $e lo esquiva.";
		victmsg = "El pie de $n va directo hacia tu cara, pero lo esquivas con destreza.";
		roommsg = "El pie de $n va directo hacia la cara de $N, pero $e lo esquiva.";
		break;
	}

	act( charmsg, ch, NULL, chToEnt(victim), TO_CHAR );
	act( victmsg, ch, NULL, chToEnt(victim), TO_VICT );
	act( roommsg, ch, NULL, chToEnt(victim), TO_NOTVICT );

	damage( ch, victim, 0, gsn_kick,DAM_BASH,FALSE);
	check_improve(ch,gsn_kick,FALSE,1);
    }

    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
    bool second;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    && (hth = get_skill(ch,gsn_hand_to_hand)) == 0 )
    {
	send_to_char( "Debes estar esgrimiendo un arma para poder desarmar.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "No estas peleando con nadie.\n\r", ch );
	return;
    }

    if ( (obj = get_eq_char( victim, WEAR_WIELD )) == NULL )
    {
    	if ( (obj = get_eq_char( victim, WEAR_SECONDARY )) == NULL )
    	{
    		send_to_char( "Tu oponente no esta esgrimiendo un arma.\n\r", ch );
    		return;
    	}
    	second = TRUE;
    }
    else
    	second = FALSE;

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch,FALSE));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim,second));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim,second));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (getNivelPr(ch) - getNivelPr(victim)) * 2;
 
    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
    check_killer(ch,victim);

    /* and now the attack */
    if (number_percent() < chance)
    {
	disarm( ch, victim, second );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	act("No pudiste desarmar a $N.",ch,NULL,chToEnt(victim),TO_CHAR);
	act("$n intento desarmarte, pero fallo.",ch,NULL,chToEnt(victim),TO_VICT);
	act("$n intento desarmar a $N, pero fallo.",ch,NULL,chToEnt(victim),TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }

    return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;

    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }

    if ( EN_ARENA(ch) )
    {
    	send_to_char( "No puedes rendirte en la arena. Matar o morir.\n\r", ch );
    	return;
    }

    act( "You surrender to $N!", ch, NULL, chToEnt(mob), TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, chToEnt(mob), TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, chToEnt(mob), TO_NOTVICT );
    stop_fighting( ch, TRUE );

    WAIT_STATE(ch, 4*PULSE_PER_SECOND);

    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( chToEnt(mob), chToEnt(ch), NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, chToEnt(mob), TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && getNivelPr(victim) >= get_trust(ch) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( (IS_NPC(victim) && getNivelPr(ch) < 54 && !IS_BUILDER(ch, ch->in_room->area))
      || (!IS_NPC(victim) && getNivelPr(ch) < 56) )
    {
    	send_to_char( "Fallaste.\n\r", ch );
    	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, chToEnt(victim), TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, chToEnt(victim), TO_NOTVICT );
    raw_kill( chToEntidad(ch, FALSE), victim, DAM_LIGHTNING );
    return;
}

void do_fee( CHAR_DATA *ch, char *argument )
{
    send_to_char(
		 "IF you wish to feed on someone or something, type FEED!\n\r",
		 ch );
    return;
}

/* Vampiric bite.  Feeding */
void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	*victim;
    OBJ_DATA	*obj;
    AFFECT_DATA	af;
    char	arg [ MAX_INPUT_LENGTH ];
    int		dam;
    int		heal;
    FRetVal	rval;

    one_argument( argument, arg );

    if ( arg[0] == '\0' && !ch->fighting )
    {
	send_to_char( "Alimentarte de quien?\n\r", ch );
	return;
    }

    if ( !( victim = ch->fighting ) )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
	}

    if ( victim == ch )
    {
	send_to_char( "You cannot lunge for your own throat.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
        return;

    check_killer( ch, victim );

    send_to_char( "Tu debes tener #BSANGRE#b!\n\r", ch );
    act( "$n salta hacia el cuello de $N!", ch, NULL, chToEnt(victim), TO_NOTVICT );
    act( "$n salta hacia tu cuello!", ch, NULL, chToEnt(victim), TO_VICT );

    WAIT_STATE( ch, 12 );

    if ( ch->race != RACE_VAMPIRE )
    {
        send_to_char( "Fallaste!\n\r", ch );
	if ( !ch->fighting )
	    set_fighting( ch, victim );
	return;
    }

    if ( ch->fighting && (number_percent() > (getNivelPr(ch) * 1.5)) )
    {
	send_to_char( "Fallaste!\n\r", ch );
	return;
    }

    /* If the victim is asleep, he deserves to be bitten.  :) */
    if ( !ch->fighting )
    {
	if ( number_percent() > ( getNivelPr(ch) * 2 ) && IS_AWAKE( victim ) )
	{
	    send_to_char( "Fallaste!\n\r", ch );
	    set_fighting( ch, victim );
	    return;
	}
    }

    dam = dice( 3, getNivelPr(ch) * 0.75 );
    rval = damage_old( ch, victim, dam, gsn_vampiric_bite, DAM_POISON, TRUE );

    if (rval == victdead || EN_ARENA(victim))
    	return;

    /* Lets see if we can get some food from this attack */
    if ( !IS_NPC( ch ) && number_percent( ) < ( 40 - getNivelPr(victim) + getNivelPr(ch) ) )
    {
        /* If not hungry, thirsty or damaged, then continue */
        if (   ch->pcdata->condition[COND_FULL  ] > 20
	    && ch->pcdata->condition[COND_THIRST] > 20
	    && ch->pcdata->condition[COND_HUNGER] > 20
	    && ( ch->hit * 100 / ch->max_hit > 75 ) )
	    return;
	else
	{
	    send_to_char( "Ahh!  #BSANGRE#b!  Comida!  Bebida!\n\r", ch );
	    heal = number_range( dam / 2, dam + 1 );
	    ch->hit = UMIN( ch->max_hit, ch->hit + heal );
	    /* Get full as per drinking blood */
	    gain_condition( ch, COND_FULL, heal * 2 );
	    /* Quench thirst as per drinking blood absolute value */
	    gain_condition( ch, COND_HUNGER, heal );
	    gain_condition( ch, COND_THIRST, heal * 1.25 );
	}
    }

    /* Ok now we see if we can infect the victim */
    if ( IS_NPC( victim ) )
        return;

    if ( mem_lookup( victim->memory, MEM_VAMPIRE )
    ||   IS_AFFECTED(victim, AFF_POLYMORPH) )
    	return;

    if ( getNivelPr(victim) < 11 || get_age( victim ) < 21 )
        return;

    if ( saves_spell( getNivelPr(ch), victim, DAM_POISON ) || number_bits( 1 ) )
	return;

    if ( ( obj = get_eq_char( victim, ITEM_HOLD ) ) )
    {
        if ( IS_OBJ_STAT( obj, ITEM_VAMPIRE_BANE ) && getNivelPr(ch) < 21 )
	    return;
	else
	{
	    if ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) )
	    {
	        if ( getNivelPr(ch) < 32 )
			return;
		else
		if ( getNivelPr(victim) > getNivelPr(ch) )
			return;
	    }
	}
    }

    af.where	 = TO_AFFECTS2;
    af.level	 = getNivelPr(ch);
    af.type      = gsn_vampiric_bite;
    af.duration  = UMAX( 5, 30 - getNivelPr(ch) );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_VAMP_BITE;
    affect_join( victim, &af );

    give_mem( victim, MEM_VAMPIRE, IS_NPC(ch) ? ch->pIndexData->vnum : ch->id );

    send_to_char( "Ahh!  Siente el #BPODER#b!\n\r", ch );
    send_to_char( "Tu sangre empieza a #BARDER#b!\n\r", victim );

    return;
}

void do_stake( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *victim;
    OBJ_DATA    *obj;
    AFFECT_DATA  af;
    char         arg [ MAX_INPUT_LENGTH ];
    bool         found = FALSE;
    int          chance;
    FRetVal	rval;

    if ( !check_blind( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stake whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "You may not stake NPC's.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
    	send_to_char( "El suicidio es un pecado mortal.\n\r", ch );
    	return;
    }

    if ( is_safe( ch, victim ) )
        return;

    check_killer( ch, victim );

    /* Check here to see if they have a stake in hand */
    obj = get_eq_char(ch, WEAR_WIELD);
    
    if ( obj
    && (obj->value[0] == WEAPON_DAGGER || obj->value[0] == WEAPON_SPEAR
     || obj->value[0] == WEAPON_EXOTIC)
    &&  obj->material == MAT_MADERA )
	found = TRUE;

    if ( !found )
    {
	send_to_char( "You lack a primary wielded stake to stake.\n\r", ch );
	act(
	    "$n screams and lunges at $N then realizes $e doesnt have a stake",
	    ch, NULL, chToEnt(victim), TO_ROOM );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
    }

    act( "$n screams and lunges at $N with $p.", ch, objToEnt(obj), chToEnt(victim), TO_NOTVICT );
    act( "You scream and lunge at $N with $p.", ch, objToEnt(obj), chToEnt(victim), TO_CHAR );
    act( "$n screams and lunges at you with $p.", ch, objToEnt(obj), chToEnt(victim), TO_VICT );

    /* Calculate chances, give vampire the ghoul effect,
       autokill the vampire. */
    chance = ( IS_NPC( ch ) ? getNivelPr(ch) : ch->pcdata->learned[gsn_stake] );
    chance = chance - getNivelPr(victim) + getNivelPr(ch);

    if ( number_percent( ) < chance
	|| !IS_AWAKE( victim )
	|| victim->race != RACE_VAMPIRE )
    {
	if ( victim->race == RACE_VAMPIRE )
	{
	    rval = damage( ch, victim, victim->hit + 11, gsn_stake, DAM_POISON, TRUE );

	    if (rval == victdead)
	    	return;

	    af.where	 = TO_AFFECTS2;
	    af.level	 = getNivelPr(ch);
	    af.type      = gsn_stake;
	    af.duration  = 10;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_GHOUL;
	    affect_join( victim, &af );
	}
	else
	{
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }
    else
    {
	do_feed( victim, ch->name );
    }
	    
    return;
}

/* This code is for PC's who polymorph into dragons.
 * Yeah I know this is specialized code, but this is fun.  :)
 * Breathe on friend and enemy alike.
 * -Kahn
 */

void pc_breathe( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    send_to_char( "You feel the urge to burp!\n\r", ch );
    act( "$n belches!", ch, NULL, NULL, TO_ROOM );

    if ( (victim = ch->fighting) == NULL )
    	return;
    
    (*skill_table[gsn_fire_breath].spell_fun) ( gsn_fire_breath, getNivelPr(ch) / 2, chToEnt(ch), chToEnt(victim), TARGET_CHAR );

    return;
}

/* This code is for PC's who polymorph into harpies.
 * Yeah I know this is specialized code, but this is fun.  :)
 * Scream into the ears of enemy and friend alike.
 * -Kahn
 */

void pc_screech( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;

    send_to_char( "You feel the urge to scream!\n\r", ch );
    interpret( ch, "scream" );
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
	victim_next = victim->next_in_room;

	if ( victim == ch )
	    continue;

        if ( !IS_NPC( victim )
          /* && ( !registered( ch, victim )
                || ( !licensed( ch ) */
                    && victim->race != RACE_VAMPIRE /* ) ) */ )
            continue;

	act( "Your ears pop from $n's scream.  Ouch!", ch, NULL, chToEnt(victim),
	    TO_VICT );

	(*skill_table[gsn_agitation].spell_fun) ( gsn_agitation, getNivelPr(ch) / 2, chToEnt(ch), chToEnt(victim), TARGET_CHAR );
    }

    return;
}



void pc_spit( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;

    send_to_char( "You feel the urge to spit!\n\r", ch );
    act( "$n spews vitriol!", ch, NULL, NULL, TO_ROOM );
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
	victim_next = victim->next_in_room;

	if ( victim == ch )
	    continue;

	if ( !IS_NPC( victim )
	   /* && ( !registered( ch, victim )
		|| ( !licensed( ch ) */
		    && victim->race != RACE_VAMPIRE /* ) ) */ )
	    continue;

	act( "You are splattered with $n's vitriol.  Ouch!", ch, NULL, chToEnt(victim),
	    TO_VICT );

	(*skill_table[gsn_poison].spell_fun) ( gsn_poison, getNivelPr(ch) / 2, chToEnt(ch), chToEnt(victim), TARGET_CHAR );

	damage( ch, victim, number_range( 1, getNivelPr(ch) ),
	       gsn_poison , DAM_POISON, TRUE );
    }

    return;
}


bool check_race_special( CHAR_DATA *ch )
{
    if ( ch->race == RACE_DRAGON )
    {
	if ( number_percent( ) < getNivelPr(ch) )
	{
	    pc_breathe( ch );
	    return TRUE;
	}
    }

/*  if ( ch->race == race_lookup( "harpy" ) )
    {
	if ( number_percent( ) < getNivelPr(ch) )
	{
	    pc_screech( ch );
	    return TRUE;
	}
    }

    if ( ch->race == race_lookup( "arachnid" )
	|| ch->race == race_lookup( "snake" ) )
    {
	if ( number_percent( ) < getNivelPr(ch) )
	{
	    pc_spit( ch );
	    return TRUE;
	}
    } */

    return FALSE;
}

void mensaje_muerte(Entity * ent, CHAR_DATA *victim, int tipo)
{
	char buf[MAX_STRING_LENGTH];

	if ( entComparar(ent, chToEntidad(victim, FALSE)) == FALSE )
		sprintf(buf, "%s murio a manos de %s\n\r",
		chcolor(victim),
		entidadToString(ent) );
	else
	{
		if ( tipo == DAM_POISON )
			sprintf(buf, "%s murio envenenado\n\r", chcolor(victim) );
		else
			sprintf(buf, "%s se suicido\n\r", chcolor(victim) );
	}

	victim->pcdata->muertes = 0;
	set_puntaje(victim);
	do_info (NULL,buf);
}

void mensaje(Entity * ent, CHAR_DATA *victim)
{
	int rnd_say;
	char *temp = "";
	CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;

	if ( ch == NULL )
		return;

	if (!IS_NPC(ch) && !IS_NULLSTR(ch->pcdata->mensaje))
	{
		mostrar_mensaje( ch, victim );
		return;
	}

        rnd_say = number_range (1, 13);
	switch(rnd_say)
	{
		case 1:		temp = "Death to is the true end...";			break;
		case 2: 	temp = "Pudrete en el infierno...";			break;
		case 3: 	temp = "I will kick your ass!!!";			break;
		case 4: 	temp = "Moriras....arrggh!";				break;
		case 5: 	temp = "Time to die....";				break;
		case 6: 	temp = "Cabrone....";					break;
		case 7: 	temp = "Welcome to your fate....";			break;
		case 8: 	temp = "Hasta la vista...baby";				break;
		case 9: 	temp = "Ever dance with the devil....";			break;
		case 10:	temp = "You're a weak, pathethic fool!";		break;
		case 11:	temp = "It hurts to be you!";				break;
		case 12:	temp = "You're an inspiration for birth control!";	break;
		case 13:	temp = "You #BSUCK#b!";					break;
	}

	act( "$n dice '$T'.", ch, NULL, strToEnt(temp,ch->in_room), TO_ROOM );
	act( "Dices '$T'.", ch, NULL, strToEnt(temp,ch->in_room), TO_CHAR );
}

void do_tail( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    FRetVal rval;

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_tail)) < 1
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL)) )
    {	
	send_to_char("Coletazos? Que es eso?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("Pero no estas peleando!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("No esta aqui.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("Espera a que se levante primero.",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("Intentas descerebrarte con un coletazo, pero fallas.\n\r",ch);
	return;
    }

    if ( !IS_SET(ch->parts, PART_TAIL) )
    {
    	send_to_char("No crees que necesitas una cola?\n\r", ch );
    	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( IS_NPC(victim) && es_kill_steal(ch, victim, TRUE) )
    	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("Pero $N es tu amigo!",ch,NULL,chToEnt(victim),TO_CHAR);
	return;
    }

    check_killer(ch,victim);

    /* modifiers */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 2;

    chance += (ch->size - victim->size) * 20;

    if ( !can_see(victim, ch) )
    	chance += 50;

    if ( !can_see(ch, victim) )
    	chance -= 50;

    if ( IS_AFFECTED(ch, AFF_HASTE)
    ||   IS_SET(ch->off_flags, OFF_FAST) )
    	chance += 30;

    if ( IS_AFFECTED(victim, AFF_HASTE)
    ||   IS_SET(victim->off_flags, OFF_FAST) )
    	chance -= 60;

    if ( IS_AFFECTED(ch, AFF_SLOW) )
    	chance /= 2;

    if ( IS_AFFECTED(victim, AFF_SLOW) )
    	chance *= 1.5;

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_tail].beats);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    	act("$n te golpea con un poderoso coletazo!",
		ch,NULL,chToEnt(victim),TO_VICT);
	act("Golpeas con tu cola a $N, y cae!",ch,NULL,chToEnt(victim),TO_CHAR);
	act("$n golpea a $N con un poderoso coletazo.",
		ch,NULL,chToEnt(victim),TO_NOTVICT);

	check_improve(ch,gsn_tail,TRUE,1);

	if ( (IS_FORM(ch, FORM_INSECT)
	  ||  IS_FORM(ch, FORM_POISON))
	&&   CHANCE(getNivelPr(ch))
	&&  !IS_AFFECTED(victim, AFF_POISON) )
		spell_poison( gsn_poison, getNivelPr(ch) / 2, chToEnt(ch), chToEnt(victim), TARGET_CHAR );

	DAZE_STATE(victim,3 * PULSE_PER_SECOND);
	WAIT_STATE(ch,skill_table[gsn_tail].beats);

	rval = damage(ch,victim,number_range(2,2 + ch->size * 5 + chance/20),gsn_tail,
	    DAM_BASH,FALSE);

	if (rval == fOK)
		victim->position = POS_RESTING;
    }
    else
    {
	damage(ch,victim,0,gsn_tail,DAM_BASH,FALSE);
	act("$N esquiva tu coletazo!",
	    ch,NULL,chToEnt(victim),TO_CHAR);
	act("$n falla al intentar golpear a $N con su cola.",
	    ch,NULL,chToEnt(victim),TO_NOTVICT);
	act("Esquivas el coletazo de $n.",
	    ch,NULL,chToEnt(victim),TO_VICT);
	check_improve(ch,gsn_tail,FALSE,1);
	WAIT_STATE(ch,skill_table[gsn_tail].beats); 
    }
}

/* Mobs using magical items by Spitfire from merc mailing list */
/* Modified to give every magical item an equal chance of being used plus
 * eating pills -Kahn */
bool use_magical_item( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *cobj     = NULL;
    int       number   = 0;
    char      buf[ MAX_INPUT_LENGTH ];

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( (   obj->item_type == ITEM_SCROLL
	      || obj->item_type == ITEM_WAND
	      || obj->item_type == ITEM_STAFF
	      || obj->item_type == ITEM_PILL )
	    && number_range( 0, number ) == 0 )
	{
	    cobj = obj;
	    number++;
	}
    }

    if ( !cobj )
        return FALSE;

    switch( cobj->item_type )
    {
        case ITEM_SCROLL:	do_recite( ch, "scroll" );
				return TRUE;
				break;

	case ITEM_WAND:		if ( cobj->wear_loc == WEAR_HOLD )
				{
					do_zap( ch, "" );
					return TRUE;
				}
				break;

	case ITEM_STAFF:	if ( cobj->wear_loc == WEAR_HOLD )
				{
					do_brandish( ch, "" );
					return TRUE;
				}
				break;

	case ITEM_POTION:	do_quaff( ch, "potion" );
				return TRUE;
				break;

	case ITEM_PILL:		sprintf( buf, "%s", cobj->name );
				do_eat( ch, buf );
				return TRUE;
				break;
    }

    return FALSE;
}

void primer_ataque( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MIL];

	if ( !char_died(ch)
	&&    IS_SET(ch->off_flags, OFF_BACKSTAB)
	&&    victim->hit > victim->max_hit / 3 )
	{
		do_backstab( ch, victim->name );
		return;
	}

	if ( IS_SET(ch->act, ACT_CLERIC) && CHANCE(getNivelPr(ch)*3) )
	{
		char *spell = NULL;
		int num;

		while (spell == NULL)
		{
			num = number_range(1,7);

			switch(num)
			{
				case 1 :	if ( es_caster(victim)
						&&  !IS_AFFECTED2(victim, AFF_MUTE) )
							spell = "mute";
						break;

				case 3 :	spell = "heat metal";
						break;

				case 6 :	if ( !IS_AFFECTED(victim, AFF_BLIND) )
							spell = "blindness";
						break;

				default :	if ( !IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
							spell = "faerie fire";
						break;
			}
		}

		sprintf( buf, "'%s' %s", spell, victim->name );
		do_cast( ch, buf );
		return;
	}

	if ( IS_SET(ch->act, ACT_WARRIOR)
	&&   getNivelPr(ch) > 7
	&&   CHANCE(80) )
	{
		do_bash( ch, victim->name );
		return;
	}

	return;
}

FRetVal newdamage(Entity * ent, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show) 
{
    bool immune;
    OBJ_DATA *corpse;
    FIGHT_DATA * fd;
    CHAR_DATA * ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;

    if ( victim->position == POS_DEAD
    ||   victim->in_room == NULL )
	return fFAIL;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1200
    && (!entEsCh(ent) || entGetCh(ent) != victim)
    &&   dt >= TYPE_HIT )
    {
	bugf( "newdamage: %d, max 1200, ent %s(%d), vict %s(%d), cuarto %d",
		dam, entidadToString(ent), entidadGetVnum(ent),
		victim->name, CHARVNUM(victim), victim->in_room->vnum );
	dam = 1200;
	if ( ch )
	{
	    OBJ_DATA *arma = get_eq_char( ch, WEAR_WIELD );
	    send_to_char("Realmente, no debieras hacer trampas.\n\r",ch);
	    if (arma != NULL)
	    {
		bugf( "Obj %s, vnum %d.", arma->name, arma->pIndexData->vnum );
		extract_obj(arma, TRUE);
	    }
	}
    }

    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)*0.5 + 35;
    if ( dam > 80)
	dam = (dam - 80)*0.5 + 80;

    if (ch)
    {
	if ( !IS_NPC(ch) )
		dam *= pcdam / 100;
	else
		dam *= mobdam / 100;
    }

    if ( ch && (victim != ch) )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	{
	    stop_fighting(ch,FALSE);
	    return fFAIL;
	}

	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
 	    {
		set_fighting( victim, ch );
 		if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
		    mp_percent_trigger( chToEnt(victim), chToEnt(ch), NULL, NULL, TRIG_KILL );
	    }
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( ch->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( ch && IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	affect_strip( ch, gsn_predinvis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n aparece en esta realidad.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */

    if ( dam > 1
    &&  !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

    if ( dam > 1
    &&   IS_AFFECTED(victim, AFF_SANCTUARY)
    &&   ch != victim )
	dam = 3 * dam / 4; /* /2 */

    if ( dam > 1
    &&   ch
    && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
     ||	(IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )) )
	dam -= dam / 4;

    immune = FALSE;

    /*
     * Check for parry, and dodge.
     */
    if ( ch
    &&   dt >= TYPE_HIT
    &&   ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return fFAIL;
	if ( check_dodge( ch, victim ) )
	    return fFAIL;
	if ( check_shield_block(ch,victim))
	    return fFAIL;
    }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):
	    dam -= dam/4; /* 3 */
	    break;
	case(IS_VULNERABLE):
	    dam += dam/4; /* 2 */
	    break;
    }

    if (show)
    	new_dam_message( ent, victim, dam, dt, immune );

    if (dam < 1)
	return fFAIL;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    if ( ch && (ch != victim) )
    {
    	fd = fdata_lookup( ch, victim->id );

    	if ( !fd )
		fd = give_fd( ch, victim );

	fd->dam += dam;
    }

    victim->hit -= dam;
    if ( !IS_NPC(victim) && victim->hit < 1 )
    {
    	if ( getNivelPr(victim) >= LEVEL_IMMORTAL
	||   (ch && ch == victim && EN_ARENA(victim))
	||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTIPO) )
		victim->hit = 1;
    }

    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n esta mortalmente herido, y morira pronto, si no es ayudado.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "Estas mortalmente herido, y moriras pronto, si no te ayudan.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n esta incapacitado y morira lentamente, si no es ayudado.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "Estas incapacitado y moriras lentamente, si no te ayudan.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n esta inconsciente, pero probablemente se recuperara.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("Estas inconsciente, pero probablemente te recuperaras.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n ha #BMUERTO#b!!", victim, 0, 0, TO_ROOM );
	send_to_char( "Has #BMUERTO#b!!\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "Eso realmente #BDOLIO#b!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "Seguramente estas #3SANGRANDO#n!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	int puntos = 0;

	if ( ch && EN_ARENA(victim) )
	{
		check_arena(ch, victim);
		return victdead;
	}

	if ( !EN_ARENA(victim) && war == FALSE )
		puntos = group_gain( ent, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s murio a manos de %s(%s) en %d",
		victim->name,
		entidadToStringNoColor(ent), entidadGetTipo(ent),
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 2/3 way back to previous level.
	     */
	    if ( victim->exp > EXP_NIVEL(victim,getNivelPr(victim)-1) + 2*EXP_NIVEL(victim,1)/3 )
	    {
		int xp_lose;

		xp_lose = (EXP_NIVEL(victim,getNivelPr(victim) - 1) + EXP_NIVEL(victim,1)/3 - victim->exp) / 2 + 50;
		xp_lose = UMAX(-1 * exp_per_level(victim,victim->pcdata->points), xp_lose);

		if ( xp_lose < 0 )
			gain_exp( victim, xp_lose );
	    }
	}

	sprintf( log_buf, "%s[%d] got toasted by %s(%s,%dxp) at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            CHARVNUM(victim),
            entidadToString(ent), entidadGetTipo(ent),
            puntos,
            victim->in_room->name, victim->in_room->vnum);

        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	if ( ch )
	{
		if ( !IS_NPC(ch) && is_clan(ch) )
		{
			if ( IS_NPC(victim) )
				get_clan_table(ch->clan)->mkills++;
			else
				get_clan_table(ch->clan)->pkills++;
		}

		if ( !IS_NPC(victim) && is_clan(victim) )
		{
			if ( IS_NPC(ch) )
				get_clan_table(victim->clan)->mdeaths++;
			else
				get_clan_table(victim->clan)->pdeaths++;
		}

		if ( ch != victim
		&&  !IS_NPC(ch)
		&&  !IS_NPC(victim)
		&&   is_clan(ch)
		&&   is_clan(victim)
		&&   is_same_clan(ch, victim) )
		{
			char buf[MIL];

			get_clan_table(victim->clan)->illpkills++;

			if ( ch->pcdata->clan_status < victim->pcdata->clan_status
			&&   ch->pcdata->max_clan_status >= victim->pcdata->clan_status )
			{
				clanlog( ch->clan, "%s subio al rango %s desde el %s matando a %s",
					ch->name,
					lookup_clan_status(victim->pcdata->clan_status),
					lookup_clan_status(ch->pcdata->clan_status),
					victim->name );
				ch->pcdata->clan_status = victim->pcdata->clan_status;
				sprintf( buf, "Ahora eres #B%s#b del clan #B%s#b!\n\r",
					lookup_clan_status(ch->pcdata->clan_status),
					get_clan_table(ch->clan)->name );
				send_to_char( buf, ch );
				sprintf( buf, "Ahora eres #B%s#b del clan #B%s#b!\n\r",
					lookup_clan_status(victim->pcdata->clan_status),
					get_clan_table(ch->clan)->name );
				send_to_char( buf, victim );
			}
			else if ( ch->pcdata->clan_status == victim->pcdata->clan_status
			&&        victim->pcdata->min_clan_status <= victim->pcdata->clan_status - 1 )
			{
				clanlog( ch->clan, "%s bajo de rango %s a %s al ser muerto por %s",
					victim->name,
					lookup_clan_status(victim->pcdata->clan_status),
					lookup_clan_status(UMAX(0, victim->pcdata->clan_status - 1)),
					ch->name );
				victim->pcdata->clan_status = UMAX(0, victim->pcdata->clan_status - 1);
				sprintf( buf, "Ahora eres #B%s#b del clan #B%s#b!\n\r",
					lookup_clan_status(victim->pcdata->clan_status),
					get_clan_table(ch->clan)->name );
				send_to_char( buf, victim );
			}

			REMOVE_BIT( ch->act, PLR_KILLER );
			REMOVE_BIT( victim->act, PLR_KILLER );
		}
	} /* clan */

	/*
	 * Death trigger
	 */
	if ( ch && IS_NPC( victim ) )
	{
		if ( HAS_TRIGGER( victim, TRIG_DEATH) )
		{
		    victim->position = POS_STANDING;
		    mp_percent_trigger( chToEnt(victim), chToEnt(ch), NULL, NULL, TRIG_DEATH );
		}

		if ( getNivelPr(victim) >= 30
		&&   victim->pIndexData->killed > 4
		&&   mem_lookup_id(victim->pIndexData->memoria, ch->id) == NULL )
		{
			MEM_DATA * mem = new_mem_data();
			mem->id	= ch->id;
			mem->reaction = MEM_HOSTILE;
			mem->when = current_time;
			mem->next = victim->pIndexData->memoria;
			victim->pIndexData->memoria = mem;
		}
	}

        /* dump the flags */
        if (ch
        &&  ch != victim
	&& !IS_NPC(ch)
        && !IS_NPC(victim)
        && (!is_clan(ch)
         || !is_clan(victim)
         || ( is_clan(ch) && is_clan(victim) && !is_same_clan(ch,victim) ) ) )
        {
            if (IS_SET(victim->act,PLR_KILLER))
                REMOVE_BIT(victim->act,PLR_KILLER);
	    if (IS_SET(victim->act,PLR_THIEF))
                REMOVE_BIT(victim->act,PLR_THIEF);
        }

        raw_kill ( ent, victim, dam_type );

        /* RT new auto commands */

	if ( ch
	&&  !IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    if ( IS_SET(ch->act, PLR_AUTOLOOT)
		&& corpse->contains) /* not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD)
		&& corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
	      	    do_get(ch, "all.gcash corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	    {
		if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse->contains)
			return victdead;  /* leave if corpse has treasure */
		else
			do_sacrifice( ch, "corpse" );
	    }
	}

	return victdead;
    }

    if ( victim == ch )
	return fOK;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return fOK;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 4)
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	{
	    do_flee( victim, "" );
	}

    tail_chain( );
    return fOK;
}

void new_dam_message( Entity * ent, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    CHAR_DATA *ch = entEsCh(ent) ? entGetCh(ent) : NULL;

    /* buf1 ... TO_ROOM */
    /* buf2 ... TO_CHAR */
    /* buf3 ... TO_VICT */

    if (victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "yerra";	vp = "yerra";		}
    else if ( dam <=   4 ) { vs = "rozas";	vp = "roza";		}
    else if ( dam <=   8 ) { vs = "rasgunas";	vp = "rasguna";		}
    else if ( dam <=  12 ) { vs = "aranas";	vp = "arana";		}
    else if ( dam <=  16 ) { vs = "lastimas";	vp = "lastima";		}
    else if ( dam <=  20 ) { vs = "hieres";	vp = "hiere";		}
    else if ( dam <=  24 ) { vs = "danas";      vp = "dana";		}
    else if ( dam <=  28 ) { vs = "diezmas";	vp = "diezma";		}
    else if ( dam <=  32 ) { vs = "rompes";	vp = "rompe";		}
    else if ( dam <=  36 ) { vs = "quiebras";	vp = "quiebra";		}
    else if ( dam <=  40 ) { vs = "#BMUTILAS#b";	vp = "#BMUTILA#b";	}
    else if ( dam <=  44 ) { vs = "#BDESTRIPAS#b";	vp = "#BDESTRIPA#b";	}
    else if ( dam <=  48 ) { vs = "#BDESMIEMBRAS#b";	vp = "#BDESMIEMBRA#b";	}
    else if ( dam <=  52 ) { vs = "#BMASACRAS#b";	vp = "#BMASACRA#b";	}
    else if ( dam <=  56 ) { vs = "#BDESGARRAS#b";	vp = "#BDESGARRA#b";	}
    else if ( dam <=  60 ) { vs = "#BDESTROZAS#b";	vp = "#BDESTROZA#b";	}
    else if ( dam <=  67 ) { vs = "*** #BDEMUELES#b ***";
			     vp = "*** #BDEMUELE#b ***";	}
    else if ( dam <=  75 ) { vs = "*** #BDEVASTAS#b ***";
			     vp = "*** #BDEVASTA#b ***";	}
    else if ( dam <=  90 ) { vs = "=== #BDESFIGURAS#b ===";
    			     vp = "=== #BDESFIGURA#b ===";	}
    else if ( dam <= 100)  { vs = "=== #BELIMINAS#b ===";
			     vp = "=== #BELIMINA#b ===";	}
    else if ( dam <= 125)  { vs = "#B#F>>> ANIQUILAS <<<#b#f";
			     vp = "#B#F>>> ANIQUILA <<<#b#f";	}
    else if ( dam <= 150)  { vs = "#B#F<<< ERRADICAS >>>#b#f";
			     vp = "#B#F<<< ERRADICA >>>#b#f";	}
    else if ( dam <= 175)  { vs = "#B#F<<<- DESTRUYES ->>>#b#f";
    			     vp = "#B#F<<<- DESTRUYE ->>>#b#f";	}
    else if ( dam <= 200)  { vs = "hace cosas #BINCREIBLES#b";
    			     vp = "hace cosas #BINCREIBLES#b";		}
    else if ( dam <= 250)  { vs = "hace cosas #BIMPRESIONANTES#b";
    			     vp = "hace cosas #BIMPRESIONANTES#b";	}
    else if ( dam <= 300)  { vs = "hace cosas #BIRRACIONALES#b";
    			     vp = "hace cosas #BIRRACIONALES#b";	}
    else if ( dam <= 400)  { vs = "hace #B#FCHUPETE#b#f";
    			     vp = "hace #B#FCHUPETE#b#f";		}
    else                   { vs = "hace cosas #B#FINDESCRIPTIBLES#b#f";
			     vp = "hace cosas #B#FINDESCRIPTIBLES#b#f";	}

    punct   = (dam <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch && ch == victim)
	{
	    sprintf( buf1, "$n se %s%c",vp,punct);
	    sprintf( buf2, "Te %s%c",vs,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s a $N%c", vp, punct );
	    sprintf( buf2, "Tu %s a $N%c", vs, punct );
	    sprintf( buf3, "$n te %s%c", vp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bugf( "new_dam_message : bad dt %d - %s", dt, entidadToString(ent) );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch && ch == victim)
	    {
		sprintf(buf1,"$n no es afectado por su %s.", attack);
		sprintf(buf2,"Por suerte, eres inmune a eso.");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N no fue afectado por %s %s de $n!", TIPO(attack), attack);
	    	sprintf(buf2,"$N no fue afectado por tu %s!",attack);
	    	sprintf(buf3,"%s %s de $n es inutil en tu contra.", TIPO(attack), attack);
	    }
	}
	else
	{
	    if (ch && ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
		sprintf( buf2, "Tu %s te %s%c",attack,vp,punct);
	    }
	    else
	    {
	    	sprintf( buf1, "%s %s de $n %s a $N%c", TIPO(attack), attack, vp, punct );
	    	sprintf( buf2, "Tu %s %s a $N%c%s",  attack, vp, punct,
	    		(ch && IS_IMMORTAL(ch)) ? parentesis(itos(dam)) : "" );
	    	sprintf( buf3, "%s %s de $n te %s%c%s", TIPO(attack), attack, vp, punct,
	    		IS_IMMORTAL(victim) ? parentesis(itos(dam)) : "" );
	    }
	}
    }

    if (ch && ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	new_act( buf1, ent, NULL, chToEnt(victim), TO_NOTVICT, POS_RESTING );
    	new_act( buf2, ent, NULL, chToEnt(victim), TO_CHAR, POS_RESTING );
    	new_act( buf3, ent, NULL, chToEnt(victim), TO_VICT, POS_RESTING );
    }

    return;
}
