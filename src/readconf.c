#include "include.h"

struct var_type
{
	char *	nombre;
	sh_int	tipo;
	void *	variable;
};

#define	TIPO_BOOL	0
#define TIPO_INTEGER	1
#define TIPO_STRING	2

int	mudport;
bool	netup;
bool	mob_recalc;
int	max_host;
bool	identd;
bool	war;
int	pcdam = 100;
int	mobdam = 100;
char *	host_name;

struct var_type var_table [] =
{
	{	"port",		TIPO_INTEGER,	&mudport	},
	{	"netup",	TIPO_BOOL,	&netup		},
	{	"recalc",	TIPO_BOOL,	&mob_recalc	},
	{	"maxhost",	TIPO_INTEGER,	&max_host	},
	{	"identd",	TIPO_BOOL,	&identd		},
	{	"pcdam",	TIPO_INTEGER,	&pcdam		},
	{	"mobdam",	TIPO_INTEGER,	&mobdam		},
	{	"hostname",	TIPO_STRING,	&host_name	},
	{	"war",		TIPO_BOOL,	&war		},
	{	NULL,		0,		NULL		}
};

int var_lookup( const char * var )
{
	int temp;

	for ( temp = 0; var_table[temp].nombre; temp++ )
		if ( !str_cmp( var, var_table[temp].nombre ) )
			return temp;

	return -1;
}

void grabar_variables( void )
{
	FILE * fp;
	int i;

	fp = fopen( DATA_DIR "config", "w" );

	if ( !fp )
	{
		perror( "grabar_variables" );
		return;
	}

	for ( i = 0; var_table[i].nombre != NULL; i++ )
		switch(var_table[i].tipo)
		{
			case TIPO_INTEGER:
			fprintf( fp, "%s %d\n", var_table[i].nombre,
					    *(int *) var_table[i].variable );
			break;

			case TIPO_STRING:
			fprintf( fp, "%s %s\n", var_table[i].nombre,
						*(char **) var_table[i].variable );
			break;
			
			case TIPO_BOOL:
			fprintf( fp, "%s %s\n", var_table[i].nombre,
				*((bool *) var_table[i].variable) == FALSE ?
					"false" : "true" );
			break;

			default:
			fprintf( fp, "%s DANGER WILL ROBINSON!\n",
				var_table[i].nombre );
			break;
		}

	fprintf(fp,"END ");

	fclose(fp);
}


void leer_variables( void )
{
	FILE *fp;
	char word[MIL], value[MIL];
	int vt, valor;
	
	fp = fopen( DATA_DIR "config", "r" );

	if ( !fp )
	{
		perror( "leer_variables" );
		return;
	}

	while(TRUE)
	{
		fscanf( fp, "%s ", word );

		if ( !str_cmp( word, "END" ) || feof(fp) )
		{
			fclose(fp);
			return;
		}

		if ( (vt = var_lookup(word)) == -1 )
		{
			bugf( "leer_variables : var %s inexistente", word );
			exit(1);
		}

		switch(var_table[vt].tipo)
		{
			case TIPO_BOOL:
			{
				bool * blah = var_table[vt].variable;

				fscanf( fp, "%s", value );

				if ( !str_cmp( value, "true" ) )
					*blah = TRUE;
				else
					*blah = FALSE;
			}
			break;

			case TIPO_INTEGER:
			{
				int * blah = var_table[vt].variable;

				fscanf( fp, "%d", &valor );

				*blah = valor;
			}
			break;

			case TIPO_STRING:
			{
				char ** blah = var_table[vt].variable;

				fscanf( fp, "%s", value );

				*blah = strdup(value);
			}
			break;
		}
	}
}

char * var_to_string( int temp )
{
	static char buf[MIL];

	switch(var_table[temp].tipo)
	{
		case TIPO_INTEGER:
		{
			int * blah = var_table[temp].variable;
			sprintf( buf, "%d", *blah );
		}
		break;

		case TIPO_STRING:
		{
			char ** blah = var_table[temp].variable;
			strcpy( buf, *blah );
		}
		break;

		case TIPO_BOOL:
		{
			bool * blah = var_table[temp].variable;
			sprintf( buf, "%s", *blah ? "TRUE" : "FALSE" );
		}
		break;
	}

	return buf;
}

void do_config( CHAR_DATA *ch, char *argument )
{
	int temp;
	char buf[MIL];
	char arg[MIL];

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "#UVariable   Valor#u\n\r", ch );
		for ( temp = 0; var_table[temp].nombre; temp++ )
		{
			sprintf( buf, "%10s %s\n\r",
				var_table[temp].nombre,
				var_to_string(temp) );
			send_to_char( buf, ch );
		}
		return;
	}

	argument = one_argument(argument, arg);

	temp = var_lookup( arg );

	if ( temp == -1 )
	{
		send_to_char( "Variable inexistente.\n\r", ch );
		return;
	}

	switch(var_table[temp].tipo)
	{
		default:
		send_to_char( "DANGER WILL ROBINSON! tipo inexistente!\n\r", ch );
		return;

		case TIPO_BOOL:
		{
			bool * var = var_table[temp].variable;

			if (!str_cmp(argument,"true"))
				*var = TRUE;
			else
				*var = FALSE;
		}
		break;

		case TIPO_INTEGER:
		{
			int * var = var_table[temp].variable;
			
			*var = atoi(argument);
		}
		break;
		
		case TIPO_STRING:
		{
			char ** var = var_table[temp].variable;
			
			free(*var);
			*var = strdup(argument);
		}
		break;
	}

	send_to_char("Ok.\n\r", ch );
	grabar_variables();
}
