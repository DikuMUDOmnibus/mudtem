/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
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
/*  The dicegames HIGHDICE and SEVEN are copyright (C) 1996 Mark Janssen   *
 *  You may do everything to this peace of code as long as you leave the   *
 *  Diku/Merc/Envy license and this text in the code...     --Maniac--     *
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
#include "gamble.h"

const	struct	game_type	game_table	[]	=
{
	{	"game_high_dice",	game_high_dice	},
	{	"game_u_l_t",		game_u_l_t	},
	{	"game_seven",		game_seven	},
	{	NULL,			NULL		}
};

/*
 * Given a name, return the appropriate game fun.
 */
GAME_FUN *game_lookup( const char *name )
{
	int i;

	for ( i = 0; game_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(game_table[i].name[0])
		    &&  !str_prefix( name,game_table[i].name))
			return game_table[i].function;
	}

	return 0;
}

char *game_name( GAME_FUN *function)
{
	int i;

	for (i = 0; game_table[i].function != NULL; i++)
	{
		if (function == game_table[i].function)
			return game_table[i].name;
	}

	return NULL;
}

/*
 *  Menu for all game functions.
 *  Thelonius (Monk)  5/94
 */
void         do_bet (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA   *croupier;

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	    || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
		send_to_char( "Parece que no puedes quebrar el silencio.\n\r", ch );
		return;
	}

	/*
       *  The following searches for a valid croupier.  It allows existing
       *  ACT_GAMBLE mobs to attempt to gamble with other croupiers, but
       *  will not allow them to gamble with themselves (i.e., switched
       *  imms).  This takes care of ch == croupier in later act()'s
       */
	for (croupier = ch->in_room->people;
	    croupier;
	    croupier = croupier->next_in_room)
	{
		if ( IS_NPC( croupier )
		    && (croupier->game_fun != 0 )
		    && !IS_AFFECTED2( croupier, AFF_MUTE )
		    && croupier != ch )
			break;
	}

	if (!croupier)
	{
		send_to_char ("No puedes jugar aqui.\n\r", ch);
		return;
	}

	/* Now for the new game approach... by Maniac */

	(*croupier->game_fun) (ch, croupier, argument);

/*	if ( croupier->game_fun == game_lookup( "game_u_l_t" ) )
		game_u_l_t( ch, croupier, argument );
	if ( croupier->game_fun == game_lookup( "game_high_dice" ) )
		game_high_dice( ch, croupier, argument );
	if ( croupier->game_fun == game_lookup( "game_seven" ) )
		game_seven( ch, croupier, argument ); */

	return;
}

/*
 * Upper-Lower-Triple
 * Game idea by Partan
 * Coded by Thelonius
 */
void         game_u_l_t (CHAR_DATA * ch, CHAR_DATA * croupier, char *argument)
{
	char         msg[MAX_STRING_LENGTH];
	char         buf[MAX_STRING_LENGTH];
	char         limit[MAX_STRING_LENGTH] = "5000";
	char         wager[MAX_INPUT_LENGTH];
	char         choice[MAX_INPUT_LENGTH];
	int          ichoice;
	int          amount;
	int          die1;
	int          die2;
	int          die3;
	int          total;

	argument = one_argument (argument, wager);
	one_argument (argument, choice);

	if (wager[0] == '\0' || !is_number (wager))
	{
		send_to_char ("Cuanto quieres apostar?\n\r", ch);
		return;
	}

	amount = atoi (wager);

	if (amount > ch->gold)
	{
		send_to_char ("No tienes esa cantidad de oro!\n\r", ch);
		return;
	}

	if (amount > atoi (limit))
	{
		act ("$n tells you, 'Lamentablemente, el limite de apuestas es $t.'",
		    croupier, strToEnt(limit,croupier->in_room), chToEnt(ch), TO_VICT);
		ch->reply = croupier;
		return;
	}
	/*
 *  At the moment, the winnings (and losses) do not actually go through
 *  the croupier.  They could do so, if each croupier is loaded with a
 *  certain bankroll.  Unfortunately, they would probably be popular
 *  (and rich) targets.
 */

	if (!str_cmp (choice, "lower"))
		ichoice = 1;
	else if (!str_cmp (choice, "upper"))
		ichoice = 2;
	else if (!str_cmp (choice, "triple"))
		ichoice = 3;
	else
	{
		send_to_char ("A que quieres apostarle: Upper, Lower, o Triple?\n\r",
		    ch);
		return;
	}
	/*
 *  Now we have a wagering amount, and a choice.
 *  Let's place the bets and roll the dice, shall we?
 */
	act ("Pones $t monedas de oro en la mesa, y apuestas '$T'.",
	    ch, strToEnt(wager,ch->in_room), strToEnt(choice,ch->in_room), TO_CHAR);
	act ("$n pone una apuesta contigo.",
	    ch, NULL, chToEnt(croupier), TO_VICT);
	act ("$n juega a los dados.",
	    ch, NULL, chToEnt(croupier), TO_NOTVICT);
	ch->gold -= amount;

	die1 = number_range (1, 6);
	die2 = number_range (1, 6);
	die3 = number_range (1, 6);
	total = die1 + die2 + die3;

	sprintf (msg, "$n lanza los dados: salen %d, %d, y %d",
	    die1, die2, die3);

	if (die1 == die2 && die2 == die3)
	{
		strcat (msg, ".");
		act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

		if (ichoice == 3)
		{
			char         haul[MAX_STRING_LENGTH];

			amount *= 37;
			sprintf (haul, "%d", amount);
			act ("Es un TRIPLE!  Ganas $t monedas de oro!",
			    ch, strToEnt(haul,ch->in_room), NULL, TO_CHAR);
			ch->gold += amount;
		}
		else
			send_to_char ("Es un TRIPLE!  Perdiste!\n\r", ch);

		return;
	}

	sprintf (buf, ", totalizan %d.", total);
	strcat (msg, buf);
	act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

	if (((total <= 10) && (ichoice == 1))
	    || ((total >= 11) && (ichoice == 2)))
	{
		char         haul[MAX_STRING_LENGTH];

		amount *= 2;
		sprintf (haul, "%d", amount);
		act ("Ganas $t monedas de oro!", ch, strToEnt(haul,ch->in_room), NULL, TO_CHAR);
		ch->gold += amount;
	}
	else
		send_to_char ("Que pena, mejor suerte la proxima vez!\n\r", ch);

	return;
}

/*
 * High-Dice
 * Game idea by The Maniac!
 * Coded by The Maniac (based on code from Thelonius for u_l_t)
 *
 * The croupier roll's 2 dice for the player, then he roll's 2 dice
 * for himself. If the player's total is higher he wins his money*2
 * if the player's total is equal to or lower than the croupier's total
 * the bank wins... and the player loses his money
 */
void         game_high_dice (CHAR_DATA * ch, CHAR_DATA * croupier, char *argument)
{
	char         msg[MAX_STRING_LENGTH];
	char         buf[MAX_STRING_LENGTH];
	char         limit[MAX_STRING_LENGTH] = "5000";
	char         wager[MAX_INPUT_LENGTH];
	int          amount;
	int          die1;
	int          die2;
	int          die3;
	int          die4;
	int          total1;
	int          total2;

	argument = one_argument (argument, wager);

	if (wager[0] == '\0' || !is_number (wager))
	{
		send_to_char ("Cuanto quieres apostar?\n\r", ch);
		return;
	}

	amount = atoi (wager);

	/* Does char have enough gold to play the game */
	if (amount > ch->gold)
	{
		send_to_char ("No tienes esa cantidad de oro!\n\r", ch);
		return;
	}

	/* Doesn't the char bet too much */
	if (amount > atoi (limit))
	{
		act ("$n tells you, 'Lo lamento, el limite de las apuestas es $t.'",
		    croupier, strToEnt(limit,croupier->in_room), chToEnt(ch), TO_VICT);
		ch->reply = croupier;
		return;
	}

	/*
 *  At the moment, the winnings (and losses) do not actually go through
 *  the croupier.  They could do so, if each croupier is loaded with a
 *  certain bankroll.  Unfortunately, they would probably be popular
 *  (and rich) targets.
 */

	/*
 *  Now we have a wagering amount...
 *  Let's place the bets and roll the dice, shall we?
 */
	sprintf (msg, "Tira los dados");	/* Because act wants it this way */
	act ("Pones $t monedas de oro en la mesa, y dices '$T'.",
	    ch, strToEnt(wager,ch->in_room), strToEnt(msg,ch->in_room), TO_CHAR);
	act ("$n pone una apuesta contigo.",
	    ch, NULL, chToEnt(croupier), TO_VICT);
	act ("$n juega a los dados.",
	    ch, NULL, chToEnt(croupier), TO_NOTVICT);
	ch->gold -= amount;

	die1 = number_range (1, 6);
	die2 = number_range (1, 6);
	die3 = number_range (1, 6);
	die4 = number_range (1, 6);
	total1 = die1 + die2;
	total2 = die3 + die4;

	sprintf (msg, "$n lanza tus dados: salen %d, y %d",
	    die1, die2);

	sprintf (buf, ", totalizan %d.", total1);
	strcat (msg, buf);
	act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

	sprintf (msg, "$n tira sus dados: salen %d, y %d",
	    die3, die4);

	sprintf (buf, ", totalizan %d.", total2);
	strcat (msg, buf);
	act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

	if (total1 > total2)
	{
		char         haul[MAX_STRING_LENGTH];

		amount *= 2;
		sprintf (haul, "%d", amount);
		act ("Ganas $t monedas de oro!", ch, strToEnt(haul,ch->in_room), NULL, TO_CHAR);
		ch->gold += amount;
	}
	else
		send_to_char ("Que pena, mejor suerte la proxima vez!\n\r", ch);
	return;
}

/*
 * Under and Over Seven
 * Game idea by Maniac
 * Coded by Maniac (with bits from Thelonius)
 *
 * This is a very simple and easy dice game... and the nice thing is...
 * the operator never goes broke (he practically always wins)
 * (better not tell the players...)
 * The player can choose to bet on: Under 7, Seven or Over 7
 * The croupier rolls 2d6 and pays double if the player chose under or
 * over 7 and was correct and the croupier pay's 5x the amount if the player
 * chose SEVEN and was right.
 */

void         game_seven (CHAR_DATA * ch, CHAR_DATA * croupier, char *argument)
{
	char         msg[MAX_STRING_LENGTH];
	char         buf[MAX_STRING_LENGTH];
	char         limit[MAX_STRING_LENGTH] = "5000";
	char         wager[MAX_INPUT_LENGTH];
	char         choice[MAX_INPUT_LENGTH];
	int          ichoice;
	int          amount;
	int          die1;
	int          die2;
	int          total;

	argument = one_argument (argument, wager);
	one_argument (argument, choice);

	if (wager[0] == '\0' || !is_number (wager))
	{
		send_to_char ("Cuanto quieres apostar?\n\r", ch);
		return;
	}

	amount = atoi (wager);

	if (amount > ch->gold)
	{
		send_to_char ("No tienes suficiente oro!\n\r", ch);
		return;
	}

	if (amount > atoi (limit))
	{
		act ("$n te dice, 'Lo lamento, el limite es $t.'",
		    croupier, strToEnt(limit,croupier->in_room), chToEnt(ch), TO_VICT);
		ch->reply = croupier;
		return;
	}
	/*
 *  At the moment, the winnings (and losses) do not actually go through
 *  the croupier.  They could do so, if each croupier is loaded with a
 *  certain bankroll.  Unfortunately, they would probably be popular
 *  (and rich) targets.
 */

	if (!str_cmp (choice, "under"))
		ichoice = 1;
	else if (!str_cmp (choice, "over"))
		ichoice = 2;
	else if (!str_cmp (choice, "seven"))
		ichoice = 3;
	else
	{
		send_to_char ("A que le quieres apostar: Under, Over, o Seven?\n\r",
		    ch);
		return;
	}
	/*
 *  Now we have a wagering amount, and a choice.
 *  Let's place the bets and roll the dice, shall we?
 */
	act ("Pones $t MO en la mesa, y apuestas '$T'.",
	    ch, strToEnt(wager,ch->in_room), strToEnt(choice,ch->in_room), TO_CHAR);
	act ("$n pone una apuesta contigo.",
	    ch, NULL, chToEnt(croupier), TO_VICT);
	act ("$n juega a los dados.",
	    ch, NULL, chToEnt(croupier), TO_NOTVICT);
	ch->gold -= amount;

	die1 = number_range (1, 6);
	die2 = number_range (1, 6);
	total = die1 + die2;

	sprintf (msg, "$n tira los dados: salen %d, y %d",
	    die1, die2);

	if (total == 7)
	{
		strcat (msg, ".");
		act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

		if (ichoice == 3)
		{
			char         haul[MAX_STRING_LENGTH];

			amount *= 5;
			sprintf (haul, "%d", amount);
			act ("Es un SIETE!  Ganaste $t MO!",
			    ch, strToEnt(haul,ch->in_room), NULL, TO_CHAR);
			ch->gold += amount;
		}
		else
			send_to_char ("Es un SIETE!  Perdiste!\n\r", ch);

		return;
	}

	sprintf (buf, ", totalizan %d.", total);
	strcat (msg, buf);
	act (msg, croupier, NULL, chToEnt(ch), TO_VICT);

	if (((total < 7) && (ichoice == 1))
	    || ((total > 7) && (ichoice == 2)))
	{
		char         haul[MAX_STRING_LENGTH];

		amount *= 2;
		sprintf (haul, "%d", amount);
		act ("Ganaste $t MO!", ch, strToEnt(haul,ch->in_room), NULL, TO_CHAR);
		ch->gold += amount;
	}
	else
		send_to_char ("Lo lamento, mejor suerte la proxima vez!\n\r", ch);

	return;
}
