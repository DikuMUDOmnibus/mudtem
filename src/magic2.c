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
#include "magic.h"
#include "tables.h"

#define CHECKENT()	if ( !entidadEsCh(ent) )		\
				return fERROR;			\
			else					\
				victim = entidadGetCh(ent);

#define CHECKOBJENT()	if ( !entidadEsObj(ent) )		\
				return fERROR;			\
			else					\
				obj = entidadGetObj(ent);

#define CHECKCH()	if ( !entidadEsCh(caster) )		\
				return fERROR;			\
			else					\
				ch = entidadGetCh(caster);

DECLARE_DO_FUN(do_scan);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_say);

extern char *target_name;

int evaluate( char *, CHAR_DATA *, CHAR_DATA *, double * );

SPELL_FUN_DEC(spell_farsight)
{
    CHAR_DATA *ch;

    CHECKCH()

    do_scan(ch,target_name);

    return fOK;
}

SPELL_FUN_DEC(spell_portal)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    CHAR_DATA *ch;

    CHECKCH()

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   entComparar(ent, caster)
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTIPO))
    ||   IS_SET(entWhereIs(caster)->room_flags, ROOM_NO_RECALL)
    ||   entGetNivel(ent) >= level + 3
    ||   (!ent_is_npc(ent) && entGetNivel(ent) >= LEVEL_HERO)  /* NOT trust */
    ||   (ent_is_npc(ent) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (ent_is_npc(ent) && ent_saves_spell( level, ent, DAM_NONE) ) 
    ||	(is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "Fallaste.\n\r", ch );
        return fFAIL;
    }   

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return fERROR;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,objToEnt(stone),NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,objToEnt(stone),NULL,TO_CHAR);
     	extract_obj(stone, TRUE);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,entWhereIs(caster));

    act("$p rises up from the ground.",ch,objToEnt(portal),NULL,TO_ROOM);
    act("$p rises up before you.",ch,objToEnt(portal),NULL,TO_CHAR);

    return fOK;
}

SPELL_FUN_DEC(spell_nexus)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;
    CHAR_DATA *ch;

    CHECKCH()

    from_room = entWhereIs(caster);
 
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   entComparar(ent, caster)
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   entGetNivel(ent) >= level + 3
    ||   (!ent_is_npc(ent) && entGetNivel(ent) >= LEVEL_HERO)  /* NOT trust */
    ||   (ent_is_npc(ent) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (ent_is_npc(ent) && ent_saves_spell( level, ent, DAM_NONE) ) 
    ||	 (is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "Fallaste.\n\r", ch );
        return fFAIL;
    }   
 
    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return fERROR;
    }
 
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,objToEnt(stone),NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,objToEnt(stone),NULL,TO_CHAR);
        extract_obj(stone, TRUE);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,objToEnt(portal),NULL,TO_ROOM);
    act("$p rises up before you.",ch,objToEnt(portal),NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return fOK;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,objToEnt(portal),NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,objToEnt(portal),NULL,TO_CHAR);
    }

    return fOK;
}

SPELL_FUN_DEC(spell_combat_mind)
{
    AFFECT_DATA af;

    if ( ent_is_affected( ent, sn ) )
    {
	if ( entComparar(ent, caster) )
		send_to_ent( "Ya entiendes tacticas de batalla.\n\r", caster );
	else
		nact( "$N ya entiende tacticas de batalla.", caster, NULL, ent, TO_CHAR );
	return fERROR;
    }

    af.type	 = sn;
    af.duration	 = level + 3;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = level / 6;
    af.level	 = level;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    af.location	 = APPLY_AC;
    af.modifier	 = - level/2 - 5;
    affect_to_ent( ent, &af );

    send_to_ent( "Aprendes un nuevo conjunto de tacticas de batalla.\n\r", ent );
    nact( "$n aprende un nuevo conjunto de tacticas de batalla.", ent, NULL, NULL, TO_ROOM );

    return fOK;
}

/*
 * Code for Psionicist spells/skills by Thelonius
 */
SPELL_FUN_DEC(spell_adrenaline_control)
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;
    CHAR_DATA *ch;

    CHECKCH()

    CHECKENT()

    if ( ent_is_affected( ent, sn ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level - 5;
    af.location	 = APPLY_DEX;
    af.level	 = level;
    af.modifier	 = 2;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    af.location	= APPLY_CON;
    affect_to_ent( ent, &af );

    send_to_char( "You have given yourself an adrenaline rush!\n\r", ch );
    act( "$n has given $mself an adrenaline rush!", ch, NULL, NULL,
	TO_ROOM );
   
    return fOK;
}

SPELL_FUN_DEC(spell_agitation )
{
    CHAR_DATA *victim;
    static const int        dam_each [ ] =
    {
	0,
	 0,  0,  0,  0,  0,      12, 15, 18, 21, 24,
	24, 24, 25, 25, 26,      26, 26, 27, 27, 27,
	28, 28, 28, 29, 29,      29, 30, 30, 30, 31,
	31, 31, 32, 32, 32,      33, 33, 33, 34, 34,
	34, 35, 35, 35, 36,      36, 36, 37, 37, 37
    };
		 int        dam;

    CHECKENT()

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   
    if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
      dam /= 2;

    return newdamage(caster,victim,dam,sn,DAM_MENTAL,TRUE );
}

SPELL_FUN_DEC(spell_aura_sight)
{
    sn = skill_lookup( "know alignment" );

    return (*skill_table[sn].spell_fun) ( sn, level, caster, ent, TAR_CHAR_SELF );
}

SPELL_FUN_DEC(spell_awe)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;

    CHECKCH()

    CHECKENT()

    if ( victim->fighting == ch && !ent_saves_spell( level, ent, DAM_MENTAL ) )
    {
	stop_fighting ( victim, TRUE);
	act( "$N is in AWE of you!", ch, NULL, ent, TO_CHAR    );
	act( "You are in AWE of $n!",ch, NULL, ent, TO_VICT    );
	act( "$N is in AWE of $n!",  ch, NULL, ent, TO_NOTVICT );
    }
    return fOK;
}

SPELL_FUN_DEC(spell_ballistic_attack )
{
                 CHAR_DATA *victim;
    static const int        dam_each [ ] =
    {
	 0,
	 3,  4,  4,  5,  6,       6,  6,  7,  7,  7,
	 7,  7,  8,  8,  8,       9,  9,  9, 10, 10,
	10, 11, 11, 11, 12,      12, 12, 13, 13, 13,
	14, 14, 14, 15, 15,      15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,      19, 19, 19, 20, 20
    };
		 int        dam;
	
    CHECKENT()

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
      dam /= 2;

    nact( "You chuckle as a stone strikes $N.", caster, NULL, ent,
	TO_CHAR );

    return newdamage( caster, victim, dam, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_biofeedback )
{
    return (*skill_table[gsn_sanctuary].spell_fun) ( gsn_sanctuary, level, caster, ent, TAR_CHAR_SELF );
}

SPELL_FUN_DEC(spell_cell_adjustment )
{
    CHAR_DATA *victim;
   
    CHECKENT()

    if ( ent_is_affected( ent, gsn_poison ) )
    {
	affect_strip( victim, gsn_poison );
	send_to_char( "A warm feeling runs through your body.\n\r", victim );
	nact( "$N looks better.", caster, NULL, ent, TO_NOTVICT );
    }
    if ( ent_is_affected( ent, gsn_curse  ) )
    {
	affect_strip( victim, gsn_curse  );
	send_to_char( "You feel better.\n\r", victim );
    }	
    send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_complete_healing )
{
    CHAR_DATA *victim;

    CHECKENT()

    change_health( caster, ent, entGetMaxHit(ent) - entGetHit(ent) );
    update_pos( victim );
    if ( !entComparar(caster, ent) )
        send_to_ent( "Ok.\n\r", caster );
    send_to_char( "Ahhhhhh...You are completely healed!\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_control_flames )
{
    CHAR_DATA *victim;
    static const int        dam_each [ ] = 
    {
	 0,
	 0,  0,  0,  0,  0,       0,  0, 16, 20, 24,
	28, 32, 35, 38, 40,      42, 44, 45, 45, 45,
	46, 46, 46, 47, 47,      47, 48, 48, 48, 49,
	49, 49, 50, 50, 50,      51, 51, 51, 52, 52,
	52, 53, 53, 53, 54,      54, 54, 55, 55, 55
    };
		 int        dam;

    CHECKENT()

    if ( !ent_get_eq_char( caster, WEAR_LIGHT ) )
    {
	send_to_ent( "You must be carrying a light source.\n\r", caster );
	return fERROR;
    }

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
        dam /= 2;

    newdamage( caster, victim, dam, sn, DAM_MENTAL, TRUE );

    if ( !char_died(victim) )
    	fire_effect( victim, level, dam, TARGET_CHAR );

    return fOK;
}

SPELL_FUN_DEC(spell_create_sound )
{
    CHAR_DATA *vch;
    char       buf1    [ MAX_STRING_LENGTH ];
    char       buf2    [ MAX_STRING_LENGTH ];
    char       speaker [ MAX_INPUT_LENGTH  ];

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r", speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER( buf1[0] );

    for ( vch = entGetPeopleRoom(caster); vch; vch = vch->next_in_room )
    {
	if ( !is_exact_name( speaker, vch->name ) && IS_AWAKE(vch) )
	    send_to_char( saves_spell( level, vch, DAM_MENTAL ) ? buf2 : buf1, vch);
    }
    return fOK;
}

SPELL_FUN_DEC(spell_death_field )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int        dam;
    int        hpch;

    if ( !ent_is_evil( caster ) )
    {
	send_to_ent( "You are not evil enough to do that!\n\r", caster);
	return fERROR;
    }

    send_to_ent( "A black haze emanates from you!\n\r", caster );
    nact ( "A black haze emanates from $n!", caster, NULL, NULL, TO_ROOM );

    for ( vch = entGetPeopleRoom(caster); vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ( !ent_is_safe_spell( caster, chToEnt(vch), TRUE) )
	{
	    hpch = URANGE( 10, entGetHit(caster), 999 );

	    dam = number_range( hpch / 8 + 1, hpch / 4);

	    if ( !saves_spell( level, vch, DAM_MENTAL ) && (entGetNivel(caster) - getNivelPr(vch) > 5) )
	    {
		send_to_char( "The haze envelops you!\n\r", vch );
		act( "The haze envelops $n!", vch, NULL, NULL, TO_ROOM );
		dam *= 2 + CHANCE(level);
	    }

	    dam = UMIN( dam, 1200 );
	    newdamage( caster, vch, dam, sn, DAM_MENTAL, TRUE );
        }
    }

    return fOK;
}

SPELL_FUN_DEC(spell_detonate )
{
    CHAR_DATA *victim;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,        0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,        0,   0,   0,   0,  75,
	 80,  85,  90,  95, 100,      102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,      122, 124, 126, 128, 130,
	132, 134, 136, 138, 140,      142, 144, 146, 148, 150
    };
    int        dam;

    CHECKENT()

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
        dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_disintegrate )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;

    CHECKENT()

    if ( !ent_is_npc(ent) )
    	return fERROR;

    if ( number_percent( ) < 2 * level && !ent_saves_spell( level, ent, DAM_MENTAL ) )
      for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
      {
	  obj_next = obj_lose->next_content;

	  if ( number_bits( 2 ) != 0 )
	      continue;

	  act( "$p disintegrates!", victim, objToEnt(obj_lose), NULL, TO_CHAR );
	  act( "$p disintegrates!", victim, objToEnt(obj_lose), NULL, TO_ROOM );
	  extract_obj( obj_lose, TRUE ) ;
      }

    if ( !ent_saves_spell( level, ent, DAM_MENTAL ) )
    /*
     * Disintegrate char, do not generate a corpse, do not
     * give experience for kill.  Extract_char will take care
     * of items carried/wielded by victim.  Needless to say,
     * it would be bad to be a target of this spell!
     * --- Thelonius (Monk)
     */
    {
	nact( "Tu #B*** #FDESINTEGRASTE#f ***#b a $N!", caster, NULL, ent, TO_CHAR );
	nact( "Fuiste #B*** #FDESINTEGRADO#f ***#b por $n!", caster, NULL, ent, TO_VICT );
	nact( "El spell de $n #B*** #FDESINTEGRA#f ***#b a $N!", caster, NULL, ent, TO_NOTVICT );

	extract_char( victim, TRUE );
	return victdead;
    }

    return fOK;
}

SPELL_FUN_DEC(spell_displacement )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) )
	return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level - 4;
    af.location	 = APPLY_AC;
    af.level	 = level;
    af.modifier	 = 4 - level;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_char( "Your form shimmers, and you appear displaced.\n\r",
		 victim );
    nact( "$N shimmers and appears in a different location.",
	caster, NULL, ent, TO_NOTVICT );
    return fOK;
}

SPELL_FUN_DEC(spell_domination )
{
    CHAR_DATA  *victim;
    CHAR_DATA *ch;
    AFFECT_DATA af;

    CHECKCH()
    CHECKENT()

    if ( entComparar(ent, caster) )
    {
	send_to_ent( "Dominate yourself?  You're weird.\n\r", caster );
	return fERROR;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   ent_IS_AFFECTED(caster, AFF_CHARM)
    ||   level < entGetNivel(ent)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   ent_saves_spell( level, ent, DAM_CHARM) )
	return fFAIL;

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_ent(
	    "The mayor does not allow charming in the city limits.\n\r",caster);
	return fERROR;
    }

    if ( victim->master )
        stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_NONE;
    af.level	 = level;
    af.modifier	 = 0;
    af.bitvector = AFF_CHARM;
    affect_to_ent( ent, &af );

    nact( "Your will dominates $N!", caster, NULL, ent, TO_CHAR );
    nact( "Your will is dominated by $n!", caster, NULL, ent, TO_VICT );
    return fOK;
}

SPELL_FUN_DEC(spell_ectoplasmic_form )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.level	 = level;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_ent( ent, &af );

    send_to_char( "You turn translucent.\n\r", victim );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    return fOK;
}

SPELL_FUN_DEC(spell_ego_whip )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) || ent_saves_spell( level, ent, DAM_MENTAL ) )
        return fFAIL;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = -2;
    af.bitvector = 0;
    af.level	 = level;
    affect_to_ent( ent, &af );

    af.location	 = APPLY_SAVING_SPELL;
    af.modifier	 = 2;
    affect_to_ent( ent, &af );

    af.location	 = APPLY_AC;
    af.modifier	 = level / 2;
    affect_to_ent( ent, &af );

    nact( "You ridicule $N about $S childhood.", caster, NULL, ent, TO_CHAR    );
    send_to_char( "Your ego takes a beating.\n\r", victim );
    nact( "$N's ego is crushed by $n!",          caster, NULL, ent, TO_NOTVICT );

    return fOK;
}

SPELL_FUN_DEC(spell_energy_containment )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level / 2 + 7;
    af.modifier	 = -level / 5;
    af.location  = APPLY_SAVING_SPELL;
    af.level	 = level;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_ent( "You can now absorb some forms of energy.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_enhance_armor)
{
    OBJ_DATA    *obj;
    AFFECT_DATA af;

    CHECKOBJENT()

    if ( obj->item_type != ITEM_ARMOR
      || IS_OBJ_STAT( obj, ITEM_MAGIC )
      || obj->affected )
    {
	send_to_ent( "Ese objeto no puede ser encantado.\n\r", caster );
	return fERROR;
    }

    af.where		= TO_OBJECT;
    af.type		= sn;
    af.duration		= -1;
    af.location		= APPLY_AC;
    af.level		= level;
    af.bitvector	= 0;

    if ( number_percent() < ent_get_skill(caster, sn)/2
	+ 3 * ( entGetNivel(caster) - obj->level ) )

    /* Good enhancement */
    {
	af.modifier   = -level / 8;

	if ( ent_is_good( caster ) )
	{
		SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
		nact( "$p empieza a brillar con un tono azul.",   caster, objToEnt(obj), NULL, TO_CHAR );
	}
	else if ( ent_is_evil( caster ) )
	{
		SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
		nact( "$p empieza a brillar con un tono rojizo.", caster, objToEnt(obj), NULL, TO_CHAR );
	}
	else
	{
		SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
		SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
		nact( "$p empieza a brillar con un tono dorado.", caster, objToEnt(obj), NULL, TO_CHAR );
	}

	SET_BIT(obj->extra_flags, ITEM_GLOW);
	send_to_ent( "Ok.\n\r", caster );
    }
    else
    /* Bad Enhancement ... opps! :) */
    {
	af.modifier	= level / 8;
	obj->cost	= 0;

	SET_BIT( obj->extra_flags, ITEM_NODROP );
	SET_BIT( obj->extra_flags, ITEM_DARK );
	nact( "$p cambia a un color oscuro.", caster, objToEnt(obj), NULL, TO_CHAR );
    }

    affect_to_obj( obj, &af );

    return fOK;
}

SPELL_FUN_DEC(spell_enhanced_strength )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn )
    ||   ent_is_affected( ent, skill_lookup( "giant strength" ) ) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already as strong as you can get!\n\r",caster);
	else
	  nact("$N can't get any stronger.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level;
    af.level	 = level;
    af.location	 = APPLY_STR;
    af.modifier	 = 1 + ( level >= 15 ) + ( level >= 25 );
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_char( "You are HUGE!\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_flesh_armor )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn )
    ||   ent_is_affected( ent, skill_lookup("stone skin") ) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("Your skin is already as hard as a rock.\n\r",caster); 
	else
	  nact("$N is already as hard as can be.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level;
    af.location	 = APPLY_AC;
    af.level	 = level;
    af.modifier	 = -40;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_char( "Your flesh turns to steel.\n\r", victim );
    nact( "$N's flesh turns to steel.", caster, NULL, ent, TO_NOTVICT);
    return fOK;
}

SPELL_FUN_DEC(spell_inertial_barrier )
{
    CHAR_DATA  *gch;
    AFFECT_DATA af;

    for ( gch = entGetPeopleRoom(caster); gch; gch = gch->next_in_room )
    {
	if ( !ent_is_same_group( chToEnt(gch), caster )
	   || IS_AFFECTED( gch, AFF_PROTECT_EVIL )
	   || IS_AFFECTED( gch, AFF_PROTECT_GOOD) )
	    continue;

	act( "An inertial barrier forms around $n.", gch, NULL, NULL,
	    TO_ROOM );
	send_to_char( "An inertial barrier forms around you.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type	     = sn;
	af.duration  = 24;
        af.level     = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_PROTECT_EVIL;
	af.caster_id = entGetId(caster);
	affect_to_char( gch, &af );
    }
    return fOK;
}

SPELL_FUN_DEC(spell_inflict_pain )
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice( 2, 10 ) + level / 2, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_intellect_fortress )
{
    CHAR_DATA  *gch;
    AFFECT_DATA af;

    for ( gch = entGetPeopleRoom(caster); gch; gch = gch->next_in_room )
    {
	if ( !ent_is_same_group( chToEnt(gch), caster ) || is_affected( gch, sn ) )
	    continue;

	send_to_char( "A virtual fortress forms around you.\n\r", gch );
	act( "A virtual fortress forms around $n.", gch, NULL, NULL, TO_ROOM );

        af.where     = TO_AFFECTS;
	af.type	     = sn;
	af.duration  = 24;
        af.level     = level;
	af.location  = APPLY_AC;
	af.modifier  = -40;
	af.bitvector = 0;
	af.caster_id = entGetId(caster);
	affect_to_char( gch, &af );
    }
    return fOK;
}

SPELL_FUN_DEC(spell_lend_health )
{
    CHAR_DATA *victim;
    int        hpch;

    CHECKENT()

    if ( entComparar(caster, ent) )
    {
	send_to_ent( "Lend health to yourself?  What a weirdo.\n\r", caster );
	return fERROR;
    }
    hpch = UMIN( 50, entGetMaxHit(ent) - entGetHit(ent) );
    if ( hpch == 0 )
    {
	nact( "Nice thought, but $N doesn't need healing.", caster, NULL,
	    ent, TO_CHAR );
	return fERROR;
    }
    if ( entGetHit(caster)-hpch < 50 )
    {
	send_to_ent( "You aren't healthy enough yourself!\n\r", caster );
	return fERROR;
    }
    change_health(caster, ent, hpch);
    change_health(caster, caster, -hpch);
    update_pos( victim );
    ent_update_pos( caster );

    nact( "You lend some of your health to $N.", caster, NULL, ent, TO_CHAR );
    nact( "$n lends you some of $s health.",     caster, NULL, ent, TO_VICT );

    return fOK;
}

SPELL_FUN_DEC(spell_levitation )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level + 3;
    af.location	 = APPLY_NONE;
    af.level     = level;
    af.modifier	 = 0;
    af.bitvector = AFF_FLYING;
    affect_to_ent( ent, &af );

    send_to_char( "Tus pies empiezan a elevarse.\n\r", victim );
    act( "$n empieza a levitar.", victim, NULL, NULL, TO_ROOM );
    return fOK;
}

SPELL_FUN_DEC(spell_mental_barrier )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = 24;
    af.location	 = APPLY_AC;
    af.modifier	 = -20;
    af.level     = level;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_char( "Levantas una barrera mental a tu alrededor.\n\r",
		 victim );
    return fOK;
}

SPELL_FUN_DEC(spell_mind_thrust )
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice( 1, 10 ) + level / 2, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_project_force )
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice( 4, 6 ) + level, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_psionic_blast )
{
                 CHAR_DATA *victim;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,        0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,        0,  45,  50,  55,  60,
	 64,  68,  72,  76,  80,       82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,      102, 104, 106, 108, 100,
	112, 114, 116, 118, 120,      122, 124, 126, 128, 130
    };
		 int        dam;

    CHECKENT()

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
        dam /= 2;

    return newdamage( caster, victim, dam, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_psychic_crush )
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice( 3, 5 ) + level, sn, DAM_MENTAL, TRUE );
}

SPELL_FUN_DEC(spell_psychic_drain )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) || ent_saves_spell( level, ent, DAM_MENTAL ) )
        return fFAIL;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level / 2;
    af.level     = level;
    af.location	 = APPLY_STR;
    af.modifier	 = -1 - ( level >= 10 ) - ( level >= 20 ) - ( level >= 30 );
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_char( "You feel drained.\n\r", victim );
    act( "$n appears drained of strength.", victim, NULL, NULL, TO_ROOM );
    return fOK;
}

SPELL_FUN_DEC(spell_psychic_healing )
{
    CHAR_DATA *victim;
    int heal;

    CHECKENT()

    heal = dice( 3, 6 ) + 2 * level / 3 ;
    change_health(caster, ent, heal);
    update_pos( victim );

    if ( !entComparar(caster, ent) )
    	send_to_ent( "Ok.\n\r", caster );

    send_to_char( "Te sientes mejor!\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_share_strength )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( entComparar(ent, caster) )
    {
	send_to_ent( "No puedes compartir fuerza contigo mismo.\n\r", caster );
	return fERROR;
    }
    if ( ent_is_affected( ent, sn ) )
    {
	nact( "$N ya esta compartiendo fuerza con alguien.", caster, NULL, ent,
	    TO_CHAR );
	return fERROR;
    }
    if ( ent_get_curr_stat( caster, STAT_STR ) <= 5 )
    {
	send_to_ent( "Estas demasiado debil como para compartir tu fuerza.\n\r", caster );
	return fERROR;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.level     = level;
    af.duration	 = level;
    af.location	 = APPLY_STR;
    af.modifier	 =  1 + ( level >= 20 ) + ( level >= 30 );
    af.bitvector = 0;
    affect_to_ent( ent, &af );
    
    af.modifier	 = -1 - ( level >= 20 ) - ( level >= 30 );
    affect_to_ent( caster,     &af );

    nact( "Compartes tu fuerza con $N.", caster, NULL, ent, TO_CHAR );
    nact( "$n comparte su fuerza contigo.",  caster, NULL, ent, TO_VICT );
    return fOK;
}

SPELL_FUN_DEC(spell_thought_shield )
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( ent_is_affected( ent, sn ) )
        return fERROR;

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.duration	 = level;
    af.level     = level;
    af.location	 = APPLY_AC;
    af.modifier	 = -20;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_ent( "Has creado un escudo a tu alrededor.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_ultrablast )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int        dam;
    int        hpch;

    for ( vch = entGetPeopleRoom(caster); vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ( ent_is_npc( caster ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
	{
	    hpch = UMAX( 10, entGetHit(caster) );
	    dam  = number_range( hpch / 8+1, hpch / 4 );
	    if ( saves_spell( level, vch, DAM_MENTAL ) )
	        dam /= 2;
	    newdamage( caster, vch, dam, sn, DAM_MENTAL, TRUE );
	}
    }
    return fOK;
}

SPELL_FUN_DEC(spell_vampiric_bite)
{
    CHAR_DATA  *victim;
    OBJ_DATA   *obj;
    AFFECT_DATA af;
    int         dam;

    CHECKENT()

    dam = dice( 5, level );
    newdamage( caster, victim, dam, sn, DAM_POISON, TRUE );

    change_health( caster, caster, dam );

    if ( entGetNivel(ent) < 11 || get_age( victim ) < 21 )
        return fERROR;

    if ( IS_AFFECTED( victim, AFF_POLYMORPH ) )
        return fERROR;

    if ( ent_saves_spell( level, ent, DAM_POISON )
	|| number_bits( 1 ) == 0 )
	return fFAIL;

    if ( ( obj = get_eq_char( victim, ITEM_HOLD ) ) )
    {
        if ( IS_OBJ_STAT( obj, ITEM_VAMPIRE_BANE )
	    && entGetNivel(caster) < 21 )
	    return fERROR;
	else
	{
	    if ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) )
	    {
	        if ( entGetNivel(caster) < 32 )
		{
		    return fERROR;
		}
		else
		{
		    if ( entGetNivel(ent) > entGetNivel(caster) )
		        return fERROR;
		}
	    }
	}
    }
		  
    af.where	 = TO_AFFECTS2;
    af.type      = sn;
    af.duration  = UMAX( 5, 30 - level );
    af.location  = APPLY_NONE;
    af.level     = level;
    af.modifier  = 0;
    af.bitvector = AFF_VAMP_BITE;
    affect_join( victim, &af );

    if ( !entComparar(caster, ent) )
	send_to_ent( "Ahh!  Taste the power!\n\r", caster );
    send_to_char( "Your blood begins to burn!\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_turn_undead)
{
    CHAR_DATA  *victim;

    CHECKENT()

    if (   entGetNivel(ent) >= level
	|| ent_saves_spell( level, ent, DAM_OTHER ) )
    {
        send_to_ent( "You have failed.\n\r", caster );
	return fFAIL;
    }

    if ( (victim->race == RACE_VAMPIRE) || IS_SET(victim->form, FORM_UNDEAD) )
    	do_flee( victim, "" );

    return fOK;
}

SPELL_FUN_DEC(spell_exorcise)
{
    CHAR_DATA *victim;

    CHECKENT()

    if ( !ent_is_affected( ent, gsn_vampiric_bite )
    &&  (!IS_IMMORTAL(victim) || !ent_is_affected( ent, gsn_stake )) )
    {
    	nact( "$N no necesita un exorcismo.", caster, NULL, ent, TO_CHAR );
        return fERROR;
    }

    /*
     * We actually give the vampiric curse a chance to save
     * at the victim's level
     */
    if ( ent_saves_spell( level, ent, DAM_POISON ) )
    {
    	send_to_ent( "No pasa nada.\n\r", caster );
	return fFAIL;
    }

    affect_strip( victim, gsn_vampiric_bite );

    if ( IS_IMMORTAL(victim) )
    	affect_strip( victim, gsn_stake );

    send_to_ent( "Ok.\n\r",                                    caster     );
    send_to_char( "A warm feeling runs through your body.\n\r", victim );
    nact( "$N looks better.", caster, NULL, ent, TO_NOTVICT );

    strip_mem_char( victim, MEM_VAMPIRE );

    return fOK;
}

SPELL_FUN_DEC(spell_mute)
{
    CHAR_DATA  *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED2( victim, AFF_MUTE )
	|| ent_saves_spell( level - 4, ent, DAM_OTHER ) )
    {
	send_to_ent( "Pronunciaste mal una silaba.\n\r", caster );
	return fFAIL;
    }

    af.where	 = TO_AFFECTS2;
    af.level	 = level - 2;
    af.type      = sn;
    af.duration  = level / 8;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_MUTE;
    affect_to_ent( ent, &af );
    
    nact( "You have silenced $N!", caster, NULL, ent, TO_CHAR    );
    nact( "$n has silenced you!",  caster, NULL, ent, TO_VICT    );
    nact( "$n has silenced $N!",   caster, NULL, ent, TO_NOTVICT );

    if ( ent_is_npc(ent) && IS_MOB_MAGIC(victim) )
    	SET_BIT(victim->act, ACT_WIMPY);

    return fOK;
}

SPELL_FUN_DEC(spell_amnesia)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;

   CHECKENT()

   if ( IS_AFFECTED2( victim, AFF_AMNESIA)
     || ent_saves_spell( level, ent, DAM_MENTAL ) )
     return fFAIL;
   
   af.where	= TO_AFFECTS2;
   af.level	= level;
   af.type	= sn;
   af.duration	= level / 4;
   af.location	= 0;
   af.modifier	= 0;
   af.bitvector	= AFF_AMNESIA;
   affect_to_ent( ent, &af );

   nact( "No recuerdas nada!", caster, NULL, ent, TO_VICT );
   nact( "$N parece estar muy confundido.", caster, NULL, ent, TO_CHAR );
   nact( "$N parece estar muy confundido.", caster, NULL, ent, TO_NOTVICT );

   return fOK;
}

SPELL_FUN_DEC(spell_levantar_muertos)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    int i;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA af;
    CHAR_DATA *ch;

    CHECKCH()

    obj = ent_get_obj_here( caster, target_name );

    if ( obj == NULL )
    {
        send_to_ent( "Resucitar que?\n\r", caster );
        return fERROR;
    }

    /* Nothing but NPC corpses. */
    if( obj->item_type != ITEM_CORPSE_NPC )
    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_ent( "No puedes resucitar jugadores.\n\r", caster );
        else
            send_to_ent( "No serviria para nada...\n\r", caster );
        return fERROR;
    }

    if( obj->value[3] == MOB_VNUM_ZOMBIE )
    {
    	send_to_ent( "No puedes resucitar un zombie!\n\r", caster );
    	return fERROR;
    }

    if( obj->level > (entGetNivel(caster) + 2) )
    {
        send_to_ent( "No puedes traer de vuelta a un espiritu tan poderoso.\n\r", caster );
        return fERROR;
    }

    if ( obj->level < 5 )
    {
    	send_to_ent( "Ese espiritu es demasiado debil.\n\r", caster );
    	return fERROR;
    }

    if( entGetPet(caster) != NULL )
    {
        send_to_ent( "Ya tienes una mascota.\n\r", caster );
        return fERROR;
    }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */

    mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );

    sprintf( buf, "zombie %s", obj->name );
    free_string(mob->name);
    mob->name = str_dup(buf);

    sprintf(buf,"Un zombie de %s esta aqui, esperando a su maestro.\n\r",obj->short_descr);
    free_string(mob->long_descr);
    mob->long_descr = str_dup(buf);

    sprintf(buf,"un zombie de %s",obj->short_descr);
    free_string(mob->short_descr);
    mob->short_descr = str_dup(buf);

    setClasePr(mob, CHANCE(80) ? CLASS_WARRIOR : CLASS_CLERIC, obj->level - 2);

    mob->max_hit                = getNivelPr(mob) * 8 + number_range(
                                        getNivelPr(mob) * getNivelPr(mob)/4,
                                        getNivelPr(mob) * getNivelPr(mob));
    mob->max_hit				= (int) (mob->max_hit * .9);
    mob->hit                    = mob->max_hit;
    mob->max_mana               = 100 + dice(getNivelPr(mob),10);
    mob->mana                   = mob->max_mana;
    for (i = 0; i < 3; i++)
        mob->armor[i]           = interpolate(getNivelPr(mob),100,-100);
    mob->armor[3]               = interpolate(getNivelPr(mob),100,0);

    for (i = 0; i < MAX_STATS; i++)
        mob->perm_stat[i] = 11 + getNivelPr(mob)/4;

    mob->damage[DICE_NUMBER]	= recval_table[getNivelPr(mob)].numdam;
    mob->damage[DICE_TYPE]	= recval_table[getNivelPr(mob)].typdam;

    /* You rang? */
    char_to_room( mob, entWhereIs(caster) );
    nact( "$p vuelve a la vida como un zombie!", caster, objToEnt(obj), NULL, TO_ROOM );
    nact( "$p vuelve a la vida como un zombie!", caster, objToEnt(obj), NULL, TO_CHAR );

    extract_obj(obj, TRUE);

    /* Yessssss, massssssster... */
    af.where		= TO_AFFECTS;
    af.level		= level;
    af.type		= sn;
    af.duration		= level;
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector	= AFF_CHARM;
    af.caster_id	= entGetId(caster);
    affect_to_char( mob, &af );

    SET_BIT(mob->act, ACT_PET);
    mob->timer	= level;
    add_follower( mob, ch );
    mob->leader	= ch;
    ch->pet = mob;
    /* For a little flavor... */
    REMOVE_BIT(mob->comm, COMM_NOCHANNELS);
    do_say( mob, "Como puedo servirte, maestro?" );
    mob->comm	= COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    return fOK;
}

SPELL_FUN_DEC(spell_creature_bond)
{
	CHAR_DATA *victim;
	CHAR_DATA *master;

	CHECKENT()

	if ( !IS_NPC( victim ) || !IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_ent( "Esa criatura no esta encantada!\n\r", caster );
		return fERROR;
	}

	master = victim->master ? victim->master : victim;

	if ( master && entGetNivel(caster) < getNivelPr(master) - 20 )
	{
		send_to_ent( "The current bond is too strong for you to overcome.\n\r", caster );
		return fERROR;
	}

	if ( number_range( 0, 105 ) < ( level + entGetNivel(caster) - getNivelPr(master) ) )
	{
		if ( ent_saves_spell( level, ent, DAM_CHARM ) )
		{
			do_say( victim, "Como te atreves!, yo ADORO a mi maestro!" );
			ent_multi_hit( ent, caster, TYPE_UNDEFINED );
			return fFAIL;
		}
		stop_follower( victim );
		SET_BIT( victim->act, ACT_PET );
		SET_BIT( victim->affected_by, AFF_CHARM );
		victim->timer = level / 3;
		ent_add_follower( ent, caster );
	}
	else
	{
		do_say( victim, "Como te atreves!, yo ADORO a mi maestro!" );
		ent_multi_hit( ent, caster, TYPE_UNDEFINED );
		return fFAIL;
	}

	return fOK;
}

SPELL_FUN_DEC(spell_corrupt_bond)
{
	CHAR_DATA *victim;
	CHAR_DATA *master = NULL;

	CHECKENT()

	if ( !IS_NPC( victim ) || !IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_ent( "Esa criatura no esta encantada!\n\r", caster );
		return fERROR;
	}

	master = victim->master ? victim->master : victim;

	if ( entGetNivel(caster) < getNivelPr(master) - 20 )
	{
		send_to_ent( "The current bond is too strong for you to corrupt.\n\r", caster );
		return fERROR;
	}

	if ( number_percent() < level + entGetNivel(caster) - getNivelPr(master) )
	{
		if ( ent_saves_spell( level, ent, DAM_CHARM ) )
		{
			do_say( victim, "Como te atreves!, yo ADORO a mi maestro!" );
			ent_multi_hit( ent, caster, TYPE_UNDEFINED );
			return fFAIL;
		}

		stop_follower( victim );
		if ( victim->in_room == master->in_room )
		{
			do_say( victim, "Ahora podre vengarme por el encantamiento!!!" );
			multi_hit( victim, master, TYPE_UNDEFINED );
			return fOK;
		}
		else
		{
			do_say( victim, "AARRGH!  ODIO estar encantado! Ahora tendre mi #BVENGANZA#b!" );
			set_hunt( victim, master, FALSE );
			return fOK;
		}
	}
	else
	{
		do_say( victim, "Como te atreves!, yo ADORO a mi maestro!" );
		ent_multi_hit( ent, caster, TYPE_UNDEFINED );
		return fFAIL;
	}

	return fOK;
}

SPELL_FUN_DEC(spell_mystic_armor)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;

   CHECKENT()

   if ( entComparar(caster, ent) )
   {
       send_to_ent( "You are mystically armoured, but it suddenly fades away!\n\r", caster );
       return fERROR;
   }

    if ( ent_is_affected( ent, sn ) )
         return fERROR;

    af.where		= TO_AFFECTS;
    af.type		= sn;
    af.duration		= 4 + (level/3);
    af.location		= APPLY_AC;
    af.modifier		= -10;
    af.bitvector	= 0;
    af.level		= level;
    affect_to_ent( ent, &af );

    nact( "$N is surrounded by your mystic armour.", caster, NULL, ent, TO_CHAR );
    nact( "You are surrounded by $n's mystic armour.", caster, NULL, ent, TO_VICT );
    nact( "$N is surrounded by $n's mystic armour.", caster, NULL, ent, TO_NOTVICT );

    return fOK;
}

SPELL_FUN_DEC(spell_stalker)
{
   /* Fixed problem of stalker not finding victim, and attacking caster,
    * thus giving an easy source of xp -S-
    */

   CHAR_DATA *victim;
   CHAR_DATA *stalker;

   if ( target_name[0] == '\0' )
   {
      send_to_ent( "Summon a stalker to hunt who?\n\r", caster );
      return fERROR;
   }

   victim = ent_get_char_world( caster, target_name );

   if ( victim == NULL )
   {
      send_to_ent( "Target can't be found!\n\r", caster );
      return fERROR;
   }

   if ( entComparar(caster, chToEnt(victim)) )
   {
      send_to_ent( "That WOULDN'T be wise!\n\r", caster );
      return fERROR;
   }

   nact( "$n calls upon the dark powers to summon forth a Stalker!", caster, NULL, NULL, TO_ROOM );
   send_to_ent( "You call upon the dark powers to summon forth a Stalker!\n\r", caster );

   stalker = create_mobile( get_mob_index( MOB_VNUM_STALKER ) );

   char_to_room( stalker, entWhereIs(caster) );
   act( "$n appears before you suddenly.", stalker, NULL, NULL, TO_ROOM );

   setNivelPr(stalker,getNivelPr(victim));
   stalker->max_hit	= victim->max_hit;
   stalker->hit		= victim->max_hit;
   stalker->exp		= getNivelPr(victim)*10;
   stalker->timer	= 10;

   if ( can_hunt( stalker, victim, TRUE ) )
   {
	act( "$n sniffs the air in search of $s prey.", stalker, NULL, NULL, TO_ROOM );
	set_hunt( stalker, victim, TRUE );
   }
   else
   {
	(*skill_table[sn].spell_fun) (sn, getNivelPr(stalker), chToEnt(stalker), caster, TARGET_CHAR );

	do_say( stalker, "How dare you waste my time!!" );
	act( "$n returns to the dark planes, vanishing suddenly!", stalker, NULL, NULL, TO_ROOM );
	extract_char( stalker, TRUE );
	return fFAIL;
   }

   return fOK;
}

SPELL_FUN_DEC(spell_hellspawn)
{
   /* High level mag / psi spell. -S- */
   CHAR_DATA *victim;
   int dam;

   CHECKENT()

   dam = number_range( level * 2, level * 6 );

   if (ent_saves_spell( level, ent, DAM_NEGATIVE ) )
      dam /= 2;

   act( "The Dark Powers of the HellSpawn strike $n!!", victim, NULL, NULL, TO_ROOM );
   send_to_char( "The Dark Powers of the HellSpawn strike you!!\n\r", victim );
   return newdamage( caster, victim, dam, sn, DAM_NEGATIVE, TRUE );
}

void room_fire_effect( EVENT *ev )
{
	ROOM_INDEX_DATA *room = ev->item.room;
	CHAR_DATA *ch, *ch_next;
	int dam;

	if (!IS_SET(room->room_flags, ROOM_FLAMING)
	||   IS_SET(room->room_flags, ROOM_PROTOTIPO))
		return;

	for ( ch = room->people; ch; ch = ch_next )
	{
		ch_next = ch->next_in_room;
		if ( !ES_IMMUNE(ch, IMM_FIRE) && !is_affected(ch,skill_lookup("fireproof")) )
		{
			send_to_char( "Las llamas te rodean...tu piel arde!\n\r", ch );
			fire_effect( ch, getNivelPr(ch), (dam = dice(5,6)), TARGET_CHAR );
			damage( ch, ch, dam, 0, DAM_FIRE, FALSE );
		}
		if ( !char_died(ch) && IS_NPC(ch) )
		{
			int blah = mob_best_door(ch);

			if (blah != -1)
				move_char( ch, blah, FALSE );
		}
	}

	if ( IS_SET(room->room_flags, ROOM_FLAMING) )
		room_event_add( room, 4*PULSE_PER_SECOND, 0, room_fire_effect );
}

void raise_affect( EVENT *ev )
{
	ROOM_INDEX_DATA *room = ev->item.room;
	AFFECT_DATA af;
	int level = (int) ev->param;

	if ( IS_SET(room->room_flags, ROOM_FLAMING) )
		return;

	af.where	= TO_ROOM_AFF;
	af.type		= skill_lookup( "raise fire" );
	af.level	= level;
	af.duration	= level / 10;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= ROOM_FLAMING;
	affect_to_room( room, &af );

	send_to_room( "El cuarto arde en llamas.", room );
	room_event_add( room, 4*PULSE_PER_SECOND, 0, room_fire_effect );
}

SPELL_FUN_DEC(spell_raise_fire)
{
	if ( IS_SET(entWhereIs(caster)->room_flags, ROOM_SAFE)
	|| IS_SET(entWhereIs(caster)->room_flags, ROOM_LAW) )
	{
		send_to_ent( "No puedes hacer eso aqui.\n\r", caster );
		return fERROR;
	}

	nact( "Alzas tus manos e invocas las llamas del averno.", caster, NULL, NULL,  TO_CHAR );
	nact( "$n alza sus manos e invoca las llamas del averno.", caster, NULL, NULL, TO_ROOM );

	room_event_add( entWhereIs(caster), skill_table[sn].beats, (void *) level, raise_affect );
	return fOK;
}

void cone_msg( EVENT *ev )
{
	ROOM_INDEX_DATA *room = ev->item.room;
	AFFECT_DATA af;
	int level = (int) ev->param;

	if ( IS_SET(room->room_flags, ROOM_CONE_OF_SILENCE) )
		return;

	af.where	= TO_ROOM_AFF;
	af.type		= skill_lookup( "cone of silence" );
	af.level	= level;
	af.duration	= level / 10;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= ROOM_CONE_OF_SILENCE;
	affect_to_room( room, &af );

	if ( room->people )
		act( "Un cono de silencio cae sobre el cuarto.", room->people, 0, 0, TO_ALL );
}

SPELL_FUN_DEC(spell_cone_of_silence)
{
	if ( IS_SET(entWhereIs(caster)->room_flags, ROOM_SAFE)
	  || IS_SET(entWhereIs(caster)->room_flags, ROOM_LAW) )
	{
		send_to_ent( "No puedes hacer eso aqui.\n\r", caster );
		return fERROR;
	}

	nact( "$n recita unas palabras en un idioma incomprensible.", caster, NULL, NULL, TO_ROOM );
	nact( "Recitas las palabras ocultas.", caster, NULL, NULL, TO_CHAR );

	room_event_add( entWhereIs(caster), skill_table[sn].beats, (void *) level, cone_msg );
	if ( entEsCh(caster) )
	{
		char_event_add( entGetCh(caster), skill_table[sn].beats, (void *) "Terminas de recitar.\n\r", stc_event );
		char_event_add( entGetCh(caster), skill_table[sn].beats, (void *) "$n termina de recitar.", to_room_act_event );
	}
	return fOK;
}

SPELL_FUN_DEC(spell_acid_arrow)
{
	CHAR_DATA *victim;
	int dam;

	CHECKENT()

	dam = dice(2,4) + level;

	if ( ent_saves_spell( level, ent, DAM_ACID ) )
		dam /= 2;

	nact( "$n te lanza una flecha acida!", caster, NULL, ent, TO_VICT );
	nact( "$n lanza una flecha acida hacia $N.", caster, NULL, ent, TO_NOTVICT );
	nact( "Lanzas una flecha acida hacia $N!", caster, NULL, ent, TO_CHAR );
	
	acid_effect( victim, level, DAM_ACID, TARGET_CHAR );
	return newdamage( caster, victim, dam, sn, DAM_ACID, TRUE );
}

void ehealth_event( EVENT *ev )
{
	CHAR_DATA *victim = ev->item.ch;
	CHAR_DATA *ch = (long) (ev->param > 0) ? get_char_from_id( (long) ev->param ) : victim;
	AFFECT_DATA af;

	if ( (ch == NULL) || (ch->in_room != victim->in_room) )
		return;

	if (is_affected(victim, gsn_enhance_health))
	{
		if ( ch != victim )
			act( "$N ya se siente #BPODEROS$t#b.", ch, strToEnt(USTRSEX(victim),ch->in_room), chToEnt(victim), TO_CHAR );
		else
			send_to_char( "Ya te sientes #BPODEROSO#b.\n\r", ch );
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= gsn_enhance_health;
	af.level	= getNivelPr(ch);
	af.duration	= getNivelPr(ch) / 2;
	af.modifier	= getNivelPr(ch) + getNivelPr(victim);
	af.location	= APPLY_HIT;
	af.bitvector	= 0;
	af.caster_id	= ch->id;
	affect_to_char( victim, &af );

	act( "Te sientes mas #BPODEROS$t#b.", victim, strToEnt(USTRSEX(victim),victim->in_room), NULL, TO_CHAR );
	act( "$n se siente mas #BPODEROS$t#b.", victim, strToEnt(USTRSEX(victim),victim->in_room), NULL, TO_ROOM );

	return;
}

SPELL_FUN_DEC(spell_enhance_health)
{
	CHAR_DATA *victim;
	Entity * blah;

	CHECKENT()

	if ( !entComparar(caster, ent) )
	{
		nact( "$n mira a $N y empieza a hacer movimientos extranos con las manos.", caster, NULL, ent, TO_NOTVICT );
		nact( "$n te mira y empieza a hacer movimientos extranos con las manos.", caster, NULL, ent, TO_VICT );
		nact( "Miras a $N y empiezas a hacer movimientos extranos con las manos.", caster, NULL, ent, TO_CHAR );
	}
	else
	{
		nact( "$n empieza a hacer movimientos extranos con las manos.", caster, NULL, NULL, TO_ROOM );
		nact( "Empiezas a hacer movimientos extranos con las manos.", caster, 0, 0, TO_CHAR );
	}

	blah = entCopiar( caster );

	ent_event_add( blah, skill_table[sn].beats, (void *) "Tus movimientos terminan.\n\r", ent_stc_event );
	ent_event_add( blah, skill_table[sn].beats + 1, NULL, ent_free_event );

	char_event_add( victim, skill_table[sn].beats, (void *) entGetId(caster), ehealth_event );

	return fOK;
}

void emana_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	AFFECT_DATA af;

	if (is_affected(ch, gsn_concentracion))
	{
		send_to_char( "Ya estas concentrado.\n\r", ch );
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= gsn_concentracion;
	af.level	= getNivelPr(ch);
	af.duration	= getNivelPr(ch);
	af.modifier	= 10 + getNivelPr(ch) * 2;
	af.location	= APPLY_MANA;
	af.bitvector	= 0;
	af.caster_id	= ch->id;
	affect_to_char( ch, &af );
	send_to_char( "Te concentras al maximo.\n\r", ch );
	act( "$n se concentra al maximo.", ch, 0, 0, TO_ROOM );
	return;
}

SPELL_FUN_DEC(spell_concentracion)
{
	nact( "$n empieza a concentrarse.", ent ? ent : caster, NULL, NULL, TO_ROOM );
	nact( "Empiezas a concentrarte.", ent ? ent : caster, 0, 0, TO_CHAR );

	if ( entEsCh(caster) )
		char_event_add( entGetCh(caster), skill_table[sn].beats, 0, emana_event );
	else
	if ( entEsCh(ent) )
		char_event_add( entGetCh(ent), skill_table[sn].beats, 0, emana_event );

	return fOK;
}

SPELL_FUN_DEC(spell_estupidez)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int mod1, mod2, modx;

	CHECKENT()

	if ( IS_AFFECTED2( victim, AFF_ESTUPIDEZ ) )
	{
		if ( !entComparar(caster, ent) )
			nact( "$N ya es un estupido.", caster, NULL, ent, TO_CHAR );
		else
			send_to_ent( "Ya eres un estupido.\n\r", caster );
		return fERROR;
	}

	if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
	{
		send_to_char( "Te sientes un poco mas tonto, pero te recuperas.\n\r", victim );
		act( "$n se pone un poco mas tonto, pero se recupera.", victim, 0, 0, TO_ROOM );
		return fFAIL;
	}

	mod1 = 10 - get_curr_stat( victim, STAT_INT );
	mod2 = - (level / 3);

	modx = mod1 < mod2 ? mod2 : mod1;

	if ( modx >= 0 )
		modx = -1;

	af.type		= sn;
	af.where	= TO_AFFECTS2;
	af.level	= level;
	af.duration	= level;
	af.modifier	= modx;
	af.location	= APPLY_INT;
	af.bitvector	= AFF_ESTUPIDEZ;
	affect_to_ent( ent, &af );

	nact( "$N empieza a babear.", caster, NULL, ent, TO_NOTVICT );
	if (!entComparar(caster, ent))
		nact( "$N empieza a babear.", caster, NULL, ent, TO_CHAR );
	nact( "Ahhhh....peleeeaa....cerveza....", caster, NULL, ent, TO_VICT );

	return fOK;
}

/* Flame shield spell from Malice of EnvyMud */
SPELL_FUN_DEC(spell_flaming_shield)
{
    AFFECT_DATA af;

    if ( ent_IS_AFFECTED2( ent, AFF_FLAMING_SHIELD ) )
    {
    	if ( entComparar(ent, caster) )
		send_to_ent( "El fuego ya te rodea.\n\r", caster );
	else
		nact( "El fuego ya rodea a $N.", caster, NULL, ent, TO_CHAR );
        return fERROR;
    }

    af.type      = sn;
    af.level	 = level;
    af.where	 = TO_AFFECTS2;
    af.duration  = number_fuzzy( level / 8 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FLAMING_SHIELD;
    affect_to_ent( ent, &af );

    send_to_ent( "Un escudo de fuego te rodea.\n\r", ent );
    nact( "$n es rodeado por un escudo de fuego.",
	ent, NULL, NULL, TO_ROOM );

    return fOK;
}

SPELL_FUN_DEC(spell_random_bueno)
{
	int uno = 0, temp = -1, num;
	char *spell = NULL;

	while( CHANCE(100 - uno*25) )
	{
		num = number_range(0,10);

		switch(num)
		{
			case 0:	spell = "cure light";		break;
			case 1: spell = "cure serious";		break;
			case 2: spell = "cure critical";	break;
			case 3: spell = "cure blindness";	break;
			case 4: spell = "cure disease";		break;
			case 5: spell = "cure poison";		break;
			case 6: spell = "heal";			break;
			case 7: spell = "sanctuary";		break;
			case 8: spell = "haste";		break;
			case 9: spell = "giant strength";	break;
			case 10: spell = "armor";		break;
		}

		temp = skill_lookup( spell );

		if ( temp > 0 )
			return (*skill_table[temp].spell_fun) (temp,level,caster,caster,TARGET_CHAR);

		uno++;
	}

	return fFAIL;
}

SPELL_FUN_DEC(spell_alucinar)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
		return fFAIL;

	send_to_char( "Uhhh...sientes que empiezas a flotar!\n\r", victim );
	act( "$n pone cara de volado.", victim, NULL, NULL, TO_ROOM );

	return fOK;
}

SPELL_FUN_DEC(spell_power_word_of_kill)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( entComparar(caster, ent) )
	{
		send_to_ent( "No seas ridiculo!\n\r", caster );
		return fERROR;
	}

	nact( "Invocas el poder de $t para destruir a $N!", caster, strToEnt(entGetClanGod(caster),entWhereIs(caster)), ent, TO_CHAR );
	nact( "$n invoca el poder de $t para destruirte!", caster, strToEnt(entGetClanGod(caster),entWhereIs(caster)), ent, TO_VICT );
	nact( "$n invoca el poder de $t para destruir a $N!", caster, strToEnt(entGetClanGod(caster),entWhereIs(caster)), ent, TO_NOTVICT );

	if ( !ent_is_npc(ent) || ent_saves_spell( level, ent, DAM_HOLY ) )
	{
		send_to_ent( "Nada pasa.\n\r", caster );
		return fFAIL;
	}

	act( "El poder de $t #BDESTRUYE#b a $n.", victim, strToEnt(entGetClanGod(caster),victim->in_room), NULL, TO_ROOM );
	extract_char( victim, TRUE );

	return victdead;
}

SPELL_FUN_DEC(spell_regeneracion)
{
	AFFECT_DATA af;

	if ( ent_is_affected(ent, sn) || ent_IS_AFFECTED(ent, AFF_REGENERATION) )
	{
		if (entComparar(caster,ent))
			send_to_ent( "Ya te estas regenerando mas rapidamente.\n\r", caster );
		else
			nact( "$N ya se esta regenerando rapidamente.", caster, NULL, ent, TO_CHAR );
		return fERROR;
	}

	af.where	= TO_AFFECTS;
	af.level	= level;
	af.type		= sn;
	af.bitvector	= AFF_REGENERATION;
	af.duration	= 2 + level / 2;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_ent( ent, &af );

	send_to_ent( "Tu metabolismo se acelera.\n\r", ent );
	nact( "Las heridas de $n cicatrizan mas rapidamente.", ent, NULL, NULL, TO_ROOM );
	return fOK;
}

SPELL_FUN_DEC(spell_web)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	CHECKENT()

	if ( IS_AFFECTED2(victim, AFF_HOLD) || ent_is_affected(ent, sn) )
	{
		nact( "$N ya esta enredado.", caster, 0, ent, TO_CHAR );
		return fERROR;
	}

	if ( ent_saves_spell( level, ent, DAM_OTHER) )
	{
		send_to_ent( "Nada sucede.\n\r", caster );
		return fFAIL;
	}

	af.where	= TO_AFFECTS2;
	af.type		= sn;
	af.level	= level;
	af.bitvector	= AFF_HOLD;
	af.duration	= 1 + level / 8;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	affect_to_ent( ent, &af );

	act( "Una telarana magica cae sobre $n!", victim, 0, 0, TO_ROOM );
	act( "Una telarana magica cae sobre ti!", victim, 0, 0, TO_CHAR );

	return fOK;
}

SPELL_FUN_DEC(spell_cansancio)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( ent_saves_spell( level, ent, DAM_MENTAL ) )
		return fFAIL;

	send_to_char( "Te estas quedando dormido.\n\r", victim );
	act( "$n empieza a cabecear.", victim, NULL, NULL, TO_ROOM );

	return fOK;
}

SPELL_FUN_DEC(spell_polymorph)
{
	Entity * victim;
	bool found = FALSE;
	int race = 0;
	char raza[MIL];
	AFFECT_DATA af;

	if ( !IS_NULLSTR(target_name) )
	{
		target_name = one_argument( target_name, raza );

		race = race_lookup( raza );

		if ( race == 0 )
		{
			send_to_ent( "Esa raza no existe.\n\r", caster );
			return fERROR;
		}

		if ( !IS_NULLSTR(target_name) )
			victim = chToEnt(ent_get_char_room( caster, target_name ));
		else
			victim = caster;
	}
	else
		victim = caster;

	if ( victim == NULL )
	{
		send_to_ent( "No esta aqui.\n\r", caster );
		return fERROR;
	}

	if ( ent_IS_AFFECTED(victim, AFF_POLYMORPH) )
	{
		if ( entComparar(caster, victim) )
			send_to_ent( "Ya tienes otra forma.\n\r", caster );
		else
			nact( "$n ya tiene otra forma.", victim, NULL, NULL, TO_CHAR );
		return fERROR;
	}

	if ( !entComparar(caster, victim) )
		if ( ent_saves_spell( level, victim, DAM_DISEASE ) )
		{
			send_to_ent( "Nada pasa.\n\r", caster );
			return fFAIL;
		}

	if ( (entComparar(caster, victim) && !ent_saves_spell( level, caster, DAM_DISEASE ))
	||    race == 0 )
		while ( found == FALSE )
		{
			race = number_range( 1, maxrace );

			if ( !race_table[race].pc_race
			||    race_table[race].remort_race
			||    race == entGetRace(victim) )
				continue;

			found = TRUE;
		}

	if ( race == entGetRace(victim) )
	{
		send_to_ent( "Fallaste.\n\r", caster );
		return fERROR;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level;
	af.modifier	= race - entGetRace(victim);
	af.location	= APPLY_RACE;
	af.bitvector	= AFF_POLYMORPH;
	affect_to_ent( victim, &af );

	nact( "$n se convierte en $t!", victim, strToEnt(race_table[race].name,entWhereIs(victim)), NULL, TO_ROOM );
	nact( "Te sientes extran$o.", victim, NULL, NULL, TO_CHAR );
	return fOK;
}

SPELL_FUN_DEC(spell_respirar_agua)
{
	AFFECT_DATA af;

	if (ent_is_affected(ent, sn)
	||  ent_is_part(ent, PART_AGALLAS) )
	{
		if (entComparar(caster, ent))
			send_to_ent( "Ya puedes respirar agua.\n\r", ent );
		else
			nact( "$N ya puede respirar agua.", caster, ent, NULL, TO_CHAR );
		return fFAIL;
	}

	af.where	= TO_PARTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= PART_AGALLAS;
	affect_to_ent( ent, &af );

	if (!entComparar(caster, ent))
		send_to_ent( "Ok.\n\r", caster);

	send_to_ent( "Sientes que el agua puede fluir por tus pulmones.\n\r", ent );
	return fOK;
}
