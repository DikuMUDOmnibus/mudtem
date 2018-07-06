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
#include "recycle.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

DO_FUN_DEC(do_gain)
{
	CHAR_DATA * mob;
	int clase;

	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_GAIN) )
			break;

	if ( !mob )
	{
		send_to_char( "Parece que nadie aqui sabe mucho acerca de eso.\n\r", ch );
		return;
	}

	if ( class_table[getClasePr(ch)].multiclase )
	{
		mob_tell( mob, ch, "Tu clase no puede ser multiclase.", NULL );
		return;
	}

	if ( class_table[getClasePr(ch)].max_num_clases > num_clases(ch) )
	{
		mob_tell( mob, ch, "No puedes desarrollar mas habilidades.", NULL );
		return;
	}

	if ( (clase = class_lookup(argument)) == -1 )
	{
		mob_tell( mob, ch, "No conozco la clase '$t'.", strToEnt(argument,ch->in_room) );
		return;
	}

	if ( !puede_ser_clase(getClasePr(ch), ch->race, clase) )
	{
		mob_tell( mob, ch, "Lo siento, no puedes especializarte en esa clase.", NULL );
		return;
	}

	mob_tell(mob,ch,"Err....",NULL);
}

/* RT spells and skills show the players spells (or skills) */
void do_spells(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
    bool fAll = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;

    if (argument[0] != '\0')
    {
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
	    }
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > LEVEL_HERO)
	    {
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
	    }

	    if (argument[0] != '\0')
	    {
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		    send_to_char("Arguments must be numerical or all.\n\r",ch);
		    return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		    send_to_char(buf,ch);
		    return;
		}

		if (min_lev > max_lev)
		{
		    send_to_char("That would be silly.\n\r",ch);
		    return;
		}
	    }
	}
    }

    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        spell_columns[level] = 0;
        spell_list[level][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	level = charMinLevelSn(ch, sn);

	if ( can_prac(ch, sn)
	&&  (fAll || level <= getNivelPr(ch))
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun != spell_null
	&&  get_skill(ch, sn) > 0)
        {
	    found = TRUE;
	    if (getNivelPr(ch) < level)
	    	sprintf(buf,"%-18s n/a      ", NOMBRE_SKILL(sn) );
	    else
	    {
		mana = UMAX(skill_table[sn].min_mana,
		    100/(2 + getNivelPr(ch) - level));
	        sprintf(buf,"%-18s  %3d mana  ", NOMBRE_SKILL(sn), mana);
	    }
 
	    if (spell_list[level][0] == '\0')
          	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++spell_columns[level] % 2 == 0)
		    strcat(spell_list[level],"\n\r          ");
          	strcat(spell_list[level],buf);
	    }
	}
    }
 
    /* return results */
 
    if (!found)
    {
      	send_to_char("No spells found.\n\r",ch);
      	return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (spell_list[level][0] != '\0')
	    add_buf(buffer,spell_list[level]);
    add_buf(buffer,"\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool fAll = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;

    if (argument[0] != '\0')
    {
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
	    }
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > LEVEL_HERO)
	    {
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
	    }

	    if (argument[0] != '\0')
	    {
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		    send_to_char("Arguments must be numerical or all.\n\r",ch);
		    return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		    send_to_char(buf,ch);
		    return;
		}

		if (min_lev > max_lev)
		{
		    send_to_char("That would be silly.\n\r",ch);
		    return;
		}
	    }
	}
    }

    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	level = charMinLevelSn(ch, sn);

	if (can_prac(ch, sn)
	&&  (fAll || level <= getNivelPr(ch))
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun == spell_null
	&& !es_skill_racial(ch->race, sn)
	&&  get_skill(ch, sn) > 0)
        {
	    found = TRUE;
	    if (getNivelPr(ch) < level)
		sprintf(buf,"%-18s n/a      ", NOMBRE_SKILL(sn) );
	    else
		sprintf(buf,"%-18s %3d%%      ", NOMBRE_SKILL(sn),
		    get_skill(ch, sn) );
 
	    if (skill_list[level][0] == '\0')
          	sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++skill_columns[level] % 2 == 0)
		    strcat(skill_list[level],"\n\r          ");
          	strcat(skill_list[level],buf);
	    }
	}
    }
 
    /* return results */
    if (!found)
    {
      	send_to_char("No skills found.\n\r",ch);
      	return;
    }

    buffer = new_buf();
    if ( !IS_NULLSTR(race_table[ch->race].skills[0]) ) /* skills raciales */
    {
	int i;

    	add_buf( buffer, "Habilidades raciales:\n\r" );
    	for ( i = 0; i < MAX_RACE_SKILL && !IS_NULLSTR(race_table[ch->race].skills[i]); i++ )
	{
		add_buf( buffer, race_table[ch->race].skills[i] );
		add_buf( buffer, "\n\r" );
	}
    }

    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (skill_list[level][0] != '\0')
	    add_buf(buffer,skill_list[level]);
    add_buf(buffer,"\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","group","cp","group","cp","group","cp");
    send_to_char(buf,ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	if (group_table[gn].name == NULL)
	    break;

        if (!ch->gen_data->group_chosen[gn] 
	&&  !ch->pcdata->group_known[gn]
	&&  group_table[gn].rating[getClasePr(ch)] > 0)
	{
	    sprintf(buf,"%-18s %-5d ",group_table[gn].name,
				    group_table[gn].rating[getClasePr(ch)]);
	    send_to_char(buf,ch);
	    if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (!ch->gen_data->skill_chosen[sn] 
	&&  ch->pcdata->learned[sn] == 0
	&&  ((getNivelPr(ch) > 0) || (skill_table[sn].rating[getClasePr(ch)] < LEVEL_HERO))
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].rating[getClasePr(ch)] > 0)
        {
            sprintf(buf,"%-18s %-5d ", NOMBRE_SKILL(sn),
                                    skill_table[sn].rating[getClasePr(ch)]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;
 
    if (IS_NPC(ch))
        return;
 
    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","group","cp","group","cp","group","cp\n\r");
    send_to_char(buf,ch);
 
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;
 
        if (ch->gen_data->group_chosen[gn] 
	&&  group_table[gn].rating[getClasePr(ch)] > 0)
        {
            sprintf(buf,"%-18s %-5d ",group_table[gn].name,
                                    group_table[gn].rating[getClasePr(ch)]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].rating[getClasePr(ch)] > 0)
        {
            sprintf(buf,"%-18s %-5d ", NOMBRE_SKILL(sn),
                                    skill_table[sn].rating[getClasePr(ch)]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

int exp_per_level(CHAR_DATA *ch, int points)
{
    int expl,inc,race, suma = 0;
    LEVEL_DATA * lev;

    if (IS_NPC(ch) || ch->level_data == NULL)
	return 1000; 

    race = ch->true_race;

    for ( lev = ch->level_data; lev; lev = lev->next )
    {
	expl = 1000;
	inc = 500;

	if (points < 40)
	{
		suma += 1000 * (race_table[race].class_mult[lev->clase] ?
		       race_table[race].class_mult[lev->clase]/100 : 1);
		continue;
	}

	/* processing */
	points -= 40;

	while (points > 9)
	{
		expl += inc;
        	points -= 10;
        	if (points > 9)
		{
		    expl += inc;
		    inc *= 2;
		    points -= 10;
		}
    	}

	expl += points * inc / 10;

	suma += expl * race_table[race].class_mult[lev->clase]/100;
    }

    return suma;
}

int race_exp_per_level( int race, int class, int points )
{
    int expl,inc;

    expl = 1000;
    inc = 500;

    if (points < 40)
	return 1000 * (race_table[race].class_mult[class] ?
		       race_table[race].class_mult[class]/100 : 1);

    /* processing */
    points -= 40;

    while (points > 9)
    {
	expl += inc;
        points -= 10;
        if (points > 9)
	{
	    expl += inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;  

    return expl * race_table[race].class_mult[class]/100;
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn,sn,i;
 
    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_help(ch,"group help");
	    return TRUE;
	}

        do_help(ch,argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("You already know that group!\n\r",ch);
		return TRUE;
	    }

	    if (group_table[gn].rating[getClasePr(ch)] < 1)
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }

	    sprintf(buf,"%s group added\n\r",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    ch->gen_data->points_chosen += group_table[gn].rating[getClasePr(ch)];
	    gn_add(ch,gn);
	    ch->pcdata->points += group_table[gn].rating[getClasePr(ch)];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > 0)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].rating[getClasePr(ch)] < 1
	    ||  skill_table[sn].spell_fun != spell_null)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }
	    sprintf(buf, "%s skill added\n\r", NOMBRE_SKILL(sn));
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[getClasePr(ch)];
	    ch->pcdata->learned[sn] = 1;
	    ch->pcdata->points += skill_table[sn].rating[getClasePr(ch)];
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("Group dropped.\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    ch->gen_data->points_chosen -= group_table[gn].rating[getClasePr(ch)];
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    ch->pcdata->points -= group_table[gn].rating[getClasePr(ch)];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[getClasePr(ch)];
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->points -= skill_table[sn].rating[getClasePr(ch)];
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_help(ch,"premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_groups(ch,argument);
	return TRUE;
    }

    return FALSE;
}
	    
	


        

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
    char buf[100];
    int gn,sn,col,tsn,tgsn;
    char tchar;

    if (IS_NPC(ch))
	return;

    col = 0;

    if (argument[0] == '\0')
    {   /* show all groups */
	
	for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    if (group_table[gn].name == NULL)
		break;
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
	    sprintf(buf,"%-20s ",group_table[gn].name);
            send_to_char(buf,ch);
	    if (++col % 3 == 0)
            	send_to_char("\n\r",ch);
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	return;
     }
	
     
     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("No group of that name exist.\n\r",ch);
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	return;
     }

     for (sn = 0; sn < MAX_IN_GROUP; sn++)
     {
	if (group_table[gn].spells[sn] == NULL)
	    break;

	tsn = skill_lookup( group_table[gn].spells[sn] );
	tgsn = group_lookup( group_table[gn].spells[sn] );

	if (tsn != -1)
		tchar = skill_table[tsn].skill_level[getClasePr(ch)] >= LEVEL_IMMORTAL ? 'X' : ' ';
	else
		tchar = ' ';

	sprintf(buf,"%c%c %-20s ",	tsn != -1 ? 'S' : (tgsn != -1 ? 'G' : 'E'),
					tchar,
					group_table[gn].spells[sn]);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	    send_to_char("\n\r",ch);
     }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];
    SpellDesc spell;

    if (IS_NPC(ch)
    ||  IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO)
    ||  EN_ARENA(ch) )
	return;

    charTieneSn(&spell, ch, sn);

    if (spell.nivel >= 0
    ||  skill_table[spell.sn].rating[spell.clase] == 0
    ||  ch->pcdata->learned[spell.sn] == 0
    ||  ch->pcdata->learned[spell.sn] == 100)
	return;  /* skill is not known */ 

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[spell.sn].rating[spell.clase] 
		*	4);
    chance += spell.nivel;

    if (number_range(1,1000) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[spell.sn], 95);
	if (number_percent() < chance)
	{
	    sprintf(buf,"Has mejorado en %s!\n\r", NOMBRE_SKILL(spell.sn));
	    send_to_char(buf,ch);
	    ch->pcdata->learned[spell.sn]++;
	    gain_exp(ch,2 * skill_table[spell.sn].rating[spell.clase]);
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[spell.sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"Aprendes de tus errores, y tu habilidad de %s mejora.\n\r",
		NOMBRE_SKILL(spell.sn) );
	    send_to_char(buf,ch);
	    ch->pcdata->learned[spell.sn] += number_range(1,3);
	    ch->pcdata->learned[spell.sn] = UMIN(ch->pcdata->learned[spell.sn],100);
	    gain_exp(ch,2 * skill_table[spell.sn].rating[spell.clase]);
	}
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;
 
    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;
    
    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	    if (deduct)
	   	ch->pcdata->points += skill_table[sn].rating[getClasePr(ch)]; 
	}
	return;
    }
	
    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
		ch->pcdata->points += group_table[gn].rating[getClasePr(ch)];
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}

// TRUE si el skill sn es de la raza race
bool es_skill_racial( int race, int sn )
{
	int i;

	for ( i = 0; i < MAX_RACE_SKILL; i++ )
		if ( !str_cmp( race_table[race].skills[i], skill_table[sn].name ) )
			return TRUE;

	return FALSE;
}

// TRUE si el jugador puede practicar el skill sn
bool can_prac( CHAR_DATA *ch, int sn )
{
	int skill = war ? get_skill(ch, sn) : (IS_NPC(ch) ? get_skill(ch, sn) : ch->pcdata->learned[sn]);
	bool found = FALSE;
	LEVEL_DATA * lev;

	if ( es_skill_racial( ch->race, sn ) )
		return TRUE;

	if ( skill < 1 )
		return FALSE;

	for ( lev = ch->level_data; lev; lev = lev->next )
		if ( lev->nivel >= skill_table[sn].skill_level[lev->clase] )
		{
			found = TRUE;
			break;
		}

	return found;
}

// retorna el nivel mas bajo en el que tiene el skill sn
int charMinLevelSn(CHAR_DATA *ch, int sn)
{
	LEVEL_DATA *lev;
	int blah = -1;

	for (lev = ch->level_data; lev; lev = lev->next )
		if (blah == -1 || blah > skill_table[sn].skill_level[lev->clase])
			blah = skill_table[sn].skill_level[lev->clase];

	return blah < 0 ? LEVEL_HERO : blah;
}

// si el jugador tiene el skill sn en su nivel actual,
// retorna el nivel mas bajo en el que lo tiene
SpellDesc * charTieneSn(SpellDesc * spell, CHAR_DATA *ch, int sn)
{
	LEVEL_DATA * lev;

	blanquear_spelldesc(spell);

	if ( es_skill_racial(ch->race, sn) )
	{
		spell->sn	= sn;
		spell->nivel	= getNivelPr(ch);
		spell->clase	= getClasePr(ch);
		return spell;
	}

	for ( lev = ch->level_data; lev; lev = lev->next )
		if ( skill_table[sn].skill_level[lev->clase] <= lev->nivel )
		{
			if ( spell->sn == -1 || spell->nivel > skill_table[sn].skill_level[lev->clase] )
			{
				spell->sn	= sn;
				spell->nivel	= lev->nivel;
				spell->clase	= lev->clase;
			}
		}

	return spell;
}

// retorna el nivel mas alto que tiene el jugador
int maxNivel( CHAR_DATA *ch )
{
	int niv = 0;
	LEVEL_DATA * lev;

	for ( lev = ch->level_data; lev; lev = lev->next )
		niv = UMAX(niv, lev->nivel);

	return niv;
}

SpellDesc * blanquear_spelldesc(SpellDesc * spell)
{
static	SpellDesc s_zero = { -1, -1, -1 };

	*spell = s_zero;

	return spell;
}
