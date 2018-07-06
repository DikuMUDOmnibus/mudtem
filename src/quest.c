#include "include.h"
#include "recycle.h"
#include "smart.h"
#include "special.h"
#include "events.h"
#include "quest.h"

#define QUEST_FUNC(blah) bool blah( CHAR_DATA * qm, CHAR_DATA *pc, char *argument, int arg )

typedef bool quest_func ( CHAR_DATA *, CHAR_DATA *, char *, int );

struct quest_premio_type
{
	char *		nombre;
	char *		desc;
	int		costo;
	quest_func *	funcion;
	int		argumento;
};

QUEST_FUNC(qp_novamp);
QUEST_FUNC(qp_trains);
QUEST_FUNC(qp_prac);
QUEST_FUNC(qp_perdon);

const struct quest_premio_type quest_premio [] =
{
	{	"novamp",	"Quita el vampirismo.",		300,	qp_novamp,	0	},
	{	"perdon",	"Perdon divino (rangers).",	300,	qp_perdon,	0	},
	{	"10trains",	"10 Trains",			1500,	qp_trains,	10	},
	{	"5trains",	"5 Trains",			800,	qp_trains,	5	},
	{	"20prac",	"20 Practicas",			1000,	qp_prac,	20	},
	{	"10prac",	"10 Practicas",			800,	qp_prac,	10	},
	{	"5prac",	"5 Practicas",			500,	qp_prac,	5	},
	{	NULL,		NULL,				0,	NULL,		0	}
};

void mob_tell( CHAR_DATA *qm, CHAR_DATA *pc, char * mensaje, Entity * arg )
{
	char buf[MIL];

	sprintf( buf, "%s te dice '%s'", PERS(qm, pc), mensaje );
	
	act( buf, qm, arg, chToEnt(pc), TO_VICT );
}

QUEST_FUNC(qp_novamp)
{
	if ( pc->race != RACE_VAMPIRE )
	{
		mob_tell( qm, pc, "No eres vampiro!", NULL );
		return FALSE;
	}

	if ( pc->true_race == RACE_VAMPIRE )
	{
		mob_tell( qm, pc, "Eres vampiro por naturaleza. Nada puedo hacer.", NULL );
		return FALSE;
	}

	if ( mem_lookup(pc->memory, MEM_VAMPIRE) == NULL )
	{
		mob_tell( qm, pc, "AGGG. Habla con el IMP.", NULL );
		return FALSE;
	}

	polymorph( pc, pc->true_race );
	strip_mem_char( pc, MEM_VAMPIRE );

	mob_tell( qm, pc, "Has vuelto a tu forma original.", NULL );

	act( "$n vuelve a ser $t otra vez.", pc, strToEnt(race_table[pc->race].name,pc->in_room), NULL, TO_ROOM );

	return TRUE;
}

QUEST_FUNC(qp_trains)
{
	pc->train += arg;

	mob_tell( qm, pc, "Ganas $t sesiones de entrenamiento.", intToEnt(arg,qm->in_room) );

	return TRUE;
}

QUEST_FUNC(qp_perdon)
{
	if (!IS_SET(pc->act, PLR_MAL_ALIGN))
	{
		mob_tell(qm, pc, "No necesitas que perdonen.", NULL );
		return FALSE;
	}

	act("$n alza los brazos e implora perdon divino para $N.", qm,
		NULL, chToEnt(pc), TO_ROOM );
	REMOVE_BIT(pc->act, PLR_MAL_ALIGN);
	return TRUE;
}

QUEST_FUNC(qp_prac)
{
	pc->practice += arg;

	mob_tell( qm, pc, "Ganas $t sesiones de practica.", intToEnt(arg,qm->in_room) );

	return TRUE;
}

#if !defined(USAR_MACROS)
bool tiene_qdata( CHAR_DATA * ch )
{
	return IS_NPC(ch) ? FALSE : ch->pcdata->quest != NULL;
}

int get_qtype( CHAR_DATA * ch )
{
	return tiene_qdata(ch) ? ch->pcdata->quest->type : QUEST_NONE;
}

int get_qestado( CHAR_DATA * ch )
{
	return tiene_qdata(ch) ? ch->pcdata->quest->estado : QUEST_INCOMPLETO;
}

int get_qtimer( CHAR_DATA * ch )
{
	return tiene_qdata(ch) ? ch->pcdata->quest->timer : 0;
}

int get_qid( CHAR_DATA * ch )
{
	return tiene_qdata(ch) ? ch->pcdata->quest->id : 0;
}

bool quest_completo( CHAR_DATA *ch )
{
	return (ch->pcdata->quest->estado == QUEST_COMPLETO);
}

void give_qdata( CHAR_DATA *ch )
{
	ch->pcdata->quest = new_quest();
}

int get_qpoints( CHAR_DATA *ch )
{
	if ( IS_NPC(ch) || ch->pcdata->quest == NULL )
		return 0;

	return ch->pcdata->quest->qpoints;
}

bool esta_en_quest( CHAR_DATA *ch )
{
	if ( IS_NPC(ch) || ch->pcdata->quest == NULL )
		return FALSE;

	if ( ch->pcdata->quest->type != QUEST_NONE )
		return TRUE;

	return FALSE;
}
#endif

bool es_quest_mob( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !esta_en_quest(ch) || !IS_NPC(victim) )
		return FALSE;

	if ( get_qtype(ch) == QUEST_KILLMOB
	&&   get_qid(ch) == victim->pIndexData->vnum )
		return TRUE;

	return FALSE;
}

void completar_quest( CHAR_DATA *ch )
{
	if ( ch->pcdata->quest->estado == QUEST_COMPLETO )
		bugf( "completar_quest : ch %s con estado == QUEST_COMPLETO",
			ch->name );

	ch->pcdata->quest->estado = QUEST_COMPLETO;
}

void quest_points( CHAR_DATA *ch, CHAR_DATA *qm )
{
	if ( get_qpoints(ch) > 0 )
		act( "$n te dice 'Tienes $t puntos de quest.'",
			qm, intToEnt(get_qpoints(ch),qm->in_room), chToEnt(ch), TO_VICT );
	else
		act( "$n te dice 'No tienes ningun punto de quest acumulado.'",
			qm, NULL, chToEnt(ch), TO_VICT );
}

void quest_time( CHAR_DATA *ch, CHAR_DATA *qm )
{
	char * mensaje = NULL;
	char buf[MIL];

	if ( get_qtimer(ch) > 0 )
	{
		if ( esta_en_quest(ch) )
			mensaje = "Te quedan %s minutos para completar tu quest.";
		else
			mensaje = "En %s minutos puedes pedirme otro quest.";
	}
	else
		mensaje = "Puedes pedir un quest si asi lo deseas.";

	sprintf( buf, mensaje, itos(get_qtimer(ch)) );

	if ( qm )
		act( buf, qm, NULL, chToEnt(ch), TO_VICT );
	else
		printf_to_char( ch, "%s\n\r", buf );
}

void give_qpoints( CHAR_DATA *ch, int points )
{
	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->quest == NULL )
		give_qdata( ch );

	ch->pcdata->quest->qpoints = UMAX(0, ch->pcdata->quest->qpoints + points);
}

void quest_clean( CHAR_DATA *ch )
{
	set_qtimer(ch, 0);
	ch->pcdata->quest->type		= QUEST_NONE;
	set_qid(ch, 0);
	ch->pcdata->quest->estado	= QUEST_INCOMPLETO;
}

void set_qid( CHAR_DATA *ch, long id )
{
	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->quest == NULL )
		give_qdata(ch);

	ch->pcdata->quest->id = id;
}

void set_qestado( CHAR_DATA *ch, int estado )
{
	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->quest == NULL )
		give_qdata(ch);

	ch->pcdata->quest->estado = estado;
}

void set_qtype( CHAR_DATA *ch, int type )
{
	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->quest == NULL )
		give_qdata(ch);

	ch->pcdata->quest->type = type;
}

void set_qtimer( CHAR_DATA *ch, int timer )
{
	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->quest == NULL )
		give_qdata( ch );

	ch->pcdata->quest->timer = timer;

	if ( !event_pending(ch->events, quest_update) )
		char_event_add( ch, PULSE_PER_SECOND*MINUTOS(1), 0, quest_update );
}

void quest_update( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;

	if ( ch->pcdata->quest == NULL
	||   ch->pcdata->quest->timer <= 0 )
		return;

	ch->pcdata->quest->timer--;

	if ( ch->pcdata->quest->timer == 0 )
	{
		if ( esta_en_quest(ch) )
		{
			send_to_char( "Se ha acabado tu tiempo de QUEST!\n\r", ch );
			send_to_char( "Puedes pedir otro en 5 minutos.\n\r", ch );
			quest_clean(ch);
			set_qtimer( ch, 5 );
			return;
		}
		else
		{
			send_to_char( "Puedes hacer quest otra vez.\n\r", ch );
		}
	}
	else
	if ( esta_en_quest(ch) && ch->pcdata->quest->timer < 5 )
		send_to_char( "Apurate, tu tiempo para acabar el quest se agota!\n\r", ch );

	char_event_add( ch, PULSE_PER_SECOND*MINUTOS(1), 0, quest_update );
}

void quest_entregar( CHAR_DATA *ch, CHAR_DATA *qm )
{
	int qp;
	MEM_DATA * mem;

	if ( !esta_en_quest(ch) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	if ( get_qtimer(ch) < 1 )
	{
		act( "$n te dice 'Llegaste tarde'.", qm, NULL, chToEnt(ch), TO_VICT );
		quest_clean(ch);
		return;
	}

	if ( !quest_completo(ch) )
	{
		act( "$n te dice 'Pero tu todavia no completas tu quest!'", qm, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	if ( get_qtype(ch) == QUEST_KILLMOB )
	{
		if ( IS_EVIL(qm) )
			act( "$n te dice 'Gracias por haber asesinado a ese miserable ser.'", qm, NULL, chToEnt(ch), TO_VICT );
		else
			act( "$n te dice 'Gracias por haber devuelto la paz a $t!'", qm, strToEnt(qm->in_room->area->name,qm->in_room), chToEnt(ch), TO_VICT );
	}

	qp = number_range( 10, 25 );

	if (CHANCE(10))
		qp *= 1.5;

	act( "$n te dice 'Como recompensa, te doy $t puntos de quest.'", qm, intToEnt(qp,qm->in_room), chToEnt(ch), TO_VICT );

	for ( mem = ch->memory; mem; mem = mem->next )
		if ( mem->reaction == MEM_QUEST
		&&   mem->id == get_qid(ch) )
			mem->reaction = MEM_QUEST_COMPLETO;

	give_qpoints( ch, qp );
	quest_clean(ch);
	set_qtimer( ch, 5 );
}

void quest_lista( CHAR_DATA *ch, CHAR_DATA *qm )
{
	int i;

	mob_tell( qm, ch, "Puedes comprar:", NULL );

	send_to_char( "#UCosto Nombre     Descripcion           #u\n\r", ch );

	for ( i = 0; quest_premio[i].nombre != NULL; i++ )
		printf_to_char( ch, "%5d %-10s %s\n\r",
			quest_premio[i].costo, quest_premio[i].nombre,
			quest_premio[i].desc );
}

void quest_info( CHAR_DATA *ch, CHAR_DATA *qm )
{
	CHAR_DATA *victim;

	if ( !esta_en_quest(ch) )
	{
		act( "$n te dice 'No estas haciendo un quest.'", qm, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	if ( get_qestado(ch) == QUEST_COMPLETO )
	{
		act( "$n te dice 'Ya terminaste tu quest!'", qm, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	switch(get_qtype(ch))
	{
		case QUEST_KILLMOB:
		for ( victim = char_list; victim; victim = victim->next )
			if ( IS_NPC(victim) && victim->pIndexData->vnum == get_qid(ch) )
				break;
		if ( !victim )
			act( "$n te dice 'Hay un pequenio problema.'", qm, NULL, chToEnt(ch), TO_VICT );
		else
			act( "$n te dice 'Estas buscando a $t!'", qm, strToEnt(SELFPERS(victim),victim->in_room), chToEnt(ch), TO_VICT );
		break;

		default:
		bugf( "quest_info : qtype %d", get_qtype(ch) );
		break;
	}
}

void quest_pedir( CHAR_DATA *ch, CHAR_DATA *qm )
{
	CHAR_DATA *victim;
	char buf[MIL];
	MEM_DATA *mem;
	int cnt = 0;

	if ( esta_en_quest(ch) )
	{
		act( "$n te dice 'Pero tu ya estas en un quest!'", qm, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	if ( get_qtimer(ch) > 0 )
	{
		act( "$n te dice 'Todavia te faltan $t minutos para poder pedir otro quest.'",
			qm, intToEnt(get_qtimer(ch),qm->in_room), chToEnt(ch), TO_VICT );
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		for ( mem = ch->memory; mem; mem = mem->next )
			if ( mem->reaction == MEM_QUEST_COMPLETO )
				cnt++;

		if ( cnt > 20 )
		{
			mob_tell( qm, ch, "Ya completaste todos los quests de tu nivel.", NULL );
			return;
		}
	}

	for ( victim = char_list; victim; victim = victim->next )
		if ( IS_NPC(victim)
		&&   victim->in_room
		&&   getNivelPr(victim) == getNivelPr(ch)
		&&   victim->hit == victim->max_hit
		&&  !is_clan(victim)
		&&  !IS_AFFECTED(victim, AFF_CHARM)
		&&  !IS_PET(victim)
		&&  !IS_SAME_ALIGN(qm, victim)
		&&   victim->pIndexData->vnum != MOB_VNUM_RATA
		&&  (mem_lookup_id(ch->memory, victim->pIndexData->vnum) == NULL)
		&&  !is_mob_safe(victim) )
			break;

	if ( !victim )
	{
		act( "$n te dice 'Lo lamento, $N, pero no tengo quests para ti en este momento.'", qm, NULL, chToEnt(ch), TO_VICT );
		act( "$n te dice 'Intentalo mas tarde.'", qm, NULL, chToEnt(ch), TO_VICT );
		return;
	}

	act( "$n te dice 'Gracias, valiente $N!'", qm, NULL, chToEnt(ch), TO_VICT );

	if ( IS_GOOD(victim) || IS_NEUTRAL(victim) )
	{
		act( "$n te dice 'Debes asesinar al maldito $t!'",
			qm, strToEnt(SELFPERS(victim),victim->in_room), chToEnt(ch), TO_VICT );
		act( "$n te dice 'Nuestros espias lo han localizado en $t.'",
			qm, strToEnt(victim->in_room->name,victim->in_room), chToEnt(ch), TO_VICT );
	}
	else
	{
		sprintf( buf, "%s te dice '%s el asesino ha huido de %s!'\n\r",
			SELFPERS(qm), SELFPERS(victim), qm->in_room->area->name );
		send_to_char( buf, ch );
		sprintf( buf, "%s te dice 'En su huida, asesino a %d de nuestros guardias!'\n\r",
			SELFPERS(qm), number_range(1,getNivelPr(victim)) );
		send_to_char( buf, ch );
		act( "$n te dice 'Nuestros vigias lo han localizado en $t.'",
			qm, strToEnt(victim->in_room->name,victim->in_room), chToEnt(ch), TO_VICT );
	}

	act( "$n te dice 'Ese lugar esta en $t.'",
		qm, strToEnt(victim->in_room->area->name,victim->in_room), chToEnt(ch), TO_VICT );

	if ( ch->pcdata->quest == NULL )
		give_qdata( ch );

	set_qtimer(ch, number_range(15,25));
	ch->pcdata->quest->type		= QUEST_KILLMOB;
	set_qid(ch, victim->pIndexData->vnum);
	give_mem( ch, MEM_QUEST, ch->pcdata->quest->id );

	act( "$n te dice 'Tienes $t minutos para lograr tu objetivo'.",
		qm, intToEnt(ch->pcdata->quest->timer,qm->in_room), chToEnt(ch), TO_VICT );
}

int qp_lookup( char * argument )
{
	int i;

	for ( i = 0; quest_premio[i].nombre; i++ )
		if ( !str_cmp( argument, quest_premio[i].nombre ) )
			return i;

	return -1;
}

void quest_comprar( CHAR_DATA *ch, CHAR_DATA *qm, char *argument )
{
	char arg[MIL];
	int indice;

	argument = one_argument( argument, arg );

	indice = qp_lookup(arg);

	if ( indice == -1 )
	{
		mob_tell( qm, ch, "No conozco nada de eso.", NULL );
		return;
	}

	if ( get_qpoints(ch) < quest_premio[indice].costo )
	{
		mob_tell( qm, ch, "No tienes suficientes puntos.", NULL );
		return;
	}

	if ( (*quest_premio[indice].funcion) ( qm, ch, argument, quest_premio[indice].argumento ) )
		give_qpoints( ch, - quest_premio[indice].costo );
}

DO_FUN_DEC( do_quest )
{
	CHAR_DATA *qm;
	char arg[MIL];

	if ( IS_NPC(ch) )
		return;

	for ( qm = ch->in_room->people; qm; qm = qm->next_in_room )
		if ( IS_NPC(qm) && SPEC_FUN(qm) == spec_questmaster )
			break;

	if ( !IS_NULLSTR(argument) && !str_prefix( argument, "tiempo" ) )
	{
		quest_time( ch, qm ); /* no importa si es NULL */
		return;
	}

	if ( !qm )
	{
		send_to_char( "No puedes hacer eso aqui.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		act( "$n te dice 'Saludos, $N.'", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas pedirme un quest, escribe '#Bquest pedir#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas entregarme un quest, escribe '#Bquest entregar#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas ver cuanto tiempo tiempo te queda disponible para completar tu", qm, NULL, chToEnt(ch), TO_VICT );
		act( "quest, escribe '#Bquest tiempo#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas saber cuantos puntos de quest tienes acumulados, '#Bquest puntos#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas ver una lista de cosas que puedes comprar con tus puntos de quest,", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si deseas ver informacion de tu quest actual, '#Bquest info#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "escribe '#Bquest lista#b'.", qm, NULL, chToEnt(ch), TO_VICT );
		act( "Si quieres comprarme algo, '#Bquest comprar#b'.",qm,NULL,chToEnt(ch),TO_VICT);
		return;
	}

	if ( !str_prefix( argument, "pedir" ) )
	{
		quest_pedir( ch, qm );
		return;
	}

	if ( !str_prefix( argument, "entregar" ) )
	{
		quest_entregar( ch, qm );
		return;
	}

	if ( !str_prefix( argument, "lista" ) )
	{
		quest_lista( ch, qm );
		return;
	}

	if ( !str_prefix( argument, "puntos" ) )
	{
		quest_points( ch, qm );
		return;
	}

	if ( !str_prefix( argument, "info" ) )
	{
		quest_info( ch, qm );
		return;
	}

	argument = one_argument( argument, arg );

	if ( !str_prefix( arg, "comprar" ) )
	{
		quest_comprar( ch, qm, argument );
		return;
	}

	do_quest( ch, "" );
	return;
}
