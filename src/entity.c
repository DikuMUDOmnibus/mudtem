#include "include.h"
#include "recycle.h"

const char * entidadToString( Entity * ent )
{
	if (ent == NULL)
		return "";

	switch( ent->tipo )
	{
		case ENT_STRING:	return ent->u.string;
		case ENT_CH:		return SELFPERS(ent->u.ch);
		case ENT_OBJ:		return ent->u.obj->short_descr;
		case ENT_INT:		return itos(ent->u.entero);
		case ENT_ROOM:		return ent->u.room->name;
	}

	bugf( "entidadToString : tipo %d invalido", ent->tipo );
	return "";
}

const char * entidadToStringNoColor( Entity * ent )
{
	if ( ent == NULL )
		return "";

	switch( ent->tipo )
	{
		case ENT_STRING:	return ent->u.string;
		case ENT_CH:		return IS_NPC(ent->u.ch) ? ent->u.ch->short_descr : ent->u.ch->name;
		case ENT_OBJ:		return ent->u.obj->short_descr;
		case ENT_INT:		return itos(ent->u.entero);
		case ENT_ROOM:		return ent->u.room->name;
	}

	bugf( "entidadToStringNoColor : tipo %d invalido", ent->tipo );
	return "";
}

const char * entidadToStringExt( Entity * ent )
{
static	char buf[MSL];

	if ( ent == NULL )
		return "";

	switch( ent->tipo )
	{
		case ENT_STRING:	strcpy(buf, ent->u.string);							break;
		case ENT_CH:		sprintf(buf, "%s(%d)", SELFPERS(ent->u.ch), CHARVNUM(ent->u.ch) );		break;
		case ENT_OBJ:		sprintf(buf, "%s(%d)", ent->u.obj->short_descr, ent->u.obj->pIndexData->vnum);	break;
		case ENT_INT:		strcpy(buf, itos(ent->u.entero));						break;
		case ENT_ROOM:		sprintf(buf, "%s(%d)", ent->u.room->name, ent->u.room->vnum );			break;
		default:		bugf( "entidadToStringExt : tipo %d invalido", ent->tipo );
					return "";
	}

	return buf;
}

int entidadGetNivel( Entity * ent )
{
	switch( ent->tipo )
	{
		case ENT_CH:		return getNivelPr(ent->u.ch);
		case ENT_OBJ:		return ent->u.obj->level;
	}

	return 0;
}

Entity * ent_temp;

Entity * objToEntidad( OBJ_DATA * obj, bool permanente )
{
	Entity * temp;
	
	if (obj == NULL)
		return NULL;

	temp		= new_entity();
	temp->tipo	= ENT_OBJ;
	temp->u.obj	= obj;

	if (permanente == FALSE)
	{
		temp->next	= ent_temp;
		ent_temp	= temp;
	} /* lo pegamos a la lista ent_temp */

	return temp;
}

Entity * intToEntidad( int entero, bool permanente, ROOM_INDEX_DATA * room )
{
	Entity * temp = new_entity();

	temp->tipo = ENT_INT;
	temp->u.entero = entero;
	temp->whereis = room;

	if (permanente == FALSE)
	{
		temp->next	= ent_temp;
		ent_temp	= temp;
	} /* lo pegamos a la lista ent_temp */

	return temp;
}

Entity * chToEntidad( CHAR_DATA * ch, bool permanente )
{
	Entity * temp;

	if ( ch == NULL )
		return NULL;

	if ( permanente == FALSE
	&&  !IS_NPC(ch)
	&&   ch->ent != NULL )
		return ch->ent;

	temp		= new_entity();
	temp->tipo	= ENT_CH;
	temp->u.ch	= ch;
	temp->whereis	= ch->in_room;

	if (permanente == FALSE)
	{
		temp->next	= ent_temp;
		ent_temp	= temp;
	} /* lo pegamos a la lista ent_temp */

	return temp;
}

Entity * stringToEntidad( char * string, bool permanente, ROOM_INDEX_DATA * room )
{
	Entity * temp;

	if ( string == NULL )
		return NULL;

	temp		= new_entity();
	temp->tipo	= ENT_STRING;
	temp->u.string	= string;
	temp->whereis	= room;

	if (permanente == FALSE)
	{
		temp->next	= ent_temp;
		ent_temp	= temp;
	}

	return temp;
}

Entity * roomToEntidad( ROOM_INDEX_DATA * room, bool permanente )
{
	Entity * temp;

	if ( room == NULL )
		return NULL;

	temp		= new_entity();
	temp->tipo	= ENT_ROOM;
	temp->u.room	= room;
	temp->whereis	= room;

	if (permanente == FALSE)
	{
		temp->next	= ent_temp;
		ent_temp	= temp;
	}

	return temp;
}

int free_ent_watermark = 0;

void limpiar_ent_temp( void )
{
	Entity * ent, * ent_next;
	int cnt = 0;

	if ( ent_temp == NULL )
		return;

	for ( ent = ent_temp; ent; ent = ent_next )
	{
		ent_next = ent->next;
		cnt++;
		free_entity(ent);
	}

	if ( free_ent_watermark < cnt )
		free_ent_watermark = cnt;

	ent_temp = NULL;
}

int entidadGetVnum( Entity * ent )
{
	if (ent == NULL)
		return -1;

	switch(ent->tipo)
	{
		case ENT_CH:	return CHARVNUM(ent->u.ch);
		case ENT_OBJ:	return ent->u.obj->pIndexData->vnum;
		case ENT_ROOM:	return ent->u.room->vnum;
	}

	return -1;
}

char * entidadGetTipo( Entity * ent )
{
	if (ent == NULL)
		return "ERROR";

	switch(ent->tipo)
	{
		case ENT_CH:		return "char";
		case ENT_OBJ:		return "obj";
		case ENT_STRING:	return "string";
		case ENT_INT:		return "int";
		case ENT_ROOM:		return "room";
	}

	bugf( "entidadGetTipo : tipo %d inexistente", ent->tipo );

	return "";
}

char * newPERS( Entity * ent, CHAR_DATA * looker )
{
	if ( ent == NULL || looker == NULL )
		return "ERROR";

	switch(ent->tipo)
	{
		case ENT_CH:
		return PERS(ent->u.ch, looker);

		case ENT_OBJ:
		if ( can_see_obj(looker, ent->u.obj) )
			return ent->u.obj->short_descr;
		else
			return "algo";

		case ENT_ROOM:
		if ( can_see_room(looker, ent->u.room) )
			return ent->u.room->name;
		else
			return "algun lugar";
	}

	bugf( "newPERS : tipo %d invalido", ent->tipo );
	return "";
}

ROOM_INDEX_DATA * entWhereIs( Entity * ent )
{
	ROOM_INDEX_DATA *room = NULL;

	if (ent == NULL)
		return NULL;

	switch(ent->tipo)
	{
		case ENT_CH:
		room = ent->u.ch->in_room;
		break;

		case ENT_OBJ:
		room = obj_whereis(ent->u.obj);
		break;

		case ENT_ROOM:
		room = ent->u.room;
		break;
	}

	if (room == NULL)
	{
		if (ent->whereis == NULL)
			bugf( "entWhereIs : entidad %s, tipo %d invalido",
				entToStringExt(ent), ent->tipo );
		else
			room = ent->whereis;
	}

	return room;
}

bool entComparar( Entity * ent1, Entity * ent2 )
{
	if ( ent1 == NULL || ent2 == NULL )
		return FALSE;

	if ( ent1->tipo != ent2->tipo )
		return FALSE;

	switch( ent1->tipo )
	{
		case ENT_CH:		return entEsCh(ent2) && ent1->u.ch == ent2->u.ch;
		case ENT_OBJ:		return entEsObj(ent2) && ent1->u.obj == ent2->u.obj;
		case ENT_STRING:	return entEsStr(ent2) && !str_cmp(ent1->u.string, ent2->u.string);
		case ENT_INT:		return entEsInt(ent2) && ent1->u.entero == ent2->u.entero;
		case ENT_ROOM:		return entEsRoom(ent2) && ent1->u.room == ent2->u.room;
	}

	bugf( "entidadComparar : tipo %d inexistente", ent1->tipo );
	return FALSE;
}

CHAR_DATA * ent_get_char_room( Entity * ent, char * argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    ROOM_INDEX_DATA *room;
    CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;
    int number;
    int count;

    room = entWhereIs(ent);

    number = number_argument( argument, arg );
    count  = 0;

    if ( !str_cmp( arg, "self" ) )
	return ch;

    if (room == NULL)
    	return NULL;

    for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( (ch && !can_see( ch, rch )) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}

bool entHasTarget( Entity * ent )
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->mprog_target != NULL;
	};

	return FALSE;
}

#if !defined(USAR_MACROS)
Entity * entGetTarget( Entity * ent )
{
	if ( ent == NULL )
		return NULL;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->mprog_target;
	};

	return NULL;
}

int entGetSex(Entity * ent)
{
	if ( ent == NULL)
		return SEX_NEUTRAL;

	switch(ent->tipo)
	{
		case ENT_CH:
		return URANGE(0, ent->u.ch->sex, 2);
	}

	return SEX_NEUTRAL;
}

long entGetId( Entity * ent )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->id;
	}

	return 0;
}

char * entGetClanGod( Entity * ent )
{
	if (ent == NULL)
		return "Mota";

	switch(ent->tipo)
	{
		case ENT_CH:	return CLAN_GOD(ent->u.ch);
	}

	return "Mota";
}

int entGetRace( Entity * ent )
{
	if (ent == NULL)
		return 0;

	if ( entEsCh(ent) )
		return ent->u.ch->race;

	return 0;
}
#endif

void send_to_ent( char * msg, Entity * ent )
{
	if (ent == NULL)
		return;

	if ( entEsCh(ent) )
		send_to_char( msg, ent->u.ch );
}

void entSetTarget( Entity * ent, Entity * target )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:
		if (ent->u.ch->mprog_target != NULL)
			free_entity(ent->u.ch->mprog_target);
		if (target == NULL)
			ent->u.ch->mprog_target = NULL;
		else
			ent->u.ch->mprog_target = entCopiar(target);
		break;
	}
}

OBJ_DATA *ent_get_obj_here( Entity * ent, char *argument )
{
    OBJ_DATA *obj;

    if (ent == NULL)
    	return NULL;

    switch(ent->tipo)
    {
    	case ENT_CH:
    	obj = get_obj_list( ent->u.ch, argument, ent->u.ch->in_room->contents );

	if ( obj != NULL )
		return obj;

	if ( ( obj = get_obj_carry( ent->u.ch, argument, ent->u.ch ) ) != NULL )
		return obj;

	if ( ( obj = get_obj_wear( ent->u.ch, argument ) ) != NULL )
		return obj;
	break;
    }

    return NULL;
}

CHAR_DATA *ent_get_char_world( Entity * ent, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;
    int number;
    int count;

    if (ent == NULL)
    	return NULL;

    if ( ( wch = ent_get_char_room( ent, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;

    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL
	||   (ch && !can_see( ch, wch ))
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *ent_get_obj_world( Entity * ent, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch = entidadEsCh(ent) ? entidadGetCh(ent) : NULL;
    OBJ_DATA *obj;
    int number;
    int count;

    if (ent == NULL)
    	return NULL;

    if ( ( obj = ent_get_obj_here( ent, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( (!ch || can_see_obj( ch, obj )) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

bool ent_can_see( Entity * looker, Entity * target )
{
	if ( looker == NULL || target == NULL )
		return FALSE;

	switch(looker->tipo)
	{
		case ENT_CH:
		switch(target->tipo)
		{
			case ENT_CH:
			return can_see(entidadGetCh(looker),entidadGetCh(target));
			case ENT_OBJ:
			return can_see_obj(entidadGetCh(looker),entidadGetObj(target));
			default:
			return TRUE;
		}
		break;
	}

	return TRUE;
}

int entGetClan( Entity * ent )
{
	switch(ent->tipo)
	{
		case ENT_CH:
		return ent->u.ch->clan;

		case ENT_OBJ:
		return ent->u.obj->pIndexData->clan;

		case ENT_ROOM:
		return ent->u.room->clan;
	}

	return 0;
}

char * entGetName( Entity * ent )
{
	if (ent == NULL)
		return "";

	switch(ent->tipo)
	{
		case ENT_CH:
		return ent->u.ch->name;

		case ENT_OBJ:
		return ent->u.obj->name;

		case ENT_ROOM:
		return ent->u.room->name;
	}

	return "ERROR";
}

char * entGetShortDescr( Entity * ent )
{
	if (ent == NULL)
		return "";

	switch(ent->tipo)
	{
		case ENT_CH:
		return ent->u.ch->short_descr;

		case ENT_OBJ:
		return ent->u.obj->short_descr;

		case ENT_ROOM:
		return ent->u.room->name;
	}

	return "ERROR";
}

const char * entPERS(Entity * looker, Entity * target)
{
	if (looker == NULL || target == NULL)
		return "";

	if ( ent_can_see(looker, target) )
		return entidadToString(target);
	else
	{
		switch(target->tipo)
		{
			case ENT_CH:	return "alguien";
			case ENT_OBJ:	return "algo";
			case ENT_ROOM:	return "algun lugar";
		}
	}

	return "algo";
}

bool ent_can_see_ch( Entity * ent, CHAR_DATA *target )
{
	if (ent == NULL || target == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:
		return can_see(ent->u.ch, target);
	}

	return FALSE;
}

MPROG_LIST * entGetProgs( Entity * ent )
{
	if (ent == NULL)
		return NULL;

	switch(ent->tipo)
	{
		case ENT_CH:	return IS_NPC(ent->u.ch) ? ent->u.ch->pIndexData->mprogs : NULL;
		case ENT_OBJ:	return ent->u.obj->pIndexData->mprogs;
		case ENT_ROOM:	return ent->u.room->mprogs;
	}

	return NULL;
}

void affect_to_ent( Entity * ent, AFFECT_DATA * aff )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	affect_to_char( ent->u.ch, aff );	return;
		case ENT_OBJ:	affect_to_obj( ent->u.obj, aff );	return;
		case ENT_ROOM:	affect_to_room( ent->u.room, aff );	return;
	}
}

void entSetSavingThrow( Entity * ent, int valor )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	ent->u.ch->saving_throw = valor;	return;
	}
}

int entGetSavingThrow( Entity * ent )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->saving_throw;
	}

	return 0;
}

bool ent_is_part( Entity * ent, long part )
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return IS_PART(ent->u.ch, part);
	}

	return FALSE;
}

void obj_to_ent( OBJ_DATA *obj, Entity * ent )
{
	if (obj == NULL || ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	obj_to_char(obj, ent->u.ch);	return;
		case ENT_OBJ:	obj_to_obj(obj, ent->u.obj);	return;
		case ENT_ROOM:	obj_to_room(obj, ent->u.room);	return;
	}

	bugf( "obj_to_ent : entidad %s(%s) invalida",
		entToString(ent), entGetTipo(ent) );
	return;
}

void change_health( Entity *ent, Entity *victim, int mod )
{
/*	FIGHT_DATA * fd; */

	if (ent == NULL || victim == NULL)
		return;

	if ( entGetMaxHit(victim) < entGetHit(victim) + mod )
		mod = entGetMaxHit(victim) - entGetHit(victim);

	entSetHit( victim, entGetHit(victim) + mod );

/*	if ( entEsCh(victim) )
		for ( fd = entGetCh(victim)->fdata; fd; fd = fd->next )
			fd->dam = UMAX(0, fd->dam - (getNivelPr(entGetCh(victim)) > 29) ? mod*2 : mod); */
}

void entSetAlignment( Entity * ent, int align )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	ent->u.ch->alignment = align;	break;
	}

	return;
}

int entGetAlignment( Entity * ent )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->alignment;
	}

	return 0;
}

int entGetHit( Entity * ent )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->hit;
	}

	return 0;
}

int entGetMaxHit( Entity * ent )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return ent->u.ch->max_hit;
	}

	return 0;
}

void entSetHit( Entity * ent, int valor )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	ent->u.ch->hit = valor;		break;
	}

	return;
}

void ent_update_pos( Entity * ent )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	update_pos(ent->u.ch);		break;
	}

	return;
}

void ent_wear_obj( Entity * ent, OBJ_DATA *obj, bool arg )
{
	if (ent == NULL)
		return;

	switch(ent->tipo)
	{
		case ENT_CH:	wear_obj( ent->u.ch, obj, arg );	break;
	}

	return;
}

OBJ_DATA * ent_get_eq_char( Entity * ent, int arg )
{
	if (ent == NULL)
		return NULL;

	switch(ent->tipo)
	{
		case ENT_CH:	return get_eq_char(ent->u.ch, arg);
	}

	return NULL;
}

bool ent_is_npc( Entity * ent )
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return IS_NPC(ent->u.ch);
	}

	return TRUE;
}

bool ent_saves_spell( int level, Entity * ent, int dam_type )
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return saves_spell( level, ent->u.ch, dam_type );
	}

	return TRUE;
}

bool ent_is_affected( Entity * ent, int sn )
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return is_affected(ent->u.ch, sn);
		case ENT_OBJ:	return is_obj_affected(ent->u.obj, sn);
		case ENT_ROOM:	return is_room_affected(ent->u.room, sn);
	}

	return FALSE;
}

bool ent_is_safe_spell( Entity * ent, Entity * target, bool arg )
{
	if (ent == NULL || target == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:
		switch(target->tipo)
		{
			case ENT_CH:	return is_safe_spell(ent->u.ch, target->u.ch, arg);
		}
	}

	return TRUE;
}

bool ent_IS_AFFECTED(Entity * ent, long arg)
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return (IS_AFFECTED(ent->u.ch, arg) ? TRUE : FALSE);
	}

	return FALSE;
}

bool ent_IS_AFFECTED2(Entity * ent, long arg)
{
	if (ent == NULL)
		return FALSE;

	switch(ent->tipo)
	{
		case ENT_CH:	return (IS_AFFECTED2(ent->u.ch, arg) ? TRUE : FALSE);
	}

	return FALSE;
}

int ent_get_skill( Entity * ent, int sn )
{
	if (ent == NULL)
		return 0;

	switch(ent->tipo)
	{
		case ENT_CH:	return get_skill(ent->u.ch, sn);
	}

	return 0;
}

bool ent_is_same_group( Entity * ent, Entity * ent2 )
{
	if (ent == NULL || ent2 == NULL)
		return 0;

	if ( entEsCh(ent) )
	{
		if (entEsCh(ent2)
		&&   is_same_group(ent->u.ch, ent2->u.ch) )
		return TRUE;

		// no es ch
		if (entEsObj(ent2)
		&&  obj_carried_by(ent2->u.obj) == ent->u.ch )
			return TRUE;
	}
	else
	if ( entEsObj(ent) )
	{
		if (entEsCh(ent2)
		&&  obj_carried_by(ent->u.obj) == ent2->u.ch )
			return TRUE;
	}

	return FALSE;
}

int ent_get_curr_stat( Entity * ent, int arg )
{
	if (ent == NULL)
		return 0;

	if ( entEsCh(ent) )
		return get_curr_stat(ent->u.ch, arg);

	return 0;
}

CHAR_DATA * entGetPet( Entity * ent )
{
	if (ent == NULL)
		return NULL;

	if ( entEsCh(ent) )
		return ent->u.ch->pet;

	return NULL;
}

void ent_multi_hit( Entity * ent, Entity * target, int arg )
{
	if (ent == NULL || target == NULL)
		return;

	if ( entEsCh(ent)
	&&   entEsCh(target) )
		multi_hit(ent->u.ch, target->u.ch, arg);
}

void ent_add_follower( Entity * ent, Entity * arg )
{
	if (ent == NULL || arg == NULL)
		return;

	if ( entEsCh(ent) && entEsCh(arg) )
		add_follower( ent->u.ch, arg->u.ch );
}

CHAR_DATA * entGetPeopleRoom( Entity * ent )
{
	ROOM_INDEX_DATA * room = entWhereIs(ent);

	if (room == NULL)
		return NULL;
	else
		return room->people;
}

ROOM_INDEX_DATA *ent_find_location( Entity *ent, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return (atoi(arg) < 0 ? NULL : get_room_index( atoi( arg ) ) );

    if ( ( victim = ent_get_char_world( ent, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = ent_get_obj_world( ent, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

void ent_free_event( EVENT * ev )
{
	Entity * ent = ev->item.ent;

	free_entity(ent);
}

Entity * entCopiar( Entity * ent )
{
	Entity * temp;

	if ( ent == NULL )
	{
		bugf( "entCopiar : NULL ent" );
		return NULL;
	}

	temp		= new_entity();
	temp->tipo	= ent->tipo;

	switch(temp->tipo)
	{
		case ENT_CH:	temp->u.ch = ent->u.ch;		break;
		case ENT_OBJ:	temp->u.obj = ent->u.obj;	break;
		case ENT_ROOM:	temp->u.room = ent->u.room;	break;
		case ENT_INT:	temp->u.entero = ent->u.entero;	break;
		case ENT_STRING:temp->u.string = ent->u.string;	break;
		default:
		bugf( "entCopiar : tipo %d invalido", temp->tipo );
	}

	return temp;
}

Entity * ent_dead;

void poner_ent_lista( Entity * ent )
{
	ent->next	= ent_dead;
	ent_dead	= ent;
}

void dead_update( void )
{
	Entity * ent, * ent_next;

	if ( ent_dead == NULL )
		return;

	for ( ent = ent_dead; ent; ent = ent_next )
	{
		ent_next = ent->next;

		free_entity(ent);
	}

	ent_dead = NULL;
}

bool ent_died( Entity * ent )
{
	Entity * temp;

	if ( ent == NULL )
		return FALSE;

	for ( temp = ent_dead; temp; temp = temp->next )
		if ( entComparar(temp, ent) )
			return TRUE;

	return FALSE;
}

int get_prog_delay( Entity * ent )
{
	EVENT * ev, * evlist;

	switch(ent->tipo)
	{
		case ENT_CH:
		evlist = entGetCh(ent)->events;
		break;

		case ENT_ROOM:
		evlist = entGetRoom(ent)->events;
		break;

		case ENT_OBJ:
		evlist = entGetObj(ent)->events;
		break;

		default:
		return 0;
	}

	ev = event_pending( evlist, ent_timer );

	return ev ? ev->when - pulseclock : 0;
}

char * entGetNombre( Entity * ent )
{
	if (ent == NULL)
		return "ERROR";

	switch(ent->tipo)
	{
		case ENT_CH:		return ent->u.ch->name;
		case ENT_OBJ:		return ent->u.obj->name;
		case ENT_ROOM:		return ent->u.room->name;
		case ENT_STRING:	return ent->u.string;
		case ENT_INT:		return itos(ent->u.entero);
	}

	bugf("entGetNombre : tipo %d indefinido, ent %s", ent->tipo, entToStringExt(ent) );
	return "ERROR";
}
