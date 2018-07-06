#include "include.h"
#include "olc.h"

#define SCEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )

const struct olc_cmd_type	scedit_table	[]	=
{
	{	"show",		scedit_show	},
	{	"new",		scedit_new	},
	{	"add",		scedit_add	},
	{	"remove",	scedit_remove	},
	{	"delete",	scedit_delete	},
	{	"list",		scedit_list	},
	{	"commands",	show_commands	},
	{	"?",		show_help	},
	{	NULL,		0		}
};
	
SCRIPT_DATA *script_free;
SCRIPT_DATA *script_list;

SCRIPT_DATA *script_lookup( int vnum );

SCRIPT_DATA *new_script( int largo )
{
	SCRIPT_DATA *temp;

	if ( script_free )
	{
		temp = script_free;
		script_free = script_free->next;
	}
	else
		temp = mud_malloc( sizeof( *temp ) );

	temp->comandos	= mud_malloc( sizeof(char *) * (largo + 1) );
	return temp;
}

void save_script( int vnum )
{
	FILE *fp;
	SCRIPT_DATA *script = script_lookup(vnum);
	char buf[MIL];
	int i;

	if ( script == NULL )
	{
		bug( "Save_script : Script con vnum %d inexistente.", vnum );
		return;
	}

	sprintf( buf, "%s/%d.scr", SCRIPT_DIR, vnum );

	if ( (fp = fopen( buf, "w" )) == NULL )
	{
		perror( buf );
		return;
	}

	for ( i = 0; script->comandos[i]; i++ )
		;

	fprintf( fp, "%d\n", i );

	for ( i = 0; script->comandos[i]; i++ )
		fprintf( fp, "%s~\n", script->comandos[i] );

	fclose( fp );
}
	
SCRIPT_DATA * load_script( int vnum )
{
	FILE *fp;
	SCRIPT_DATA *script;
	int largo, i = 0;
	char buf[MIL];

	sprintf( buf, "%s/%d.scr", SCRIPT_DIR, vnum );

	if ( (fp = fopen( buf, "r" )) == NULL )
	{
		perror( buf );
		return NULL;
	}

	sprintf( buf, "Cargando script %d.scr.", vnum );
	log_string( buf );

	largo			= fread_number( fp );
	script			= new_script( largo );
	while ( largo-- )
		script->comandos[i++]	= fread_string( fp );
	script->comandos[i]	= NULL;
	script->timer		= 0;
	script->posicion	= 0;
	script->vnum		= vnum;
	script->next		= script_list;
	script_list		= script;
	fclose( fp );

	return script;
}

SCRIPT_DATA *script_pedir( int vnum )
{
	SCRIPT_DATA * script = script_lookup(vnum);

	if (script)
		return script;

	return load_script(vnum);
}

SCRIPT_DATA *script_lookup( int vnum )
{
	SCRIPT_DATA *script;

	for ( script = script_list; script; script = script->next )
		if ( script->vnum == vnum )
			return script;

	return NULL;
}

void scedit( CHAR_DATA *ch, char *argument)
{
    SCRIPT_DATA *pScript;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_SCRIPT(ch, pScript);

    if (ch->pcdata->security < 5)
    {
        send_to_char("SCEdit: Insuficiente seguridad para modificar script.\n\r",ch);
        edit_done(ch);
        return;
    }

    if (command[0] == '\0')
    {
        scedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "delete") )
    {
    	scedit_delete(ch, argument);
    	return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; scedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix(command, scedit_table[cmd].name) )
        {
           if ((*scedit_table[cmd].olc_fun) (ch, argument))
		save_script(pScript->vnum);
           return;
        }
    }

    interpret(ch, arg);
    return;
}

void do_scedit(CHAR_DATA *ch, char *argument)
{
    SCRIPT_DATA *pScript;
    char command[MSL];
    char *temp;

    if ( IS_NPC(ch) )
    	return;

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Sintaxis : SCEdit [script]\n\r", ch );
    	return;
    }

    if (ch->pcdata->security < 5)
    {
    	send_to_char( "SCEdit : Insuficiente seguridad para editar scripts.\n\r", ch );
    	return;
    }

    temp = one_argument( argument, command );

    if ( !str_cmp( command, "new" ) )
    {
	scedit_new( ch, temp );
    	return;
    }

    if ( (pScript = script_pedir(atoi(argument))) == NULL )
    {
    	send_to_char( "SCEdit : Script no existe.\n\r", ch );
    	return;
    }

    ch->desc->pEdit	= (void *) pScript;
    ch->desc->editor	= ED_SCRIPT;

    return;
}

SCEDIT( scedit_show )
{
	SCRIPT_DATA *pScript;
	char buf[MSL];
	int i;

	EDIT_SCRIPT( ch, pScript );

	sprintf( buf, "Vnum  : [%d]\n\r", pScript->vnum );
	send_to_char( buf, ch );

	send_to_char( "Codigo:\n\r", ch );

	for ( i = 0; pScript->comandos[i]; ++i )
	{
		sprintf( buf, "%3d. %s\n\r", i, pScript->comandos[i] );
		send_to_char( buf, ch );
	}

	return FALSE;
}

SCEDIT( scedit_delete )
{
	SCRIPT_DATA *script, *tscr;
	MOB_INDEX_DATA *mob;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	int vnum, i;
	char buf[MSL];

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : delete [vnum]\n\r", ch );
		return FALSE;
	}

	vnum = atoi( argument );

	if ( ( script = script_lookup( vnum )) == NULL )
	{
		send_to_char( "SCEdit : script inexistente.\n\r", ch );
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		tch = CH(d);

		if ( tch && tch->desc && (tch->desc->editor == ED_SCRIPT) )
			edit_done( tch );
	}

	for ( i = 0; i < MAX_KEY_HASH; ++i )
		for ( mob = mob_index_hash[i]; mob; mob = mob->next )
			if ( mob->script == script )
				mob->script = NULL;

	for ( i = 0; script->comandos[i]; ++i )
		free_string( script->comandos[i] );

	free( script->comandos );
	script->timer		= 0;
	script->posicion	= 0;

	if ( script == script_list )
	{
		script_list		= script_list->next;
		script->next		= script_free;
		script_free		= script;
	}
	else
	{
		for ( tscr = script_list; tscr; tscr = tscr->next )
			if ( tscr->next == script )
				break;

		if ( !tscr || (tscr->next != script) )
		{
			bug( "Script_delete : Script %d no encontrado.", script->vnum );
			return FALSE;
		}

		tscr->next	= script->next;
		script->next	= script_free;
		script_free	= script;
	}

	sprintf( buf, "%s/%d.scr", SCRIPT_DIR, vnum );
	unlink( buf );

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SCEDIT( scedit_remove )
{
	SCRIPT_DATA *script;
	int i, maxl, linea, x = 0;
	char **newcom;

	EDIT_SCRIPT(ch,script);

	if (IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char( "Sintaxis : remove [numero linea]\n\r", ch );
		return FALSE;
	}

	linea = atoi( argument );

	for ( i = 0; script->comandos[i]; ++i )
		;

	maxl = i;

	if ( linea >= maxl )
	{
		send_to_char( "SCEdit : Linea inexistente.\n\r", ch );
		return FALSE;
	}

	script->posicion	= 0;
	script->timer		= 0;

	newcom = mud_malloc( sizeof(char *) * maxl ); /* uno menos */

	for ( i = 0; i < maxl; ++i )
	{
		if ( i == linea )
			continue;
		newcom[x++] = script->comandos[i];
	}

	newcom[maxl - 1] = NULL;
	free( script->comandos );
	script->comandos = newcom;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}


SCEDIT( scedit_add )
{
	SCRIPT_DATA *script;
	int linea, largo, i, found;
	char *temp;
	char **newcom;
	char palabra[MIL];

	EDIT_SCRIPT( ch, script );

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis: add [linea] [comandos]\n\r", ch );
		send_to_char( "          add [comandos]\n\r", ch );
		return FALSE;
	}

	temp = one_argument( argument, palabra );

	for ( i = 0; script->comandos[i]; ++i )
		;

	if ( is_number( palabra ) )
	{
		argument = temp;
		linea = atoi( palabra );
	}
	else	linea = i;

	if ( (largo = i) < linea )
	{
		send_to_char( "SCEdit : Script no tiene tantas lineas.\n\r", ch );
		return FALSE;
	}

	newcom	= mud_malloc( sizeof( char * ) * ( largo + 2 ) );
	found	= 0;

	for ( i = 0; i < (largo + 1); ++i )
	{
		if ( i == linea )
		{
			found		= 1;
			newcom[i]	= str_dup( argument );
		}
		else
			newcom[i]	= script->comandos[i - found];
	}

	newcom[largo + 1]	= NULL;
	free( script->comandos );
	script->comandos = newcom;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SCEDIT( scedit_new )
{
	SCRIPT_DATA *script;
	int vnum;

	if ( IS_NULLSTR(argument) || !is_number( argument ) )
	{
		send_to_char( "Sintaxis : new [vnum]\n\r", ch );
		return FALSE;
	}

	vnum = atoi( argument );

	if ( script_lookup( vnum ) )
	{
		send_to_char( "SCEdit : script ya existe.\n\r", ch );
		return FALSE;
	}

	script			= new_script( 0 );
	script->comandos[0]	= NULL;
	script->timer		= 0;
	script->posicion	= 0;
	script->vnum		= vnum;
	script->next		= script_list;
	script_list		= script;

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SCEDIT( scedit_list )
{
	SCRIPT_DATA *script;
	int i = 0;
	char buf[MIL];

	for ( script = script_list; script; script = script->next )
	{
		sprintf( buf, "%2d. #B%5d#b ", i++, script->vnum );
		send_to_char( buf, ch );
		if ( i % 10 == 0 )
			send_to_char( "\n\r", ch );
	}

	if ( i % 10 != 0 )
		send_to_char( "\n\r", ch );
	return FALSE;
}
