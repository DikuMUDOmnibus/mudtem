/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>	// para unlink()
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "events.h"
#include "screen.h"
#include "olc.h"
#include "smart.h"
#include "act_info.h"

int	close		args( ( int fd ) );
#if defined(linux)
int     execl           args( ( const char *path, const char *arg, ... ) );
#endif

#if defined(WIN32)
#include <process.h>
#endif

bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_asave		);
DECLARE_DO_FUN(do_puntajes	);
DECLARE_DO_FUN(do_clans		);

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
bool fight_in_progress;
void shutdown_mud_now(CHAR_DATA *);

int	shutdown_type = SHUTDOWN_NONE;

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("Saliendo de Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Bienvenido a Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Bienvenido a Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Saliendo de Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Estado de Wiznet:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Opciones de Wiznet disponibles para ti son:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("Opcion inexistente.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"No veras mas %s en wiznet.\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"Ahora veras %s en wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;
    char buf[20];

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
        &&  d->character != NULL
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch )
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("--> ",d->character);
	    if (IS_SET(d->character->wiznet,WIZ_TIMESTAMP))
	    {
		strftime(buf, 20, "[#B%H#b:#B%M#b] ", localtime(&current_time) );
		send_to_char(buf, d->character);
	    }
            act_new(string,d->character,objToEnt(obj),chToEnt(ch),TO_CHAR,POS_DEAD);
        }
    }
 
    return;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;
    int level = CLAN_GOMA;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if (arg3[0]) {
	if (arg3[0] != 0 && !str_prefix(arg3,"goma"))
	    level = CLAN_GOMA;
	else if (arg3[0] != 0 && !str_prefix(arg3,"comandante"))
	    level = CLAN_COMANDANTE;
	else if (arg3[0] != 0 && !str_prefix(arg3,"ministro"))
	    level = CLAN_MINISTRO;
	else if (arg3[0] != 0 && !str_prefix(arg3,"senador"))
	    level = CLAN_SENADOR;
	else if (arg3[0] != 0 && !str_prefix(arg3,"dictador"))
	    level = CLAN_DICTADOR;
	else
	    level = -1;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || level == -1)
    {
        send_to_char( "Syntax: guild <char> <cln name> [leader]\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
    	send_to_char( "No en NPC's.\n\r", ch );
    	return;
    }

    if (getNivelPr(victim) < 10) {
	send_to_char("Their level is too low to join a clan.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2,"none"))
    {
	send_to_char("They are now clanless.\n\r",ch);
	send_to_char("You are now a member of no clan!\n\r",victim);
	victim->clan = 0;
	victim->pcdata->clan_status = 0;
	return;
    }

    if ((clan = clan_lookup(arg2)) == 0)
    {
	send_to_char("No such clan exists.\n\r",ch);
	return;
    }

    if (ES_INDEP(clan))
    {
	sprintf(buf,"They are now a %s.\n\r", get_clan_table(clan)->name);
	send_to_char(buf,ch);
	sprintf(buf,"You are now a %s.\n\r", get_clan_table(clan)->name);
	send_to_char(buf,victim);
    }
    else
    {
	sprintf(buf,"They are now %s of clan %s.\n\r",
	    lookup_clan_status(level),
	    capitalize(get_clan_table(clan)->name));
	send_to_char(buf,ch);
	sprintf(buf,"You are now %s of clan %s.\n\r",
	    lookup_clan_status(level),
	    capitalize(get_clan_table(clan)->name));
	send_to_char(buf,victim);
    }

    victim->clan = clan;
    victim->pcdata->clan_status = level;
}

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if (getNivelPr(ch) > 5 || IS_NPC(ch))
    {
	send_to_char("Buscalos tu mismo!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    if ( (ch->race == RACE_VAMPIRE) && (( obj = get_eq_char( ch, WEAR_LENTES ) ) == NULL) )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_LENTES), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LENTES );
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
    	sn = 0; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->pcdata->learned[sn] < 
		ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

    	obj = create_object(get_obj_index(vnum),0);
	obj->cost = 0;
     	obj_to_char(obj,ch);
    	equip_char(ch,obj,WEAR_WIELD);
    }

    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
    ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
    &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    send_to_char("Has sido equipado por Mota.\n\r",ch);
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel a quien?\n\r", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "Fallaste.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "Los Dioses han devuelto tus privilegios de uso de canales.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removido.\n\r", ch );
	sprintf(buf,"$N devuelve uso de canales a %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "Los Dioses han anulado tus privilegios de uso de canales.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS activado.\n\r", ch );
	sprintf(buf,"$N anula uso de canales a %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "No puedes mostrar tus emociones.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote que?\n\r", ch );
        return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("Debes incluir tu nombre en un smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	if (argument[0] == '\0')
	{
	    sprintf(buf,"Tu poofin es %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("Debes incluir tu nombre.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Tu poofin es ahora %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        if (argument[0] == '\0')
        {
            sprintf(buf,"Tu poofout es %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("Debes incluir tu nombre.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Tu poofout es ahora %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "No en NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "Se te nego el acceso!\n\r", victim );
    sprintf(buf,"$N niega el acceso de %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Desconectar a quien?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N no tiene un descriptor.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor no encontrado!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Sintaxis: pardon <jugador> <killer|thief>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "No en NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removido.\n\r", ch );
	    send_to_char( "Ya no eres un ASESINO.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removido.\n\r", ch );
	    send_to_char( "Ya no eres un LADRON.\n\r", victim );
	}
	return;
    }

    send_to_char( "Sintaxis: pardon <jugador> <killer|thief>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo que?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "\n*** ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_act( CHAR_DATA *ch, char *argument )
{
	act( argument, ch, NULL, NULL, TO_ALL );
}

void do_info( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MIL];

	if (ch)
	{
		if ( IS_NPC(ch) )
			return;

		if ( IS_NULLSTR(argument) )
		{
			TOGGLE_BIT(ch->comm, COMM_NOINFO);
			printf_to_char( ch, "Canal de Informacion %s.\n\r",
				!IS_SET(ch->comm, COMM_NOINFO) ?
				"#BACTIVADO#b" : "desactivado" );
			return;
		}

		if ( !IS_IMMORTAL(ch) )
		{
			send_to_char( "Huh?\n\r", ch );
			return;
		}
	}

	sprintf( buf, "\n\r[#BINFO#b] %s%s", argument, ch ? "\n\r" : "" );

	for ( d = descriptor_list; d; d = d->next )
		if ( d->connected == CON_PLAYING
		&&   d->character
		&&  !IS_SET(d->character->comm, COMM_NOINFO) )
		{
			if ( ch == NULL )
				write_to_buffer( d, buf, 0 ); // msgs importantes pasan directo
			else
				send_to_char( buf, d->character );
		}
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo que?\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo que?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo que?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Objetivo no encontrado.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return (atoi(arg) < 0 ? NULL : get_room_index( atoi( arg ) ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transferir a quien (y donde)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character )
	    &&	 getNivelPr(d->character) <= getNivelPr(ch) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No existe ese lugar.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "Ese cuarto es privado en este momento.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "Esta en el limbo.\n\r", ch );
	return;
    }

    if ( getNivelPr(victim) > getNivelPr(ch) )
    {
    	send_to_char( "No puedes transferir a un superior.\n\r", ch );
    	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n es rodeado por una nube y desaparece.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n aparece desde una nube de humo.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n te ha transferido.", ch, NULL, chToEnt(victim), TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At que donde?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No existe ese lugar.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "Ese cuarto es privado en este momento.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto donde?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No existe ese lugar.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char( "Ese cuarto es privado en este momento.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,strToEnt(ch->pcdata->bamfout,ch->in_room),chToEnt(rch),TO_VICT);
	    else
		act("$n desaparece dentro de una nube de humo.",ch,NULL,chToEnt(rch),TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,strToEnt(ch->pcdata->bamfin,ch->in_room),chToEnt(rch),TO_VICT);
            else
                act("$n aparece desde una nube de humo.",ch,NULL,chToEnt(rch),TO_VICT);
        }
    }

    do_look( ch, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto donde?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No existe ese lugar.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "Ese cuarto no es privado, usa goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,strToEnt(ch->pcdata->bamfout,ch->in_room),chToEnt(rch),TO_VICT);
            else
                act("$n desaparece dentro de una nube de humo.",ch,NULL,chToEnt(rch),TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,strToEnt(ch->pcdata->bamfin,ch->in_room),chToEnt(rch),TO_VICT);
            else
                act("$n aparece desde una nube de humo.",ch,NULL,chToEnt(rch),TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Sintaxus:\n\r",ch);
	send_to_char("  stat <nombre>\n\r",ch);
	send_to_char("  stat obj <nombre>\n\r",ch);
	send_to_char("  stat mob <nombre>\n\r",ch);
 	send_to_char("  stat room <numero>\n\r",ch);
 	send_to_char("  stat char <nombre>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nada con ese nombre en este lado de la realidad.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    AFFECT_DATA *pAf;
    EXIT_DATA *pexit;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No existe ese lugar.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "Ese cuarto es privado en este momento.\n\r", ch );
	return;
    }

    sprintf( buf, "Nombre: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %s.\n\rDescripcion:\n\r%s",
	flag_string( room_flags, location->room_flags ),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Jugadores:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjetos:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( pexit = location->exits; pexit; pexit = pexit->next )
    {
	    sprintf( buf,
		"Puerta: %d.  Hacia: %d.  Llave: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Descripcion: %s",
		pexit->direccion,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(ninguna).\n\r" );
	    send_to_char( buf, ch );
    }

    for ( pAf = location->affected; pAf; pAf = pAf->next )
    {
    	sprintf( buf, "#BRAff#b: Spell '%15s'  Nivel:%2d  Duracion:%2d  Anade bit #B%s#b.\n\r",
    		skill_table[pAf->type].name,
    		pAf->level,
    		pAf->duration,
    		flag_string( room_flags, pAf->bitvector ) );
    	send_to_char( buf, ch );
    }

    return;
}

void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat que?\n\r", ch );
	return;
    }

    obj = get_obj_world( ch, argument );

    if (!obj)
    {
	OBJ_INDEX_DATA *temp;
	char *name = NULL;
	temp = get_obj_index( atoi(argument) );
	if (temp)
		name = temp->name;
	if (name)
		obj = get_obj_world( ch, name );
    }

/*  if ( ( obj = get_obj_world( ch, argument ) ) == NULL ) */

    if ( !obj )
    {
	send_to_char( "Nada como eso en el cielo, la tierra y el infierno.\n\r", ch );
	return;
    }

    sprintf( buf, "Nombre(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Tipo: %s  Resets: %d Material: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_name(obj->item_type), obj->pIndexData->reset_num,
	obj->material );
    send_to_char( buf, ch );

    sprintf( buf, "Descripcion corta: %s\n\rDescription larga: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\r", flag_string( wear_flags, obj->wear_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Extra bits: %s\n\r", flag_string( extra_flags, obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Numero: %d/%d  Peso: %d/%d/%d (10th libras)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Nivel: %d  Costo: %d  Condicion: %d  Timer: %d Owner: %s\n\r",
	obj->level, obj->cost, obj->condition, obj->timer,
	( obj->owner ? obj->owner : "nadie" ) );
    send_to_char( buf, ch );

    sprintf( buf, "Count:  Actual: %d  Maximo: %d  Detected: %s\n\r",
    	obj->pIndexData->count,
    	obj->pIndexData->max_count,
    	print_flags(obj->detected) );
    send_to_char( buf, ch );

    sprintf( buf,
	"En cuarto: %d  Dentro de: %s  Llevado por: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(ninguno)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(nadie)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "alguien",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Valores: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Spells de nivel %d de :", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name,ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Tiene %d(%d) cargas de nivel %d de",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    sprintf(buf,"Tiene %s de color %s.\n\r",
		liq_table[obj->value[2]].liq_name,
		liq_table[obj->value[2]].liq_color);
	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Tipo de arma es ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotica\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("espada\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("daga\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("lanza/vara\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mazo/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("hacha\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("latigo\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
	    	default: 
		    send_to_char("desconocida\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Dano es %dd%d (promedio %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Dano es %d hasta %d (promedio %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Verbo del dano es %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "desconocido");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    flag_string( weapon_type2, obj->value[4] ) );
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Clase de armadura es %d pierce, %d bash, %d slash, y %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
            sprintf(buf,"Capacidad: %d#  Peso maximo: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3],
                flag_string( container_flags, obj->value[1] ) );
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Multiplicador de peso: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    if ( obj->enchanted )
    	send_to_char( "Objeto esta encantado.\n\r", ch );

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Afecta %s por %d, nivel %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d horas.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n\r",
			flag_string( affect_flags, paf->bitvector ) );
		    break;
		case TO_AFFECTS2:
		    sprintf(buf,"Adds %s extended affects.\n\r",
		    	flag_string( affect2_flags, paf->bitvector ) );
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n\r",
                        flag_string( weapon_type2, paf->bitvector ) );
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n\r",
			flag_string( extra_flags, paf->bitvector ) );
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"Adds immunity to %s.\n\r",
			flag_string( imm_flags, paf->bitvector ));
		    break;
		case TO_RESIST:
		    sprintf(buf,"Adds resistance to %s.\n\r",
			flag_string( imm_flags, paf->bitvector ));
		    break;
		case TO_VULN:
		    sprintf(buf,"Adds vulnerability to %s.\n\r",
			flag_string( imm_flags, paf->bitvector ));
		    break;
		case TO_PARTS:
		    sprintf(buf,"Anade parte %s.\n\r",
		    	flag_string( part_flags, paf->bitvector ));
		    break;
		default:
		    sprintf(buf,"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "#B*#b Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n\r",
                        flag_string( affect_flags, paf->bitvector ) );
                    break;
                case TO_AFFECTS2:
                    sprintf(buf,"Adds %s extended affects.\n\r",
                    	flag_string( affect2_flags, paf->bitvector ) );
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n\r",
                        flag_string( extra_flags, paf->bitvector ) );
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n\r",
                        flag_string( imm_flags, paf->bitvector ));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        flag_string( res_flags, paf->bitvector ));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        flag_string( vuln_flags, paf->bitvector ));
                    break;
                case TO_PARTS:
                    sprintf(buf,"Anade parte %s.\n\r",
                    	flag_string( part_flags, paf->bitvector ));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    if ( IS_TRAP(obj) )
    {
    	sprintf( buf, "Tipo de dano de la trampa es %s.\n\r",
    			flag_string( trapd_flags, obj->trap->trap_dam) );
    	send_to_char( buf, ch );
    	sprintf( buf, "Efecto de la trampa es %s.\n\r",
    			flag_string( trap_flags, obj->trap->trap_eff) );
    	send_to_char( buf, ch );
    	sprintf( buf, "Cargas de la trampa : %d.\n\r", obj->trap->trap_charge );
    	send_to_char( buf, ch );
    }

    if ( obj->pIndexData->max_count > 0 )
    {
    	LIMIT_DATA * limit;
    	int cnt = 1;

	for ( limit = obj->pIndexData->limit; limit; limit = limit->next )
	{
		CHAR_DATA * temp = get_char_from_id(limit->carrier_id);

		sprintf( buf, "%d. %3d %ld(%s)\n\r",
			cnt++, limit->id, limit->carrier_id, temp ? temp->name : "" );
		send_to_char( buf, ch );
	}
    }

    return;
}

void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim, *blah;
    FIGHT_DATA * fd;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "id" ) )
    {
 	victim = get_char_world( ch, arg );

 	if (!victim)
 	{
		MOB_INDEX_DATA *temp;
		char *name = NULL;
		temp = get_mob_index( atoi(arg) );
		if (temp)
			name = temp->player_name;
		if (name)
			victim = get_char_world( ch, name );
 	}
    }
    else
    	victim = get_char_from_id( atol(argument) );

    if ( !victim )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %s\n\r",
	victim->name);
    send_to_char( buf, ch );

    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s(%s)  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	(victim->race == victim->true_race) ? "" : race_table[victim->true_race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d  Timer: %d  Safe: %c  Shop: %c  Repair: %c\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed,
	    victim->timer, is_mob_safe(victim) ? 's' : 'n',
	    victim->pIndexData->pShop != NULL ? 's' : 'n',
	    victim->pIndexData->pRepair != NULL ? 's' : 'n');
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d Trains: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice,
	IS_NPC(ch) ? 0 : victim->train );
    send_to_char( buf, ch );
	
    sprintf( buf,
	"Lv: %d  Class: %s  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
	getNivelPr(victim),       
	IS_NPC(victim) ? "mobile" : class_table[getClasePr(victim)].name,            
	victim->alignment,
	victim->gold, victim->silver, victim->exp );
    send_to_char( buf, ch );

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }

    sprintf( buf, "Wait: %d  Daze: %d Clan: %d ClanSt: %d(%d/%d) Clase: %s\n\r",
	victim->wait, victim->daze,
	victim->clan,
	IS_NPC(victim) ? 0 : victim->pcdata->clan_status,
	IS_NPC(victim) ? 0 : victim->pcdata->min_clan_status,
	IS_NPC(victim) ? 0 : victim->pcdata->max_clan_status,
	class_table[getClasePr(victim)].name );
    send_to_char( buf, ch );

    sprintf( buf, "Fighting: %s  On: %s\n\r",
    	victim->fighting	? victim->fighting->name	: "(none)",
    	victim->on		? victim->on->name		: "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  Term: %s\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->desc ? term_table[victim->desc->term].name : "sin descriptor" );
	send_to_char( buf, ch );
    }

    sprintf( buf, "Carry number: %d  Carry weight: %ld  Luck: %d  Aire: %d\n\r",
	victim->carry_number, get_carry_weight(victim) / 10,
	victim->luck, victim->aire );
    send_to_char( buf, ch );

    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d  Quaff: %d  Corpse: %s\n\r",
	    get_age(victim),
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer,
	    victim->pcdata->quaff,
	    victim->pcdata->corpse ? "S" : "N" );
	send_to_char( buf, ch );
    }

    sprintf(buf, "Act: %s\n\r", flag_string( IS_NPC(victim) ? act_flags : plr_flags, victim->act ) );
    send_to_char(buf,ch);

    if (victim->comm)
    {
    	sprintf(buf,"Comm: %s\n\r", flag_string( comm_flags, victim->comm ) );
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
	sprintf(buf, "Offense: %s\n\r", flag_string( off_flags, victim->off_flags ));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r", flag_string( imm_flags, victim->imm_flags ));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", flag_string( res_flags, victim->res_flags ));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", flag_string( vuln_flags, victim->vuln_flags ));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\r", flag_string( form_flags, victim->form ) );
    send_to_char(buf,ch);
    sprintf(buf, "Parts: %s\n\r", flag_string( part_flags, victim->parts) );
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    flag_string( affect_flags, victim->affected_by ) );
	send_to_char(buf,ch);
    }

    if (victim->affected2_by)
    {
         sprintf(buf, "Also affected by %s\n\r",
            flag_string( affect2_flags, victim->affected2_by ) );
            send_to_char(buf,ch);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char( buf, ch );

    if (!IS_NPC(victim))
    {
	struct prob_data *prob;
	BANK_DATA *bank;

	sprintf( buf, "Security: %d Qpoints: %d Qtimer: %d Qtype: %d Qestado: %d Qid: %ld\n\r",
		victim->pcdata->security,
		get_qpoints(victim),
		get_qtimer(victim),
		get_qtype(victim),
		get_qestado(victim),
		get_qid(victim) );
	send_to_char( buf, ch ); /* OLC */

	for ( prob = victim->pcdata->prohibido; prob; prob = prob->next )
	{
		sprintf( buf, "Comando '%s' prohibido.\n\r", prob->comando );
		send_to_char( buf, ch );
	}

	for (bank = victim->pcdata->bank; bank; bank = bank->next )
	{
		sprintf( buf, "Banco : Valor %8ld Tipo %2d Interes %1.3f Cuando: %s\n\r",
			bank->valor, bank->tipo, bank->interes, tformat(bank->when) );
		send_to_char( buf, ch );
	}
    }
    
    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->pIndexData->script )
    {
    	sprintf( buf, "Script %d, posicion %d, timer %d.\n\r",
    		victim->pIndexData->script->vnum,
    		victim->pIndexData->script->posicion,
    		victim->pIndexData->script->timer );
    	send_to_char( buf, ch );
    }

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    if ( IS_NPC(victim) && victim->game_fun != 0 )
    {
    	sprintf(buf,"Mobile tiene game_fun %s.\n\r",
    		game_name(victim->game_fun));
    	send_to_char(buf,ch);
    }

    if ( victim->memory )
    {
	MEM_DATA *memory = victim->memory;

	while ( memory )
	{ 
	   	sprintf(buf, "Memoria: Id: %d (%s) Reaccion: %s Cuando: %s",
	    		memory->id,
	    		(blah = get_char_from_id( memory->id )) ? blah->name : "none",
	    		flag_string( mem_types, memory->reaction),
	    		ctime(&memory->when) );
	    	send_to_char( buf, ch );
		memory = memory->next;
	}
    }

    if ( IS_NPC(victim) && is_hunting(victim) )
    {
	CHAR_DATA * temp = get_char_from_id( victim->hunt_data->id );

        sprintf(buf, "Cazando victima: %s (%ld/%s) - Buscando cuarto: %d\n\r",
		victim->hunt_data->victima ? SELFPERS(victim->hunt_data->victima) : "*",
		victim->hunt_data->id,
		temp ? SELFPERS(temp) : "",
		huntroom(victim) ? huntroom(victim)->vnum : 0 );
        send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    flag_string( paf->where != TO_AFFECTS2 ? affect_flags : affect2_flags, paf->bitvector ),
	    paf->level
	    );
	send_to_char( buf, ch );
    }

    for ( fd = victim->fdata; fd; fd = fd->next )
    {
    	sprintf( buf, "Fdata : Id %ld(%s) Dam %d When %s",
    		fd->id, (blah = get_char_from_id(fd->id)) ? blah->name : "",
    		fd->dam, ctime(&fd->when) );
    	send_to_char( buf, ch );
    }

    show_event_list( ch, victim->events );

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found, fArea = FALSE;
    int number = 0, max_found;
    char *oldarg;
    char arg[MIL];

    found = FALSE;
    number = 0;
    max_found = 200;

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    buffer = new_buf();

    oldarg = argument;
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "area" ) )
	fArea = TRUE;
    else
    	argument = oldarg;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj )
        ||   !is_name( argument, obj->name )
        ||   (fArea && obj->pIndexData->area != ch->in_room->area)
        ||    getNivelPr(ch) < obj->level)
            continue;

        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s(%d) is carried by %s(%d) [Room %d]\n\r",
                number, obj->short_descr, obj->level,
                PERS(in_obj->carried_by, ch), getNivelPr(in_obj->carried_by),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s(%d) is in %s [Room %d]\n\r",
                number, obj->short_descr, obj->level,
                in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);

        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0, act;
    char * temparg;

    temparg = one_argument(argument, buf);

    if ( !IS_NULLSTR(buf) && !str_cmp(buf, "act") )
    	act = flag_value( act_flags, temparg );
    else
    	act = -1;
    	
    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&  ( (act != -1 && IS_NPC(victim) && (victim->act & act) > 0)
	  ||   is_name( argument, victim->name ) ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    extern void reboot_event( EVENT * ); /* update.c */

    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg, "en" ) )
    {
    	int value = atoi( argument );
    	
    	if ( value < 1 || value > 30 )
    	{
    		send_to_char( "Valores entre 1 y 30.\n\r", ch );
    		return;
    	}
    	
	shutdown_type = SHUTDOWN_REBOOT;
	generic_event_add( PULSE_PER_SECOND, (void *) (MINUTOS(value) + 1), reboot_event );
    	send_to_char( "Ok.\n\r", ch );
    	return;
    }

    if ( str_cmp( arg, "ahora" ) )
    {
    	send_to_char( "Sintaxis : reboot en [minutos]\n\r", ch );
    	send_to_char( "           reboot ahora\n\r", ch );
    	return;
    }

    do_asave( ch, "changed" );

    shutdown_type = SHUTDOWN_REBOOT;
    shutdown_mud_now(ch);

    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    extern void reboot_event( EVENT * ); /* update.c */

    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg, "en" ) )
    {
    	int value = atoi( argument );
    	
    	if ( value < 1 || value > 30 )
    	{
    		send_to_char( "Valores entre 1 y 30.\n\r", ch );
    		return;
    	}
    	
	shutdown_type = SHUTDOWN_NORMAL;
	generic_event_add( PULSE_PER_SECOND, (void *) (MINUTOS(value) + 1), reboot_event );
    	send_to_char( "Ok.\n\r", ch );
    	return;
    }

    if ( str_cmp( arg, "ahora" ) )
    {
    	send_to_char( "Sintaxis : shutdown en [minutos]\n\r", ch );
    	send_to_char( "           shutdown ahora\n\r", ch );
    	return;
    }

    shutdown_type = SHUTDOWN_NORMAL;
    shutdown_mud_now(ch);

    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,chToEnt(victim),TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,chToEnt(victim),TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) 
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    victim->comm	= ch->comm;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );

    if (!IS_NPC(ch) && ch->pcdata->prompt != NULL)
    {
	free_string(ch->pcdata->prompt);
	ch->pcdata->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,objToEnt(clone),NULL,TO_ROOM);
	act("You clone $p.",ch,objToEnt(clone),NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_object;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((getNivelPr(mob) > 20 && !IS_TRUSTED(ch,GOD))
	||  (getNivelPr(mob) > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (getNivelPr(mob) >  5 && !IS_TRUSTED(ch,DEMI))
	||  (getNivelPr(mob) >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);

	while ( clone->affected != NULL )
		affect_strip( clone, clone->affected->type );

	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_object = create_object(obj->pIndexData,0);
		clone_object(obj,new_object);
		recursive_clone(ch,obj,new_object);
		obj_to_char(new_object,clone);
		new_object->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,chToEnt(clone),TO_ROOM);
        act("You clone $N.",ch,NULL,chToEnt(clone),TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	send_to_char("  load clans\n\r", ch );
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"clans"))
    {
	borrar_clanes();
	cargar_clanes();
    	do_clans(ch, NULL);
    	return;
    }

    /* echo syntax */
    do_load(ch,"");
}

void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, chToEnt(victim), TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, objToEnt(obj), NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	if ( getNivelPr(ch) < GOD
	&&  !IS_BUILDER(ch, ch->in_room->area) )
	{
		send_to_char( "No puedes hacerlo.\n\r", ch );
		return;
	}

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj, TRUE );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( (obj = get_obj_list( ch, arg, ch->carrying )) != NULL )
    {
	if ( !IS_IMMORTAL(ch) && IS_OBJ_STAT(obj, ITEM_PROTOTIPO) )
	{
		send_to_char( "No puedes hacerlo.\n\r", ch );
		return;
	}

    	if ( !IS_IMMORTAL(ch) && !IS_OBJ_STAT(obj, ITEM_NOPURGE) )
    	{
    		act( "No puedes purgear $p.", ch, objToEnt(obj), NULL, TO_CHAR );
    		return;
    	}

	act( "$p se desvanece.", ch, objToEnt(obj), NULL, TO_CHAR );
	extract_obj(obj, TRUE);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,chToEnt(victim),TO_NOTVICT);

    	if (getNivelPr(victim) > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    if ( getNivelPr(ch) < GOD
    &&   IS_NPC(victim)
    &&  !IS_SET(victim->act, ACT_PROTOTIPO) )
    {
	send_to_char( "No puedes hacerlo.\n\r", ch );
	return;
    }

    act( "$n purges $N.", ch, NULL, chToEnt(victim), TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= getNivelPr(victim) )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	setNivelPr(victim,1);
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim, TRUE );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = getNivelPr(victim) ; iLevel < level; iLevel++ )
    {
	setNivelPr(victim, getNivelPr(victim) + 1);
	advance_level( victim,TRUE);
    }
    sprintf(buf,"You are now level %d.\n\r",getNivelPr(victim));
    send_to_char(buf,victim);
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, getNivelPr(victim) );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            affect_strip(vch,gsn_amnesia);
	    affect_strip(vch,gsn_mute);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
            update_pos( vch);
            act("$n has restored you.",ch,NULL,chToEnt(vch),TO_VICT);
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
	    affect_strip(victim,gsn_mute);
            affect_strip(victim,gsn_amnesia);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,chToEnt(victim),TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    affect_strip(victim,gsn_mute);
    affect_strip(victim,gsn_amnesia);
    affect_strip(victim,skill_lookup("faerie fire"));
    affect_strip(victim,skill_lookup("estupidez"));
    victim->hit 	= victim->max_hit;
    victim->mana	= victim->max_mana;
    victim->move	= victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, chToEnt(victim), TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "Fallaste.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set char  <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char *pArg;
    char cEnd;
    char buf[100];
    CHAR_DATA *victim;
    int value,max;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
/*    strcpy( arg3, argument ); */

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg3;
    while ( isspace( *argument ) )
        argument++;
    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';
    while ( isspace( *argument ) )
        argument++;

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex class level\n\r",	ch );
	send_to_char( "    race group gold silver hp mana move prac\n\r",ch);
	send_to_char( "    align train thirst hunger drunk full\n\r",ch );
	send_to_char( "    learn hunt security whotext questpoints\n\r", ch);
	send_to_char( "    timer truerace questtimer minclanst maxclanst\n\r", ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d.\n\r",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "questpoints" ) )
    {
	if ( value < 1 || value > 3000 )
	{
	    sprintf(buf, "Rango es entre 1 y 3000.\n\r");
	    send_to_char(buf,ch);
	    return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Victima debe ser un PC.\n\r",ch);
		return;
	}

	give_qpoints( victim, value );
	return;
    }

    if ( !str_cmp( arg2, "questtimer" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf, "Rango es entre 1 y 30.\n\r");
	    send_to_char(buf,ch);
	    return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Victima debe ser un PC.\n\r",ch);
		return;
	}

	set_qtimer( victim, value );
	return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
    	if ( value < 0 )
    	{
    		send_to_char( "Solo valores positivos.\n\r", ch );
    		return;
    	}
    	
	if ( !IS_NPC(victim) )
	{
		send_to_char( "Victima debe ser un NPC.\n\r", ch );
		return;
	}

    	victim->timer = value;
    	return;
    }

    /*
     * Whotext by Canth, phule@xs4all.nl
     */
    if ( !str_cmp( arg2, "whotext" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        /* If the string is longer than 17 chars, cut it off */
        if ( strlen_color ( arg3 ) > 13 )
            arg3[13] = '\0';

        /*
         * Bugfix for resetting who_text by Chil (bbailey@mail.public.lib.ga.us)
         */
        if ( !str_cmp( arg3, "@" ) )
        {
	    free_string(victim->pcdata->who_text);
            victim->pcdata->who_text = str_dup( "@" );
            return;
        }

        /* add (17-length of word)/2 spaces behind arg3 */
        /* This is needed to centre the text in the who listing */
        /* If anyone's got a better way, I'm listening :) */

        max = ( ( 13 - strlen_color(arg3) ) / 2 );
        while ( max >=1 )
        {
            strcat ( arg3, " " );
            max--;
        }
        free_string ( victim->pcdata->who_text );
        victim->pcdata->who_text = str_dup( arg3 );
        return;
    }

    if ( !str_cmp( arg2, "learn" ) )
    {
        if (IS_NPC(victim))
                send_to_char("Not on mob's\n\r", ch);
        else
                victim->pcdata->learn = value;
        return;
    }

    if ( !str_cmp( arg2, "minclanst" ) )
    {
        if (IS_NPC(victim))
                send_to_char("Not on mob's\n\r", ch);
        else
                victim->pcdata->min_clan_status = value;
        return;
    }

    if ( !str_cmp( arg2, "maxclanst" ) )
    {
    	if ( IS_NPC(victim) )
    		send_to_char( "No en mobs.\n\r", ch );
    	else
    		victim->pcdata->max_clan_status = value;
    	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf2[MAX_STRING_LENGTH];

        	strcpy( buf2, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf2, " " );
            	    strcat( buf2, class_table[class].name );
        	}
            strcat( buf2, ".\n\r" );

	    send_to_char(buf2,ch);
	    return;
	}

	setClasePr(victim, class, getNivelPr(victim));
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    send_to_char( "Level range is 0 to 60.\n\r", ch );
	    return;
	}
	setNivelPr(victim, value);
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;

	return;
    }
   
    if (!str_prefix( arg2, "truerace" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->true_race = race;

	return;
    }
   
    /* Change their hunting status. */
    if (!str_prefix(arg2, "hunt"))
    {
        CHAR_DATA *hunted = 0;

        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

	hunted = get_char_world( victim, arg3 );
	
	if ( !hunted || !can_hunt(victim,hunted,TRUE) )
	{
		send_to_char( "Nada sucede.\n\r", ch );
		return;
	}

	set_hunt( victim, hunted, TRUE );

        return;
    }

    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }


    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended owner\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
    	}

	if ( !IS_NPC(victim) && (get_trust(ch) < get_trust(victim)) )
	{
		send_to_char( "No puedes hacer eso.\n\r", ch );
		return;
	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}

	if ( !str_prefix( arg2, "game" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "No en PC's.\n\r", ch );
			return;
		}
		
		if ( ( victim->game_fun = game_lookup( arg3 ) ) == 0 )
		{
			send_to_char( "No existe ese game_fun.\n\r", ch );
			return;
		}
		
		return;
	}
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

	if ( !str_prefix( arg2, "owner" ) )
	{
	    free_string( obj->owner );
	    obj->owner = str_dup( arg3 );
	    return;
	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer condition\n\r",	ch );
	send_to_char("    trapeff trapdam charges limit\n\r",			ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_cmp( arg2, "condition" ) )
    {
    	obj->condition = value;
    	return;
    }
    
    if ( !str_cmp( arg2, "trapeff" ) && IS_TRAP(obj) )
    {
	obj->trap->trap_eff = value;
    	return;
    }
    
    if ( !str_cmp( arg2, "limit" ) )
    {
    	obj->pIndexData->max_count = value;
    	return;
    }

    if ( !str_cmp( arg2, "trapdam" ) && IS_TRAP(obj) )
    {
	obj->trap->trap_dam = value;
    	return;
    }
    
    if ( !str_cmp( arg2, "charges" ) && IS_TRAP(obj) )
    {
	obj->trap->trap_charge = value;
    	return;
    }
    
    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';

    one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL && can_see( ch, d->character ) 
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))
        {
            char addr[MAX_STRING_LENGTH];
            
            count++;
            if ( !str_cmp( d->ident, "???" ) )
                strcpy( addr, d->host );
            else
            	sprintf( addr, "%s@%s", d->ident, d->host );
/*          sprintf( buf + strlen(buf), "[%3d %2d] %s (#B%s#b)\n\r", */
            sprintf( buf + strlen(buf), "[%3d %-14s] %s (#B%s#b)\n\r",
                d->descriptor,
                flag_string(con_table,d->connected),
                d->original  ? d->original->name  :
                d->character ? d->character->name : "(none)",
                addr
                );
        }
/*	{
	    count++;
	    sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host
		);
	} */
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    extern bool forced_player;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	forced_player = TRUE;
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, chToEnt(vch), TO_VICT );
		interpret( vch, argument );
	    }
	}
	forced_player = FALSE;
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
	forced_player = TRUE;
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 getNivelPr(vch) < LEVEL_HERO)
            {
                act( buf, ch, NULL, chToEnt(vch), TO_VICT );
                interpret( vch, argument );
            }
        }
	forced_player = FALSE;
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
	forced_player = TRUE;
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   getNivelPr(vch) >= LEVEL_HERO)
            {
                act( buf, ch, NULL, chToEnt(vch), TO_VICT );
                interpret( vch, argument );
            }
        }
	forced_player = FALSE;
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( getNivelPr(ch) <= ANGEL
	&&  !IS_SET(victim->in_room->room_flags, ROOM_PROTOTIPO) )
	{
		send_to_char( "No puedes hacerlo.\n\r", ch );
		return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	forced_player = TRUE;
	act( buf, ch, NULL, chToEnt(victim), TO_VICT );
	interpret( victim, argument );
	forced_player = FALSE;
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    {
	/* take the default path */
	if ( ch->invis_level)
	{
		ch->invis_level = 0;
		act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly fade back into existence.\n\r", ch );
	}
	else
	{
		ch->invis_level = get_trust(ch);
		act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly vanish into thin air.\n\r", ch );
	}
    }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	/* take the default path */
	if ( ch->incog_level)
	{
		ch->incog_level = 0;
		act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You are no longer cloaked.\n\r", ch );
	}
	else
	{
		ch->incog_level = get_trust(ch);
		act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You cloak your presence.\n\r", ch );
	}
    }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
    {
    	send_to_char( "No en NPC's.\n\r", ch );
    	return;
    }

    if (argument[0] == '\0')
    {
	if (ch->pcdata->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->pcdata->prefix);
	ch->pcdata->prefix = str_dup("");
	return;
    }

    if (ch->pcdata->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->pcdata->prefix);
    }
    else
	sprintf(buf,"Prefix set to %s.\r\n",argument);

    send_to_char( buf, ch );
    ch->pcdata->prefix = str_dup(argument);
}

void do_olevel(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        char arg[MAX_INPUT_LENGTH];
        BUFFER *buffer;
        OBJ_INDEX_DATA *pObjIndex;
        int vnum, level, col = 0;
        bool found, fAll;

        argument=one_argument(argument, arg);
        if (!is_number(arg) )
        {
            send_to_char("Syntax: olevel [level]\n\r",ch);
            return;
        }
        level=atoi(arg);
        buffer=new_buf();
        found=FALSE;
	fAll = !str_cmp("all", argument);
        
        for(vnum=0;vnum<=top_vnum_obj;vnum++)
        {
           if ( (pObjIndex=get_obj_index(vnum)) == NULL)
              continue;
           if (level == pObjIndex->level
           && (fAll || pObjIndex->area == ch->in_room->area) )
           {
              found=TRUE;
              sprintf(buf, "[%5d] %-17.16s ",pObjIndex->vnum,
                        pObjIndex->short_descr);
              add_buf(buffer,buf);
		if ( ++col % 3 == 0 )
		    add_buf( buffer, "\n\r" );
            }
         }

	 if ( col % 3 )
	 	add_buf( buffer, "\n\r" );

         if (!found)
             send_to_char("No objects by that level.\n\r",ch);
         else
             page_to_char(buf_string(buffer),ch);
         free_buf(buffer);
         return;
}

void do_mlevel(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        char arg[MAX_INPUT_LENGTH];
        BUFFER *buffer;
        MOB_INDEX_DATA *pMobIndex;
        int vnum, level, col = 0;
        bool found, fAll;

	argument=one_argument(argument, arg);
	if (!is_number(arg) )
	{
		send_to_char("Syntax: mlevel [level]\n\r",ch);
		return;
	}

	fAll = !str_cmp("all", argument);
        level=atoi(arg);
        buffer=new_buf();
        found=FALSE;
        
        for(vnum=0;vnum<=top_vnum_mob;vnum++)
        {
           if ( (pMobIndex=get_mob_index(vnum)) == NULL)
              continue;
           if (level == pMobIndex->level
           && (fAll || pMobIndex->area == ch->in_room->area) )
           {
              found=TRUE;
              sprintf(buf, "[%5d] %-17.16s",pMobIndex->vnum,
                        pMobIndex->short_descr);
              add_buf(buffer,buf);
		if ( ++col % 3 == 0 )
		    add_buf( buffer, "\n\r" );
            }
         }

	 if ( col % 3 )
	 	add_buf( buffer, "\n\r" );

         if (!found)
             send_to_char("No mobs by that level.\n\r",ch);
         else
             page_to_char(buf_string(buffer),ch);
         free_buf(buffer);
         return;
}

void do_update( CHAR_DATA *ch, char *argument )         /* by Maniac */
{
        char    arg[MAX_INPUT_LENGTH];

/*        if ( !authorized( ch, "iscore" ) )
                return; */

        if ( argument[0] == '\0' )      /* No options ??? */
        {
                send_to_char( "Update, call some game functions\n\r\n\r", ch );
                send_to_char( "bank: Update the share_value.\n\r", ch );
                return;
        }

        argument = one_argument(argument, arg);

        if (!str_prefix(arg, "bank" ) )
        {
                bank_update ( );
                send_to_char ("Ok...bank updated.\n\r", ch);
                return;
        }

        return;
}

void do_dog( CHAR_DATA *ch, char *argument )
{
   /* A real silly command which switches the (mortal) victim into
    * a mob.  As the victim is mortal, they won't be able to use
    * return ;P  So will have to be released by someone...
    * -S-
    */
 
    ROOM_INDEX_DATA *location;
    MOB_INDEX_DATA  *pMobIndex;
    CHAR_DATA       *mob;
    CHAR_DATA       *victim;
 
    if ( argument[0] == '\0' )
    {
       send_to_char( "Turn WHO into a little doggy?\n\r", ch );
       return;
    }
 
    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }
 
    if ( IS_NPC(victim) )
    {
       send_to_char( "Cannot do this to mobs, only pcs.\n\r",ch);
       return;
    }
 
    if ( ( pMobIndex = get_mob_index( MOB_VNUM_FIDO ) ) == NULL )
    {
       send_to_char( "Couldn't find the doggy's vnum!!\n\r", ch );
       return;
    }
 
    if ( victim->desc == NULL )
    {
       send_to_char( "Already switched, like.\n\r", ch );
       return;
    }

    if ( get_trust(victim) >= get_trust(ch) )
    {
	send_to_char( "You cannot dog your peer nor your superior.\n\r", ch );
	return;
    }

    send_to_char( "You suddenly turn into a small doggy!\n\r", victim );
    act( "$n is suddenly turned into a small doggy!!", victim, NULL, NULL, TO_ROOM );

    mob = create_mobile( pMobIndex );
    location = victim->in_room;         /* Remember where to load doggy! */
    char_from_room( victim );
 
    char_to_room( victim, get_room_index( ROOM_VNUM_LIMBO ) );
    char_to_room( mob, location );
 
 
    /* Instead of calling do switch, just do the relevant bit here */
    victim->desc->character = mob;
    victim->desc->original  = victim;
    mob->desc               =  victim->desc;
    victim->desc            = NULL;
 
    send_to_char( "Ok.\n\r", ch );
    return;
}

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void hacer_copyover (CHAR_DATA *ch)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100], buf3[100];
	extern int puerto,cControl,ftp_control; /* db.c */

	fp = fopen (COPYOVER_FILE, "w");

	if (!fp)
	{
		if (ch)
			send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
                flog ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}

	/* Consider changing all saved areas here, if you use OLC */
	do_asave (NULL, ""); /* autosave changed areas */

	if (ch)
		sprintf (buf, "\n\r *** COPYOVER por %s - espera unos momentos.\n\r", ch->name);
	else
		sprintf (buf, "\n\r *** COPYOVER automatico - espera unos momentos.\n\r");

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */

		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor (d->descriptor, "\n\rLo lamento, pero estamos rebooteando. Vuelve en unos minutos.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s %s\n", d->descriptor, och->name, d->host, d->ident);

			if ( och->pcdata->corpse )
			{
				save_corpse(och, FALSE);
				extract_obj(och->pcdata->corpse, FALSE); // salvemos el limit
			}
			save_char_obj (och);

			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	grabar_plist();

	/* Close reserve and other always-open files and release other resources */
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	sprintf (buf, "%d", puerto);
	sprintf (buf2, "%d", cControl);
	sprintf (buf3, "%d", ftp_control);

	if ( (fp = fopen( NEW_EXE_FILE, "r" )) != NULL )
	{
		fclose(fp);
		rename( NEW_EXE_FILE, EXE_FILE );
	}

	execl (EXE_FILE, EXE_FILE, buf, "copyover", buf2, buf3, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
	if (ch)
		send_to_char ("Copyover FAILED!\n\r",ch);

	/* Here you might want to reopen fpReserve */
	fpReserve = fopen (NULL_FILE, "r");
}

void do_copyover( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    extern void reboot_event( EVENT * ); /* update.c */

    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg, "en" ) )
    {
    	int value = atoi( argument );
    	
    	if ( value < 1 || value > 30 )
    	{
    		send_to_char( "Valores entre 1 y 30.\n\r", ch );
    		return;
    	}

	shutdown_type = SHUTDOWN_COPYOVER;
	generic_event_add( PULSE_PER_SECOND, (void *) (MINUTOS(value) + 1), reboot_event );
    	send_to_char( "Ok.\n\r", ch );
    	return;
    }

    if ( str_cmp( arg, "ahora" ) )
    {
    	send_to_char( "Sintaxis : copyover en [minutos]\n\r", ch );
    	send_to_char( "           copyover ahora\n\r", ch );
    	return;
    }

    hacer_copyover(ch);

    return;
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
extern	void war_enter( DESCRIPTOR_DATA *, CHAR_DATA * );
	FILE *fp;
	char name [100];
	char host[MSL];
	char ident [100];
	int desc;
	bool fOld;
	
        flog ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
                flog ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s %s\n", &desc, name, host, ident );

		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */		
		if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r",0))
		{
			close (desc); /* nope */
			continue;
		}
		
		d		= new_descriptor();
		d->descriptor	= desc;
		
		d->host		= str_dup (host);
		d->ident	= str_dup (ident);
		d->next		= descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
	
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name);
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
			write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0);
	
			/* Just In Case */
			if (!d->character->was_in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);
			else
				d->character->in_room = d->character->was_in_room;

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room (d->character, d->character->in_room);
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;

			if (war == TRUE)
				war_enter(d, d->character);
			else
			if (d->character->pet != NULL)
			{
			    char_to_room(d->character->pet,d->character->in_room);
			    act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
			}

			do_look (d->character, "auto");
		}
	}
	fclose( fp );
}

void shutdown_mud_now( CHAR_DATA *ch )
{
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;
    char buf[MIL];
    char *stype = NULL;

    switch(shutdown_type)
    {
    	case SHUTDOWN_REBOOT:
    	stype = "Reboot";
    	break;

	case SHUTDOWN_COPYOVER:
	stype = "Copyover";
	break;

	case SHUTDOWN_NORMAL:
	stype = "Shutdown";
	break;

	case SHUTDOWN_NONE:
	bugf( "shutdown_mud_now : shutdown_type == none" );
	return;

	default:
	bugf( "shutdown_mud_now : shutdown_type = %d", shutdown_type );
	shutdown_type = SHUTDOWN_NONE;
	return;
    }

    if ( ch && ch->invis_level < LEVEL_HERO )
    	sprintf( buf, "\n\r\n\r*** %s por %s.\n\r\n\r", stype, ch->name );
    else
	sprintf( buf, "\n\r\n\r*** %s automatico.\n\r\n\r", stype );

    if (shutdown_type == SHUTDOWN_NORMAL)
    {
    	FILE *fp = fopen( SHUTDOWN_FILE, "w" );

	if ( !fp )
		bugf( "shutdown_mud_now : NULL fp" );
	else
	{
		fprintf( fp, "Shutdown por %s.\n", ch ? ch->name : "alguien" );
		fclose( fp );
	}
    }

    do_asave( NULL, "" );

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	{
	    if ( vch->pcdata->corpse )
	    {
		save_corpse(vch, FALSE);
		extract_obj(vch->pcdata->corpse, FALSE);
	    }
	    save_char_obj(vch);
	}
	write_to_descriptor (d->descriptor, buf, 0);
    	close_socket(d);
    }
}

void do_destruccion( CHAR_DATA *ch, char *argument )
{
	extern bool destruccion;
	char buf[MIL];

	if (IS_NULLSTR(argument))
	{
		sprintf( buf, "Modo #BDESTRUCCION#b : #B%s#b.\n\r",
			destruccion ? "ON" : "off" );
		send_to_char( buf, ch );
		return;
	}

	if (!str_cmp(argument, "on"))
	{
		sprintf( buf, "Modo #BDESTRUCCION#b : #BACTIVADO#b.\n\r" );
		do_info( NULL, buf );
		destruccion = TRUE;
		return;
	}

	if (!str_cmp(argument,"off"))
	{
		sprintf( buf, "Modo #BDESTRUCCION#b : #BDESACTIVADO#b.\n\r" );
		if (destruccion)
			do_info( NULL, buf );
		destruccion = FALSE;
		return;
	}

	send_to_char( "Sintaxis : destruccion [on/off]\n\r", ch );
	return;
}
