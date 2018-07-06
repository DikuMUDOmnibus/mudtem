/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  EnvyMud 2.0 improvements copyright (C) 1995 by Michael Quan and        *
 *  Mitchell Tse.                                                          *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This code is (C) 1996 The Maniac,
 * Do whatever you want with it, but keep my name in it,
 * and mention that this code is written by me somewhere in
 * your mud...          -=Greetz The Maniac=-
 */

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "language.h"

char    *makedrunk      args( (char *string ,CHAR_DATA *ch) );

/*
 * Lookup a language by name.
 */
int          lang_lookup (const char *name)
{
      int          ln;

      for (ln = 0; ln < MAX_LANGUAGE; ln++)
      {
	    if (!lang_table[ln].name)
		  break;
	    if (LOWER (name[0]) == LOWER (lang_table[ln].name[0])
		&& !str_prefix (name, lang_table[ln].name))
		  return ln;
      }

      return -1;
}

int	get_skill_lang( CHAR_DATA *ch, int lang )
{
      if ( lang == COMMON )
      	return 100;
      
      if ( IS_NPC(ch) )
      {
      	if ( lang == race_table[ch->race].race_lang )
      		return 100;
      	return 0;
      }
      else
	return ch->pcdata->language[lang];
}

void         do_speak (CHAR_DATA * ch, char *argument)
{
      char         arg[MAX_INPUT_LENGTH];
      char         buf[MAX_STRING_LENGTH];
      int          speaking;
      int          canspeak;

      argument = one_argument (argument, arg);

      buf[0] = '\0';

      if (IS_NPC (ch))
      {
	    send_to_char ("Mobs no pueden hablar!\n\r", ch);
	    return;
      }

      if (arg[0] == '\0')
      {
	    sprintf (buf, "Hablas %s actualmente.\n\r", lang_table[ch->pcdata->speaking].name);
	    send_to_char (buf, ch);
      }
      else
      {
	    if ((speaking = lang_lookup (arg)) != -1)
	    {
		  if ((canspeak = ch->pcdata->language[speaking]) == 0)
		  {
			sprintf (buf, "Pero tu no sabes como hablar %s.\n\r", lang_table[speaking].name);
			send_to_char (buf, ch);
		  }
		  else
		  {
			ch->pcdata->speaking = speaking;
			sprintf (buf, "Hablaras %s desde ahora.\n\r", lang_table[ch->pcdata->speaking].name);
			send_to_char (buf, ch);
		  }
	    }
	    else
	    {
		  sprintf (buf, "%s no es un lenguaje valido!\n\r", arg);
		  send_to_char (buf, ch);
	    }
      }
}

void         do_lset (CHAR_DATA * ch, char *argument)
{
      CHAR_DATA   *victim;
      char         arg1[MAX_INPUT_LENGTH];
      char         arg2[MAX_INPUT_LENGTH];
      char         arg3[MAX_INPUT_LENGTH];
      int          value;
      int          ln;
      bool         fAll;

      argument = one_argument (argument, arg1);
      argument = one_argument (argument, arg2);
      one_argument (argument, arg3);

      if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
      {
	    send_to_char ("Syntax: lset <victim> <lang> <value>\n\r", ch);
	    send_to_char ("or:     lset <victim> all    <value>\n\r", ch);
	    send_to_char ("Lang being any language.\n\r", ch);
	    return;
      }

      if (!(victim = get_char_world (ch, arg1)))
      {
	    send_to_char ("They aren't here.\n\r", ch);
	    return;
      }

      if (IS_NPC (victim))
      {
	    send_to_char ("Not on NPC's.\n\r", ch);
	    return;
      }

      if (getNivelPr(ch) <= getNivelPr(victim) && ch != victim)
      {
	    send_to_char ("You may not lset your peer nor your superior.\n\r", ch);
	    return;
      }

      fAll = !str_cmp (arg2, "all");
      ln = 0;
      if (!fAll && (ln = lang_lookup (arg2)) < 0)
      {
	    send_to_char ("No such language.\n\r", ch);
	    return;
      }

      /*
       * Snarf the value.
       */
      if (!is_number (arg3))
      {
	    send_to_char ("Value must be numeric.\n\r", ch);
	    return;
      }

      value = atoi (arg3);
      if (value < 0 || value > 100)
      {
	    send_to_char ("Value range is 0 to 100.\n\r", ch);
	    return;
      }

      if (fAll)
      {
	    if (get_trust (ch) < MAX_LEVEL)
	    {
		  send_to_char ("Only Seniors may lset all.\n\r", ch);
		  return;
	    }
	    for (ln = 0; ln < MAX_LANGUAGE; ln++)
		  if (lang_table[ln].name)
			victim->pcdata->language[ln] = value;
      }
      else
      {
	    victim->pcdata->language[ln] = value;
      }
      return;
}

void         do_lstat (CHAR_DATA * ch, char *argument)
/* lstat by Maniac && Canth */
{
      CHAR_DATA   *victim;
      char         arg[MAX_INPUT_LENGTH];
      char         buf1[MAX_STRING_LENGTH];
      char         buf2[MAX_STRING_LENGTH];
      int          ln;
      int          col;

      one_argument (argument, arg);
      col = 0;

      if (arg[0] == '\0')
      {
	    send_to_char ("lstat a quien?\n\r", ch);
	    return;
      }

      if ((victim = get_char_world (ch, arg)) == NULL)
      {
	    send_to_char ("No esta aqui.\n\r", ch);
	    return;
      }

      if (IS_NPC (victim))
      {
	    send_to_char ("No en NPC's.\n\r", ch);
	    return;
      }

      buf2[0] = '\0';

      for (ln = 0; ln < MAX_LANGUAGE; ln++)
      {
	    if (lang_table[ln].name == NULL)
		  break;
	    sprintf (buf1, "%18s %3d %% ", lang_table[ln].name,
		     victim->pcdata->language[ln]);
	    strcat (buf2, buf1);
	    if (++col % 3 == 0)
		  strcat (buf2, "\n\r");
      }
      if (col % 3 != 0)
	    strcat (buf2, "\n\r");
      sprintf (buf1, "%s tiene %d sesiones de aprendizaje.\n\r", victim->name,
	       victim->pcdata->learn);
      strcat (buf2, buf1);

      send_to_char (buf2, ch);
      return;
}

void         do_learn (CHAR_DATA * ch, char *argument)
{
      char         buf[MAX_STRING_LENGTH];
      char         buf1[MAX_STRING_LENGTH * 2];
      int          ln;
      int          money = getNivelPr(ch) * getNivelPr(ch) * 10;

      if (IS_NPC (ch))
	    return;

      buf1[0] = '\0';

      if (getNivelPr(ch) < 3)
      {
	    send_to_char (
	      "Debes ser de tercer nivel para aprender lenguajes.\n\r", ch);
	    return;
      }

      if (argument[0] == '\0')
      {
	    CHAR_DATA   *mob;
	    int          col;

	    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	    {
		  if (IS_NPC (mob) && IS_SET (mob->act, ACT_TEACHER))
			break;
	    }

	    col = 0;
	    for (ln = 0; ln < MAX_LANGUAGE; ln++)
	    {
		  if (!lang_table[ln].name)
			break;

		  if ((mob) || (ch->pcdata->language[ln] > 0))
		  {
			sprintf (buf, "%18s %3d%%  ",
			     lang_table[ln].name, ch->pcdata->language[ln]);
			strcat (buf1, buf);
			if (++col % 3 == 0)
			      strcat (buf1, "\n\r");
		  }
	    }

	    if (col % 3 != 0)
		  strcat (buf1, "\n\r");

	    sprintf (buf, "Te quedan %d lecciones.\n\r",
		     ch->pcdata->learn);
	    strcat (buf1, buf);
	    sprintf (buf, "Costo de las lecciones es %d MP.\n\r", money);
	    strcat (buf1, buf);
	    send_to_char (buf1, ch);
      }
      else
      {
	    CHAR_DATA   *mob;
	    int          adept;

	    if (!IS_AWAKE (ch))
	    {
		  send_to_char ("En tus suenos, o que?\n\r", ch);
		  return;
	    }

	    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	    {
		  if (IS_NPC (mob) && IS_SET (mob->act, ACT_TEACHER))
			break;
	    }

	    if (!mob)
	    {
		  send_to_char ("No puedes hacer eso aqui.\n\r", ch);
		  return;
	    }

	    if (ch->pcdata->learn <= 0)
	    {
		  send_to_char ("No te quedan lecciones.\n\r", ch);
		  return;
	    }
	    else if (money > ch->silver)
	    {
		  send_to_char ("No tienes dinero suficiente para tomar lecciones.\n\r", ch);
		  return;
	    }

	    if ((ln = lang_lookup (argument)) < 0)
	    {
		  send_to_char ("No puedes practicar eso.\n\r", ch);
		  return;
	    }


	    adept = UMIN(100,(get_curr_stat (ch, STAT_INT) * 5)); /* Max learned = int*5 */

	    if (ch->pcdata->language[ln] >= adept)
	    {
		  sprintf (buf, "Ya eras experto en %s.\n\r",
			   lang_table[ln].name);
		  send_to_char (buf, ch);
	    }
	    else
	    {
		  ch->pcdata->learn--;
		  ch->silver -= money;
		  ch->pcdata->language[ln] += int_app[get_curr_stat (ch, STAT_INT)].learn;
		  if (ch->pcdata->language[ln] < adept)
		  {
			act ("Tomas lecciones de $T.",
			     ch, NULL, strToEnt(lang_table[ln].name,ch->in_room), TO_CHAR);
			act ("$n practica $T.",
			     ch, NULL, strToEnt(lang_table[ln].name,ch->in_room), TO_ROOM);
		  }
		  else
		  {
			ch->pcdata->language[ln] = adept;
			act ("Ahora eres experto en $T.",
			     ch, NULL, strToEnt(lang_table[ln].name,ch->in_room), TO_CHAR);
			act ("$n es ahora experto en $T.",
			     ch, NULL, strToEnt(lang_table[ln].name,ch->in_room), TO_ROOM);
		  }
	    }
      }
      return;
}

void         do_common (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, COMMON);
}

void         do_human (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, HUMAN);
}

void         do_dwarvish (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, DWARVISH);
}

void         do_elvish (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, ELVISH);
}

void         do_gnomish (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, GNOMISH);
}

void         do_goblin (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, GOBLIN);
}

void         do_orcish (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, ORCISH);
}

void         do_ogre (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, OGRE);
}

void         do_drow (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, DROW);
}

void         do_kobold (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, KOBOLD);
}

void         do_trollish (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, TROLLISH);
}

void         do_hobbit (CHAR_DATA * ch, char *argument)
{
      do_language (ch, argument, HOBBIT);
}

/* ========== Language ======================== */
void         do_language (CHAR_DATA * ch, char *argument, int language)
{
      CHAR_DATA   *och;
      int          chance;
      int          chance2;
      char        *lan_str;
      char         buf[256];

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
		send_to_char("Los dioses han eliminado tus privilegios de uso de canales.\n\r",ch);
		return;
	}

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	|| IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
		send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
		return;
	}

	argument = makedrunk ( (char *) argument , ch);

      lan_str = lang_table[language].name;

      buf[0] = '\0';
      /* Now find out if we can speak it... */
      if ((chance = get_skill_lang(ch,language)) == 0)
      {
	    sprintf (buf, "No sabes hablar %s.\n\r", lan_str);
	    send_to_char (buf, ch);
	    return;
      }

      if (argument[0] == '\0')
      {
	    buf[0] = '\0';
	    sprintf (buf, "Decir QUE en %s ??\n\r", lan_str);
	    send_to_char (buf, ch);
	    return;
      }

      if (number_percent () <= chance)
      {
	    buf[0] = '\0';
	    sprintf (buf, "En %s, dices '%s'#n\n\r", lan_str, argument);
	    send_to_char (buf, ch);
	    for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
	    {
		  if (!IS_NPC (och) && (och != ch) && IS_AWAKE(och))
		  {
			if ((chance2 = get_skill_lang(och,language)) == 0)
			      act ("$n dice algo en una lengua desconocida.", ch, NULL, chToEnt(och), TO_VICT);
			else if (number_percent () <= chance2)
			{
			      buf[0] = '\0';
			      sprintf (buf, "En %s, %s dice, '%s'#n\n\r", lan_str, PERS (ch, och), argument);
			      send_to_char (buf, och);
			}
			else
			{
			      buf[0] = '\0';
			      sprintf (buf, "En %s, %s dice algo que no puedes entender.\n\r", lan_str, PERS (ch, och));
			      send_to_char (buf, och);
			}
		  }
	    }
      }
      else
      {
	    buf[0] = '\0';
	    sprintf (buf, "En %s, intentas decir '%s'#n, pero no suena correctamente.\n\r", lan_str, argument);
	    send_to_char (buf, ch);
	    for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
	    {
		  if (!IS_NPC (och) && (och != ch) && IS_AWAKE(och))
		  {
			if ((chance2 = get_skill_lang(och,language)) == 0)
			      act ("$n dice algo en una lengua rara.", ch, NULL, chToEnt(och), TO_VICT);
			else if (number_percent () <= chance2)
			{
			      buf[0] = '\0';
			      sprintf (buf, "En una rara forma de %s, %s dice algo incomprensible.\n\r", lan_str, PERS (ch, och));
			      send_to_char (buf, och);
			}
			else
			{
			      buf[0] = '\0';
			      sprintf (buf, "En %s, %s dice algo que no puedes entender.\n\r", lan_str, PERS (ch, och));
			      send_to_char (buf, och);
			}
		  }
	    }
      }
}
