#define char_color(ch) TRUE
#include "include.h"
#include "puntos.h"

struct	rec_muerte puntajes[MAX_LEVEL/10 + 1][LARGO_TABLA_PUNTOS];

void destroy_puntajes(void)
{
	sh_int i,j;
	for ( j = 0; j < MAX_LEVEL/10; j++ )
		for (i = 0; i < LARGO_TABLA_PUNTOS; i++)
		{
			if (puntajes[j][i].name)
				free_string(puntajes[j][i].name);
			if (puntajes[j][i].who_name)
				free_string(puntajes[j][i].who_name);
		}
}

void do_puntajes(CHAR_DATA * ch, char *argument)
{
	sh_int i;
	char buf[MIL];
	int nivel;

	if ( is_number(argument) )
		nivel = atoi(argument);
	else
		nivel = getNivelPr(ch);

	nivel = URANGE(0, nivel, MAX_LEVEL);

	sprintf(buf, "Tabla de puntajes para los jugadores de nivel #B%d#b - #B%d#b:\n\r",
		(nivel/10)*10, (nivel/10 + 1)*10 - 1);
	send_to_char(buf, ch);

	sprintf(buf, "#U#BAct Mrt Niv Nombre#b                                                        #u\n\r");
	send_to_char(buf, ch);

	for (i = 0; i < LARGO_TABLA_PUNTOS; ++i)
	{
		sprintf(buf, "%s %c  %3d %3d %s %s #n\n\r",
			i == 0 ? "#B" : "",
			puntajes[nivel/10][i].activo ? '*' : ' ',
			puntajes[nivel/10][i].muertes,
			puntajes[nivel/10][i].nivel,
			CHECKNULLSTR(puntajes[nivel/10][i].name),
			CHECKNULLSTR(puntajes[nivel/10][i].who_name));
		send_to_char(buf, ch);
	}
}

void set_puntaje(CHAR_DATA * ch)
{
    sh_int i;

    for (i = 0; i < LARGO_TABLA_PUNTOS; ++i)
	if (!str_cmp(ch->name, puntajes[getNivelPr(ch)/10][i].name))
	    puntajes[getNivelPr(ch)/10][i].activo = 0;
}

int comp_punt( const void *vp, const void *vq )
{
	struct rec_muerte *p, *q;

	p = (struct rec_muerte *) vp;
	q = (struct rec_muerte *) vq;
	
	return ( q->muertes - p->muertes );
}

void revisa_muertes(CHAR_DATA * ch)
{
    sh_int i, j;
    bool activo;
    int nivel = getNivelPr(ch);

    if (!ch || IS_NPC(ch) || ch->pcdata->muertes < puntajes[nivel/10][LARGO_TABLA_PUNTOS - 2].muertes)
    	return;

    activo = FALSE;

    for ( i = 0; i < LARGO_TABLA_PUNTOS; ++i )
    	if ( !str_cmp( puntajes[nivel/10][i].name, ch->name) && puntajes[nivel/10][i].activo )
    	{
    		activo = TRUE;
    		break;
    	}

    if ( activo == FALSE ) 
    {
    	/* encontrar el primer jugador que tenga un puntaje menor que ch */
    	
    	for ( i = 0; i < LARGO_TABLA_PUNTOS; ++i )
		if ( puntajes[nivel/10][i].muertes < ch->pcdata->muertes )
    			break;
    	
    	if ( i == LARGO_TABLA_PUNTOS )
    	{
		bugf( "Revisa_muertes : ch %s no encontrado, nivel %d", ch->name, nivel );
		return;
	}
	
	/* liberamos los strings del ultimo pq se perderan */
	free_string(puntajes[nivel/10][LARGO_TABLA_PUNTOS-1].name);
	free_string(puntajes[nivel/10][LARGO_TABLA_PUNTOS-1].who_name);

	for ( j = LARGO_TABLA_PUNTOS - 1; j > i; --j )
		puntajes[nivel/10][j] = puntajes[nivel/10][j-1]; /* movemos la tabla hacia abajo */

	/* ahora escribimos la nueva entrada */
	puntajes[nivel/10][i].name	= str_dup( ch->name );
	puntajes[nivel/10][i].who_name	= str_dup( ch->pcdata->title );
	puntajes[nivel/10][i].muertes	= ch->pcdata->muertes;
	puntajes[nivel/10][i].activo	= TRUE;
	puntajes[nivel/10][i].nivel	= nivel;
	return;
    }

    puntajes[nivel/10][i].muertes	= ch->pcdata->muertes;
    puntajes[nivel/10][i].nivel		= nivel;
    puntajes[nivel/10][i].activo	= TRUE;

    qsort( puntajes[nivel/10], LARGO_TABLA_PUNTOS, sizeof( struct rec_muerte ), (void *) comp_punt );
}
