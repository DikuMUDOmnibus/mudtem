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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "screen.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	&&  !str_prefix(name,flag_table[flag].name)
	&&   flag_table[flag].settable)
	    return flag_table[flag].bit;
    }

    return NO_FLAG;
}

int position_lookup (const char *name)
{
   int pos;

   for (pos = 0; position_table[pos].name != NULL; pos++)
   {
	if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
	&&  !str_prefix(name,position_table[pos].name))
	    return pos;
   }
   
   return -1;
}

int sex_lookup (const char *name)
{
   int sex;
   
   for (sex = 0; sex_table[sex].name != NULL; sex++)
   {
	if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
	&&  !str_prefix(name,sex_table[sex].name))
	    return sex;
   }

   return -1;
}

int size_lookup (const char *name)
{
   int size;
 
   for ( size = 0; size_table[size].name != NULL; size++)
   {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix( name,size_table[size].name))
            return size;
   }
 
   return -1;
}

PETITION_DATA * petition_lookup( CHAR_DATA *ch )
{
	PETITION_DATA * pet;
	extern PETITION_DATA * petition_list;

	for ( pet = petition_list; pet; pet = pet->next )
		if ( pet->id == ch->id )
			return pet;

	return NULL;
}

PETITION_DATA * petition_lookup_name( char * name )
{
	PETITION_DATA * pet;
	extern PETITION_DATA * petition_list;

	for ( pet = petition_list; pet; pet = pet->next )
		if ( !str_cmp( pet->name, name ) )
			return pet;

	return NULL;
}

HELP_DATA * help_lookup( char *keyword )
{
	HELP_DATA *pHelp;
	char temp[MIL], argall[MIL];

	argall[0] = '\0';

	while (keyword[0] != '\0' )
	{
		keyword = one_argument(keyword, temp);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall, temp);
	}

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
		if ( is_name( argall, pHelp->keyword ) )
			return pHelp;

	return NULL;
}

HELP_AREA * had_lookup( char *arg )
{
	HELP_AREA * temp;
	extern HELP_AREA * had_list;

	for ( temp = had_list; temp; temp = temp->next )
		if ( !str_cmp( arg, temp->filename ) )
			return temp;

	return NULL;
}

int term_lookup (const char *name)
{
   int term;
 
   for ( term = 0; term_table[term].name != NULL; term++)
   {
        if (LOWER(name[0]) == LOWER(term_table[term].name[0])
        &&  !str_prefix( name,term_table[term].name))
            return term;
   }
 
   return -1;
}

EXIT_DATA * exit_lookup( ROOM_INDEX_DATA *room, sh_int door )
{
	EXIT_DATA * temp;

	for ( temp = room->exits; temp; temp = temp->next )
		if ( temp->direccion == door )
			return temp;

	return NULL;
}

int attack_lookup  (const char *name)
{
    int att;
    extern bool fBootDb;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    if ( fBootDb )
    	bugf( "attack_lookup : ataque %s inexistente", name );

    return 0;
}

int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
} 

FIGHT_DATA * fdata_lookup( CHAR_DATA *ch, long id )
{
	FIGHT_DATA * fd;

	for ( fd = ch->fdata; fd; fd = fd->next )
		if ( fd->id == id )
			return fd;

	return NULL;
}
