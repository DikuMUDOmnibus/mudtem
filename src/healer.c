/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "merc.h"
#include "magic.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "No puedes hacer eso aqui.\n\r", ch );
        return;
    }

    if ( IS_SET(mob->act, ACT_PROTOTIPO) )
//  &&  !IS_IMMORTAL(ch) )
    {
	AFFECT_DATA af;

    	send_to_char( "Te sientes estupido.\n\r", ch );

	af.where	= TO_VULN;
	af.type		= 0;
	af.level	= MAX_LEVEL;
	af.duration	= -1;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= VULN_WEAPON;
	af.caster_id	= ch->id;
	
	if ( !ES_VULN(ch, VULN_WEAPON) )
		affect_to_char( ch, &af );

	af.bitvector	= VULN_MAGIC;
	if ( !ES_VULN(ch, VULN_MAGIC) )
		affect_to_char( ch, &af );

	af.bitvector	= VULN_MENTAL;
	if ( !ES_VULN(ch, VULN_MENTAL) )
		affect_to_char( ch, &af );

	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
	act("$N te dice 'Ofrezco los siguientes hechizos:'",ch,NULL,chToEnt(mob),TO_CHAR);
	send_to_char("  exorcismo: quita vampiric bite 20000 gold\n\r",ch);
	send_to_char("  light: cure light wounds       10 gold\n\r",ch);
	send_to_char("  serious: cure serious wounds   15 gold\n\r",ch);
	send_to_char("  critic: cure critical wounds   25 gold\n\r",ch);
	send_to_char("  heal: healing spell            50 gold\n\r",ch);
	send_to_char("  blind: cure blindness          20 gold\n\r",ch);
	send_to_char("  disease: cure disease          15 gold\n\r",ch);
	send_to_char("  poison:  cure poison           25 gold\n\r",ch); 
	send_to_char("  uncurse: remove curse          50 gold\n\r",ch);
	send_to_char("  refresh: restore movement       5 gold\n\r",ch);
	send_to_char("  mana:  restore mana            10 gold\n\r",ch);
	send_to_char("  cancel: recupera contra spells 80 gold\n\r",ch);
	send_to_char(" Escribe #Bheal <tipo>#b para ser sanado.\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"light"))
    {
        spell = spell_cure_light;
	sn    = gsn_cure_light;
	words = "judicandus dies";
	 cost  = 1000;
    }

    else if (!str_prefix(arg,"serious"))
    {
	spell = spell_cure_serious;
	sn    = gsn_cure_serious;
	words = "judicandus gzfuajg";
	cost  = 1600;
    }

    else if (!str_prefix(arg,"critical"))
    {
	spell = spell_cure_critical;
	sn    = gsn_cure_critical;
	words = "judicandus qfuhuqar";
	cost  = 2500;
    }

    else if (!str_prefix(arg,"heal"))
    {
	spell = spell_heal;
	sn = gsn_heal;
	words = "pzar";
	cost  = 5000;
    }

    else if (!str_prefix(arg,"blindness"))
    {
	spell = spell_cure_blindness;
	sn    = gsn_cure_blindness;
      	words = "judicandus noselacri";		
        cost  = 2000;
    }

    else if (!str_prefix(arg,"disease"))
    {
	spell = spell_cure_disease;
	sn    = gsn_cure_disease;
	words = "judicandus eugzagz";
	cost = 1500;
    }

    else if (!str_prefix(arg,"poison"))
    {
	spell = spell_cure_poison;
	sn    = gsn_cure_poison;
	words = "judicandus sausabru";
	cost  = 2500;
    }
	
    else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
	spell = spell_remove_curse; 
	sn    = gsn_remove_curse;
	words = "candussido judifgz";
	cost  = 5000;
    }

    else if (!str_prefix(arg,"mana") || !str_prefix(arg,"energize"))
    {
        spell = NULL;
        sn = -1;
        words = "energizer";
        cost = 1000;
    }

    else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
	spell =  spell_refresh;
	sn    = skill_lookup("refresh");
	words = "candusima"; 
	cost  = 500;
    }

    else if (!str_prefix(arg,"cancel"))
    {
    	spell	= spell_cancellation;
    	sn	= gsn_cancellation;
    	words	= "shinkuuhadoken";
    	cost	= 7500;
    }

    else if (!str_prefix(arg,"exorcismo"))
    {
    	spell	= spell_exorcise;
    	sn	= skill_lookup("exorcise");
    	words	= "tatsumakisenpuukiaku";
    	cost	= 2000000;
    }

    else 
    {
	act("$N te dice 'Escribe 'heal' para una lista de hechizos.'",
	    ch,NULL,chToEnt(mob),TO_CHAR);
	return;
    }

    if (cost > (ch->gold * 100 + ch->silver))
    {
	act("$N te dice 'No tienes suficiente dinero como para pagar mis servicios.'",
	    ch,NULL,chToEnt(mob),TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    deduct_cost(ch,cost);
    mob->gold += cost;
    mob->silver += cost % 100;
    act("$n murmura las palabras '$T'.",mob,NULL,strToEnt(words,mob->in_room),TO_ROOM);

    if (spell == NULL)  /* restore mana trap...kinda hackish */
    {
	ch->mana += dice(2,8) + getNivelPr(mob) / 3;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r",ch);
	return;
     }

     if (sn < 1)
	return;

     spell(sn,getNivelPr(mob),chToEnt(mob),chToEnt(ch),TARGET_CHAR);
}
