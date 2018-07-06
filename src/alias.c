/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
    static char szero[MSL];
    char *temparg;
    char args[5][MSL];
    int alias, i;
    bool aempty = FALSE;
    char *temp;

    smash_tilde(argument);

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (!IS_NPC(ch) && ch->pcdata->prefix[0] != '\0' && str_prefix("prefix",argument))
    {
	if (strlen(ch->pcdata->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
	    send_to_char("Linea demasiado larga, prefix no procesado.\r\n",ch);
	else
	{
	    sprintf(prefix,"%s %s",ch->pcdata->prefix,argument);
	    argument = prefix;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument)) 
    {
	if ( !run_olc_editor( d, d->incomm ) )
		interpret(d->character,argument);
	return;
    }

    for ( i = 0; i < 5; ++i )
    	args[i][0] = '\0';

    strcpy( buf, argument );
    argument = one_argument( argument, name ); /* saquemos el comando */
    i = 0;

    temparg = argument;
    while ( i < 5 && (aempty == FALSE) )
    {
    	temparg = one_argument( temparg, args[i] );
    	if ( IS_NULLSTR(temparg) || IS_NULLSTR(args[i]) )
    		aempty = TRUE;
    	else
    		i++;
    }

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!strcmp(ch->pcdata->alias[alias],name))
	{
		int pos = 0;

		strncpy(buf,szero,MSL);
		temp	= ch->pcdata->alias_sub[alias];
		while ( *temp )
		{
			if ( *temp != '$' )
				buf[pos++] = *temp;
			else
			{
				temp++;
				switch (*temp)
				{
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					if ( strlen( buf) + strlen( args[*temp - '1'] ) < MSL )
					{
						strcat( &buf[pos], args[*temp - '1'] );
						pos += strlen( args[*temp - '1'] );
					}
					break;

					case '*':
					if ( strlen(buf) + strlen(argument) < MSL )
					{
						strcat( &buf[pos], argument );
						pos += strlen( argument );
					}
					break;

					default:
					if ( pos < MSL )
						buf[pos++] = *temp;
				} /* switch */
			} /* else */
			if ( temp )
				temp++;
		} /* while */

		if (strlen(buf) > MAX_INPUT_LENGTH - 1)
		{
			send_to_char("Substitucion del alias demasiado larga. Truncada.\r\n",ch);
			buf[MAX_INPUT_LENGTH - 1] = '\0';
		}
	}
    }
    if ( !run_olc_editor( d, buf ) )
	interpret(d->character,buf);
}

void do_alia(CHAR_DATA *ch, char *argument)
{
    send_to_char("Lo lamento, alias debe estar escrito completamente.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);
    

    if (arg[0] == '\0')
    {

	if (rch->pcdata->alias[0] == NULL)
	{
	    send_to_char("No tienes aliases definidos.\n\r",ch);
	    return;
	}
	send_to_char("Tus aliases son:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    sprintf(buf,"    %s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	send_to_char("Lo lamento, esa palabra es reservada.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
		sprintf(buf,"%s es alias de '%s'.\n\r",rch->pcdata->alias[pos],
			rch->pcdata->alias_sub[pos]);
		send_to_char(buf,ch);
		return;
	    }
	}

	send_to_char("Ese alias no esta definido.\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	send_to_char("Eso no sera hecho!\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    sprintf(buf,"%s es ahora alias de '%s'.\n\r",arg,argument);
	    send_to_char(buf,ch);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	send_to_char("Lo lamento, alcanzaste el limite de aliases.\n\r",ch);
	return;
     }
  
     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     sprintf(buf,"%s es ahora alias de '%s'.\n\r",arg,argument);
     send_to_char(buf,ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument,arg);

    if (arg == '\0')
    {
	send_to_char("Unalias que?\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    send_to_char("Alias borrado.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No se encontro ningun alias con ese nombre.\n\r",ch);
}
