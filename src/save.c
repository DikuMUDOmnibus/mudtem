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
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>	// para unlink()
#include <errno.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "screen.h"
#include "tables.h"
#include "plist.h"
#if defined(CRC)
#include "crc.h"
#endif
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

#if !defined(_AIX)
int rename(const char *oldfname, const char *newfname);
#endif

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest, ROOM_INDEX_DATA *room ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp, OBJ_DATA *corpse, ROOM_INDEX_DATA *room ) );


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;
    PLIST * pl;

    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_NOSAVE) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    update_player(ch);

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || getNivelPr(ch) >= LEVEL_IMMORTAL)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}
	else
	{
		fprintf(fp,"Lev %2d Trust %2d Sec %d  %s%s\n",
			getNivelPr(ch), get_trust(ch),
			ch->pcdata->security,ch->name, ch->pcdata->title);
		fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );

	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0, NULL );

	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);

	if (ch->pcdata->corpse)
		save_corpse(ch, TRUE);

	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    rename(TEMP_FILE,strsave);

    pl = plist_lookup(ch->name);

#if defined(CRC)
    if ( pl )
    	pl->crc = calcular_crc(strsave);
#endif

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, gn, pos;
    LEVEL_DATA *lev;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %ld\n",	ch->id			);
#if !defined(MSDOS)
    fprintf( fp, "LogO %ld\n",	current_time		);
#else
    fprintf( fp, "LogO %d\n",	current_time		);
#endif
    fprintf( fp, "Vers %d\n",   5			);
    if (ch->short_descr[0] != '\0')
      	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if (ch->description[0] != '\0')
    	fprintf( fp, "Desc %s~\n",	ch->description	);

    fprintf( fp, "Race %s~\n", race_table[ch->race].name );

    if (ch->clan)
    	fprintf( fp, "Clan %s~\n",get_clan_table(ch->clan)->name);
    fprintf( fp, "Sex  %d\n",	ch->sex			);
/*  fprintf( fp, "Cla  %d\n",	getClasePr(ch)		);
    fprintf( fp, "Levl %d\n",	getNivelPr(ch)		); */
    for ( lev = ch->level_data; lev; lev = lev->next )
    {
    	if (lev->clase < 0 || lev->clase >= MAX_CLASS )
    		bugf("fwrite_char : char %s con lev->clase %d", ch->name, lev->clase );
    	if (lev->nivel < 1 || lev->nivel > MAX_LEVEL )
    		bugf("fwrite_char : char %s con lev->nivel %d", ch->name, lev->nivel );
    	fprintf( fp, "Nivel '%s' %d\n",
    		ENTRE_I(0, lev->clase, MAX_CLASS - 1) ?
    			class_table[lev->clase].name : "error",
    		URANGE(1, lev->nivel, MAX_LEVEL) );
    }
    if (ch->trust)
	fprintf( fp, "Tru  %d\n",	ch->trust	);
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    fprintf( fp, "Luck %d\n",		ch->luck	);
    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    if (ch->gold > 0)
      fprintf( fp, "Gold %ld\n",	ch->gold	);
    else
      fprintf( fp, "Gold %d\n", 0			); 
    if (ch->silver > 0)
	fprintf( fp, "Silv %ld\n",ch->silver		);
    else
	fprintf( fp, "Silv %d\n",0			);
    fprintf( fp, "Exp  %d\n",	ch->exp			);
    if (ch->act != 0)
	fprintf( fp, "Act  %s\n",	print_flags(ch->act)		);
    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",	print_flags(ch->affected_by)	);
    if (ch->affected2_by != 0)
	fprintf( fp, "Af2By %s\n",	print_flags(ch->affected2_by)	);
    if (ch->res_flags != 0)
    	fprintf( fp, "Res  %s\n",	print_flags(ch->res_flags)	);
    if (ch->vuln_flags != 0)
    	fprintf( fp, "Vuln %s\n",	print_flags(ch->vuln_flags)	);
    if (ch->imm_flags != 0)
    	fprintf( fp, "Imm  %s\n",	print_flags(ch->imm_flags)	);
    fprintf( fp, "Comm %s\n",		print_flags(ch->comm)		);
    if (ch->wiznet)
    	fprintf( fp, "Wizn %s\n",	print_flags(ch->wiznet)		);
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n",	ch->invis_level			);
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",ch->incog_level);
    fprintf( fp, "Pos  %d\n",	
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->practice != 0)
    	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Alig  %d\n",	ch->alignment		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",	ch->hitroll	);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",	ch->damroll	);
    fprintf( fp, "ACs %d %d %d %d\n",	
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    fprintf (fp, "AMod %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	struct prob_data *prob;
	BANK_DATA *bank;
	MEM_DATA *mem;

	for ( mem = ch->memory; mem; mem = mem->next )
#if defined(MSDOS)
                fprintf( fp, "Mem %d %s %d\n",
#else
		fprintf( fp, "Mem %ld %s %d\n",
#endif
			mem->when, print_flags(mem->reaction), mem->id );

	fprintf( fp, "Victorias %d\nDerrotas %d\n",
		ch->pcdata->victorias, ch->pcdata->derrotas );

	if ( ch->pcdata->quaff )
		fprintf( fp, "Quaff %d\n", ch->pcdata->quaff );

	if ( ch->desc )
		fprintf( fp, "Term %s~\n", term_table[ch->desc->term].name );

	for ( prob = ch->pcdata->prohibido; prob; prob = prob->next )
		fprintf( fp, "Prob %s~\n", prob->comando );

	for ( bank = ch->pcdata->bank; bank; bank = bank->next )
		if ( bank->valor > 0 )
#if defined(MSDOS)
                        fprintf( fp, "NBank %ld %d %d %d %d\n",
#else
			fprintf( fp, "NBank %ld %d %ld %ld %d\n",
#endif
				bank->valor,
				bank->tipo,
				bank->when,
				bank->start,
				(int) (bank->interes * 100.0) );

	if (ch->pcdata->prompt != NULL && str_cmp(ch->pcdata->prompt, PROMPT_ALL) )
	    fprintf( fp, "Prom %s~\n",	ch->pcdata->prompt	);
	fprintf( fp, "Scro %d\n", 	ch->pcdata->lines	);
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (!IS_NULLSTR(ch->pcdata->bamfin))
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (!IS_NULLSTR(ch->pcdata->bamfout))
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	if (!IS_NULLSTR(ch->pcdata->mensaje))
		fprintf( fp, "Mensaje %s~\n",	ch->pcdata->mensaje);

#if !defined(MSDOS)
	fprintf( fp, "Not  %ld %ld %ld %ld %ld\n",
		ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_penalty,
		ch->pcdata->last_news,ch->pcdata->last_changes	);
#else
	fprintf( fp, "Not  %d %d %d %d %d\n",
		ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_penalty,
		ch->pcdata->last_news,ch->pcdata->last_changes	);
#endif

	if ( ch->pcdata->security != 0 )
		fprintf( fp, "Sec %d\n",	ch->pcdata->security	);
        fprintf( fp, "Learn %d\n",	ch->pcdata->learn	);
        fprintf( fp, "Speak %d\n",	ch->pcdata->speaking	);
	fprintf( fp, "ClanS %d\n",	ch->pcdata->clan_status	);
	fprintf( fp, "MinClanS %d\n",	ch->pcdata->min_clan_status );
	fprintf( fp, "MaxClanS %d\n",	ch->pcdata->max_clan_status );

	if (ch->pcdata->color)
		fprintf( fp, "Color %d\n", ch->pcdata->color);

        for (sn = 0; sn< MAX_LANGUAGE; ++sn )
		fprintf ( fp, "Lang %d %d\n",sn,ch->pcdata->language[sn]);

	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
        fprintf( fp, "WhoTxt %s~\n",	ch->pcdata->who_text    );
    	fprintf( fp, "Pnts %d\n",	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "TAli %d\n",	ch->pcdata->true_align	);
	fprintf( fp, "TRac %d\n",	ch->true_race		);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit, 
						   ch->pcdata->perm_mana,
						   ch->pcdata->perm_move);
	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );

	fprintf( fp, "Incr %d\n",
	    ch->pcdata->incarnations );

	if (tiene_qdata(ch)) /* quests */
	{
		fprintf( fp, "Qpoints %d\n",	get_qpoints(ch) );
		fprintf( fp, "Qid %ld\n",	get_qid(ch) );
		fprintf( fp, "Qtype %d\n",	get_qtype(ch) );
		fprintf( fp, "Qestado %d\n",	get_qestado(ch) );
		fprintf( fp, "Qtimer %d\n",	get_qtimer(ch) );
	}

        fprintf( fp, "Muertes %d\n", ch->pcdata->muertes);
	/* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	for (pos = 0; pos < MAX_IGNORE; pos++)
		if (!IS_NULLSTR(ch->pcdata->ignore[pos]))
			fprintf( fp, "Ignorar %s~\n", ch->pcdata->ignore[pos] );

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
            {
                fprintf( fp, "Gr '%s'\n",group_table[gn].name);
            }
        }
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;
	
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
#if !defined(MSDOS)
    fprintf(fp,"LogO %ld\n", current_time);
#else
    fprintf(fp,"LogO %d\n",  current_time);
#endif
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->timer)
    	fprintf(fp,"Timer %d\n", pet->timer);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf( fp, "Clan %s~\n",get_clan_table(pet->clan)->name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (getNivelPr(pet) != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", getNivelPr(pet));
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest, ROOM_INDEX_DATA *room )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest, room );

    /*
     * Castrate storage characters.
     */
    if ( (ch && (getNivelPr(ch) < obj->level - 2) && obj->item_type != ITEM_CONTAINER)
    ||   obj->item_type == ITEM_KEY
    ||   obj->item_type == ITEM_CORPSE_PC
    ||	 IS_OBJ_STAT(obj, ITEM_PROTOTIPO)
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

/*  if (room && IS_SET(room->room_flags, ROOM_GRABADO))
    {
    	RESET_DATA *reset;

	for (reset = room->reset_first; reset; reset = reset->next )
		if (((reset->command == 'O') || (reset->command == 'P')) && (reset->arg1 == obj->pIndexData->vnum))
			return;
    } */

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    if (obj->pIndexData->max_count > 0)
    	fprintf( fp, "Id %d\n",	obj->id );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->owner && str_cmp(obj->owner,""))
        fprintf( fp, "Owner %s~\n",	obj->owner		     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );

    /* variable data */

    fprintf( fp, "Wear %d\n",		obj->wear_loc			);
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level			);
    if (obj->timer != 0)
	fprintf( fp, "Time %d\n",	obj->timer			);
    if (obj->detected > 0)
	fprintf( fp, "Detected %s\n",	print_flags(obj->detected)	);
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1, room );

    return;
}

void fwrite_corpse( FILE *fp, CHAR_DATA * ch )
{
    fprintf( fp, "Where %d\n", obj_whereis_vnum(ch->pcdata->corpse) );
    fwrite_obj( ch, ch->pcdata->corpse->contains, fp, 0, NULL );
    fprintf( fp, "#END\n" );
}

void save_corpse( CHAR_DATA *ch, bool bak )
{
	if ( ch->pcdata->corpse->contains )
	{
		FILE *fp;
		char buf[MIL];

		sprintf( buf, "%s%s", bak ? CORPSE_BAK_DIR : CORPSE_DIR, ch->name );

		fp = fopen( buf, "w" );

		if ( !fp )
		{
			perror( "save_corpse" );
			return;
		}

		fwrite_corpse( fp, ch );

		fclose( fp );
	}
}

bool fread_corpse( FILE *fp, CHAR_DATA *ch )
{
	OBJ_DATA *corpse;
	char buf[MIL];
	ROOM_INDEX_DATA *location = NULL;
	int iNest, vnum;
	char *temp;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	temp = fread_word( fp );

	if ( str_cmp( temp, "Where" ) )
	{
		bugf( "Cuerpo %s : where no encontrado", ch->name );
		return FALSE;
	}

	vnum = fread_number( fp );

	if ( vnum != -1 && vnum != ROOM_VNUM_LIMBO )
		location = get_room_index( vnum );

	if ( location == NULL )
		location = get_room_index( ROOM_VNUM_MORGUE );

	corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );

	sprintf( buf, "corpse cuerpo %s", ch->name );
	free_string( corpse->name );
	corpse->name = str_dup( buf );

	sprintf( buf, "el cuerpo de %s", ch->name );
	free_string( corpse->short_descr );
	corpse->short_descr = str_dup( buf );

	sprintf( buf, "El cuerpo de %s esta aqui.", ch->name );
	free_string( corpse->description );
	corpse->description = str_dup( buf );

	free_string( corpse->owner );
	corpse->owner = str_dup( ch->name );

	ch->pcdata->corpse = corpse;
	obj_to_room( corpse, location );

	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Fread_corpse: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );

	         if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( NULL, fp, corpse, NULL );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( NULL, fp, corpse, NULL );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bugf( "Load_char_obj: bad section (%s)", word );
		break;
	    }
	}

	flog( "Cuerpo de %s cargado.", ch->name );

	return TRUE;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    PLIST * pl = plist_lookup(name);
#if !defined(MSDOS) && defined(COMPRESS)
    char buf[100];
#endif
    CHAR_DATA *ch;
    FILE *fp;
    bool found, crcfail = FALSE;
    int i;

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );

#if defined(CRC)
    if (pl && pl->crc > 0)
    {
	WORD crc;

	crc = calcular_crc(strsave);
	
	if (crc != pl->crc)
	{
		bugf("load_char_obj : %s con crc invalido!", name);
		crcfail = TRUE;
	}
    }
#endif

    ch					= new_char();
    ch->pcdata				= new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();

    set_char(ch,0);

    found = FALSE;
    fclose( fpReserve );

#if defined(unix) && defined(COMPRESS)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
#endif

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp, NULL, NULL );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp, NULL, NULL );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }
    else
    	if (errno != ENOENT)
    		perror(strsave);

    fpReserve = fopen( NULL_FILE, "r" );

    /* inicializar datos varios */
    if (found)
    {
	MEM_DATA * mem;
	LEVEL_DATA * lev;

	if (crcfail == TRUE)
		SET_BIT(ch->act, PLR_DENY);

	if (pl == NULL)
		bugf("load_char_obj : jugador %s (nivel %d) no esta en la plist",
			ch->name, getNivelPr(ch) );

	if (getNivelPr(ch) == 0)
		setNivelPr(ch,1);

	if (ch->race == 0)
		ch->race		= RACE_HUMAN;

	if (ch->desc && ch->desc->term == -1)
		ch->desc->term = 0;

	if (ch->true_race == -1)
		ch->true_race	= ch->race;

	ch->exp = UMAX(ch->exp, EXP_NIVEL(ch, getNivelPr(ch) - 1));

	if ( ch->race != RACE_VAMPIRE
	&&  !IS_AFFECTED(ch, AFF_POLYMORPH)
	&&   ch->true_race != RACE_VAMPIRE
	&&  !IS_AFFECTED2(ch, AFF_VAMP_BITE)
	&&  (mem = mem_lookup( ch->memory, MEM_VAMPIRE )) )
	{
		bugf( "load_char_obj : char %s raza %s con MEM_VAMP",
			ch->name, NOM_RAZA(ch->race) );
		extract_mem( ch, mem );
	}

	if ( ch->race == RACE_VAMPIRE
	&&  !IS_AFFECTED(ch, AFF_POLYMORPH)
	&&   ch->true_race != RACE_VAMPIRE
	&&   mem_lookup(ch->memory, MEM_VAMPIRE) == NULL )
	{
		bugf( "load_char_obj : char %s vampiro sin memoria, race %d, true_race %d",
			ch->name, ch->race, ch->true_race );
		polymorph( ch, ch->true_race );
	}

	ch->size	= race_table[ch->race].size;

	if ( ch->pcdata->min_clan_status == -1 )
		ch->pcdata->min_clan_status = 0;
	if ( ch->pcdata->max_clan_status == -1 )
		ch->pcdata->max_clan_status = CLAN_DICTADOR;

	if ( race_table[ch->race].dam_type > 0 )
		ch->dam_type	= race_table[ch->race].dam_type;
	else
	{
		if ( IS_SET(ch->parts, PART_CLAWS) )
			ch->dam_type	= attack_lookup( "claw" );
		else
			ch->dam_type	= attack_lookup( "punch" );
	}

	for (i = 0; i < 5; i++)
	{
	    if (race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch,race_table[ch->race].skills[i],FALSE);
	}

	group_add(ch,"rom basics",FALSE);

	for ( lev = ch->level_data; lev; lev = lev->next )
	{
        	group_add(ch,class_table[lev->clase].base_group,FALSE);
		group_add(ch,class_table[lev->clase].default_group,FALSE);
		// no subiremos los puntos de creacion (espero)
	}

	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->affected2_by = ch->affected2_by|race_table[ch->race].aff2;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;

	ch->ent		= chToEntidad(ch, TRUE);

	REMOVE_BIT(ch->act, PLR_NOPK);
	REMOVE_BIT(ch->act, PLR_NOAUCTION);
    } /* found? */

    return found;
}


/*
 *  Read in a char.
 */

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int count = 0, igcount = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_flag( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "Af2By",	ch->affected2_by,	fread_flag( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->alias[count]        = str_dup(fread_word(fp));
                ch->pcdata->alias_sub[count]    = fread_string(fp);
                count++;
                fMatch = TRUE;
                break;
            }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
 
                paf->where  = fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    if ( !str_cmp( word, "Bank" ) )
	    {
		BANK_DATA *bank = new_bank();

		bank->valor		= fread_number(fp);
		bank->tipo		= fread_number(fp);
		bank->when		= (time_t) fread_number(fp);
		bank->start		= current_time;
		bank->interes		= (float) fread_number(fp) / 100.0;
		bank->next		= ch->pcdata->bank;
		ch->pcdata->bank	= bank;
		fMatch = TRUE;

		if ( bank->tipo		== BANK_DEPOSITO
		&&   bank->interes	!= INTERES_DEPOSITO )
			bank->interes = INTERES_DEPOSITO;

		break;
	    }
	    if ( !str_cmp(word, "Balance") )
	    {
	    	BANK_DATA *bank = new_bank();

		bank->valor		= fread_number(fp);
		bank->tipo		= BANK_DEPOSITO;
		bank->when		= current_time;
		bank->start		= current_time;
		bank->interes		= INTERES_DEPOSITO;
		bank->next		= ch->pcdata->bank;
		ch->pcdata->bank	= bank;
		fMatch = TRUE;
		break;
	    }
	    KEYS( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEYS( "Bin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    break;

	case 'C':
	    KEY_DO( "Class",	setClasePr(ch, fread_number(fp), -1) );
	    KEY_DO( "Cla",	setClasePr(ch, fread_number(fp), -1) );
	    KEY( "Clan",	ch->clan,		clan_lookup(fread_string(fp)));
            KEY( "ClanS",	ch->pcdata->clan_status, fread_number(fp) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
	    KEY("Comm",		ch->comm,		fread_flag( fp ) ); 
	    KEY("Color",	ch->pcdata->color,	fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Derrotas",	ch->pcdata->derrotas,	fread_number( fp ) );
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEYS( "Description",ch->description,	fread_string( fp ) );
	    KEYS( "Desc",	ch->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0
    		&&  !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit)   * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move) * percent / 100;
    		}
		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;

		temp	= fread_word( fp );
		gn	= group_lookup( temp );
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }
	    break;

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
	    KEY_IGNORE( "Hmtown",			fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
            KEY( "Incr",	ch->pcdata->incarnations, fread_number(fp) );
            KEY( "Imm",		ch->imm_flags,		fread_flag( fp ) );

	    if ( !str_cmp(word, "Ignore"))
	    {
	    	fread_to_eol(fp);
	    	fMatch = TRUE;
	    	break;
	    }

            if (!str_cmp( word, "Ignorar"))
            {
                if (igcount >= MAX_IGNORE)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->ignore[igcount]	= str_dup(fread_string(fp));
                igcount++;
                fMatch = TRUE;
                break;
            }
	    break;

	case 'L':
            if ( !str_cmp( word, "Lang" ) )
            {
                int gn;
                int temp;
 
                gn = fread_number( fp ) ;
                temp = fread_number ( fp );
                if ( gn > MAX_LANGUAGE )
                {
                    fprintf(stderr,"%i",temp);
                    bug( "Fread_char: unknown lang. ", 0 );
                }
                else
		    ch->pcdata->language[gn]=temp;
                fMatch = TRUE;
            }
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Learn",	ch->pcdata->learn,	fread_number( fp ) );
	    KEY_DO( "Level",	setNivelPr(ch, fread_number(fp)) );
	    KEY_DO( "Lev",	setNivelPr(ch, fread_number(fp)) );
	    KEY_DO( "Levl",	setNivelPr(ch, fread_number(fp)) );
	    KEY( "LogO",	lastlogoff,		fread_number( fp ) );
	    KEYS( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEYS( "LnD",	ch->long_descr,		fread_string( fp ) );
	    KEY( "Luck",	ch->luck,		fread_number( fp ) );
	    break;

        case 'M':
	    if ( !str_cmp( "Mem", word ) )
	    {
	    	time_t when;
	    	long id;
	    	int reaction;

		when		= fread_number(fp);
		reaction	= fread_flag(fp);
		id		= fread_number(fp);

		give_mem_when( ch, when, reaction, id );

		fMatch = TRUE;
		break;
	    }
            KEY( "Muertes",	ch->pcdata->muertes,	fread_number( fp ) );
            KEY_IGNORE( "Mental",			fread_number( fp ) );
	    KEYS( "Mensaje",	ch->pcdata->mensaje,	fread_string( fp ) );
	    KEY( "MinClanS",	ch->pcdata->min_clan_status, fread_number( fp ) );
	    KEY( "MaxClanS",	ch->pcdata->max_clan_status, fread_number( fp ) );
	    break;

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
	    KEY( "Note",	ch->pcdata->last_note,	fread_number( fp ) );
	    if (!str_cmp(word,"Nivel"))
	    {
	    	char * blah = fread_word(fp);
	    	int bleh = fread_number(fp), bloh = class_lookup(blah);

		if (bloh == -1)
			bugf("fread_char : clase %s no encontrada", blah );
		if (!ENTRE_I(1, bleh, MAX_LEVEL))
			bugf("fread_char : nivel %d inexistente", bleh );

		addClase(ch,
			bloh != -1 ? bloh : CLASS_WARRIOR,
			URANGE(1, bleh, MAX_LEVEL) );

		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word,"Not"))
	    {
		ch->pcdata->last_note			= fread_number(fp);
		ch->pcdata->last_idea			= fread_number(fp);
		ch->pcdata->last_penalty		= fread_number(fp);
		ch->pcdata->last_news			= fread_number(fp);
		ch->pcdata->last_changes		= fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "NBank" ) )
	    {
		BANK_DATA *bank = new_bank();

		bank->valor		= fread_number(fp);
		bank->tipo		= fread_number(fp);
		bank->when		= (time_t) fread_number(fp);
		bank->start		= (time_t) fread_number(fp);
		bank->interes		= (float) fread_number(fp) / 100.0;
		bank->next		= ch->pcdata->bank;
		ch->pcdata->bank	= bank;
		fMatch = TRUE;

		if ( bank->tipo		== BANK_DEPOSITO
		&&   bank->interes	!= INTERES_DEPOSITO )
			bank->interes = INTERES_DEPOSITO;

		break;
	    }
	    break;

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
            KEYS( "Prompt",	ch->pcdata->prompt,	fread_string( fp ) );
 	    KEYS( "Prom",	ch->pcdata->prompt,	fread_string( fp ) );
	    if ( !str_cmp( word, "Prob" ) )
	    {
	    	struct prob_data *prob;

		prob			= mud_malloc( sizeof( struct prob_data ) );
		prob->comando		= fread_string( fp );
		prob->next		= ch->pcdata->prohibido;
		ch->pcdata->prohibido	= prob;
		fMatch = TRUE;
		break;
	    }
	    break;

        case 'Q':
	    KEY( "Quaff",	ch->pcdata->quaff,	fread_number( fp ) );

	    KEY_DO( "QuestPnts",give_qpoints( ch, fread_number(fp) ) );
	    KEY_DO( "Qpoints",	give_qpoints( ch, fread_number(fp) ) );
	    KEY_DO( "Qtimer",	set_qtimer( ch, fread_number(fp) ) );
	    KEY_DO( "Qid",	set_qid( ch, fread_number(fp) ) );
	    KEY_DO( "Qestado",	set_qestado( ch, fread_number(fp) ) );
	    KEY_DO( "Qtype",	set_qtype( ch, fread_number(fp) ) );

	    KEY_IGNORE( "QuestNext", fread_number(fp) );
	    KEY_IGNORE( "QuestObj", fread_number(fp) );
	    KEY_IGNORE( "QuestMob", fread_number(fp) );
	    KEY_IGNORE( "QuestCount", fread_number(fp) );
	    KEY_IGNORE( "QuestWhere", fread_number(fp) );
            break;

	case 'R':
	    KEY( "Race",        ch->race,	
				race_lookup(fread_string( fp )) );
	    KEY( "Res",		ch->res_flags,		fread_flag( fp ) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		/* !! */
		ch->was_in_room = get_room_index( fread_number( fp ) );
		if ( ch->was_in_room == NULL )
			ch->was_in_room = get_room_index( ROOM_VNUM_LIMBO );
		else
		if ( IS_SET(ch->was_in_room->room_flags, ROOM_ARENA) )
			ch->was_in_room = get_room_index( ROOM_VNUM_ALTAR );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->pcdata->lines,	fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    if ( !str_cmp(word, "Shares") )
	    {
	    	BANK_DATA *bank		= new_bank();

		bank->valor		= fread_number( fp );
		bank->tipo		= BANK_ACCIONES;
		bank->when		= current_time;
		bank->start		= current_time;
		bank->interes		= 1;
		bank->next		= ch->pcdata->bank;
		ch->pcdata->bank	= bank;
		fMatch = TRUE;
		break;
	    }
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,	fread_string( fp ) );
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
            KEY( "Silv",        ch->silver,             fread_number( fp ) );
	    KEY( "Speak",	ch->pcdata->speaking,	fread_number( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );

		if ( value < 1 || value > 100 )
			value = 0;

		temp	= fread_word( fp ) ;
		sn	= skill_lookup(temp);

		if ( !str_cmp(temp, "sobrecargar") )
		{
			flog( "fread_char : dandole 4 trains y %d practicas a %s",
				value / 25, ch->name );
			ch->train += 4;
			ch->practice += value / 10;
		}

		if ( sn < 0 )
			bugf( "Fread_char: unknown skill (%s).", temp );
		else
			ch->pcdata->learned[sn] = value;

		fMatch = TRUE;
	    }
	    break;

	case 'T':
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "TAli",	ch->pcdata->true_align,	fread_number( fp ) );
	    KEY( "TRac",	ch->true_race,		fread_number( fp ) );
	    KEY( "Term",	ch->desc->term,		term_lookup( fread_string(fp) ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Victorias",	ch->pcdata->victorias,	fread_number ( fp ) );
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    KEY( "Vuln",	ch->vuln_flags,		fread_flag( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
            KEY( "WhoTxt",      ch->pcdata->who_text,   fread_string( fp ) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    bugf( "Fread_char: no match (%s)", word );
	    fread_to_eol( fp );
	}

	if (feof(fp))
	{
		bugf("fread_char : feof (jugador %s)", CHECKNULLSTR(ch->name) );
		return;
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
             KEY( "Clan",       pet->clan,       clan_lookup(fread_string(fp)));
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
	     KEY_DO( "Levl",	setNivelPr(pet, fread_number(fp)) );
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
	case 'T' :
	    KEY( "Timer",	pet->timer,		fread_number(fp));
	    break;

    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}

extern	OBJ_DATA	*obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp, OBJ_DATA *corpse, ROOM_INDEX_DATA *room )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );

    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
        OBJ_INDEX_DATA *pObj;

	first = FALSE;  /* fp will be in right place */

        vnum = fread_number( fp );
        if ( (pObj = get_obj_index( vnum )) == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    if (pObj->max_count > 0)
		obj = new_create_object(pObj,-1,FALSE);
	    else
	    	obj = new_create_object(pObj,-1,TRUE);
	    new_format = TRUE;
	}
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
	obj->owner		= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		char *blah;
		int sn;

		paf = new_affect();

		blah = fread_word(fp);

		sn = skill_lookup(blah);

		if (sn < 0)
		    bugf("Fread_obj: unknown skill (%s).", blah);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEYS( "Description",obj->description,	fread_string( fp ) );
	    KEYS( "Desc",	obj->description,	fread_string( fp ) );
	    KEY( "Detected",	obj->detected,		fread_flag( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    rgObjNest[iNest] = NULL;
		    return;
		}
		else
	        {
		    if ( !fVnum )
		    {
			free_obj( obj );
			obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
		    }

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj, TRUE);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }

		    if (obj->clan == 0
		    &&  obj->pIndexData->clan != 0)
		    	obj->clan = obj->pIndexData->clan;

		    if ( iNest == 0 || rgObjNest[iNest-1] == NULL )
		    {
		    	if ( corpse )
				obj_to_obj( obj, corpse );
		    	else if ( ch )
		    	{
		    		if (obj->clan > 0
		    		&&  obj->clan != ch->clan)
		    		{
		    			extract_obj(obj, TRUE);
		    			return;
		    		}
		    		else
		    			obj_to_char( obj, ch );
		    	}
		    	else
		    		obj_to_room( obj, room );
		    }
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );

		    if ( obj->id == -1 )
		    	obj->id = get_obj_id(obj);

		    if (es_obj_random(obj) && obj->timer == 0)
		    	obj->timer = number_range(50, 400);

		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "Id",		obj->id,		fread_number( fp ) );
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEYS( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    KEYS( "Owner",	obj->owner,		fread_string( fp ) );
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    

	case 'S':
	    KEYS( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEYS( "ShD",	obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void fread_room( ROOM_INDEX_DATA *room )
{
	FILE *fp;
	char cname[MIL];
	int iNest;

	sprintf( cname, "%s%d", ROOM_DIR, room->vnum );

	if ( ( fp = fopen( cname, "r" )) == NULL )
		return;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Fread_room: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );

	         if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( NULL, fp, NULL, room );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( NULL, fp, NULL, room );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}

	fclose( fp );
	unlink( cname );
}

void fwrite_room( ROOM_INDEX_DATA *room )
{
	FILE *fp;
	char buf[MIL];

	sprintf( buf, "%s%d", ROOM_DIR, room->vnum );

	if (!room->contents)
	{
		unlink(buf);
		return;
	}

	if ( (fp = fopen( buf, "w" )) == NULL )
		return;

	fwrite_obj( NULL, room->contents, fp, 0, room );

	fprintf( fp, "#END\n" );

	fclose( fp );
	return;
}
