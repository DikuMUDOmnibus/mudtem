#include "include.h"
#include "clan.h"
#include "olc.h"
#include "lookup.h"
#include "recycle.h"
#include "tables.h"

#include <time.h>

DECLARE_DO_FUN (do_petition);
DECLARE_DO_FUN (do_clans);
DECLARE_DO_FUN (do_look);

sh_int			clan_count = 0;
struct clan_type *	clan_table = NULL;
PETITION_DATA *		petition_list;
void			grabar_clanes (void);

const char  *lookup_clan_status (int cs)
{
	switch (cs)
	{
	case CLAN_DICTADOR:
		return "DICTADOR";
	case CLAN_SENADOR:
		return "Senador Designado";
	case CLAN_MINISTRO:
		return "Ministro";
	case CLAN_CORONEL:
		return "Coronel";
	case CLAN_COMANDANTE:
		return "Comandante";
	case CLAN_TENIENTE:
		return "Teniente";
	case CLAN_CONCEJAL:
		return "Concejal";
	case CLAN_CABO:
		return "Cabo";
	case CLAN_CONSCRIPTO:
		return "Conscripto";
	case CLAN_PERKINS:
		return "perkins";
	case CLAN_GOMA:
		return "goma";
	}

	return "un miembro";
}

int          min_level_for_status (int cs)
{
      switch (cs)
      {
	case CLAN_DICTADOR:
		return 25;
	case CLAN_SENADOR:
		return 25;
	case CLAN_MINISTRO:
		return 20;
	case CLAN_CORONEL:
		return 20;
	case CLAN_COMANDANTE:
		return 18;
	case CLAN_TENIENTE:
		return 17;
	case CLAN_CONCEJAL:
		return 16;
	case CLAN_CABO:
		return 14;
	case CLAN_CONSCRIPTO:
		return 13;
	case CLAN_PERKINS:
		return 12;
	case CLAN_GOMA:
		return 10;
      }
      return 10;
}

int          parse_clan_status (const char *s)
{
      if (s[0])
      {
	    if (!str_prefix (s, "goma"))
		  return CLAN_GOMA;
	    if (!str_prefix (s, "comandante"))
		  return CLAN_COMANDANTE;
	    if (!str_prefix (s, "coronel"))
	    	return CLAN_CORONEL;
	    if (!str_prefix (s, "concejal"))
	    	return CLAN_CONCEJAL;
	    if (!str_prefix (s, "cabo"))
	    	return CLAN_CABO;
	    if (!str_prefix (s, "conscripto"))
	    	return CLAN_CONSCRIPTO;
	    if (!str_prefix (s, "perkins"))
	    	return CLAN_PERKINS;
	    if (!str_prefix (s, "ministro"))
		  return CLAN_MINISTRO;
	    if (!str_prefix (s, "senador"))
		  return CLAN_SENADOR;
	    if (!str_prefix (s, "dictador"))
		  return CLAN_DICTADOR;
	    if (!str_prefix(s, "teniente"))
	    	return CLAN_TENIENTE;
      }
      return -1;
}

void do_cwho( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *vict;
	char buf[MSL];
	
	if ( !is_clan(ch) )
	{
		send_to_char( "No estas en un clan.\n\r", ch );
		return;
	}
	
	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING
		|| ( vict = d->character ) == NULL
		|| !can_see( ch, vict )
		|| !is_same_clan( ch, vict ) )
			continue;
		
		sprintf( buf, "[%2d %5s %3s %17s] %s%s #n\n\r",
			getNivelPr(vict),
			race_table[vict->race].who_name,
			class_table[getClasePr(vict)].who_name,
			IS_NPC(vict) ? "" : lookup_clan_status(vict->pcdata->clan_status),
			vict->name,
			IS_NPC(vict) ? "" : vict->pcdata->title );
		send_to_char(buf,ch);
	}
}

void check_new_peticiones( CHAR_DATA *ch )
{
	char buf[MIL];
	PETITION_DATA *pet;
	bool first = TRUE;

	for ( pet = petition_list; pet; pet = pet->next )
		if ( pet->clan == ch->clan && pet->status == PET_INDEF )
		{
			if ( first )
			{
				send_to_char( "Los siguientes jugadores han hecho una peticion de entrada a tu clan:\n\r", ch );
				send_to_char( "#UNombre     Ni Status     Anterior   ->Nuevo      #u\n\r", ch );
				first = FALSE;
			}

			sprintf( buf, "%-10.10s %2d %-10.10s %-11.11s->%-11.11s\n\r",
				pet->name,
				pet->nivel,
				flag_string( petstat_table, pet->status ),
				clan_table[pet->oldclan].name,
				clan_table[pet->clan].name );
			send_to_char( buf, ch );
		}

	return;
}

bool check_peticiones( void )
{
	PETITION_DATA *pet, *pet_next;
	CHAR_DATA *ch;
	bool blah = FALSE;

	for ( pet = petition_list; pet; pet = pet_next )
	{
		pet_next = pet->next;

		if ( pet->status == PET_INDEF )
			continue;

		ch = get_char_from_id( pet->id );

		if ( !ch )
			continue;

		if ( pet->status == PET_ACEPTADA )
		{
			printf_to_char( ch, "Tu solicitud de entrada al clan #B%s#b fue #BACEPTADA#b!\n\r",
				clan_table[pet->clan].name );
			ch->clan		= pet->clan;
			ch->pcdata->clan_status	= CLAN_GOMA;
		}
		else
		if ( pet->status == PET_RECHAZADA )
			printf_to_char( ch, "Tu solicitud de entrada al clan #B%s#b fue #BRECHAZADA#b!\n\r",
				clan_table[pet->clan].name );
			
		petition_delete(pet);
		blah = TRUE;
	}

	if (blah)
		grabar_peticiones();

	return blah;
}

/*
 *  petition [clan]
 *  petition accept [player]
 *  petition reject [player]
 *  petition remove [player]
 *  petition raise <player> <status>
 */
void         do_petition_list (CHAR_DATA * ch)
{
	PETITION_DATA * pet;
	char buf[MIL], buf2[7];
	bool fAll = FALSE;

	if ( IS_IMMORTAL(ch) )
		fAll = TRUE;

	send_to_char( "#UNombre     Ni Cuando Status     Anterior   ->Nuevo      #u\n\r", ch );

	for ( pet = petition_list; pet; pet = pet->next )
		if ( fAll || ch->clan == pet->clan )
		{
			strftime( buf2, 7, "%H/%m", localtime(&(pet->when)) );

			sprintf( buf, "%-10.10s %2d %s %-10.10s %-11.11s->%-11.11s\n\r",
				pet->name,
				pet->nivel,
				buf2,
				flag_string( petstat_table, pet->status ),
				clan_table[pet->oldclan].name,
				clan_table[pet->clan].name );
			send_to_char( buf, ch );
		}

	return;
}

void         do_petition (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MSL];
	char comando[MIL], jugador[MIL];
	sh_int	status, rango, val = 0;

	if ( IS_NPC(ch) )
		return;

	status = ch->clan ? ch->pcdata->clan_status : 0;

	if ( (val = clan_lookup(argument)) > 0 ) /* peticion */
	{
		PETITION_DATA *pet;

		if ( (pet = petition_lookup(ch)) != NULL )
		{
			petition_delete(pet);
			send_to_char( "Retiras tu peticion.\n\r", ch );
			return;
		}

		if ( getNivelPr(ch) < 10 )
		{
			send_to_char( "Eres de nivel demasiado bajo como para entrar a un clan.\n\r", ch );
			return;
		}

		if ( is_clan(ch) )
		{
			if ( ch->clan == val )
			{
				send_to_char( "Ya estas en ese clan.\n\r", ch );
				return;
			}

			if ( !ES_INDEP(ch->clan) && (status == CLAN_DICTADOR) )
			{
				send_to_char( "Un lider de un clan no puede hacer peticiones.\n\r", ch );
				return;
			}
		}

		if ( IS_GOOD(ch) && IS_CLAN_NOGOOD(val) )
		{
			send_to_char( "Ese clan no acepta jugadores buenos.\n\r", ch );
			return;
		}

		if ( IS_EVIL(ch) && IS_CLAN_NOEVIL(val) )
		{
			send_to_char( "Ese clan no acepta jugadores malvados.\n\r", ch );
			return;
		}

		if ( IS_NEUTRAL(ch) && IS_CLAN_NONEUTRAL(val) )
		{
			send_to_char( "Ese clan no acepta jugadores neutrales.\n\r", ch );
			return;
		}

		if ( ES_INDEP(val) )
		{
			send_to_char( "Ese clan es solitario.\n\r", ch );
			return;
		}

		sprintf( buf, "Haces una peticion de entrada al clan #B%s#b.\n\r", clan_table[val].name );
		send_to_char( buf, ch );

		pet		= new_petition();
		pet->name	= str_dup( ch->name );
		pet->id		= ch->id;
		pet->oldclan	= ch->clan;
		pet->clan	= val;
		pet->status	= PET_INDEF;
		pet->nivel	= getNivelPr(ch);
		pet->when	= current_time;
		pet->next	= petition_list;
		petition_list	= pet;

		grabar_peticiones();

		return;
	}

	if ( IS_IMMORTAL(ch) )
	{
		char * tmparg = one_argument(argument, comando);

		if ( !str_cmp(comando, "lista") )
		{
			do_petition_list(ch);
			return;
		}

		if ( !str_cmp(comando, "borrar") )
		{
			PETITION_DATA * pet = petition_lookup_name(tmparg);

			if ( pet == NULL )
			{
				send_to_char( "Ese jugador no ha hecho una peticion.\n\r", ch );
				return;
			}

			petition_delete(pet);
			send_to_char( "Petition borrada.\n\r", ch );
			return;
		}
	}

	if (is_clan(ch) && status >= CLAN_MINISTRO)
	{
		if ( !str_cmp( argument, "lista" ) )
		{
			do_petition_list(ch);
			return;
		}

		argument = one_argument( argument, comando );

		if ( !str_cmp( comando, "aceptar" ) )
		{
			PETITION_DATA * pet = petition_lookup_name( argument );

			if ( !pet || pet->clan != ch->clan )
			{
				send_to_char( "Ese jugador no ha hecho una peticion a tu clan.\n\r", ch );
				return;
			}

			if ( pet->oldclan == ch->clan )
			{
				send_to_char( "Ya esta en tu clan.\n\r", ch );
				petition_delete(pet);
				return;
			}
			
			if ( pet->nivel < 10 )
			{
				send_to_char( "Ese jugador es de muy bajo nivel.\n\r", ch );
				return;
			}

			pet->status = PET_ACEPTADA;

			sprintf( buf, "Ahora %s es un miembro de tu clan.\n\r", argument );
			send_to_char( buf, ch );

			clanlog( pet->clan, "%s fue aceptado en el clan por %s",
				argument, ch->name );

			if ( !check_peticiones() )
				grabar_peticiones();

			return;
		}

		if ( !str_cmp( comando, "rechazar" ) )
		{
			PETITION_DATA * pet = petition_lookup_name( argument );

			if ( !pet || pet->clan != ch->clan )
			{
				send_to_char( "Ese jugador no ha hecho una peticion a tu clan.\n\r", ch );
				return;
			}

			pet->status = PET_RECHAZADA;
			sprintf( buf, "La solicitud de %s de entrar a tu clan ha sido rechazada.\n\r", argument );
			send_to_char( buf, ch );

			if ( !check_peticiones() )
				grabar_peticiones();

			return;
		}

		if ( !str_cmp( comando, "retirar" ) )
		{
			victim = get_char_world( ch, argument );
			
			if ( !victim || IS_NPC(victim) )
			{
				send_to_char( "No esta jugando.\n\r", ch );
				return;
			}

			if ( !is_same_clan( ch, victim ) )
			{
				send_to_char( "No esta en tu clan.\n\r", ch );
				return;
			}

			if ( victim->pcdata->clan_status >= status )
			{
				send_to_char( "No puedes sacar del clan a alguien con status igual o superior al tuyo.\n\r", ch );
				return;
			}

			if ( victim->pcdata->min_clan_status > 1 )
			{
				send_to_char( "No puedes sacarlo del clan.\n\r", ch );
				return;
			}

			if (victim->in_room->clan == victim->clan)
			{
				char_from_room(victim);
				char_to_room(victim, get_room_index(ROOM_VNUM_TEMPLE) );
				do_look(victim, "auto");
			}

			clanlog(ch->clan, "%s (%s) fue retirado del clan por %s",
				victim->name,
				lookup_clan_status(victim->pcdata->clan_status),
				ch->name );

			victim->pcdata->clan_status	= 0;
			victim->clan			= 0;

			sprintf( buf, "%s te retiro del clan #B%s#b.\n\r", chcolor(ch), clan_table[ch->clan].name );
			send_to_char( buf, victim );
			sprintf( buf, "%s fue retirado de tu clan.\n\r", chcolor(victim) );
			send_to_char( buf, ch );

			return;
		}

		if ( status >= CLAN_SENADOR && !str_cmp( comando, "subir" ) )
		{
			argument = one_argument( argument, jugador );

			victim = get_char_world( ch, jugador );

			if ( !victim || IS_NPC(victim) )
			{
				send_to_char( "No esta jugando.\n\r", ch );
				return;
			}

			if ( !is_same_clan( ch, victim ) )
			{
				send_to_char( "No esta en tu clan.\n\r", ch );
				return;
			}

			if ( victim->pcdata->clan_status >= status )
			{
				send_to_char( "Ya es de un rango igual o superior al tuyo.\n\r", ch );
				return;
			}

			rango = parse_clan_status( argument );
			
			if ( rango == -1 )
			{
				send_to_char( "Que rango es ese?\n\r", ch );
				send_to_char( "Los rangos posibles son:\n\r", ch );
				if ( status > CLAN_SENADOR )
					send_to_char( "#USenador#u al mando\n\r", ch );
				send_to_char( "#UMinistro#u del clan\n\r", ch );
				send_to_char( "#UCoronel#u del clan\n\r", ch );
				send_to_char( "#UComandante#u del clan\n\r", ch );
				send_to_char( "#UTeniente#u del clan\n\r", ch );
				send_to_char( "#UConcejal#u del clan\n\r", ch );
				send_to_char( "#UCabo#u del clan\n\r", ch );
				send_to_char( "#UConscripto#u del clan\n\r", ch );
				send_to_char( "#UPerkins#u del clan\n\r", ch );
				return;
			}

			if ( rango <= victim->pcdata->clan_status || rango >= status )
			{
				send_to_char( "Eso no es muy logico.\n\r", ch );
				return;
			}

			if ( getNivelPr(victim) < min_level_for_status( rango ) )
			{
				send_to_char( "Le falta experiencia como para asumir un rango tan alto.\n\r", ch );
				return;
			}

			if ( victim->pcdata->max_clan_status < rango )
			{
				act( "No puedes subirl$O a ese rango.", ch, NULL, chToEnt(victim), TO_CHAR );
				return;
			}

			clanlog( ch->clan, "%s subio a %s al rango %s desde el %s",
				ch->name, victim->name, lookup_clan_status(rango),
				lookup_clan_status(victim->pcdata->clan_status) );

			victim->pcdata->clan_status = rango;
			sprintf( buf, "%s es ahora #B%s#b del clan.\n\r", chcolor(victim), lookup_clan_status( rango ) );
			send_to_char( buf, ch );
			sprintf( buf, "%s te subio de rango, ahora eres #B%s#b del clan.\n\r", chcolor(ch), lookup_clan_status( rango ) );
			send_to_char( buf, victim );
			return;
		}

		if ( status >= CLAN_SENADOR && !str_cmp( comando, "bajar" ) )
		{
			argument = one_argument( argument, jugador );

			victim = get_char_world( ch, jugador );

			if ( !victim || IS_NPC(victim) )
			{
				send_to_char( "No esta jugando.\n\r", ch );
				return;
			}
			
			if ( !is_same_clan( ch, victim ) )
			{
				send_to_char( "No esta en tu clan.\n\r", ch );
				return;
			}

			if ( victim->pcdata->clan_status >= status )
			{
				send_to_char( "Ya es de un rango igual o superior al tuyo.\n\r", ch );
				return;
			}

			rango = parse_clan_status( argument );

			if ( rango == -1 )
			{
				send_to_char( "Que rango es ese?\n\r", ch );
				send_to_char( "Los rangos posibles son:\n\r", ch );
				if ( status > CLAN_SENADOR )
					send_to_char( "#USenador#u al mando\n\r", ch );
				send_to_char( "#UMinistro#u del clan\n\r", ch );
				send_to_char( "#UCoronel#u del clan\n\r", ch );
				send_to_char( "#UComandante#u del clan\n\r", ch );
				send_to_char( "#UTeniente#u del clan\n\r", ch );
				send_to_char( "#UConcejal#u del clan\n\r", ch );
				send_to_char( "#UCabo#u del clan\n\r", ch );
				send_to_char( "#UConscripto#u del clan\n\r", ch );
				send_to_char( "#UPerkins#u del clan\n\r", ch );
				return;
			}

			if ( rango >= victim->pcdata->clan_status || rango >= status )
			{
				send_to_char( "Eso no es muy logico.\n\r", ch );
				return;
			}

			if ( victim->pcdata->min_clan_status > rango )
			{
				act( "No puedes bajarl$O a ese rango.", ch, NULL, chToEnt(victim), TO_CHAR );
				return;
			}

			clanlog(ch->clan, "%s bajo a %s al rango %s desde el %s",
				ch->name, victim->name,
				lookup_clan_status(rango),
				lookup_clan_status(victim->pcdata->clan_status) );

			victim->pcdata->clan_status = rango;
			sprintf( buf, "%s es ahora #B%s#b del clan.\n\r", chcolor(victim), lookup_clan_status( rango ) );
			send_to_char( buf, ch );
			sprintf( buf, "%s te bajo de rango, ahora eres #B%s#b del clan.\n\r", chcolor(ch), lookup_clan_status( rango ) );
			send_to_char( buf, victim );
			return;
		}
	}

	send_to_char( "Sintaxis :\n\r", ch );

	if ( status >= CLAN_SENADOR )
	{
		send_to_char( "#Bpeticion subir#b [#Bjugador#b] [#BRango#b] : Sube de status a un miembro del clan.\n\r", ch );
		send_to_char( "#Bpeticion bajar#b [#Bjugador#b] [#BRango#b] : Baja de status a un miembro del clan.\n\r", ch );
	}

	if ( status >= CLAN_MINISTRO )
	{
		send_to_char( "#Bpeticion lista#b                   : Muestra la lista de personas que han pedido\n\r"
		              "                                   entrar al clan.\n\r", ch );
		send_to_char( "#Bpeticion aceptar#b [#Bjugador#b]       : Acepta la peticion de un jugador.\n\r", ch );
		send_to_char( "#Bpeticion rechazar#b [#Bjugador#b]      : Rechaza la peticion de un jugador.\n\r", ch );
		send_to_char( "#Bpeticion retirar#b [#Bjugador#b]       : Retira a un miembro del clan.\n\r", ch );
	}
	else
	if ( IS_IMMORTAL(ch) )
		send_to_char( "#Bpeticion lista#b                   : Muestra la lista de personas que han pedido\n\r"
			      "                                   entrar al clan.\n\r", ch );

	if ( IS_IMMORTAL(ch) )
		send_to_char( "#Bpeticion borrar#b [#Bnombre#b]       : Borra la peticion del jugador [nombre]\n\r", ch );

	if ( status != CLAN_DICTADOR )
		send_to_char( "#Bpeticion#b [#Bclan#b]                  : Hace peticion de entrada a un clan.\n\r", ch );

	return;
}

void         do_clans (CHAR_DATA * ch, char *argument)
{
      sh_int i;
      char buf[MIL];

      if (!ch)
	    return;
      send_to_char ("nombre     who             god        hall recall death pks pds mks mds ill\n\r", ch);
      send_to_char ("---------------------------------------------------------------------------\n\r", ch);
      for (i = 0; i < clan_count; i++)
      {
	    sprintf (buf, "%-10s %-15s %-10s %4d %6d %5d %3d %3d %3d %3d %3d\n\r",
	    clan_table[i].name,
	    clan_table[i].who_name,
	    clan_table[i].god,
	    clan_table[i].hall,
	    clan_table[i].recall,
	    clan_table[i].death,
	    clan_table[i].pkills,
	    clan_table[i].pdeaths,
	    clan_table[i].mkills,
	    clan_table[i].mdeaths,
	    clan_table[i].illpkills );
	    send_to_char (buf, ch);
      }
}

struct clan_type *get_clan_table (int id)
{
      static struct clan_type null_clan =
      {
	    "", "", "", ROOM_VNUM_ALTAR, ROOM_VNUM_TEMPLE, ROOM_VNUM_MORGUE, ROOM_VNUM_ALTAR, 0, 0, 0, 0, 0, 0
      };

      if ( id < 0 || id >= clan_count )
	return &null_clan;
      else
        return &clan_table[id];
}

int          clan_lookup (const char *name)
{
      sh_int i;

      for (i = 0; i < clan_count; i++)
	    if (UPPER (name[0]) == UPPER (clan_table[i].name[0])
		&& !str_prefix (name, clan_table[i].name))
			return i;
      return 0;
}

bool         is_independent (CHAR_DATA * ch)
{
	return (ES_INDEP(ch->clan));
}

const struct olc_cmd_type cedit_table[] =
{
	{	"commands",	show_commands	},
	{	"show",		cedit_show	},
	{	"nombre",	cedit_name	},
	{	"who",		cedit_who	},
	{	"dios",		cedit_god	},
	{	"recall",	cedit_recall	},
	{	"death",	cedit_death	},
	{	"hall",		cedit_hall	},
	{	"pit",		cedit_pit	},
	{	"new",		cedit_new	},
	{	"flags",	cedit_flags	},
	{	"?",		show_help	},

	{	NULL,		0		}
};

void cedit( CHAR_DATA *ch, char *argument)
{
    CLAN_TYPE *pClan;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_CLAN(ch, pClan);
    if (ch->pcdata->security < 7)
    {
        send_to_char("CEdit : Insuficiente seguridad para modificar clan.\n\r",ch);
        edit_done(ch);
        return;
    }

    if (command[0] == '\0')
    {
        cedit_show(ch, argument);
        return;
    }
    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; cedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix(command, cedit_table[cmd].name) )
        {
           if ((*cedit_table[cmd].olc_fun) (ch, argument))
           	grabar_clanes();
           return;
        }
    }

    interpret(ch, arg);
    return;
}

void do_cedit(CHAR_DATA *ch, char *argument)
{
    CLAN_TYPE *pClan = NULL;
    int clan;

    if ( IS_NPC(ch) || ch->pcdata->security < 7 )
    {
    	send_to_char( "CEdit : Insuficiente seguridad para editar clanes.\n\r", ch );
    	return;
    }

    if(is_number(argument) )
    {
	pClan = get_clan_table( atoi(argument) );
	
	if ( IS_NULLSTR(pClan->name) )
	{
		send_to_char("CEdit : Ese vnum no existe.\n\r",ch);
		return;
	}
    }
    else
    {
    	if ( (clan = clan_lookup(argument)) == 0 )
	{
		send_to_char( "CEdit : Ese clan no existe.\n\r", ch );
		return;
	}
	else
		pClan = &clan_table[clan];
    }

    ch->desc->pEdit	= (void *)pClan;
    ch->desc->editor	= ED_CLAN;
    return;
}

CEDIT( cedit_show )
{
    CLAN_TYPE *pClan;
    char buf[MAX_STRING_LENGTH];

    EDIT_CLAN(ch,pClan);

    sprintf(buf,
           "Flags          : [%s]\n\r"
           "Nombre         : [%s]\n\r"
           "Who            : [%s]\n\r"
           "Dios           : [%s]\n\r"
           "Hall           : [%5d]\n\r"
           "Recall         : [%5d]\n\r"
           "Death          : [%5d]\n\r"
           "Pit            : [%5d]\n\r",
	   flag_string( clan_flags, pClan->flags ),
           pClan->name,
           pClan->who_name,
           pClan->god,
           pClan->hall,
           pClan->recall,
           pClan->death,
           pClan->pit);
    send_to_char(buf, ch);

    return FALSE;
}

CEDIT(cedit_name)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);
	
	if (IS_NULLSTR(argument))
	{
		send_to_char( "Sintaxis : name [nombre-clan]\n\r", ch );
		return FALSE;
	}
	
	if ( clan_lookup(argument) != 0 )
	{
		send_to_char( "CEdit : Un clan con ese nombre ya existe.\n\r", ch );
		return FALSE;
	}

	free_string(pClan->name);
	pClan->name = str_dup(argument);
	
	send_to_char( "Nombre del clan seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_who)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : who [mensaje-who]\n\r", ch );
		return FALSE;
	}

	free_string(pClan->who_name);
	pClan->who_name = str_dup(argument);
	
	send_to_char( "Who_name seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_god)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : god [nombre-dios]\n\r", ch );
		return FALSE;
	}

	free_string(pClan->god);
	pClan->god = str_dup(argument);
	
	send_to_char( "Dios seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_recall)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);
	
	if (IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char( "Sintaxis : recall <vnum>\n\r", ch );
		return FALSE;
	}
	
	if ( !get_room_index(atoi(argument)) )
	{
		send_to_char( "CEdit : Cuarto no existe.\n\r", ch );
		return FALSE;
	}

	pClan->recall = atoi(argument);

	send_to_char( "Recall seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_death)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);

	if (IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char( "Sintaxis : death <vnum>\n\r", ch );
		return FALSE;
	}

	if ( !get_room_index(atoi(argument)) )
	{
		send_to_char( "CEdit : Cuarto no existe.\n\r", ch );
		return FALSE;
	}

	pClan->death = atoi(argument);

	send_to_char( "Death room seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_hall)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);

	if (IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char( "Sintaxis : hall <vnum>\n\r", ch );
		return FALSE;
	}

	if ( !get_room_index(atoi(argument)) )
	{
		send_to_char( "CEdit : Cuarto no existe.\n\r", ch );
		return FALSE;
	}

	pClan->hall = atoi(argument);

	send_to_char( "Hall room seteado.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_pit)
{
	CLAN_TYPE *pClan;
	
	EDIT_CLAN(ch,pClan);

	if (IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char( "Sintaxis : pit <vnum>\n\r", ch );
		return FALSE;
	}

	if ( !get_room_index(atoi(argument)) )
	{
		send_to_char( "CEdit : Cuarto no existe.\n\r", ch );
		return FALSE;
	}

	pClan->pit = atoi(argument);

	send_to_char( "Pit room seteado.\n\r", ch );
	return TRUE;
}

void do_clanstatus( CHAR_DATA *ch, char *argument )
{
	char buf[MIL];
	int i, pkill = 0, mkill = 0, pdeath = 0, mdeath = 0, illpk = 0, mporcent = 0;

	sprintf( buf, "#UClan       Pkills Pdeaths Mkills Mdeaths Efect Illpks#u\n\r" );
	send_to_char( buf, ch );

	for ( i = 1; i < clan_count; ++i )
	{
		sprintf( buf, "%s%-10.10s %6d %7d %6d %7d %4d%% %6d%s\n\r",
			i == clan_count - 1 ? "#U" : "",
			clan_table[i].name,
			clan_table[i].pkills,
			clan_table[i].pdeaths,
			clan_table[i].mkills,
			clan_table[i].mdeaths,
			(int) (clan_table[i].mkills / (float) (clan_table[i].mdeaths ? clan_table[i].mdeaths : 1) * 100.0),
			clan_table[i].illpkills,
			i == clan_count - 1 ? "#u" : "" );
		send_to_char( buf, ch );
		pkill += clan_table[i].pkills;
		pdeath += clan_table[i].pdeaths;
		mkill += clan_table[i].mkills;
		mdeath += clan_table[i].mdeaths;
		illpk += clan_table[i].illpkills;
		mporcent += (int) (clan_table[i].mkills / (float) (clan_table[i].mdeaths ? clan_table[i].mdeaths : 1) * 100.0);
	}
	sprintf( buf, "#B%-10.10s %6d %7d %6d %7d %4d%% %6d#b\n\r",
			"Total", pkill, pdeath,	mkill, mdeath,
			mporcent / (clan_count - 1), illpk );
	send_to_char( buf, ch );
	return;
}

CEDIT( cedit_new )
{
	struct clan_type *new_table;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	int iClan;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : new [nombre-del-nuevo-clan]\n\r", ch );
		return FALSE;
	}

	iClan = clan_lookup( argument );

	if (iClan != 0)
	{
		send_to_char ("CEdit : Un clan con ese nombre ya existe.\n\r",ch);
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_CLAN )
		  	edit_done(ch);
	}
	
	clan_count++;
	new_table = realloc( clan_table, sizeof(struct clan_type) * ( clan_count + 1 ) );
	if ( !new_table )
	{
		send_to_char( "Falla en realloc. Preparate para el impacto.\n\r", ch );
		return FALSE;
	}

	clan_table = new_table;
	clan_table[clan_count-1].name		= str_dup(argument);
	clan_table[clan_count-1].who_name	= str_dup(argument);
	clan_table[clan_count-1].god		= str_dup(argument);
	clan_table[clan_count-1].hall		= ROOM_VNUM_LIMBO;
	clan_table[clan_count-1].recall		= ROOM_VNUM_LIMBO;
	clan_table[clan_count-1].death		= ROOM_VNUM_LIMBO;
	clan_table[clan_count-1].flags		= 0;
	clan_table[clan_count-1].pkills		= 0;
	clan_table[clan_count-1].pdeaths	= 0;
	clan_table[clan_count-1].mkills		= 0;
	clan_table[clan_count-1].mdeaths	= 0;
	clan_table[clan_count-1].illpkills	= 0;

	clan_table[clan_count].name		= NULL;

	ch->desc->editor	= ED_CLAN;
	ch->desc->pEdit		= (void *) &clan_table[clan_count-1];

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

CEDIT(cedit_flags)
{
	struct clan_type *pClan;
	int value;

	EDIT_CLAN(ch,pClan);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : flags [flags]\n\r", ch );
		return FALSE;
	}

	if ( (value = flag_value( clan_flags, argument )) == NO_FLAG )
	{
		send_to_char( "CEdit : Argumento invalido.\n\r", ch );
		return FALSE;
	}

	TOGGLE_BIT(pClan->flags, value);
	send_to_char( "Flags cambiadas.\n\r", ch );
	return TRUE;
}

void borrar_clanes(void)
{
	int i;

	for ( i = 0; i < clan_count; i++ )
	{
		free_string( clan_table[i].name );
		free_string( clan_table[i].who_name );
		free_string( clan_table[i].god );
	}

	free( clan_table );
	clan_table = NULL;
	clan_count = 0;
}
