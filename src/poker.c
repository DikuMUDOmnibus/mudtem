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
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "poker.h"

char        *palabra2 (int valor)
{
      switch (valor)
      {
      case 0:
	    return "C";
      case 1:
	    return "T";
      case 2:
	    return "D";
      case 3:
	    return "P";
      default:
	    return "?";
      }

      return "";
}

char        *palabra (int valor)
{
      switch (valor)
      {
      case 1:
	    return "1";
      case 2:
	    return "2";
      case 3:
	    return "3";
      case 4:
	    return "4";
      case 5:
	    return "5";
      case 6:
	    return "6";
      case 7:
	    return "7";
      case 8:
	    return "8";
      case 9:
	    return "9";
      case 10:
	    return "10";
      case 11:
	    return "J";
      case 12:
	    return "Q";
      case 13:
	    return "K";
      case 14:
	    return "A";
      default:
	    return "?";
      }

      return "";
}

POKER_DATA  *max_valor (void)
{
      POKER_DATA  *poker, *mejor = NULL;
      int          max_val = 0;

      for (poker = poker_list; poker; poker = poker->next)
	    if (valor_cartas (poker) > max_val)
	    {
		  max_val = valor_cartas (poker);
		  mejor = poker;
	    }

      if (mejor == NULL)
	    bug ("max_valor : mejor == NULL", 0);

      return mejor;
}

char        *color (int i)
{
      switch (i)
      {
      case 0:
	    return "#3";
      case 1:
	    return "#7";
      case 2:
	    return "#3";
      case 3:
	    return "#7";
      default:
	    return "?";
      }

      return "";
}

long         pot2 (int i)
{
      if (i == 0)
	    return 1;
      else
	    return (2 * pot2 (i - 1));
}

void         ordenar_cartas (POKER_DATA * poker)
{
      int          i, j, temp;

      for (i = 0; i < 4; ++i)
	    for (j = 4; j > i; --j)
		  if (poker->poker[j - 1] > poker->poker[j])
		  {
			temp = poker->poker[j - 1];
			poker->poker[j - 1] = poker->poker[j];
			poker->poker[j] = temp;
		  }

      return;
}

POKER_DATA  *get_turno (void)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    if (poker->turno)
		  return poker;

      bug ("get_turno : turno nulo", 0);
      return NULL;
}

int          valor_cartas (POKER_DATA * poker)
{
      int          ult, ultcart, pinta, i;
      bool         ccolor = TRUE, escala = TRUE;

      ultcart = NUMERO (0);
      ult = FIGURA (0);

      for (i = 1; i < 5; ++i)	/* escala real , escala, pinta */
      {
	    pinta = FIGURA (i);

	    if (pinta != ult)
		  ccolor = FALSE;

	    if (ultcart != (NUMERO (i) - 1))
		  escala = FALSE;

	    ultcart = NUMERO (i);
	    ult = pinta;

      }

      if (escala && ccolor)	/* escala real */
	    return (8000 + NUMERO (4));
      else if (escala)		/* escala simple */
	    return (4000 + NUMERO (4));

      if ((NUMERO (0) == NUMERO (3))
	  || (NUMERO (1) == NUMERO (4)))
	    return (7000 + NUMERO (2));		/* poker */

      if (ccolor)
	    return (6000 + NUMERO (4));		/* color */

      if (((NUMERO (0) == NUMERO (1) && NUMERO (1) == NUMERO (2)) &&
	   (NUMERO (3) == NUMERO (4))) || ((NUMERO (0) == NUMERO (1)) &&
		    (NUMERO (2) == NUMERO (3) && NUMERO (3) == NUMERO (4))))	/* full */
	    return (5000 + NUMERO (2) * 50 +
		    ((NUMERO (0) == NUMERO (1)) ? NUMERO (0) : NUMERO (4)));

      if ((NUMERO (0) == NUMERO (1) && NUMERO (1) == NUMERO (2)) ||
	  (NUMERO (1) == NUMERO (2) && NUMERO (2) == NUMERO (3)) ||
	  (NUMERO (2) == NUMERO (3) && NUMERO (3) == NUMERO (4)))
	    return (3000 + NUMERO (2));		/* trio */

      if (((NUMERO (0) == NUMERO (1)) && (NUMERO (3) == NUMERO (4))) ||
	  ((NUMERO (0) == NUMERO (1)) && (NUMERO (2) == NUMERO (3))) ||
	  ((NUMERO (1) == NUMERO (2)) && (NUMERO (3) == NUMERO (4))))
	    return (2000 + NUMERO (3) * 50 + NUMERO (1));	/* dos pares */

      if (NUMERO (0) == NUMERO (1))
	    return (1000 + NUMERO (1));		/* par 1 */

      if (NUMERO (1) == NUMERO (2))
	    return (1000 + NUMERO (2));		/* par 2 */

      if (NUMERO (2) == NUMERO (3))
	    return (1000 + NUMERO (3));		/* par 3 */

      if (NUMERO (3) == NUMERO (4))
	    return (1000 + NUMERO (4));		/* par 4 */

      return (NUMERO (4));	/* nada */
}

void         mostrar_cartas (POKER_DATA * poker, bool to_all)
{
      int          i = 0, carta_min;
      char         buf[MAX_STRING_LENGTH];
      char         buf2[MAX_STRING_LENGTH * 4];

      carta_min = 15 - numero_cartas / 4;

      buf2[0] = '\0';

      for (i = 0; i < 5; ++i)
      {
	    sprintf (buf, "#R%s%2s   #n ", color (FIGURA (i)), palabra (NUMERO (i) + carta_min));
	    strcat (buf2, buf);
      }

      strcat (buf2, "\n\r");

      for (i = 0; i < 5; ++i)
      {
	    sprintf (buf, "#R%s  %s  #n ", color (FIGURA (i)), palabra2 (FIGURA (i)));
	    strcat (buf2, buf);
      }

      strcat (buf2, "\n\r");

      for (i = 0; i < 5; ++i)
      {
	    sprintf (buf, "#R%s   %2s#n ", color (FIGURA (i)), palabra (NUMERO (i) + carta_min));
	    strcat (buf2, buf);
      }

      strcat (buf2, "\n\r");

      if (poker_stat == POKER_JUGANDO)
	    for (i = 0; i < 5; ++i)
	    {
		  sprintf (buf, "%s ", (IS_SET (poker->cambiar, pot2 (i)) ? "#BCAMBR#b" : "     "));
		  strcat (buf2, buf);
	    }

      strcat (buf2, "\n\r");

      if (poker_stat != POKER_NADA && !to_all)
      {
	    sprintf (buf, "Es el turno de %s.\n\r", PERS (get_turno ()->jugador, get_turno ()->jugador));
	    strcat (buf2, buf);
      }

      sprintf (buf, "Valor cartas : %d\n\r", valor_cartas (poker));
      strcat (buf2, buf);

      if (to_all)
	    mensaje_jugadores (buf2);
      else
	    send_to_char (buf2, poker->jugador);

      return;
}

void         actualizar_turno (void)
{
      POKER_DATA  *poker;

      poker = get_turno ();

      poker->turno = FALSE;

      if (poker->next)
	    poker->next->turno = TRUE;
      else
	    poker_list->turno = TRUE;

      if (poker_stat != POKER_NADA)
	    send_to_char ("Tu turno.\n\r", get_turno ()->jugador);

      return;
}

void         repartir_cartas (void)
{
      POKER_DATA  *poker;
      int          numero, i;

      for (poker = poker_list; poker; poker = poker->next)
      {
	    i = 0;

	    while (i < 5)
	    {
		  numero = number_range (0, numero_cartas - 1);
		  if (cartas[numero] == FALSE)
		  {
			cartas[numero] = TRUE;
			poker->poker[i] = numero;
			i++;
		  }
	    }
	    ordenar_cartas (poker);
	    mostrar_cartas (poker, FALSE);
      }
}

void         limpiar_turnos (void)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    poker->turno = FALSE;

      return;
}

bool         es_jugador (CHAR_DATA * ch)
{
      POKER_DATA  *poker;

      if (IS_NPC (ch))
	    return FALSE;

      for (poker = poker_list; poker; poker = poker->next)
	    if (poker->jugador == ch)
		  return TRUE;

      return FALSE;
}

bool         todos_cambiaron (void)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    if (!IS_SET (poker->cambiar, CAMBIAR_LISTO))
		  return FALSE;

      return TRUE;
}

POKER_DATA  *get_poker_data (CHAR_DATA * ch)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    if (poker->jugador == ch)
		  return poker;

      bug ("get_poker_data : ch no estaba en poker_list", 0);
      return NULL;
}

int          cartas_camb (POKER_DATA * poker)
{
      int          i, cnt = 0;

      for (i = 0; i < 5; ++i)
	    if (IS_SET (poker->cambiar, pot2 (i)))
		  cnt++;

      return cnt;
}

void         inscrib_char (CHAR_DATA * ch)
{
      POKER_DATA  *poker;
      int          i;

      poker = new_poker_data ();
      poker->jugador = ch;
      poker->cambiar = 0;
      poker->turno = FALSE;

      for (i = 0; i < 5; ++i)
	    poker->poker[i] = 0;

      numero_jugadores++;

      return;
}

POKER_DATA  *new_poker_data (void)
{
      POKER_DATA  *poker;

      if (poker_free)
      {
	    poker = poker_free;
	    poker_free = poker_free->next;
      }
      else
	    poker = alloc_mem (sizeof (*poker));

      poker->next = poker_list;
      poker_list = poker;

      return poker;
}

void         elim_char (CHAR_DATA * ch)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    if (poker->jugador == ch)
	    {
		  free_poker (poker);
		  numero_jugadores--;
		  break;
	    }

      return;
}

void         free_poker (POKER_DATA * poker)
{
      bool         found = FALSE;

      if (poker == poker_list)
      {
	    poker_list = poker->next;
	    poker->next = poker_free;
	    poker_free = poker;
	    found = TRUE;
      }
      else
      {
	    POKER_DATA  *prev;

	    for (prev = poker_list; prev; prev = prev->next)
		  if (prev->next == poker)
			break;

	    if (prev == NULL || prev->next != poker)
	    {
		  bug ("free_poker : prev no encontrado", 0);
		  return;
	    }

	    prev->next = poker->next;
	    poker->next = poker_free;
	    poker_free = poker;
	    found = TRUE;
      }

      if (found == FALSE)
	    bug ("elim_char : ch no estaba en poker_list", 0);

      return;
}

void         mensaje_jugadores (char *buf)
{
      POKER_DATA  *poker;

      for (poker = poker_list; poker; poker = poker->next)
	    send_to_char (buf, poker->jugador);

      return;
}

void         limpiar_poker (void)
{
      POKER_DATA  *poker;
      int          i;

      if (poker_list != NULL)
      {
	    for (poker = poker_list; poker; poker = poker->next)
		  if (poker->next == NULL)
			break;

	    if (poker == NULL || poker->next != NULL)
	    {
		  bug ("limpiar_poker : poker->next != NULL", 0);
		  return;
	    }

	    poker->next = poker_free;
	    poker_free = poker_list;
	    poker_list = NULL;
      }

      for (poker = poker_free; poker; poker = poker->next)
      {
	    poker->jugador = NULL;
	    poker->cambiar = 0;
	    poker->turno = FALSE;
	    for (i = 0; i < 5; ++i)
		  poker->poker[i] = 0;
      }

      for (i = 0; i < MAX_CARTAS; ++i)
	    cartas[i] = FALSE;

      pozo = 0;
      ultap = 0;
      numero_jugadores = 0;
      poker_stat = POKER_NADA;

      return;
}

void         poker_retirar (CHAR_DATA * ch)
{
      POKER_DATA  *poker;
      char         buf[MSL];

      poker = get_poker_data (ch);

      if (poker->turno)
      {
	    if (poker->next)
		  poker->next->turno = TRUE;
	    else
		  poker_list->turno = TRUE;
      }

      send_to_char ("Ok.\n\r", ch);

      sprintf (buf, "%s se retira del juego.\n\r", PERS (ch, ch));
      mensaje_jugadores (buf);

      elim_char (ch);

      if (numero_jugadores == 1)
      {
	    send_to_char ("Ganaste por abandono.\n\r", poker_list->jugador);
	    poker_list->jugador->silver += pozo;
	    limpiar_poker ();
	    return;
      }

      if (numero_jugadores == 0)
	    limpiar_poker ();

      return;
}

void         poker_cambiar (CHAR_DATA * ch, char *argument)
{
      POKER_DATA  *poker;
      char         arg1[MSL];
      int          valor, carta;

      poker = get_poker_data (ch);

      if (argument[0] == '\0')
	    return;

      if (IS_SET (poker->cambiar, CAMBIAR_LISTO))
      {
	    send_to_char ("Ya cambiaste tus cartas.\n\r", ch);
	    return;
      }

      argument = one_argument (argument, arg1);

      if (!is_number (arg1))
      {
	    send_to_char ("Sintaxis: poker cambiar 1 2 3..\n\r", ch);
	    return;
      }

      valor = atoi (arg1);

      if (valor < 1 || valor > 5)
      {
	    send_to_char ("Valores entre 1 y 5.\n\r", ch);
	    return;
      }

      carta = pot2 (valor - 1);

      if (IS_SET (poker->cambiar, carta))
	    send_to_char ("Mantendras esa carta.\n\r", ch);
      else
	    send_to_char ("Cambiaras esa carta.\n\r", ch);

      TOGGLE_BIT (poker->cambiar, carta);

      if (argument[0] != '\0')
	    poker_cambiar (ch, argument);

      return;
}

void         cambiar_cartas (POKER_DATA * poker)
{
      int          cnt = 0;
      char         buf[MSL];

      int          i, x;
      bool         encont = FALSE;

      for (i = 0; i < 5; ++i)
      {
	    encont = FALSE;
	    if (IS_SET (poker->cambiar, pot2 (i)))
		  while (encont == FALSE)
		  {
			x = number_range (0, numero_cartas - 1);
			if (cartas[x] == FALSE)
			{
			      poker->poker[i] = x;
			      cartas[x] = TRUE;
			      encont = TRUE;
			      cnt++;
			}
		  }
      }

      poker->cambiar = CAMBIAR_LISTO;

      ordenar_cartas (poker);
      mostrar_cartas (poker, FALSE);
      sprintf (buf, "%s cambia %d de sus cartas.\n\r", PERS (poker->jugador, poker->jugador), cnt);
      mensaje_jugadores (buf);

      return;
}

void         poker_cambiar_listo (CHAR_DATA * ch)
{
      POKER_DATA  *poker;
      char         buf[MSL];

      poker = get_poker_data (ch);

      if (IS_SET (poker->cambiar, CAMBIAR_LISTO))
      {
	    send_to_char ("Ya cambiaste tus cartas.\n\r", ch);
	    return;
      }

      cambiar_cartas (poker);
      SET_BIT (poker->cambiar, CAMBIAR_LISTO);

      if (poker_stat == POKER_JUGANDO && todos_cambiaron ())
      {
	    sprintf (buf, "%s dice 'Empiezan las apuestas!'.\n\r", PERS (ch, ch));
	    mensaje_jugadores (buf);
	    poker_stat = POKER_APUESTAS;
      }

      actualizar_turno ();

      return;
}


void         do_poker (CHAR_DATA * ch, char *argument)
{
      char         buf[MSL];
      char         arg1[MSL] /*, arg2[MSL] */ ;

      if (IS_NPC (ch))
	    return;

      if (argument[0] == '\0')
      {
	    send_to_char ("Comandos de poker:\n\r", ch);
	    send_to_char (" poker inscribir        : para inscribirte en el juego.\n\r", ch);
	    send_to_char (" poker jugar            : para empezar el juego entre los ya inscritos.\n\r", ch);
	    send_to_char (" poker retirar          : para retirarte del juego.\n\r", ch);
	    send_to_char (" poker cambiar <numero> : para marcar la carta numero <numero> para cambiar.\n\r", ch);
	    send_to_char (" poker cambiar listo    : para cambiar las cartas marcadas.\n\r", ch);
	    send_to_char (" poker subir <numero>   : para subir las apuestas en <numero> monedas de plata.\n\r", ch);
	    send_to_char (" poker cubrir           : para cubrir la apuesta actual.\n\r", ch);
	    send_to_char (" poker terminar         : para alcanzar la apuesta y terminar el juego.\n\r", ch);
	    return;
      }

      argument = one_argument (argument, arg1);

      if (!str_cmp (arg1, "inscribir"))
      {
	    if (es_jugador (ch))
	    {
		  send_to_char ("Pero tu ya estas inscrito!\n\r", ch);
		  return;
	    }

	    if (poker_stat == POKER_JUGANDO || poker_stat == POKER_APUESTAS)
	    {
		  send_to_char ("Espera a que desocupen la mesa.\n\r", ch);
		  return;
	    }

	    if (numero_jugadores > 4)
	    {
		  send_to_char ("La mesa esta llena.\n\r", ch);
		  return;
	    }

	    if (DINERO (ch) < 50)
	    {
		  send_to_char ("No tienes suficiente dinero.\n\r", ch);
		  return;
	    }

	    if (numero_jugadores == 0)
		  limpiar_poker ();

	    deduct_cost (ch, 50);
	    pozo += 50;

	    act ("$n quiere jugar poker.", ch, NULL, NULL, TO_ROOM);
	    send_to_char ("Pagas la inscripcion de 50 monedas de plata.\n\r", ch);

	    sprintf (buf, "%s paga la inscripcion.\n\r", PERS (ch, ch));
	    mensaje_jugadores (buf);

	    inscrib_char (ch);

	    return;
      }

      if (!str_cmp (arg1, "retirar"))
      {
	    if (!es_jugador (ch))
	    {
		  send_to_char ("Ni siquiera estas jugando!\n\r", ch);
		  return;
	    }

	    poker_retirar (ch);
	    return;
      }

      if (!str_cmp (arg1, "jugar"))
      {
	    if (!es_jugador (ch))
	    {
		  send_to_char ("Ni siquiera estas jugando!\n\r", ch);
		  return;
	    }

	    if (poker_stat != POKER_NADA)
	    {
		  send_to_char ("Ya estan jugando!\n\r", ch);
		  return;
	    }

	    if (numero_jugadores < 2)
	    {
		  send_to_char ("No puedes jugar solo!\n\r", ch);
		  return;
	    }

	    /* let's play */

	    poker_stat = POKER_JUGANDO;
	    limpiar_turnos ();
	    poker_list->turno = TRUE;
	    numero_cartas = 12 + numero_jugadores * 8;
	    sprintf (buf, "%s inicia el juego.\n\r", PERS (poker_list->jugador, poker_list->jugador));
	    mensaje_jugadores (buf);
	    repartir_cartas ();
	    send_to_char ("Tu turno.\n\r", poker_list->jugador);
	    return;
      }

      if (!str_cmp (arg1, "cambiar"))
      {
	    if (!es_jugador (ch))
	    {
		  send_to_char ("Ni siquiera estas jugando.\n\r", ch);
		  return;
	    }

	    if (poker_stat != POKER_JUGANDO)
	    {
		  send_to_char ("No es hora de cambiar.\n\r", ch);
		  return;
	    }

	    if (str_cmp (argument, "listo"))
	    {
		  poker_cambiar (ch, argument);
		  return;
	    }

	    if (get_turno ()->jugador != ch)
	    {
		  sprintf (buf, "Es el turno de %s.\n\r", PERS (get_turno ()->jugador, get_turno ()->jugador));
		  send_to_char (buf, ch);
		  return;
	    }

	    poker_cambiar_listo (ch);

	    return;
      }

      if (!str_cmp (arg1, "ver"))
      {
	    if (!es_jugador (ch))
	    {
		  send_to_char ("Ni siquiera estas jugando.\n\r", ch);
		  return;
	    }

	    if (poker_stat == POKER_NADA)
	    {
		  send_to_char ("Todavia no empiezas a jugar!\n\r", ch);
		  return;
	    }

	    mostrar_cartas (get_poker_data (ch), FALSE);
	    return;
      }

      if (!str_cmp (arg1, "subir"))
      {
	    int          valor;

	    if (!is_number (argument))
	    {
		  send_to_char ("Debes poner un valor como argumento.\n\r", ch);
		  return;
	    }

	    valor = atoi (argument);

	    if (!es_jugador (ch))
	    {
		  send_to_char ("No estas jugando!\n\r", ch);
		  return;
	    }

	    if (DINERO(ch) < ultap + valor)
	    {
		  sprintf (buf, "No tienes %ld monedas de plata!\n\r", ultap + valor);
		  send_to_char (buf, ch);
		  return;
	    }

	    if (poker_stat != POKER_APUESTAS)
	    {
		  send_to_char ("Ni siquiera estas apostando.\n\r", ch);
		  return;
	    }

	    if (get_turno ()->jugador != ch)
	    {
		  sprintf (buf, "Es el turno de %s!\n\r", PERS (get_turno ()->jugador, get_turno ()->jugador));
		  send_to_char (buf, ch);
		  return;
	    }

	    sprintf (buf, "%s sube las apuestas en %d MP.\n\r", PERS (ch, ch), valor);
	    mensaje_jugadores (buf);
	    sprintf (buf, "Para alcanzar su apuesta, necesitas %ld MP.\n\r", ultap + valor);
	    mensaje_jugadores (buf);
	    actualizar_turno ();
	    ultap += valor;
	    deduct_cost(ch, ultap);
	    pozo += ultap;
	    return;
      }

      if (!str_cmp (arg1, "terminar"))
      {
	    POKER_DATA  *ganador;

	    if (!es_jugador (ch))
	    {
		  send_to_char ("No estas jugando!\n\r", ch);
		  return;
	    }

	    if (poker_stat != POKER_APUESTAS)
	    {
		  send_to_char ("Ni siquiera estas apostando.\n\r", ch);
		  return;
	    }

	    if (get_turno ()->jugador != ch)
	    {
		  sprintf (buf, "Es el turno de %s!\n\r", PERS (get_turno ()->jugador, get_turno ()->jugador));
		  send_to_char (buf, ch);
		  return;
	    }

	    if (DINERO(ch) < ultap)
	    {
		  sprintf (buf, "No tienes %ld monedas de plata!\n\r", ultap);
		  send_to_char (buf, ch);
		  return;
	    }

	    deduct_cost(ch, ultap);
	    pozo += ultap;
	    sprintf (buf, "%s alcanza la apuesta!\n\r", PERS (ch, ch));
	    mensaje_jugadores (buf);
	    ganador = max_valor ();
	    sprintf (buf, "El ganador es %s!\n\r", PERS (ganador->jugador, ganador->jugador));
	    mensaje_jugadores (buf);
	    mostrar_cartas (ganador, TRUE);
	    sprintf (buf, "El pozo acumulado era %ld.\n\r", pozo);
	    mensaje_jugadores (buf);
	    sprintf (buf, "Ganaste %ld MP!\n\r", pozo);
	    send_to_char (buf, ganador->jugador);
	    ganador->jugador->silver += pozo;
	    limpiar_poker ();
	    poker_stat = POKER_NADA;
	    return;
      }

      if (!str_cmp (arg1, "cubrir"))
      {
	    if (!es_jugador (ch))
	    {
		  send_to_char ("No estas jugando!\n\r", ch);
		  return;
	    }
	    if (poker_stat != POKER_APUESTAS)
	    {
		  send_to_char ("Ni siquiera estas apostando.\n\r", ch);
		  return;
	    }
	    if (get_turno ()->jugador != ch)
	    {
		  sprintf (buf, "Es el turno de %s!\n\r", PERS (get_turno ()->jugador, get_turno ()->jugador));
		  send_to_char (buf, ch);
		  return;
	    }
	    if (DINERO(ch) < ultap)
	    {
		  sprintf (buf, "No tienes %ld monedas de plata!\n\r", ultap);
		  send_to_char (buf, ch);
		  return;
	    }
	    sprintf (buf, "%s cubre.\n\r", ch->name);
	    mensaje_jugadores (buf);
	    deduct_cost(ch, ultap);
	    pozo += ultap;
	    actualizar_turno ();
	    return;
      }

      do_poker (ch, "");
}

void         do_spoker (CHAR_DATA * ch, char *argument)
{
      POKER_DATA  *poker;
      char         buf[MSL];

      if (numero_jugadores == 0)
      {
	    send_to_char ("No hay juego en desarrollo.\n\r", ch);
	    return;
      }

      for (poker = poker_list; poker; poker = poker->next)
      {
	    sprintf (buf, "Jugador: %s\n\r", PERS (poker->jugador, poker->jugador));
	    send_to_char (buf, ch);
      }

      sprintf (buf, "Pozo: $%ld.\n\r", pozo);
      send_to_char (buf, ch);

      return;
}

void         do_pmem (CHAR_DATA * ch, char *argument)
{
      POKER_DATA  *poker;
      char         buf[MSL];
      int          cnt_poker = 0, cnt_free = 0;

      for (poker = poker_list; poker; poker = poker->next)
	    cnt_poker++;

      for (poker = poker_free; poker; poker = poker->next)
	    cnt_free++;

      sprintf (buf, "Poker_list : %d\n\rPoker_free : %d\n\r", cnt_poker, cnt_free);
      send_to_char (buf, ch);

      return;
}

void         do_ptell (CHAR_DATA * ch, char *argument)
{
      char         buf[MSL];

      if (argument[0] == '\0')
      {
	    send_to_char ("Que quieres decirle a los jugadores de poker?\n\r", ch);
	    return;
      }

      if (!es_jugador (ch))
      {
	    send_to_char ("No estas jugando.\n\r", ch);
	    return;
      }

      sprintf (buf, "#BPOKER#b - %s : '%s'#n\n\r", PERS (ch, ch), argument);
      mensaje_jugadores (buf);

      return;
}
