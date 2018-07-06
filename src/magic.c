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
#include "recycle.h"
#include "events.h"
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

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_visible	);
DECLARE_DO_FUN(do_mproomhunt	);

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );

int skill_level( CHAR_DATA *ch, int sn )
{
	SpellDesc spell;

	if ( IS_NPC(ch) )
	{
		if ( IS_MOB_CASTER(ch) )
			return getNivelPr(ch);
		else
			return getNivelPr(ch) / 2;
	}

	charTieneSn(&spell, ch, sn);

	if (spell.nivel < 1)
		return 1;

	if (spell.clase == CLASS_RANGER )
		return URANGE(1, (spell.nivel - 25) * 2, LEVEL_HERO);

	if ( class_table[spell.clase].fMana )
		return spell.nivel;

	return spell.nivel/2;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn,valor;
    struct skhash *skill;
    extern struct skhash *skill_hash_table[26];

    if ( IS_NULLSTR(name) || ( ( valor = (LOWER(name[0]) - 'a') ) < 0 || valor > 25 ) )
    	return -1;

    for ( skill = skill_hash_table[valor]; skill; skill = skill->next )
    {
	sn = skill->sn;

	if ( (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ))
	||   (!IS_NULLSTR(skill_table[sn].nombre)
	  &&   LOWER(name[0]) == LOWER(skill_table[sn].nombre[0])
	  &&  !str_prefix(name, skill_table[sn].nombre) ) )
	    return sn;
    }

    return -1;
}

SpellDesc * find_spell( SpellDesc * spell, CHAR_DATA *ch, char *name )
{
    /* finds a spell the character can cast if possible */
    int sn;

    if (IS_NPC(ch))
    {
    	spell->sn	= skill_lookup(name);
    	spell->nivel	= getNivelPr(ch);
    	spell->clase	= getClasePr(ch);
	return spell;
    }

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;

	if ( ( (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	   &&  is_name_order(name,skill_table[sn].name) )
	 ||  (!IS_NULLSTR(skill_table[sn].nombre)
	  &&  LOWER(name[0]) == LOWER(skill_table[sn].nombre[0])
	  &&  is_name_order(name,skill_table[sn].nombre)) ) )
		return charTieneSn(spell, ch, sn);
    }

    return blanquear_spelldesc(spell);
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = NOMBRE_SKILL(sn); *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n murmura las palabras, '%s'.", buf );
    sprintf( buf,  "$n murmura las palabras, '%s'.", NOMBRE_SKILL(sn) );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act((!IS_NPC(rch) && es_clase(ch, getClasePr(rch))) ? buf : buf2,
	        ch, NULL, chToEnt(rch), TO_VICT );
    }

    return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    if (victim == NULL)
    	return TRUE;

    save = 50 + (getNivelPr(victim) - level) * 5 - victim->saving_throw * 2;
/*  save = 50 + (getNivelPr(victim) - level) * 6 - victim->saving_throw * 2; */

/*  if (IS_AFFECTED(victim,AFF_BERSERK))
	save += getNivelPr(victim)/4; */

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save *= 2;	break;	/* +2? */
	case IS_VULNERABLE:	save /= 2;	break;	/* -2? */
    }

    if (!IS_NPC(victim) && class_table[getClasePr(victim)].fMana)
	save = 9 * save / 10;

    save += victim->luck / 10;

    save = URANGE( 5, save, 95 );

    return number_percent( ) < save;
}

/* RT save for dispels */
bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;	/* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;

    save = URANGE( 5, save, 95 );

    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( !IS_NULLSTR(skill_table[sn].msg_off) )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    if ( !IS_NULLSTR(skill_table[sn].msg_room) )
		    	act( skill_table[sn].msg_room, victim, NULL, NULL, TO_ROOM );
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
/* int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (getNivelPr(ch) + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + getNivelPr(ch) - level)));
} */

int mana_cost( int sn, int level, int clase )
{
    if (level + 2 == skill_table[sn].skill_level[clase])
	return 50;
    else
	return UMAX( skill_table[sn].min_mana,
		100 / ( 2 + level - skill_table[sn].skill_level[clase] ) );
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    Entity * ent;
    int mana;
    int target;
    extern bool order_mob;
    char *fullarg = argument;
    SpellDesc spell;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch)
    &&   order_mob == TRUE )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
    {
    	send_to_char( "La magia no existe en este lugar.\n\r", ch );
    	return;
    }

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Castear que cuando donde?\n\r", ch );
	return;
    }

    find_spell(&spell, ch, arg1);

    if (spell.sn < 1
    ||  skill_table[spell.sn].spell_fun == spell_null )
    {
	if ( IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) )
		bugf( "do_cast : spell %s no encontrado", fullarg );
	send_to_char( "No conoces ningun hechizo con ese nombre.\n\r", ch );
	return;
    }

    if ( ch->position < skill_table[spell.sn].minimum_position )
    {
	send_to_char( "No puedes concentrarte lo suficiente.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   !CAN_SAY(ch)
    &&   !IS_SET(skill_table[spell.sn].flags, SKILL_NOSOUND) )
    {
	send_to_char( "No puedes...estas mudo!\n\r", ch );
	return;
    }

    if (spell.nivel + 2 == skill_table[spell.sn].skill_level[spell.clase])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[spell.sn].min_mana,
	    100 / ( 2 + spell.nivel - skill_table[spell.sn].skill_level[spell.clase] ) );

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    ent		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[spell.sn].target )
    {
    default:
	bugf( "Do_cast: bad target for sn %d, char %s, vnum %d, id %ld, skill %s.",
		spell.sn, ch->name, CHARVNUM(ch), ch->id, skill_table[spell.sn].name );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "A quien deseas hechizar?\n\r", ch );
	    if ( IS_NPC(ch) && ch->desc )
	    	printf_to_char( ch, "Spell : %s\n\r", skill_table[spell.sn].name );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "No esta aqui.\n\r", ch );
		return;
	    }
	}

	if (is_safe(ch,victim) && victim != ch)
	{
		send_to_char("No a esa victima.\n\r",ch);
		return;
	}

	check_killer(ch,victim);

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    act( "$N es tu amad$O maestr$O.", ch, NULL, chToEnt(victim), TO_CHAR );
	    return;
	}

	if ( es_kill_steal(ch, victim, TRUE) )
		return;

	ent = chToEnt(victim);
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "No esta aqui.\n\r", ch );
		return;
	    }
	}

	ent = chToEnt(victim);
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "Este hechizo no lo puedes lanzar hacia otra persona.\n\r", ch );
	    if ( IS_NPC(ch) && ch->desc )
	    	printf_to_char( ch, "Spell : %s\n\r", skill_table[spell.sn].name );
	    return;
	}

	ent = chToEnt(ch);
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Que deseas hechizar?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "No estas llevando eso.\n\r", ch );
	    if ( IS_NPC(ch) && ch->desc )
	    	printf_to_char( ch, "Spell : %s\n\r", skill_table[spell.sn].name );
	    return;
	}

	ent = objToEnt(obj);
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("A que o quien deseas hechizar?\n\r",ch);
		if ( IS_NPC(ch) && ch->desc )
	    		printf_to_char( ch, "Spell : %s\n\r", skill_table[spell.sn].name );
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("No a esa victima.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
		act( "No puedes hacerle eso a tu maestr$O.",
			ch, NULL, chToEnt(victim), TO_CHAR );
                return;
            }

	    if (!IS_NPC(ch))
		check_killer(ch,victim);

	    ent = chToEnt(victim);
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    ent = objToEnt(obj);
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            ent = chToEnt(ch);
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
            ent = chToEnt(victim);
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    ent = objToEnt(obj);
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("No ves eso aqui.\n\r",ch);
	    return;
	}
	break;
    }
	    
    if ( ch->mana < mana )
    {
	send_to_char( "No tienes suficiente mana.\n\r", ch );
	return;
    }
      
    if ( str_cmp( skill_table[spell.sn].name, "ventriloquate" )
      && str_cmp( skill_table[spell.sn].name, "create sound" ) )
	say_spell( ch, spell.sn );
      
    WAIT_STATE( ch, skill_table[spell.sn].beats );

    if ( number_percent( ) > get_skill(ch,spell.sn) )
    {
	send_to_char( "Perdiste la concentracion.\n\r", ch );
	check_improve(ch,spell.sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
	FRetVal retval;

        ch->mana -= mana;

	retval = (*skill_table[spell.sn].spell_fun) (spell.sn, skill_level(ch, spell.sn), chToEnt(ch), ent, target);

	if (!char_died(ch))
	{
		if (retval == fERROR 
		&&   IS_NPC(ch)
		&&  !IS_SET(ch->act, ACT_IS_HEALER) )
			bugf( "do_cast : spell %s retorno fERROR, char %s(%d), arg %s, room %d",
				skill_table[spell.sn].name,
				ch->name, CHARVNUM(ch),
				fullarg, ch->in_room->vnum );
		check_improve(ch,spell.sn,TRUE,1);
	}
    }

    if ( (skill_table[spell.sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[spell.sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&  !char_died(ch)
    &&  !char_died(victim)
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;

	    if ( victim == vch
	    &&   victim->fighting == NULL )
	    {
		check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, Entity * caster, CHAR_DATA *ch, Entity * enttarget )
{
    Entity * ent;
    int target = TARGET_NONE;
    CHAR_DATA *victim = NULL;

    if ( sn <= 0 || ent_died(caster) || char_died(ch) || ent_died(enttarget) )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
    	victim = NULL;
	ent = NULL;
	caster = chToEnt(ch);
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( !entidadEsCh(enttarget) )
	{
		send_to_char( "No puedes hacer eso.\n\r", ch );
		return;
	}
	victim = entidadGetCh(enttarget);
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "No puedes hacer eso.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}

	if ( es_kill_steal(ch, victim, TRUE) )
		return;

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
		act( "$N es tu amado maestro.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	ent = chToEnt(victim);
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( enttarget == NULL )
	    enttarget = chToEnt(ch);

	ent = enttarget;
	victim = entEsCh(ent) ? entGetCh(ent) : NULL;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( !entEsObj(enttarget) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	ent = enttarget;
	target = TARGET_OBJ;
	victim = NULL;
	break;

    case TAR_OBJ_CHAR_OFF:
	if ( enttarget == NULL )
	{
		if ( ch->fighting != NULL )
			enttarget = chToEnt(ch->fighting);
		else
		{
			send_to_char( "No puedes hacer eso.\n\r", ch );
			return;
		}
	}

	if (entEsCh(enttarget))
	{
		victim = entGetCh(enttarget);

		if ( es_kill_steal(ch, victim, TRUE) )
			return;

		if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
		{
			act( "$N is your beloved master.", ch, NULL, chToEnt(victim), TO_CHAR );
			return;
		}

		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		ent = enttarget;
		target = TARGET_CHAR;
	}
	else
	if ( entEsObj(enttarget) )
	{
	    	ent = enttarget;
	    	target = TARGET_OBJ;
	    	victim = NULL;
	}
	else
	{
		send_to_char( "Algo esta mal.\n\r", ch );
		return;
	}
        break;

    case TAR_OBJ_CHAR_DEF:
	if (enttarget == NULL)
	{
		ent = chToEnt(ch);
		target = TARGET_CHAR;
		victim = ch;
	}
	else if (entEsCh(enttarget))
	{
	    ent = enttarget;
	    target = TARGET_CHAR;
	    victim = entGetCh(enttarget);
	}
	else if (entEsObj(enttarget))
	{
	    ent = enttarget;
	    target = TARGET_OBJ;
	    victim = NULL;
	}
	else
	{
		send_to_char( "Algo esta mal.\n\r", ch );
		return;
	}
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, caster, ent, target );

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != NULL
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}

/*
 * Spell functions.
 */
SPELL_FUN_DEC(spell_acid_blast)
{
    CHAR_DATA *victim;
    int dam;

    CHECKENT()

    dam = dice( level, 12 );

    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;

    return newdamage( caster, victim, dam, sn, DAM_ACID, TRUE);
}

SPELL_FUN_DEC(spell_armor)
{
    AFFECT_DATA af;

    if ( ent_is_affected( ent, sn ) )
    {
	if (entComparar(caster, ent))
	  send_to_ent("Ya te rodea una armadura magica.\n\r", ent);
	else
	  nact("$N ya fue rodead$O por una armadura magica.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_ent( "Sientes que alguien te protege.\n\r", ent );

    if ( !entComparar(ent, caster) )
	nact("$N es protegid$O por tu magia.", caster, NULL, ent, TO_CHAR);

    return fOK;
}

SPELL_FUN_DEC(spell_bless)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (entidadEsObj(ent))
    {
	obj = entidadGetObj(ent);

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    nact("$p ya fue bendito.",caster,ent,NULL,TO_CHAR);
	    return fERROR;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		nact("$p brilla con un tono azul palido.",caster,ent,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return fOK;
	    }
	    else
	    {
		nact("The evil of $p is too powerful for you to overcome.",
		    caster,ent,NULL,TO_CHAR);
		return fFAIL;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 6 + level;
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	nact("$p glows with a holy aura.",caster,ent,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE )
		entSetSavingThrow(caster, entGetSavingThrow(caster) - 1);

	return fOK;
    }

    /* character target */
    CHECKENT()

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (entComparar(caster, ent))
		send_to_ent("You are already blessed.\n\r",caster);
	else
		nact("$N already has divine favor.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );

    if ( !entComparar(caster, ent) )
	nact("You grant $N the favor of your god.",caster,NULL,ent,TO_CHAR);

    return fOK;
}

SPELL_FUN_DEC(spell_blindness)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_BLIND) )
    {
	nact( "$E ya esta cieg$O.", caster, NULL, ent, TO_CHAR );
	return fERROR;
    }

    if ( saves_spell(level,victim,DAM_LIGHT) )
    {
    	send_to_ent( "No pasa nada.\n\r", caster );
    	return fFAIL;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );

    if ( ent_is_part(caster, PART_HANDS)
    &&  !entComparar(caster, ent) )
    {
	nact( "$n extiende su mano hacia ti y una potente luz sale de ella.", caster, NULL, ent, TO_VICT );
	nact( "Extiendes tu mano hacia $N y una potente luz sale de ella.", caster, NULL, ent, TO_CHAR );
    }


    nact( "Estas cieg$o!\n\r", ent, NULL, NULL, TO_CHAR );
    nact( "$n parece estar cieg$o.", ent,NULL,NULL,TO_ROOM);

    if (CHANCE(10))
    	nact( "$n grita 'DIOS MIO! He quedado cieg$o!'", ent, NULL, NULL, TO_ROOM );

    return fOK;
}

SPELL_FUN_DEC(spell_burning_hands)
{
    CHAR_DATA *victim;

    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48
    };
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );

    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;

    return newdamage( caster, victim, dam, sn, DAM_FIRE,TRUE);
}

SPELL_FUN_DEC(spell_call_lightning)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    CHAR_DATA *ch;

    CHECKCH()

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return fERROR;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return fERROR;
    }

    dam = dice(level/2, 8);

    send_to_char( "Mota's lightning strikes your foes!\n\r", ch );
    nact( "$n calls Mota's lightning to strike $s foes!",
	caster, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;

	if ( vch->in_room == NULL )
	    continue;

	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		newdamage( caster, vch, saves_spell(level, vch, DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return fOK;
}

/* RT calm spell stops all fighting in the room */

SPELL_FUN_DEC(spell_calm)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;
    CHAR_DATA *ch;

    CHECKCH()

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += getNivelPr(vch);
	    else
	      mlevel += getNivelPr(vch)/2;
	    high_level = UMAX(high_level,getNivelPr(vch));
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      continue;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,gsn_frenzy))
	      continue;

	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);

	    af.where	= TO_AFFECTS;
	    af.type	= sn;
  	    af.level	= level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    af.caster_id = entGetId(caster);
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_cancellation)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    bool found = FALSE;

    CHECKCH()
    CHECKENT()

    level += 2;

    if ( ch != victim
    &&  !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  (IS_NPC(victim) || (is_clan(ch) && is_clan(victim) && !is_same_clan(ch, victim))) )
    {
    	send_to_char( "Fallaste. Intenta dispel magic.\n\r", ch );
    	return fFAIL;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,gsn_sanctuary))
        found = TRUE;

    if (check_dispel(level,victim,gsn_armor))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_bless))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_blindness))
        found = TRUE;

    if (check_dispel(level,victim,gsn_calm))
 	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("change sex")))
        found = TRUE;

    if (check_dispel(level,victim,gsn_charm_person))
        found = TRUE;

    if (check_dispel(level,victim,gsn_domination))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_chill_touch))
        found = TRUE;

    if (check_dispel(level,victim,gsn_fireproof))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_curse))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_mystic_armor))
	found = TRUE;

    if (check_dispel(level,victim,gsn_detect_evil))
        found = TRUE;

    if (check_dispel(level,victim,gsn_detect_good))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_magic))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_combat_mind))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_amnesia))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_estupidez))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_faerie_fire))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_flaming_shield))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_fly))
        found = TRUE;

    if (check_dispel(level,victim,gsn_frenzy))
	found = TRUE;
 
    if (check_dispel(level,victim,gsn_giant_strength))
        found = TRUE;

    if (check_dispel(level,victim,gsn_haste))
	found = TRUE;
 
    if (check_dispel(level,victim,gsn_infravision))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_mass_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_pass_door))
        found = TRUE;

    if (check_dispel(level,victim,gsn_protect_evil))
        found = TRUE;

    if (check_dispel(level,victim,gsn_protect_good))
        found = TRUE; 
 
    if (check_dispel(level,victim,gsn_polymorph))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_shield))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_sleep))
        found = TRUE;

    if (check_dispel(level,victim,gsn_slow))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_stone_skin))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_weaken))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_mute))
        found = TRUE;

    if (check_dispel(level,victim,gsn_ego_whip))
	found = TRUE;

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Fallaste.\n\r",ch);

    return found == TRUE ? fOK : fFAIL;
}

SPELL_FUN_DEC(spell_cause_light)
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice(1, 8) + level / 3, sn,DAM_HARM,TRUE);
}

SPELL_FUN_DEC(spell_cause_critical)
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice(3, 8) + level - 6, sn,DAM_HARM,TRUE);
}

SPELL_FUN_DEC(spell_cause_serious)
{
    CHAR_DATA *victim;

    CHECKENT()

    return newdamage( caster, victim, dice(2, 8) + level / 2, sn,DAM_HARM,TRUE);
}

SPELL_FUN_DEC(spell_chain_lightning)
{
    CHAR_DATA *victim, *ch;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    CHECKCH()
    CHECKENT()

    /* first strike */

    nact("A lightning bolt leaps from $n's hand and arcs to $N.",
        caster,NULL,ent,TO_ROOM);
    nact("A lightning bolt leaps from your hand and arcs to $N.",
	caster,NULL,ent,TO_CHAR);
    nact("A lightning bolt leaps from $n's hand and hits you!",
	caster,NULL,ent,TO_VICT);  

    dam = dice(level,6);
    if (saves_spell(level,victim,DAM_LIGHTNING))
 	dam /= 2;
    newdamage(caster,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,6);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 2;
	    newdamage(caster,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */
	
	if (!found) /* no target found, hit the caster */
	{
	  if (char_died(ch))
     	    return chdead;

	  if (last_vict == ch) /* no double hits */
	  {
	    nact("The bolt seems to have fizzled out.",caster,NULL,NULL,TO_ROOM);
	    nact("The bolt grounds out through your body.",
		caster,NULL,NULL,TO_CHAR);
	    return fOK;
	  }

	  last_vict = ch;
	  nact("The bolt arcs to $n...whoops!",caster,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,6);
	  if (saves_spell(level,ch,DAM_LIGHTNING))
	    dam /= 2;
	  newdamage(caster,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (char_died(ch))
	  	return chdead;
	}
    /* now go back and find more targets */
    }

    return fOK;
}
	  
SPELL_FUN_DEC(spell_change_sex)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    AFFECT_DATA af;

    CHECKCH()
    CHECKENT()

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("Otra vez?\n\r",ch);
	else
	  nact("$N ya tiene su sexo cambiado.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    if (saves_spell(level , victim,DAM_OTHER))
	return fFAIL;	

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Te sientes diferente.\n\r", victim );
    act("$n no se ve como si mism$o...",victim,NULL,NULL,TO_ROOM);
    return fOK;
}

SPELL_FUN_DEC(spell_charm_person)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    AFFECT_DATA af;

    CHECKCH()
    CHECKENT()

    if (is_safe(ch,victim))
	return fERROR;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return fERROR;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < getNivelPr(victim)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_CHARM) )
	return fFAIL;

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return fERROR;
    }

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );

    nact( "Isn't $n just so nice?", caster, NULL, ent, TO_VICT );

    if ( ch != victim )
	nact("$N looks at you with adoring eyes.",caster,NULL,ent,TO_CHAR);

    return fOK;
}

SPELL_FUN_DEC(spell_chill_touch)
{
    CHAR_DATA *victim;

    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );

    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    return newdamage( caster, victim, dam, sn, DAM_COLD,TRUE );
}

SPELL_FUN_DEC(spell_colour_spray)
{
    CHAR_DATA *victim;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,	76, 77, 78, 79, 79
    };
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

    dam		= number_range( dam_each[level] / 2,  dam_each[level] * 2 );

    if ( saves_spell( level, victim,DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(gsn_blindness,
	    level/2,caster,ent,TARGET_CHAR);

    return newdamage( caster, victim, dam, sn, DAM_LIGHT,TRUE );
}

SPELL_FUN_DEC(spell_continual_light)
{
    OBJ_DATA *light;
    CHAR_DATA *ch;

    CHECKCH()

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name,ch);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return fERROR;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    nact("$p is already glowing.",caster,objToEnt(light),NULL,TO_CHAR);
	    return fERROR;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	nact("$p glows with a white light.",caster,objToEnt(light),NULL,TO_ALL);
	return fOK;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    nact( "$n twiddles $s thumbs and $p appears.",   caster, objToEnt(light), NULL, TO_ROOM );
    nact( "You twiddle your thumbs and $p appears.", caster, objToEnt(light), NULL, TO_CHAR );
    return fOK;
}

SPELL_FUN_DEC(spell_control_weather) 
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
    {
	send_to_ent ("Do you want it to get better or worse?\n\r", caster );
	return fERROR;
    }

    send_to_ent( "Ok.\n\r", ent );

    return fOK;
}

SPELL_FUN_DEC(spell_create_food)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;

    obj_to_room( mushroom, entWhereIs(caster) );

    nact( "$p suddenly appears.", caster, objToEnt(mushroom), NULL, TO_ROOM );
    nact( "$p suddenly appears.", caster, objToEnt(mushroom), NULL, TO_CHAR );

    return fOK;
}

SPELL_FUN_DEC(spell_create_rose)
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    nact("$n has created a beautiful red rose.",caster,objToEnt(rose),NULL,TO_ROOM);
    send_to_ent("You create a beautiful red rose.\n\r",caster);
    obj_to_ent(rose,caster);
    return fOK;
}

SPELL_FUN_DEC(spell_create_spring)
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, entWhereIs(caster) );

    nact( "$p flows from the ground.", caster, objToEnt(spring), NULL, TO_ROOM );
    nact( "$p flows from the ground.", caster, objToEnt(spring), NULL, TO_CHAR );

    return fOK;
}

SPELL_FUN_DEC(spell_create_water)
{
    OBJ_DATA *obj;
    CHAR_DATA *ch;
    int water;

    CHECKCH()
    CHECKOBJENT()

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return fERROR;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return fERROR;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	nact( "$p is filled.", caster, ent, NULL, TO_CHAR );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_cure_blindness)
{
    CHAR_DATA *victim;

    CHECKENT()

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (entComparar(caster, ent))
          send_to_ent("You aren't blind.\n\r",caster);
        else
          nact("$N doesn't appear to be blinded.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    	return fOK;

    return fFAIL;
}

SPELL_FUN_DEC(spell_cure_critical)
{
    int heal;

    heal = dice(3, 8) + level - 6;
    change_health( caster, ent, heal );
    ent_update_pos( ent );
    send_to_ent( "Te sientes mejor!\n\r", ent );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

/* RT added to cure plague */
SPELL_FUN_DEC(spell_cure_disease)
{
    CHAR_DATA *victim;

    CHECKENT()

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (entComparar(ent, caster))
	  nact("No estas enferm$o.", caster, NULL,NULL,TO_CHAR);
        else
          nact("$N no esta enferm$O.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }
    
    if (check_dispel(level,victim,gsn_plague))
	return fOK;

    send_to_ent("Fallaste.\n\r",caster);

    return fFAIL;
}

SPELL_FUN_DEC(spell_cure_light)
{
    int heal;

    heal = dice(1, 8) + level / 3;
    change_health( caster, ent, heal );
    ent_update_pos( ent );
    send_to_ent( "Te sientes mejor!\n\r", ent );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_cure_poison)
{
    CHAR_DATA *victim;
 
    CHECKENT()

    if ( !is_affected( victim, gsn_poison ) )
    {
        if (entComparar(caster, ent))
          nact( "No estas envenenad$o.", caster, NULL, NULL, TO_CHAR );
        else
	  nact( "$N no parece estar envenenad$O.", caster, NULL, ent, TO_CHAR );
        return fERROR;
    }
 
    if (check_dispel(level,victim,gsn_poison))
	return fOK;

    send_to_ent("Fallaste. Nada sucedio.\n\r",caster);
    return fFAIL;
}

SPELL_FUN_DEC(spell_cure_serious)
{
    int heal;

    heal = dice(2, 8) + level /2 ;
    change_health(caster, ent, heal);
    ent_update_pos( ent );
    send_to_ent( "Te sientes mejor!\n\r", ent );

    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );

    return fOK;
}

SPELL_FUN_DEC(spell_curse)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (entidadEsObj(ent))
    {
        obj = entidadGetObj(ent);

        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            nact("$p is already filled with evil.",caster,ent,NULL,TO_CHAR);
            return fERROR;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,gsn_bless);
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                nact("$p glows with a red aura.",caster,ent,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return fOK;
            }
            else
            {
                nact("The holy aura of $p is too powerful for you to overcome.",
                    caster,ent,NULL,TO_CHAR);
                return fFAIL;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = 2 * level;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        nact("$p glows with a malevolent aura.",caster,ent,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    entSetSavingThrow( caster, entGetSavingThrow(caster) + 1 );
        return fOK;
    }

    /* character curses */
    CHECKENT()

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return fFAIL;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( !entComparar(caster, ent) )
	nact("$N looks very uncomfortable.",caster,NULL,ent,TO_CHAR);
    return fOK;
}

/* RT replacement demonfire spell */

SPELL_FUN_DEC(spell_demonfire)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam;

    CHECKCH()
    CHECKENT()

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);

    if (!entComparar(ent, caster))
    {
	nact("$n calls forth the demons of Hell upon $N!",
	    caster,NULL,ent,TO_ROOM);
        nact("$n has assailed you with the demons of Hell!",
	    caster,NULL,ent,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    spell_curse(gsn_curse, 3 * level / 4, caster, ent, TARGET_CHAR);
    return newdamage( caster, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

SPELL_FUN_DEC(spell_detect_evil)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You can already sense evil.\n\r",caster);
	else
	  nact("$N can already detect evil.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_detect_good)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (entComparar(ent, caster))
          send_to_ent("You can already sense good.\n\r",caster);
        else
          nact("$N can already detect good.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( !entComparar(caster, ent) )
        send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_detect_hidden)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (entComparar(ent, caster))
          send_to_ent("You are already as alert as you can be. \n\r",caster);
        else
          nact("$N can already sense hidden lifeforms.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_detect_invis)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (entComparar(ent, caster))
          send_to_ent("You can already see invisible.\n\r",caster);
        else
          nact("$N can already see invisible things.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_detect_magic)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (entComparar(ent, caster))
          send_to_ent("You can already sense magical auras.\n\r",caster);
        else
          nact("$N can already detect magic.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_detect_poison)
{
    OBJ_DATA *obj;

    CHECKOBJENT()

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_ent( "You smell poisonous fumes.\n\r", caster );
	else
	    send_to_ent( "It looks delicious.\n\r", caster );
    }
    else
    {
	send_to_ent( "It doesn't look poisoned.\n\r", caster );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_dispel_evil)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam;
  
    CHECKCH()
    CHECKENT()

    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	nact( "Mota protects $N.", caster, NULL, ent, TO_NOTVICT );
	return fERROR;
    }

    if ( IS_NEUTRAL(victim) )
    {
	nact( "$N does not seem to be affected.", caster, NULL, ent, TO_CHAR );
	return fERROR;
    }

    if (victim->hit > (getNivelPr(ch) * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_HOLY ,TRUE);
}

SPELL_FUN_DEC(spell_dispel_good)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam;
 
    CHECKCH()
    CHECKENT()

    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;
 
    if ( IS_EVIL(victim) )
    {
        nact( "$N is protected by $S evil.", caster, NULL, ent, TO_NOTVICT );
        return fERROR;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
        nact( "$N does not seem to be affected.", caster, NULL, ent, TO_CHAR );
        return fERROR;
    }
 
    if (victim->hit > (getNivelPr(ch) * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_NEGATIVE ,TRUE);
}

/* modified for enhanced use */
SPELL_FUN_DEC(spell_dispel_magic)
{
    CHAR_DATA *victim;
    bool found = FALSE;

    CHECKENT()

    if (saves_spell(level, victim,DAM_OTHER))
    {
	if (entEsCh(ent) && IS_NPC(entGetCh(ent)) && !IS_NPC(victim))
		flog("spell_dispel_magic : victima %s salva, level %d", victim->name, level);

	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_ent( "Fallaste.\n\r", caster);
	return fFAIL;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,gsn_armor))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_bless))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_blindness))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_calm))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_change_sex))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_charm_person))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_domination))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_chill_touch))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_curse))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_evil))
        found = TRUE;

    if (check_dispel(level,victim,gsn_detect_good))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_detect_magic))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_faerie_fire))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_flaming_shield))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_fly))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_frenzy))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_giant_strength))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_haste))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_infravision))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_mass_invis))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_pass_door))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_protect_evil))
        found = TRUE;

    if (check_dispel(level,victim,gsn_protect_good))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_sanctuary))
        found = TRUE;

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, getNivelPr(victim),-1)
	&& !is_affected(victim,gsn_sanctuary))
    {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
        nact("The white aura around $n's body vanishes.",
            ent,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,gsn_shield))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_sleep))
        found = TRUE;

    if (check_dispel(level,victim,gsn_slow))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_stone_skin))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_weaken))
        found = TRUE;
 
    if (check_dispel(level,victim,gsn_combat_mind))
    	found = TRUE;
    
    if (check_dispel(level,victim,gsn_levitation))
    	found = TRUE;
    
    if (check_dispel(level,victim,gsn_displacement))
    	found = TRUE;
    
    if (check_dispel(level,victim,gsn_energy_containment))
    	found = TRUE;
    
    if (check_dispel(level,victim,gsn_flesh_armor))
    	found = TRUE;
    
    if (check_dispel(level,victim,gsn_inertial_barrier))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_intellect_fortress))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_mental_barrier))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_thought_shield))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_biofeedback))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_adrenaline_control))
    	found = TRUE;

    if (check_dispel(level,victim,gsn_fireproof))
    	found = TRUE;

    if (found)
        send_to_ent("Ok.\n\r",caster);
    else
        send_to_ent("Fallaste.\n\r",caster);

    return found == TRUE ? fOK : fFAIL;
}

SPELL_FUN_DEC(spell_earthquake)
{
    CHAR_DATA *ch;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    CHECKCH()

    send_to_ent( "The earth trembles beneath your feet!\n\r", caster );
    nact( "$n makes the earth tremble and shiver.", caster, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if (IS_AFFECTED(vch,AFF_FLYING))
			newdamage(caster, vch, 0, sn, DAM_BASH, TRUE);
		else
			newdamage(caster, vch, level + dice(2, 8), sn, DAM_BASH, TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_enchant_armor)
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    CHECKOBJENT()

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_ent("That isn't an armor.\n\r",caster);
	return fERROR;
    }

    if (obj->wear_loc != -1)
    {
	send_to_ent("The item must be carried to be enchanted.\n\r",caster);
	return fERROR;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	nact("$p flares blindingly... and evaporates!",caster,ent,NULL,TO_CHAR);
	nact("$p flares blindingly... and evaporates!",caster,ent,NULL,TO_ROOM);
	extract_obj(obj, TRUE);
	return fFAIL;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	nact("$p glows brightly, then fades...oops.",caster,ent,NULL,TO_CHAR);
	nact("$p glows brightly, then fades.",caster,ent,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return fOK;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_ent("Nothing seemed to happen.\n\r",caster);
	return fOK;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (90 - level/5))  /* success! */
    {
	nact("$p shimmers with a gold aura.",caster,ent,NULL,TO_CHAR);
	nact("$p shimmers with a gold aura.",caster,ent,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	nact("$p glows a brillant gold!",caster,ent,NULL,TO_CHAR);
	nact("$p glows a brillant gold!",caster,ent,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
 	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
	paf->modifier	= added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    return fOK;
}

SPELL_FUN_DEC(spell_enchant_weapon)
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    CHECKOBJENT()

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_ent("That isn't a weapon.\n\r",caster);
	return fERROR;
    }

    if (obj->wear_loc != -1)
    {
	send_to_ent("The item must be carried to be enchanted.\n\r",caster);
	return fERROR;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	nact("$p shivers violently and explodes!",caster,ent,NULL,TO_CHAR);
	nact("$p shivers violently and explodeds!",caster,ent,NULL,TO_ROOM);
	extract_obj(obj, TRUE);
	return fFAIL;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	nact("$p glows brightly, then fades...oops.",caster,ent,NULL,TO_CHAR);
	nact("$p glows brightly, then fades.",caster,ent,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return fOK;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_ent("Nothing seemed to happen.\n\r",caster);
	return fFAIL;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	nact("$p glows blue.",caster,ent,NULL,TO_CHAR);
	nact("$p glows blue.",caster,ent,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	nact("$p glows a brillant blue!",caster,ent,NULL,TO_CHAR);
	nact("$p glows a brillant blue!",caster,ent,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();
 
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

    return fOK;
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
SPELL_FUN_DEC(spell_energy_drain)
{
    CHAR_DATA *victim;
    int dam;

    CHECKENT()

    if (!entComparar(ent, caster))
	entSetAlignment( caster, UMAX(-1000, entGetAlignment(caster) - 50) );

    if ( saves_spell( level, victim, DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);
	return fFAIL;
    }

    if ( getNivelPr(victim) <= 2 )
    {
	dam		 = entGetHit(caster) + 1;
    }
    else
    {
	gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	change_health(caster, caster, dam);
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_ent("Wow....what a rush!\n\r",caster);
    return newdamage( caster, victim, dam, sn, DAM_NEGATIVE ,TRUE);
}

SPELL_FUN_DEC(spell_fireball)
{
    CHAR_DATA *victim;
    FRetVal retval;
    static const sh_int dam_each[] = 
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130
    };
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;
    retval = newdamage( caster, victim, dam, sn, DAM_FIRE ,TRUE);
    if ( !char_died(victim) )
    	fire_effect( victim, level, dam, TARGET_CHAR );
    return retval;
}

SPELL_FUN_DEC(spell_fireproof)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    if ( entidadEsObj(ent) )
    {
	obj = entidadGetObj(ent);

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
		nact("$p is already protected from burning.",caster,ent,NULL,TO_CHAR);
		return fERROR;
	}

	af.where     = TO_OBJECT;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ITEM_BURN_PROOF;
	affect_to_obj(obj,&af);

	nact("You protect $p from fire.",caster,ent,NULL,TO_CHAR);
	nact("$p is surrounded by a protective aura.",caster,ent,NULL,TO_ROOM);
    }
    else if ( entidadEsCh(ent) )
    {
	victim = entidadGetCh(ent);

	if ( IS_SET(victim->imm_flags, IMM_FIRE) || is_affected(victim,sn) )
	{
		if ( entComparar(caster, ent) )
			send_to_ent( "Ya estas protegido contra el fuego.\n\r", caster );
		else
			nact( "$N ya esta protegid$O contra el fuego.", caster, NULL, ent, TO_CHAR );
		return fERROR;
	}

	af.where	= TO_RESIST;
	af.type		= sn;
	af.level	= level;
	af.duration	= number_fuzzy(level / 10);
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= RES_FIRE;
	af.caster_id	= entGetId(caster);
	affect_to_char(victim,&af);

	if ( !ES_VULN(victim, VULN_COLD) )
	{
		af.where	= TO_VULN;
		af.bitvector	= VULN_COLD;
		affect_to_char(victim,&af);
	}

	if ( !entComparar(caster, ent) )
		nact( "Proteges a $N del fuego.",caster,NULL,ent,TO_CHAR);

	send_to_char( "Un aura protectiva te rodea.\n\r", victim);
	nact( "$N es rodead$O por un aura protectiva.",caster,NULL,ent,TO_NOTVICT);
    }

    return fOK;
}

SPELL_FUN_DEC(spell_flamestrike)
{
    CHAR_DATA *victim;
    int dam;

    CHECKENT()

    dam = dice(6 + level / 2, 8);
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_FIRE ,TRUE);
}

SPELL_FUN_DEC(spell_faerie_fire)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return fERROR;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );

    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );

    return fOK;
}

SPELL_FUN_DEC(spell_faerie_fog)
{
    CHAR_DATA *ich;
    CHAR_DATA *ch;

    CHECKCH()

    nact( "$n conjures a cloud of purple smoke.", caster, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level, ich, DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_floating_disc)
{
    OBJ_DATA *disc, *floating;

    floating = ent_get_eq_char(caster,WEAR_FLOAT);

    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	nact("You can't remove $p.",caster,objToEnt(floating),NULL,TO_CHAR);
	return fERROR;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= entGetNivel(caster) * 10; /* 10 pounds per level capacity */
    disc->value[3]	= entGetNivel(caster) * 5; /* 5 pounds per level max per item */
    disc->timer		= entGetNivel(caster) * 2 - number_range(0,level / 2); 

    nact("$n has created a floating black disc.",caster,NULL,NULL,TO_ROOM);
    send_to_ent("You create a floating disc.\n\r",caster);
    obj_to_ent(disc,caster);
    ent_wear_obj(caster,disc,TRUE);
    return fOK;
}

SPELL_FUN_DEC(spell_fly)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already airborne.\n\r",caster);
	else
	  nact("$N doesn't need your help to fly.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );

    return fOK;
}

/* RT clerical berserking spell */

SPELL_FUN_DEC(spell_frenzy)
{
    AFFECT_DATA af;

    if (ent_is_affected(ent,sn) || ent_IS_AFFECTED(ent,AFF_BERSERK))
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already in a frenzy.\n\r",caster);
	else
	  nact("$N is already in a frenzy.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    if (ent_is_affected(ent,gsn_calm))
    {
	if (entComparar(ent, caster))
	  send_to_ent("Why don't you just relax for a while?\n\r",caster);
	else
	  nact("$N doesn't look like $e wants to fight anymore.",
	      caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    if ((ent_is_good(caster) && !ent_is_good(ent)) ||
        (ent_is_neutral(caster) && !ent_is_neutral(ent)) ||
        (ent_is_evil(caster) && !ent_is_evil(ent))
       )
    {
	nact("Your god doesn't seem to like $N",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_ent(ent,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_ent(ent,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_ent(ent,&af);

    send_to_ent("You are filled with holy wrath!\n\r",ent);
    nact("$n gets a wild look in $s eyes!",ent,NULL,NULL,TO_ROOM);
    return fOK;
}

/* RT ROM-style gate */
    
SPELL_FUN_DEC(spell_gate)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    bool gate_pet;

    CHECKCH()

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
    ||   (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTIPO))
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   getNivelPr(victim) >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && getNivelPr(victim) >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_ent( "Fallaste.\n\r", caster );
        return fFAIL;
    }

    if (ch->pet != NULL
    &&  ch->in_room == ch->pet->in_room
    &&  ch->pet != victim)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;

    nact("$n steps through a gate and vanishes.",caster,NULL,NULL,TO_ROOM);
    send_to_ent("You step through a gate and vanish.\n\r",caster);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    nact("$n has arrived through a gate.",caster,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }

    return fOK;
}

SPELL_FUN_DEC(spell_giant_strength)
{
    AFFECT_DATA af;

    if ( ent_is_affected( ent, sn )
    ||   ent_is_affected( ent, gsn_enhanced_strength ) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already as strong as you can get!\n\r",caster);
	else
	  nact("$N can't get any stronger.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    send_to_ent( "Tus musculos se agrandan!\n\r", ent );
    nact("Los musculos de $n se agrandan.",ent,NULL,NULL,TO_ROOM);

    return fOK;
}

SPELL_FUN_DEC(spell_harm)
{
    CHAR_DATA *victim;
    int dam;

    CHECKENT()

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell( level, victim,DAM_HARM) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    return newdamage( caster, victim, dam, sn, DAM_HARM ,TRUE);
}

/* RT haste spell */

SPELL_FUN_DEC(spell_haste)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    CHECKENT()

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (entComparar(ent, caster))
	  send_to_ent("You can't move any faster!\n\r",caster);
 	else
	  nact("$N is already moving as fast as $E can.",
	      caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (!entComparar(ent, caster))
	        send_to_ent("Fallaste.\n\r",caster);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return fFAIL;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return fOK;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (entComparar(ent, caster))
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Sientes que te mueves mas rapidamente.\n\r", victim );
    act("$n se mueve mas rapidamente.",victim,NULL,NULL,TO_ROOM);
    if ( !entComparar(caster, ent) )
        send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_heal)
{
    change_health(caster, ent, 100);
    ent_update_pos( ent );
    send_to_ent( "A warm feeling fills your body.\n\r", ent );
    if ( !entComparar(caster, ent) )
	send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

void obj_quemar( EVENT *ev )
{
	OBJ_DATA *obj = ev->item.obj;
	CHAR_DATA *ch = obj->carried_by;
	int dam = 0;

	if (ch && !IS_SET(ch->imm_flags, IMM_FIRE) && !is_affected(ch,skill_lookup("fireproof")) )
	{
		act( "La piel de $n arde por el calor de $p!", ch, objToEnt(obj), NULL, TO_ROOM );
		act( "Tu piel arde por el calor de $p!", ch, objToEnt(obj), NULL, TO_CHAR );
		dam = (obj->wear_loc == WEAR_NONE ? 1 : number_range( 0, obj->level / 10 ) + 1 );
		obj_damage( obj, ch, dam, TYPE_UNDEFINED, DAM_FIRE, FALSE );

		if (char_died(ch))
			return;

		if ( CHANCE(60 - obj->level) && (obj->condition > 1) )
			obj->condition--;

		if ( ch && IS_NPC(ch) && IS_SET(ch->form, FORM_SENTIENT) )
		{
			obj_from_char( obj );
			obj_to_room( obj, ch->in_room );
			act( "$n bota $p.", ch, objToEnt(obj), NULL, TO_ROOM );
		}
	}
	else if ( obj->in_obj
	      && !IS_OBJ_STAT(obj->in_obj, ITEM_BURN_PROOF)
	      && obj->in_obj->item_type != ITEM_CORPSE_PC
	      &&  CHANCE(60 - obj->level) )
		obj->in_obj->condition--;

	if ( IS_OBJ_STAT(obj, ITEM_FLAMING) )
		obj_event_add( obj, 5*PULSE_PER_SECOND, NULL, obj_quemar );
}

SPELL_FUN_DEC(spell_heat_metal)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;
    AFFECT_DATA af;
 
    CHECKENT()

   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= level / 2;
		af.duration	= 1 + (level > 25) + (level > 45);
		af.location	= 0;
		af.modifier	= 0;
		af.bitvector	= ITEM_FLAMING;
		affect_to_obj(obj_lose, &af);

		obj_event_add( obj_lose, 5*PULSE_PER_SECOND, NULL, obj_quemar );

		switch ( obj_lose->item_type )
                {
			case ITEM_ARMOR:
			if (obj_lose->wear_loc != -1) /* remove the item */
			{
				if (can_drop_obj(victim,obj_lose)
				&&  (obj_lose->weight / 10) < number_range(1,2 * get_curr_stat(victim,STAT_DEX))
				&&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
				{
					act("$n yelps and throws $p to the ground!",
						victim,objToEnt(obj_lose),NULL,TO_ROOM);
					act("You remove and drop $p before it burns you.",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level) / 3);
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
					fail = FALSE;
				}
				else /* stuck on the body! ouch! */
				{
					act("Your skin is seared by $p!",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level));
					fail = FALSE;
				}
			}
			else /* drop it if we can */
			{
				if (can_drop_obj(victim,obj_lose))
				{
					act("$n yelps and throws $p to the ground!",
						victim,objToEnt(obj_lose),NULL,TO_ROOM);
					act("You remove and drop $p before it burns you.",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level) / 6);
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
					fail = FALSE;
				}
				else /* cannot drop */
				{
					act("Your skin is seared by $p!",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level) / 2);
					fail = FALSE;
				}
			}
			break; /* ITEM_ARMOR */

			case ITEM_WEAPON:
			if (obj_lose->wear_loc != -1) /* try to drop it */
			{
				if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
					continue;

				if (can_drop_obj(victim,obj_lose) 
				&&  remove_obj(victim,obj_lose->wear_loc,TRUE))
				{
					act("$n is burned by $p, and throws it to the ground.",
						victim,objToEnt(obj_lose),NULL,TO_ROOM);
					send_to_char( "You throw your red-hot weapon to the ground!\n\r", victim);
					dam += 1;
					obj_from_char(obj_lose);
					obj_to_room(obj_lose,victim->in_room);
					fail = FALSE;
				}
				else /* YOWCH! */
				{
					send_to_char("Your weapon sears your flesh!\n\r", victim);
					dam += number_range(1,obj_lose->level);
					fail = FALSE;
				}
			}
			else /* drop it if we can */
			{
				if (can_drop_obj(victim,obj_lose))
				{
					act("$n throws a burning hot $p to the ground!",
						victim,objToEnt(obj_lose),NULL,TO_ROOM);
					act("You remove and drop $p before it burns you.",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level) / 6);
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
					fail = FALSE;
				}
				else /* cannot drop */
				{
					act("Your skin is seared by $p!",
						victim,objToEnt(obj_lose),NULL,TO_CHAR);
					dam += (number_range(1,obj_lose->level) / 2);
					fail = FALSE;
				}
			}
			break; /* ITEM_WEAPON */
		} /* switch */
	    } /* if */
	} /* for */
    } /* if */

    if (fail)
    {
        send_to_ent("Your spell had no effect.\n\r", caster);
	send_to_char("You feel momentarily warmer.\n\r",victim);
	return fFAIL;
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	return newdamage(caster,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

/* RT really nasty high-level attack spell */
SPELL_FUN_DEC(spell_holy_word)
{
    CHAR_DATA *ch;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    CHECKCH()

    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    nact("$n utters a word of divine power!",caster,NULL,NULL,TO_ROOM);
    send_to_ent("You utter a word of divine power.\n\r",caster);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
 	  send_to_char("You feel full more powerful.\n\r",vch);
	  spell_frenzy(frenzy_num,level,caster,chToEnt(vch),TARGET_CHAR); 
	  spell_bless(bless_num,level,caster,chToEnt(vch),TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level,caster,chToEnt(vch),TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    newdamage(caster,vch,dam,sn,DAM_ENERGY,TRUE);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level/2,caster,chToEnt(vch),TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,4);
	    newdamage(caster,vch,dam,sn,DAM_ENERGY,TRUE);
   	  }
	}
    }  
    
    send_to_ent("You feel drained.\n\r",caster);
    ch->move = 0;
    change_health(caster, caster, - (ch->hit/2) );
    return fOK;
}

SPELL_FUN_DEC(spell_identify)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *ch = NULL;

    if ( entEsCh(caster) )
    	ch = entGetCh(caster);
    else
    if ( entEsObj(caster) )
	ch = (entGetObj(caster))->carried_by;
 
    if ( ch == NULL )
	return fFAIL;

    CHECKOBJENT()

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->name,
	item_name(obj->item_type),
	flag_string( extra_flags, obj->extra_flags ),
	obj->weight / 10,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Hechizos nivel %d de:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
        sprintf(buf,"It holds %s-colored %s.\n\r",
            liq_table[obj->value[2]].liq_color,
            liq_table[obj->value[2]].liq_name);
        send_to_char(buf,ch);
        break;

    case ITEM_CONTAINER:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3],
	    flag_string( container_flags, obj->value[1] ) );
	send_to_char(buf,ch);
	if (obj->value[4] != 100)
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
		
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("spear/staff.\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("mace/club.\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("axe.\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("flail.\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);	break;
	    default		: send_to_char("unknown.\n\r",ch);	break;
 	}
	if (obj->pIndexData->new_format)
	    sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );
        if (obj->value[4])  /* weapon flags */
        {
            sprintf(buf,"Weapons flags: %s\n\r", flag_string( weapon_type2, obj->value[4] ) );
            send_to_char(buf,ch);
        }
	break;

    case ITEM_ARMOR:
	sprintf( buf, 
	"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
	break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n\r",
                            flag_string( affect_flags, paf->bitvector ) );
                        break;
                    case TO_AFFECTS2:
                    	sprintf( buf,"Adds %s extended affects.\n\r",
                    	    flag_string( affect2_flags, paf->bitvector ) );
                    	break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n\r",
                            flag_string( extra_flags, paf->bitvector ) );
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
            if ( paf->duration > -1)
                sprintf(buf,", %d hours.\n\r",paf->duration);
            else
                sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n\r",
                            flag_string( affect_flags, paf->bitvector ) );
                        break;
                    case TO_AFFECTS2:
                    	sprintf(buf,"Adds %s extended affects.\n\r",
                    	    flag_string( affect2_flags, paf->bitvector ) );
                    	break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n\r",
                            flag_string( extra_flags, paf->bitvector ) );
                        break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n\r",
			    flag_string( weapon_type2, paf->bitvector ) );
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            flag_string( imm_flags, paf->bitvector ));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_infravision)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You can already see in the dark.\n\r",caster);
	else
	  nact("$N already has infravision.\n\r",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    nact( "$n's eyes glow red.", caster, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_invis)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (entidadEsObj(ent))
    {
	obj = entidadGetObj(ent);

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    nact("$p is already invisible.",caster,ent,NULL,TO_CHAR);
	    return fERROR;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	nact("$p fades out of sight.",caster,ent,NULL,TO_ALL);
	return fOK;
    }

    /* character invisibility */
    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return fERROR;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_know_alignment)
{
    CHAR_DATA *victim;
    char *msg;
    int ap;

    CHECKENT()

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    nact( msg, caster, NULL, ent, TO_CHAR );
    return fOK;
}

SPELL_FUN_DEC(spell_lightning_bolt)
{
    CHAR_DATA *victim;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,	61, 62, 62, 63, 64
    };
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_LIGHTNING ,TRUE);
}

SPELL_FUN_DEC(spell_locate_object)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;
    CHAR_DATA *ch;

    CHECKCH()

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	||   getNivelPr(ch) < obj->level)
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "uno lo lleva %s\n\r",
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "uno esta en %s [Cuarto %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
		sprintf( buf, "uno esta en %s en %s\n\r",
		    in_obj->in_room == NULL
		    	? "algun lugar" : in_obj->in_room->name,
		    in_obj->in_room && in_obj->in_room->area ?
		    	in_obj->in_room->area->name :
		    	"algun lugar" );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_ent( "Nothing like that in heaven or earth.\n\r", caster );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return found ? fOK : fFAIL;
}

SPELL_FUN_DEC(spell_magic_missile)
{
    CHAR_DATA *victim;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	14, 14, 14, 14, 14
    };
    int dam, cant;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );

    if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;

    cant = number_range(0, level / 10) + number_fuzzy(1);

    if ( entEsCh(caster) )
	while ( entGetCh(caster)->fighting == victim && cant-- >= 0 )
		newdamage( caster, victim, dam, sn, DAM_ENERGY ,TRUE);

    return fOK;
}

SPELL_FUN_DEC(spell_mass_healing)
{
    CHAR_DATA *gch;
    CHAR_DATA *ch;
    int heal_num, refresh_num;
    
    CHECKCH()

    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,caster,chToEnt(gch),TARGET_CHAR);
	    spell_refresh(refresh_num,level,caster,chToEnt(gch),TARGET_CHAR);  
	}
    }

    return fOK;
}
	    
SPELL_FUN_DEC(spell_mass_invis)
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = entGetPeopleRoom(caster); gch != NULL; gch = gch->next_in_room )
    {
	if ( !ent_is_same_group( chToEnt(gch), caster ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	af.caster_id = entGetId(caster);
	affect_to_char( gch, &af );
    }
    send_to_ent( "Ok.\n\r", caster );

    return fOK;
}

SPELL_FUN_DEC(spell_null)
{
    send_to_ent( "That's not a spell!\n\r", caster );
    bugf("spell_null : llamado por %s",
	entToStringExt(caster) );
    return fERROR;
}

SPELL_FUN_DEC(spell_pass_door)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already out of phase.\n\r",caster);
	else
	  nact("$N is already shifted out of phase.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return fOK;
}

/* RT plague spell, very nasty */

SPELL_FUN_DEC(spell_plague)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (entComparar(caster, ent))
		send_to_ent("Te sientes ligeramente enfermo por un momento.\n\r",caster);
	else
		nact("$N parece no haber sido afectado.",caster,NULL,ent,TO_CHAR);
	return fFAIL;
    }

    af.where		= TO_AFFECTS;
    af.type		= sn;
    af.level		= level;
    af.duration		= level;
    af.location		= APPLY_STR;
    af.modifier		= -5; 
    af.bitvector	= AFF_PLAGUE;
    affect_join(victim,&af);

    send_to_char("Te retuerces en agonia mientras las pustulas de la plaga surgen en tu piel.\n\r",victim);
    act("$n se retuerce en agonia mientras las pustulas de la plaga surgen en su piel.",
		victim,NULL,NULL,TO_ROOM);

    return fOK;
}

SPELL_FUN_DEC(spell_poison)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    if (entidadEsObj(ent))
    {
	obj = entidadGetObj(ent);

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		nact("Tu hechizo no pudo corromper $p.",caster,ent,NULL,TO_CHAR);
		return fERROR;
	    }
	    SET_BIT(obj->value[3], FOOD_POISON);
	    nact("$p es mezclado con vapores venenosos.",caster,ent,NULL,TO_ALL);
	    return fOK;
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
		nact("No puedes envenenar $p.",caster,ent,NULL,TO_CHAR);
		return fERROR;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		nact("$p ya esta envenenado.",caster,ent,NULL,TO_CHAR);
		return fERROR;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    nact("$p es cubierto con un veneno mortal.",caster,ent,NULL,TO_ALL);
	    return fOK;
	}

	nact("No puedes envenenar $p.",caster,ent,NULL,TO_CHAR);
	return fERROR;
    }

    CHECKENT()

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n se pone ligeramente verde, pero se recupera.",victim,NULL,NULL,TO_ROOM);
	send_to_char("Te sientes ligeramente enfermo, pero te recuperas.\n\r",victim);
	return fFAIL;
    }

    af.where		= TO_AFFECTS;
    af.type		= sn;
    af.level		= level;
    af.duration		= level;
    af.location		= APPLY_STR;
    af.modifier		= -2;
    af.bitvector	= AFF_POISON;
    affect_join_alt( victim, &af );

    /* hitroll */
    af.where		= TO_AFFECTS;
    af.type		= sn;
    af.level		= level;
    af.duration		= level;
    af.location		= APPLY_HITROLL;
    af.modifier		= -2;
    af.bitvector	= AFF_POISON;
    affect_join_alt( victim, &af );

    /* damroll */
    af.location		= APPLY_DAMROLL;
    affect_join_alt( victim, &af );

    send_to_char( "Te sientes muy enfermo.\n\r", victim );
    act("$n se ve muy enfermo.",victim,NULL,NULL,TO_ROOM);

    return fOK;
}

SPELL_FUN_DEC(spell_protection_evil)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
        if (entComparar(ent, caster))
          send_to_ent("You are already protected.\n\r",caster);
        else
          nact("$N is already protected.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( !entComparar(caster, ent) )
        nact("$N is protected from evil.",caster,NULL,ent,TO_CHAR);
    return fOK;
}
 
SPELL_FUN_DEC(spell_protection_good)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
        if (entComparar(ent, caster))
          send_to_ent("You are already protected.\n\r",caster);
        else
          nact("$N is already protected.",caster,NULL,ent,TO_CHAR);
        return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( !entComparar(caster, ent) )
        nact("$N is protected from good.",caster,NULL,ent,TO_CHAR);
    return fOK;
}

SPELL_FUN_DEC(spell_ray_of_truth)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam, align;
 
    CHECKCH()
    CHECKENT()

    if (IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
    if (!entComparar(ent, caster))
    {
        nact("$n raises $s hand, and a blinding ray of light shoots forth!",
            caster,NULL,NULL,TO_ROOM);
        send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,NULL,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return fERROR;
    }

    dam = dice( level, 10 );

    if ( saves_spell( level, victim,DAM_HOLY) )
        dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;

    spell_blindness(gsn_blindness, 
	3 * level / 4, caster, (void *) victim,TARGET_CHAR);

    return char_died(victim) ? victdead : newdamage( caster, victim, dam, sn, DAM_HOLY, TRUE);
}

SPELL_FUN_DEC(spell_recharge)
{
    OBJ_DATA *obj;
    int chance, percent;

    CHECKOBJENT()

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_ent("That item does not carry charges.\n\r",caster);
	return fERROR;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_ent("Your skills are not great enough for that.\n\r",caster);
	return fERROR;
    }

    if (obj->value[1] == 0)
    {
	send_to_ent("That item has already been recharged once.\n\r",caster);
	return fERROR;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	nact("$p glows softly.",caster,ent,NULL,TO_CHAR);
	nact("$p glows softly.",caster,ent,NULL,TO_ROOM);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return fOK;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	nact("$p glows softly.",caster,ent,NULL,TO_CHAR);
	nact("$p glows softly.",caster,ent,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return fOK;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_ent("Nothing seems to happen.\n\r",caster);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return fFAIL;
    }

    else /* whoops! */
    {
	nact("$p glows brightly and explodes!",caster,ent,NULL,TO_CHAR);
	nact("$p glows brightly and explodes!",caster,ent,NULL,TO_ROOM);
	extract_obj(obj, TRUE);
	return fFAIL;
    }
}

SPELL_FUN_DEC(spell_refresh)
{
    CHAR_DATA *victim;

    CHECKENT()

    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("Te sientes completamente recuperado!\n\r",victim);
    else
        send_to_char( "Te sientes menos cansado.\n\r", victim );
    if ( !entComparar(caster, ent) )
        send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_remove_curse)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (entidadEsObj(ent))
    {
	obj = entidadGetObj(ent);

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		nact("$p glows blue.",caster,ent,NULL,TO_ALL);
		return fOK;
	    }

	    nact("The curse on $p is beyond your power.",caster,ent,NULL,TO_CHAR);
	    return fFAIL;
	}
	nact("There doesn't seem to be a curse on $p.",caster,ent,NULL,TO_CHAR);
	return fERROR;
    }

    /* characters */
    CHECKENT()

    if ( is_affected(victim, gsn_curse)
    &&   check_dispel(level,victim,gsn_curse))
    {
	send_to_char("Te sientes mejor.\n\r",victim);
	act("$n se ve mas relajado.",victim,NULL,NULL,TO_ROOM);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,objToEnt(obj),NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,objToEnt(obj),NULL,TO_ROOM);
            }
         }
    }

    return fOK;
}

SPELL_FUN_DEC(spell_sanctuary)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("You are already in sanctuary.\n\r",caster);
	else
	  nact("$N is already in sanctuary.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    act( "$n es rodead$o por un aura blanca.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Un aura blanca te rodea.\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_shield)
{
    AFFECT_DATA af;

    if ( ent_is_affected( ent, sn ) )
    {
	if (entComparar(ent, caster))
		send_to_ent("Ya estas protegido del dano.\n\r",caster);
	else
		nact("$N ya esta protegid$O del dano.", caster, NULL, ent, TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_ent( ent, &af );

    nact( "$n es rodead$o por un campo de fuerza.", ent, NULL, NULL, TO_ROOM );
    send_to_ent( "Un campo de fuerza te rodea.\n\r", ent );

    return fOK;
}

SPELL_FUN_DEC(spell_shocking_grasp)
{
    CHAR_DATA *victim;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57
    };
    int dam;

    CHECKENT()

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_LIGHTNING ,TRUE);
}

SPELL_FUN_DEC(spell_sleep)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
  
    CHECKENT()

    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < getNivelPr(victim)
    ||   saves_spell( level-4, victim,DAM_CHARM) )
	return fFAIL;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }

    return fOK;
}

SPELL_FUN_DEC(spell_slow)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    CHECKENT()

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (entComparar(ent, caster))
          send_to_ent("You can't move any slower!\n\r",caster);
        else
          nact("$N can't get any slower than that.",
              caster,NULL,ent,TO_CHAR);
        return fERROR;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (!entComparar(ent, caster))
            send_to_ent("Nothing seemed to happen.\n\r",caster);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return fFAIL;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (!entComparar(ent, caster))
            	send_to_ent("Fallaste.\n\r",caster);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return fFAIL;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return fOK;
    }
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return fOK;
}

SPELL_FUN_DEC(spell_stone_skin)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( is_affected( victim, sn )
    ||   is_affected( victim, skill_lookup( "flesh armor" ) ) )
    {
	if (entComparar(ent, caster))
	  send_to_ent("Your skin is already as hard as a rock.\n\r",caster); 
	else
	  nact("$N is already as hard as can be.",caster,NULL,ent,TO_CHAR);
	return fERROR;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    act( "La piel de $n se transforma en piedra.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Tu piel se transforma en piedra.\n\r", victim );
    return fOK;
}

SPELL_FUN_DEC(spell_summon)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    extern AREA_DATA * laberinto;

    CHECKCH()

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO)
    ||	 ch->in_room->area == laberinto
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTIPO)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   getNivelPr(victim) >= level + 3
    ||   (!IS_NPC(victim) && getNivelPr(victim) >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )

    {
	send_to_char( "Fallaste.\n\r", ch );
	return fFAIL;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );

    char_from_room( victim );
    char_to_room( victim, ch->in_room );

    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    nact( "$n has summoned you!", caster, NULL, chToEnt(victim), TO_VICT );

    do_look( victim, "auto" );

    if (IS_NPC(victim))
    {
	if ( getNivelPr(ch) > getNivelPr(victim) + 3 ) /* huir!!! */
    	{
    		int door = mob_best_door(victim);

		give_mem( victim, MEM_AFRAID, ch->id );

		if ( door != -1 )
		{
			act( "$n huye aterrorizad$o!", victim, NULL, NULL, TO_ROOM );
			move_char( victim, door, FALSE );
		}
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_teleport)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    CHAR_DATA *ch;

    CHECKCH()
    CHECKENT()

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_ent( "Fallaste.\n\r", caster );
	return fFAIL;
    }

    pRoomIndex = get_random_room(ent);

    if (!entComparar(ent, caster))
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
    	REMOVE_BIT(victim->act, ACT_STAY_AREA);
	set_hunt( victim, ch, FALSE );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_ventriloquate)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *ch;

    CHECKCH()

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
	    send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return fOK;
}

SPELL_FUN_DEC(spell_weaken)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    CHECKENT()

    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return fFAIL;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    af.caster_id = entGetId(caster);
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return fOK;
}

/* RT recall spell is back */

SPELL_FUN_DEC(spell_word_of_recall)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    
    CHECKENT()

    if (IS_NPC(victim))
      return fERROR;
   
    if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
    {
	send_to_char("You are completely lost.\n\r",victim);
	return fERROR;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Fallaste.\n\r",victim);
	return fERROR;
    }

    mob_odio_check(victim->in_room, victim);

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);

    victim->move /= 2;

    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_look(victim,"auto");

    return fOK;
}

/*
 * NPC spells.
 */
SPELL_FUN_DEC(spell_acid_breath)
{
    CHAR_DATA *victim;
    int dam,hp_dam,dice_dam,hpch;

    CHECKENT()

    nact("$n spits acid at $N.",caster,NULL,ent,TO_NOTVICT);
    nact("$n spits a stream of corrosive acid at you.",caster,NULL,ent,TO_VICT);
    nact("You spit acid at $N.",caster,NULL,ent,TO_CHAR);

    hpch = UMAX(12,entGetHit(caster));
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
    if (saves_spell(level,victim,DAM_ACID))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	return newdamage(caster,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	return newdamage(caster,victim,dam,sn,DAM_ACID,TRUE);
    }
}

SPELL_FUN_DEC(spell_fire_breath)
{
    CHAR_DATA *victim;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;
    CHAR_DATA *ch;

    CHECKENT()
    CHECKCH()

    nact("$n breathes forth a cone of fire.",caster,NULL,ent,TO_NOTVICT);
    nact("$n breathes a cone of hot fire over you!",caster,NULL,ent,TO_VICT);
    nact("You breath forth a cone of fire.",caster,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, entGetHit(caster) );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);

    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		newdamage(caster,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		newdamage(caster,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		newdamage(caster,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		newdamage(caster,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_frost_breath)
{
    CHAR_DATA *victim;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam, hpch;
    CHAR_DATA *ch;

    CHECKENT()
    CHECKCH()

    nact("$n breathes out a freezing cone of frost!",caster,NULL,ent,TO_NOTVICT);
    nact("$n breathes a freezing cone of frost over you!",
	caster,NULL,ent,TO_VICT);
    nact("You breath out a cone of frost.",caster,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,entGetHit(caster));
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		newdamage(caster,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		newdamage(caster,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		newdamage(caster,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		newdamage(caster,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_gas_breath)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *ch;
    int dam,hp_dam,dice_dam,hpch;

    CHECKCH()

    nact("$n breathes out a cloud of poisonous gas!",caster,NULL,NULL,TO_ROOM);
    nact("You breath out a cloud of poisonous gas.",caster,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,entGetHit(caster));
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    newdamage(caster,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    newdamage(caster,vch,dam,sn,DAM_POISON,TRUE);
	}
    }

    return fOK;
}

SPELL_FUN_DEC(spell_lightning_breath)
{
    CHAR_DATA *victim;
    int dam,hp_dam,dice_dam,hpch;

    CHECKENT()

    nact("$n breathes a bolt of lightning at $N.",caster,NULL,ent,TO_NOTVICT);
    nact("$n breathes a bolt of lightning at you!",caster,NULL,ent,TO_VICT);
    nact("You breathe a bolt of lightning at $N.",caster,NULL,ent,TO_CHAR);

    hpch = UMAX(10,entGetHit(caster));
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	return newdamage(caster,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	return newdamage(caster,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
SPELL_FUN_DEC(spell_general_purpose)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam;
 
    CHECKCH()
    CHECKENT()

    dam = number_range( 25, 100 );

    if ( !IS_NPC(ch) )
    	dam = (int) (dam * (getNivelPr(ch) / (float) (number_fuzzy(3) * LEVEL_HERO) ));

    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    return newdamage( caster, victim, dam, sn, DAM_PIERCE ,TRUE);
}

SPELL_FUN_DEC(spell_high_explosive)
{
    CHAR_DATA *victim;
    CHAR_DATA *ch;
    int dam;
 
    CHECKENT()
    CHECKCH()

    dam = number_range( 30, 120 );

    if ( !IS_NPC(ch) )
    	dam = (int) (dam * (getNivelPr(ch) / (float) (number_fuzzy(3) * LEVEL_HERO)));

    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;

    return newdamage( caster, victim, dam, sn, DAM_PIERCE ,TRUE);
}

void do_invis_pred( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int chance;

    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
    	do_visible( ch, "" );
    	send_to_char( "Lentamente, reapareces en esta realidad.\n\r", ch );
    	act( "$n aparece en esta realidad.", ch, NULL, NULL, TO_ROOM );
    	return;
    }

    chance = get_skill(ch, gsn_predinvis);

    if ( ch->race == race_lookup( "predator" ) )
    	chance += 30;
    else
    	chance -= 30;

    chance = URANGE( 0, chance, 100 );

    send_to_char( "Intentas disolverte en el aire.\n\r", ch );
    WAIT_STATE(ch, skill_table[gsn_predinvis].beats );

    if (CHANCE(chance))
    {
	act( "$n se disuelve en el aire.", ch, NULL, NULL, TO_ROOM );

	af.where     = TO_AFFECTS;
	af.type      = gsn_predinvis;
	af.level     = getNivelPr(ch) / 2;
	af.duration  = getNivelPr(ch) / 3;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	af.caster_id = ch->id;
	affect_to_char( ch, &af );

	check_improve(ch, gsn_predinvis, TRUE, 2 );
    }
    else
    	check_improve(ch, gsn_predinvis, FALSE, 2 );

    return;
}

SPELL_FUN_DEC(spell_make_bag)
{
    OBJ_DATA *obj;
    OBJ_DATA *bag;
    char buf[MAX_STRING_LENGTH];

	CHECKOBJENT()

        if ( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
		return fERROR;

	if ( get_obj_index(  3032  ) == NULL )
	{
		send_to_ent( "fERROR! Dile al Imp que algo estupido esta sucediendo.\n\r", caster );
		bug("spell_make_bag : NULL obj vnum 3032", 0);
		return fERROR;
	}

	bag = create_object(get_obj_index(3032), level ); 
	sprintf( buf, "bolso %s", obj->short_descr );
        free_string( bag->name );
        bag->name = str_dup(buf);

        sprintf( buf, "Un bolso hecho de piel de %s llama tu atencion.",
           obj->short_descr );
        free_string( bag->description );
        bag->description = str_dup( buf );

        sprintf( buf, "bolso hecho de %s", obj->short_descr );
        free_string( bag->short_descr );
        bag->short_descr = str_dup( buf );

    bag->item_type = ITEM_CONTAINER;
    bag->wear_flags = ITEM_HOLD|ITEM_TAKE;
    bag->timer = 0;
    bag->weight = 5;
    bag->level = obj->level / 3;
    bag->cost = obj->level * 50;
    bag->value[0] = obj->level * 10;                 /* Weight capacity */
    bag->value[1] = 1;                          /* Closeable */
    bag->value[2] = -1;                         /* No key needed */
    bag->value[3] = number_range(90,100);
    nact( "Tu nuevo $p se ve muy elegante.", caster, objToEnt(bag), NULL, TO_CHAR );
    nact( "El nuevo $p de $n se ve muy elegante.", caster, objToEnt(bag), NULL, TO_ROOM );

    obj_to_ent(bag,caster);
    extract_obj(obj, TRUE);
    send_to_ent( "Ok.\n\r", caster );
    return fOK;
}

SPELL_FUN_DEC(spell_quench)
{
    CHAR_DATA *ch;

    CHECKCH()

    if ( IS_NPC(ch) )
	return fERROR;
  
    ch->pcdata->condition[COND_THIRST] = UMIN(ch->pcdata->condition[COND_THIRST] + level/2, 30);
    send_to_char( "You have quenched your thirst.\n\r", ch );
    return fOK;
}

SPELL_FUN_DEC(spell_sate)
{
    CHAR_DATA *ch;

    CHECKCH()

    if ( IS_NPC(ch) )
	return fERROR;
  
    ch->pcdata->condition[COND_HUNGER] = UMIN(ch->pcdata->condition[COND_HUNGER] + level/2, 30);
    send_to_char( "You have sated your hunger.\n\r", ch );
    return fOK;
}

SPELL_FUN_DEC(spell_imprint)
{
    OBJ_DATA *obj;
    CHAR_DATA *ch;
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];

    CHECKCH()
    CHECKOBJENT()

    if (skill_table[sn].spell_fun == spell_null )
    {
	send_to_ent("Eso no es un spell.\n\r",caster);
	return fERROR;
    }

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++) 
	if (obj->value[i] != -1)
	    sp_slot++;

    if (sp_slot > 3)
    {
	nact ("$p no puede tener mas spells.", caster, ent, NULL, TO_CHAR);
	return fERROR;
    }

   /* scribe/brew costs 4 times the normal mana required to cast the spell */

    mana = 4 * UMAX( skill_table[sn].min_mana, 100 / ( 2 + getNivelPr(ch) - skill_table[sn].skill_level[getClasePr(ch)] ) );
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "No tienes suficiente mana.\n\r", ch );
	return fFAIL;
    }
      
    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
	send_to_char( "Perdiste la concentracion.\n\r", ch );
	ch->mana -= mana / 2;
	return fFAIL;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or 
       scrolls - JH */ 

    switch( sp_slot )
    {
   
    default:
	bug( "sp_slot has more than %d spells.", sp_slot );
	return fERROR;

    case 1:
        if ( number_percent() > 80 )
        { 
          sprintf(buf, "El encantamiento magico ha fallado --- %s %s se desvanece.\n\r",
          (obj->item_type == ITEM_SCROLL ? "el" : "la"), item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj, TRUE );
	  return fFAIL;
	}     
	break;
    case 2:
        if ( number_percent() > 40 )
        { 
          sprintf(buf, "El encantamiento magico ha fallado --- %s %s se desvanece.\n\r",
          (obj->item_type == ITEM_SCROLL ? "el" : "la"), item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj, TRUE );
	  return fFAIL;
	}     
	break;

    case 3:
        if ( number_percent() > 20 )
        { 
          sprintf(buf, "El encantamiento magico ha fallado --- %s %s se desvanece.\n\r",
          (obj->item_type == ITEM_SCROLL ? "el" : "la"), item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj, TRUE );
	  return fFAIL;
	}     
	break;
    } 

    /* labeling the item */

    free_string (obj->short_descr);
    sprintf ( buf, "a %s of ", item_name(obj->item_type) ); 
    for (i = 1; i <= sp_slot ; i++)
      if (obj->value[i] != -1)
      {
	strcat (buf, skill_table[obj->value[i]].name);
        (i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ; 
      }
    obj->short_descr = str_dup(buf);
	
    sprintf( buf, "%s %s", obj->name, item_name(obj->item_type) );
    free_string( obj->name );
    obj->name = str_dup( buf );        

    sprintf(buf, "Has implantado un nuevo spell a %s.\n\r", item_name(obj->item_type) );
    send_to_char( buf, ch );

    return fOK;
}

SPELL_FUN_DEC(spell_hambre)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( IS_NPC(victim) )
		return fFAIL;

	send_to_char( "Te sientes hambriento.\n\r", victim );
	victim->pcdata->condition[COND_HUNGER] = UMAX(0, victim->pcdata->condition[COND_HUNGER] - level);

	return fOK;
}

SPELL_FUN_DEC(spell_sed)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( IS_NPC(victim) )
		return fFAIL;

	send_to_char( "Te sientes sediento.\n\r", victim );
	victim->pcdata->condition[COND_THIRST] = UMAX(0, victim->pcdata->condition[COND_THIRST] - level);

	return fOK;
}

SPELL_FUN_DEC(spell_ebriedad)
{
	CHAR_DATA *victim;

	CHECKENT()

	if ( IS_NPC(victim) )
		return fFAIL;

	send_to_char( "Te sientes ebrio. *HIC*\n\r", victim );
	victim->pcdata->condition[COND_DRUNK] = UMIN(50, victim->pcdata->condition[COND_DRUNK] + level);

	return fOK;
}
