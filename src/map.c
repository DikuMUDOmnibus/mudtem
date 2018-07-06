#include "include.h"
#include "lookup.h"

int mapa[10][10];

void llenar_mapa(ROOM_INDEX_DATA *room, int x, int y, int dir)
{
	EXIT_DATA *tmpr;
	int salida = 0;

	mapa[x][y] = 0;

	if ((tmpr = exit_lookup(room, DIR_NORTH)))
	{
		SET_BIT(salida, 1 << (DIR_NORTH + 1));

		if ((dir == -1 || dir != DIR_SOUTH)
		&&   y > 0
		&&   mapa[x][y-1] == -1)
			llenar_mapa(tmpr->u1.to_room, x, y-1, DIR_NORTH);
	}

	if ((tmpr = exit_lookup(room, DIR_SOUTH)))
	{
		SET_BIT(salida, 1 << (DIR_SOUTH + 1));
		
		if ((dir == -1 || dir != DIR_NORTH)
		&&   y < 9
		&&   mapa[x][y+1] == -1)
			llenar_mapa(tmpr->u1.to_room, x, y+1, DIR_SOUTH);
	}

	if ((tmpr = exit_lookup(room, DIR_WEST)))
	{
		SET_BIT(salida, 1 << (DIR_WEST + 1));
		
		if ((dir == -1 || dir != DIR_EAST)
		&&   x > 0
		&&   mapa[x-1][y] == -1)
			llenar_mapa(tmpr->u1.to_room, x-1, y, DIR_WEST);
	}

	if ((tmpr = exit_lookup(room, DIR_EAST)))
	{
		SET_BIT(salida, 1 << (DIR_EAST + 1));

		if ((dir == -1 || dir != DIR_WEST)
		&&   x < 9
		&&   mapa[x+1][y] == -1)
			llenar_mapa(tmpr->u1.to_room, x+1, y, DIR_EAST);
	}

	mapa[x][y] = salida;
}

void do_mapa( CHAR_DATA *ch, char * argument )
{
	int i, j;
	char tlinea1[4], tlinea2[8], tlinea3[4];
	char linea1[MIL], linea2[MIL], linea3[MIL];

	if (IS_SET(ch->in_room->room_flags, ROOM_NOWHERE))
	{
		send_to_char("Estas perdido.\n\r", ch );
		return;
	}

	if (IS_AFFECTED(ch, AFF_BLIND)
	|| !can_see_room(ch, ch->in_room))
	{
		send_to_char("No puedes ver nada!\n\r", ch );
		return;
	}

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			mapa[i][j] = -1;

	llenar_mapa(ch->in_room, 5, 5, -1);

	for (j = 0; j < 7; j++)
	{
		linea1[0] = '\0';
		linea2[0] = '\0';
		linea3[0] = '\0';

		for ( i = 0; i < 10; i++)
		{
			strcpy(tlinea1,"   ");
			if (i == 5 && j == 5)
				strcpy(tlinea2," + ");
			else
				strcpy(tlinea2," * ");
			strcpy(tlinea3,"   ");
	
			if ( mapa[i][j] == -1 )
			{
				strcat(linea1, "   ");
				strcat(linea2, "   ");
				strcat(linea3, "   ");
				continue;
			}

			if ( (1 << (DIR_NORTH + 1)) & mapa[i][j] )
				strcpy(tlinea1," | ");
			if ( (1 << (DIR_SOUTH + 1)) & mapa[i][j] )
				strcpy(tlinea3," | ");
			if ( (1 << (DIR_EAST + 1)) & mapa[i][j] )
				tlinea2[2] = '-';
			if ( (1 << (DIR_WEST + 1)) & mapa[i][j] )
				tlinea2[0] = '-';
			strcat(linea1, tlinea1);
			strcat(linea2, tlinea2);
			strcat(linea3, tlinea3);
		}
		strcat(linea1, "\n\r");
		strcat(linea2, "\n\r");
		strcat(linea3, "\n\r");
		send_to_char(linea1, ch);
		send_to_char(linea2, ch);
		send_to_char(linea3, ch);
	}
}
