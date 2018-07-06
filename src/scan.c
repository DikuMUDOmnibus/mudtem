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
#include "lookup.h"

char *const distance[4] =
{
   "justo aqui.", "cerca hacia %s.", "no muy lejos hacia %s.", "lejos a la distancia hacia %s."
};

void scan_list args ((ROOM_INDEX_DATA * scan_room, CHAR_DATA * ch,
	  sh_int depth, sh_int door));
void scan_char args ((CHAR_DATA * victim, CHAR_DATA * ch,
	  sh_int depth, sh_int door));

void do_scan (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *scan_room;
	EXIT_DATA *pExit;
	sh_int door, depth;

	if (IS_AFFECTED(ch,AFF_BLIND))
	{
		send_to_char("Maybe it would help if you could see?\n\r",ch);
		return;
	}

	argument = one_argument (argument, arg1);

	if (arg1[0] == '\0')
	{
		act ("$n mira alrededor.", ch, NULL, NULL, TO_ROOM);
		send_to_char ("Mirando alrededor ves:\n\r", ch);
		scan_list (ch->in_room, ch, 0, -1);

		for (door = 0; door < MAX_DIR; door++)
		{
			if ((pExit = exit_lookup(ch->in_room, door)) != NULL)
			{
				if (IS_CLOSED (pExit))
					act ("Hay una puerta cerrada hacia $T.", ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_CHAR);
				else
					scan_list (pExit->u1.to_room, ch, 2, door);
			}
		}
		return;
	}
	else
		door = find_exit (ch, arg1);

	if (door == -1)
	{
		send_to_char ("Hacia que direccion quieres buscar?\n\r", ch);
		return;
	}

	act ("Buscas algo hacia $T.", ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_CHAR);
	act ("$n busca algo hacia $T.", ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_ROOM);
	sprintf (buf, "Mirando hacia %s ves:\n\r", dir_nombre[door]);

	scan_room = ch->in_room;

	for (depth = 1; depth < 4; depth++)
	{
		if ((pExit = exit_lookup(scan_room, door)) != NULL)
		{
			if (IS_CLOSED (pExit))
			{
				act ("Hay una puerta cerrada hacia $T.", ch, NULL, strToEnt(dir_nombre[door],ch->in_room), TO_CHAR);
				break;
			}
			scan_room = pExit->u1.to_room;
			if ( !scan_room || !can_see_room( ch, scan_room ) || room_is_dark(scan_room) )
				break;
			scan_list (pExit->u1.to_room, ch, depth, door);
		}
	}
	return;
}

void scan_list (ROOM_INDEX_DATA * scan_room, CHAR_DATA * ch, sh_int depth,
   sh_int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL)
		return;
	for (rch = scan_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (rch == ch)
			continue;
		if (!IS_NPC (rch) && rch->invis_level > get_trust (ch))
			continue;
		if (can_see (ch, rch))
			scan_char (rch, ch, depth, door);
	}
	return;
}

void scan_char (CHAR_DATA * victim, CHAR_DATA * ch, sh_int depth, sh_int door)
{
	char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

	buf[0] = '\0';

	strcat (buf, PERS (victim, ch));
	strcat (buf, ", ");
	sprintf (buf2, distance[depth], dir_nombre[door]);
	strcat (buf, buf2);
	strcat (buf, "\n\r");

	send_to_char (buf, ch);
	return;
}
