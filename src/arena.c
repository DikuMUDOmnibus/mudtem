#include "include.h"
#include "plist.h"
#include "arena.h"
#include "tables.h"
#include "recycle.h"

#include <stdarg.h>

COMMAND(do_look)
COMMAND(do_retar)
COMMAND(do_aceptar)
COMMAND(do_rechazar)
COMMAND(do_apostar)
COMMAND(do_info)
COMMAND(do_arena_probar)

void arena_reset_char( CHAR_DATA *, int, int );

CHAR_DATA * retador( CHAR_DATA * );

#define ARENA_PERS_DEFAULT 0

struct arena_type arena_table [] =
{
	{	"chica",	2,	2,	3037,	3037,	ARENA_LIBRE,	ARENA_PERSONAL, NULL,	0	},
	{	"personal",	2,	2,	3370,	3373,	ARENA_LIBRE,	ARENA_PERSONAL, NULL,	0	},
	{	"multi",	2,	4,	3360,	3368,	ARENA_LIBRE,	ARENA_NONE,	NULL,	0	},
	{	"grande",	2,	20,	22000,	22048,	ARENA_LIBRE,	ARENA_ETERNA,	NULL,	0	},
	{	NULL,		0,	0,	0,	0,	ARENA_LIBRE,	ARENA_NONE,	NULL,	0	}
};

void inscribir_jugador( CHAR_DATA *ch, int temp )
{
	ALIST *al;

	al				= new_alist();
	al->ch				= ch;
	al->frags			= 0;
	al->apuestas			= 0;
	al->curhit			= ch->hit;
	al->curmana			= ch->mana;
	al->curmove			= ch->move;

	al->next			= arena_table[temp].jugadores;
	arena_table[temp].jugadores	= al;
}

void send_to_alist (int tmp, char * fmt, ...)
{
	char buf [MIL];
	ALIST * al;

	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	for ( al = arena_table[tmp].jugadores; al; al = al->next )
		send_to_char( buf, al->ch );
}

ALIST * en_alista( CHAR_DATA *ch, ALIST * al )
{
	for ( ; al; al = al->next )
		if ( al->ch == ch )
			return al;

	return NULL;
}

int	count_alista( ALIST * al )
{
	int cnt = 0;

	for ( ; al; al = al->next )
		cnt++;

	return cnt;
}

int arena_lookup ( const char * arg )
{
	int i;

	for ( i = 0; arena_table[i].name; i++ )
		if ( !str_prefix( arg, arena_table[i].name ) )
			return i;

	return -1;
}

int esta_inscrito( CHAR_DATA * ch )
{
	int i;

	for ( i = 0; arena_table[i].name; i++ )
		if ( en_alista( ch, arena_table[i].jugadores ) )
			return i;

	return -1;
}

char * alist_to_list( ALIST * al )
{
	static char buf[MIL];

	buf[0] = '\0';

	for ( ; al; al = al->next )
	{
		strcat( buf, al->ch->name );
		if ( al->next )
			strcat( buf, ", " );
		else
			strcat( buf, "." );
	}

	return buf;
}

void do_arenastatus( CHAR_DATA *ch, char *argument )
{
	int temp = 0;
	char buf[MIL];
	ALIST * al;

	if ( (temp = arena_room_lookup(ch->in_room->vnum)) == -1 )
	{
		if ( IS_NULLSTR(argument)
		||  (temp = arena_lookup(argument)) == -1 )
		{
			send_to_char( "Esa arena no existe.\n\r", ch );
			return;
		}
	}

	sprintf( buf,	"Numero minimo de jugadores : %d\n\r"
			"Numero actual de jugadores : %d\n\r"
			"Numero maximo de jugadores : %d\n\r"
			"Jugadores :\n\r", arena_table[temp].minjug,
					   count_alista(arena_table[temp].jugadores),
					   arena_table[temp].maxjug );
	send_to_char( buf, ch );

	sprintf( buf, "#UNombre     Nivel Clase Raza Frags#u\n\r" );
	send_to_char( buf, ch );

	for ( al = arena_table[temp].jugadores; al; al = al->next )
	{
		sprintf( buf, "%-10.10s %-5.5d %-5.5s %-4.4s %d\n\r",
			al->ch->name,
			getNivelPr(al->ch),
			class_table[getClasePr(al->ch)].name,
			race_table[al->ch->race].name,
			al->frags );
		send_to_char( buf, ch );
	}

	return;
}

void do_arenalist( CHAR_DATA *ch, char *argument )
{
	int i;
	char buf[MIL];

	sprintf( buf, "#U%-10.10s Min Max Act St %-51.51s#u\n\r", "Nombre", "Jugadores" );
	send_to_char( buf, ch );

	for ( i = 0; arena_table[i].name; i++ )
	{
		sprintf( buf, "%-10.10s %3d %3d %3d %c  %-51.51s\n\r",
			arena_table[i].name,
			arena_table[i].minjug,
			arena_table[i].maxjug,
			count_alista(arena_table[i].jugadores),
			arena_table[i].status == ARENA_LIBRE ? 'L' : 'O',
			alist_to_list(arena_table[i].jugadores) );
		send_to_char( buf, ch );
	}

	return;
}

void do_inscribir( CHAR_DATA *ch, char * argument )
{
	int temp;
	CHAR_DATA *victim;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : arena inscribir [arena]\n\r", ch );
		return;
	}

	if ( IS_IMMORTAL(ch) )
	{
		send_to_char( "Sabes que vas a ganar.\n\r", ch );
		return;
	}

	if ( getNivelPr(ch) < 5 )
	{
		send_to_char( "Eres de nivel demasiado bajo.\n\r", ch );
		return;
	}

	if ( esta_inscrito( ch ) != -1 )
	{
		send_to_char( "Ya estas inscrito.\n\r", ch );
		return;
	}

	if ( ch->pcdata->retando )
	{
		act( "Estas retando a $N. Espera su respuesta.", ch, NULL, chToEnt(ch->pcdata->retando), TO_CHAR );
		return;
	}

	if ( (victim = retador(ch)) )
	{
		act( "$N te reto. Acepta o rechaza.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	if ( (temp = arena_lookup( argument )) == -1 )
	{
		send_to_char( "Esa arena no existe.\n\r", ch );
		return;
	}
	
	if ( count_alista(arena_table[temp].jugadores) >= arena_table[temp].maxjug )
	{
		send_to_char( "Esa arena esta llena.\n\r", ch );
		return;
	}

	if ( arena_table[temp].status != ARENA_LIBRE
	&&  !IS_SET(arena_table[temp].flags, ARENA_ETERNA) )
	{
		send_to_char( "Esa arena esta ocupada.\n\r", ch );
		return;
	}

	send_to_alist( temp, "[#BARENA#b] #B%s#b se inscribe para la Arena.\n\r", ch->name );

	inscribir_jugador( ch, temp );

	send_to_char( "Ok. Ahora estas inscrito.\n\r", ch );
	act( "$n se inscribe para la arena $t.", ch, strToEnt(arena_table[temp].name,ch->in_room), NULL, TO_ROOM );

	if ( IS_SET(arena_table[temp].flags, ARENA_ETERNA)
	&&   arena_table[temp].status != ARENA_LIBRE )
		arena_reset_char(ch, number_range(arena_table[temp].desde, arena_table[temp].hasta), 0 );
}

void do_retirar( CHAR_DATA *ch, char *argument )
{
	ALIST * al;
	int temp;

	if ( (temp = esta_inscrito(ch)) == -1 )
	{
		send_to_char( "No estas inscrito.\n\r", ch );
		return;
	}

	if ( arena_table[temp].status != ARENA_LIBRE
	&&  !IS_SET(arena_table[temp].flags, ARENA_ETERNA) )
	{
		send_to_char( "Demasiado tarde.\n\r", ch );
		return;
	}

	if ( (al = en_alista(ch, arena_table[temp].jugadores )) == NULL )
	{
		send_to_char( "???\n\r", ch );
		bugf( "do_retirar : jugador %s no esta en lista, arena %d", ch->name, temp );
		return;
	}

	arena_table[temp].jugadores = extract_alist( al, arena_table[temp].jugadores );

	send_to_alist( temp, "[#BARENA#b] #B%s#b se retira de la Arena.\n\r", ch->name );

	send_to_char( "Ok.\n\r", ch );

	if ( IS_SET(arena_table[temp].flags, ARENA_ETERNA) )
		arena_reset_char( ch, ROOM_VNUM_ALTAR, 0 );

	if ( arena_table[temp].jugadores == NULL )
		arena_table[temp].status = ARENA_LIBRE;
}

ALIST * extract_alist( ALIST *al, ALIST *lista )
{
	if ( al == lista )
		lista = lista->next;
	else
	{
		ALIST * alt;

		for ( alt = lista; alt; alt = alt->next )
			if ( alt->next == al )
				break;

		if ( !alt )
		{
			bugf( "extract_alist : lista invalida, char %s", al->ch->name );
			return lista;
		}

		alt->next = al->next;
	}

	free_alist(al);

	return lista;
}

void arena_reset_char( CHAR_DATA *ch, int vnum, int reset )
{
	AFFECT_DATA *paf, *paf_next;
	int pos;
	const struct flag_type * flagt;
	int indice = esta_inscrito(ch);
	ALIST * al = indice != -1 ? en_alista(ch,arena_table[indice].jugadores) : NULL;

	if ( reset > 0 )
	{
		if ( reset == 2 )
			for ( paf = ch->affected; paf; paf = paf_next )
			{
				paf_next = paf->next;

				if ( paf->type == TO_AFFECTS
				||   paf->type == TO_AFFECTS2 )
				{
					flagt = (paf->type == TO_AFFECTS) ? affect_flags : affect2_flags;

					pos = flagtablepos( flagt, paf->bitvector );

					if ( pos != -1 && flagt[pos].settable == FALSE )
						continue;
				}

				affect_remove( ch, paf );
			}

		if (al)
		{
			ch->hit			= al->curhit;
			ch->mana		= al->curmana;
			ch->move		= al->curmove;
		}
		else
		{
			ch->hit		= ch->max_hit;
			ch->mana	= ch->max_mana;
			ch->move	= ch->max_move;
		}
	}

	if (ch->fighting)
		stop_fighting(ch, FALSE);

	update_pos(ch);

	if ( vnum )
	{
		char_from_room(ch);
		char_to_room(ch, get_room_index(vnum) );
		do_look(ch, "auto");
	}
}

void do_arena_listo( CHAR_DATA *ch, char *argument )
{
	ALIST * al;
	int temp;

	if ( (temp = esta_inscrito(ch)) == -1 )
	{
		send_to_char( "No estas inscrito.\n\r", ch );
		return;
	}

	if ( arena_table[temp].status != ARENA_LIBRE )
	{
		send_to_char( "Demasiado tarde.\n\r", ch );
		return;
	}

	if ( arena_table[temp].minjug > count_alista(arena_table[temp].jugadores) )
	{
		printf_to_char( ch, "Faltan %d jugadores para poder empezar la batalla.\n\r",
			arena_table[temp].minjug - count_alista(arena_table[temp].jugadores) );
		return;
	}

	if ( !can_recall(ch) )
	{
		send_to_char( "Debes poder hacer recall.\n\r", ch );
		return;
	}

	if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
	{
		send_to_char( "Debes estar en un cuarto seguro.\n\r", ch );
		return;
	}

	send_to_alist( temp, "[#BARENA#b] #B%s#b empieza la batalla.\n\r", ch->name );

	for ( al = arena_table[temp].jugadores; al; al = al->next )
		arena_reset_char( al->ch, number_range(arena_table[temp].desde, arena_table[temp].hasta), 0 );

	arena_table[temp].status = ARENA_OCUPADA;
}

void do_arena( CHAR_DATA *ch, char * argument )
{
	char arg[MIL];

	if ( IS_NPC(ch) )
		return;

	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "jugar" ) )
	{
		do_arena_listo( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "inscribir" ) )
	{
		do_inscribir( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "retirar" ) )
	{
		do_retirar( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "lista" ) )
	{
		do_arenalist( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "status" ) )
	{
		do_arenastatus( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "retar" ) )
	{
		do_retar( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "aceptar" ) )
	{
		do_aceptar( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "rechazar" ) )
	{
		do_rechazar( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "apostar" ) )
	{
		do_apostar( ch, argument );
		return;
	}

	if ( !str_cmp( arg, "probar" ) )
	{
		do_arena_probar( ch, argument );
		return;
	}

	send_to_char( "Sintaxis : arena lista\n\r", ch );
	send_to_char( "           arena status\n\r", ch );
	send_to_char( "           arena inscribir [arena]\n\r", ch );
	send_to_char( "           arena retirar\n\r", ch );
	send_to_char( "           arena jugar\n\r", ch );
	send_to_char( "           arena retar [jugador]\n\r", ch );
	send_to_char( "           arena retar anular\n\r", ch );
	send_to_char( "           arena aceptar\n\r", ch );
	send_to_char( "           arena rechazar\n\r", ch );
	send_to_char( "           arena apostar [jugador] [cantidad]\n\r", ch );
	send_to_char( "           arena probar [nombre monstruo]\n\r", ch );

	return;
}

void arena_reset( int temp )
{
	ALIST * atmp, * atmpnext;

	for ( atmp = arena_table[temp].jugadores; atmp; atmp = atmpnext )
	{
		atmpnext = atmp->next;
		arena_table[temp].jugadores = extract_alist(atmp, arena_table[temp].jugadores);
	}

	arena_table[temp].status	= ARENA_LIBRE;
}

void arena_apuestas_check(int temp, CHAR_DATA *ganador, CHAR_DATA *perdedor, int apuestas)
{
	CHAR_DATA *ch;
	float porcentaje;
	int ganado;

	for ( ch = char_list; ch; ch = ch->next )
		if ( !IS_NPC(ch) && ch->pcdata->apostando )
		{
			if (ch->pcdata->apostando == ganador)
			{
				porcentaje = (float) ch->pcdata->apuesta / apuestas;
				ganado = arena_table[temp].pozo * porcentaje;

				printf_to_char( ch, "Ganas %d monedas de oro!\n\r", ganado );
				ch->gold += ganado;
				ch->pcdata->apostando = NULL;
				ch->pcdata->apuesta = 0;
			}
			else
			if (ch->pcdata->apostando == perdedor)
			{
				send_to_char( "Perdiste!\n\r", ch );
				ch->pcdata->apostando = NULL;
				ch->pcdata->apuesta = 0;
			}
		}
}

void check_arena( CHAR_DATA * ch, CHAR_DATA * victim )
{
	ALIST * al = NULL, *tmpal = NULL;
	int temp = esta_inscrito(ch);

	if (temp == -1)
	{
		temp = esta_inscrito(victim);

		if (temp == -1)
		{
			bugf("check_arena : ni ch %s ni vict %s estan inscritos",
				ch->name, victim->name );
			if (IS_NPC(victim))
				extract_char(victim,TRUE);
			return;
		}
	}

	send_to_alist( temp, "[#BARENA#b] #B%s#b ha muerto a manos de #B%s#b.\n\r", victim->name, ch->name );

	stop_fighting(ch, TRUE);
	stop_fighting(victim, TRUE);

	al = en_alista( ch, arena_table[temp].jugadores );
	tmpal = en_alista( victim, arena_table[temp].jugadores );

	if ( IS_SET(arena_table[temp].flags, ARENA_ETERNA) )
	{
		if (!IS_NPC(ch))
		{
			if (al)
				al->frags++;
			else
				bugf( "check_arena : al NULL, char %s, vict %s", ch->name, victim->name );
		}
		if (!IS_NPC(victim))
			arena_reset_char(victim, number_range(arena_table[temp].desde, arena_table[temp].hasta), 2 );
		else
			extract_char(victim,TRUE);
		return;
	}

	if ( IS_SET(arena_table[temp].flags, ARENA_PERSONAL) )
	{
		char buf[MIL];

		if ( !IS_NPC(ch) && !IS_NPC(victim) )
		{
			sprintf( buf, "[#BARENA#b] #B%s#b ha derrotado a #B%s#b!\n\r",
				ch->name, victim->name );
			do_info( NULL, buf );
		}

		if (!IS_NPC(ch))
			ch->pcdata->victorias++;
		if (!IS_NPC(victim))
			victim->pcdata->derrotas++;

		arena_apuestas_check(temp, ch, victim, al->apuestas);
		arena_reset(temp);
		if (IS_NPC(ch))
			extract_char(ch, TRUE);
		else
			arena_reset_char(ch, ROOM_VNUM_AWINNER, 1);
		if (IS_NPC(victim))
			extract_char(victim, TRUE);
		else
			arena_reset_char(victim, ROOM_VNUM_ALOSER, 1);
		return;
	}

	if (IS_NPC(victim))
		extract_char(victim,TRUE);
	else
		arena_reset_char(victim, ROOM_VNUM_ALTAR, 2);

	if (tmpal)
		arena_table[temp].jugadores = extract_alist( tmpal, arena_table[temp].jugadores );

	if ( count_alista(arena_table[temp].jugadores) == 1 )
	{
		send_to_char( "Eres el #B#FVENCEDOR#f#b!!!\n\r", ch );
		arena_reset_char(ch, ROOM_VNUM_ALTAR, 2);
		arena_table[temp].jugadores = extract_alist( arena_table[temp].jugadores, arena_table[temp].jugadores );
		arena_table[temp].status = ARENA_LIBRE;
		return;
	}

	return;
}

int arena_room_lookup( int vnum )
{
	int i;

	for ( i = 0; arena_table[i].name; i++ )
		if ( ENTRE_I(arena_table[i].desde, vnum, arena_table[i].hasta) )
			return i;

	return -1;
}

void arena_quit_handler( CHAR_DATA *ch, int temp )
{
	ALIST * al = en_alista(ch, arena_table[temp].jugadores);

	if ( !al )
	{
		bugf( "arena_quit_handler : al NULL, ch %s", ch->name );
		return;
	}

	arena_table[temp].jugadores = extract_alist( al, arena_table[temp].jugadores );

	if ( arena_table[temp].status != ARENA_LIBRE
	&&   count_alista(arena_table[temp].jugadores) == 1 )
	{
		send_to_char( "Ganaste por abandono.\n\r", arena_table[temp].jugadores->ch );
		arena_reset_char( arena_table[temp].jugadores->ch, ROOM_VNUM_ALTAR, 1 );
		arena_reset(temp);
	}
}

CHAR_DATA *retador( CHAR_DATA *ch )
{
	CHAR_DATA *temp;

	for ( temp = char_list; temp; temp = temp->next )
		if ( !IS_NPC(temp) && temp->pcdata->retando == ch )
			return temp;

	return NULL;
}

void do_arena_probar( CHAR_DATA *ch, char * argument )
{
	CHAR_DATA *victim;
	ALIST * al;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : #Barena probar [nombre monstruo]#n\n\r", ch );
		return;
	}

	if ( (victim = get_char_world(ch, argument)) == NULL )
	{
		send_to_char( "Ese monstruo no existe.\n\r", ch );
		return;
	}

	if ( !IS_NPC(victim) )
	{
		act( "$N es un jugador, no un monstruo.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	if ( IS_SET(victim->act, ACT_PROTOTIPO) )
	{
		act( "$N no puede pelear todavia.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	if ( DINERO(ch) < getNivelPr(victim)*1000 )
	{
		send_to_char( "No tienes suficiente dinero!\n\r", ch );
		return;
	}

	if ( arena_table[ARENA_PERS_DEFAULT].jugadores != NULL )
	{
		send_to_char( "Espera a que se desocupe la arena personal.\n\r", ch );
		return;
	}

	if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
	{
		send_to_char( "Debes estar en cuarto seguro antes de entrar a pelear.\n\r", ch );
		return;
	}

	if ( !can_recall(ch) )
	{
		send_to_char( "Debes poder hacer recall.\n\r", ch );
		return;
	}

	if (ch->hit < ch->max_hit)
	{
		send_to_char( "Debes estar perfectamente sano para poder entrar a la arena.\n\r", ch );
		return;
	}

	deduct_cost(ch, getNivelPr(victim)*1000);

	victim = create_mobile(victim->pIndexData);
	char_to_room(victim, get_room_index(ROOM_VNUM_LIMBO));

	inscribir_jugador(ch, ARENA_PERS_DEFAULT);
	inscribir_jugador(victim, ARENA_PERS_DEFAULT);

	for ( al = arena_table[ARENA_PERS_DEFAULT].jugadores; al; al = al->next )
		arena_reset_char( al->ch, number_range(arena_table[ARENA_PERS_DEFAULT].desde, arena_table[ARENA_PERS_DEFAULT].hasta), 0 );

	arena_table[ARENA_PERS_DEFAULT].status = ARENA_OCUPADA;
	arena_table[ARENA_PERS_DEFAULT].pozo = 0;
}

void do_aceptar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int temp;
	ALIST *al;
	char buf[MIL];

	if ( IS_NPC(ch) || !ch->desc )
		return;

	if ( (victim = retador(ch)) == NULL )
	{
		send_to_char( "No has sido retado por nadie.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
		temp = ARENA_PERS_DEFAULT;
	else
		temp = arena_lookup( argument );

	if ( temp == -1 )
	{
		send_to_char( "Debes especificar una arena para poder pelear.\n\r", ch );
		return;
	}

	if ( !IS_SET(arena_table[temp].flags, ARENA_PERSONAL) )
	{
		send_to_char( "Esa arena no sirve.\n\r", ch );
		return;
	}

	if ( arena_table[temp].jugadores != NULL )
	{
		send_to_char( "Esa arena esta ocupada.\n\r", ch );
		return;
	}

	if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   !IS_SET(victim->in_room->room_flags, ROOM_SAFE) )
	{
		send_to_char( "Ambos deben estar en cuartos seguros antes de entrar a pelear.\n\r", ch );
		return;
	}

	if ( !can_recall(ch) || !can_recall(victim) )
	{
		send_to_char( "Deben poder hacer recall.\n\r", ch );
		return;
	}

	if (ch->hit < ch->max_hit)
	{
		send_to_char( "Debes estar perfectamente sano para poder entrar a la arena.\n\r", ch );
		return;
	}

	if ( victim->hit < victim->max_hit )
	{
		send_to_char( "Tu retador no esta lo suficientemente sano para poder entrar a la arena.\n\r", ch );
		return;
	}

	do_info( NULL, "[#BARENA#b] Los siguientes jugadores han entrado a luchar en la arena:\n\r" );

	sprintf( buf, "[#BARENA#b] %s (nivel %d/%d victorias/%d derrotas)",
			ch->name, getNivelPr(ch), ch->pcdata->victorias,
			ch->pcdata->derrotas );
	do_info( NULL, buf );

	sprintf( buf, "[#BARENA#b] %s (nivel %d/%d victorias/%d derrotas)\n\r",
			victim->name, getNivelPr(victim), victim->pcdata->victorias,
			victim->pcdata->derrotas );
	do_info( NULL, buf );

	inscribir_jugador( ch, temp );
	inscribir_jugador( victim, temp );

	for ( al = arena_table[temp].jugadores; al; al = al->next )
		arena_reset_char( al->ch, number_range(arena_table[temp].desde, arena_table[temp].hasta), 0 );

	arena_table[temp].status = ARENA_OCUPADA;
	arena_table[temp].pozo = 0;

	victim->pcdata->retando = NULL;
}

void do_retar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_NPC(ch) || !ch->desc )
		return;

	if ( ch->pcdata->retando )
	{
		if ( !str_cmp(argument, "anular") )
		{
			act( "$N retira su reto.", ch->pcdata->retando, NULL, chToEnt(ch), TO_CHAR );
			send_to_char( "Retiras tu reto.\n\r", ch );
			ch->pcdata->retando = NULL;
			return;
		}

		printf_to_char( ch, "Estas retando a %s.\n\r", SELFPERS(ch->pcdata->retando) );
		return;
	}

	if ( (victim = retador(ch)) )
	{
		printf_to_char( ch, "%s te reto a una lucha. Acepta o rechaza.\n\r", SELFPERS(victim) );
		return;
	}

	if ( esta_inscrito(ch) != -1 )
	{
		send_to_char( "Estas inscrito en la arena. No puedes retar.\n\r", ch );
		return;
	}

	if ( (victim = get_char_world( ch, argument )) == NULL
	||    IS_NPC(victim)
	||    ch == victim)
	{
		send_to_char( "Esa persona no esta jugando.\n\r", ch );
		return;
	}

	if ( getNivelPr(victim) < 10 )
	{
		act( "$N es un$O novat$O.", ch, NULL, chToEnt(victim), TO_CHAR );
		return;
	}

	if ( victim->pcdata->retando )
	{
		printf_to_char( ch, "%s esta retando a otra persona.\n\r", SELFPERS(victim) );
		return;
	}

	if ( esta_inscrito(victim) != -1 )
	{
		printf_to_char( ch, "%s esta inscrito en la arena. No puedes retarlo.\n\r", SELFPERS(victim) );
		return;
	}

	if ( !ch->desc )
		return;

	if ( !victim->desc )
	{
		printf_to_char( ch, "%s no tiene link.\n\r", SELFPERS(victim) );
		return;
	}

	if ( !str_cmp(victim->desc->host, ch->desc->host)
	&&   !str_cmp(victim->desc->ident, ch->desc->ident) )
	{
		send_to_char( "Si, claro. Como no.\n\r", ch );
		return;
	}

	ch->pcdata->retando = victim;

	printf_to_char( ch, "Has retado a %s a una lucha mortal.\n\r", SELFPERS(victim) );
	printf_to_char( victim, "%s te ha retado a una lucha mortal.\n\r", SELFPERS(ch) );
	send_to_char( "Escribe : #Barena ACEPTAR#b para aceptar el reto.\n\r", victim );
	send_to_char( "          #Barena RECHAZAR#b si te acobardaste.\n\r", victim );
}

void do_rechazar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_NPC(ch) || !ch->desc )
		return;

	if ( (victim = retador(ch)) == NULL )
	{
		send_to_char( "Nadie te ha retado.\n\r", ch );
		return;
	}

	act( "$N ha rechazado tu reto.", victim, NULL, chToEnt(ch), TO_CHAR );
	act( "Rechazas cobardemente el reto de $N.", ch, NULL, chToEnt(victim), TO_CHAR );

	victim->pcdata->retando = NULL;
}

void do_apostar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	ALIST * al;
	int temp, valor;
	char arg[MIL];

	if ( IS_NPC(ch) || !ch->desc )
		return;

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : arena apostar [jugador] [cantidad]\n\r", ch );
		return;
	}

	if ( ch->pcdata->apostando )
	{
		printf_to_char( ch, "Ya le estas apostando $%d a %s.\n\r",
			ch->pcdata->apuesta, SELFPERS(ch->pcdata->apostando) );
		return;
	}

	if ( (victim = get_char_world(ch, arg)) == NULL
	||   IS_NPC(victim) )
	{
		send_to_char( "No esta jugando.\n\r", ch );
		return;
	}

	if ( (temp = esta_inscrito(victim)) == -1 )
	{
		send_to_char( "No esta inscrito.\n\r", ch );
		return;
	}

	if ( !IS_SET(arena_table[temp].flags, ARENA_PERSONAL) )
	{
		send_to_char( "No puedes apostar en esa pelea.\n\r", ch );
		return;
	}

	valor = atoi(argument);

	if ( valor < 1 || valor > 1000 )
	{
		send_to_char( "Cantidad es entre 1 y 1000.\n\r", ch );
		return;
	}

	if ( ch->gold < valor )
	{
		send_to_char( "No tienes esa cantidad de monedas de oro!\n\r", ch );
		return;
	}

	al = en_alista( victim, arena_table[temp].jugadores );

	if ( !al )
	{
		send_to_char( "Hay un error.\n\r", ch );
		bugf( "do_apostar : jugador %s no esta en arena %d!",
			victim->name, temp );
		return;
	}

	ch->gold -= valor;
	ch->pcdata->apostando = victim;
	ch->pcdata->apuesta = valor;
	arena_table[temp].pozo += valor;
	al->apuestas += valor;

	printf_to_char( ch, "Apuestas %d a %s.\n\r", valor, SELFPERS(victim) );

	return;
}
