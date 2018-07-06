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
 **************************************************************************/

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
#if !defined(WIN32)
#include <sys/time.h>
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>	// para atoi()
#include <time.h>
#include <ctype.h>	// para toupper()
#include <unistd.h>	// para unlink()

#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "plist.h"
#include "screen.h"
#include "lookup.h"
#include "comm.h"
#include "language.h"
#include "auction.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);
DECLARE_DO_FUN(do_clear	);

void do_quote_exit(CHAR_DATA *ch);

#define FORCE_LANGUAGE

char    *makedrunk      args( (char *string ,CHAR_DATA *ch) );
bool	can_talk	args( (CHAR_DATA *ch) );

/*
 * Drunk struct
 */
struct struckdrunk
{
        int     min_drunk_level;
        int     number_of_rep;
        char    *replacement[11];
};

int esta_ignorando( CHAR_DATA *ch, char *str )
{
	int i;

	if (IS_NPC(ch))
		return -1;
	
	for ( i = 0; i < MAX_IGNORE; ++i )
		if ( ch->pcdata->ignore[i] && !str_prefix( str, ch->pcdata->ignore[i] ) )
			return i;

	return -1;
}

/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("Debes escribir el comando completo para poder borrarte.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Estado de delete removido.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    char * name = str_dup(ch->name);

    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N se convierte en ruido de linea.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
	    do_quit(ch,"");
	    unlink(strsave);
	    player_delete(name);
	    nuke_corpse(name, TRUE);
	    free_string(name);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Solo escribe delete. Sin argumento.\n\r",ch);
	return;
    }

    send_to_char("Escribe delete otra vez para confirmar el comando.\n\r",ch);
    send_to_char("ADVERTENCIA: este comando es irreversible.\n\r",ch);
    send_to_char("Escribe delete con un argumento para eliminar el status actual de delete.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N se pregunta si hacer delete o no.",ch,NULL,0,0,get_trust(ch));
}
	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    send_to_char("   canal       status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("chat           ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("remates        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("P/R            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("citas          ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUOTE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("felicitaciones ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_IMMORTAL(ch))
    {
      send_to_char("god channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }

    send_to_char("exclamaciones  ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("tells          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("Estas AFK.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("Eres inmune al snoop.\n\r",ch);
   
    if (!IS_NPC(ch) && (ch->pcdata->lines != PAGELEN))
    {
	if (ch->pcdata->lines)
	{
	    sprintf(buf,"Ves %d lineas de scroll.\n\r", ch->pcdata->lines + 2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Buffer de scroll esta off.\n\r",ch);
    }

    if (!IS_NPC(ch) && ch->pcdata->prompt != NULL)
    {
	sprintf(buf,"Tu prompt actual es: %s\n\r",ch->pcdata->prompt);
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("No puedes exclamar.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("No puedes hacer tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("No puedes usar canales.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("No puedes mostrar emociones.\n\r",ch);

    if (IS_SET(ch->comm,COMM_OLC_PAUSA))
      send_to_char("Tienes pausa en los editores OLC.\n\r", ch );

    if (IS_SET(ch->comm,COMM_OLCX))
    	send_to_char("Tienes activadas las extensiones VT100 para OLC.\n\r", ch );
}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("Puedes escuchar tells de nuevo.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("Desde ahora, no escucharas tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Modo Quiet desactivado.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("Desde ahora, solo oiras says y emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_AFK))
    {
	send_to_char("Modo AFK removido.", ch );
	if ( !IS_NPC(ch) && !IS_NULLSTR(buf_string(ch->pcdata->buffer)) )
		send_to_char( "Escribe 'replay' para ver los tells.",ch);
	send_to_char( "\n\r", ch );
	REMOVE_BIT(ch->comm,COMM_AFK);
    }
    else
    {
     send_to_char("Estas ahora en el modo AFK.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
    }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("No tienes replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("No te han hecho tells.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* RT chat replaced with ROM gossip */
void do_gossip( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
        send_to_char("Canal de Chat ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
        send_to_char("Canal de Chat ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else  /* gossip message sent, turn gossip on if it isn't already */
    {
	if (!can_talk(ch))
		return;

      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu charlas '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOGOSSIP) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          act_new( "$n charla '$t'#n", 
		   ch,strToEnt(argument,ch->in_room), chToEnt(d->character), TO_VICT,POS_SLEEPING );
        }
      }
    }
}

void do_grats( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGRATS))
      {
        send_to_char("Canal de Felicitaciones ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOGRATS);
      }
      else
      {
        send_to_char("Canal de Felicitaciones ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOGRATS);
      }
    }
    else  /* grats message sent, turn grats on if it isn't already */
    {
	if (!can_talk(ch))
		return;
	
      REMOVE_BIT(ch->comm,COMM_NOGRATS);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu felicitas '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOGRATS) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          act_new( "$n felicita '$t'#n",
                   ch,strToEnt(argument,ch->in_room), chToEnt(d->character), TO_VICT,POS_SLEEPING );
        }
      }
    }
}

void do_quote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUOTE))
      {
        send_to_char("Canal de Citas ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUOTE);
      }
      else
      {
        send_to_char("Canal de Citas ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUOTE);
      }
    }
    else  /* quote message sent, turn quote on if it isn't already */
    {
	if (!can_talk(ch))
		return;

      REMOVE_BIT(ch->comm,COMM_NOQUOTE);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu citas '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUOTE) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          act_new( "$n cita '$t'#n",
                   ch,strToEnt(argument,ch->in_room), chToEnt(d->character), TO_VICT,POS_SLEEPING );
        }
      }
    }
}

/* RT question channel */
void do_question( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
        send_to_char("Canal de P/R ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
        send_to_char("Canal de P/R ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* question sent, turn Q/A on if it isn't already */
    {
	if (!can_talk(ch))
		return;

        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu preguntas '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUESTION) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	  act_new("$n pregunta '$t'#n",
	 	  ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT,POS_SLEEPING);
        }
      }
    }
}

/* RT answer channel - uses same line as questions */
void do_answer( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
        send_to_char("Canal de P/R ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
        send_to_char("Canal de P/R ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* answer sent, turn Q/A on if it isn't already */
    {
	if (!can_talk(ch))
		return;

        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu respondes '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUESTION) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	  act_new("$n responde '$t'#n",
		  ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT,POS_SLEEPING);
        }
      }
    }
}

/* RT music channel */
void do_music( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
        send_to_char("Canal de Musica esta ahora ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
        send_to_char("Canal de Musica esta ahora OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
	if (!can_talk(ch))
		return;

        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
 
      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu cantas : '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      sprintf( buf, "$n canta : '%s'#n", argument );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOMUSIC) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    act_new("$n canta : '$t'#n",
		    ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT,POS_SLEEPING);
        }
      }
    }
}

/* clan channels */
void do_clantalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( !is_clan(ch) || ES_INDEP(ch->clan) )
    {
	send_to_char("No estas en un clan.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCLAN))
      {
        send_to_char("Canal de Clan ahora esta ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOCLAN);
      }
      else
      {
        send_to_char("Canal de Clan ahora esta OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCLAN);
      }
      return;
    }

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

        REMOVE_BIT(ch->comm,COMM_NOCLAN);

      argument = makedrunk ( (char *) argument , ch);
      sprintf( buf, "Tu dices al clan '%s'#n\n\r", argument );
      send_to_char( buf, ch );
      sprintf( buf, "$n dice al clan '%s'#n", argument );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
	     is_same_clan(ch,d->character) &&
             !IS_SET(d->character->comm,COMM_NOCLAN) &&
	     !IS_SET(d->character->comm,COMM_QUIET) )
        {
            act_new("$n dice al clan '$t'#n",ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT,POS_DEAD);
        }
    }

    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Canal Immortal ahora esta ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Canal Immortal ahora esta OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

    sprintf( buf, "$n: %s#n", argument );
    act_new("$n: $t#n",ch,strToEnt(argument,ch->in_room),NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     IS_IMMORTAL(d->character) && 
             !IS_SET(d->character->comm,COMM_NOWIZ) )
	{
	    act_new("$n: $t#n",ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT,POS_DEAD);
	}
    }

    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *mob, *mob_next;
	int oneshot = 0;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Que quieres decir?\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch) )
		do_language( ch, argument, ch->pcdata->speaking );
	else
		do_language( ch, argument, race_table[ch->race].race_lang );

	if ( HAS_ROOM_TRIGGER(ch->in_room, RTRIG_SPEECH) )
		mp_act_trigger( argument, roomToEnt(ch->in_room), chToEnt(ch), NULL, NULL, RTRIG_SPEECH );

	if ( !char_died(ch) )
		for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
 		{
			mob_next = mob->next_in_room;
			if ( IS_NPC(mob)
			&&   mob != ch
			&&  (oneshot == 0 || oneshot != mob->pIndexData->vnum)
			&&   mob->position == mob->pIndexData->default_pos )
			{
				if ( HAS_TRIGGER( mob, TRIG_SPEECH ) )
				{
					if (mp_act_trigger( argument, chToEnt(mob), chToEnt(ch), NULL, NULL, TRIG_SPEECH )
					&&  HAS_TRIGGER(mob,TRIG_ONESHOT))
						oneshot = mob->pIndexData->vnum;
				}
				else
				if ( HAS_TRIGGER( mob, TRIG_NSPEECH ) )
					mp_act_trigger( argument, chToEnt(mob), chToEnt(ch), NULL, NULL, TRIG_NSPEECH );
			}
		}

	return;
}

void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("Puedes escuchar exclamaciones otra vez.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("No escucharas mas exclamaciones.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "No puedes exclamar.\n\r", ch );
        return;
    }
 
	if ( IS_AFFECTED2( ch, AFF_MUTE )
	     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
        	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
	        return;
	}

    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );

    argument = makedrunk ( (char *) argument , ch);
    act( "Tu exclamas '$T'", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    act("$n exclama '$t'#n", ch, strToEnt(argument,ch->in_room), chToEnt(d->character), TO_VICT);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Tu mensaje no salio.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "Debes apagar el modo quiet primero.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("Debes apagar el modo deaf primero.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
	send_to_char( "Decirle que a quien?\n\r", ch );
	return;
    }

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Que quieres decirle?\n\r", ch );
    	return;
    }

    if ( IS_AFFECTED2( ch, AFF_MUTE )
     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
       	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
        return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N parece que se cayo ...intentalo mas tarde.",
	    ch, NULL, chToEnt(victim),TO_CHAR);
        sprintf(buf,"%s te dice '%s'#n\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && getNivelPr(ch) > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E no puede oirte.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E no esta recibiendo tells.", ch, NULL, chToEnt(victim), TO_CHAR );
  	return;
    }

    if ( !IS_NPC(victim) && (esta_ignorando(victim,ch->name) != -1) )
    {
    	act( "$E te esta ignorando.", ch, NULL, chToEnt(victim), TO_CHAR );
    	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E esta AFK, y no recibe tells.",ch,NULL,chToEnt(victim),TO_CHAR);
	    return;
	}

	act("$E esta AFK, pero tu tell llegara cuando $E vuelva.",
	    ch,NULL,chToEnt(victim),TO_CHAR);
	sprintf(buf,"%s te dice '%s'#n\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    act( "Le dices a $N '$t'#n", ch, strToEnt(argument,ch->in_room), chToEnt(victim), TO_CHAR );
    act_new("$n te dice '$t'#n",ch, strToEnt(argument,ch->in_room),chToEnt(victim),TO_VICT,POS_DEAD);
    victim->reply	= ch;

     if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
 	mp_act_trigger( argument, chToEnt(victim), chToEnt(ch), NULL, NULL, TRIG_SPEECH );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Tu mensaje no salio.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
        	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
	        return;
	}

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N parece que se cayo ...intentalo mas tarde.",
            ch,NULL,chToEnt(victim),TO_CHAR);
        sprintf(buf,"%s te dice '%s'#n\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E no puede oirte.", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
        act_new( "$E no esta recibiendo tells.", ch, NULL, chToEnt(victim), TO_CHAR,POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "En tus suenos, o que?\n\r", ch );
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E esta AFK, y no recibe tells.",
		ch,NULL,chToEnt(victim),TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E esta AFK, pero tu tell llegara cuando $E vuelva.",
            ch,NULL,chToEnt(victim),TO_CHAR,POS_DEAD);
        sprintf(buf,"%s te dice '%s'#n\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    act_new("Le dices a $N '$t'#n",ch,strToEnt(argument,ch->in_room),chToEnt(victim),TO_CHAR,POS_DEAD);
    act_new("$n te dice '$t'#n",ch,strToEnt(argument,ch->in_room),chToEnt(victim),TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "No puedes gritar.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Gritar que?\n\r", ch );
	return;
    }

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
        	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
	        return;
	}

    argument = makedrunk ( (char *) argument , ch);
    act("Tu gritas '$t'#n",ch,strToEnt(argument,ch->in_room),NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("$n grita '$t'#n",ch,strToEnt(argument,ch->in_room),chToEnt(d->character),TO_VICT);
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
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
 
    argument = makedrunk ( (char *) argument , ch);

    MOBtrigger = FALSE;
    act( "$n $T", ch, NULL, strToEnt(argument,ch->in_room), TO_ROOM );
    act( "$n $T", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
    MOBtrigger = TRUE;

    return;
}

void do_pmote( CHAR_DATA *ch, char *argument )
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
 
    argument = makedrunk ( (char *) argument , ch);
    act( "$n $t#n", ch, strToEnt(argument,ch->in_room), NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
            MOBtrigger = FALSE;
	    act("$N $t#n",vch,strToEnt(argument,ch->in_room),chToEnt(ch),TO_CHAR);
            MOBtrigger = TRUE;
	    continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
   	last[0] = '\0';
 	name = vch->name;
	
	for (; *letter != '\0'; letter++)
	{ 
	    if (*letter == '\'' && matches == (signed) strlen(vch->name))
	    {
		strcat(temp,"r");
		continue;
	    }

	    if (*letter == 's' && matches == (signed) strlen(vch->name))
	    {
		matches = 0;
		continue;
	    }
	    
 	    if (matches == (signed) strlen(vch->name))
	    {
		matches = 0;
	    }

	    if (*letter == *name)
	    {
		matches++;
		name++;
		if (matches == (signed) strlen(vch->name))
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
        MOBtrigger = FALSE;
	act("$N $t#n",vch,strToEnt(temp,ch->in_room),chToEnt(ch),TO_CHAR);
        MOBtrigger = TRUE;
    }
	
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "Rebosas de energia.",
	    "$n rebosa de energia.",
	    "Te sientes muy pur$o.",
	    "$n se siente muy pur$o.",
	    "Haces un pequeno truco con cartas.",
	    "$n hace un pequeno truco con cartas .",
	    "Muestras tus sobresalientes musculos.",
	    "$n muestra sus sobresalientes musculos.",
            "Stop it with the Ouija board, will ya?",
            "Great, $n is playing with $s Ouija board again."
	}
    },

    {
	{
	    "Te conviertes en una mariposa, y despues vuelves a tu forma normal.",
	    "$n se convierte en una mariposa, y despues vuelve a su forma normal.",
	    "Conviertes vino en agua. Asombroso.",
	    "$n convierte vino en agua. Asombroso.",
	    "Mueves tus orejas alternativamente.",
	    "$n mueve sus orejas alternativamente.",
	    "Rompes una nuez con tus dedos.",
	    "$n rompe una nuez con sus dedos.",
            "You read everyone's mind....and shudder with disgust.",
            "$n reads your mind...eww, you pervert!"
	}
    },

    {
	{
	    "Chispas azules salen desde tus dedos.",
	    "Chispas azules salen desde los dedos de $n.",
	    "Una aureola aparece sobre tu cabeza.",
	    "Una aureola aparece sobre la cabeza de $n.",
	    "Agilmente te tuerces en forma de nudo.",
	    "$n agilmente se tuerce en forma de nudo.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
            "You show everyone your awards for perfect school attendance",
            "You aren't impressed by $n's school attendance awards.  Geek."
	}
    },

    {
	{
	    "Pequenas luces rojas bailan en tus ojos.",
	    "Pequenas luces rojas bailan en los ojos de $n.",
	    "Recitas palabras de sabiduria.",
	    "$n recita palabras de sabiduria.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "Golpeas tu cabeza, y tus ojos dan vueltas.",
	    "$n golpea su cabeza, y sus ojos dan vueltas.",
            "A will-o-the-wisp arrives with your slippers.",
            "A will-o-the-wisp arrives with $n's slippers."
	}
    },

    {
	{
	    "Un monstruo verde y baboso aparece delante tuyo y se inclina ante ti.",
	    "Un monstruo verde y baboso aparece delante de $n y se inclina ante $e.",
	    "Meditando profundamente, empiezas a levitar.",
	    "Meditando profundamente, $n empieza a levitar.",
	    "Robas la ropa interior de cada persona del cuarto.",
	    "Tu ropa interior no esta!  $n la robo!",
	    "Crunch, crunch -- mascas una botella.",
	    "Crunch, crunch -- $n masca una botella.",
            "What's with the extra leg?",
            "Why did $n sprout an extra leg just now?"
	}
    },

    {
	{
	    "Conviertes a todo en diminutos elefantes rosados.",
	    "$n te convirtio en un diminuto elefante rosado.",
	    "Un angel te consulta.",
	    "Un angel consulta a $n.",
	    "Tiras los dados ... y ganas otra vez.",
	    "$n tira los dados ... y gana otra vez.",
	    "... 98, 99, 100 ... haces flexiones.",
	    "... 98, 99, 100 ... $n hace flexiones.",
            "The spoons flee as you begin to concentrate.",
            "The spoons flee as $n begins to concentrate."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique.",
            "Stop wiggling your brain at people.",
            "Make $n stop wiggling $s brain at you!"
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
            "MENSA called...they want your opinion on something.",
            "MENSA just called $n for consultation."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
            "Chairs fly around the room at your slightest whim.",
            "Chairs fly around the room at $n's slightest whim."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
            "Oof...maybe you shouldn't summon any more hippopotamuses.",
            "Oof!  Guess $n won't be summoning any more hippos for a while."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
            "Oops...your hair is sizzling from thinking too hard.",
            "Oops...$n's hair is sizzling from thinking too hard."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
            "What?  You were too busy concentrating.",
            "What?  Oh, $n was lost in thought...again."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
            "Will you get down here before you get hurt?",
            "Quick, get a stick, $n is doing $s pinata impression again."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
            "Careful...don't want to disintegrate anyone!",
            "LOOK OUT!  $n is trying to disintegrate something!"
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
            "You run off at the mouth about 'mind over matter'.",
            "Yeah, yeah, mind over matter.  Shut up, $n."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
            "Thud.",
            "Thud."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god Mota gives you a staff.",
	    "The great god Mota gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
            "You charm the pants off everyone...and refuse to give them back.",
            "Your pants are charmed off by $n, and $e won't give them back."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( getNivelPr(ch), sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*getClasePr(ch)+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*getClasePr(ch)+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "No hay rent aqui.  Solo save y quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Si quieres irte, debes escribirlo bien.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;
/*  extern char * help_bye; */

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "De ninguna manera! Estas peleando.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "No estas MUERTO todavia.\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
        return;
    }

    if ( ch->fdata != NULL
    &&   difftime(current_time, ch->fdata->when) < MINUTOS(1)
    &&   ch->pcdata->confirm_delete == FALSE )
    {
	act( "Todavia estas demasiado excitad$o por la pelea que acaba de ocurrir!\n\r", ch, NULL, NULL, TO_CHAR );
    	return;
    }

    if ( IS_SET(ch->comm, COMM_OLCX) )
	do_clear( ch, "reset" );

    do_quote_exit(ch);
    act( "$n ha dejado el juego.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s se retira.", ch->name );
    log_string( log_buf );
    wiznet("$N vuelve al mundo real.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    if ( !IS_SET(ch->act, PLR_NOSAVE) && ch->pcdata->corpse )
    {
	save_corpse(ch, FALSE);
	extract_obj(ch->pcdata->corpse, FALSE); // guardamos los limites!
    }

    save_char_obj( ch );

    id = ch->id;
    d = ch->desc;

    if (ch->in_room != NULL
    && !is_in_room(ch, ch->in_room) )
    	char_to_room( ch, get_room_index(ROOM_VNUM_LIMBO) );

    extract_char( ch, TRUE );

    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

    return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    send_to_char("Grabando. Recuerda que ROM graba automaticamente.\n\r", ch);
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Seguir a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "Pero tu prefieres seguir a $N!", ch, NULL, chToEnt(ch->master), TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    act("Ya te sigues a ti mism$o.", ch, NULL, NULL, TO_CHAR );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N parece no querer seguidores.",
             ch,NULL,chToEnt(victim), TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n te sigue ahora.", ch, NULL, chToEnt(master), TO_VICT );

    act( "Ahora sigues a $N.",  ch, NULL, chToEnt(master), TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch )
    &&   ch->in_room != NULL
    &&   ch->master->in_room != NULL
    &&   is_in_room(ch, ch->in_room)
    &&   is_in_room(ch->master, ch->master->in_room) )
    {
	act( "$n deja de seguirte.",	ch, NULL, chToEnt(ch->master), TO_VICT );
    	act( "Dejas de seguir a $N.",	ch, NULL, chToEnt(ch->master), TO_CHAR );
    }

    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N se desvanece lentamente.",ch,NULL,chToEnt(pet),TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch, bool pets )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( !pets && IS_NPC(fch) && IS_PET(fch) && fch->master == ch )
		continue;
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}

bool order_mob = FALSE;

void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))
    {
        send_to_char("Eso NO sera hecho.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Ordenar a quien hacer que?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "Tienes ganas de tomar, no de dar, ordenes.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "OK OK, de inmediato!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Hazlo tu mismo!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n te ordena '%s'#n.", argument );
	    act( buf, ch, NULL, chToEnt(och), TO_VICT );
	    if (IS_NPC(och))
	    	order_mob = TRUE;
	    interpret( och, argument );
	    order_mob = FALSE;
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "No tienes seguidores aqui.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "Grupo de %s:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
		    getNivelPr(gch),
		    IS_NPC(gch) ? "Mob" : class_table[getClasePr(gch)].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "Pero tu estas siguiendo a otro!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N no te esta siguiendo.",ch,NULL,chToEnt(victim),TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("No puedes sacar a mobs afectados por CHARM de tu grupo.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act_new("Te gusta demasiado seguir a tu maestro $m para dejarlo!",
	    ch,NULL,chToEnt(victim),TO_VICT,POS_SLEEPING);
    	return;
    }

    if (is_clan(ch) && is_clan(victim) && !is_same_clan(ch, victim))
    {
    	send_to_char("El lider de tu clan se enojaria al saber que quieres agruparte con esa basura!\n\r", ch );
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act_new("$n saca a $N de su grupo.",
	    ch,NULL,chToEnt(victim),TO_NOTVICT,POS_RESTING);
	act_new("$n te saca de su grupo.",
	    ch,NULL,chToEnt(victim),TO_VICT,POS_SLEEPING);
	act_new("Sacas a $N de tu grupo.",
	    ch,NULL,chToEnt(victim),TO_CHAR,POS_SLEEPING);
	return;
    }

    victim->leader = ch;
    act_new("$N se une al grupo de $n.",ch,NULL,chToEnt(victim),TO_NOTVICT,POS_RESTING);
    act_new("Te unes al grupo de $n.",ch,NULL,chToEnt(victim),TO_VICT,POS_SLEEPING);
    act_new("$N se une a tu grupo.",ch,NULL,chToEnt(victim),TO_CHAR,POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Repartir cuanto?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if ( amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "A tu grupo no le gustaria eso.\n\r", ch );
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "Les das cero monedas, pero nadie se da cuenta.\n\r", ch );
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	send_to_char( "No tienes tantas monedas para repartir.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Mejor quedate con todo.\n\r", ch );
	return;
    }
	    
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "No molestes, amarrete.\n\r", ch );
	return;
    }

    ch->silver	-= amount_silver;
    ch->silver	+= share_silver + extra_silver;
    ch->gold 	-= amount_gold;
    ch->gold 	+= share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "Repartes %d monedas de plata. Tu parte son %d monedas.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "Repartes %d monedas de oro. Tu parte son %d monedas.\n\r",
	     amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$n reparte %d monedas de plata. Tu parte son %d monedas.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n reparte %d monedas de oro. Tu parte son %d monedas.",
		amount_gold,share_gold);
    }
    else
    {
	sprintf(buf,
"$n reparte %d monedas de plata y %d de oro, tu parte son %d de plata y %d de oro.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, chToEnt(gch), TO_VICT );
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
    if ( argument[0] == '\0' )
    {
	send_to_char( "Que quieres decirle a tu grupo?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Tu mensaje no salio!\n\r", ch );
	return;
    }

    sprintf( buf, "Dices al grupo '%s'#n\n\r", argument );
    send_to_char( buf, ch );

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    act_new("$n dice al grupo '$t'#n",
		ch,strToEnt(argument,ch->in_room),chToEnt(gch),TO_VICT,POS_SLEEPING);
    }

    return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

char * makedrunk (char *string, CHAR_DATA * ch)
{
/* This structure defines all changes for a character */
  struct struckdrunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

#define LARGO (MAX_INPUT_LENGTH)
	char buf[LARGO*2];
	char temp;
	int pos = 0;
	int drunklevel;
	int randomnum;

	/* Check how drunk a person is... */
	if (IS_NPC(ch))
		drunklevel = 0;
	else
		drunklevel = ch->pcdata->condition[COND_DRUNK];

	if (drunklevel > 0)
	{
		do
		{
			temp = toupper (*string);
			if ((temp >= 'A') && (temp <= 'Z'))
			{
				if (drunklevel > drunk[temp - 'A'].min_drunk_level)
				{
					randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
					if ( (pos + strlen(drunk[temp - 'A'].replacement[randomnum])) < (LARGO - 2) )
					{
						strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
						pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
					}
				}
				else if ( (pos + 1) < (LARGO - 2) )
					buf[pos++] = *string;
			}
			else
			{
				if ((temp >= '0') && (temp <= '9') && ((pos + 1) < (LARGO - 2)) )
				{
					temp = '0' + number_range (0, 9);
					buf[pos++] = temp;
				}
				else if ( (pos + 1) < (LARGO - 2) )
					buf[pos++] = *string;
			}
		}
		while (*string++);

		buf[UMIN(pos,MAX_INPUT_LENGTH - 2)] = '\0';          /* Mark end of the string... */
		strcpy(string, buf);
		return(string);
	}

  return (string);
}

void do_beep ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    argument = one_argument( argument, arg );

    if  ( arg[0] == '\0' )
    {
        send_to_char( "Beep a quien?\n\r", ch );
        return;
    }

    if (!can_talk(ch))
	return;

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim))
    {
        send_to_char( "No es beepeable.\n\r", ch );
        return;
    }

    if ( esta_ignorando( victim, ch->name ) != -1 )
    {
    	act( "$N no quiere mensajes tuyos.", ch, NULL, chToEnt(victim), TO_CHAR );
    	return;
    }

    sprintf( buf, "\aBeepeas a %s.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "\a%s te beepeo.\n\r", ch->name );
    send_to_char( buf, victim );

    return;
}

bool	can_talk	(CHAR_DATA *ch)
{
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("Debes apagar el modo quiet primero.\n\r",ch);
	  return FALSE;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("Los dioses han eliminado tus privilegios de uso de canales.\n\r",ch);
	  return FALSE;
        }

	if ( IS_AFFECTED2( ch, AFF_MUTE )
	     || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
        	send_to_char( "Parece que no puedes romper el silencio.\n\r", ch );
	        return FALSE;
	}

	return TRUE;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"#BREMATE#b: %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->comm,COMM_NOAUCTION))
            act (buf, original, NULL, NULL, TO_CHAR);
    }
}

void do_clear(CHAR_DATA *ch, char *argument)
{
	if ( !str_cmp( argument, "reset" ) )
		send_to_char(term_table[ch->desc->term].reset, ch);
	send_to_char(term_table[ch->desc->term].clear, ch);

	if (ch->desc && ch->desc->screenmap)
		InitScreenMap(ch->desc);

	return;
}

void do_mensaje(CHAR_DATA *ch, char*argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		sprintf( buf, "Tu mensaje actual es: %s\n\r", ch->pcdata->mensaje );
		send_to_char( buf, ch );
		return;
	}

	if ( strlen(argument) > 70 )
		argument[70] = '\0';

	free_string( ch->pcdata->mensaje );
	if ( !str_cmp( argument, "ninguno" ) )
		ch->pcdata->mensaje = str_dup( "" );
	else
		ch->pcdata->mensaje = str_dup(argument);

	send_to_char( "Ok.\n\r", ch );
	return;
}

void mostrar_mensaje( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	const char *str;
	bool mob = FALSE;
	
	if (IS_NPC(ch))
		return;

	str = ch->pcdata->mensaje;

	sprintf( buf, "%s ", SELFPERS(ch) );

	while ( *str )
	{
		if ( *str == '*' && mob == FALSE )
		{
			strcat(buf, PERS(victim, ch) );
			mob = TRUE;
		}
		else
		{
			sprintf( buf2, "%c", *str );
			strcat( buf, buf2 );
		}
		++str;
	}
	 
	strcat( buf, " #n" );

	act( buf, ch, NULL, NULL, TO_ALL );

	return;
}
		
/*
 * Structure for a Quote
 */

struct quote_type
{
    char *      text;
    char *      by;
};

/*
 * The Quotes - insert yours in, and increase MAX_QUOTES in merc.h
 */
#define MAX_QUOTES 30

const struct quote_type quote_table [MAX_QUOTES] =
{
    { "Todo lo bueno debe acabarse alguna vez.",		NULL		},
    { "Cogito Ergo Sum.",					"Descartes"	},
    { "Acabe con el hambre y la pobreza. Comase un pobre.",	NULL		},
    { "640kb should be enough for everyone!",			"Bill Gates"	},
    { "Muchas cosas se juzgan imposibles de hacer...hasta que estan hechas.", "Plinio"	},
    { "Nunca seas el primero en ser el segundo.",		NULL		},
    { "Soy el mejor en lo que hago, pero lo que hago no es muy grato", "Wolverine" },
    { "It's always funny...until someone gets hurt",	"Ricochet - Faith No More" },
    { "I have never understood the female capacity to avoid a direct answer to\n\r"
      "any question.", "Spock, \"This Side of Paradise\", stardate 3417.3" },
    { "Extreme feminine beauty is always disturbing.", "Spock, \"The Cloud Minders\", stardate 5818.4" },
    { "-Sr. Pinochet, que opina Ud. de que se hayan encontrado tumbas con varias personas dentro?\n\r"
      "-Bueno, que economia mas grande!!!", NULL },
    { "Haz lo necesario, luego lo posible, entonces te encontraras haciendo lo imposible.", NULL },
    { "No me importa ser ciego, lo que realmente me molestaria es ser negro", "Stevie Wonder" },
    { "Para ser un buen guerrero...primero hazte invencible", "Sun Tzu" },
    { "Un buen guerrero nunca va a la pelea...la pelea viene hacia el", "Sun Tzu" },
    { "Joven...dile no a la droga, somos muchos y la droga es poca", NULL	},
    { "Mas vale una al a~no que cien en el ba~no",		NULL	},
    { "Si la vida te da la espalda, agarrale el culo!",		NULL	},
    { "-Cual es el colmo de un sanguche de potito?\n\r-Que se lo coma Paul Schaeffer.", NULL },
    { "El masoquista le dice al sadico : 'PEGAME!!!!'.\n\rY el sadico le responde : 'No'.", NULL },
    { "Mi esposa tiene el mejor fisico del mundo.",	"Albert Einstein" },
    { "Lo mejor que hace un curado no sirve para nada.",	NULL	},
    { "La mentira tiene patas cortas.",				NULL	},
    { "Callense, comunistas de mierda!!!",		"Evelyn Matthei" },
    { "Segmentation fault (core dumped)",			NULL	},
    { "Traicionaron a Cristo y no van a traicionar a mi padre.", "El hijo de Pinochet" },
    { "Nunca te cases con una mujer perfecta...te casarias con un hombre",
    							"Loki"		},
    { "Toda mi vida es un juego",				"Mario Bros." },
    { "Autoridad que no abusa pierde su prestigio.",		NULL	},
    { "La conciencia es lo que duele cuando todo se siente bien.", NULL }
};

/*
 * The Routine
 * Quote Code is by elfren@aros.net
 */
void do_quote_exit( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int number;

    number = number_range(0, 1000) % MAX_QUOTES;

    sprintf( buf, "\n\r%s\n\r", quote_table[number].text );
    send_to_char( buf, ch );
    
    if ( quote_table[number].by )
    {
    	sprintf( buf, " - #B%s#b\n\r", quote_table[number].by );
    	send_to_char( buf, ch );
    }

    return;
}

void do_pausa( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	char *mensaje = NULL;
	
	if ( IS_SET(ch->comm, COMM_OLC_PAUSA) )
		mensaje = "OFF"; /* al reves! */
	else
		mensaje = "ON";
	
	sprintf( buf, "Pausas en OLC estan ahora #B%s#b.\n\r", mensaje );
	send_to_char( buf, ch );
	TOGGLE_BIT(ch->comm, COMM_OLC_PAUSA);
	return;
}
	
void do_condicion( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	char *mensaje = NULL;

	if (IS_NULLSTR(argument))
	{
		sprintf(buf,"Condicion propia durante las peleas : #B%s#b.\n\r",
			IS_SET(ch->comm,COMM_STAT) ? "ON" : "OFF" );
		send_to_char( buf, ch );
		sprintf(buf,"Condicion del equipo al ver el eq   : #B%s#b.\n\r",
			IS_SET(ch->comm,COMM_CONDICION) ? "ON" : "OFF" );
		send_to_char(buf, ch );
		send_to_char("Para cambiar la configuracion, escribe '#Bcondicion propia#b' o '#Bcondicion eq#b'.\n\r", ch );
		return;
	}

	if ( !str_cmp(argument,"propia") )
	{
		if ( IS_SET(ch->comm, COMM_STAT) )
			mensaje = "OFF"; /* al reves! */
		else
			mensaje = "ON";
		sprintf( buf, "Condicion propia durante las peleas ahora esta #B%s#b.\n\r", mensaje );
		send_to_char( buf, ch );
		TOGGLE_BIT(ch->comm, COMM_STAT);
		return;
	}

	if ( !str_cmp(argument,"eq") )
	{
		if ( IS_SET(ch->comm, COMM_CONDICION) )
			mensaje = "OFF"; /* al reves! */
		else
			mensaje = "ON";
		sprintf( buf, "Condicion del equipo al ver el eq ahora esta #B%s#b.\n\r", mensaje );
		send_to_char( buf, ch );
		TOGGLE_BIT(ch->comm, COMM_CONDICION);
		return;
	}
	
	do_condicion(ch,"");
	return;
}

void do_ignorar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int i, pos;
	bool found = FALSE;
	char buf[MIL];
	
	if (IS_NPC(ch))
		return;

	if (IS_NULLSTR(argument))
	{
		for ( i = 0; i < MAX_IGNORE; ++i )
			if (!IS_NULLSTR(ch->pcdata->ignore[i]))
			{
				if (!found)
					send_to_char( "Estas ignorando a:\n\r", ch );
				sprintf( buf, "%2d. %s\n\r", i, ch->pcdata->ignore[i] );
				send_to_char( buf, ch );
				found = TRUE;
			}
		
		if (!found)
			send_to_char( "No estas ignorando a nadie.\n\r", ch );
		return;
	}

	if ( (pos = esta_ignorando(ch,argument)) != -1 )
	{
		free_string(ch->pcdata->ignore[pos]);
		ch->pcdata->ignore[pos] = str_dup( "" );
		act( "Dejas de ignorar a $T.", ch, NULL, strToEnt(argument,ch->in_room), TO_CHAR );
		return;
	}

	if ( (victim = get_char_world( ch, argument )) == NULL || IS_NPC(victim) )
	{
		send_to_char( "No esta jugando.\n\r", ch );
		return;
	}

	if ( IS_IMMORTAL(victim) )
	{
		send_to_char( "No es ignorable.\n\r", ch );
		return;
	}

	for ( i = 0; i < MAX_IGNORE; ++i )
		if (IS_NULLSTR(ch->pcdata->ignore[i]))
		{
			free_string(ch->pcdata->ignore[i]);
			ch->pcdata->ignore[i] = str_dup( victim->name );
			act( "Ahora ignoras a $N.", ch, NULL, chToEnt(victim), TO_CHAR );
			return;
		}

	send_to_char( "No puedes ignorar a mas personas.\n\r", ch );
	return;
}

void do_term( CHAR_DATA *ch, char * argument )
{
	int term = -1;

	if ( !ch->desc )
		return;

	if ( IS_NULLSTR(argument) )
	{
		printf_to_char( ch, "Tu tipo de terminal es : %s.\n\r",
			term_table[ch->desc->term].name );
		send_to_char( "Para ver la lista de terminales, escribe '#Bterm lista#b'.\n\r", ch );
		return;
	}

	if ( !str_cmp( argument, "lista" ) )
	{
		send_to_char( "Configuraciones disponibles:\n\r", ch );
		for ( term = 0; term_table[term].name; term++ )
			printf_to_char( ch, "#B%d#b. #B%s#b\n\r", term + 1,
				term_table[term].name );
		return;
	}

	term = term_lookup( argument );

	if ( term == -1 )
	{
		send_to_char( "Ese terminal no existe.\n\r", ch );
		return;
	}

	ch->desc->term = term;
	send_to_char( "Ok.\n\r", ch );
}

void do_color( CHAR_DATA *ch, char *argument )
{
	int color;

	if ( IS_NPC(ch) )
		return;

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : color [1..15]\n\r", ch );
		send_to_char( "El color estandar es el 7.\n\r", ch );
		send_to_char( "Colores arriba de 7 son los mismos de 1..7 pero con #Bnegrita#b.\n\r", ch );
		return;
	}

	color = atoi(argument);

	if ( color < 1 || color > 15 )
	{
		send_to_char( "Solo numeros entre 1 y 15.\n\r", ch );
		return;
	}

	ch->pcdata->color = color;
	send_to_char( "Ok.\n\r", ch );
}

DO_FUN_DEC(do_olcx)
{
	if ( IS_SET(ch->comm, COMM_OLCX) )
		send_to_char("Extensiones VT100 para OLC desactivadas.\n\r", ch );
	else
		send_to_char("Extensiones VT100 para OLC activadas.\n\r", ch );

	TOGGLE_BIT(ch->comm, COMM_OLCX);
}
