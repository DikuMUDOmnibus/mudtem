/*
 * The Mythran Mud Economy Snippet Version 2 (used to be banking.c)
 *
 * Copyrights and rules for using the economy system:
 *
 *	The Mythran Mud Economy system was written by The Maniac, it was
 *	loosly based on the rather simple 'Ack!'s banking system'
 *
 *	If you use this code you must follow these rules.
 *		-Keep all the credits in the code.
 *		-Mail Maniac (v942346@si.hhs.nl) to say you use the code
 *		-Send a bug report, if you find 'it'
 *		-Credit me somewhere in your mud.
 *		-Follow the envy/merc/diku license
 *		-If you want to: send me some of your code
 *
 * All my snippets can be found on http://www.hhs.nl/~v942346/snippets.html
 * Check it often because it's growing rapidly	-- Maniac --
 */

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_say	);
DECLARE_DO_FUN( do_save	);

#define COMUNICAR(buf) comunicar(mob ? mob : ch,obj,buf)

#define MAX_PLATA pow(2, 30)

int	share_value = SHARE_VALUE;	/* External share_value by Maniac */

#define cuenta_banco(ch) get_bank(ch, BANK_DEPOSITO)

BANK_DATA * get_bank( CHAR_DATA *ch, int tipo )
{
	BANK_DATA *bd;

	if ( IS_NPC(ch) )
		return NULL;

	for ( bd = ch->pcdata->bank; bd; bd = bd->next )
		if ( bd->tipo == tipo )
			return bd;

	return NULL;
}

BANK_DATA * bank_give( CHAR_DATA *ch, long valor, int tipo, time_t tiempo, float interes )
{
	BANK_DATA *bank 	= new_bank();

	bank->valor		= valor;
	bank->tipo		= tipo;
	bank->when		= tiempo;
	bank->start		= tiempo;
	bank->interes		= interes;

	bank->next		= ch->pcdata->bank;
	ch->pcdata->bank	= bank;

	return bank;
}

void comunicar( CHAR_DATA *ch, OBJ_DATA *obj, char *argument )
{
	if ( obj && ch )
		act( "$p dice '$T'.", ch, objToEnt(obj), strToEnt(argument,ch->in_room), TO_ALL );
	else if ( ch )
	{
		REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
		do_say( ch, argument );
	}
	else if ( obj && obj->in_room && obj->in_room->people )
		act( "$p dice '$T'.", obj->in_room->people, objToEnt(obj), strToEnt(argument,obj->in_room), TO_ALL );
}

void do_bank( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *mob;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Los servicios del banco estan solamente disponibles para los jugadores\n\r", ch );
		return;
	}
  
	/* Check for mob with act->banker */
	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER ) )
			break;
	}

	if ( !mob )
	{
		for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
			if ( IS_OBJ_STAT(obj, ITEM_ATM) )
				break;
	}
	
	if ( !mob && !obj )
	{
		send_to_char( "No puedes hacer eso aqui.\n\r", ch );
		return;
	}

	if ( mob
	&& ( time_info.hour < 9
	||   time_info.hour > 17 ) )
	{
		COMUNICAR( "El banco esta cerrado, solo esta abierto de 9AM a 5PM." );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "Opciones del banco:\n\r", ch );
		if ( ch->pcdata->bank )
		{
			send_to_char( "#BBALANCE#b              : entrega tu balance.\n\r", ch );
			send_to_char( "#BWITHDRAW#b <cnt>       : Retira oro de tu cuenta.\n\r", ch );
			send_to_char( "#BTRANSFER#b <cnt> <plr> : Transfiere <cnt> de oro a la cuenta de <plr>.\n\r", ch); 
			send_to_char( "#BPAGAR#b                : Cancelas tu deuda con el Banco.\n\r", ch );
		}
		send_to_char( "#BCHECK#b                : entrega el valor actual de las acciones.\n\r", ch);
		if (!obj)
		{
			if (ch->pcdata->bank)
			{
				send_to_char( "#BDEPOSIT#b <cnt>        : Deposita oro en tu cuenta al 2% de interes.\n\r", ch );
				send_to_char( "#BBUY#b <cnt>            : Compra <cnt> acciones.\n\r", ch);
				send_to_char( "#BSELL#b <cnt>           : Vende <cnt> acciones.\n\r", ch);
				send_to_char( "#BPRESTAMO#b <cnt>       : El Banco te presta <cnt> MO con un interes del 3%.\n\r", ch);
			}
			else
				send_to_char( "#BABRIR#b                : Abre una cuenta con tu nombre.\n\r", ch );
		}
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	/* Now work out what to do... */
	if ( !str_prefix( arg1, "abrir" ) )
	{
		if ( cuenta_banco(ch) )
		{
			COMUNICAR("Ya tienes una cuenta.");
			return;
		}

		if ( ch->gold < 1000 )
		{
			COMUNICAR("Necesitas al menos 1000 monedas de oro para abrir una cuenta.");
			return;
		}

		ch->gold -= 1000;
		bank_give(ch,1000, BANK_DEPOSITO, current_time, INTERES_DEPOSITO);

		COMUNICAR("Ahora tienes una cuenta de 1000MO en el banco.");
		return;
	}

        if ( !str_prefix( arg1, "check" ) )
	{
		BANK_DATA *accion = get_bank(ch, BANK_ACCIONES);

		sprintf (buf, "El valor actual de las acciones es %d.", share_value);
		COMUNICAR(buf);
		if (accion)
		{
		    sprintf (buf, "Tienes %ld acciones (%d c/u), en total valen %ld MO.",
			accion->valor,  share_value,
			accion->valor * share_value );
		    COMUNICAR(buf);
		}
		return;
        }

	if ( !str_prefix( arg1, "balance" ) )
	{
		BANK_DATA *cuenta = cuenta_banco(ch);
		BANK_DATA *pres = get_bank(ch, BANK_PRESTAMO);

		if ( cuenta == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

		sprintf(buf,"Tu balance actual es: #B%ld#b MO.", cuenta->valor );
		COMUNICAR(buf);
		if (pres && pres->valor > 0)
		{
			sprintf(buf,"Tienes una deuda de #B%ld#b MO con el Banco.", pres->valor );
			COMUNICAR(buf);
		}
		return;
	}

	if ( !str_prefix(arg1, "pagar") )
	{
		BANK_DATA *cuenta = cuenta_banco(ch);
		BANK_DATA *pres = get_bank(ch, BANK_PRESTAMO);

		if ( !cuenta )
		{
			COMUNICAR( "Necesitas una cuenta para eso." );
			return;
		}

		if ( !pres || pres->valor == 0 )
		{
			COMUNICAR( "No le debes nada al Banco." );
			return;
		}

		if ( cuenta->valor < pres->valor )
		{
			COMUNICAR( "No tienes suficiente dinero en tu cuenta como para cancelar tu deuda." );
			return;
		}

		printf_to_char( ch, "Desembolsas #B%ld#b MO.\n\r", pres->valor );
		cuenta->valor	-= pres->valor;
		pres->valor	 = 0;
		extract_bank( ch->pcdata, pres );
		COMUNICAR( "Acabas de cancelar tu deuda con el Banco." );
		return;
	}

	if ( !str_prefix(arg1, "prestamo") && !obj )
	{
		BANK_DATA *cuenta;
		BANK_DATA *pres;
		int cnt = atoi(arg2);

		if ( getNivelPr(ch) < 10 )
		{
			COMUNICAR( "Debes ser al menos nivel 10 para pedir un prestamo." );
			return;
		}

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

		if ( (pres = get_bank(ch, BANK_PRESTAMO)) == NULL )
			pres = bank_give(ch, 0, BANK_PRESTAMO, current_time, INTERES_PRESTAMO);

		if ( cnt <= 0 )
		{
			send_to_char( "Huh?\n\r", ch );
			return;
		}

		if ( pres->valor + cnt > 100000 )
		{
			COMUNICAR( "Tu credito es de solo 100000 MO." );
			return;
		}

		pres->valor += cnt;
		cuenta->valor += cnt;
		sprintf( buf, "El Banco transfirio %d MO a tu cuenta.", cnt );
		COMUNICAR(buf);

		return;
	}

	if ( !str_prefix( arg1, "deposit" ) && !obj )
	{
		BANK_DATA *cuenta;
		int amount; 

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );

			if (amount > ch->gold )
			{
				sprintf( buf, "Como quieres depositar %d MO cuando solo tienes %ld?", amount, ch->gold );
				COMUNICAR(buf);
				return;
			}

			if (amount < 0 )
			{
				COMUNICAR ("Solo son permitidos valores positivos.");
				return;
			}

			if ( (cuenta->valor + amount) > MAX_PLATA )
			{
				COMUNICAR("No puedes tener tanto dinero en tu cuenta.");
				return;
			}

			ch->gold	-= amount;
			cuenta->valor	+= amount;
			sprintf ( buf, "Depositas %d MO.  Tu nuevo balance es %ld MO.\n\r",  
					amount, cuenta->valor );
			send_to_char( buf, ch );
			do_save( ch, "" );
			return;
		}
	}

	if ( !str_prefix( arg1, "transfer" ) )
	{
		int amount;
		CHAR_DATA *victim;
		BANK_DATA *cuenta, *cuenta2;

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );

			if ( amount > cuenta->valor )
			{
				sprintf( buf, "Como quieres transferir %d MO cuando tu balance es %ld?",
				amount, cuenta->valor );
				COMUNICAR(buf);
				return;
			}

                        if (amount < 0 )
                        {
                                COMUNICAR ("Solo valores positivos son permitidos.");
                                return;
                        }

			if ( !( victim = get_char_world( ch, argument ) ) )
			{
				sprintf (buf, "%s no tiene una cuenta en el banco.", argument );
				COMUNICAR(buf);
				return;
			}

			if (IS_NPC(victim))
			{
				COMUNICAR("Solo puedes transferir dinero a los jugadores.");
				return;
			}

			if ( (cuenta2 = cuenta_banco(victim)) == NULL )
			{
				sprintf(buf, "%s no tiene cuenta.", PERS(victim, ch) );
				COMUNICAR(buf);
				return;
			}

			if ( (cuenta2->valor + amount) > MAX_PLATA )
			{
				COMUNICAR("Esa cuenta no puede tener tanto dinero.");
				return;
			}

			cuenta->valor	-= amount;
 			cuenta2->valor	+= amount;
			sprintf( buf, "Transfieres %d MO. Tu nuevo balance es %ld MO.\n\r",
				amount, cuenta->valor );
			send_to_char( buf, ch );
			sprintf (buf, "[#BBANCO#b] %s transfirio %d MO a tu cuenta.\n\r", ch->name, amount);
			send_to_char( buf, victim );
			do_save( ch, "" );
			do_save( victim, "");
			return;
		}
	}

	if ( !str_prefix( arg1, "withdraw" ) )
	{
		int amount; 
		BANK_DATA *cuenta;

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );
			if ( amount > cuenta->valor )
			{
				sprintf( buf, "Como puedes retirar %d MO cuando tu balance es %ld?",
				amount, cuenta->valor );
				COMUNICAR (buf);
				return;
			}

                        if (amount < 0 )
                        {
                                COMUNICAR( "Solo valores positivos son permitidos.");
                                return;
                        }

			if ( get_carry_weight(ch) + gold_weight(amount) > can_carry_w(ch) )
			{
				COMUNICAR( "No puedes llevar tanto peso." );
				return;
			}

			cuenta->valor	-= amount;
			ch->gold	+= amount;
			sprintf( buf, "Retiras %d MO.  Tu nuevo balance es %ld MO.\n\r", amount, cuenta->valor );
			send_to_char( buf, ch );
			do_save( ch, "" );
			return;
 		}
	}

        if ( !str_prefix( arg1, "buy" ) && !obj )
        {
                int amount;
                BANK_DATA *cuenta;

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

                if ( is_number ( arg2 ) )
                {
			BANK_DATA *accion = get_bank(ch, BANK_ACCIONES);

                        amount = atoi( arg2 );
                        if ( (amount * share_value) > cuenta->valor )
			{
                                sprintf( buf, "%d acciones te costaran %d, junta mas dinero.", amount, (amount * share_value) );
                                COMUNICAR(buf);
                                return;
                        }

                        if (amount < 0 )
                        {
                                COMUNICAR("Si quieres vender acciones solo tienes que decirlo.");
                                return;
                        }

			if ( accion && (accion->valor + amount > MAX_PLATA) )
			{
				COMUNICAR("No puedes tener tantas acciones.");
				return;
			}

                        cuenta->valor -= (amount * share_value);

			if (accion)
				accion->valor += amount;
			else
				accion = bank_give(ch, amount, BANK_ACCIONES, current_time, 1);

			sprintf( buf, "Compras %d acciones por %d MO, ahora tienes %ld acciones.",
					amount, amount * share_value,
					accion->valor );
                        COMUNICAR(buf);
                        do_save( ch, "" );
                        return;
                }
        }

        if ( !str_prefix( arg1, "sell" ) && !obj )
        {
                int amount;
		BANK_DATA *cuenta;

		if ( (cuenta = cuenta_banco(ch)) == NULL )
		{
			COMUNICAR("Necesitas una cuenta para eso.");
			return;
		}

                if ( is_number ( arg2 ) )
                {
			BANK_DATA *accion = get_bank(ch, BANK_ACCIONES);

                        amount = atoi( arg2 );

			if ( !accion )
			{
				COMUNICAR( "No tienes dinero invertido en acciones." );
				return;
			}

                        if ( amount > accion->valor )
			{
                                sprintf( buf, "Solo tienes %ld acciones.", accion->valor );
                                COMUNICAR(buf);
                                return;
                        }

                        if (amount < 0 )
                        {
                                COMUNICAR ("Si quieres comprar acciones solo tienes que decirlo.");
                                return;
                        }

			if ( (cuenta->valor + amount*share_value) > MAX_PLATA )
			{
				COMUNICAR("No puedes tener tanto dinero.");
				return;
			}

                        cuenta->valor		+= (amount * share_value);
			accion->valor		-= amount;
                        sprintf( buf, "Vendes %d acciones por %d MO, ahora tienes %ld acciones.",
                        	amount, amount * share_value,
                        	accion->valor );
                        COMUNICAR (buf);
                        do_save( ch, "" );
                        return;
                }
        }

	COMUNICAR("No se de que hablas");
	do_bank( ch, "" );		/* Generate Instructions */
	return;
}
