#include "include.h"
#include <time.h>
#include "lookup.h"
#include "tables.h"
#include "events.h"
#include "plist.h"
#include "recycle.h"
#include "tablesave.h"

void grabar_plist(void);

PLIST * player_list[26];

PLIST * plist_lookup( const char * str )
{
	PLIST * temp;

	for ( temp = player_list[HASHKEY(str[0])]; temp; temp = temp->next )
		if ( !str_cmp(temp->name, str) )
			return temp;

	return NULL;
}

PLIST * plist_lookup_id( long id )
{
	PLIST * temp;
	int key;

	for ( key = 0; key < 26; ++key )
		for ( temp = player_list[key]; temp; temp = temp->next )
			if ( temp->id == id )
				return temp;

	return NULL;
}

void save_plist_event( EVENT *ev )
{
	grabar_plist();
	grabar_clanes();
	grabar_puntajes();

	generic_event_add( PULSE_SAVEPLIST, NULL, save_plist_event );
}

void new_player( CHAR_DATA *ch )
{
	PLIST * pl;

	pl		= new_plist();
	pl->name	= str_dup( ch->name );
	pl->id		= ch->id;
	pl->nivel	= getNivelPr(ch);
	pl->clan	= ch->clan;
	pl->clan_status	= ch->pcdata->clan_status;
	pl->sex		= ch->sex;
	pl->class	= getClasePr(ch);
	pl->race	= ch->race;
	pl->lastlog	= current_time;
	pl->host	= ch->desc ? str_dup( ch->desc->host ) : "NULL";
#if defined(CRC)
	pl->crc		= -1;
#endif

	pl->next				= player_list[HASHKEY(ch->name[0])];
	player_list[HASHKEY(ch->name[0])]	= pl;
	ch->pcdata->pdata			= pl;
	grabar_plist();
}

void update_player( CHAR_DATA *ch )
{
	PLIST * pl = plist_lookup( ch->name );

	if ( war == TRUE )
		return;

	if ( !pl )
	{
		new_player( ch );
		return;
	}

	ch->pcdata->pdata	= pl;
	pl->nivel		= getNivelPr(ch);
	if (pl->id != ch->id)
	{
		flog( "Actualizando ID jugador %s, id %d -> %d",
			ch->name, pl->id, ch->id );
		pl->id = ch->id;
	}
	pl->race		= ch->race;
	pl->clan		= ch->clan;
	pl->clan_status		= ch->pcdata->clan_status;
	pl->lastlog		= current_time;

	if ( IS_NULLSTR(pl->host) && ch->desc )
	{
		if (pl->host)
			free_string(pl->host);
		pl->host	= str_dup( ch->desc->host );
	}

	return;
}

void player_delete( char * str )
{
	PLIST *plx, *pl;

	pl = plist_lookup(str);

	if ( !pl )
	{
		bugf( "player_delete : jugador %s inexistente",
			str );
		return;
	}

	plx = player_list[HASHKEY(str[0])];

	if ( pl == plx )
		player_list[HASHKEY(str[0])] = plx->next;
	else
	{
		for ( ; plx; plx = plx->next )
			if ( plx->next == pl )
				break;

		if ( !plx )
		{
			bugf( "player_delete : jugador %s inexistente", pl->name );
			return;
		}

		plx->next = pl->next;
	}

	free_plist( pl );
	return;
}

void do_plist( CHAR_DATA *ch, char * argument )
{
	PLIST * pl;
	int key, minlev = -1, maxlev = -1, clan = -1, clase = -1, raza = -1, cnt = 0, level = 0;
	char buftime[MIL], buf[MIL];
	BUFFER * buffer;
	bool primero = TRUE;

	if ( !IS_NULLSTR(argument) )
	{
		while( *argument )
		{
			argument = one_argument( argument, buf );

			if ( clan == -1 )
			{
				clan = clan_lookup( buf );

				if ( clan == 0 )
					clan = -1;
			}

			if ( clase == -1 )
				clase = class_lookup( buf );

			if ( raza == -1 )
			{
				raza = race_lookup( buf );

				if ( raza == 0 )
					raza = -1;
			}

			if ( is_number(buf) )
			{
				if (primero)
				{
					minlev = atoi(buf);
					primero = FALSE;
				}
				else
					maxlev = atoi(buf);
			}
		} /* while */
	}

	if (minlev == -1)
		minlev = 0;
	else
	if (maxlev == -1)
		maxlev = minlev;

	if (maxlev == -1)
		maxlev = 60;

	buffer = new_buf();

	sprintf( buf, "#U%-10.10s %-2.2s %-10.10s %-10.10s %-5.5s %-3.3s %-3.3s %-7.7s#u\n\r",
		"Nombre", "Ni", "Clan", "ClanSt", "Sexo", "Cla", "Raz", "LastLog" );
	add_buf( buffer, buf );

	for ( key = 0; key < 26; key++ )
		for ( pl = player_list[key]; pl; pl = pl->next )
			if ( (clan == -1 || pl->clan == clan)
			&&   (clase == -1 || pl->class == clase)
			&&   (raza == -1 || pl->race == raza)
			&&   ENTRE(minlev - 1, pl->nivel, maxlev + 1) )
			{
				cnt++;
				level += pl->nivel;
				strftime( buftime, MIL, "%d/%b", localtime(&pl->lastlog) );
				sprintf( buf, "#B%-10.10s#b %-2.2d %-10.10s %-10.10s %-5.5s %-3.3s %-3.3s %-7.7s\n\r",
					pl->name,
					pl->nivel,
					pl->clan > 0 ? get_clan_table(pl->clan)->name : "",
					(pl->clan > 0 && !ES_INDEP(pl->clan)) ? lookup_clan_status(pl->clan_status) : "",
					sex_table[pl->sex].name,
					class_table[pl->class].name,
					race_table[pl->race].name,
					buftime );
				add_buf( buffer, buf );
			}

	sprintf( buf, "Total de jugadores : #B%d#b.\n\r", cnt );
	add_buf( buffer, buf );

	sprintf( buf, "Promedio de niveles: #B%f#b.\n\r",
		cnt > 0 ? (float) level/cnt : 0.0 );
	add_buf( buffer, buf );

	page_to_char( buf_string(buffer), ch );
	free_buf( buffer );

	return;
}

void clean_pl( CHAR_DATA *ch, PLIST * pl, char * buf )
{
	nuke_corpse( pl->name, TRUE );
	unlink( buf );
	printf_to_char( ch, "%s...borrado.\n\r", pl->name );
	flog( "%s...borrado.", pl->name );
	player_delete(pl->name);
}

void do_limpiar( CHAR_DATA *ch, char *argument )
{
	PLIST * pl, * pl_next;
	int i;
	char buf[MIL];
	FILE *fp;

	for ( i = 0; i < 26; i++ )
		for ( pl = player_list[i]; pl; pl = pl_next )
		{
			pl_next = pl->next;
			sprintf( buf, PLAYER_DIR "%s", capitalize(pl->name) );

			if ( get_char_from_id(pl->id) != NULL )
				continue;

			if ( pl->nivel == 1 && difftime(current_time, pl->lastlog) > DIAS(30) )
			{
				clean_pl(ch, pl, buf);
				continue;
			}

			if ( (fp = fopen( buf, "r" )) == NULL )
			{
				clean_pl(ch, pl, buf);
				continue;
			}
			else
				fclose(fp);
		}

	return;
}
