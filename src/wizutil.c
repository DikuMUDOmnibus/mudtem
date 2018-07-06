#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#include <string.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "events.h"
#include "plist.h"
#include "lookup.h"
#include "special.h" /* taxi */

/* To have VLIST show more than vnum 0 - 9900, change the number below: */

#define MAX_SHOW_VNUM  250 /* show only 1 - 100*100 */

#define NUL '\0'

DECLARE_SPELL_FUN(	spell_null	);
COMMAND( do_delete )

extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */

bool	show_help	args ( ( CHAR_DATA *ch, char *argument ) );

/* opposite directions */
extern	const	sh_int	rev_dir[];

/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
char * area_name (AREA_DATA *pArea)
{
        static char buffer[64]; /* short filename */
        char  *period;

        assert (pArea != NULL);
        
        strncpy (buffer, pArea->file_name, 64); /* copy the filename */
        period = strchr (buffer, '.'); /* find the period (midgaard.are) */
        if (period) /* if there was one */
                *period = '\0'; /* terminate the string there (midgaard) */
                
        return buffer;
}

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex,char *buffer)
{
        char *sExit;
        
        switch (ex)
        {
                default:
                        sExit = "??"; break; /* invalid usage */
                case exit_from:
                        sExit = "< "; break;
                case exit_to:
                        sExit = " >"; break;
                case exit_both:
                        sExit = "<>"; break;
        }
        
sprintf (buffer, "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r",
                          left->vnum, left->name,
                          sExit,
                          right->vnum, right->name,
                          area_name(right->area)
            );
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
        char buf[MAX_STRING_LENGTH];
        sh_int i;
	EXIT_DATA *salida, *nExit;
        ROOM_INDEX_DATA *to_room;
        
        strcpy (buffer, "");
        for (i = 0; i < MAX_DIR; i++)
        {
		salida = exit_lookup(room, i);
                if (!salida)
                        continue;
                else
                        to_room = salida->u1.to_room;
                
                if (to_room)  /* there is something on the other side */
		{
                        if ( (room->area == pArea) && (to_room->area != pArea) )
                        { /* an exit from our area to another area */
                          /* check first if it is a two-way exit */
                        
				nExit = exit_lookup(to_room, rev_dir[i]);

                                if ( nExit && nExit->u1.to_room == room )
                                        room_pair (room,to_room,exit_both,buf); /* <> */
                                else
                                        room_pair (room,to_room,exit_to,buf); /* > */
                                
                                strcat (buffer, buf);

                        }
                        else
                        if ( (room->area != pArea) && (salida->u1.to_room->area == pArea) )
                        { /* an exit from another area to our area */

				nExit = exit_lookup(to_room, rev_dir[i]);

                                if  (!(nExit && nExit->u1.to_room == room ) )
                                /* two-way exits are handled in the other if */
                                {
                                        room_pair (to_room,room,exit_from,buf);
                                        strcat (buffer, buf);
                                }
                                
                        } /* if room->area */
                }
        } /* for */
        
}

/* for now, no arguments, just list the current area */
void do_exlist (CHAR_DATA *ch, char * argument)
{
        AREA_DATA* pArea;
        ROOM_INDEX_DATA* room;
        int i;
        char buffer[MAX_STRING_LENGTH];
        
        pArea = ch->in_room->area; /* this is the area we want info on */
        for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
        for (room = room_index_hash[i]; room != NULL; room = room->next)
        /* run through all the rooms on the MUD */
        
        {
                checkexits (room, pArea, buffer);
                send_to_char (buffer, ch);
        }
}

/* show a list of all used VNUMS */

#define COLUMNS                 5   /* number of columns */
#define MAX_ROW                 ((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

void do_vlist (CHAR_DATA *ch, char *argument)
{
        int i,j,vnum;
        ROOM_INDEX_DATA *room;
        char buffer[MAX_ROW*100]; /* should be plenty */
        char buf2 [100];
        
        for (i = 0; i < MAX_ROW; i++)
        {
                strcpy (buffer, ""); /* clear the buffer for this row */
                
                for (j = 0; j < COLUMNS; j++) /* for each column */
                {
                        vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
                        if (vnum < MAX_SHOW_VNUM)
                        {
                                room = get_room_index (vnum * 100 + 1); /* each zone has to have a XXX01 room */
                                sprintf (buf2, "%3d %-8.8s  ", vnum,
                                                 room ? area_name(room->area) : "-" );
                                                 /* something there or unused ? */
                                strcat (buffer,buf2);

                        }
                } /* for columns */
                
                send_to_char (buffer,ch);
                send_to_char ("\n\r",ch);
        } /* for rows */
}

/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players.
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (char* name);  /* comm.c */

void do_rename (CHAR_DATA* ch, char* argument)
{
        char old_name[MAX_INPUT_LENGTH],
             new_name[MAX_INPUT_LENGTH],
             strsave [MAX_INPUT_LENGTH];

        CHAR_DATA* victim;
        FILE* file;
        
        argument = one_argument(argument, old_name); /* find new/old name */
        one_argument (argument, new_name);
        
        /* Trivial checks */
        if (!old_name[0])
        {
                send_to_char ("Rename who?\n\r",ch);
                return;
        }
        
        victim = get_char_world (ch, old_name);
        
        if (!victim)
        {
                send_to_char ("There is no such a person online.\n\r",ch);
                return;
        }
        
        if (IS_NPC(victim))
        {
                send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
                return;
        }

        /* allow rename self new_name,but otherwise only lower level */
        if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
        {
                send_to_char ("You failed.\n\r",ch);
                return;
        }
        
        if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
        {
                send_to_char ("This player has lost his link or is inside a pager or the like.\n\r",ch);
                return;
        }

        if (!new_name[0])
        {
                send_to_char ("Rename to what new name?\n\r",ch);
                return;
        }
        
        /* Insert check for clan here!! */
        if (victim->clan)
        {
                send_to_char ("This player is member of a clan, remove him from there first.\n\r",ch);
                return;
        }
        
        if (!check_parse_name(new_name))
        {
                send_to_char ("The new name is illegal.\n\r",ch);
                return;
        }

	/* First, check if there is a player named that off-line */
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char ("A player with that name already exists!\n\r",ch);
                fclose (file);
		fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        /* Check .gz file ! */
	sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
                fclose (file);
		fpReserve = fopen( NULL_FILE, "r" );
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        if (get_char_world(ch,new_name)) /* check for playing level-1 non-saved */
        {
                send_to_char ("A player with the name you specified already exists!\n\r",ch);
                return;
        }

	/* lo sacamos de la plist */
	player_delete(victim->name);

        /* Save the filename of the old name */
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );

        /* Rename the character and save him to a new file */
        /* NOTE: Players who are level 1 do NOT get saved under a new name */
        free_string (victim->name);
        victim->name = str_dup (capitalize(new_name));

	/* lo ponemos con el nuevo nombre */
	update_player(victim);
        save_char_obj (victim);
        
        /* unlink the old file */
        unlink (strsave); /* unlink does return a value.. but we do not care */

        /* That's it! */
	send_to_char ("Character renamed.\n\r",ch);

        victim->position = POS_STANDING; /* I am laaazy */
        act ("$n has renamed you to $N!",ch,NULL,chToEnt(victim),TO_VICT);
} /* do_rename */

void do_sstat( CHAR_DATA *ch, char *argument )
/* sstat by Garion */
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH * 4];
    int skill;
    int sn;
    int col;
    int clase;

    one_argument( argument, arg );
    col = 0;
 
    if ( arg[0] == '\0' )
    {
        send_to_char("Sstat whom?\n\r", ch);
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char("That person isn't logged on.\n\r", ch);
        return;
    }
 
    if ( !IS_NPC(victim) )
    	clase = getClasePr(victim);
    else
    {
	if (IS_SET(victim->act,ACT_CLERIC) && IS_SET(victim->act,ACT_WARRIOR))
		clase = CLASS_RANGER;
	else
    	if (IS_SET(victim->act,ACT_CLERIC))
    		clase = CLASS_CLERIC;
    	else
    	if (IS_SET(victim->act,ACT_MAGE))
    		clase = CLASS_MAGE;
    	else
    	if (IS_SET(victim->act,ACT_WARRIOR))
    		clase = CLASS_WARRIOR;
    	else
    	if (IS_SET(victim->act,ACT_THIEF))
    		clase = CLASS_THIEF;
    	else
    		clase = CLASS_WARRIOR;
    }

    buf2[0] = '\0';
 
    for ( sn = 0; sn < MAX_SKILL ; sn++ )
    {
        if ( skill_table[sn].name == NULL )
            break;

        if ( getNivelPr(victim) < skill_table[sn].skill_level[clase] )
            continue;

        if ( !IS_NPC(victim) )
        	skill = victim->pcdata->learned[sn];
        else
        	skill = get_skill(victim,sn);

	if ( skill == 0 )
		continue;

	if ( skill > 100 )
		bug( "get_skill retorno %d", skill );

	sprintf( buf1, "%18s %3d %% ", skill_table[sn].name, skill );

        strcat( buf2, buf1 );
        if ( ++col %3 == 0 )
            strcat( buf2, "\n\r" );
    }
    if ( col % 3 != 0 )
         strcat( buf2, "\n\r" );
    sprintf( buf1, "%s has %d practice sessions left.\n\r", victim->name,
                victim->practice );
    strcat( buf2, buf1 );
    send_to_char( buf2, ch );
    return;
 
}

void do_omni( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int immmatch;
    int mortmatch;
    int hptemp;
 
 /*
 * Initalize Variables.
 */

    immmatch = 0;
	mortmatch = 0;
    buf[0] = '\0';
    output = new_buf();

 /*
 * Count and output the IMMs.
 */

	sprintf( buf, " ----Immortals:----\n\r");
	add_buf(output,buf);
	sprintf( buf, "Name          Level   Wiz   Incog   [ Vnum]\n\r");
	add_buf(output,buf);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch)  || getNivelPr(wch) < 52)
	    continue;
 
        immmatch++;

	sprintf( buf, "%-14s %d     %-2d     %-2d     [%5d]%s%s\n\r",
			wch->name,
			getNivelPr(wch),
			wch->invis_level,
			wch->incog_level,
			wch->in_room->vnum,
			(IS_IMMORTAL(ch) && IS_SWITCHED(wch)) ? "-->" : "",
			(IS_IMMORTAL(ch) && IS_SWITCHED(wch)) ? wch->name : "");
			add_buf(output,buf);
    }
    
    
 /*
 * Count and output the Morts.
 */
	sprintf( buf, " \n\r ----Mortals:----\n\r");
	add_buf(output,buf);
	sprintf( buf, "Name           Race/Class   Position        Lev  %%hps  [ Vnum]\n\r");
	add_buf(output,buf);
	hptemp = 0;
	
   for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
        
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch) || getNivelPr(wch) > getNivelPr(ch) || getNivelPr(wch) > 51)
	    continue;
 
        mortmatch++;
 	if ((wch->max_hit != wch->hit) && (wch->hit > 0))
 		hptemp = (wch->hit*100)/wch->max_hit;
 	else if (wch->max_hit == wch->hit)
 		hptemp = 100;
 	else if (wch->hit < 0)
 		hptemp = 0;
 		
	class = class_table[getClasePr(wch)].who_name;

	sprintf( buf, "%-14s %5s/%3s    %-15s %-2d   %3d%%  [%5d]\n\r",
		wch->name,
		race_table[wch->race].who_name,
	    class, capitalize( position_table[wch->position].name) , 
		getNivelPr(wch),
		hptemp,
		wch->in_room->vnum);
	add_buf(output,buf);
    }

/*
 * Tally the counts and send the whole list out.
 */
   sprintf( buf2, "\n\rIMMs found: %d\n\r", immmatch );
    add_buf(output,buf2);
    sprintf( buf2, "Morts found: %d\n\r", mortmatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		send_to_char("Addlag a quien?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("No esta aqui.\n\r", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("Eso NO TIENE mucho sentido.\n\r", ch);
		return;
	}

	if (x > 900)
	{
		send_to_char("Hay un LIMITE para los castigos crueles y despiadados.\n\r", ch);
		return;
	}

	send_to_char("A alguien, REALMENTE, le caiste mal.\n\r", victim);
	WAIT_STATE(victim, x);
	send_to_char("Anadiendo lag...\n\r", ch);
	return;
}

void do_astat( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *area;
	MOB_INDEX_DATA *mob = NULL;
	MOB_INDEX_DATA *highlev = NULL;
	MOB_INDEX_DATA *lowlev = NULL;
	char buf[MSL];
	int vnum, cnt = 0, level, minlevel = 60, maxlevel = 0;
	float promedio = 0;

	if ( ch->in_room == NULL )
		return;
	
	if ( ( area = ch->in_room->area ) == NULL )
		return;
	
	sprintf( buf, "Estadisticas del area '%s' :\n\r", area->name );
	send_to_char( buf, ch );

	sprintf( buf, "Nombre de archivo : %s\n\r", area->file_name );
	send_to_char( buf, ch );
	
	sprintf( buf, "Creditos          : %s\n\r", area->credits );
	send_to_char( buf, ch );
	
	send_to_char( "Nivel recomendado :\n\r", ch );
	
	sprintf( buf, "           Minimo : %2d\n\r", area->low_range );
	send_to_char( buf, ch );
	
	sprintf( buf, "           Maximo : %2d\n\r", area->high_range );
	send_to_char( buf, ch );

	for ( vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ )
	{
		mob = get_mob_index( vnum );

		if ( mob == NULL )
			continue;

		level = mob->level;
		
		if ( level < minlevel )
		{
			minlevel = level;
			lowlev = mob;
		}
		
		if ( level > maxlevel )
		{
			maxlevel = level;
			highlev  = mob;
		}
		
		promedio += level;
		cnt++;
	}
	
	if ( lowlev )
	{
		sprintf( buf, "Mob de nivel mas bajo : %2d [%5d] %s\n\r",
		 minlevel, lowlev->vnum, lowlev->player_name );
		send_to_char( buf, ch );
	}
	
	if ( highlev )
	{
		sprintf( buf, "Mob de nivel mas alto : %2d [%5d] %s\n\r",
		 maxlevel, highlev->vnum, highlev->player_name );
		send_to_char( buf, ch );
	}
	
	sprintf( buf, "Numero de mobs        : %d\n\r", cnt );
	send_to_char( buf, ch );
	
	if ( cnt )
	{
		sprintf( buf, "Promedio de niveles   : %f\n\r", promedio / cnt );
		send_to_char( buf, ch );
	}

	return;
}

void do_fulldump( CHAR_DATA *ch, char *argument )
{
    char buf[MIL];
    FILE *fp, *fp2, *fp3, *fp4, *fp5, *fp6;
    int i,x;
    OBJ_INDEX_DATA *obj;
    MOB_INDEX_DATA *mob;
    
    /* open file */
    fclose(fpReserve);

    sprintf( buf, "%sskills.dmp", DUMP_DIR );
    fp = fopen(buf,"w");

    fprintf( fp, "Sn  Slt %-19.19s Sp Flags  Mage  Cleri Thief Warri Psici Range\n", "Nombre" );
    for ( i = 0; i < MAX_SKILL; i++ )
	if ( skill_table[i].name )
	{
		fprintf(fp,"%3d %3d %19s %s %-6.6s ",
			i, skill_table[i].slot, skill_table[i].name,
			skill_table[i].spell_fun != spell_null ? "Si" : "No",
			print_flags(skill_table[i].flags) );
		for ( x = 0; x < MAX_CLASS; x++ )
			fprintf( fp, "%2d/%-2d ", skill_table[i].skill_level[x],
				skill_table[i].rating[x] );
		fprintf( fp, "\n" );
	}

    fclose(fp);

    fp  = fopen( DUMP_DIR "weapons.dmp", "w" );
    fp2 = fopen( DUMP_DIR "armor.dmp", "w" );
    fp3 = fopen( DUMP_DIR "potions.dmp", "w" );
    fp4 = fopen( DUMP_DIR "scrolls.dmp", "w" );
    fp5 = fopen( DUMP_DIR "pills.dmp", "w" );
    fp6 = fopen( DUMP_DIR "wands.dmp", "w" );

    fprintf( fp,  "Vnum  %-36.36s Lv %-10.10s Material v1 v2 v4     Weight Aff\n", "Nombre", "Tipo" );
    fprintf( fp2, "Vnum  %-36.36s Lv Pier Bash Slas Exot Weight Aff\n", "Nombre" );
    fprintf( fp3, "Vnum  %-20.20s Lv V0 %-10.10s %-10.10s %-10.10s %-10.10s\n",
    	"Nombre", "Spell1", "Spell2", "Spell3", "Spell4" );
    fprintf( fp4, "Vnum  %-20.20s Lv V0 %-10.10s %-10.10s %-10.10s %-10.10s\n",
    	"Nombre", "Spell1", "Spell2", "Spell3", "Spell4" );
    fprintf( fp5, "Vnum  %-20.20s Lv V0 %-10.10s %-10.10s %-10.10s %-10.10s\n",
    	"Nombre", "Spell1", "Spell2", "Spell3", "Spell4" );
    fprintf( fp6, "Vnum  %-20.20s Lv V0 CT CL Spell\n", "Nombre" );

    for ( i = 0; i < MAX_VNUM; i++ )
    {
    	obj = get_obj_index(i);
    	
    	if ( !obj )
    		continue;
    	
    	if ( obj->item_type == ITEM_WEAPON )
		fprintf( fp, "%5d %-36.36s %2d %-10.10s %-8.8s %2d %2d %-6.6s %6d %s\n", i,
			obj->name, obj->level, weapon_name(obj->value[0]),
			flag_string(mat_table, obj->material), obj->value[1], obj->value[2],
			flag_string(weapon_type2, obj->value[4]),
			obj->weight, affect_list(obj->affected) );
	else
	if ( obj->item_type == ITEM_ARMOR )
		fprintf( fp2, "%5d %-36.36s %2d %-4d %-4d %-4d %-4d %6d %s\n",
			i, obj->name, obj->level, obj->value[0], obj->value[1],
			obj->value[2], obj->value[3], obj->weight,
			affect_list(obj->affected) );
	else
	if ( obj->item_type == ITEM_POTION )
		fprintf( fp3, "%5d %-20.20s %2d %2d %-10.10s %-10.10s %-10.10s %-10.10s\n",
			i, obj->name, obj->level, obj->value[0],
			obj->value[1] > 0 ? skill_table[obj->value[1]].name : "",
			obj->value[2] > 0 ? skill_table[obj->value[2]].name : "",
			obj->value[3] > 0 ? skill_table[obj->value[3]].name : "",
			obj->value[4] > 0 ? skill_table[obj->value[4]].name : "");
	else
	if ( obj->item_type == ITEM_SCROLL )
		fprintf( fp4, "%5d %-20.20s %2d %2d %-10.10s %-10.10s %-10.10s %-10.10s\n",
			i, obj->name, obj->level, obj->value[0],
			obj->value[1] > 0 ? skill_table[obj->value[1]].name : "",
			obj->value[2] > 0 ? skill_table[obj->value[2]].name : "",
			obj->value[3] > 0 ? skill_table[obj->value[3]].name : "",
			obj->value[4] > 0 ? skill_table[obj->value[4]].name : "");
	else
	if ( obj->item_type == ITEM_PILL )
		fprintf( fp5, "%5d %-20.20s %2d %2d %-10.10s %-10.10s %-10.10s %-10.10s\n",
			i, obj->name, obj->level, obj->value[0],
			obj->value[1] > 0 ? skill_table[obj->value[1]].name : "",
			obj->value[2] > 0 ? skill_table[obj->value[2]].name : "",
			obj->value[3] > 0 ? skill_table[obj->value[3]].name : "",
			obj->value[4] > 0 ? skill_table[obj->value[4]].name : "");
	else
	if ( obj->item_type == ITEM_WAND )
		fprintf( fp6, "%5d %-20.20s %2d %2d %2d %2d %s\n",
			obj->vnum, obj->name,
			obj->level, obj->value[0],
			obj->value[1], obj->value[2],
			obj->value[3] > 0 ? skill_table[obj->value[3]].name : "" );
    }

    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
    fclose(fp5);
    fclose(fp6);

    sprintf( buf, "%smobact.dmp", DUMP_DIR );
    fp = fopen( buf, "w" );
    
    fprintf( fp, "Vnum  %-35.35s Lv Act\n", "Nombre" );
    
    for ( i = 0; i < MAX_VNUM; i++ )
    {
    	mob = get_mob_index( i );
    	
    	if ( !mob )
    		continue;

    	fprintf( fp, "%5d %-35.35s %2d %s, %s\n", mob->vnum, mob->player_name,
    		mob->level,
    		flag_string( act_flags, mob->act ),
		flag_string( off_flags, mob->off_flags ) );
    }

    fclose( fp );

    fpReserve = fopen( NULL_FILE, "r" );
}

void do_deprototipizar( CHAR_DATA *ch, char *argument )
{
	int vnum;
	AREA_DATA *pArea;
	MOB_INDEX_DATA *pMob;
	ROOM_INDEX_DATA *pRoom;
	OBJ_INDEX_DATA *pObj;

	if (IS_NPC(ch) || ch->pcdata->security < 9)
	{
		send_to_char( "No puedes hacer eso.\n\r", ch );
		return;
	}
	
	pArea = ch->in_room->area;
	
	if ( !IS_SET(pArea->area_flags, AREA_PROTOTIPO) )
	{
		send_to_char( "Esta area no es prototipo.\n\r", ch );
		return;
	}
	
	REMOVE_BIT(pArea->area_flags, AREA_PROTOTIPO);
	SET_BIT(pArea->area_flags, AREA_CHANGED);
	
	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	{
		if ( (pObj = get_obj_index(vnum)) != NULL )
			REMOVE_BIT(pObj->extra_flags, ITEM_PROTOTIPO);
		if ( (pMob = get_mob_index(vnum)) != NULL )
			REMOVE_BIT(pMob->act, ACT_PROTOTIPO);
		if ( (pRoom = get_room_index(vnum)) != NULL )
			REMOVE_BIT(pRoom->room_flags, ROOM_PROTOTIPO);
	}
	
	send_to_char( "Area deprototipizada.\n\r", ch );
	return;
}

void do_prototipizar( CHAR_DATA *ch, char *argument )
{
	int vnum;
	AREA_DATA *pArea;
	MOB_INDEX_DATA *pMob;
	ROOM_INDEX_DATA *pRoom;
	OBJ_INDEX_DATA *pObj;

	if (IS_NPC(ch) || ch->pcdata->security < 9)
	{
		send_to_char( "No puedes hacer eso.\n\r", ch );
		return;
	}

	pArea = ch->in_room->area;

	if ( IS_SET(pArea->area_flags, AREA_PROTOTIPO) )
	{
		send_to_char( "Esta area ya es prototipo.\n\r", ch );
		return;
	}

	SET_BIT(pArea->area_flags, AREA_PROTOTIPO);
	SET_BIT(pArea->area_flags, AREA_CHANGED);

	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	{
		if ( (pObj = get_obj_index(vnum)) != NULL )
			SET_BIT(pObj->extra_flags, ITEM_PROTOTIPO);
		if ( (pMob = get_mob_index(vnum)) != NULL )
			SET_BIT(pMob->act, ACT_PROTOTIPO);
		if ( (pRoom = get_room_index(vnum)) != NULL )
			SET_BIT(pRoom->room_flags, ROOM_PROTOTIPO);
	}

	send_to_char( "Area prototipizada.\n\r", ch );
	return;
}

void do_shell( CHAR_DATA *ch, char *argument )
{
	FILE *fp;
	int c;
	char buf[2];
	char comando[MSL];

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : shell [comandos]\n\r", ch );
		return;
	}

	buf[1] = '\0';
	strcpy(comando, argument);
	strcat(comando, " < /dev/null");

#if defined(WIN32)
	fp = _popen( comando, "r" );
#else
	fp = popen( comando, "r" );
#endif

	while ( (c = getc(fp)) != EOF )
	{
		buf[0] = c;
		send_to_char( buf, ch );
	}
#if defined(WIN32)
	_pclose(fp);
#else
	pclose(fp);
#endif

	send_to_char( "Ok.\n\r", ch );
}

void do_grep( CHAR_DATA *ch, char *argument )
{
	int players, mobs, warriors, mages, clerics, thieves, psi, taxis, group, hunt, noclase, rangers;
	CHAR_DATA *temp;
	char buf[MSL];
	int clase;

	players = mobs = warriors = mages = clerics = thieves = psi = taxis = group = hunt = rangers = noclase = 0;

	for ( temp = char_list; temp; temp = temp->next )
	{
		if ( !IS_NPC(temp) )
			players++;
		else
		{
			mobs++;
			clase = getClasePr(temp);

			switch(clase)
			{
				case CLASS_WARRIOR:	warriors++;	break;
				case CLASS_MAGE:	mages++;	break;
				case CLASS_CLERIC:	clerics++;	break;
				case CLASS_THIEF:	thieves++;	break;
				case CLASS_PSI:		psi++;		break;
				case CLASS_RANGER:	rangers++;	break;
			}

			if ( !IS_SET(temp->act, ACT_MAGE)
			&&   !IS_SET(temp->act, ACT_CLERIC)
			&&   !IS_SET(temp->act, ACT_PSI)
			&&   !IS_SET(temp->act, ACT_WARRIOR)
			&&   !IS_SET(temp->act, ACT_THIEF) )
				noclase++;

			if ( temp->spec_fun == spec_taxi )
				taxis++;
			if ( IS_SET(temp->act, ACT_GROUP) )
				group++;
			if ( is_hunting(temp) )
				hunt++;
		}
	}

	sprintf(buf,	"Jugadores   : %4d\n\r"
			"Mobs        : %4d\n\r"
			"PCs         : %4d\n\r"
			"- Warriors  : %4d (#B%3.2f%%#b)\n\r"
			"- Magos     : %4d (#B%3.2f%%#b)\n\r"
			"- Clerigos  : %4d (#B%3.2f%%#b)\n\r"
			"- Ladrones  : %4d (#B%3.2f%%#b)\n\r"
			"- Psionicos : %4d (#B%3.2f%%#b)\n\r"
			"- Taxis     : %4d\n\r"
			"- Group     : %4d\n\r"
			"- Cazando   : %4d\n\r"
			"- Sin clase : %4d\n\r",
				mobs + players,
				mobs,
				players,
				warriors, warriors / (float) (mobs + players) * 100.0,
				mages, mages / (float) (mobs + players) * 100.0,
				clerics, clerics / (float) (mobs + players) * 100.0,
				thieves, thieves / (float) (mobs + players) * 100.0,
				psi, psi / (float) (mobs + players) * 100.0,
				taxis,
				group,
				hunt,
				noclase );
	send_to_char( buf, ch );

	return;
}

void slowprint( CHAR_DATA *ch, char *argument, int x, int y, int speed, bool preprint )
{
	int i,j;
	char temp[MSL], buf[MIL];

	sprintf( temp, "\e[%d;%dH", y, x );
	send_to_char( temp, ch );

	if ( preprint == 1 )
	{
		sprintf( temp,
#if defined(linux)
		 "%s\e[%dD", argument, strlen(argument) );
#else
		 "%s\e[%ldD", argument, strlen(argument) );
#endif
		send_to_char( temp, ch );
	}

#if defined(linux)
	sprintf( buf, "\e[%dD", strlen(argument) );
#else
	sprintf( buf, "\e[%ldD", strlen(argument) );
#endif
	temp[1] = '\0';

	for ( i = 0; i < strlen(argument); ++i )
	{
		if ( preprint != 2 )
		{
			temp[0] = argument[i];
			for ( j = 0; j < speed; ++j )
			{
				send_to_char( "#B", ch );
				send_to_char( temp, ch );
				send_to_char( "#n\e[1D", ch );
			}
			send_to_char( temp, ch );
		}
		else
		{
			for ( j = 0; j < strlen(argument); ++j )
			{
				temp[0] = argument[j];
				if ( i == j )
					send_to_char( "#B", ch );
				send_to_char( temp, ch );
				if ( i == j )
					send_to_char( "#n", ch );
			}
			send_to_char( buf, ch ); /* para hacerlo volver */
		}
	}

	if ( preprint == 2 )
		send_to_char( argument, ch );
}

void do_test( CHAR_DATA *ch, char *argument )
{
	char buf[MIL];
	char *mensaje = "BLACK DRAGON";
	int i = 0, j;

	sprintf( buf, "\e[H\e[2J" );
	send_to_char( buf, ch );

	while ( i < 33 )
	{
		sprintf( buf, "\e[1;%dH BLACK", ++i );
		send_to_char( buf, ch );
		sprintf( buf, "\e[1;%dHDRAGON ", 73 - i );
		send_to_char( buf, ch );
	}

	for ( i = 0; i < 3; ++i )
		send_to_char( "\e[13D#BBLACK DRAGON ", ch );
	send_to_char( "\e[13D#bBLACK DRAGON", ch );
	send_to_char( "\e[12D", ch );
	i = 0; buf[1] = '\0';

	for ( i = 0; i < (signed) strlen(mensaje); ++i )
	{
		for ( j = 0; j < (signed) strlen(mensaje); ++j )
		{
			if ( i == j )
				send_to_char( "#B", ch );
			buf[0] = ( i == j ) ? UPPER(mensaje[j]) : mensaje[j];
			send_to_char( buf, ch );
			if ( i == j )
				send_to_char( "#n", ch );
		}
		send_to_char( "\e[12D", ch );
	}
	send_to_char( "BLACK DRAGON", ch );

	for ( i = 0; i < 37; ++i )
	{
		sprintf( buf, "\e[2;%dH M", i + 1 );
		send_to_char( buf, ch );
		sprintf( buf, "\e[2;%dHD ", 76 - i );
		send_to_char( buf, ch );
	}
	for ( i = 0; i < 3; ++i )
		send_to_char( "\e[2;38H#BMUD#n", ch );
	send_to_char( "\e[2;38HMUD\n\r", ch );

	slowprint( ch, "Bienvenido a ROM 2.4.  Por favor no alimenten a los mobiles.", 1, 4, 1, 1 );
	send_to_char( "\n\r", ch );
}

bool prohibido( CHAR_DATA *ch, char *comando )
{
	struct prob_data *prob;

	if ( IS_NPC(ch) )
		return FALSE;

	for ( prob = ch->pcdata->prohibido; prob; prob = prob->next )
		if ( !str_cmp( comando, prob->comando ) )
			return TRUE;

	return FALSE;
}

void extract_prob( CHAR_DATA *ch, struct prob_data *prob )
{
	struct prob_data *prev;

	if ( ch->pcdata->prohibido == prob )
	{
		ch->pcdata->prohibido	= ch->pcdata->prohibido->next;
		free_string(prob->comando);
		free(prob);
		return;
	}

	for ( prev = ch->pcdata->prohibido; prev; prev = prev->next )
		if ( prev->next == prob )
			break;

	if ( !prev )
	{
		bug( "Extract_prob : prev no encontrado", 0 );
		return;
	}

	prev->next	= prob->next;
	free_string(prob->comando);
	free(prob);
	return;
}

void do_prohibir( CHAR_DATA *ch, char *argument )
{
	int cmd_lookup( char * );
	char jugador[MIL];
	struct prob_data *temp;
	CHAR_DATA *victim;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : prohibir [jugador] [comando]\n\r", ch );
		return;
	}

	argument = one_argument( argument, jugador );

	if ( (victim = get_char_world( ch, jugador )) == NULL )
	{
		send_to_char( "No esta jugando.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) || (get_trust(victim) >= get_trust(ch)) )
	{
		send_to_char( "No puedes hacer eso.\n\r", ch );
		return;
	}

	if ( cmd_lookup(argument) == -1 )
	{
		send_to_char( "Comando inexistente.\n\r", ch );
		return;
	}

	if ( prohibido(victim, argument) )
	{
		for ( temp = victim->pcdata->prohibido; temp; temp = temp->next )
			if ( !str_cmp(temp->comando, argument) )
			{
				extract_prob( victim, temp );
				break;
			}
		send_to_char( "Comando habilitado.\n\r", ch );
		return;
	}

	temp				= malloc( sizeof(struct prob_data) );
	temp->comando			= str_dup(argument);

	temp->next			= victim->pcdata->prohibido;
	victim->pcdata->prohibido	= temp;

	send_to_char( "Ok.\n\r", ch );
}

void do_corpse( CHAR_DATA *ch, char *argument )
{
	FILE *fp;
	char buf[MIL];
	bool result;
	extern bool fread_corpse( FILE *, CHAR_DATA * );

	if ( IS_NPC(ch) )
		return;

	sprintf( buf, "%s%s", CORPSE_DIR, ch->name);
	fp = fopen( buf, "r" );

	if ( !fp )
	{
		send_to_char( "No tienes cadaver.\n\r", ch );
		return;
	}

	result = fread_corpse( fp, ch );

	fclose(fp);

	if ( result )
	{
		send_to_char( "Ok.\n\r", ch );
		nuke_corpse(ch->name, TRUE);
	}
	else
		send_to_char( "Algo salio mal.\n\r", ch );

	return;
}

void do_nuke( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( (victim = get_char_world( ch, argument )) == NULL
	||    IS_NPC(victim) )
	{
		send_to_char( "Victima inexistente.\n\r", ch );
		return;
	}

	victim->pcdata->confirm_delete = TRUE;
	do_delete( victim, "" );
}

DO_FUN_DEC(do_limit)
{
	OBJ_INDEX_DATA * pObj;
	char arg[MIL];

	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "listar" ) )
	{
		int ihash;

		printf_to_char( ch, "#U Vnum   Cn Ni Nombre          #u\n\r" );

		for ( ihash = 0; ihash < MAX_KEY_HASH; ihash++ )
			for ( pObj = obj_index_hash[ihash]; pObj; pObj = pObj->next )
				if ( pObj->max_count > 0 )
					printf_to_char( ch, "[%5d] %2d %2d %s\n\r",
						pObj->vnum,
						pObj->max_count,
						pObj->level,
						pObj->name );

		return;
	}

	if ( !str_cmp( arg, "vaciar" ) )
	{
		int vnum = atoi(argument);
		LIMIT_DATA * limit, * limit_next;

		if ( (pObj = get_obj_index(vnum)) == NULL )
		{
			send_to_char( "Obj inexistente.\n\r", ch );
			return;
		}

		for ( limit = pObj->limit; limit; limit = limit_next )
		{
			limit_next = limit->next;

			free_limit(limit);
		}

		pObj->limit = NULL;
		send_to_char( "Ok.\n\r", ch );
		return;
	}

	send_to_char(	"Sintaxis : limit listar\n\r"
			"           limit vaciar [vnum]\n\r", ch );
	return;
}

DO_FUN_DEC(do_objlist)
{
	int hash;
	OBJ_INDEX_DATA *obj;

	if (IS_NULLSTR(argument))
	{
		send_to_char( "Sintaxis : objlist [tipo obj]\n\r", ch );
		return;
	}

	for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
			if ( obj->area == ch->in_room->area
			&&   obj->item_type == ITEM_WEAPON )
				printf_to_char( ch, "%5d %-14.14s %2d %d %d %s %s\n\r",
					obj->vnum, obj->name, obj->level, obj->value[1],
					obj->value[2], flag_string(weapon_type2, obj->value[3]),
					affect_list(obj->affected) );
}

DO_FUN_DEC(do_tablista)
{
	show_help(ch,argument);
}
