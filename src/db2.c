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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#endif

#include "merc.h"
#include "db.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "olc.h"

SCRIPT_DATA * script_pedir( int vnum );

/* values for db2.c */
struct		social_type	*social_table;

/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
        pMobIndex->area                 = area_last;               /* OLC */
	pMobIndex->new_format		= TRUE;
	pMobIndex->norecalc		= FALSE;
	pMobIndex->clase		= -1;
	newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );

#if defined(AREA_FIX)
	if ( IS_NULLSTR(pMobIndex->player_name)
	  || IS_NULLSTR(pMobIndex->short_descr)
	  || IS_NULLSTR(pMobIndex->long_descr) )
		bug( "Load_mobiles : mob %d con nom, short o long desc NULL.", pMobIndex->vnum );

	if ( !str_prefix("oldstyle ", pMobIndex->player_name) )
	{
		bugf( "load_mobiles : mob %d con prefix oldstyle", pMobIndex->vnum );
		pMobIndex->player_name = &pMobIndex->player_name[9];
	}
#endif

	if (pMobIndex->area->version == 0)
		pMobIndex->race	= race_lookup(fread_string(fp));
	else
		pMobIndex->race = race_lookup(fread_word(fp));

        if ( pMobIndex->race == 0 )
        {
        	bug( "Load_mobiles : mob %d con raza invalida", pMobIndex->vnum );
        	pMobIndex->race = RACE_HUMAN;
	}

        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
        
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;

        if (IS_SET(area_last->area_flags, AREA_PROTOTIPO))
           SET_BIT(pMobIndex->act, ACT_PROTOTIPO);
        else
           REMOVE_BIT(pMobIndex->act, ACT_PROTOTIPO);

        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;

	if ( IS_SET( area_last->area_flags, AREA_ROM_OLD ) )
		pMobIndex->affected2_by		= race_table[pMobIndex->race].aff2;
	else
		pMobIndex->affected2_by		= fread_flag( fp ) | race_table[pMobIndex->race].aff2;

	REMOVE_BIT(pMobIndex->affected2_by, AFF_VAMP_BITE);
	REMOVE_BIT(pMobIndex->affected2_by, AFF_AMNESIA);
	REMOVE_BIT(pMobIndex->affected2_by, AFF_HOLD);
	REMOVE_BIT(pMobIndex->affected2_by, AFF_ESTUPIDEZ);
	REMOVE_BIT(pMobIndex->affected2_by, AFF_GHOUL);
	REMOVE_BIT(pMobIndex->affected_by, AFF_FAERIE_FIRE);

        pMobIndex->pShop                = NULL;
        pMobIndex->pRepair		= NULL;
        pMobIndex->alignment            = fread_number( fp );
        pMobIndex->group                = fread_number( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= attack_lookup(fread_word(fp));

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flag( fp ) 
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].imm;
	pMobIndex->res_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].res;
	pMobIndex->vuln_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].vuln;

	/* vital statistics */
	pMobIndex->start_pos		= position_lookup(fread_word(fp));
	pMobIndex->default_pos		= position_lookup(fread_word(fp));
	pMobIndex->sex			= sex_lookup(fread_word(fp));

	pMobIndex->wealth		= fread_number( fp );

	if (pMobIndex->wealth > 1000 * pMobIndex->level)
	{
		bugf( "load_mobiles : mob %d con wealth %d",
			pMobIndex->vnum, pMobIndex->wealth );
		pMobIndex->wealth = 1000 * pMobIndex->level;
	}

	pMobIndex->form			= fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts		= fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	pMobIndex->size			= size_lookup(fread_word(fp));
	pMobIndex->material		= str_dup(fread_word( fp ));
 
	for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'F')
            {
		char *word;
		long vector;

                word                    = fread_word(fp);
		vector			= fread_flag(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
                else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"off"))
		    REMOVE_BIT(pMobIndex->off_flags,vector);
		else if (!str_prefix(word,"imm"))
		    REMOVE_BIT(pMobIndex->imm_flags,vector);
		else if (!str_prefix(word,"res"))
		    REMOVE_BIT(pMobIndex->res_flags,vector);
		else if (!str_prefix(word,"vul"))
		    REMOVE_BIT(pMobIndex->vuln_flags,vector);
		else if (!str_prefix(word,"for"))
		    REMOVE_BIT(pMobIndex->form,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else
		{
		    bug("Flag remove: flag not found.",0);
		    exit(1);
		}
	     }
	     else if ( letter == 'M' )
	     {
		MPROG_LIST *pMprog;
		char *word;
		int trigger = 0;
		
		pMprog              = alloc_perm(sizeof(*pMprog));
		word   		    = fread_word( fp );
		if ( (trigger = flag_value( mprog_flags, word )) == NO_FLAG )
		{
		    bug("MOBprogs: invalid trigger.",0);
		    exit(1);
		}
		SET_BIT( pMobIndex->mprog_flags, trigger );
		pMprog->trig_type   = trigger;
		pMprog->vnum        = fread_number( fp );
		pMprog->trig_phrase = fread_string( fp );
		pMprog->next        = pMobIndex->mprogs;
		pMobIndex->mprogs   = pMprog;
             }
	     else if (letter == 'C') // clase
	     {
		pMobIndex->clase	= class_lookup(fread_word(fp));
	     }
	     else if (letter == 'N')
	     {
	     	pMobIndex->clan	     = clan_lookup( fread_string( fp ) );
	     }
	     else if (letter == 'L')
	     {
	     	pMobIndex->clan	= clan_lookup(fread_word(fp));
	     }
	     else if (letter == 'G')
	     {
	     	fread_string( fp );
	     }
	     else if (letter == 'S')
		pMobIndex->script = script_pedir( fread_number(fp) );
	     else if (letter == 'R')
	     {
		pMobIndex->norecalc = TRUE;
	     }
	     else
	     {
		ungetc(letter,fp);
		break;
	     }
	}

	if (pMobIndex->clase == -1)
	{
		if ( IS_SET(pMobIndex->act, ACT_MAGE) )
			pMobIndex->clase = CLASS_MAGE;
		else
		if ( IS_SET(pMobIndex->act, ACT_THIEF) )
			pMobIndex->clase = CLASS_THIEF;
		else
		if ( IS_SET(pMobIndex->act, ACT_PSI) )
			pMobIndex->clase = CLASS_PSI;
		else
		if ( IS_SET(pMobIndex->act, ACT_WARRIOR)
		&&   IS_SET(pMobIndex->act, ACT_CLERIC) )
			pMobIndex->clase = CLASS_RANGER;
		else
		if ( IS_SET(pMobIndex->act, ACT_CLERIC) )
			pMobIndex->clase = CLASS_CLERIC;
		else
		if ( IS_SET(pMobIndex->act, ACT_WARRIOR) )
			pMobIndex->clase = CLASS_WARRIOR;
		else
		{
			int clase = CLASS_WARRIOR;

			if ( IS_SET(pMobIndex->form, FORM_SENTIENT) )
			{
				switch(number_range(0,5))
				{
					case 0:	clase = CLASS_WARRIOR;	break;
					case 1: clase = CLASS_RANGER;	break;
					case 2: clase = CLASS_CLERIC;	break;
					case 3: clase = CLASS_THIEF;	break;
					case 4: clase = CLASS_PSI;	break;
					case 5: clase = CLASS_MAGE;	break;
				}
			}
			else // animal tonto
			{
				switch(number_range(0,1))
				{
					case 0: clase = CLASS_WARRIOR;	break;
					case 1: clase = CLASS_THIEF;	break;
				}
			}
			pMobIndex->clase = clase;
		}
	}

	if (IS_SET(area_last->area_flags, AREA_RECALC) && !pMobIndex->norecalc)
		recalc(pMobIndex);

	iHash			= vnum % MAX_KEY_HASH;
	pMobIndex->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMobIndex;
	top_mob_index++;
	top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
	assign_area_vnum( vnum );                                  /* OLC */
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
#if defined(AREA_FIX)
    int pos;
#endif
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum                 = vnum;
        pObjIndex->area                 = area_last;            /* OLC */
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
	pObjIndex->material_string	= fread_string(fp);
	pObjIndex->material		= mat_lookup(pObjIndex->material_string);

#if defined(AREA_FIX)
	if (pObjIndex->material == -1)
	{
		char buf[MIL];
		sprintf(buf,"material %s invalido, vnum %d", pObjIndex->material_string, pObjIndex->vnum);
//		bugf("load_objects : %s", buf);
		append_file(NULL, DATA_DIR"materiales", buf);
	}

	if ( IS_NULLSTR(pObjIndex->name)
	  || IS_NULLSTR(pObjIndex->short_descr)
	  || IS_NULLSTR(pObjIndex->description) )
		bug( "Load_objects : obj %d sin nom, short o desc.", pObjIndex->vnum );
#endif

        pObjIndex->item_type            = item_lookup(fread_word( fp ));
        pObjIndex->extra_flags          = fread_flag( fp );

#if defined(AREA_FIX)
	if (IS_OBJ_STAT(pObjIndex, ITEM_MELT_DROP)
	&&  IS_OBJ_STAT(pObjIndex, ITEM_NODROP) )
		bugf( "load_objects : obj %d con MELT_DROP y NODROP",
			pObjIndex->vnum );
#endif

        if (IS_SET(area_last->area_flags, AREA_PROTOTIPO))
           SET_BIT(pObjIndex->extra_flags, ITEM_PROTOTIPO);
        else
           REMOVE_BIT(pObjIndex->extra_flags, ITEM_PROTOTIPO);

	pObjIndex->wear_flags           = fread_flag( fp );

	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);

	    if (pObjIndex->value[3] < 0)
	    {
	    	bugf( "load_objects : obj %d con damtype invalido",
	    		pObjIndex->vnum );
	    }
	    else
	    if ( pObjIndex->value[0] != WEAPON_EXOTIC
	    &&   attack_table[pObjIndex->value[3]].damage != DAM_SLASH
	    &&   attack_table[pObjIndex->value[3]].damage != DAM_BASH
	    &&   attack_table[pObjIndex->value[3]].damage != DAM_PIERCE )
	    {
		bugf( "load_objects : obj %d con ataque %s",
			pObjIndex->vnum,
			attack_table[pObjIndex->value[3]].name );
	    }

	    if ( (pObjIndex->value[0] == WEAPON_MACE
	      ||  pObjIndex->value[0] == WEAPON_WHIP
	      ||  pObjIndex->value[0] == WEAPON_FLAIL
	      ||  pObjIndex->value[0] == WEAPON_POLEARM)
	    &&  IS_WEAPON_STAT(pObjIndex, WEAPON_SHARP) )
	    {
	    	bugf( "load_objects : obj %d con WEAPON_SHARP", pObjIndex->vnum );
	    	REMOVE_BIT(pObjIndex->value[4], WEAPON_SHARP);
	    }
	    break;

	case ITEM_CONTAINER:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);

#if defined(AREA_FIX)
	if ( !pObjIndex->value[4]
	&&   !IS_OBJ_STAT(pObjIndex, ITEM_PROTOTIPO)
	&&    IS_SET(pObjIndex->wear_flags, ITEM_TAKE) )
	{
		bug( "Load_objects : container vnum %d con mult de peso 0", pObjIndex->vnum );
/*		pObjIndex->value[4] = number_range( 100 - pObjIndex->level, 100 ); */
	}
#endif
	    break;

        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
            pObjIndex->value[2]         = liq_lookup(fread_word(fp));
            if (pObjIndex->value[2] == -1)
	    {
            	pObjIndex->value[2] = 0;
            	bug("Liquido desconocido",0);
            }
            pObjIndex->value[3]         = fread_number(fp);
            pObjIndex->value[4]         = fread_number(fp);
            break;

	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;

	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;

	case ITEM_FUMABLE:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    break;

	default:
            pObjIndex->value[0]		= fread_flag( fp );
            pObjIndex->value[1]		= fread_flag( fp );
            pObjIndex->value[2]		= fread_flag( fp );
            pObjIndex->value[3]		= fread_flag( fp );
	    pObjIndex->value[4]		= fread_flag( fp );
	    break;
	}

	pObjIndex->level		= abs ( fread_number( fp ) );
        pObjIndex->weight               = abs ( fread_number( fp ) );
        pObjIndex->cost                 = abs ( fread_number( fp ) );
       	pObjIndex->trap			= NULL;
       	pObjIndex->clan			= 0;

#if defined(AREA_FIX)
	if ( !pObjIndex->weight
	&&   !IS_OBJ_STAT(pObjIndex, ITEM_PROTOTIPO)
	&&    IS_SET(pObjIndex->wear_flags, ITEM_TAKE)
	&&    pObjIndex->item_type != ITEM_MONEY )
	{
		bug( "Load_objects : vnum %d con peso 0", pObjIndex->vnum );
/*		pObjIndex->weight = number_range((pObjIndex->level + 1) * 2, (pObjIndex->level + 1) * 5 ); */
	}
#endif

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}

#if defined(AREA_FIX)
	if ( !IS_OBJ_STAT(pObjIndex, ITEM_PROTOTIPO) )
	{
		pos = itemtablepos( pObjIndex->item_type );

		if ( pos == -1 )
			bug( "Obj %d : itemtablepos == -1!", pObjIndex->vnum );
		else if ( (pObjIndex->wear_flags & item_table[pos].min_wear_flags) != item_table[pos].min_wear_flags )
			bug( "Obj %d sin min_wear_flags", pObjIndex->vnum );
	}
#endif

        for ( ; ; )
        {
            char letra;
 
            letra = fread_letter( fp );
 
            if ( letra == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		paf->where		= TO_OBJECT;
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );

		if ( paf->location == APPLY_SPELL_AFFECT )
			bug( "Load_objects : objeto %d con APPLY_SPELL_AFFECT", pObjIndex->vnum );

                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }

	    else if (letra == 'F')
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		letra 			= fread_letter(fp);
		switch (letra)
	 	{
		case 'A':
                    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where		= TO_IMMUNE;
		    break;
		case 'R':
		    paf->where		= TO_RESIST;
		    break;
		case 'V':
		    paf->where		= TO_VULN;
		    break;
		case 'P':
		    paf->where		= TO_PARTS;
		    break;
		default:
            	    bug( "Load_objects: Bad where on flag set.", 0 );
            	   exit( 1 );
		}
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
                paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flag(fp);
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }
 
            else if ( letra == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }
 
	    else if ( letra == 'T' )
	    {
		pObjIndex->trap			= new_trap();
		pObjIndex->trap->trap_eff	= fread_number( fp );
		pObjIndex->trap->trap_dam	= fread_number( fp );
		pObjIndex->trap->trap_charge	= fread_number( fp );
	    }

	    else if ( letra == 'C' )
	    	pObjIndex->clan			= clan_lookup( fread_string( fp ) );

		else if (letra == 'N')
			pObjIndex->clan	= clan_lookup(fread_word(fp));

	    else if ( letra == 'M' )
	    {
		MPROG_LIST *pMprog;
		char *word;
		int trigger = 0;

		pMprog              = alloc_perm(sizeof(*pMprog));
		word   		    = fread_word( fp );
		if ( (trigger = flag_lookup( word, oprog_flags )) == NO_FLAG )
		{
		    bug("OBJprogs: invalid trigger.",0);
		    exit(1);
		}
		SET_BIT( pObjIndex->mprog_flags, trigger );
		pMprog->trig_type   = trigger;
		pMprog->vnum        = fread_number( fp );
		pMprog->trig_phrase = fread_string( fp );
		pMprog->next        = pObjIndex->mprogs;
		pObjIndex->mprogs   = pMprog;
	    }

	    else if ( letra == 'R' )
	    {
		fread_number( fp );
	    	pObjIndex->max_count	= fread_number( fp );
	    }

	    else if ( letra == 'L' )
	    {
	    	LIMIT_DATA * limit;

		limit			= new_limit();
		limit->carrier_id	= fread_number(fp);
		limit->id		= fread_number(fp);

		limit->next		= pObjIndex->limit;
		pObjIndex->limit	= limit;
	    }

            else
            {
                ungetc( letra, fp );
                break;
            }
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );                                   /* OLC */
    }
 
    return;
}

#if defined(OLDSTYLE)
/*****************************************************************************
 Name:	        convert_objects
 Purpose:	Converts all old format objects to new format
 Called by:	boot_db (db.c).
 Note:          Loops over all resets to find the level of the mob
                loaded before the object to determine the level of
                the object.
		It might be better to update the levels in load_resets().
		This function is not pretty.. Sorry about that :)
 Author:        Hugin
 ****************************************************************************/
void convert_objects( void )
{
    int vnum;
    AREA_DATA  *pArea;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;

    if ( newobjs == top_obj_index ) return; /* all objects in new format */

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	{
	    if ( !( pRoom = get_room_index( vnum ) ) ) continue;

	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	    {
		switch ( pReset->command )
		{
		case 'M':
		    if ( !( pMob = get_mob_index( pReset->arg1 ) ) )
			bug( "Convert_objects: 'M': bad vnum %d.", pReset->arg1 );
		    break;

		case 'O':
		    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
		    {
			bug( "Convert_objects: 'O': bad vnum %d.", pReset->arg1 );
			break;
		    }

		    if ( pObj->new_format )
			continue;

		    if ( !pMob )
		    {
			bug( "Convert_objects: 'O': No mob reset yet.", 0 );
			break;
		    }

		    pObj->level = pObj->level < 1 ? pMob->level - 2
			: UMIN(pObj->level, pMob->level - 2);
		    break;

		case 'P':
		    {
			OBJ_INDEX_DATA *pObjN, *pObjTo;

			if ( !( pObjN = get_obj_index( pReset->arg1 ) ) )
			{
			    bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg1 );
			    break;
			}

			if ( pObjN->new_format )
			    continue;

			if ( !( pObjTo = get_obj_index( pReset->arg3 ) ) )
			{
			    bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg3 );
			    break;
			}

			pObjN->level = pObjN->level < 1 ? pObjTo->level
			    : UMIN(pObjN->level, pObjTo->level);
		    }
		    break;

		case 'G':
		case 'E':
		    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
		    {
			bug( "Convert_objects: 'E' or 'G': bad vnum %d.", pReset->arg1 );
			break;
		    }

		    if ( !pMob )
		    {
			bug( "Convert_objects: 'E' or 'G': null mob for vnum %d.",
			     pReset->arg1 );
			break;
		    }

		    if ( pObj->new_format )
			continue;

		    if ( pMob->pShop )
		    {
			switch ( pObj->item_type )
			{
			default:
			    pObj->level = UMAX(0, pObj->level);
			    break;
			case ITEM_PILL:
			case ITEM_POTION:
			    pObj->level = UMAX(5, pObj->level);
			    break;
			case ITEM_SCROLL:
			case ITEM_ARMOR:
			case ITEM_WEAPON:
			    pObj->level = UMAX(10, pObj->level);
			    break;
			case ITEM_WAND:
			case ITEM_TREASURE:
			    pObj->level = UMAX(15, pObj->level);
			    break;
			case ITEM_STAFF:
			    pObj->level = UMAX(20, pObj->level);
			    break;
			}
		    }
		    else
			pObj->level = pObj->level < 1 ? pMob->level
			    : UMIN( pObj->level, pMob->level );
		    break;
		} /* switch ( pReset->command ) */
	    }
	}
    }

    /* do the conversion: */

    for ( pArea = area_first; pArea ; pArea = pArea->next )
	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	    if ( (pObj = get_obj_index( vnum )) )
 		if ( !pObj->new_format )
		    convert_object( pObj );

    return;
}



/*****************************************************************************
 Name:		convert_object
 Purpose:	Converts an old_format obj to new_format
 Called by:	convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_object( OBJ_INDEX_DATA *pObjIndex )
{
    int level;
    int number, type;  /* for dice-conversion */

    if ( !pObjIndex || pObjIndex->new_format ) return;

    level = pObjIndex->level;

    pObjIndex->level    = UMAX( 0, pObjIndex->level ); /* just to be sure */
    pObjIndex->cost     = 10*level;

    switch ( pObjIndex->item_type )
    {
        default:
            bug( "Obj_convert: vnum %d bad type.", pObjIndex->vnum );
            break;

        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
	    break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
	    break;

        case ITEM_WEAPON:

	    /*
	     * The conversion below is based on the values generated
	     * in one_hit() (fight.c).  Since I don't want a lvl 50 
	     * weapon to do 15d3 damage, the min value will be below
	     * the one in one_hit, and to make up for it, I've made 
	     * the max value higher.
	     * (I don't want 15d2 because this will hardly ever roll
	     * 15 or 30, it will only roll damage close to 23.
	     * I can't do 4d8+11, because one_hit there is no dice-
	     * bounus value to set...)
	     *
	     * The conversion below gives:

	     level:   dice      min      max      mean
	       1:     1d8      1( 2)    8( 7)     5( 5)
	       2:     2d5      2( 3)   10( 8)     6( 6)
	       3:     2d5      2( 3)   10( 8)     6( 6)
	       5:     2d6      2( 3)   12(10)     7( 7)
	      10:     4d5      4( 5)   20(14)    12(10)
	      20:     5d5      5( 7)   25(21)    15(14)
	      30:     5d7      5(10)   35(29)    20(20)
	      50:     5d11     5(15)   55(44)    30(30)

	     */

	    number = UMIN(level/4 + 1, 5);
	    type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
	    break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
	    break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
	    pObjIndex->value[0] = pObjIndex->cost;
	    break;
    }

    pObjIndex->new_format = TRUE;
    ++newobjs;

    return;
}




/*****************************************************************************
 Name:		convert_mobile
 Purpose:	Converts an old_format mob into new_format
 Called by:	load_old_mob (db.c).
 Note:          Dug out of create_mobile (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_mobile( MOB_INDEX_DATA *pMobIndex )
{
    int i;
    int type, number, bonus;
    int level;

    if ( !pMobIndex || pMobIndex->new_format ) return;

    level = pMobIndex->level;

    pMobIndex->act              |= ACT_WARRIOR;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

       level:     dice         min         max        diff       mean
         1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
	 2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
	 3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
	 5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
	10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
	15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
	30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
	50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

	The values in parenthesis give the values generated in create_mobile.
        Diff = max - min.  Mean is the arithmetic mean.
	(hmm.. must be some roundoff error in my calculations.. smurfette got
	 1d6+23 hp at level 3 ? -- anyway.. the values above should be
	 approximately right..)
     */
    type   = level*level*27/40;
    number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*(8 + level)*.9 - number*type);

    pMobIndex->hit[DICE_NUMBER]    = number;
    pMobIndex->hit[DICE_TYPE]      = type;
    pMobIndex->hit[DICE_BONUS]     = bonus;

    pMobIndex->mana[DICE_NUMBER]   = level;
    pMobIndex->mana[DICE_TYPE]     = 10;
    pMobIndex->mana[DICE_BONUS]    = 100;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
    type   = level*7/4;
    number = UMIN(type/8 + 1, 5);
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*9/4 - number*type);

    pMobIndex->damage[DICE_NUMBER] = number;
    pMobIndex->damage[DICE_TYPE]   = type;
    pMobIndex->damage[DICE_BONUS]  = bonus;

    switch ( number_range( 1, 3 ) )
    {
        case (1): pMobIndex->dam_type =  3;       break;  /* slash  */
        case (2): pMobIndex->dam_type =  7;       break;  /* pound  */
        case (3): pMobIndex->dam_type = 11;       break;  /* pierce */
    }

    for (i = 0; i < 3; i++)
        pMobIndex->ac[i]         = interpolate( level, 100, -100);
    pMobIndex->ac[3]             = interpolate( level, 100, 0);    /* exotic */

    pMobIndex->wealth           /= 100;
    pMobIndex->size              = SIZE_MEDIUM;
    pMobIndex->material          = str_dup("none");

    pMobIndex->new_format        = TRUE;
    ++newmobs;

    return;
}
#endif // OLDSTYLE

void recalc( MOB_INDEX_DATA *pMob )
{
	int hplev, aclev, damlev, hitbonus;
	int i, cnt = 0, clase[10];
	float n;
	char buf[MIL];

	if ( pMob->level == 0 )
		return;

	hplev = 0; aclev = 0; damlev = 0; hitbonus = 0;

	if ( IS_PET(pMob) )
	{
		pMob->imm_flags = 0;
		REMOVE_BIT(pMob->affected_by, AFF_HIDE);
		REMOVE_BIT(pMob->affected_by, AFF_SNEAK);
		REMOVE_BIT(pMob->affected_by, AFF_INVISIBLE);
	}

#if defined(AREA_FIX)
	if ( ( !str_infix("thief",  pMob->player_name)
	    || !str_infix("midget", pMob->player_name) )
	&&     !IS_SET(pMob->act, ACT_THIEF) )
	{
		sprintf(buf,"mob %d sin ACT_THIEF",pMob->vnum);
		append_file(NULL,DATA_DIR"badact",buf);
	}

	if ( ( !str_infix("mage",   pMob->player_name)
	    || !str_infix("wizard", pMob->player_name) )
	&&     !IS_SET(pMob->act, ACT_MAGE) )
	{
		sprintf(buf,"mob %d sin ACT_MAGE",pMob->vnum);
		append_file(NULL,DATA_DIR"badact",buf);
	}

	if ( ( !str_infix("cleric", pMob->player_name)
	    || !str_infix("monk",   pMob->player_name)
	    || !str_infix("priest", pMob->player_name) )
	&&     !IS_SET(pMob->act, ACT_CLERIC) )
	{
		sprintf(buf,"mob %d sin ACT_CLERIC",pMob->vnum);
		append_file(NULL,DATA_DIR"badact",buf);
	}

        if ( !str_infix("ranger",pMob->player_name)
	&& ( !IS_SET(pMob->act, ACT_WARRIOR)
	  || !IS_SET(pMob->act, ACT_CLERIC)) )
	{
		sprintf(buf,"mob %d sin ACT_WARRIOR|ACT_CLERIC",pMob->vnum);
		append_file(NULL,DATA_DIR"badact",buf);
	}

	if ( ( !str_infix("warrior", pMob->player_name)
	    || !str_infix("guard",   pMob->player_name)
	    || !str_infix("soldier", pMob->player_name)
	    || !str_infix("fighter", pMob->player_name) )
	&&     !IS_SET(pMob->act, ACT_WARRIOR) )
	{
		sprintf(buf,"mob %d sin ACT_WARRIOR",pMob->vnum);
		append_file(NULL,DATA_DIR"badact",buf);
	}
#endif

	if ( !IS_PET(pMob) )
	{
		if ( !IS_SET(pMob->act, ACT_WARRIOR)
		  && !IS_SET(pMob->act, ACT_CLERIC)
		  && !IS_SET(pMob->act, ACT_THIEF)
		  && !IS_SET(pMob->act, ACT_MAGE)
		  && !IS_SET(pMob->act, ACT_PSI)
		  && (!fBootDb || pMob->level > 20) )
		{
			int num = number_range(0,4);

			switch(num)
			{
				case 0:	SET_BIT( pMob->act, ACT_WARRIOR );	break;
				case 1: SET_BIT( pMob->act, ACT_MAGE );	break;
				case 2: SET_BIT( pMob->act, ACT_CLERIC);	break;
				case 3: SET_BIT( pMob->act, ACT_THIEF );	break;
				case 4: SET_BIT( pMob->act, ACT_PSI );	break;
			}
		}

		CLEAR_OFF(pMob);

		if ( IS_SET(pMob->act, ACT_WARRIOR) )
		{
			SET_BIT(pMob->off_flags, OFF_PARRY);
			SET_BIT(pMob->off_flags, OFF_RESCUE);

			if ( pMob->level > 6 )
				SET_BIT(pMob->off_flags, OFF_BASH);

			if ( pMob->level > 7
			&&   IS_SET(pMob->parts, PART_LEGS) )
				SET_BIT(pMob->off_flags, OFF_KICK);

			if ( pMob->level > 10 )
				SET_BIT(pMob->off_flags, OFF_DISARM);

			if ( pMob->level > 12 )
				SET_BIT(pMob->off_flags, OFF_DODGE);

			if ( pMob->level > 17 )
				SET_BIT(pMob->off_flags, OFF_BERSERK);
		}

		if ( IS_SET(pMob->act, ACT_THIEF) )
		{
			SET_BIT(pMob->off_flags, OFF_TRIP);
			SET_BIT(pMob->off_flags, OFF_BACKSTAB);
			SET_BIT(pMob->off_flags, OFF_DODGE);

			if ( pMob->level > 2
			&&   IS_SET(pMob->parts, PART_LEGS) )
				SET_BIT(pMob->off_flags, OFF_KICK_DIRT);

			if ( pMob->level > 5
			&&   IS_SET(pMob->parts, PART_TAIL) )
				SET_BIT(pMob->off_flags, OFF_TAIL);

			if ( pMob->level > 11 )
				SET_BIT(pMob->off_flags, OFF_DISARM);

			if ( pMob->level > 12 )
				SET_BIT(pMob->off_flags, OFF_PARRY);

			if ( pMob->level > 13
			&&   IS_SET(pMob->parts, PART_LEGS) )
				SET_BIT(pMob->off_flags, OFF_KICK);

			if ( pMob->level > 14 )
				SET_BIT(pMob->off_flags, OFF_CIRCLE);
		}

		if ( IS_SET(pMob->act, ACT_MAGE) )
		{
			if ( pMob->level > 19 )
				SET_BIT(pMob->off_flags, OFF_DODGE);
			if ( pMob->level > 20
			&&   IS_SET(pMob->parts, PART_TAIL) )
				SET_BIT(pMob->off_flags, OFF_TAIL);
			if ( pMob->level > 21 )
				SET_BIT(pMob->off_flags, OFF_PARRY);
		}

		if ( IS_SET(pMob->act, ACT_CLERIC) )
		{
			if ( pMob->level > 11
			&&   IS_SET(pMob->parts, PART_LEGS) )
				SET_BIT(pMob->off_flags, OFF_KICK);
			if ( pMob->level > 19 )
				SET_BIT(pMob->off_flags, OFF_PARRY);
			if ( pMob->level > 20 
			&&   IS_SET(pMob->parts, PART_TAIL) )
				SET_BIT(pMob->off_flags, OFF_TAIL);
			if ( pMob->level > 21 )
				SET_BIT(pMob->off_flags, OFF_DODGE);
		}

		if ( IS_SET(pMob->act, ACT_PSI) )
		{
			if ( pMob->level > 17 )
			{
				SET_BIT(pMob->off_flags, OFF_PARRY);
				if ( IS_SET(pMob->parts, PART_TAIL) )
					SET_BIT(pMob->off_flags, OFF_TAIL);
				if ( IS_SET(pMob->parts, PART_LEGS) )
					SET_BIT(pMob->off_flags, OFF_KICK);
			}
			if ( pMob->level > 19 )
				SET_BIT(pMob->off_flags, OFF_DODGE);
		}

		if ( IS_SET(pMob->act, ACT_WARRIOR) )
			SET_BIT(pMob->off_flags, ASSIST_GUARD);

		SET_BIT(pMob->off_flags, ASSIST_VNUM);

		if ( !IS_SET(pMob->parts, PART_LEGS) )
		{
			REMOVE_BIT(pMob->off_flags, OFF_KICK);
			REMOVE_BIT(pMob->off_flags, OFF_KICK_DIRT);
		}
	}
	else
	{
		REMOVE_BIT(pMob->off_flags, OFF_DISARM);
		REMOVE_BIT(pMob->off_flags, OFF_KICK_DIRT);
	}

	if ( IS_SET(pMob->act, ACT_WARRIOR) )
	{
		hplev += 1;
		clase[cnt++] = ACT_WARRIOR;
		aclev += 2; // !!!
	}

	if ( IS_SET(pMob->act, ACT_THIEF) )
	{
		hplev -= 1; aclev -= 1; damlev -= 1;
		clase[cnt++] = ACT_THIEF;
	}

	if ( IS_SET(pMob->act, ACT_CLERIC) )
	{
		damlev -= 2;
		clase[cnt++] = ACT_CLERIC;
	}

	if ( IS_SET(pMob->act, ACT_MAGE) )
	{
//		hplev -= 1; aclev -= 1; damlev -= 3;
		hplev -= 2; aclev -= 1; damlev -= 3;
		clase[cnt++] = ACT_MAGE;
	}

	if ( IS_SET(pMob->act, ACT_PSI) )
	{
		hplev -= 2; damlev -= 1;
		clase[cnt++] = ACT_PSI;
	}

	if (cnt > 1)
	{
		flog( "recalc : mob %d con mas de una clase",
			pMob->vnum );

		REMOVE_BIT(pMob->act, ACT_CLERIC);
		REMOVE_BIT(pMob->act, ACT_MAGE);
		REMOVE_BIT(pMob->act, ACT_THIEF);
		REMOVE_BIT(pMob->act, ACT_WARRIOR);
		REMOVE_BIT(pMob->act, ACT_PSI);

		SET_BIT(pMob->act, clase[number_range(0, cnt - 1)]);
	}

	hplev	+= pMob->level;
	aclev	+= pMob->level;
	damlev	+= pMob->level;

	hplev	= URANGE( 1, hplev, 60 ) - 1;
	aclev	= URANGE( 1, aclev, 60 ) - 1;
	damlev	= URANGE( 1, damlev, 60 ) - 1;

	pMob->hit[DICE_NUMBER]		= recval_table[hplev].numhit;
	pMob->hit[DICE_TYPE]		= recval_table[hplev].typhit;
	pMob->hit[DICE_BONUS]		= recval_table[hplev].bonhit;

	pMob->damage[DICE_NUMBER]	= recval_table[damlev].numdam * 1.5;
	pMob->damage[DICE_TYPE]		= recval_table[damlev].typdam;
	pMob->damage[DICE_BONUS]	= recval_table[damlev].bondam * 1.5;

	pMob->mana[DICE_NUMBER]		= pMob->level;
	pMob->mana[DICE_TYPE]		= 10 + (pMob->level / 8);
	pMob->mana[DICE_BONUS]		= 100;

	if ( ES_PMOB_CASTER(pMob) )
		pMob->mana[DICE_BONUS]	*= (1 + pMob->level);

	for ( i = 0; i < 3; i++ )
		pMob->ac[i]	= recval_table[aclev].ac * 10;

	if ( IS_SET(pMob->act, ACT_UNDEAD)
	||   IS_SET(pMob->form, FORM_UNDEAD)
	||   IS_SET(pMob->form, FORM_MAGICAL) )
		n	= 0;
	else
	if ( IS_SET(pMob->act, ACT_MAGE) )
		n	= 1;
	else
	if ( IS_SET(pMob->act,ACT_THIEF)
	||   IS_SET(pMob->act,ACT_CLERIC)
	||   IS_SET(pMob->act,ACT_PSI) )
		n	= 2;
	else
		n	= 3;

	aclev = UMAX(0, aclev - n);

	pMob->ac[3]	= recval_table[aclev].ac * 10;

	if ( IS_SET(pMob->act, ACT_WARRIOR) )
	{
		hitbonus = pMob->level * 2 + UMAX(pMob->level - 30, 0)*6;
		pMob->damage[DICE_BONUS] += pMob->level;
	}
	else
	if ( IS_SET(pMob->act, ACT_THIEF) )
		hitbonus = pMob->level * 3 / 2;
	else
	if ( ES_PMOB_CASTER(pMob) )
		hitbonus = pMob->level;

	pMob->hitroll	= hitbonus;

	return;
}
