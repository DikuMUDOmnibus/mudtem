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
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "plist.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_play		);
DECLARE_DO_FUN( do_info		);
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
char *	condicion_obj	args( ( int percent ) );

/* comm.c */
char *	condicion	( int, int );

char *	const	where_name	[] =
{
    "<como luz>       ",
    "<en el dedo>     ",
    "<en el dedo>     ",
    "<en el cuello>   ",
    "<en el cuello>   ",
    "<en el torso>    ",
    "<en la cabeza>   ",
    "<en las piernas> ",
    "<en los pies>    ",
    "<en las manos>   ",
    "<en los brazos>  ",
    "<como escudo>    ",
    "<en el cuerpo>   ",
    "<en la cintura>  ",
    "<en la muneca>   ",
    "<en la muneca>   ",
    "<esgrimido>      ",
    "<sosteniendolo>  ",
    "<flotando cerca> ",
    "<arma secundaria>",
    "<como lentes>    ",
    "<como aros>      "
};


/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )	strcat( buf, "#5(Invis)#n "     );

    if ( DETECTED(obj, DETECTED_EVIL)
    || (IS_AFFECTED(ch, AFF_DETECT_EVIL)
     && IS_OBJ_STAT(obj, ITEM_EVIL)) )		strcat( buf, "#1(Aura Roja)#n "  );	/* rojo */

    if ( IS_OBJ_STAT( obj, ITEM_HIDDEN) )	strcat( buf, "#6(#BEscondido#b)#n " );	/* cyan */
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))	      strcat(buf,"#4(#BAura Azul#b)#n "	);	/* azul */
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "#5(#BMagico#b)#n "   );	/* purpura */
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "#F(#BBrillante#b)#n "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "#2(#BHumming#b)#n "   );
    if ( IS_OBJ_STAT(obj, ITEM_FLAMING)	  )   strcat( buf, "#1(#B#FFlaming#b#f)#n ");	/* rojo */

    if ( ES_ARMA(obj) && DETECTED(obj, DETECTED_SHARP) )
						strcat( buf, "(#BSharp#b) " );
    if ( DETECTED(obj, DETECTED_CURSE) )
    						strcat( buf, "(Maldito) ");

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }

    if (strlen(buf)<=0)
        strcat(buf,"This object has no description. Please inform the IMP.");

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nada.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];
    char buf2[64];

    buf[0] = '\0';

    if ( IS_SET(victim->comm,COMM_AFK	  )   ) strcat( buf, "#6[#BAFK#b]#n "	     );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "#5(Invis)#n "      );
    if ( IS_AFFECTED2(victim, AFF_AMNESIA)    ) strcat( buf, "(#BNULL#b Aura) " );
    if ( IS_AFFECTED2(victim, AFF_FLAMING_SHIELD) ) strcat(buf, "#0#B(#nAura Negra#0#B)#n " );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "#1#B(Wizi)#n "	     );
    if ( !IS_NPC(victim) && !victim->desc     ) strcat( buf, "#0#B(Linkdead)#n ");
    if ( IS_AFFECTED(victim, AFF_HIDE)        )
    {
	strcpy( buf2, "#4#B(Escondid )#n " );
	buf2[13] = CHAR_SEXO(victim);
    	strcat( buf, buf2 );
    }
    if ( IS_AFFECTED(victim, AFF_CHARM)       )
    {
	strcpy( buf2, "#6(Encantad )#n " );
	buf2[11] = CHAR_SEXO(victim);
	strcat( buf, buf2 );
    }
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "#0#B(Translucent)#n ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "#5#B(Aura Rosada)#n "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "#1(Aura Roja)#n "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "#3(#BAura Dorada#b)#n ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "#B(Aura Blanca)#n " );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
    {
    	strcpy( buf2, "#1(ASESIN )#n " );
    	buf2[9] = CHAR_SEXO_UPPER(victim);
    	strcat( buf, buf2 );
    }
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
    {
    	strcpy( buf2, "#1(#BLADRON #b)#n " );
    	buf2[11] = (victim->sex == SEX_FEMALE ? 'A' : ' ');
    	strcat( buf, buf2 );
    }
    if ( es_quest_mob(ch, victim) )
	strcat( buf, "#B[#FVICTIMA#f]#n " );

/*  if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' ) */
    if ( victim->position == victim->default_pos && victim->long_descr[0] != '\0' )
    {
	strcat( buf, capitalizar(victim->long_descr) );
	send_to_char( buf, ch );
	return;
    }

    if ( !IS_AFFECTED2( ch, AFF_AMNESIA ) )
    {
	strcat( buf, capitalizar(PERS( victim, ch )) );

	if ( !IS_NPC(victim)
	&&   !IS_SET(ch->comm, COMM_BRIEF) 
	&&   victim->position == POS_STANDING
	&&   ch->on == NULL )
		strcat( buf, victim->pcdata->title );
    }
    else
    {
	if ( IS_NPC(victim) )
		strcat( buf, capitalizar(PERS( victim, ch )) );
	else
	{
	    	strcat( buf, "Un" );
	    	strcat( buf, victim->sex == SEX_FEMALE ? "a " : " " );
	    	strcat( buf, victim->sex == SEX_FEMALE ? race_table[victim->race].hembra : race_table[victim->race].macho );
	}
    }

    switch ( victim->position )
    {
    case POS_DEAD:
    	sprintf( buf2, " esta MUERT%c!!", CHAR_SEXO_UPPER(victim) );
    	strcat(buf, buf2);
    	break;
    case POS_MORTAL:
	sprintf( buf2, " esta mortalmente herid%c.", CHAR_SEXO(victim) );
	strcat(buf, buf2);
	break;
    case POS_INCAP:
	sprintf( buf2, " esta incapacitad%c.", CHAR_SEXO(victim) );
	strcat(buf, buf2);
	break;
    case POS_STUNNED:
        sprintf( buf2, " esta aturdid%c.", CHAR_SEXO(victim) );
        strcat( buf, buf2 );
        break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," esta durmiendo en %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," esta durmiendo sobre %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " esta durmiendo dentro de %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," esta durmiendo aqui.");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," esta descansando en %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," esta descansando encima de %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " esta descansando dentro de %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, " esta descansando aqui." );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," esta sentado en %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," esta sentado encima de %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " esta sentado dentro de %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, " esta sentado aqui.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," esta parado en %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," esta parado encima de %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," esta parado dentro de %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " esta aqui." );
	break;
    case POS_FIGHTING:
	strcat( buf, " esta aqui, peleando " );
	if ( victim->fighting == NULL )
	    strcat( buf, "con el aire??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "#BCONTIGO#b!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, "con ");
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "con alguien que se fue??" );
	break;
    }

    strcat( buf, "#n\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n se mira a si mism$o.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n te mira.", ch, NULL, chToEnt(victim), TO_VICT    );
	    act( "$n mira a $N.",  ch, NULL, chToEnt(victim), TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "No ves nada especial.", ch, NULL, NULL, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );
    strcat( buf, " " );
    strcat( buf, condicion(percent, victim->sex) );
    strcat( buf, "\n\r" );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N esta usando:", ch, NULL, chToEnt(victim), TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "\n\rMiras su inventario:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room ) )
	{
		if ( IS_AFFECTED(rch, AFF_INFRARED ) )
		{
			send_to_char( "Ves grandes ojos rojos APUNTANDOTE!\n\r", ch );
		}
		else
		if ( IS_AFFECTED(ch, AFF_INFRARED)
		&&  !IS_FORM(rch, FORM_COLD_BLOOD) )
		{
			send_to_char( "Ves una forma indefinida cerca tuyo.\n\r", ch );
		}
	}
    }

    return;
}

bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "No puedes ver nada!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    if ( IS_NPC(ch) )
    {
    	send_to_char( "No en NPC's.\n\r", ch );
    	return;
    }

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->pcdata->lines == 0)
	    send_to_char("No vas a pausar el texto.\n\r",ch);
	else
	{
	    sprintf(buf,"Ahora ves %d lineas por pagina.\n\r",
		    ch->pcdata->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("Debes entregar un numero.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Pausas desactivadas.\n\r",ch);
        ch->pcdata->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("Debes poner un numero razonable.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll puesto en %d lineas.\n\r",lines);
    send_to_char(buf,ch);
    ch->pcdata->lines = lines - 2;

    if (ch->desc->screenmap || ch->desc->oldscreenmap)
    {
    	free(ch->desc->screenmap);
    	free(ch->desc->oldscreenmap);
    	ch->desc->screenmap = ch->desc->oldscreenmap = NULL;
    }

    if ( IS_SET(ch->comm, COMM_OLCX) )
	InitScreen(ch->desc);
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch); 

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("compact mode   ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("Tu cuerpo esta a salvo de los ladrones.\n\r",ch);
    else 
        send_to_char("Tu cuerpo puede ser saqueado.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("No puedes ser summoneado.\n\r",ch);
    else
	send_to_char("Puedes ser summoneado.\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("No aceptas seguidores.\n\r",ch);
    else
	send_to_char("Aceptas seguidores.\n\r",ch);

    send_to_char("mudFTP         ",ch);
    if (IS_SET(ch->act,PLR_MUDFTP))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist desactivado.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("Ahora asistiras cuando te necesiten.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Las salidas no seran mostradas.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Las salidas seran mostradas.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold desactivado.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Saqueo automatico del oro de los cuerpos activado.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autoloot desactivado.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Saqueo automatico de los cuerpos activado.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrifice desactivado.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Sacrificio automatico de los cuerpos activado.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplit desactivado.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Division de el dinero entre el grupo activado.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Descripciones completas activadas.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Descripciones cortas activadas.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Modo compacto desactivado.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Modo compacto activado.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects no seran mostrados en el score.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("Affects seran mostrados en el score.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( IS_NPC(ch) )
   {
   	send_to_char( "No en NPCs.\n\r", ch );
   	return;
   }

   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
      	    send_to_char("No veras el prompt.\n\r",ch);
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
      	    send_to_char("Ahora ves el prompt.\n\r",ch);
      	    SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, PROMPT_ALL );
   else
   if( !strcmp( argument, "full" ) )
      strcpy( buf, PROMPT_FULL );
   else
   if ( !str_cmp( argument, "full2" ) )
   	strcpy( buf, PROMPT_FULL2 );
   else
   if( !strcmp( argument, "warrior") )
      strcpy( buf, PROMPT_WARRIOR );
   else
   if ( !strcmp( argument, "olc") )
      strcpy( buf, PROMPT_OLC );
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      if (str_suffix("%c",buf))
	strcat(buf," ");
   }
 
   free_string( ch->pcdata->prompt );
   ch->pcdata->prompt = str_dup( buf );
   sprintf(buf,"Prompt es ahora %s\n\r",ch->pcdata->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Inventario largo seleccionado.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Inventario combinado seleccionado.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Tu cuerpo esta a salvo de los ladrones.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Tu cuerpo puede ser saqueado.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("Aceptas seguidores.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("No aceptas seguidores.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch, FALSE );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("No eres inmune al summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("Ahora eres inmune al summon.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("No eres inmune al summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("Ahora eres inmune al summon.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "No ves nada excepto estrellas!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "No puedes ver nada, estas durmiendo!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "Esta un poco oscuro ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( ch->in_room->name, ch );

	if ((IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
	|| (!IS_NPC(ch) && IS_BUILDER(ch,ch->in_room->area)))
	{
	    sprintf(buf," [Room %d]",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "\n\r", ch );

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_exits( ch, "auto" );
	}

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "#2  ",ch);
	    send_to_char( ch->in_room->description, ch );
	}

	send_to_char("#6",ch); /* cyan */
	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	send_to_char("#n",ch); /* gris */
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Mirar en que?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No ves eso aqui.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "Eso no es un contenedor.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "Esta vacio.\n\r", ch );
		break;
	    }

	    sprintf( buf, "Esta lleno hasta %s de un liquido de color %s.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "menos de la mitad" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "la mitad"     : "mas de la mitad",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "Esta cerrado.\n\r", ch );
		break;
	    }

	    act( "$p tiene:", ch, objToEnt(obj), NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
	    	if (++count == number)
	    	{
			send_to_char( pdesc, ch );
			return;
		}
		else
			continue;
	    }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
 	    {
 		if (++count == number)
 		{	
			send_to_char( pdesc, ch );
			return;
		}
		else
			continue;
	    }

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"Solo ves un %s aqui.\n\r",arg3);
    	else
    	    sprintf(buf,"Solo ves %d de esos aqui.\n\r",count);
    	
    	send_to_char(buf,ch);
    	return;
    }

    if ( ( door = find_exit( ch, arg1 ) ) == -1 )
    	return;

    /* 'look direction' */
    if ( ( pexit = exit_lookup(ch->in_room, door) ) == NULL )
    {
	send_to_char( "Nada especial alli.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nada especial alli.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "El $d esta cerrado.", ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "El $d esta abierto.",   ch, NULL, strToEnt(pexit->keyword,ch->in_room), TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examinar que?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;
	
	case ITEM_JUKEBOX:
	    do_play(ch,"list");
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Raro...no hay monedas en la pila.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. Una moneda de oro.\n\r");
		else
		    sprintf(buf,"Hay %d monedas de oro en la pila.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. Una moneda de plata.\n\r");
		else
		    sprintf(buf,"Hay %d monedas de plata en la pila.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "Hay %d monedas de oro y %d de plata en la pila.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_look( ch, buf );
	}
	sprintf( buf, "%s esta en ", obj->short_descr );
	send_to_char( buf, ch );
	if ( obj->condition > 89 )
		send_to_char( "excelente condicion.\n\r", ch );
	else
	if ( obj->condition > 75 )
		send_to_char( "buena condicion.\n\r", ch );
	else
	if ( obj->condition > 50 )
		send_to_char( "regular condicion.\n\r", ch );
	else
	if ( obj->condition > 25 )
		send_to_char( "malas condiciones.\n\r", ch );
	else
		send_to_char( "pesima condicion.\n\r", ch );
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_nom[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto)
	sprintf(buf,"#0#B[#b#7Salidas:#B");
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"Salidas obvias del cuarto %d:\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"Salidas obvias:\n\r");

    found = FALSE;

    for ( pexit = ch->in_room->exits; pexit; pexit = pexit->next )
    {
	if ( pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room) 
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_nom[pexit->direccion] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_nom[pexit->direccion] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Demasiado oscuro"
			: pexit->u1.to_room->name
		    );
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( HAS_ROOM_TRIGGER( ch->in_room, RTRIG_EXCOMM ) )
    {
	MPROG_LIST *prg;

	for ( prg = ch->in_room->mprogs; prg != NULL; prg = prg->next )
	{
		if ( prg->trig_type == RTRIG_EXCOMM )
		{
			found = TRUE;
			if ( fAuto )
			{
				strcat( buf, " " );
				strcat( buf, prg->trig_phrase );
			}
			else
				sprintf( buf + strlen(buf), "%-5s -\n\r",
					prg->trig_phrase );
		}
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " ninguna" : "Ninguna.\n\r" );

    if ( fAuto )
	strcat( buf, "#0#B]#7#b\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Tienes %ld monedas de oro y %ld de plata.\n\r",
	ch->gold, ch->silver );
    send_to_char(buf,ch);

    if (IS_NPC(ch))
    	return;

    sprintf( buf, "Tienes #B%d#b puntos de experiencia.\n\r",
    		ch->exp );
    send_to_char( buf, ch );

    sprintf( buf, "Necesitas #B%d#b puntos de experiencia para subir de nivel.\n\r",
	(getNivelPr(ch) + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp );
    send_to_char(buf,ch);

    return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i;
    char * string = NULL;

    sprintf( buf,
	"Tu eres %s%s, nivel %d, %d anos (%d horas).\n\r",
	chcolor(ch),
	IS_NPC(ch) ? "" : ch->pcdata->title,
	getNivelPr(ch), get_age(ch),
        ( ch->played + (int) (current_time - ch->logon) ) / 3600);
    send_to_char( buf, ch );

    if ( get_trust( ch ) != getNivelPr(ch) )
    {
	sprintf( buf, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );
	send_to_char( buf, ch );
    }

    sprintf(buf, "Raza: %s%s  Sexo: %s  Clase: %s\n\r",
	race_table[ch->race].name,
	(ch->true_race != ch->race) ? parentesis(race_table[ch->true_race].name) : "",
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "masculino" : "femenino",
 	IS_NPC(ch) ? "mobil" : class_table[getClasePr(ch)].name);
    send_to_char(buf,ch);
	
    sprintf( buf,
	"Tienes #1#B%d#n/#<%d#n hit, #<%d#n/#<%d#n mana, #<%d#n/#<%d#n movimiento.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move);
    send_to_char( buf, ch );

    sprintf( buf,
	"Tienes #6%d#n practicas y #6%d#n sesiones de entrenamiento.\n\r",
	ch->practice, ch->train);
    send_to_char( buf, ch );

    sprintf( buf,
	"Estas llevando #2%d#n/#2%d#n items con #2%ld#n/#2%d#n libras de peso.\n\r",
	ch->carry_number, can_carry_n(ch),
	get_carry_weight(ch) / 10, can_carry_w(ch) /10 );
    send_to_char( buf, ch );

    sprintf( buf,
	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	ch->perm_stat[STAT_STR],
	get_curr_stat(ch,STAT_STR),
	ch->perm_stat[STAT_INT],
	get_curr_stat(ch,STAT_INT),
	ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS),
	ch->perm_stat[STAT_DEX],
	get_curr_stat(ch,STAT_DEX),
	ch->perm_stat[STAT_CON],
	get_curr_stat(ch,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf,
	"Tienes #B%d#b puntos de exp, y tienes %ld monedas de oro y %ld de plata.\n\r",
	ch->exp,  ch->gold, ch->silver );
    send_to_char( buf, ch );

    if ( !IS_NPC(ch) )
    {
	BANK_DATA *acciones = get_bank( ch, BANK_ACCIONES );

	if ( acciones )
	{
		sprintf(buf,"Tienes %ld monedas de oro invertidas en %ld acciones (%d c/u).\n\r",
			acciones->valor * share_value,
			acciones->valor,  share_value );
		send_to_char( buf, ch );
	}
    }

    /* RT shows exp to level */
    if (!IS_NPC(ch) && getNivelPr(ch) < LEVEL_HERO)
    {
      sprintf (buf, 
	"Necesitas %d exp para subir de nivel.\n\r",
	((getNivelPr(ch) + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
      send_to_char( buf, ch );
    }

    sprintf( buf, "Wimpy esta en %d hit points.\n\r", ch->wimpy );
    send_to_char( buf, ch );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	act( "Estas ebri$o. *HIC*", ch, NULL, NULL, TO_CHAR );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    	act( "Estas sedient$o.", ch, NULL, NULL, TO_CHAR );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER]   ==  0 )
    	act( "Estas hambrient$o.", ch, NULL, NULL, TO_CHAR );

    switch ( ch->position )
    {
    case POS_DEAD:     
	string = "Estas #BMUERT$k#b!!";
	break;
    case POS_MORTAL:
	string = "Estas mortalment herid$o.";
	break;
    case POS_INCAP:
	string = "Estas incapacitad$o.";
	break;
    case POS_STUNNED:
	string = "Estas desmayad$o.";
	break;
    case POS_SLEEPING:
	string = "Estas durmiendo.";
	break;
    case POS_RESTING:
	string = "Estas descansando.";
	break;
    case POS_SITTING:
	string = "Estas sentad$o.";
	break;
    case POS_STANDING:
	string = "Estas de pie.";
	break;
    case POS_FIGHTING:
	string = "Estas luchando.";
	break;
    }

    act(string,ch,NULL,NULL,TO_CHAR);

    /* print AC values */
    if (getNivelPr(ch) >= 25)
    {	
	sprintf( buf,"Armadura: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
		 GET_AC(ch,AC_PIERCE),
		 GET_AC(ch,AC_BASH),
		 GET_AC(ch,AC_SLASH),
		 GET_AC(ch,AC_EXOTIC));
    	send_to_char(buf,ch);
    }

    for (i = 0; i < 4; i++)
    {
	char * temp;

	switch(i)
	{
	    case(AC_PIERCE):	temp = "piercing";	break;
	    case(AC_BASH):	temp = "bashing";	break;
	    case(AC_SLASH):	temp = "slashing";	break;
	    case(AC_EXOTIC):	temp = "magic";		break;
	    default:		temp = "error";		break;
	}
	
	send_to_char("Estas ", ch);

	if      (GET_AC(ch,i) >=  101 ) 
	    sprintf(buf,"irremediablemente vulnerable a %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 80) 
	    sprintf(buf,"desprotegido de %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 60)
	    sprintf(buf,"poco protegido de %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 40)
	    sprintf(buf,"ligeramente protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 20)
	    sprintf(buf,"casi protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 0)
	    sprintf(buf,"protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -20)
	    sprintf(buf,"bien protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -40)
	    sprintf(buf,"muy bien protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -60)
	    sprintf(buf,"fuertemente protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -80)
	    sprintf(buf,"super protegido contra %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -100)
	    sprintf(buf,"casi invulnerable a %s.\n\r",temp);
	else
	    sprintf(buf,"divinamente protegido contra %s.\n\r",temp);

	send_to_char(buf,ch);
    }

    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(ch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
        send_to_char("on",ch);
      else
        send_to_char("off",ch);
 
      if (ch->invis_level)
      {
        sprintf( buf, "  Invisible: nivel %d",ch->invis_level);
        send_to_char(buf,ch);
      }

      if (ch->incog_level)
      {
	sprintf(buf,"  Incognito: nivel %d",ch->incog_level);
	send_to_char(buf,ch);
      }
      send_to_char("\n\r",ch);
    }

    if ( IS_IMMORTAL(ch) && getNivelPr(ch) >= 15 )
    {
	sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );
	send_to_char( buf, ch );
    }
    
    if ( getNivelPr(ch) >= 10 )
    {
	sprintf( buf, "Alineacion: %d.  ", ch->alignment );
	send_to_char( buf, ch );
    }

    send_to_char( "Eres ", ch );
         if ( ch->alignment >  900 ) send_to_char( "angelical.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "santo.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "bueno.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "casi bueno.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "malo.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "perverso.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demoniaco.\n\r", ch );
    else                             send_to_char( "satanico.\n\r", ch );

    if (!IS_NPC(ch))
    {
	sprintf ( buf, "Has derrotado a %d enemigos.\n\r", ch->pcdata->muertes );
	send_to_char(buf,ch);
    }

    if ( IS_IMMORTAL(ch) )
    {
	sprintf(buf,"Regeneracion : %d hp - %d mana.\n\r", hit_gain(ch), mana_gain(ch));
	send_to_char(buf,ch);
    }

    if ( !IS_NPC(ch) )
    {
    	MEM_DATA *mem;

	mem = mem_lookup(ch->memory, MEM_VAMPIRE);

	if ( mem )
	{
		PLIST *pl = plist_lookup_id(mem->id);

		if ( pl )
			printf_to_char( ch, "%s es el culpable de que seas vampiro.\n\r",
				pl->name );
		else
			printf_to_char( ch, "El vampiro que te mordio no esta en la lista de jugadores.\n\r"
					    "Habla con el IMP.\n\r" );
	}
    }

    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
	do_affects(ch,"");
}

void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];
    
    if ( ch->affected != NULL )
    {
	send_to_char( "Estas afectado por los siguientes spells:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if (paf->type < 0 || paf->type >= MAX_SKILL)
	    {
		send_to_char( "#B#UAlgo raro pasa. Comunicate con el IMP.#b#u\n\r", ch );
	    	continue;
	    }

	    if (paf_last != NULL && paf->type == paf_last->type)
	    {
		if (getNivelPr(ch) >= 20)
		    sprintf( buf, "                      ");
		else
		    continue;
	    }
	    else
	    	sprintf( buf, "Spell: %-15s", NOMBRE_SKILL(paf->type) );

	    send_to_char( buf, ch );

	    if ( getNivelPr(ch) >= 20 )
	    {
		sprintf( buf,
		    ": modifica %s por %d ",
		    affect_loc_name( paf->location ),
		    paf->modifier);
		send_to_char( buf, ch );
		if ( paf->duration == -1 )
		    sprintf( buf, "permanentemente" );
		else
		    sprintf( buf, "por %d horas", paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( "\n\r", ch );
	    paf_last = paf;
	}
    }
    else 
	send_to_char("No estas afectado por ningun spell.\n\r",ch);

    return;
}



char *	const	day_name	[] =
{
    "la Luna", "el Toro", "Decepcion", "el Trueno", "la Libertad",
    "los Grandes Dioses", "el Sol"
};

char *	const	month_name	[] =
{
    "Invierno", "el Lobo Invernal", "el Gigante Helado", "las Fuerzas Antiguas",
    "la Gran Batalla", "la Primavera", "la Naturaleza", "Futilidad", "el Dragon",
    "el Sol", "el Calor", "la Guerra", "las Sombras Oscuras", "las Sombras",
    "las Grandes Sombras", "la Oscuridad", "el Gran Demonio"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    int day;

    day     = time_info.day + 1;

    if ( IS_IMMORTAL(ch) && !IS_NULLSTR(argument) )
    {
	argument = one_argument(argument, buf);

    	if ( !is_number(buf) || atoi(buf) < 0 || atoi(buf) > 23 )
    	{
    		send_to_char( "Debes entrar un numero.\n\r", ch );
    		return;
    	}

	time_info.hour = atoi(buf); // 0..34 dias, 0..23 horas

	if ( is_number(argument) && atoi(argument) >= 0 && atoi(argument) <= 34)
		time_info.day = atoi(argument);

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    sprintf( buf,
	"Son las %d de la %s, Dia de %s, dia %d de el Mes de %s.\n\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? (time_info.hour > 19 ? "noche" : "tarde") : "manana",
	day_name[day % 7],
	day, /* suf, */
	month_name[time_info.month]);
    send_to_char(buf, ch);

    sprintf( buf, "#BROM#b empezo en %s.\n\r", str_boot_time );
    send_to_char(buf, ch);
    
    strftime( buf, MAX_STRING_LENGTH,
     "La hora del sistema es %H:%M:%S, %A %d de %B de %Y.\n\r", localtime(&current_time) );
    send_to_char( buf, ch );
    
    if ( IS_IMMORTAL(ch) || IS_BUILDER(ch, ch->in_room->area) )
    {
	sprintf( buf, "time_info.day = %d, time_info.hour = %d.\n\r",
		time_info.day, time_info.hour );
	send_to_char( buf, ch );
    }


    return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"despejado",
	"nuboso",
	"lluvioso",
	"iluminado por los relampagos"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "No puedes ver el tiempo dentro de un edificio.\n\r", ch );
	return;
    }

    sprintf( buf, "El cielo esta %s y %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "una tibia brisa del sur sopla"
	: "una fria brisa del norte sopla"
	);
    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int level;

    output = new_buf();

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
    	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

	if (level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    /* add seperator if found */
	    if (found)
		add_buf(output,
    "\n\r============================================================\n\r\n\r");
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		add_buf(output,pHelp->keyword);
		add_buf(output,"\n\r");
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		add_buf(output,pHelp->text+1);
	    else
		add_buf(output,pHelp->text);
	    found = TRUE;
	    /* small hack :) */
	    if (ch->desc != NULL && ch->desc->connected != CON_PLAYING)
		break;
	}
    }

    if (!found)
    	send_to_char( "No hay ayuda para ese tema.\n\r", ch );
    else
	page_to_char(buf_string(output),ch);
    free_buf(output);
}

/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("Debes poner un nombre.\n\r",ch);
	return;
    }

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;
	    
	    /* work out the printing */
	    class = class_table[getClasePr(wch)].who_name;
	    switch(getNivelPr(wch))
	    {
		case MAX_LEVEL - 0 : class = "IMP"; 	break;
		case MAX_LEVEL - 1 : class = "CRE";	break;
		case MAX_LEVEL - 2 : class = "SUP";	break;
		case MAX_LEVEL - 3 : class = "DEI";	break;
		case MAX_LEVEL - 4 : class = "GOD";	break;
		case MAX_LEVEL - 5 : class = "IMM";	break;
		case MAX_LEVEL - 6 : class = "DEM";	break;
		case MAX_LEVEL - 7 : class = "ANG";	break;
		case MAX_LEVEL - 8 : class = "AVA";	break;
	    }
    
	    /* a little formatting */
	    sprintf(buf, "[%2d %6s %s] %s%s%s%s%s%s%s%s\n\r",
		getNivelPr(wch),
		race_table[wch->race].who_name,
		class,
	     wch->incog_level >= LEVEL_HERO ? "(Incog) ": "",
 	     wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	     get_clan_table(wch->clan)->who_name,
	     IS_SET(wch->comm, COMM_AFK) ? "#6[#BAFK#b]#n " : "",
             IS_SET(wch->act,PLR_KILLER) ? "#3(#BKILLER#b)#n " : "",
             IS_SET(wch->act,PLR_THIEF) ? "#1(#BTHIEF#b)#n " : "",
		wch->name, IS_NPC(wch) ? "" : wch->pcdata->title);
	    add_buf(output,buf);
	}
    }

    if (!found)
    {
	send_to_char("Nadie con ese nivel esta jugando.\n\r",ch);
	return;
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
#if !defined(WIN32)
    bool rgfRace[maxrace];
#else
	bool * rgfRace;
#endif
    bool rgfClan[UPPER_MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
 
	/*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
#if defined(WIN32)
	rgfRace	= mud_calloc(sizeof(bool),maxrace);
#endif
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < maxrace; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < UPPER_MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
 
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
 
        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Solo dos numeros son permitidos.\n\r", ch );
#if defined(WIN32)
				free(rgfRace);
#endif
                return;
            }
        }
        else
        {
 
            /*
             * Look for classes to turn on.
             */
            if (!str_prefix(arg,"immortals"))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup(arg);
                if (iClass == -1)
                {
                    iRace = race_lookup(arg);
 
                    if (iRace == 0 || iRace >= maxrace)
		    {
			if (!str_prefix(arg,"clan"))
			    fClan = TRUE;
			else
		        {
			    iClan = clan_lookup(arg);
			    if (iClan)
			    {
				fClanRestrict = TRUE;
			   	rgfClan[iClan] = TRUE;
			    }
			    else
			    {
                        	send_to_char(
                            	"Ese no es una raza, clase, o clan validos.\n\r",
				   ch);
                            	return;
#if defined(WIN32)
							free(rgfRace);
#endif
			    }
                        }
		    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }
 
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

        if ( getNivelPr(wch) < iLevelLower
        ||   getNivelPr(wch) > iLevelUpper
        || ( fImmortalOnly  && getNivelPr(wch) < LEVEL_IMMORTAL )
        || ( fClassRestrict && !rgfClass[getClasePr(wch)] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
            continue;
 
        nMatch++;
 
        /*
         * Figure out what to print for class.
	 */
	class = class_table[getClasePr(wch)].who_name;
	switch ( getNivelPr(wch) )
	{
	default: break;
            {
                case MAX_LEVEL - 0 : class = "IMP";     break;
                case MAX_LEVEL - 1 : class = "CRE";     break;
                case MAX_LEVEL - 2 : class = "SUP";     break;
                case MAX_LEVEL - 3 : class = "DEI";     break;
                case MAX_LEVEL - 4 : class = "GOD";     break;
                case MAX_LEVEL - 5 : class = "IMM";     break;
                case MAX_LEVEL - 6 : class = "DEM";     break;
                case MAX_LEVEL - 7 : class = "ANG";     break;
                case MAX_LEVEL - 8 : class = "AVA";     break;
            }
	}

	/*
	 * Format it up.
	 */
	sprintf( buf, "%s%s%s%s%s%s%s%s#n\n\r",
	    wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	    wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	    is_clan(wch) ? get_clan_table(wch->clan)->who_name : "",
	    IS_SET(wch->comm, COMM_AFK) ? "#6[#BAFK#b]#n " : "",
            IS_SET(wch->act, PLR_KILLER) ? "#3(#BKILLER#b)#n " : "",
            IS_SET(wch->act, PLR_THIEF)  ? "#1(#BTHIEF#b)#n "  : "",
	    chcolor(wch),
	    IS_NPC(wch) ? "" : wch->pcdata->title );

        if ( !str_cmp(wch->pcdata->who_text,"@") || !str_cmp(wch->pcdata->who_text,""))
                sprintf( buf2, "[%2d %6s %s] ",
                    getNivelPr(wch),
          	    race_table[wch->race].who_name,
                    class);
        else
	{
                if (strlen_color(wch->pcdata->who_text) != (signed) strlen(wch->pcdata->who_text))
		{
			int i = 13 - strlen_color(wch->pcdata->who_text);
			sprintf(buf2, "[");
			while (i-- > 0)
				strcat(buf2," ");
			strcat(buf2,wch->pcdata->who_text);
			strcat(buf2,"] ");
		}
		else
			sprintf(buf2, "[%13s] ", wch->pcdata->who_text);
	}

        strcat(buf2,buf);
	add_buf(output,buf2);
    }

    sprintf( buf2, "\n\rJugadores encontrados: %d\n\r", nMatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
#if defined(WIN32)
	free(rgfRace);
#endif
    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"Hay %d jugadores activos, el maximo de hoy.\n\r",
	    count);
    else
	sprintf(buf,"Hay %d jugadores activos, el maximo de hoy fue %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Estas llevando:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear, i;
    bool found;
    char buf[MSL], buf2[MSL], buf3[MIL], buf4[MIL];

    send_to_char( "Estas usando:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    buf[0] = '\0';
	    buf2[0] = '\0';
	    buf3[0] = '\0';
	    buf4[0] = '\0';
	    strcat(buf2, format_obj_to_char(obj,ch,TRUE));
	    if (IS_SET(ch->comm,COMM_CONDICION))
	    {
		    while (strlen_color(buf2) < 41)
		    	strcat(buf2," ");
		    i = 0;
		    while( buf2[i] )
		    {
		    	sprintf( buf3, "%c", buf2[i] );
		    	strcat( buf4, buf3 );
		    	if ( strlen_color(buf4) > 40 )
		    		break;
			i++;
		    }
		    strcat( buf, " " );
		    strcat( buf, buf4 );
		    strcat( buf, " #B" );
		    strcat( buf, condicion_obj(obj->condition) );
		    strcat( buf, "#b\n\r" );
	    }
	    else
	    	sprintf( buf, " %s\n\r", buf2 );
	    send_to_char( buf, ch );
	}
	else
	{
	    send_to_char( "algo.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nada.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Comparar que con que?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "No tienes ese item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("No estas usando nada comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
    {
	send_to_char("No tienes ese item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "Comparas $p consigo mismo.  Se ven parecidos.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "No puedes comparar $p y $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "No puedes comparar $p y $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p y $P se ven parecidos.";
	else if ( value1  > value2 ) msg = "$p se ve mejor que $P.";
	else                         msg = "$p se ve peor que $P.";
    }

    act( msg, ch, objToEnt(obj1), objToEnt(obj2), TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Jugadores cerca tuyo:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
 	    &&   (is_room_owner(ch,victim->in_room) 
	    ||    !room_is_private(victim->in_room))
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Ninguno\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "No encuentras ningun $T.", ch, NULL, strToEnt(arg,ch->in_room), TO_CHAR );
    }

    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      *buf                      = '\0';
    char      *msg;
    char       arg [ MAX_INPUT_LENGTH ];
    int        diff;
    int        hpdiff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Considerar a quien?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim) )
    {
    	send_to_char( "Ni siquiera lo pienses.\n\r", ch );
    	return;
    }

    diff = getNivelPr(victim) - getNivelPr(ch);

         if ( diff <= -10 ) msg = "Puedes matar a $N desnudo y desarmado.";
    else if ( diff <=  -5 ) msg = "$N no es rival para ti.";
    else if ( diff <=  -2 ) msg = "$N parece ser un adversario facil.";
    else if ( diff <=   1 ) msg = "El adversario perfecto!";
    else if ( diff <=   4 ) msg = "$N dice 'Te sientes suertudo, #BP#bunk?'.";
    else if ( diff <=   9 ) msg = "$N se rie de ti sin piedad.";
    else                    msg = "La Muerte te agradecera por tu regalo.";

    act( msg, ch, NULL, chToEnt(victim), TO_CHAR );

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = ( ch->hit - victim->hit );

    if ( ( ( diff >= 0) && ( hpdiff <= 0 ) )
	|| ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
    {
        send_to_char( "Tambien,", ch );
    }
    else
    {
        send_to_char( "Lamentablemente,", ch );
    }

    if ( hpdiff >= 101 )
        buf = " estas en mucho mejor estado que $E.";
    if ( hpdiff <= 100 )
        buf = " estas en mejor estado que $E.";
    if ( hpdiff <= 50 ) 
        buf = " estas un poco mejor que $E.";
    if ( hpdiff <= 25 )
        buf = " estas ligeramente mejor que $E.";
    if ( hpdiff <= 0 )
        buf = " $E esta ligeramente mejor que tu.";
    if ( hpdiff <= -25 )
        buf = " $E esta un poco mejor que tu.";
    if ( hpdiff <= -50 )
        buf = " $E esta en mejor estado que tu.";
    if ( hpdiff <= -100 )
        buf = " $E esta en mucho mejor estado que tu.";

    act( buf, ch, NULL, chToEnt(victim), TO_CHAR );
    return;
}

void do_evaluar( CHAR_DATA *ch, char *argument )
 {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int dif;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Evaluar a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
    	send_to_char( "Evaluarte a ti mismo? Heh.\n\r", ch );
    	return;
    }

    if ( get_skill(ch,gsn_evaluar) == 0 )
    {
    	send_to_char( "Evaluar? Que es eso?\n\r", ch);
    	return;
    }

    WAIT_STATE( ch, skill_table[gsn_evaluar].beats );

    if (number_percent() < get_skill(ch,gsn_evaluar))
    {
	sprintf(buf,"%s es nivel %d.\n\r",capitalizar(PERS(victim,ch)),getNivelPr(victim));
	send_to_char(buf,ch);
	sprintf(buf,"Tiene #1#B%d#n/#<%d#n hp y #<%d#n/#<%d#n mana.\n\r",victim->hit,
		victim->max_hit,victim->mana,victim->max_mana);
	send_to_char(buf,ch);

	if (get_skill(ch,gsn_evaluar) > 80)
	{
		dif = get_curr_stat(ch,STAT_STR) - get_curr_stat(victim,STAT_STR);
		if (dif > 2)
			sprintf(buf,"Eres mucho mas fuerte que %s. No sera problema.\n\r",PERS(victim,ch));
		else if (dif == 2)
			sprintf(buf,"Eres mas fuerte que %s.\n\r",PERS(victim,ch));
		else if (dif == 1)
			sprintf(buf,"Eres un poco mas fuerte que %s.\n\r",PERS(victim,ch));
		else if (dif == 0)
		        sprintf(buf,"%s es tan fuerte como tu.\n\r",capitalizar(PERS(victim,ch)));
		else if (dif == -1)
		    	sprintf(buf,"%s es un poco mas fuerte que tu.\n\r",capitalizar(PERS(victim,ch)));
		else if (dif == -2)
		        sprintf(buf,"%s es mas fuerte que tu.\n\r",capitalizar(PERS(victim,ch)));
		else if (dif < -2)
		        sprintf(buf,"Ni siquiera lo pienses.\n\r");
		send_to_char(buf,ch);
	}
	check_improve(ch,gsn_evaluar,TRUE,1);
    }
    else
    {
	sprintf(buf,"Intentas evaluar a %s,pero fracasas miserablemente.\n\r",
		PERS(victim,ch));
	send_to_char(buf,ch);
        act("$n intenta evaluar a $N, pero fracasa miserablemente.",
        	ch,NULL,chToEnt(victim),TO_NOTVICT);
	act("$n trato de evaluarte.",ch,NULL,chToEnt(victim),TO_VICT);
       	check_improve(ch,gsn_evaluar,FALSE,1);
    }     

    return;
}

void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Cambiar tu titulo a que?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_description( CHAR_DATA *ch, char *argument )
{
	if ( IS_NPC(ch) || !ch->desc )
		return;

	string_append( ch, &ch->description );
	return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"Dices 'Tengo %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n dice 'Tengo %d/%d hp %d/%d mana %d/%d mv %d xp.'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
		if ( skill_table[sn].name == NULL )
			break;

		if ( !can_prac(ch, sn) )
			continue;

		sprintf(buf, "%s",
		skill_table[sn].spell_fun == spell_null ? "#B*#b" : " " );
		send_to_char(buf, ch);

		if (IS_AFFECTED2(ch, AFF_AMNESIA))
			sprintf( buf, "%-18s ???%%  ", NOMBRE_SKILL(sn) );
		else
			sprintf( buf, "%-18s %3d%%  ",
				NOMBRE_SKILL(sn),
				ch->pcdata->learned[sn] );
		send_to_char( buf, ch );

		if ( ++col % 3 == 0 )
			send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

	sprintf( buf, "Tienes %d sesiones de practica.\n\r",
	    ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept, rating;
	SpellDesc spell;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "En tus suenos, o que?\n\r", ch );
	    return;
	}

	if ( IS_NPC(ch) )
		return;

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "No puedes hacer eso aqui.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "No te quedan sesiones de practica.\n\r", ch );
	    return;
	}

	find_spell(&spell, ch, argument);

	if (  spell.sn < 0
	||   !can_prac( ch, spell.sn ) )
	{
		send_to_char( "No puedes practicar eso.\n\r", ch );
		return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[spell.clase].skill_adept;

	rating = skill_table[spell.sn].rating[spell.clase];

	if ( rating == 0 )
	{
		if ( es_skill_racial( ch->race, spell.sn ) )
			rating = 2;
		else
		{
			bugf( "do_practice : skill %d(%s) con rating 0 en clase %s",
				spell.sn, skill_table[spell.sn].name,
				class_table[spell.clase].name );
			send_to_char( "Uhmmm...hay un error. Dile al IMP.\n\r", ch );
			return;
		}
	}

	if ( ch->pcdata->learned[spell.sn] >= adept )
	{
	    sprintf( buf, "Ya eres un experto de %s.\n\r",
		NOMBRE_SKILL(spell.sn) );
	    send_to_char( buf, ch );
	}
	else
	{
	    int porcent = int_app[get_curr_stat(ch,STAT_INT)].learn / rating;

	    ch->practice--;
	    ch->pcdata->learned[spell.sn] += porcent;

	    if ( ch->pcdata->learned[spell.sn] < adept )
	    {
		printf_to_char( ch, "Practicas %s. Tu habilidad subio en #B%d%%#b.\n\r",
			NOMBRE_SKILL(spell.sn), porcent );
		act( "$n practica $T.",
		    ch, NULL, strToEnt(NOMBRE_SKILL(spell.sn),ch->in_room), TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[spell.sn] = adept;
		act( "Ahora eres experto de $T.",
		    ch, NULL, strToEnt(NOMBRE_SKILL(spell.sn),ch->in_room), TO_CHAR );
		act( "$n es ahora experto en $T.",
		    ch, NULL, strToEnt(NOMBRE_SKILL(spell.sn),ch->in_room), TO_ROOM );
	    }
	}
    }
    return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Tu coraje excede tu sabiduria.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Tal cobardia te enoja.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy puesto en %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Sintaxis: password <antiguo> <nuevo>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Password erroneo.  Espera 10 segundos.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "Password nuevo debe ser de al menos cinco caracteres.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"Password nuevo no aceptable, intentalo de nuevo.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_nivel( CHAR_DATA *ch, char *argument )
{
	char buf[MIL], arg[MIL];
	int i;
	CHAR_DATA *tch;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : nivel subir\n\r", ch );
		send_to_char( "           nivel listar\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "subir" ) )
	{
		for ( tch = ch->in_room->people; tch; tch = tch->next_in_room )
			if ( IS_NPC(tch) && IS_SET(tch->act, ACT_PRACTICE) )
				break;

		if ( !tch )
		{
			send_to_char( "No puedes hacer eso aqui.\n\r", ch );
			return;
		}

		if ( ch->exp < EXP_NIVEL(ch, getNivelPr(ch)+1) )
		{
			act( "$n te dice 'Todavia te falta experiencia'.", tch, NULL, chToEnt(ch), TO_VICT );
			return;
		}

		level_up( ch );
		return;
	}

	if ( !str_cmp( arg, "listar" ) )
	{
		send_to_char( "#U#BNivel Exp     #u\n\r", ch );
		for ( i = getNivelPr(ch); i < getNivelPr(ch) + 6; i++ )
		{
			sprintf( buf, "   %2d %8d\n\r", i, EXP_NIVEL(ch, i) );
			send_to_char( buf, ch );
		}
		return;
	}
}

char *condicion_obj( int percent )
{
	if (percent >= 100)
		return "perfecto estado";
	else if (percent >= 90)
		return "excelente condicion";
	else if (percent >= 75)
		return "buena condicion";
	else if (percent >= 50)
		return "regular condicion";
	else if (percent >= 30)
		return "mala condicion";
	else if (percent >= 15)
		return "pesima condicion";
	else if (percent >= 0)
		return "condicion critica";
	else
		return "";
}

void do_truealign( CHAR_DATA *ch, char *argument )
{
	int align = 0;

	if ( IS_NPC(ch) )
		return;

	if ( ch->pcdata->true_align != -1 )
	{
		send_to_char( "Ya elegiste tu alineacion.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Cual es tu alineacion?\n\r", ch );
		return;
	}

	if ( !str_cmp( "good", argument ) || !str_cmp( "bueno", argument ) )
	{
		if ( IS_EVIL(ch) || IS_NEUTRAL(ch) )
		{
			send_to_char( "Eso no va de acuerdo con tu alineacion actual!\n\r", ch );
			return;
		}
		align = ALIGN_GOOD;
	}
	else
	if ( !str_cmp( "evil", argument) || !str_cmp( "malo", argument ) )
	{
		if ( IS_GOOD(ch) || IS_NEUTRAL(ch) )
		{
			send_to_char( "Eso no va de acuerdo con tu alineacion actual!\n\r", ch );
			return;
		}
		align = ALIGN_EVIL;
	}
	else
	if ( !str_cmp( "neutral", argument ) )
	{
		if ( IS_GOOD(ch) || IS_EVIL(ch) )
		{
			send_to_char( "Eso no va de acuerdo con tu alineacion actual!\n\r", ch );
			return;
		}
		align = ALIGN_NEUTRAL;
	}
	else
	{
		send_to_char( "Esa no es una alineacion valida.\n\r", ch );
		return;
	}

	ch->pcdata->true_align = align;
	send_to_char( "Ok.\n\r", ch );
	return;
}

DO_FUN_DEC(do_newscore)
{
	char buf[MSL];
	AFFECT_DATA *paf, *paf_last = NULL;
	char buf2[MIL];

        send_to_char(
        "+----------------------------------------------------------------------------+\n\r", ch );

	sprintf( buf,
	"|Nombre: %-10s        Sexo: %-10s            Raza  : %-10s    |\n\r"
	"|Clase : %-10s        Edad: %d                    Nivel : %2d            |\n\r",
		ch->name, sex_table[URANGE(0, ch->sex, 2)].name, race_table[ch->race].name,
		class_table[getClasePr(ch)].name, get_age(ch), getNivelPr(ch) );
	send_to_char( buf, ch );

	sprintf( buf,
	"|Puntos Golpe (hp): %4d/%4d                                                |\n\r"
	"|       Mana  (m) : %4d/%4d                                                |\n\r"
	"|       Mov   (mv): %4d/%4d                                                |\n\r",
		UMIN(9999, ch->hit), UMIN(9999, ch->max_hit),
		UMIN(9999, ch->mana), UMIN(9999, ch->max_mana),
		UMIN(999, ch->move), UMIN(999, ch->max_move) );
	send_to_char( buf, ch );

	send_to_char(
	"|                                                                            |\n\r"
	"|Atributos Principales:                                                      |\n\r", ch );

	sprintf( buf,
	"|Frza : %2d(%2d)  Monedas de oro  : %5ld%c   N. objs inventario: %3d  Max: %3d |\n\r"
	"|Dest : %2d(%2d)             plata: %5ld%c   Peso objs: %5ld  Max: %5d      |\n\r"
        "|Cons : %2d(%2d)                                                               |\n\r",
		ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR), UMIN(99999, ch->gold),
			ch->gold > 99999 ? '+' : ' ',
			UMIN(999, ch->carry_number), UMIN(999, can_carry_n(ch)),
		ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX), UMIN(99999, ch->silver),
			ch->silver > 99999 ? '+' : ' ',
			UMIN(99999, get_carry_weight(ch)/10), UMIN(99999, can_carry_w(ch)/10),
		ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON) );
	send_to_char(buf, ch);

	sprintf( buf,
	"|Inte : %2d(%2d)  Sesiones de entrenamiento: %2d ",
	ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT), UMIN(99, ch->train) );
	send_to_char(buf, ch);

	if (getNivelPr(ch) >= 25)
	{
		sprintf(buf,				  " AC: Pierce:%4d Bash :%4d    |\n\r",
			GET_AC(ch,AC_PIERCE), GET_AC(ch,AC_BASH) );
		send_to_char(buf,ch);
	}
	else
		send_to_char("                               |\n\r", ch );

	sprintf( buf,
	"|Sabi : %2d(%2d)              practica     : %2d ",
	ch->perm_stat[STAT_WIS], get_curr_stat(ch,STAT_WIS), UMIN(99, ch->practice) );
	send_to_char( buf, ch );

	if (getNivelPr(ch) >= 25)
	{
		sprintf(buf,				  "     Slash :%4d Magic:%4d    |\n\r",
			GET_AC(ch,AC_SLASH), GET_AC(ch,AC_EXOTIC) );
		send_to_char(buf,ch);
	}
	else
		send_to_char("                               |\n\r", ch );

	send_to_char(
		"+----------------------------------------------------------------------------+\n\r",
		ch );

	if (!IS_SET(ch->comm,COMM_SHOW_AFFECTS))
		return;

	if (ch->affected)
	{
		send_to_char(
		"|Estas afectado por los siguientes spells:                                   |\n\r", ch );

		for ( paf = ch->affected; paf != NULL; paf = paf->next )
		{
			strcpy(buf2,"|");

			if (paf->type < 0 || paf->type >= MAX_SKILL)
			{
				send_to_char( "#B#UAlgo raro pasa. Comunicate con el IMP.#b#u\n\r", ch );
				continue;
			}

			if (paf_last != NULL && paf->type == paf_last->type)
			{
				if (getNivelPr(ch) >= 20)
					strcat( buf2, "                      ");
				else
					continue;
			}
			else
			{
				sprintf( buf, "Spell: %-15s", NOMBRE_SKILL(paf->type) );
				strcat(buf2, buf);
			}

			if ( getNivelPr(ch) >= 20 )
			{
				sprintf( buf,
					": modifica %s por %d ",
					affect_loc_name( paf->location ),
					paf->modifier);
				strcat(buf2, buf);
				if ( paf->duration == -1 )
					sprintf( buf, "permanentemente" );
				else
					sprintf( buf, "por %d horas", paf->duration );
				strcat(buf2, buf);
			}

			while(strlen(buf2) < 77)
				strcat(buf2, " ");
			strcat(buf2,"|\n\r");
			send_to_char( buf2, ch );
			paf_last = paf;
		}
	}
	else 
		send_to_char(
	"|No estas afectado por ningun spell.                                                  |\n\r",ch);

	send_to_char(
	"+----------------------------------------------------------------------------+\n\r", ch );
}

void do_mudftp(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_MUDFTP))
    {
      send_to_char("You will now use the normal editor to edit strings.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_MUDFTP);
    }
    else
    {
      send_to_char("You will now use the mudFTP protocol to edit strings.\n\r",ch);
      SET_BIT(ch->act,PLR_MUDFTP);
    }
 }
