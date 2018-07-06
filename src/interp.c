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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#if !defined(WIN32)
#include <unistd.h> /* unlink() */
#endif
#include "recycle.h"

bool  check_disabled (const struct cmd_type *command);
bool  prohibido (CHAR_DATA *, char *);

DISABLED_DATA *disabled_first;
#define END_MARKER    "END" /* for load_disabled() and save_disabled() */

bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );

char last_command[MAX_STRING_LENGTH];

int	num_letra[26];

struct	command_type
{
	char *	type;
	sh_int	num_type; 
};

const	struct	command_type	cmd_type_table [] =
{
	{	"#BComunicacion#b",		TYP_CMM		},
	{	"#BCombate#b",			TYP_CBT		},
	{	"#BEspeciales#b",		TYP_ESP		},
	{	"#BGrupo#b",			TYP_GRP		},
	{	"#BObjetos#b",			TYP_OBJ		},
	{	"#BInformacion#b",		TYP_INF		},
	{	"#BOtros#b",			TYP_OTH		},
	{	"#BMovimiento#b",		TYP_MVT		},
	{	"#BConfiguracion#b",		TYP_CNF		},
	{	"#BLenguajes#b",		TYP_LNG		},
	{	"#BManejo de jugadores#b",	TYP_PLR		},
	{	"#BCreacion (OLC)#b",		TYP_OLC		},
	{	NULL,				0		}
};

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;
bool				forced_player	= FALSE;

extern	struct	cmd_type	*cmd_table;

void procesar_comando( DO_FUN * funcion, CHAR_DATA * ch, char * argument )
{
	char * blah = str_dup(argument);

	(*funcion) (ch, blah);

	free_string(argument);
}

void ordenar_tabla_com( void )
{
	int i, j, cnt, maxcnt = 0, cntl[27], ilet, calc;
	char letra;
	struct cmd_type *new_cmd_table;
	struct cmd_type **temptabla;

	for ( i = 0; i < 27; ++i )
		cntl[i] = 0;

	for ( i = 0; i < 26; i++ )
	{
		cnt = 0;

		for ( j = 0; j < MAX_CMD; ++j )
		{
			letra = LOWER(cmd_table[j].name[0]) - 'a';
			if ( i == (int) letra )
				cnt++;
		}

		if ( cnt > maxcnt )
			maxcnt = cnt;
	}

	temptabla = mud_calloc( (maxcnt*27), sizeof(struct cmd_type *) );

	for ( i = 0; i < (maxcnt*27); ++i )
		temptabla[i] = NULL;

	for ( i = 0; i < MAX_CMD; ++i )
	{
		if ( !isalpha(cmd_table[i].name[0]) )
			temptabla[cntl[0]++] = &cmd_table[i];
		else
		{
			letra = LOWER(cmd_table[i].name[0]);
			ilet = (int) letra;
			ilet -= 'a';
			ilet++;
			cntl[ilet]++;
			calc = (maxcnt * ilet) + cntl[ilet];
			temptabla[calc] = &cmd_table[i];
		}
	}

	new_cmd_table = mud_malloc (sizeof(struct cmd_type) * (MAX_CMD + 1));

	i = cnt = 0;
	while ( i < (maxcnt*27) )
	{
		if ( temptabla[i] )
			new_cmd_table[cnt++] = *temptabla[i];
		i++;
	}

	new_cmd_table[MAX_CMD].name = str_dup( "" );

	free(temptabla);
	free(cmd_table);
	cmd_table = new_cmd_table;
}

void crear_tabla_com( void )
{
	int cmd;
	char letra;

	/* Ordenar tabla e inicializar vector */
	ordenar_tabla_com();
	for ( cmd = 0; cmd < 26; cmd++ )
		num_letra[cmd] = -1;

	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; ++cmd )
	{
		letra = LOWER(cmd_table[cmd].name[0]);
		if ( !isalpha(letra) )
			continue;
		letra -= 'a'; /* rango 0...26-1 */
		if ( num_letra[(int) letra] == -1 )
			num_letra[(int) letra] = cmd;
	}
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    int primcmd, ultcmd = -1;
    bool found;
    char letra;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    if ( ch == NULL
    ||   char_died(ch)
    ||   ch->in_room == NULL )
	return;

    /*
     * No hiding.
     */
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) && !forced_player )
    {
	send_to_char( "Estas totalmente congelado!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );

    if ( isalpha(command[0]) )
    {
	letra = LOWER(command[0]) - 'a'; /* 0...26-1 */
    	primcmd = num_letra[(int) letra];
	if (primcmd != -1)
		while ( ultcmd == -1 && letra <= 25 )
		{
			letra++;
			ultcmd  = (letra >= 25 ? MAX_CMD : num_letra[(int) letra]);
		}
    }
    else
    {
    	primcmd = 0;
    	ultcmd  = num_letra[0];
    }

    if ( HAS_ROOM_TRIGGER( ch->in_room, RTRIG_COMM )
      || HAS_ROOM_TRIGGER( ch->in_room, RTRIG_EXCOMM ) )
    {
	MPROG_LIST *prg;

	if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
	{
		write_to_buffer( ch->desc->snoop_by, "% ",    2 );
		write_to_buffer( ch->desc->snoop_by, logline, 0 );
		write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
	}

	for ( prg = ch->in_room->mprogs; prg != NULL; prg = prg->next )
	{
		if ( ((prg->trig_type == RTRIG_COMM)
		||    (prg->trig_type == RTRIG_EXCOMM))
		&&  strstr( command, prg->trig_phrase ) != NULL )
		{
			program_flow( prg->vnum, prg->code, roomToEnt(ch->in_room), chToEnt(ch), NULL, NULL );
			return;
		}
	}
    }

    if ( primcmd == -1 )
    {
	found = FALSE;
	cmd = MAX_CMD;
    }
    else
	for ( cmd = primcmd; cmd < ultcmd; cmd++ )
	{
		if ( command[0] == cmd_table[cmd].name[0]
		&&   !str_prefix( command, cmd_table[cmd].name )
		&&   cmd_table[cmd].level <= trust )
		{
		    found = TRUE;
		    break;
		}
	}

    /*
     * Log and snoop.
     */
    if ( found && cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   ( found && cmd_table[cmd].log == LOG_ALWAYS ) ) && str_cmp( logline, "" ) )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_PLOG,0,get_trust(ch));
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
		log_char( ch, logline );
	else
		log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }
    else /* a normal valid command.. check if it is disabled */
    {
      	if (check_disabled (&cmd_table[cmd]))
      	{
              send_to_char ("This command has been temporarily disabled.\n\r", ch);
              return;
      	}
    	if ( prohibido(ch, cmd_table[cmd].name) )
    	{
    		send_to_char( "Huh?\n\r", ch );
    		return;
    	}
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Quedate quieto; estas #BMUERTO#b.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "Estas demasiado herido como para hacer eso.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "Estas demasiado aturdido como para hacer eso.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "En tus suenos, o que?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... Te sientes muy relajado...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Mejor levantate primero.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "De ninguna manera!  Estas peleando!\n\r", ch);
	    break;

	}
	return;
    }

    /* Record the command */

    sprintf (last_command, "[%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    /* Record that the command was the last done, but it is finished */
    sprintf (last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);

    tail_chain( );
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "Eres un anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Quedate quieto; estas #BMUERTO#b.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "Estas demasiado herido para poder hacer eso.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "Estas demasiado aturdido para poder hacer eso.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "En tus suenos, o que?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;

    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, chToEnt(victim), TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, chToEnt(victim), TO_CHAR    );
    }
    else if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta jugando.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, chToEnt(victim), TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, chToEnt(victim), TO_CHAR    );
    }
    else
    {
	if ( ch->in_room == victim->in_room )
	{
		act( social_table[cmd].others_found,  ch, NULL, chToEnt(victim), TO_NOTVICT );
		act( social_table[cmd].char_found,    ch, NULL, chToEnt(victim), TO_CHAR    );
		act( social_table[cmd].vict_found,    ch, NULL, chToEnt(victim), TO_VICT    );
	}
	else
	{
		ROOM_INDEX_DATA * room = ch->in_room;
		char buf[MSL];

		if ( !IS_NULLSTR(social_table[cmd].others_found) )
		{
			sprintf( buf, "Desde lejos, %s", social_table[cmd].others_found );
			buf[13] = tolower(buf[13]);
			act( buf, ch, NULL, chToEnt(victim), TO_NOTVICT );
			ch->in_room = victim->in_room;
			sprintf( buf, "Desde lejos, %s", social_table[cmd].others_found );
			buf[13] = tolower(buf[13]);
			act( buf, ch, NULL, chToEnt(victim), TO_NOTVICT );
			ch->in_room = room;
		}

		if ( !IS_NULLSTR(social_table[cmd].char_found) )
		{
			sprintf( buf, "Desde lejos, %s", social_table[cmd].char_found );
			buf[13] = tolower(buf[13]);
			act( buf, ch, NULL, chToEnt(victim), TO_CHAR );
		}

		if ( !IS_NULLSTR(social_table[cmd].vict_found) )
		{
			sprintf( buf, "Desde lejos, %s", social_table[cmd].vict_found );
			buf[13] = tolower(buf[13]);
			act( buf, ch, NULL, chToEnt(victim), TO_VICT );
		}
	}

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&    ch->in_room == victim->in_room
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, chToEnt(ch), TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, chToEnt(ch), TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, chToEnt(ch), TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n le da una cachetada a $N.",  victim, NULL, chToEnt(ch), TO_NOTVICT );
		act( "Le das una cachetada a $N.",  victim, NULL, chToEnt(ch), TO_CHAR    );
		act( "$n te da una cachetada.", victim, NULL, chToEnt(ch), TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
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
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
        &&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/

void do_disable (CHAR_DATA *ch, char *argument)
{
        int i;
        DISABLED_DATA *p,*q;
        char buf[100];
        
        if (IS_NPC(ch))
        {
                send_to_char ("RETURN first.\n\r",ch);
                return;
        }
        
        if (!argument[0]) /* Nothing specified. Show disabled commands. */
        {
                if (!disabled_first) /* Any disabled at all ? */
                {
                        send_to_char ("There are no commands disabled.\n\r",ch);
                        return;
                }

                send_to_char ("Disabled commands:\n\r"
                              "Command      Level   Disabled by\n\r",ch);
                
                for (p = disabled_first; p; p = p->next)
                {
                        sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
                        send_to_char (buf,ch);
                }
                return;
        }
        
        /* command given */

        /* First check if it is one of the disabled commands */
        for (p = disabled_first; p ; p = p->next)
                if (!str_cmp(argument, p->command->name))
                        break;
                        
        if (p) /* this command is disabled */
        {
        /* Optional: The level of the imm to enable the command must equal or exceed level
           of the one that disabled it */
        
                if (get_trust(ch) < p->level)
                {
                        send_to_char ("This command was disabled by a higher power.\n\r",ch);
                        return;
                }
                
                /* Remove */
                
                if (disabled_first == p) /* node to be removed == head ? */
                        disabled_first = p->next;
                else /* Find the node before this one */
                {
                        for (q = disabled_first; q->next != p; q = q->next); /* empty for */
                        q->next = p->next;
                }
                
                free_string (p->disabled_by); /* free name of disabler */
                free_mem (p,sizeof(DISABLED_DATA)); /* free node */
                save_disabled(); /* save to disk */
                send_to_char ("Command enabled.\n\r",ch);
        }
        else /* not a disabled command, check if that command exists */
        {
                /* IQ test */
                if (!str_cmp(argument,"disable"))
                {
                        send_to_char ("You cannot disable the disable command.\n\r",ch);
                        return;
                }

                /* Search for the command */
                for (i = 0; cmd_table[i].name[0] != '\0'; i++)
                        if (!str_cmp(cmd_table[i].name, argument))
                                break;

                /* Found? */
                if (cmd_table[i].name[0] == '\0')
                {
                        send_to_char ("No such command.\n\r",ch);
                        return;
                }

                /* Can the imm use this command at all ? */
        
                if (cmd_table[i].level > get_trust(ch))
                {
                        send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
                        return;
                }
                
                /* Disable the command */
                
                p = alloc_mem (sizeof(DISABLED_DATA));

                p->command = &cmd_table[i];
                p->disabled_by = str_dup (ch->name); /* save name of disabler */
                p->level = get_trust(ch); /* save trust */
                p->next = disabled_first;
                disabled_first = p; /* add before the current first element */
                
                send_to_char ("Command disabled.\n\r",ch);
                save_disabled(); /* save to disk */
        }
}

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled (const struct cmd_type *command)
{
        DISABLED_DATA *p;
        
        for (p = disabled_first; p ; p = p->next)
                if (p->command->do_fun == command->do_fun)
                        return TRUE;

        return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
        FILE *fp;
        DISABLED_DATA *p;
        char *name;
        int i;
        
        disabled_first = NULL;
        
        fp = fopen (DISABLED_FILE, "r");
        
        if (!fp) /* No disabled file.. no disabled commands : */
                return;
                
        name = fread_word (fp);
        
        while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER:) */
        {
                /* Find the command in the table */
                for (i = 0; cmd_table[i].name[0] ; i++)
                        if (!str_cmp(cmd_table[i].name, name))
                                break;
                                
                if (!cmd_table[i].name[0]) /* command does not exist? */
                {
                        bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
                        fread_number(fp); /* level */
                        fread_word(fp); /* disabled_by */
                }
                else /* add new disabled command */
                {
                        p = alloc_mem(sizeof(DISABLED_DATA));
                        p->command = &cmd_table[i];
                        p->level = fread_number(fp);
                        p->disabled_by = str_dup(fread_word(fp));
                        p->next = disabled_first;
                        
                        disabled_first = p;

                }
                
                name = fread_word(fp);
        }

        fclose (fp);
}

/* Save disabled commands */
void save_disabled()
{
        FILE *fp;
        DISABLED_DATA *p;
        
        if (!disabled_first) /* delete file if no commands are disabled */
        {
                unlink (DISABLED_FILE);
                return;
        }
        
        fp = fopen (DISABLED_FILE, "w");
        
        if (!fp)
        {
                bug ("Could not open " DISABLED_FILE " for writing",0);
                return;
        }
        
        for (p = disabled_first; p ; p = p->next)
                fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
                
        fprintf (fp, "%s\n",END_MARKER);
                
        fclose (fp);
}

void do_newcommands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int cmd;
    int col;
    int tmp;
 
	buffer = new_buf();

	for ( tmp = 0; cmd_type_table[tmp].type != NULL; tmp++ )
	{
	    col = 0;
	    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	    {
	        if ( cmd_table[cmd].level <  LEVEL_HERO
	       	&&   cmd_table[cmd].level <= get_trust( ch ) 
		&&   cmd_table[cmd].show == cmd_type_table[tmp].num_type )
		{
		    if (!col)
			{
			    sprintf( buf, "%s\n\r", cmd_type_table[tmp].type );
			    add_buf( buffer, buf );
			}		    
		    sprintf( buf, "%-12.12s", cmd_table[cmd].name );
		    add_buf( buffer, buf );
		    if ( ++col % 6 == 0 )
			add_buf( buffer, "\n\r" );
		}
	    }
	if (col != 0)
		add_buf( buffer, "\n\r" );
	if ( col % 6 != 0 )
		add_buf( buffer, "\n\r" );
	}

    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);

    return;
}

void do_newwizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int cmd;
    int col;
    int tmp;
 
	buffer = new_buf();

	for ( tmp = 0; cmd_type_table[tmp].type != NULL; tmp++ )
	{
	    col = 0;
	    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	    {
	        if ( cmd_table[cmd].level >= LEVEL_HERO
	       	&&   cmd_table[cmd].level <= get_trust( ch ) 
		&&   cmd_table[cmd].show == cmd_type_table[tmp].num_type )
		{
		    if (!col)
			{
			    sprintf( buf, "%s\n\r", cmd_type_table[tmp].type );
			    add_buf( buffer, buf );
			}		    
		    sprintf( buf, "%-12.12s", cmd_table[cmd].name );
		    add_buf( buffer, buf );
		    if ( ++col % 6 == 0 )
			add_buf( buffer, "\n\r" );
		}
	    }
	if (col != 0)
		add_buf( buffer, "\n\r" );
	if ( col % 6 != 0 )
		add_buf( buffer, "\n\r" );
	}

    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);

    return;
}

void do_scommands( CHAR_DATA *ch, char *argument )
{
	MPROG_LIST *pMprog;

	if ( !HAS_ROOM_TRIGGER( ch->in_room, RTRIG_COMM ) )
	{
		send_to_char( "No hay comandos especiales en este cuarto.\n\r", ch );
		return;
	}

	send_to_char( "Lista de comandos especiales:\n\r", ch );

	for ( pMprog = ch->in_room->mprogs; pMprog; pMprog = pMprog->next )
	{
		if ( pMprog->trig_type == RTRIG_COMM && pMprog->trig_phrase )
		{
			send_to_char( pMprog->trig_phrase, ch );
			send_to_char( "\n\r", ch );
		}
	}

	return;
}
