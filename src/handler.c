/**************************************************************************r
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "events.h"
#include "arena.h"
#include "screen.h"
#include "lookup.h"
#include "smart.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return	);
bool es_jugador(CHAR_DATA *ch);
void poker_retirar(CHAR_DATA *ch);
void mover_lanzable( EVENT *ev );


/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_ALIGN)
    &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim))
    ||	 (IS_EVIL(ch) && IS_EVIL(victim))
    ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
	return TRUE;

    if ( IS_SET(ch->act, ACT_GROUP) && IS_SET(victim->act, ACT_GROUP) )
    	return TRUE;

    return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     
int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
	    return liq;
    }

    return -1;
}

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
 
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
        &&  !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    }
 
    return WEAPON_EXOTIC;
}


int item_lookup(const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(item_table[type].name[0])
        &&  !str_prefix(name,item_table[type].name))
            return item_table[type].type;
    }
 
    return -1;
}

char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

char *weapon_name( int weapon_typ )
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_typ == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

char *weapon_nombre( int weapon_typ )
{
    int type;
 
    for (type = 0; weapon_table[type].nombre != NULL; type++)
        if (weapon_typ == weapon_table[type].type)
            return weapon_table[type].nombre;
    return "exotica";
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* returns class number */
int class_lookup (const char *name)
{
   int class;
 
   for ( class = 0; class < MAX_CLASS; class++)
   {
        if (LOWER(name[0]) == LOWER(class_table[class].name[0])
        &&  !str_prefix( name,class_table[class].name))
            return class;
   }
 
   return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
	case(DAM_CHARM):	bit = IMM_CHARM;	break;
	case(DAM_SOUND):	bit = IMM_SOUND;	break;
	default:		return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}

/*
bool is_clan(CHAR_DATA *ch)
{
    if ((ch->clan) != 0 ) return TRUE;
     else
      return FALSE;
}
*/

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ES_INDEP(ch->clan))
	return FALSE;
    else 
	return (ch->clan == victim->clan);
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (IS_IMMORTAL(ch))
    	return 100;

    else if (sn == -1) /* shorthand for level based skills */
    {
	skill = getNivelPr(ch) * 5 / 2;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

    else if (!IS_NPC(ch))
    {
	SpellDesc spell;

	charTieneSn(&spell, ch, sn);

	if (spell.nivel < 0) // no lo tiene
	    skill = 0;
	else
	    skill = war ? 100 : ch->pcdata->learned[sn];

	if (es_clase(ch,CLASS_RANGER) && IS_EVIL(ch))
	{
		if ( skill_table[sn].spell_fun != spell_null )
			return 0;
		if ( sn == gsn_ambidiestro )
			skill /= 2;
	}
    }

    else /* mobiles */
    {
        if (skill_table[sn].spell_fun != spell_null)
	    skill = 50 + 2 * getNivelPr(ch);

	else if (sn == gsn_ambidiestro)
	    skill = 35 + 2 * getNivelPr(ch);

	else if (sn == gsn_enhanced_damage)
		skill = (int) (getNivelPr(ch) * 2.5) + 30;

	else if (sn == gsn_steal && IS_SET(ch->act, ACT_THIEF) )
	    skill = 30 + getNivelPr(ch) * 2;

	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = getNivelPr(ch) * 2 + 20;

        else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
		skill = 20 + getNivelPr(ch) * 3;

 	else if (sn == gsn_shield_block)
	    skill = 30 + (int) (2.5 * getNivelPr(ch));

	else if (sn == gsn_second_attack)
		skill = 20 + 4 * getNivelPr(ch);

	else if (sn == gsn_third_attack)
		skill = 4 * getNivelPr(ch);

	else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = 3 * getNivelPr(ch);

	else if (sn == gsn_hand_to_hand)
		skill = 30 + 2 * getNivelPr(ch);

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 30 + (int) (2.5 * getNivelPr(ch));

	else if (sn == gsn_tail && IS_SET(ch->off_flags,OFF_TAIL))
	    skill = 10 + 3 * getNivelPr(ch);

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * getNivelPr(ch);

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM)
	     ||   IS_SET(ch->act,ACT_WARRIOR)
	     ||	  IS_SET(ch->act,ACT_THIEF)))
	{
		if ( IS_PET(ch) )
			skill = 0;
		else
			skill = 20 + 3 * getNivelPr(ch);
	}

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * getNivelPr(ch);

	else if (sn == gsn_kick)
		skill = 20 + 3 * getNivelPr(ch);

	else if (sn == gsn_backstab && (IS_SET(ch->act,ACT_THIEF) || IS_SET(ch->off_flags,OFF_BACKSTAB)) )
	    skill = 20 + 3 * getNivelPr(ch);

	else if (sn == gsn_circle && (IS_SET(ch->act,ACT_THIEF) || IS_SET(ch->off_flags, OFF_CIRCLE) ) )
	    skill = 20 + 3 * getNivelPr(ch);

  	else if (sn == gsn_rescue)
	    	skill = 30 + 3 * getNivelPr(ch);

	else if (sn == gsn_blindfight)
		skill = 20 + getNivelPr(ch) * (IS_SET(ch->act, ACT_WARRIOR) ? 2 : 1);

	else if (sn == gsn_dirt && IS_SET(ch->off_flags, OFF_KICK_DIRT))
	{
		if (IS_PET(ch))
			skill = 0;
		else
			skill = 20 + 3 * getNivelPr(ch);
	}

	else if (sn == gsn_recall)
	    skill = 20 + getNivelPr(ch) * 2;

	else if (sn == gsn_fast_healing || sn == gsn_meditation)
		skill = 20 + getNivelPr(ch) * 3;

	else if (sn == gsn_snare && IS_SET(ch->act, ACT_THIEF))
	{
		if ( IS_PET(ch) )
			skill = 0;
		else
			skill = 30 + getNivelPr(ch);
	}

	else if (sn == gsn_staves
	||  sn == gsn_wands
	||  sn == gsn_scrolls )
	    skill = 20 + getNivelPr(ch) * 3;

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm)
	    skill = 30 + 3 * getNivelPr(ch);

	else 
	   skill = 0;
    }

    skill = URANGE(0, skill, 100);

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    if (IS_AFFECTED2(ch, AFF_AMNESIA))
	skill /= number_fuzzy(2);

    if (skill > 0)
	skill += ch->luck / 10;

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch, bool secondary)
{
    OBJ_DATA *wield;
    int sn;

    if (secondary)
    	wield = get_eq_char( ch, WEAR_SECONDARY );
    else
	wield = get_eq_char( ch, WEAR_WIELD );

    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * getNivelPr(ch);
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 3 * getNivelPr(ch); /* era 2 */
	else 
	    skill = 40 + 3 * getNivelPr(ch); /* era 5/2 */
    }
    
    else
    {
	if (sn == -1)
	    skill = getNivelPr(ch) * get_curr_stat(ch, STAT_WIS) / 10;
//	    skill = 3 * getNivelPr(ch);
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
	int loc,mod,stat;
	OBJ_DATA *obj;
	AFFECT_DATA *af;
	int i;

	if (IS_NPC(ch))
		return;

	if (ch->pcdata->perm_hit == 0 
	||  ch->pcdata->perm_mana == 0
	||  ch->pcdata->perm_move == 0 )
	{
		/* do a FULL reset */
		for (loc = 0; loc < MAX_WEAR; loc++)
		{
			obj = get_eq_char(ch,loc);
			if (obj == NULL)
				continue;
			if (!obj->enchanted)
				for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
				{
					mod = af->modifier;
					switch(af->location)
					{
						case APPLY_SEX:	ch->sex		-= mod;
						if (ch->sex < 0 || ch->sex >2)
							ch->sex = IS_NPC(ch) ?
							0 :
							ch->pcdata->true_sex;
						break;

						case APPLY_MANA:	ch->max_mana	-= mod;		break;
						case APPLY_HIT:		ch->max_hit	-= mod;		break;
						case APPLY_MOVE:	ch->max_move	-= mod;		break;
					}
				}

			for ( af = obj->affected; af != NULL; af = af->next )
			{
				mod = af->modifier;
				switch(af->location)
				{
					case APPLY_SEX:     ch->sex         -= mod;         break;
					case APPLY_MANA:    ch->max_mana    -= mod;         break;
					case APPLY_HIT:     ch->max_hit     -= mod;         break;
					case APPLY_MOVE:    ch->max_move    -= mod;         break;
				}
			}
		}

		/* now reset the permanent stats */
		ch->pcdata->perm_hit 	= ch->max_hit;
		ch->pcdata->perm_mana 	= ch->max_mana;
		ch->pcdata->perm_move	= ch->max_move;
		ch->pcdata->last_level	= ch->played/3600;
		if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		{
			if (ch->sex > 0 && ch->sex < 3)
				ch->pcdata->true_sex	= ch->sex;
			else
				ch->pcdata->true_sex	= 0;
		}
	}

	/* now restore the character to his/her true condition */
	for (stat = 0; stat < MAX_STATS; stat++)
		ch->mod_stat[stat] = 0;

	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		ch->pcdata->true_sex = 0;

	ch->sex		= ch->pcdata->true_sex;
	ch->max_hit	= ch->pcdata->perm_hit;
	ch->max_mana	= ch->pcdata->perm_mana;
	ch->max_move	= ch->pcdata->perm_move;

	for (i = 0; i < 4; i++)
		ch->armor[i]	= 100;

	ch->hitroll		= 0;
	ch->damroll		= 0;
	ch->saving_throw	= 0;

	/* now start adding back the effects */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
		obj = get_eq_char(ch,loc);
		if (obj == NULL)
			continue;
		for (i = 0; i < 4; i++)
			ch->armor[i] -= apply_ac( obj, loc, i );

		if (!obj->enchanted)
			for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
			{
				mod = af->modifier;
				switch(af->location)
				{
					case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
					case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
					case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
					case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
					case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
					case APPLY_SEX:		ch->sex			+= mod; break;
					case APPLY_MANA:	ch->max_mana		+= mod; break;
					case APPLY_HIT:		ch->max_hit		+= mod; break;
					case APPLY_MOVE:	ch->max_move		+= mod; break;

					case APPLY_AC:
					for (i = 0; i < 4; i ++)
						ch->armor[i] += mod; 
					break;
					case APPLY_HITROLL:	ch->hitroll		+= mod; break;
					case APPLY_DAMROLL:	ch->damroll		+= mod; break;

					case APPLY_SAVES:		ch->saving_throw += mod; break;
					case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
					case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
					case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
					case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
	    			}
			}

		for ( af = obj->affected; af != NULL; af = af->next )
		{
			mod = af->modifier;
			switch(af->location)
			{
				case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
				case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
				case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
				case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
				case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;

				case APPLY_SEX:         ch->sex                 += mod; break;
				case APPLY_MANA:        ch->max_mana            += mod; break;
				case APPLY_HIT:         ch->max_hit             += mod; break;
				case APPLY_MOVE:        ch->max_move            += mod; break;

				case APPLY_AC:
				for (i = 0; i < 4; i ++)
					ch->armor[i] += mod;
				break;

				case APPLY_HITROLL:     ch->hitroll             += mod; break;
				case APPLY_DAMROLL:     ch->damroll             += mod; break;

				case APPLY_SAVES:         ch->saving_throw += mod; break;
				case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
				case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
				case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
				case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
			}
		}
	}

	/* now add back spell effects */
	for (af = ch->affected; af != NULL; af = af->next)
	{
		mod = af->modifier;
		switch(af->location)
		{
			case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
			case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
			case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
			case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
			case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;

			case APPLY_SEX:         ch->sex                 += mod; break;
			case APPLY_MANA:        ch->max_mana            += mod; break;
			case APPLY_HIT:         ch->max_hit             += mod; break;
			case APPLY_MOVE:        ch->max_move            += mod; break;

			case APPLY_AC:
			for (i = 0; i < 4; i ++)
				ch->armor[i] += mod;
			break;

			case APPLY_HITROLL:		ch->hitroll	 += mod; break;
			case APPLY_DAMROLL:		ch->damroll	 += mod; break;

			case APPLY_SAVES:		ch->saving_throw += mod; break;
			case APPLY_SAVING_ROD:		ch->saving_throw += mod; break;
			case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
			case APPLY_SAVING_BREATH:	ch->saving_throw += mod; break;
			case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
		} 
	}

	/* make sure sex is RIGHT!!!! */
	if (ch->sex < 0 || ch->sex > 2)
		ch->sex = ch->pcdata->true_sex;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if (ch->trust)
	return ch->trust;

    if ( IS_NPC(ch) && getNivelPr(ch) >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return getNivelPr(ch);
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || getNivelPr(ch) > LEVEL_IMMORTAL)
	max = 25;

    else
    {
	max = race_table[ch->race].max_stats[stat] + 4;

	if (class_table[getClasePr(ch)].attr_prime == stat)
	    max += 2;

	if ( ch->race == RACE_HUMAN )
	    max += 1;

 	max = UMIN(max,25);
    }
  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || getNivelPr(ch) > LEVEL_IMMORTAL)
	return 25;

    max = race_table[ch->race].max_stats[stat];

    if (class_table[getClasePr(ch)].attr_prime == stat)
    {
	if (ch->race == RACE_HUMAN)
	   max += 3;
	else
	   max += 2;
    }

    return UMIN(max,25);
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && getNivelPr(ch) >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + getNivelPr(ch);
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && getNivelPr(ch) >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + getNivelPr(ch) * 25;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list;
    char *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
    	return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if (str[0] == '\0')
	return FALSE;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

bool is_exact_name(char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
	return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}

bool is_name_order( char *str, char *namelist )
{
	char name[MIL];
	char temp[MIL];

	if ( IS_NULLSTR(namelist) || IS_NULLSTR(str) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		str = one_argument( str, temp );

		if ( IS_NULLSTR(name) )
			return IS_NULLSTR(temp) ? TRUE : FALSE;

		if ( IS_NULLSTR(temp) )
			return TRUE;

		if ( str_prefix( temp, name ) )
			return FALSE;
	}
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}
           

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_AFFECTS2:
	    SET_BIT(ch->affected2_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	case TO_PARTS:
	    SET_BIT(ch->parts,paf->bitvector);
	    break;
	}
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case TO_AFFECTS2:
            REMOVE_BIT(ch->affected2_by, paf->bitvector);
            break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        case TO_PARTS:
            REMOVE_BIT(ch->parts,paf->bitvector);
            break;
        }
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:	      					break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SAVES:   ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
    case APPLY_SPELL_AFFECT:  					break;
    case APPLY_RACE:	      polymorph( ch, ch->race + mod );	break;
    }

    if ( !IS_NPC(ch)
    &&    ch->in_room != NULL
    &&    ch->position != POS_DEAD
    &&   !char_died(ch)
    &&   !event_pending(ch->events, check_strength) )
	char_event_add( ch, 1, NULL, check_strength );

    return;
}

/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
                case TO_AFFECTS2:
                    SET_BIT(ch->affected2_by, vector);
                    break;
	        case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
		case TO_PARTS:
		    SET_BIT(ch->parts,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                  	break;
                    case TO_PARTS:
                    	SET_BIT(ch->parts,vector);
                    	break;
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                    case TO_PARTS:
                        SET_BIT(ch->parts,vector);
                        break;
                }
                return;
            }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );

    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }

    return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
	ch->affected	= paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bugf( "Affect_remove: cannot find paf, char %s, vnum %d, where %d.",
	    	ch->name, CHARVNUM(ch), WHEREIS(ch) );
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch,where,vector);

    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Return true if an obj is affected by a spell.
 */
bool is_obj_affected( OBJ_DATA *obj, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );

    return;
}

/*
 * Add or enhance an affect (version alterada).
 */
void affect_join_alt( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type
	&&   paf_old->location == paf->location )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bugf( "Char_from_room: NULL ch->in_room, vnum %d, char %s",
		CHARVNUM(ch), ch->name );
	return;
    }

    ch->ultimo_cuarto = ch->in_room;

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bugf( "Char_from_room: ch %s not found, room %d.", ch->name, ch->in_room->vnum );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  /* sanity check! */
    return;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;
    bool aggress = FALSE;

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	bugf( "Char_to_room: NULL room, char %s, vnum %d.", ch->name,
		CHARVNUM(ch) );

	if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
	    char_to_room(ch,room);
	
	return;
    }

    if (pRoomIndex->people
    && !IS_SET(pRoomIndex->room_flags, ROOM_SAFE)
    && !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP) )
    	aggress = TRUE;

    if (ch->ultimo_cuarto != NULL
    && !IS_SET(pRoomIndex->room_flags, ROOM_PROTOTIPO)
    &&  IS_SET(ch->ultimo_cuarto->room_flags, ROOM_PROTOTIPO))
    	for ( obj = ch->carrying; obj; obj = obj->next_content )
    		if ( IS_SET(obj->extra_flags, ITEM_PROTOTIPO) )
    			obj_event_add(obj, 1, NULL, obj_extract_event);

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }
        
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->level == 1)
            return;

	plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1; 
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
        
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(3) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }

    if (aggress && !event_pending(pRoomIndex->events, room_aggress_event))
    	room_event_add(pRoomIndex, 1, NULL, room_aggress_event);

    return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    obj->carry_timer	 = 0;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );

    if (!IS_NPC(ch) && obj->pIndexData->max_count > 0)
    	give_limit(obj,ch);

    if (ch->in_room
    &&  IS_SET(obj->extra_flags, ITEM_PROTOTIPO)
    && !IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO))
    	obj_event_add(obj, 1, NULL, obj_extract_event);
}

/*
 * Take an obj from its character.
 */
void new_obj_from_char( OBJ_DATA *obj, bool limit )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch, vnum %d.", obj->pIndexData->vnum );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if (limit == TRUE) // borremos el limite
	extract_limit(obj);

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    obj->detected	 = 0;
    obj->carry_timer	 = 0;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );

    return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_LENTES:	return	   obj->value[type];
    case WEAR_OREJAS:	return	   obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bugf( "Equip_char: already equipped (iWear %d, chvnum %d, objvnum %d, room %d).",
		iWear, CHARVNUM(ch), obj->pIndexData->vnum,
		ch->in_room ? ch->in_room->vnum : 0 );
	return;
    }

    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act( "You are zapped by $p and drop it.", ch, objToEnt(obj), NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, objToEnt(obj), NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );
    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT )
	        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
    	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    if ( HAS_TRIGGER( obj, TRIG_WEAR ) )
    	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_WEAR );

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->location == APPLY_SPELL_AFFECT )
	    {
	        for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	        {
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
		        (lpaf->level == paf->level) &&
		        (lpaf->location == APPLY_SPELL_AFFECT))
		    {
		        affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
	        }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
	}

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	{
	    bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( HAS_TRIGGER( obj, TRIG_REMOVE ) )
    	mp_percent_trigger( objToEnt(obj), chToEnt(ch), NULL, NULL, TRIG_REMOVE );

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "Obj_from_room: NULL obj->in_room, vnum %d.", obj->pIndexData->vnum );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
FRetVal extract_obj( OBJ_DATA *obj, bool limit )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    CHAR_DATA *ch, *carried_by = NULL;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( (carried_by = obj->carried_by) != NULL )
	new_obj_from_char( obj, limit );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;

	if ( extract_obj( obj_content, limit ) == fFAIL )
		bugf( "extract_obj : FAIL al extraer obj_content de vnum %d",
			obj->pIndexData->vnum );
    }

    if (limit == TRUE)
	extract_limit(obj);

    if ( obj->item_type == ITEM_CORPSE_PC )
	for ( ch = char_list; ch; ch = ch->next )
		if ( !IS_NPC(ch) && ch->pcdata->corpse == obj )
			ch->pcdata->corpse = NULL;

    poner_ent_lista(objToEntidad(obj, TRUE));

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return fFAIL;
	}
    }

    --obj->pIndexData->count;

    free_obj(obj);

    return fOK;
}

/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj, *obj_next;
    EVENT *ev, *ev_next;
    FIGHT_DATA * fd;
    Entity * ent;

    if ( ch->in_room == NULL )
    {
	bugf( "Extract_char: NULL ch->in_room, ch %s, id %ld.",
		ch->name, ch->id );
	return;
    }

    for (ev = ch->events; ev; ev=ev_next)
    {
	ev_next=ev->nextitem;
	if (event_borrable(ev))
		event_delete(ev);
    }

    if ( !IS_NPC(ch) )
    {
	int temp;

	nuke_mem( ch, 0, FALSE ); /* borrarle la memoria al jugador */
	nuke_id( ch->id, FALSE ); /* borrarle los recuerdos del jugador al resto */

    	if ( es_jugador(ch) )
		poker_retirar(ch);
	if ( (temp = esta_inscrito(ch)) != -1 )
		arena_quit_handler(ch, temp);
    }

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )
	die_follower( ch, TRUE );

//  stop_fighting( ch, TRUE );

    ent = chToEnt(ch);

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	// no importa a lo que apunte ch->fighting...
	// stop_...
	if ( wch->fighting == ch )
	{
		if (IS_NPC(wch) && is_mob_fighting(wch))
			stop_mob_fight(wch);
		wch->fighting = NULL;
		wch->position = IS_NPC(wch) ? wch->default_pos : POS_STANDING;
		update_pos(wch);
	}
	// ...fighting

	if ( wch->reply == ch )
		wch->reply = NULL;
    	if ( is_hunting(wch) && huntvictim(wch) == ch )
		stop_hunting(wch,TRUE,ch->position == POS_DEAD ? TRUE : FALSE,FALSE);
	if ( entComparar(ent, wch->mprog_target) )
		entSetTarget(chToEnt(wch), NULL);
	if ( !IS_NPC(wch) && wch->pcdata->retando == ch )
		wch->pcdata->retando = NULL;
	while( (fd = fdata_lookup(wch, ch->id)) != NULL )
		extract_fd( wch, fd );
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	extract_obj( obj, (!IS_NPC(ch) && fPull) ? FALSE : TRUE );
    }

    char_from_room( ch );

    /* Death room is set in the clan table now */
    if ( !fPull )
    {
	if (war)
	{
		ch->hit = ch->max_hit;
		ch->mana = ch->max_mana;
		ch->move = ch->max_move;
	}

	if (ch->clan)
		char_to_room(ch, get_room_index(get_clan_table(ch->clan)->hall));
	else
		char_to_room(ch, get_room_index(ROOM_VNUM_ALTAR));

	return;
    }

    if ( IS_NPC(ch) )
    {
	--ch->pIndexData->count;
	if ( !char_died(ch) )
		poner_ent_lista(chToEntidad(ch,TRUE));
    }
    else
    if ( ch->pcdata->corpse )
	extract_obj( ch->pcdata->corpse, FALSE );

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bugf( "Extract_char: char %s, id %d, room %d, vnum %d not found.",
	    		ch->name,
	    		ch->id,
			WHEREIS(ch),
			CHARVNUM(ch) );
	    return;
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;

    free_char( ch );

    return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;

    if (!ch->in_room)
    	return NULL;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}

/*
 * Find a char in the room. Version especial.
 */
CHAR_DATA * alt_get_char_room( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if (!room)
    	return NULL;

    for ( ch = room->people; ch; ch = ch->next_in_room )
    {
	if ( !is_name( arg, ch->name ) )
	    continue;
	if ( ++count == number )
	    return ch;
    }

    return NULL;
}

/*
 * Find a char in an area.
 * (from get_char_world)
 *
 * (by Mikko Kilpikoski 09-Jun-94)
 */
CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *ach;
  int number;
  int count;

  if ( ( ach = get_char_room( ch, argument ) ) != NULL )
    return ach;

  number = number_argument( argument, arg );
  count  = 0;
  for ( ach = char_list; ach != NULL ; ach = ach->next )
    {
      if ( ach->in_room->area != ch->in_room->area
	  || !can_see( ch, ach ) || !is_name( arg, ach->name ) )
	continue;
      if ( ++count == number )
	return ach;
    }

  return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || !can_see( ch, wch ) 
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *alt_get_char_world( char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }
    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   (can_see_obj( viewer, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
			return obj;
	}
    }

    return NULL;
}

// Version especial
OBJ_DATA *alt_get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
			return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );

    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/* deduct cost from a character */
void deduct_cost(CHAR_DATA *ch, int cost)
{
    int silver = 0, gold = 0;

    silver = UMIN(ch->silver,cost); 

    if (silver < cost)
    {
	gold = ((cost - silver + 99) / 100);
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    if (ch->gold < 0)
    {
	bug("deduct costs: gold %d < 0",ch->gold);
	ch->gold = 0;
    }
    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(gold,silver));
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold/10;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/50;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	sprintf( buf, obj->short_descr, silver, gold );
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->cost		= 100 * gold + silver;
	obj->weight		= gold / 10 + silver / 50;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

char * chcolor(CHAR_DATA *ch)
{
	static char buf[2][MIL];
	static int toggle;

	toggle = (++toggle) % 2;
	buf[toggle][0] = '\0';

	if (IS_NPC(ch)
	||  ch->desc == NULL
	||  ch->desc->term == 0
	||  ch->pcdata->color == 0 )
		return ch->name;
	else
	if (ch->pcdata->color < 8)
		sprintf(buf[toggle],"#%d%s#n",ch->pcdata->color,ch->name);
	else
		sprintf(buf[toggle],"#B#%d%s#n",ch->pcdata->color - 8,ch->name);

	return buf[toggle];
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_PROTOTIPO))
    	return TRUE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  getNivelPr(ch) > 5 && !IS_IMMORTAL(ch))
	return FALSE;

    if (!IS_IMMORTAL(ch) && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;

    if ( IS_SET(victim->act, ACT_PROTOTIPO) )
    	return TRUE;

    if ( get_trust(ch) < victim->invis_level)
	return FALSE;

    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_DARK_VISION) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
    {
	AFFECT_DATA * paf = affect_find(victim->affected, gsn_invis);
	AFFECT_DATA * npaf = affect_find(ch->affected, gsn_detect_invis);

	if ( paf == NULL )
	{
		paf = affect_find(victim->affected, gsn_mass_invis);

		if ( paf == NULL )
			paf = affect_find(victim->affected, gsn_predinvis);
	}

	if ( paf != NULL
	&&  npaf != NULL )
	{
		if ( paf->level > npaf->level )
			return FALSE;
	}
	else
	if ( !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
		return FALSE;
    }

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
    {
	int chance;
	chance = get_skill(victim,gsn_sneak);
	chance += get_curr_stat(victim,STAT_DEX) * 3/2;
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= getNivelPr(ch) - getNivelPr(victim) * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
	return FALSE;

    return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_PROTOTIPO) )
    	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_SET( obj->extra_flags, ITEM_HIDDEN )
        && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN  ) )
        return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && getNivelPr(ch) >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVES:		return "saves";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_RACE:		return "race";
    case APPLY_SPELL_AFFECT:	return "none";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}

char *color_value_string( int term, int color, bool bold, bool flash, bool reverse,
				bool underline )
{
	static char buf[MIL];
	char colorbuf[20];

	if ( color != -1 )
		sprintf( colorbuf, term_table[term].fgcolor, color + term_table[term].colormod );
	else
		*colorbuf = '\0';

	sprintf( buf, "%s%s%s%s%s%s",
		term_table[term].clearattr,
		bold ? term_table[term].bold : "",
		flash ? term_table[term].blink : "",
		reverse ? term_table[term].reverse : "",
		underline ? term_table[term].underline : "",
		colorbuf );

	return buf;
}

int strlen_color( char *argument )
{
    char 	*str;
    sh_int	length;
    
    if ( argument==NULL || argument[0]=='\0' )
 	return 0; 

    length=0;
    str=argument;

    while ( *str != '\0' )
    {
    	if ( *str != '#' )
    	{
    	    str++;
    	    length++;
    	    continue;
    	}

    	switch ( *(++str) )
    	{
    	    default:    length+=2; 	break;
    	    case '-': 	
    	    case 'x': 	length++; 	break;
    	    case '<': 	 
    	    case '0':	
    	    case '1':	
    	    case '2':	
    	    case '3':	
    	    case '4':	
    	    case '5':	
    	    case '6':	
    	    case '7':	
    	    case 'B':	
    	    case 'b':	
    	    case 'F':	
    	    case 'f':	
	    case 'R':
	    case 'r':
	    case 'u':
	    case 'U':
    	    case 'n': 			break;
	    case '\0':			return length;
    	} 
    	
    	str++; 
    }
    
    return length;
}

int roll_stat( CHAR_DATA *ch, int stat )
{
    int temp,low,high;

    high = race_table[ch->race].max_stats[stat];
    low = race_table[ch->race].stats[stat];
    temp = number_range(low - 2,high - 4);

    if (class_table[getClasePr(ch)].attr_prime == stat)
    {
	if (ch->race == race_lookup("human"))
		temp += 3;
	else
		temp += 2;
    }

    return UMIN(temp,high);
}

/*
 * Returns an initial-capped string. Sin quitar caps.
 */
char *capitalizar( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = str[i];
    strcap[i] = '\0';

    if (strcap[0] != '#')
	strcap[0] = UPPER(strcap[0]);
    else
    if (i > 2 && strcap[2] != '#')
    	strcap[2] = UPPER(strcap[2]);
    else
    if (i > 4)
    	strcap[4] = UPPER(strcap[4]);

    return strcap;
}

int advatoi (const char *s)
{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi(tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{

  int newbet = 0;                /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
  char *stringptr = string;      /* a pointer we can move around */

  strcpy (string,argument);      /* make a work copy of argument */


  if (*stringptr)               /* check for an empty string */
  {

    if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
      newbet = advatoi (stringptr); /* parse and set newbet to that value */

    else
      if (*stringptr == '+') /* add ?? percent */
      {
        if (strlen (stringptr) == 1) /* only + specified, assume default */
          newbet = (currentbet * 125) / 100; /* default: add 25% */
        else
          newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
      }
      else
        {
		printf ("considering: * x \n\r");
		if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
		{
			if (strlen (stringptr) == 1) /* only x specified, assume default */
				newbet = currentbet * 2 ; /* default: twice */
			else /* user specified a number */
				newbet = currentbet * atoi (++stringptr); /* cut off the first char */
		}
        }
  }

  return newbet;        /* return the calculated bet */
}

void extract_mem( CHAR_DATA *ch, MEM_DATA *mem )
{
	MEM_DATA *temp = NULL;
	
	if ( mem == ch->memory )
	{
		temp		= ch->memory->next;
		free_mem_data( ch->memory );
		ch->memory	= temp;
	}
	else
	{
		for ( temp = ch->memory; temp; temp = temp->next )
			if ( temp->next == mem )
				break;
		
		if ( temp == NULL )
		{
			bug( "extract_mem : prev no encontrado", 0 );
			return;
		}
		
		temp->next	= mem->next;
		free_mem_data(mem);
	}
	
	return;
}

CHAR_DATA *get_char_from_id( long id )
{
	CHAR_DATA *ch;

	for ( ch = char_list; ch; ch = ch->next )
		if ( ch->id == id )
			return ch;

	return NULL;
}

CHAR_DATA *get_char_id( long id )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ch;

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING
		|| ( (ch = d->character) == NULL )
		|| (ch->in_room == NULL) )
			continue;

		if ( ch->id == id )
			return ch;
	}

	return NULL;
}

int itemtablepos( int objtype )
{
	int temp;
	
	for ( temp = 0; item_table[temp].name; ++temp )
		if ( item_table[temp].type == objtype )
			return temp;
	
	return -1;
}

int get_destino( char *argument )
{
	extern const struct taxi_dest_type taxi_dest[];
	int i;
	
	for ( i = 0; taxi_dest[i].vnum; ++i )
		if ( !str_prefix( argument, taxi_dest[i].lugar ) )
			return i;
	
	return -1;
}

bool es_vuln_mat( CHAR_DATA *victim,  int material )
{
	if ( !victim || material == MAT_INDEF )
		return FALSE;
	
	if ( (ES_VULN(victim, VULN_WOOD) && material == MAT_MADERA)
	||   (ES_VULN(victim, VULN_IRON) && material == MAT_HIERRO)
	||   (ES_VULN(victim, VULN_SILVER) && material == MAT_PLATA) )
		return TRUE;

	return FALSE;
}

bool es_res_mat( CHAR_DATA *victim, int material )
{
	if ( !victim || material == MAT_INDEF )
		return FALSE;

    	if ( (ES_RES(victim,RES_WOOD) && material == MAT_MADERA)
        ||   (ES_RES(victim,RES_IRON) && material == MAT_HIERRO)
        ||   (ES_RES(victim,RES_SILVER) && material == MAT_PLATA) )
		return TRUE;

	return FALSE;
}

/* da un affect a un cuarto */
void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= room->affected;
    room->affected	= paf_new;

    if ( !event_pending( room->events, room_update_event ) )
	room_event_add( room, PULSE_TICK, 0, room_update_event );

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
	{
		case TO_ROOM_AFF:
			SET_BIT(room->room_flags,paf->bitvector);
			break;
	}

    return;
}

void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    int where, vector;
    if ( room->affected == NULL )
    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the room if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
		case TO_ROOM_AFF:
			REMOVE_BIT(room->room_flags,paf->bitvector);
			break;
        }

    if ( paf == room->affected )
    {
        room->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = room->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
		bug( "Affect_remove_room: cannot find paf.", 0 );
		return;
        }
    }

    free_affect(paf);

    return;
}

CHAR_DATA *obj_carried_by( OBJ_DATA *obj )
{
	OBJ_DATA *in_obj;

	if ( !obj )
		return NULL;
	if ( obj->carried_by )
		return obj->carried_by;

	for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
		;

	return in_obj->carried_by;
}

void strip_mem_char( CHAR_DATA *ch, int type )
{
	MEM_DATA *mem, *mem_next;

	for ( mem = ch->memory; mem; mem = mem_next )
	{
		mem_next = mem->next;

		if ( mem->reaction == type )
			extract_mem( ch, mem );
	}
}

CHAR_DATA *get_random_char_world( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *temp;

	for ( temp = char_list; temp; temp = temp->next )
		if ((temp != ch)
		&& (temp != victim)
		&& (number_range(1,10) == number_range(1,10)) )
			return temp;

	return victim;
}

bool mob_can_move( CHAR_DATA *ch, EXIT_DATA * salida )
{
	if (   salida != NULL
	    && salida->u1.to_room != NULL
	    && !IS_SET(salida->exit_info, EX_CLOSED)
	    && !IS_SET(salida->u1.to_room->room_flags, ROOM_NO_MOB)
	    && (!IS_SET(ch->act, ACT_STAY_AREA)
		|| salida->u1.to_room->area == ch->in_room->area ) 
	    && (!IS_SET(ch->act, ACT_OUTDOORS)
		|| !IS_SET(salida->u1.to_room->room_flags,ROOM_INDOORS))
	    && (!IS_SET(ch->act, ACT_INDOORS)
		|| IS_SET(salida->u1.to_room->room_flags,ROOM_INDOORS)) )
		return TRUE;

	return FALSE;
}

bool pc_in_room( ROOM_INDEX_DATA *room, CHAR_DATA * tch )
{
	CHAR_DATA *ch;

	for ( ch = room->people; ch; ch = ch->next_in_room )
		if ( !IS_NPC(ch) &&
			( !tch || ENTRE_I(getNivelPr(tch) - 5, getNivelPr(ch), getNivelPr(tch) + 5) ) )
			return TRUE;

	return FALSE;
}

int numfriends( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	CHAR_DATA *gch;
	int cnt = 0;

	for ( gch = room->people; gch; gch = gch->next_in_room )
		if ( IS_NPC(gch) && is_friend(ch, gch) )
			cnt++;

	return cnt;
}

int mob_best_door( CHAR_DATA *ch )
{
	int cnt = 0, friends, max_friends, door;
static	EXIT_DATA * salidas[MAX_DIR];
	EXIT_DATA * best_exit, * salida;

	if ( ch->in_room == NULL )
	{
		bugf( "Mob_best_door : ch %s en room NULL, vnum %d",
			ch->name, CHARVNUM(ch) );
		return -1;
	}

	for ( salida = ch->in_room->exits; salida; salida = salida->next )
	{
		if ( salida->u1.to_room == NULL )
			continue;

		if ( IS_SET(salida->u1.to_room->room_flags, ROOM_SAFE) )
			return salida->direccion;

		if ( mob_can_move(ch, salida) && !pc_in_room(salida->u1.to_room, ch) )
			salidas[cnt++] = salida;
	}

	salidas[cnt]	= NULL;
	best_exit	= NULL;
	max_friends	= numfriends( ch, ch->in_room );

	for ( door = 0; salidas[door] != NULL; door++ )
	{
		friends = numfriends(ch, salidas[door]->u1.to_room);

		if ( friends > max_friends )
		{
			max_friends	= friends;
			best_exit	= salidas[door];
		}
	}

	if (best_exit == NULL && max_friends == 0 && salidas[0] != NULL)
		best_exit = salidas[number_range(0, cnt-1)];

	return (best_exit ? best_exit->direccion : -1);
}

CHAR_DATA *get_char_id_room( CHAR_DATA *ch, long id )
{
	CHAR_DATA *gch;

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
		if ( gch->id == id )
			return gch;

	return NULL;
}

void give_mem_when( CHAR_DATA *ch, time_t when, int reaction, long id )
{
	MEM_DATA *mem;

	mem		= new_mem_data();
	mem->id		= id;
	mem->reaction	= reaction;
	mem->when	= when;
	mem->next	= ch->memory;
	ch->memory	= mem;

	if ( IS_NPC(ch) && !event_pending(ch->events, mem_check_event) )
		char_event_add( ch, PULSE_PER_SECOND, 0, mem_check_event );
}

char *tformat( time_t when )
{
	static char buf[MIL];

	strftime( buf, MAX_INPUT_LENGTH,
	"%H:%M:%S, %A %d de %B de %Y", localtime(&when) );

	return buf;
}

void set_char( CHAR_DATA *ch, int level )
{
	int stat;

	ch->race				= RACE_HUMAN;
	ch->act					= PLR_NOSUMMON | PLR_AUTOASSIST
    						| PLR_AUTOEXIT | PLR_AUTOSAC | PLR_AUTOLOOT
    						| PLR_AUTOGOLD | PLR_AUTOSPLIT;
	ch->comm				= COMM_COMBINE | COMM_PROMPT |
						  COMM_SHOW_AFFECTS | COMM_CONDICION;
	ch->luck				= 0;
	ch->clan				= 0;
	ch->sex					= SEX_MALE;

	if (level > 0)
	{
		setClasePr(ch, CLASS_WARRIOR, level);
		ch->hit		= ch->max_hit		= 20 * level;
		ch->mana	= ch->max_mana		= 100 + 15 * level;
		ch->move	= ch->max_move		= 100 + 5 * level;
		ch->exp					= 1000 * level;
	}
	else
	{
		ch->hit		= ch->max_hit = 20;
		ch->mana	= ch->max_mana = 100;
		ch->move	= ch->max_move = 100;
		ch->exp		= 1000;
	}
	ch->trust				= 0;
	ch->gold	= ch->silver		= 0;
	ch->train	= ch->practice		= 0;
	ch->alignment				= 0;
	for (stat = 0; stat < MAX_STATS; stat++)
		ch->perm_stat[stat]		= 13;
	ch->pcdata->points			= 40;
	ch->pcdata->prompt			= str_dup(PROMPT_ALL);
	ch->pcdata->prefix			= str_dup( "" );
	ch->pcdata->lines			= PAGELEN;
	ch->pcdata->confirm_delete		= FALSE;
	ch->pcdata->pwd				= str_dup( "" );
	ch->pcdata->bamfin			= str_dup( "" );
	ch->pcdata->bamfout			= str_dup( "" );
	ch->pcdata->title			= str_dup( "" );
	ch->pcdata->who_text			= str_dup( "" );
	ch->pcdata->mensaje			= str_dup( "" );
	ch->pcdata->condition[COND_THIRST]	= 48;
	ch->pcdata->condition[COND_FULL]	= 48;
	ch->pcdata->condition[COND_HUNGER]	= 48;
	ch->pcdata->security			= 0;
	ch->pcdata->incarnations		= 1;
	ch->pcdata->language[COMMON]		= 100;
	ch->pcdata->true_sex			= SEX_MALE;
	ch->pcdata->true_align			= -1;
	ch->true_race				= ch->race;
	ch->pcdata->victorias			= 0;
	ch->pcdata->derrotas			= 0;
	ch->pcdata->quaff			= 0;
	ch->pcdata->min_clan_status		= -1;
	ch->pcdata->max_clan_status		= -1;
}

bool is_mob_grouped( CHAR_DATA *ch )
{
	CHAR_DATA *gch;

	if ( ch->leader != ch && ch->leader != NULL )
		return TRUE;

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
		if ( ch != gch && gch->leader == ch )
			return TRUE;

	return FALSE;
}

int obj_whereis_vnum( OBJ_DATA *obj )
{
	CHAR_DATA *ch;

	if ( obj->in_room )
		return obj->in_room->vnum;

	if ( (ch = obj_carried_by(obj)) == NULL )
		return -1;
	else
		return ch->in_room->vnum;
}

ROOM_INDEX_DATA * obj_whereis( OBJ_DATA *obj )
{
	OBJ_DATA *temp;

	if ( obj->in_room )
		return obj->in_room;

	for ( temp = obj; temp->in_obj; temp = temp->in_obj )
		;

	if ( temp->carried_by )
		return temp->carried_by->in_room;
	else
	if ( temp->in_room )
		return temp->in_room;
	else
	{
		bugf( "obj_whereis : obj %d en cuarto NULL", obj->pIndexData->vnum );
		return NULL;
	}
}

OBJ_DATA * get_obj_type_list( int type, OBJ_DATA *list )
{
	OBJ_DATA * obj;

	for ( obj = list; obj; obj = obj->next_content )
		if ( obj->item_type == type )
			return obj;

	return NULL;
}

char *itos( int temp )
{
	static char buf[64];

	sprintf( buf, "%d", temp );

	return buf;
}

#if !defined(USAR_MACROS)
bool can_recall( CHAR_DATA *ch )
{
	if ( IS_AFFECTED(ch, AFF_CURSE) )
		return FALSE;

	if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
		return FALSE;

	return TRUE;
}
#endif

MEM_DATA * mem_lookup( MEM_DATA * inicio, int tipo )
{
	MEM_DATA * mem;

	for ( mem = inicio; mem; mem = mem->next )
		if ( mem->reaction == tipo )
			return mem;

	return NULL;
}

void exit_set( ROOM_INDEX_DATA *room, EXIT_DATA * pExit )
{
	EXIT_DATA * prev = NULL, * temp;

	for ( temp = room->exits; temp; temp = temp->next )
	{
		if ( temp->direccion > pExit->direccion )
			break;
		prev = temp;
	}

	if ( !prev )
	{
		pExit->next	= room->exits;
		room->exits	= pExit;
	}
	else
	{
		pExit->next	= prev->next;
		prev->next	= pExit;
	}
}

bool es_kill_steal( CHAR_DATA *ch, CHAR_DATA *victim, bool msg )
{
	if ( IS_NPC(ch) && !IS_PET(ch) )
		return FALSE;

	if ( victim->fighting != NULL
	&&  !EN_ARENA(victim)
	&&   war == FALSE
	&&  !is_same_group(ch,victim->fighting))
	{
		if (msg)
			send_to_char( "Kill stealing no esta permitido.\n\r", ch );
		return TRUE;
	}

	return FALSE;
}

int flagtablepos( const struct flag_type * flag_table, int flag )
{
	int i;

	for ( i = 0; flag_table[i].name; i++ )
		if ( flag_table[i].bit == flag )
			return i;

	return -1;
}

void nuke_id( long id, bool fAll )
{
	CHAR_DATA *ch;

	for ( ch = char_list; ch; ch = ch->next )
		if ( ch->memory )
			nuke_mem( ch, id, fAll );
}

void nuke_mem( CHAR_DATA *ch, long id, bool fAll )
{
	MEM_DATA *mem, *mem_next;
	int posic;

	for ( mem = ch->memory; mem; mem = mem_next )
	{
		mem_next = mem->next;

		if ( id && mem->id != id )
			continue;

		if (fAll == FALSE)
		{
			posic = flagtablepos(mem_types, mem->reaction);

			if ( posic == -1 )
			{
				bugf( "nuke_mem : jugador %s con reaction %d", ch->name, mem->reaction );
				continue;
			}

			if ( mem_types[posic].settable == FALSE )
				continue;
		}

		extract_mem( ch, mem );
	}
}

char * parentesis( const char * frase )
{
	static char buf[2][MIL];
	static int toggle;

	toggle = (++toggle) % 2;
	buf[toggle][0] = '\0';

	sprintf( buf[toggle], "(%s)", frase );

	return buf[toggle];
}

void remove_affects( CHAR_DATA *ch, int type, int flags )
{
	AFFECT_DATA *paf, *paf_next;

	for ( paf = ch->affected; paf; paf = paf_next )
	{
		paf_next = paf->next;

		if ( paf->type == type && (paf->bitvector & flags) )
			affect_remove( ch, paf );
	}
}

void polymorph( CHAR_DATA *ch, int raza )
{
	int affflags, aff2flags, formflags, partflags, offflags,
	    immflags, resflags, vulnflags;

	if ( raza == ch->race )
	{
		bugf( "polymorph : raza == race (%d)", ch->name, raza );
		return;
	}

	/* sacamos los que la raza destino ya tiene */
	remove_affects( ch, TO_AFFECTS, race_table[raza].aff );
	remove_affects( ch, TO_AFFECTS2, race_table[raza].aff2 );
	remove_affects( ch, TO_IMMUNE, race_table[raza].imm );
	remove_affects( ch, TO_RESIST, race_table[raza].res );
	remove_affects( ch, TO_VULN, race_table[raza].vuln );
	remove_affects( ch, TO_PARTS, race_table[raza].parts );

	affflags	= ~race_table[ch->race].aff & ch->affected_by;
	aff2flags	= ~race_table[ch->race].aff2 & ch->affected2_by;
	formflags	= ~race_table[ch->race].form & ch->form;
	partflags	= ~race_table[ch->race].parts & ch->parts;
	offflags	= ~race_table[ch->race].off & ch->off_flags;
	immflags	= ~race_table[ch->race].imm & ch->imm_flags;
	resflags	= ~race_table[ch->race].res & ch->res_flags;
	vulnflags	= ~race_table[ch->race].vuln & ch->vuln_flags;

	ch->race		= raza;
	ch->form		= race_table[ch->race].form | formflags;
	ch->parts		= race_table[ch->race].parts | partflags;
	ch->affected_by		= race_table[ch->race].aff | affflags;
	ch->affected2_by	= race_table[ch->race].aff2 | aff2flags;
	ch->off_flags		= race_table[ch->race].off | offflags;
	ch->imm_flags		= race_table[ch->race].imm | immflags;
	ch->res_flags		= race_table[ch->race].res | resflags;
	ch->vuln_flags		= race_table[ch->race].vuln | vulnflags;
	ch->size		= race_table[ch->race].size;
	ch->dam_type		= race_table[ch->race].dam_type;
}

bool emptystring( const char * str )
{
	int i = 0;

	for ( ; str[i]; i++ )
		if ( str[i] != ' ' )
			return FALSE;

	return TRUE;
}

FIGHT_DATA * give_fd( CHAR_DATA *ch, CHAR_DATA *victim )
{
	FIGHT_DATA * fd;

	fd		= new_fdata ();
	fd->atacante	= victim;
	fd->id		= victim->id;
	fd->dam		= 0;
	fd->when	= current_time;
	fd->next	= ch->fdata;
	ch->fdata	= fd;

	return fd;
}

void extract_fd( CHAR_DATA *ch, FIGHT_DATA * fd )
{
	FIGHT_DATA * temp;

	if ( ch->fdata == fd )
		ch->fdata = ch->fdata->next;
	else
	{
		for ( temp = ch->fdata; temp; temp = temp->next )
			if ( temp->next == fd )
				break;

		if ( !fd )
		{
			bugf( "extract_fd : fd no encontrado en lista, jugador %s", ch->name );
			return;
		}

		temp->next = fd->next;
	}

	free_fdata( fd );
}

int get_points( int race, int class )
{
	int x;

	x = group_lookup(class_table[class].default_group);

	if ( x == -1 )
	{
		bugf( "get_points : grupo %s inexistente, raza %d, clase %d",
			class_table[class].default_group,
			race, class );
		return -1;
	}

	return group_table[x].rating[class] + race_table[race].points;
}

MEM_DATA * mem_lookup_id( MEM_DATA * mem, long id )
{
	for ( ; mem; mem = mem->next )
		if ( mem->id == id )
			return mem;

	return NULL;
}

bool is_in_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *tch;

	for ( tch = ch->in_room->people; tch; tch = tch->next_in_room )
		if ( tch == ch )
			return TRUE;

	return FALSE;
}

void abrir_salidas( AREA_DATA * area )
{
	int i, cnt = 0;
	ROOM_INDEX_DATA * room;
	EXIT_DATA * pExit;

	for ( i = 0; i < MAX_KEY_HASH; i++ )
		for ( room = room_index_hash[i]; room; room = room->next )
			for ( pExit = room->exits; pExit; pExit = pExit->next )
				if ( room->area == area
				&&  !ENTRE_I(area->min_vnum, pExit->u1.vnum, area->max_vnum) )
				{
					REMOVE_BIT(pExit->exit_info, EX_CLOSED );
					cnt++;
				}

	if ( cnt == 0 )
		flog( "abrir_salidas : Area %s : area cerrada", area->name );
}

void cerrar_salidas( AREA_DATA * area )
{
	int i, cnt = 0;
	ROOM_INDEX_DATA * room;
	EXIT_DATA * pExit;

	for ( i = 0; i < MAX_KEY_HASH; i++ )
		for ( room = room_index_hash[i]; room; room = room->next )
			for ( pExit = room->exits; pExit; pExit = pExit->next )
				if ( room->area == area
				&&  !ENTRE_I(area->min_vnum, pExit->u1.vnum, area->max_vnum) )
				{
					SET_BIT(pExit->exit_info, EX_CLOSED );
					SET_BIT(pExit->exit_info, EX_LOCKED );
					SET_BIT(pExit->exit_info, EX_NOPASS );
					cnt++;
				}

	if ( cnt == 0 )
		flog( "cerrar_salidas : Area %s : area cerrada", area->name );
}

bool is_room_affected( ROOM_INDEX_DATA *room, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = room->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

LIMIT_DATA * obj_id_lookup( sh_int id, OBJ_INDEX_DATA * obj )
{
	LIMIT_DATA * temp;

	for ( temp = obj->limit; temp; temp = temp->next )
		if ( temp->id == id )
			return temp;

	return NULL;
}

sh_int get_obj_id( OBJ_DATA * obj )
{
	sh_int id;

	if ( obj->pIndexData->max_count == 0 )
		return 0;

	for ( id = 0; id < 255; ++id )
		if ( obj_id_lookup(id, obj->pIndexData) == NULL )
			return id;

	bugf( "get_obj_id : no hay ids disponibles para obj %d",
		obj->pIndexData->vnum );
	return 0;
}

LIMIT_DATA * limit_lookup( OBJ_DATA *obj )
{
	LIMIT_DATA * temp;

	for ( temp = obj->pIndexData->limit; temp; temp = temp->next )
		if ( temp->id == obj->id )
			return temp;

	return NULL;
}

void extract_limit( OBJ_DATA *obj )
{
	LIMIT_DATA * limit = limit_lookup(obj);

	if ( limit == NULL )
		return;

	SET_BIT(obj->pIndexData->area->area_flags, AREA_CHANGED);

	if ( limit == obj->pIndexData->limit )
		obj->pIndexData->limit = obj->pIndexData->limit->next;
	else
	{
		LIMIT_DATA * temp;

		for ( temp = obj->pIndexData->limit; temp; temp = temp->next )
			if ( temp->next == limit )
				break;

		if ( temp == NULL )
		{
			bugf( "extract_limit : prev no encontrado" );
			return;
		}

		temp->next = limit->next;
	}

	free_limit(limit);
}

void give_limit( OBJ_DATA *obj, CHAR_DATA *ch )
{
	LIMIT_DATA * limit;

	if ( (limit = limit_lookup(obj)) != NULL )
	{
		if ( ch->id != limit->carrier_id )
			limit->carrier_id = ch->id;
		return;
	}

	limit			= new_limit();
	limit->carrier_id	= ch->id;
	limit->id		= obj->id;
	limit->next		= obj->pIndexData->limit;
	obj->pIndexData->limit	= limit;

	SET_BIT(obj->pIndexData->area->area_flags, AREA_CHANGED);
}

int get_count( OBJ_INDEX_DATA * obj )
{
	LIMIT_DATA * limit;
	int cnt = 0;

	for ( limit = obj->limit; limit; limit = limit->next )
		cnt++;

	return cnt;
}

char * affect_list( AFFECT_DATA * paf )
{
	static char buf[MSL];
	char buf2[MIL];
	AFFECT_DATA * af;

	buf[0] = '\0';

	for ( af = paf; af; af = af->next )
	{
		sprintf( buf2, "%d %s ",
			af->modifier,
			flag_string(apply_flags, af->location) );
		strcat( buf, buf2 );
		if ( af->bitvector != 0 )
			strcat( buf, "BITVECTOR " );
	}

	return buf;
}

void check_strength( EVENT * ev )
{
    OBJ_DATA *wield;
    CHAR_DATA * ch = ev->item.ch;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
      || ( wield = get_eq_char( ch, WEAR_SECONDARY ) ) != NULL )
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth = 0;

	if ( depth == 0 )
	{
	    depth++;
	    act( "Botas $p.", ch, objToEnt(wield), NULL, TO_CHAR );
	    act( "$n bota $p.", ch, objToEnt(wield), NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }
}

MEM_DATA * mem_lookup_react_id( MEM_DATA * lista, int react, int id )
{
	MEM_DATA * temp;

	for ( temp = lista; temp; temp = temp->next )
		if ( temp->reaction == react
		&&   temp->id == id )
			return temp;

	return NULL;
}

bool es_spell_clase(int sn, int clase)
{
	int bit;

	switch(clase)
	{
		case CLASS_CLERIC:
		bit = SKILL_CLERIC;
		break;

		case CLASS_MAGE:
		bit = SKILL_MAGE;
		break;

		case CLASS_WARRIOR:
		bit = SKILL_WARRIOR;
		break;

		case CLASS_THIEF:
		bit = SKILL_THIEF;
		break;

		case CLASS_PSI:
		bit = SKILL_PSI;
		break;

		case CLASS_RANGER:
		bit = SKILL_RANGER;
		break;

		default:
		bugf( "es_spell_clase : clase %d invalida",
			clase );
		return FALSE;
	}

	return ( IS_SET(skill_table[sn].flags, bit) );
}

void listar_razas( DESCRIPTOR_DATA * d, CHAR_DATA * ch )
{
	int race;

	for ( race = 1; race_table[race].name != NULL; race++ )
	{
		if (!race_table[race].pc_race
		||  !race_table[race].seleccionable
		||  (IS_GOOD(ch)	&& IS_SET(race_table[race].noalign, NOALIGN_GOOD))
		||  (IS_NEUTRAL(ch)	&& IS_SET(race_table[race].noalign, NOALIGN_NEUTRAL))
		||  (IS_EVIL(ch)	&& IS_SET(race_table[race].noalign, NOALIGN_EVIL)) )
			continue;
		
		if ( !race_table[race].remort_race || IS_SET(ch->act,PLR_REMORT) )
		{
			write_to_buffer(d,race_table[race].name,0);
			write_to_buffer(d," ",1);
		}
	}
	write_to_buffer(d, "\n\r", 0);
}

OBJ_DATA * get_obj_vnum_list( OBJ_DATA * list, int vnum )
{
	for ( ; list; list = list->next_content )
		if ( list->pIndexData->vnum == vnum )
			return list;

	return NULL;
}

#if !defined(USAR_MACROS)
sh_int getNivelPr(CHAR_DATA *ch)
{
	if ( ch == NULL
	||   ch->level_data == NULL )
		return 0;

	return ch->level_data->nivel; // el primero de la lista
}

sh_int getClasePr(CHAR_DATA *ch)
{
	if ( ch == NULL
	||   ch->level_data == NULL )
		return 0;

	return ch->level_data->clase;
}
#endif

void setNivelPr(CHAR_DATA *ch, int nivel)
{
	if ( ch == NULL )
		return;

	if ( ch->level_data == NULL )
		ch->level_data = new_level_data();

	ch->level_data->nivel = (sh_int) nivel;
}

void setClasePr(CHAR_DATA *ch, int clase, int nivel)
{
	if ( ch == NULL )
		return;

	if ( ch->level_data == NULL )
		ch->level_data = new_level_data();

	ch->level_data->clase = (sh_int) clase;

	if (nivel >= 0)
		ch->level_data->nivel = (sh_int) nivel;
}

void addClase(CHAR_DATA *ch, int clase, int nivel)
{
	LEVEL_DATA * lev;

	if ( ch == NULL )
		return;

	if ( es_clase(ch, clase) )
	{
		bugf( "addClase : char %s ya tiene clase %s",
			ch->name, class_table[clase].name );
		return;
	}

	lev		= new_level_data();
	lev->clase	= (sh_int) clase;
	lev->nivel	= (sh_int) nivel;

	if (ch->level_data == NULL)
		ch->level_data = lev;
	else
	{
		LEVEL_DATA * t;

		for ( t = ch->level_data; t->next; t = t->next )
			;

		t->next = lev;
	}
}

int get_vnum_mob_name_area( char * name, AREA_DATA * pArea )
{
	int hash;
	MOB_INDEX_DATA * mob;

	for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
			if ( mob->area == pArea
			&&  !str_prefix(name, mob->player_name) )
				return mob->vnum;

	return 0;
}

int get_vnum_obj_name_area( char * name, AREA_DATA * pArea )
{
	int hash;
	OBJ_INDEX_DATA * obj;

	for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
			if ( obj->area == pArea
			&&  !str_prefix(name, obj->name) )
				return obj->vnum;

	return 0;
}

int num_clases( CHAR_DATA * ch )
{
	LEVEL_DATA * niv;
	int cnt = 0;

	for ( niv = ch->level_data; niv; niv = niv->next )
		cnt++;

	return cnt;
}

bool puede_ser_clase( int claseinic, int raza, int clase )
{
	if ( race_table[raza].noclase[clase] == TRUE )
		return FALSE;
	if ( class_table[claseinic].noclase[clase] == TRUE )
		return FALSE;
	return TRUE;
}

bool odia_alguien_cuarto( CHAR_DATA *ch )
{
	CHAR_DATA * vict = ch->in_room->people;

	for ( ; vict; vict = vict->next_in_room )
		if ( vict != ch && HATES(ch, vict) )
			return TRUE;

	return FALSE;
}

bool es_clase( CHAR_DATA *ch, int clase )
{
	LEVEL_DATA * lev = ch->level_data;

	for ( ; lev; lev = lev->next )
		if ( lev->clase == clase )
			return TRUE;

	return FALSE;
}

char * colorstrip( char * str )
{
	int i = 0, l = strlen(str), cnt = 0;
static	char buf[MSL];

	for ( ; i < l; i++ )
	{
		if ( str[i] == '#' )
			switch(str[++i])
			{
				default:
				break;

				case '\0':
				return buf;

				case '-':
				case 'x':
				case '<':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case 'B': case 'b':
				case 'F': case 'f':
				case 'R': case 'r':
				case 'U': case 'u':
				case 'n':
				i++;
				break;
			} // switch

		buf[cnt++] = str[i];
	}

	buf[cnt] = '\0';

	return buf;
}

int mat_lookup(char *material)
{
	if (!str_cmp(material,"unknown")
	||  !str_cmp(material,"none")
	||  !str_cmp(material,"nada")
	||  !str_cmp(material,"")
	||  !str_cmp(material,"desconocido")
	||  !str_cmp(material,"oldstyle")
	||  !str_cmp(material,"indef"))
		return MAT_INDEF;
	if (!str_cmp(material,"gold")
	||  !str_cmp(material,"oro"))
		return MAT_ORO;
	if (!str_cmp(material,"silver")
	||  !str_cmp(material,"plata"))
		return MAT_PLATA;
	if (!str_cmp(material,"wood")
	||  !str_cmp(material,"wooden")
	||  !str_cmp(material,"madera"))
		return MAT_MADERA;
	if (!str_cmp(material,"metal")
	||  !str_cmp(material,"morrocotudo"))
		return MAT_METAL;
	if (!str_cmp(material,"iron")
	||  !str_cmp(material,"fierro")
	||  !str_cmp(material,"hierro"))
		return MAT_HIERRO;
	if (!str_cmp(material,"steel")
	||  !str_cmp(material,"acero"))
		return MAT_ACERO;
	if (!str_cmp(material,"mithril"))
		return MAT_MITHRIL;
	if (!str_cmp(material,"glass")
	||  !str_cmp(material,"vidrio"))
		return MAT_VIDRIO;
	if (!str_cmp(material,"crystal")
	||  !str_cmp(material,"cristal"))
		return MAT_CRISTAL;
	if (!str_cmp(material,"paper")
	||  !str_cmp(material,"parchment")
	||  !str_cmp(material,"papiro")
	||  !str_cmp(material,"papel")
	||  !str_cmp(material,"vellum"))
		return MAT_PAPEL;
	if (!str_cmp(material,"food")
	||  !str_cmp(material,"comida"))
		return MAT_COMIDA;
	if (!str_cmp(material,"marmol")
	||  !str_cmp(material,"marble"))
		return MAT_MARMOL;
	if (!str_cmp(material,"marfil")
	||  !str_cmp(material,"ivory"))
		return MAT_MARFIL;
	if (!str_cmp(material,"carne")
	||  !str_cmp(material,"flesh")
	||  !str_cmp(material,"meat"))
		return MAT_CARNE;
	if (!str_cmp(material,"cuero")
	||  !str_cmp(material,"leather"))
		return MAT_CUERO;
	if (!str_cmp(material,"bronze")
	||  !str_cmp(material,"bronce")
	||  !str_cmp(material,"brass"))
		return MAT_BRONCE;
	if (!str_cmp(material,"ice")
	||  !str_cmp(material,"hielo"))
		return MAT_HIELO;
	if (!str_cmp(material,"copper")
	||  !str_cmp(material,"cobre"))
		return MAT_COBRE;
	if (!str_cmp(material,"piedra")
	||  !str_cmp(material,"stone"))
		return MAT_PIEDRA;
	if (!str_cmp(material,"hueso")
	||  !str_cmp(material,"oseo")
	||  !str_cmp(material,"bone"))
		return MAT_HUESO;
	if (!str_cmp(material,"cloth")
	||  !str_cmp(material,"hilo")
	||  !str_cmp(material,"tela")
	||  !str_cmp(material,"genero"))
		return MAT_TELA;
	if (!str_cmp(material,"plastico"))
		return MAT_PLASTICO;
	if (!str_cmp(material,"seda"))
		return MAT_SEDA;
	if (!str_cmp(material,"lana"))
		return MAT_LANA;
	if (!str_cmp(material,"terciopelo")
	||  !str_cmp(material,"velvet"))
		return MAT_TERCIOPELO;
	if (!str_cmp(material,"agua")
	||  !str_cmp(material,"water"))
		return MAT_AGUA;
	if (!str_cmp(material,"liquid")
	||  !str_cmp(material,"liquido"))
		return MAT_LIQUIDO;
	if (!str_cmp(material,"cemento"))
		return MAT_CEMENTO;
	if (!str_cmp(material,"diamond")
	||  !str_cmp(material,"diamante"))
		return MAT_DIAMANTE;
	if (!str_cmp(material,"adamantium")
	||  !str_cmp(material,"adamantite")
	||  !str_cmp(material,"adamantio"))
		return MAT_ADAMANTIO;
	if (!str_cmp(material,"aluminium")
	||  !str_cmp(material,"aluminum")
	||  !str_cmp(material,"lata")
	||  !str_cmp(material,"aluminio"))
		return MAT_ALUMINIO;
	if (!str_cmp(material,"ebony")
	||  !str_cmp(material,"ebano"))
		return MAT_EBANO;
	if (!str_cmp(material,"goma")
	||  !str_cmp(material,"caucho")
	||  !str_cmp(material,"rubber")
	||  !str_cmp(material,"hule"))
		return MAT_GOMA;
	if (!str_cmp(material,"titanium")
	||  !str_cmp(material,"titanio"))
		return MAT_TITANIO;
	if (!str_cmp(material,"clay")
	||  !str_cmp(material,"greda"))
		return MAT_GREDA;
	if (!str_cmp(material,"quartz")
	||  !str_cmp(material,"cuarzo"))
		return MAT_CUARZO;
	if (!str_cmp(material,"pearl")
	||  !str_cmp(material,"perla"))
		return MAT_PERLA;
	if (!str_cmp(material,"gema")
	||  !str_cmp(material,"gem"))
		return MAT_GEMA;
	if (!str_cmp(material,"platinum")
	||  !str_cmp(material,"platino"))
		return MAT_PLATINO;
	if (!str_cmp(material,"uranio")
	||  !str_cmp(material,"uranium"))
		return MAT_URANIO;
	if (!str_cmp(material,"plutonio")
	||  !str_cmp(material,"plutonium"))
		return MAT_PLUTONIO;
	if (!str_cmp(material,"tierra")
	||  !str_cmp(material,"arena")
	||  !str_cmp(material,"dirt"))
		return MAT_TIERRA;
	if (!str_cmp(material,"tabaco"))
		return MAT_TABACO;
	if (!str_cmp(material,"bread")
	||  !str_cmp(material,"pan"))
		return MAT_PAN;

	return -1;
}

void set_mob_raza( MOB_INDEX_DATA * mob, int race )
{
	mob->race	= race;
	mob->affected_by = race_table[mob->race].aff;
	mob->affected2_by = race_table[mob->race].aff2;
	mob->imm_flags	= race_table[mob->race].imm;
	mob->res_flags	= race_table[mob->race].res;
	mob->vuln_flags	= race_table[mob->race].vuln;
	mob->form	= race_table[mob->race].form;
	mob->parts	= race_table[mob->race].parts;
}

void war_enter(DESCRIPTOR_DATA *d, CHAR_DATA *ch)
{
	char player_name[MIL];
	char title[MSL];
	int raza	= ch->race;
	int sex		= ch->sex;
	int nivel	= getNivelPr(ch);
	long id		= ch->id;
	int clan	= ch->clan;
	int class	= getClasePr(ch);
	int clanst	= ch->pcdata->clan_status;
	int i;

	strcpy( player_name, ch->name );
	strcpy( title, ch->pcdata->title );

	free_char( ch );

	ch		= new_char ();
	ch->pcdata	= new_pcdata();
	d->character	= ch;
	ch->desc	= d;

	set_char( ch, WAR_LEVEL );

	ch->name		= str_dup( player_name );
	ch->pcdata->title	= str_dup( title );
	ch->id			= id;
	ch->race		= raza;
	ch->true_race		= raza;
	ch->size		= race_table[raza].size;
	ch->dam_type		= race_table[raza].dam_type;
	ch->sex			= sex;
	ch->pcdata->true_sex	= sex;
	ch->clan		= clan;
	ch->pcdata->clan_status	= clanst;
	setClasePr(ch, class, WAR_LEVEL);
	ch->was_in_room		= get_random_room(chToEnt(ch));

	if (nivel > 0)
		ch->trust	= nivel;

	SET_BIT(ch->act, PLR_NOSAVE);

	for ( i = 0; i < 10; i++ ) // 10 armas por area
		crear_armadura_random(ch);
}

void mob_odio_check(ROOM_INDEX_DATA *room, CHAR_DATA *ch)
{
	CHAR_DATA *gch;

	if (IS_NPC(ch) && !IS_PET(ch))
		return;

	for ( gch = room->people; gch; gch = gch->next_in_room )
	{
		if ( !IS_NPC(gch)
		||    gch->fighting != ch
		||    is_hunting(gch)
		||    IS_PET(gch)
		||    IS_AFFECTED(gch, AFF_CHARM) )
			continue;

		if ( IS_SET(gch->act, ACT_SENTINEL) )
		{
			if ( !IS_SET(gch->act, ACT_AGGRESSIVE) )
			{
				strip_mem_char(gch, MEM_HOSTILE);
				strip_mem_char(gch, MEM_AFRAID);
				give_mem(gch, MEM_HOSTILE, ch->id);
			}
		}
		else
		if ( getNivelPr(ch) > 5
		&&   can_hunt( gch, ch, FALSE ) )
			set_hunt( gch, ch, FALSE );
	}
}
