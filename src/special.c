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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "special.h"
#include "lookup.h"
#include "smart.h"

/* command procedures needed */
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_cast		);
DECLARE_DO_FUN(do_open		);
DECLARE_DO_FUN(do_close		);
DECLARE_DO_FUN(do_lock		);
DECLARE_DO_FUN(do_unlock	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_put		);
DECLARE_DO_FUN(do_order		);
DECLARE_DO_FUN(do_emote		);

/* the function table */
const   struct  spec_type    spec_table[] =
{
    {	"spec_breath_any",		spec_breath_any		},
    {	"spec_breath_acid",		spec_breath_acid	},
    {	"spec_breath_fire",		spec_breath_fire	},
    {	"spec_breath_frost",		spec_breath_frost	},
    {	"spec_breath_gas",		spec_breath_gas		},
    {	"spec_breath_lightning",	spec_breath_lightning	},	
    {	"spec_cast_adept",		spec_cast_adept		},
    {	"spec_cast_cleric",		spec_cast_cleric	},
    {	"spec_cast_judge",		spec_cast_judge		},
    {	"spec_cast_mage",		spec_cast_mage		},
    {	"spec_cast_undead",		spec_cast_undead	},
    {	"spec_executioner",		spec_executioner	},
    {	"spec_fido",			spec_fido		},
    {	"spec_guard",			spec_guard		},
    {	"spec_janitor",			spec_janitor		},
    {	"spec_mayor",			spec_mayor		},
    {	"spec_poison",			spec_poison		},
    {	"spec_thief",			spec_thief		},
    {	"spec_nasty",			spec_nasty		},
    {	"spec_troll_member",		spec_troll_member	},
    {	"spec_ogre_member",		spec_ogre_member	},
    {	"spec_patrolman",		spec_patrolman		},
    {   "spec_drunk",			spec_drunk		},
    {   "spec_basura",			spec_basura		},
    {	"spec_taxi",			spec_taxi		},
    {   "spec_taxidermist",		spec_taxidermist	},
    {   "spec_snake_charm",		spec_snake_charm	},
    {   "spec_questmaster",		spec_questmaster	},
    {   "spec_assassin",		spec_assassin		},
    {	"spec_cast_psionicist",		spec_cast_psionicist	},
    {	"spec_kungfu_poison",		spec_kungfu_poison	},
    {	"spec_teacher",			spec_teacher		},
    {	"spec_guard_white",		spec_guard_white	},
    {	"spec_amnesia",			spec_amnesia		},
    {	"spec_cast_ghost",		spec_cast_ghost		},
    {	"spec_hunter",			spec_hunter		},
    {	"spec_caminante",		spec_caminante		},
    {	"spec_buddha",			spec_buddha		},
    {	"spec_antivampiro",		spec_antivampiro	},
    {	"spec_plague",			spec_plague		},
    {	"spec_rat",			spec_rat		},
    {	"spec_troll_noche",		spec_troll_noche	},
    {	"spec_atacar_align_opuesta",	spec_atacar_align_opuesta	},
    {	NULL,				NULL			}
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
   int i;
 
   for ( i = 0; spec_table[i].name != NULL; i++)
   {
        if (LOWER(name[0]) == LOWER(spec_table[i].name[0])
        &&  !str_prefix( name,spec_table[i].name))
            return spec_table[i].function;
   }
 
    return 0;
}

char *spec_name( SPEC_FUN *function)
{
    int i;

    for (i = 0; spec_table[i].function != NULL; i++)
    {
	if (function == spec_table[i].function)
	    return spec_table[i].name;
    }

    return NULL;
}

bool spec_troll_member( CHAR_DATA *ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL 
    || IS_AFFECTED(ch,AFF_CHARM)
    || ch->fighting != NULL)
	return FALSE;

    /* find an ogre to beat up */
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
	if (!IS_NPC(vch) || ch == vch)
	    continue;

	if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	    return FALSE;

	if (vch->pIndexData->group == GROUP_VNUM_OGRES
	&&  getNivelPr(ch) > getNivelPr(vch) - 2 && !is_safe(ch,vch))
	{
	    if (number_range(0,count) == 0)
		victim = vch;

	    count++;
	}
    }

    if (victim == NULL)
	return FALSE;

    /* say something, then raise hell */
    switch (number_range(0,6))
    {
	default:  message = NULL; 	break;
	case 0:	message = "$n yells 'I've been looking for you, punk!'";
		break;
	case 1: message = "With a scream of rage, $n attacks $N.";
		break;
	case 2: message = 
		"$n says 'What's slimy Ogre trash like you doing around here?'";
		break;
	case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
		break;
	case 4: message = "$n says 'There's no cops to save you this time!'";
		break;	
	case 5: message = "$n says 'Time to join your brother, spud.'";
		break;
	case 6: message = "$n says 'Let's rock.'";
		break;
    }

    if (message != NULL)
    	act(message,ch,NULL,chToEnt(victim),TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

bool spec_ogre_member( CHAR_DATA *ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;
 
    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL
    || IS_AFFECTED(ch,AFF_CHARM)
    || ch->fighting != NULL)
        return FALSE;

    /* find an troll to beat up */
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
        if (!IS_NPC(vch) || ch == vch)
            continue;
 
        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return FALSE;
 
        if (vch->pIndexData->group == GROUP_VNUM_TROLLS
        &&  getNivelPr(ch) > getNivelPr(vch) - 2 && !is_safe(ch,vch))
        {
            if (number_range(0,count) == 0)
                victim = vch;
 
            count++;
        }
    }
 
    if (victim == NULL)
        return FALSE;
 
    /* say something, then raise hell */
    switch (number_range(0,6))
    {
	default: message = NULL;	break;
        case 0: message = "$n yells 'I've been looking for you, punk!'";
                break;
        case 1: message = "With a scream of rage, $n attacks $N.'";
                break;
        case 2: message =
                "$n says 'What's Troll filth like you doing around here?'";
                break;
        case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
                break;
        case 4: message = "$n says 'There's no cops to save you this time!'";
                break;
        case 5: message = "$n says 'Time to join your brother, spud.'";
                break;
        case 6: message = "$n says 'Let's rock.'";
                break;
    }
 
    if (message != NULL)
    	act(message,ch,NULL,chToEnt(victim),TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

bool spec_patrolman(CHAR_DATA *ch)
{
    CHAR_DATA *vch,*victim = NULL;
    OBJ_DATA *obj;
    char *message;
    int count = 0;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
        return FALSE;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch == ch)
	    continue;

	if (vch->fighting != NULL)  /* break it up! */
	{
	    if (number_range(0,count) == 0)
	        victim = (getNivelPr(vch) > getNivelPr(vch->fighting)) 
		    ? vch : vch->fighting;
	    count++;
	}
    }

    if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun))
	return FALSE;

    if (((obj = get_eq_char(ch,WEAR_NECK_1)) != NULL 
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
    ||  ((obj = get_eq_char(ch,WEAR_NECK_2)) != NULL
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
	act("You blow down hard on $p.",ch,objToEnt(obj),NULL,TO_CHAR);
	act("$n blows on $p, ***WHEEEEEEEEEEEET***",ch,objToEnt(obj),NULL,TO_ROOM);

    	for ( vch = char_list; vch != NULL; vch = vch->next )
    	{
            if ( vch->in_room == NULL )
            	continue;

            if (vch->in_room != ch->in_room 
	    &&  vch->in_room->area == ch->in_room->area)
            	send_to_char( "You hear a shrill whistling sound.\n\r", vch );
    	}
    }

    switch (number_range(0,6))
    {
	default:	message = NULL;		break;
	case 0:	message = "$n yells 'All roit! All roit! break it up!'";
		break;
	case 1: message = 
		"$n says 'Society's to blame, but what's a bloke to do?'";
		break;
	case 2: message = 
		"$n mumbles 'bloody kids will be the death of us all.'";
		break;
	case 3: message = "$n shouts 'Stop that! Stop that!' and attacks.";
		break;
	case 4: message = "$n pulls out his billy and goes to work.";
		break;
	case 5: message = 
		"$n sighs in resignation and proceeds to break up the fight.";
		break;
	case 6: message = "$n says 'Settle down, you hooligans!'";
		break;
    }

    if (message != NULL)
	act(message,ch,NULL,NULL,TO_ALL);

    multi_hit(ch,victim,TYPE_UNDEFINED);

    return TRUE;
}
	

bool spec_nasty( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    long gold;
 
    if (!IS_AWAKE(ch))
	return FALSE;
 
    if (ch->position != POS_FIGHTING)
    {
	for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
	{
		v_next = victim->next_in_room;

		if (!IS_NPC(victim)
		&& (getNivelPr(victim) > getNivelPr(ch) - 5)
		&& (getNivelPr(victim) < getNivelPr(ch) + 10))
		{
			do_backstab(ch,victim->name);
			if ( !char_died(victim) && ch->position != POS_FIGHTING )
				do_murder(ch,victim->name);
			/* should steal some coins right away? :) */
			return TRUE;
		}
	}

	return FALSE;	/* No one to attack */
    }

    /* okay, we must be fighting.... steal some coins and flee */
    if ( (victim = ch->fighting) == NULL)
        return FALSE;   /* let's be paranoid.... */

    switch ( number_bits(2) )
    {
        case 0:  act( "$n rips apart your coin purse, spilling your gold!",
                     ch, NULL, chToEnt(victim), TO_VICT);
                 act( "You slash apart $N's coin purse and gather his gold.",
                     ch, NULL, chToEnt(victim), TO_CHAR);
                 act( "$N's coin purse is ripped apart!",
                     ch, NULL, chToEnt(victim), TO_NOTVICT);
                 gold = victim->gold / 10;  /* steal 10% of his gold */
                 victim->gold -= gold;
                 ch->gold     += gold;
                 return TRUE;
 
        case 1:  do_flee( ch, "");
                 return TRUE;

        default: return FALSE;
    }
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( number_fuzzy(2) ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), NULL, TARGET_CHAR);
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell = NULL;
    int temp = 0;
    char buf[MIL];

    if ( ch->fighting )
    {
    	if (CHANCE(50))
    		return spec_cast_cleric(ch);
    	else
    		return spec_cast_mage(ch);
    }

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch
	&& can_see( ch, victim )
	&& !IS_NPC(victim)
	&& (number_bits(1) == 0)
	&& ((getNivelPr(victim) < 10) || (is_clan(ch) && is_same_clan(ch, victim) && (getNivelPr(victim) < 15))) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_range(0,13) )
    {
	case 0:	spell	= "armor";		temp = gsn_armor;		break;
	case 1:	spell	= "bless";		temp = gsn_bless;		break;
	case 2:	if (victim->hit < victim->max_hit)
		{
			spell = "cure light";
			temp = 0;
		}
		else
			return FALSE;
		break;
	case 3:	spell	= "cure blindness";	temp = gsn_blindness * -1;	break;
	case 4:	spell	= "cure poison";	temp = gsn_poison * -1;		break;
	case 5:	spell	= "refresh";		temp = 0;			break;
	case 6:	spell	= "cure disease";	temp = gsn_plague * -1;		break;
	case 7:	spell	= "giant strength";	temp = gsn_giant_strength;	break;
	case 8:	spell	= "combat mind";	temp = gsn_combat_mind;		break;
	case 9:	spell	= "enhance health";	temp = gsn_enhance_health;	break;
	case 10: spell	= "mystic armor";	temp = gsn_mystic_armor;	break;
	case 11: if ( !IS_AFFECTED(victim, AFF_FLYING) )
		 {
			spell	= "fly";
			temp	= gsn_fly;
		 }
		 else
		 	return FALSE;
		 break;
	case 12: spell	= "sanctuary";		temp = gsn_sanctuary;		break;
	case 13: spell	= "haste";		temp = gsn_haste;		break;
    }

    if ( temp > 0 && is_affected(victim, temp) )
    	return FALSE;

    if ( temp < 0 && !is_affected(victim, temp * -1) )
    	return FALSE;

    sprintf( buf, "'%s' %s", spell, victim->name );
    ch->mana = ch->max_mana;
    do_cast( ch, buf );

    return TRUE;
}

bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
	return FALSE;

    if ( IS_SET(ch->act, ACT_CLERIC) )
    	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 1 ) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( IS_AFFECTED2(ch, AFF_MUTE) )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  6: min_level = 12; spell = "change sex";     break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8: min_level = 15; spell = "estupidez";	   break;
	case  9: min_level = 14; spell = "mute";           break;
	case 10: min_level = 15; spell = "harm";           break;
	case 11: min_level = 15; spell = "plague";	   break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( getNivelPr(ch) >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;

    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);

    return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( ch->position != POS_FIGHTING )
        return FALSE;
 
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 1 ) )
            break;
    }

    if ( victim == NULL )
        return FALSE;
 
    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);
    return TRUE;
}

bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
	return FALSE;

    if ( IS_SET(ch->act, ACT_MAGE) )
    	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 1 ) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( IS_AFFECTED2(ch, AFF_MUTE) )
    	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
		case  0: min_level =  0; spell = "blindness";      break;
		case  1: min_level =  3; spell = "chill touch";    break;
		case  2: min_level =  7; spell = "weaken";         break;
		case  3: min_level =  8; spell = "teleport";       break;
		case  4: min_level = 11; spell = "colour spray";   break;
		case  5: min_level = 12; spell = "change sex";     break;
		case  6: min_level = 13; spell = "energy drain";   break;
		case  7: min_level = 20; spell = "amnesia";	   break;
		case  8: min_level = 15; spell = "estupidez";	   break;
		case  9: min_level = 15; spell = "fireball";       break;
		case 10: min_level = 20; spell = "plague";	   break;
		case 11: min_level = 15; spell = "alucinar";	   break;
		default: min_level = 20; spell = "acid blast";     break;
	}

	if ( getNivelPr(ch) >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;

    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);

    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 1 ) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 20; spell = "plague";	   break;
 	case  9: min_level = 15; spell = "estupidez";	   break;
 	case 10: if ( ch->race == RACE_VAMPIRE )
 		 {
 		    min_level = 24;
 		    spell = "vampiric bite";  break;
 		 }
	case 11: min_level = 15; spell = "alucinar";	   break;
	default: min_level = 18; spell = "harm";           break;
	}

	if ( getNivelPr(ch) >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);
    return TRUE;
}


bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;
    static bool trolls_here = FALSE;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    if ( (victim = ch->fighting) != NULL
    &&   IS_NPC(victim)
    &&   trolls_here == FALSE
    &&   ch->pIndexData->vnum >= 3000
    &&   ch->pIndexData->vnum <= 3399
    &&   victim->race == RACE_TROLL )
    {
	trolls_here = TRUE;

	REMOVE_BIT(ch->comm, COMM_NOSHOUT);
	do_yell( ch, "#BLOS TROLLS ATACAN MIDGAARD! A ELLOS!#b" );
    }

    if ( trolls_here && ch->position == POS_STANDING )
    {
	if ( time_info.hour == 5 )
	{
		trolls_here = FALSE;
		if ( !is_hunting(ch) )
			set_hunt_room( ch, ch->was_in_room );
		return FALSE;
	}

    	if ( is_hunting(ch) )
    		return FALSE;	// ya estamos ocupados

    	for ( victim = char_list; victim; victim = victim->next )
    		if ( IS_NPC(victim)
    		&&   victim->race == RACE_TROLL
    		&&   victim->in_room->area == ch->in_room->area
    		&&   can_see(ch, victim)
    		&&   can_hunt(ch, victim, TRUE) )
 			break;
 
 	if ( victim == NULL )
 		return FALSE;

	set_hunt(ch, victim, TRUE);
	return TRUE;
    }

    if (ch->fighting != NULL)
    	return FALSE;

    crime = "";
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	&&   can_see(ch,victim))
	    { crime = "KILLER"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	&&   can_see(ch,victim))
	    { crime = "THIEF"; break; }
    }

    if ( victim == NULL )
	return FALSE;

    sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	victim->name, crime );
    REMOVE_BIT(ch->comm,COMM_NOSHOUT);
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

			

bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse, TRUE );
	return TRUE;
    }

    return FALSE;
}

bool check_assist_guard( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MIL];
	CHAR_DATA *ech;

	if ( IS_AFFECTED(ch, AFF_CHARM) || number_bits(4) )
		return FALSE;

	for ( ech = char_list; ech; ech = ech->next )
    	{
    		if ( IS_NPC(ech)
		&&   (ech != ch)
    		&&  !IS_AFFECTED(ech,AFF_CHARM)
    		&&  !IS_SET(ech->act, ACT_PET)
		&&  !IS_SET(ech->act, ACT_SENTINEL)
    		&&   IS_AWAKE(ech)
    		&&   (ech->fighting == NULL)
    		&&  !is_hunting(ech)
		&&   (ech->in_room->area == ch->in_room->area)
		&&   (abs(getNivelPr(ech) - getNivelPr(ch)) < 3)
    		&&   ((ech->pIndexData == ch->pIndexData) || IS_SET(ech->off_flags, OFF_RESCUE) ) )
    			break;
	}

	if ( ech && CHANCE(getNivelPr(ech) * 2) )
	{
		SET_BIT( ech->affected_by, AFF_DETECT_INVIS );
		SET_BIT( ech->affected_by, AFF_DETECT_HIDDEN );
	}

	if ( !ech || !can_hunt( ech, victim, FALSE ) )
		return FALSE;

	if ( CAN_SAY(ch) )
	{
		sprintf( buf, "#BAYUDA#b! Estoy siendo atacado por %s!",
			PERS( victim, ch ) );
		REMOVE_BIT( ch->comm, COMM_NOSHOUT );
		do_yell( ch, buf );
	}
	else
		do_emote(ch, "empieza a silbar." );

	set_hunt( ech, victim, FALSE );

	return TRUE;
}

bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;
    static bool trolls_here = FALSE;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    if ( (victim = ch->fighting) )
    {
	if (!IS_NPC(victim)
	&&   getNivelPr(ch) > 30
	&&   check_assist_guard( ch, victim ) )
		return TRUE;

	if (IS_NPC(victim)
	&&  trolls_here == FALSE
	&&  ch->pIndexData->vnum >= 3000
	&&  ch->pIndexData->vnum <= 3399
	&&  victim->race == RACE_TROLL )
	{
		trolls_here = TRUE;

		REMOVE_BIT(ch->comm, COMM_NOSHOUT);
		do_yell( ch, "#BLOS TROLLS ATACAN MIDGAARD! A ELLOS!#b" );
	}
    }

    if ( ch->fighting )
    	return FALSE;

    if ( trolls_here )
    {
	if ( time_info.hour == 5 )
	{
		trolls_here = FALSE;
		if ( !is_hunting(ch) )
			set_hunt_room( ch, ch->was_in_room );
		return FALSE;
	}

    	if ( is_hunting(ch) )
    		return FALSE;	// ya estamos ocupados

    	for ( victim = char_list; victim; victim = victim->next )
    		if ( IS_NPC(victim)
    		&&   victim->race == RACE_TROLL
    		&&   victim->in_room->area == ch->in_room->area
    		&&   can_see(ch, victim)
    		&&   can_hunt(ch, victim, TRUE) )
 			break;
 
 	if ( victim == NULL )
 		return FALSE;

	set_hunt(ch, victim, TRUE);
	return TRUE;
    }

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	&&   can_see(ch,victim))
	    { crime = "KILLER"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	&&   can_see(ch,victim))
	    { crime = "THIEF"; break; }

	if ( victim->fighting != NULL
	&&   victim->fighting != ch
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL )
    {
	sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
	    victim->name, crime );
 	REMOVE_BIT(ch->comm,COMM_NOSHOUT);
	do_yell( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL )
    {
	act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	    ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n despierta y bosteza.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n se sienta y cae dormido.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n dice 'Hola, querida!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n dice 'Que vista!  Debo hacer algo contra ese basural!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n dice 'Vandalos!  Los jovenes ya no respetan nada!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n dice 'Buenos dias, ciudadanos!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n dice 'Abro oficialmente la ciudad de Midgaard!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n dice 'Cierro oficialmente la ciudad de Midgaard!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
	do_unlock( ch, "gate" );
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
	do_lock( ch, "gate" );
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > (getNivelPr(ch) + 20) )
	return FALSE;

    act( "Muerdes a $N!",  ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n muerde a $N!",  ch, NULL, chToEnt(victim), TO_NOTVICT );
    act( "$n te muerde!", ch, NULL, chToEnt(victim), TO_VICT    );

    spell_poison( gsn_poison, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);

    return TRUE;
}

bool spec_plague( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > (getNivelPr(ch) + 20) )
	return FALSE;

    act( "Muerdes a $N!",  ch, NULL, chToEnt(victim), TO_CHAR    );
    act( "$n muerde a $N!",  ch, NULL, chToEnt(victim), TO_NOTVICT );
    act( "$n te muerde!", ch, NULL, chToEnt(victim), TO_VICT    );

    spell_plague( gsn_plague, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);

    return TRUE;
}

bool spec_rat( CHAR_DATA *ch )
{
	if ( ch->fighting == NULL )
		return FALSE;

	if (CHANCE(50))
		return spec_plague(ch);
	else
		return spec_poison(ch);
}

bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold,silver;

    if ( ch->position != POS_STANDING
    ||   is_mob_safe(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   getNivelPr(victim) >= LEVEL_IMMORTAL
	||   number_bits( 3 ) != 0 
	||   !can_see(ch,victim))
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, getNivelPr(ch) ) == 0 )
	{
	    act( "Descubres las manos de $n en tu billetera!",
	    	ch, NULL, chToEnt(victim), TO_VICT );
	    act( "$N descubre las manos de $n en su billetera!",
	    	ch, NULL, chToEnt(victim), TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    gold = victim->gold * UMIN(number_range(1,20),getNivelPr(ch) / 2) / 100;
	    gold = UMIN(gold, getNivelPr(ch) * getNivelPr(ch) * 10 );
	    ch->gold     += gold;
	    victim->gold -= gold;
	    silver = victim->silver * UMIN(number_range(1,20),getNivelPr(ch)/2)/100;
	    silver = UMIN(silver,getNivelPr(ch)*getNivelPr(ch) * 25);
	    ch->silver	+= silver;
	    victim->silver -= silver;
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_drunk ( CHAR_DATA *ch )
{
    static const char open_path[] =
	"WEEaEEbEE3EEeEEcEE1EES.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( (time_info.hour % 4) ==  0 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n se despierta y bosteza.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n cae dormido encima de la mesa.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n toma un poco de cerveza de su barril.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n lanza un eructo que te deja mareado.",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n hace sus necesidades basicas en medio de la calle.",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n vomita...puaj!",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
        break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}

bool spec_basura( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;
    static const char open_path[] =
	"W00000B22222PS.";
    static const char *path;
    static int pos;
    static bool move;
    
    if ( !move )
    {
	if ( (time_info.hour) ==  8 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting )
	return spec_cast_mage( ch );

    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n se despierta y bosteza.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n cae dormido encima de un monton de basura.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
        break;

    case 'P':
        do_put(ch,"all tarro");
        break;

    case 'B':
        container = get_obj_world(ch,"donation");

	found = FALSE;

	if ( !container )
		break;

	for ( obj = container->contains; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		if ( can_see_obj( ch, obj ) && (obj->level < 11) )
		{
		    found = TRUE;
		    get_obj( ch, obj, container );
		}
	}
        break;
        
    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}

bool spec_taxidermist( CHAR_DATA *ch )
{
    OBJ_DATA *inv;
    int sn;

    if ( ch->position != POS_STANDING )
        return FALSE;

    if ( ch->pIndexData->pShop == 0 ||
         time_info.hour < ch->pIndexData->pShop->open_hour ||
         time_info.hour > ch->pIndexData->pShop->close_hour )
       return FALSE;

    for ( inv = ch->carrying; inv != NULL; inv = inv->next_content )
    {
	if (inv->item_type == ITEM_CORPSE_NPC)
	{
		if ( ( sn = skill_lookup( "make bag" ) ) >= 0 )
			(*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), objToEnt(inv), TARGET_CHAR);
		return TRUE;
	}
	else if (inv->wear_loc == WEAR_NONE && number_percent() < 3)
	{
		act( "$n te sugiere que compres $p.", ch, objToEnt(inv), NULL, TO_ROOM );
		return TRUE;
	}
    }

    return FALSE;
}

bool spec_snake_charm( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( ch->position != POS_FIGHTING )
       {
       switch ( number_bits( 3 ) ) {
       case 0:
          do_order( ch, "all sing charmer" ); /* a chance to get free here */
          break;
       case 1:
          do_order( ch,
             "all chat El area del encantador de serpientes es bacan..."
             "estoy ganando un monton de experiencia rapido!" );
          break;
       case 2:
          do_order( ch,
             "all chat 'SI!  Gane 327xp por matar al encantador de serpientes!");
          break;
       case 3:
          do_order( ch, "all remove dagger" );
          do_order( ch, "all give dagger charmer" );
          break;
       case 4:
          do_order( ch, "all remove sword" );
          do_order( ch, "all give sword charmer" );
          break;
       case 5:
          do_order( ch, "all remove mace" );
          do_order( ch, "all give mace charmer" );
          break;
       case 6:
	  do_order( ch, "all remove all" );
          do_order( ch, "all drop all" );
          break;
       case 7:
          do_order( ch, "all cast 'cure light' charmer" );
          break;
       };

       return TRUE;
       }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }

    if ( victim == NULL )
        return FALSE;

    act( "$n empieza a tocar una hermosa cancion.", ch, NULL, NULL, TO_ROOM );

    spell_charm_person(gsn_charm_person, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR );

    if (IS_AFFECTED(victim, AFF_CHARM))
    {
	stop_fighting( ch, FALSE );
	stop_fighting( victim, FALSE );
    }

    return TRUE;
}

bool spec_questmaster (CHAR_DATA *ch)
{
	return FALSE;
}

bool spec_taxi (CHAR_DATA *ch)
{
	return FALSE;
}

bool spec_assassin( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    char *message;

    if ( ch->fighting != NULL )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
	/* this should kill mobs as well as players */
	if (!es_clase(victim,CLASS_THIEF)) /* thieves */
		break;
    }

    if ( victim == NULL || victim == ch || IS_IMMORTAL(victim) )
        return FALSE;

    if ( getNivelPr(victim) > getNivelPr(ch) + 7 || IS_NPC(victim))
        return FALSE;

   switch (number_range(0,6))
   {
        default:  message = NULL;       break;
        case 0:  message =  "$n screams 'Meet The Grim Reaper'";
                 break;
        case 1:  message =  "$n yells 'Death to is the true end...'";
                 break;
        case 2:  message = "$n shouts ' Time to Die....'";
                 break;
        case 3:  message = "$n rasps 'Cabrone....'"; 
                 break;
        case 4:  message = "$n drools 'Welcome to your fate....'";
                 break;
        case 5:  message = "$n screams 'The Executioner has arrived.....'";
                 break;
        case 6:  message = "$n says 'Ever dance with the devil....'"; 
   }
   if (message != NULL)
        act(message,ch,NULL,chToEnt(victim),TO_ALL);
    multi_hit( ch, victim, gsn_backstab );
    return TRUE;
}

/*
 * Psionicist spec_fun by Thelonius for EnvyMud.
 */
bool spec_cast_psionicist( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    char      *spell;
    int        sn;

    if ( ch->position != POS_FIGHTING )
        return FALSE;

    for ( victim = ch->in_room->people; victim; victim = victim->next_in_room )
    {
        if ( victim->fighting == ch && can_see( ch, victim )
	    && number_bits( 1 ) )
            break;
    }

    if ( !victim )
        return FALSE;

    for ( ; ; )
    {
        int min_level;

        switch ( number_bits( 4 ) )
        {
        case  0: min_level =  0; spell = "mind thrust";          break;
        case  1: min_level =  4; spell = "psychic drain";        break;
        case  2: min_level =  6; spell = "agitation";            break;
        case  3: min_level =  8; spell = "psychic crush";        break;
        case  4: min_level =  9; spell = "project force";        break;
        case  5: min_level = 13; spell = "ego whip";             break;
        case  6: min_level = 14; spell = "energy drain";         break;
        case  7:
        case  8: min_level = 17; spell = "psionic blast";        break;
        case  9: min_level = 20; spell = "detonate";             break;
	case 10: min_level = 27; spell = "disintegrate";         break;
        default: min_level = 25; spell = "ultrablast";           break;
        }

        if ( getNivelPr(ch) >= min_level )
            break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR );
    return TRUE;
}

bool spec_kungfu_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * getNivelPr(ch) )
	return FALSE;

    act( "You hit $N with a poison palm technique!",  ch, NULL, chToEnt(victim), TO_CHAR);
    act( "$n hits $N with the poison palm technique!",  ch, NULL, chToEnt(victim), TO_NOTVICT);
    act( "$n hits you with the poison palm technique!", ch, NULL, chToEnt(victim), TO_VICT);
    spell_poison( gsn_poison, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);
    return TRUE;
}

bool spec_guard_white( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    if ( ( victim = ch->fighting)
    &&  !IS_NPC(victim)
    &&   getNivelPr(ch) > 30 )
	if ( check_assist_guard( ch, victim ) )
		return TRUE;

    if ( ch->fighting )
    	return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
	    { crime = "KILLER"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
	    { crime = "THIEF"; break; }

	if ( victim->fighting != NULL
	&&   victim->fighting != ch
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL )
    {
	sprintf( buf, "%s is a %s!  How DARE you come to the Temple!!!!",
	    victim->name, crime );
	REMOVE_BIT(ch->comm,COMM_NOSHOUT);
	do_yell( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL )
    {
	act( "$n screams 'Now you DIE you Bastard!!!!'",
	    ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}

bool spec_buddha( CHAR_DATA *ch )
{
	return spec_cast_cleric( ch );
}

bool spec_teacher( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n sacrifices the corpse to Buddha!", ch, NULL, NULL, TO_ROOM);
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse, TRUE );
	return TRUE;
    }

    return FALSE;
}

bool spec_amnesia( CHAR_DATA *ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POS_FIGHTING
	|| ( victim = ch->fighting ) == NULL
	||   number_percent( ) > 2 * getNivelPr(ch) )
		return FALSE;

	spell_amnesia( gsn_amnesia, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR);

	return TRUE;
}

bool spec_cast_ghost( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    char      *spell;
    int        sn;

    if  ( weather_info.sunlight != SUN_DARK )
    {
      if ( !ch->in_room )
      {
	  bug( "Spec_cast_ghost: NULL in_room.", 0 );
	  return FALSE;
      }

      if ( ch->fighting )
	  stop_fighting( ch, TRUE );

      act( "A beam of sunlight strikes $n, destroying $m.",
	  ch, NULL, NULL, TO_ROOM);

      extract_char ( ch, TRUE );
      return TRUE;
    }

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim; victim = victim->next_in_room )
    {
	if ( victim->fighting == ch && can_see( ch, victim )
	    && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim )
	return FALSE;

    for ( ; ; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 15; spell = "estupidez";	   break;
	case  9: min_level = 15; spell = "alucinar";	   break;
	default: min_level = 24; spell = "slow";           break;
	}

	if ( getNivelPr(ch) >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, getNivelPr(ch), chToEnt(ch), chToEnt(victim), TARGET_CHAR );
    return TRUE;
}

bool spec_hunter( CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *pc;

	if ( ch->in_room == NULL
	||   ch->position != POS_STANDING
	||   is_hunting(ch)
	||   ch->hit < ch->max_hit / 2
	||   mem_lookup(ch->memory, MEM_AFRAID) )
		return FALSE;

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING )
			continue;

		pc = CH(d);

		if ( !pc || !pc->in_room || IS_IMMORTAL(pc) )
			continue;

		if ( is_clan(ch) && is_same_clan(ch,pc) )
			continue;

		if ( (pc->in_room->area == ch->in_room->area)
		&& (is_clan(ch) || (abs(getNivelPr(pc) - getNivelPr(ch)) < 3))
		&& can_hunt(ch,pc, FALSE) )
		{
			set_hunt( ch, pc, FALSE );
			return TRUE;
		}
	}

	return FALSE;
}

bool spec_caminante( CHAR_DATA *ch )
{
	int door;
	EXIT_DATA *pexit;

	if ( !ch->in_room
	  ||  ch->fighting
	  ||  is_hunting(ch)
	  ||  ch->position != POS_STANDING )
		return FALSE;

	if ( !IS_SET(ch->act, ACT_SENTINEL) 
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

	return TRUE;
}

bool spec_antivampiro( CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *pc;

	if ( ch->in_room == NULL
	||   ch->position != POS_STANDING
	||   is_hunting(ch) )
		return FALSE;

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING )
			continue;

		pc = CH(d);

		if ( !pc
		||   !pc->in_room
		||    IS_IMMORTAL(pc)
		||    pc->race != RACE_VAMPIRE
		||    pc->in_room->area != ch->in_room->area
		||   (getNivelPr(pc) > getNivelPr(ch) + 8)
		||   (getNivelPr(pc) < getNivelPr(ch) - 8) )
			continue;

		set_hunt( ch, pc, FALSE );
		return TRUE;
	}

	return FALSE;
}

bool spec_troll_noche( CHAR_DATA *ch )
{
	if ( ch->in_room->vnum == 23090 )
		return FALSE;

	if ( weather_info.sunlight == SUN_DARK )
		return FALSE;

	if ( !IS_OUTSIDE(ch) )
		return FALSE;

	if ( ch->position != POS_STANDING )
		return FALSE;

	act( "$n ve la luz del sol y huye despavorido!", ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, get_room_index(23090) );

	return TRUE;
}

bool spec_atacar_align_opuesta( CHAR_DATA *ch )
{
	CHAR_DATA *tch;

	if ( ch->position != POS_STANDING )
		return FALSE;

	for ( tch = ch->in_room->people; tch; tch = tch->next_in_room )
		if ( can_see(ch, tch)
		&&  !IS_IMMORTAL(tch)
		&& (!IS_NPC(tch) || IS_PET(tch))
		&&   ES_ALIGN_OPUESTA(ch, tch) )
			break;

	if ( tch == NULL )
		return FALSE;

	if ( IS_EVIL(ch) )
	{
		act( "$n te mira con odio y grita 'Sal de aqui, bastardo!!!'",
			ch, NULL, chToEnt(tch), TO_VICT );
		act( "$n mira a $N y grita 'Sal de aqui, bastardo!!!'",
			ch, NULL, chToEnt(tch), TO_NOTVICT );
		act( "Miras a $n y gritas 'Sal de aqui, bastardo!!!'",
			ch, NULL, chToEnt(tch), TO_CHAR );
	}
	else
	{
		act( "$n te mira con odio y grita 'Que haces aqui, maldito!!!'",
			ch, NULL, chToEnt(tch), TO_VICT );
		act( "$n mira a $N y grita 'Que haces aqui, maldito!!!'",
			ch, NULL, chToEnt(tch), TO_NOTVICT );
		act( "Miras a $n y gritas 'Que haces aqui, maldito!!!'",
			ch, NULL, chToEnt(tch), TO_CHAR );
	}

	multi_hit( ch, tch, TYPE_UNDEFINED );

	return TRUE;
}
