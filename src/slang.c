#include "include.h"
#include "olc.h"
#include "tables.h"
#include "lookup.h"

#include <slang.h>
#include <_slang.h>

#define SLANG_ENT_TYPE		0x30

int spell_sn;

extern int _SLang_push_void_star	(unsigned char, VOID_STAR);

void slerr_hook( char * error )
{
	CHAR_DATA * ch;

	for ( ch = char_list; ch; ch = ch->next )
		if ( !IS_NPC(ch)
		&&    IS_SET(ch->in_room->room_flags, ROOM_PROTOTIPO) )
			send_to_room(error, ch->in_room);
}

int	slang_damage(Entity ** ent, Entity ** vent, int * cantidad, int * tipodano)
{
	CHAR_DATA * victim = entEsCh(*vent) ? entGetCh(*vent) : NULL;

	if (victim)
		newdamage( *ent, victim, *cantidad, spell_sn, *tipodano, TRUE );

	return 0;
}

void	slang_send_to_ent( Entity ** ent, char * argument )
{
	send_to_ent( argument, *ent );
}

void	slang_send_to_id( void * id, char * argument )
{
	CHAR_DATA *ch = get_char_from_id( *(int *) id );

	if ( ch != NULL )
		send_to_char( argument, ch );
}

int	slang_dice( int * dados, int * caras )
{
	return dice(*dados,*caras);
}

void	slang_act( char * cadena, Entity ** arg1, Entity ** arg2, Entity ** arg3, int * blah )
{
	new_act( cadena, arg1 ? *arg1 : NULL, arg2 ? *arg2 : NULL, arg3 ? *arg3 : NULL, *blah, POS_RESTING );
}

void	slang_interpret( Entity ** ent, char * arg )
{
	if ( (*ent)->tipo == ENT_CH )
		interpret( (*ent)->u.ch, arg );
}

int	slang_IS_AFFECTED( Entity ** ent, int * bitv )
{
	return ent_IS_AFFECTED( *ent, *bitv );
}

int	slang_rand( int * arg )
{
	return number_percent() < URANGE(0, *arg, 100);
}

int	slang_ispc( Entity ** ent )
{
	return entEsCh(*ent) ? !IS_NPC(entGetCh(*ent)) : 0;
}

int	slang_isgood( Entity ** ent )
{
	return ent_is_good(*ent);
}

int	slang_isevil( Entity ** ent )
{
	return ent_is_evil(*ent);
}

int	slang_isneutral( Entity ** ent )
{
	return ent_is_neutral(*ent);
}

int	slang_level( Entity ** ent )
{
	return entGetNivel(*ent);
}

char *	slang_nombre( Entity ** ent )
{
	return entGetNombre(*ent);
}

void	slang_mob( Entity ** ent, char * arg )
{
	mob_interpret( *ent, arg );
}

int	slang_isimmort( Entity ** ent )
{
	return entEsCh(*ent) && IS_IMMORTAL(entGetCh(*ent));
}

int	slang_uses( Entity ** ent, char * arg )
{
	return	entEsCh(*ent)
	&&	has_item(entGetCh(*ent), -1, item_lookup(arg), TRUE);
}

int	slang_class( Entity ** ent )
{
	return	entEsCh(*ent) ? getClasePr(entGetCh(*ent)) : -1;
}

int	slang_race( Entity ** ent )
{
	return	entGetRace(*ent);
}

int	slang_racelookup( char * arg )
{
	return race_lookup(arg);
}

int	slang_money( Entity ** ent )
{
	return entEsCh(*ent) ? DINERO(entGetCh(*ent)) : 0;
}

int	slang_classlookup( char * arg )
{
	return class_lookup(arg);
}

int	slang_clanlookup( char * arg )
{
	return clan_lookup(arg);
}

int	slang_clan( Entity ** ent )
{
	return entGetClan(*ent);
}

int	slang_hastarget( Entity ** ent )
{
	return entHasTarget(*ent);
}

int	slang_wears( Entity ** ent, char * arg )
{
	return entEsCh(*ent) && get_obj_wear(entGetCh(*ent), arg);
}

Entity **	slang_target( Entity ** ent )
{
static	Entity * bleh;

	bleh = entHasTarget(*ent) ? entGetTarget(*ent) : NULL;

	return &bleh;
}

int	slang_mobhere( Entity ** ent, int * vnum )
{
	ROOM_INDEX_DATA * room = entWhereIs(*ent);
	CHAR_DATA *ch;

	if (!room)
		return FALSE;

	for ( ch = room->people; ch; ch = ch->next_in_room )
		if ( IS_NPC(ch) && ch->pIndexData->vnum == *vnum )
			return TRUE;

	return FALSE;
}

int	slang_isfollow( Entity **ent )
{
	return  entEsCh(*ent)
	&&	entGetCh(*ent)->master
	&&	entGetCh(*ent)->master->in_room == entGetCh(*ent)->in_room;
}

void	slang_sendtoroom( Entity ** ent, char * arg )
{
	new_act(arg, *ent, NULL, NULL, TO_ALL, POS_RESTING);
}

SLang_Intrin_Fun_Type Intrinsecos [] =
{
	MAKE_INTRINSIC_2("send_to_ent", slang_send_to_ent, SLANG_VOID_TYPE,	SLANG_ENT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_2("send_to_room",slang_sendtoroom,  SLANG_VOID_TYPE,	SLANG_ENT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_2("send_to_id",	slang_send_to_id,  SLANG_VOID_TYPE,	SLANG_INT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_4("damage",	slang_damage,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE,	SLANG_ENT_TYPE,	SLANG_INT_TYPE,	SLANG_INT_TYPE),
	MAKE_INTRINSIC_5("act",		slang_act,	   SLANG_VOID_TYPE,	SLANG_STRING_TYPE, SLANG_ENT_TYPE, SLANG_ENT_TYPE, SLANG_ENT_TYPE, SLANG_INT_TYPE),
	MAKE_INTRINSIC_2("dice",	slang_dice,	   SLANG_INT_TYPE,	SLANG_INT_TYPE, SLANG_INT_TYPE),
	MAKE_INTRINSIC_2("interpret",	slang_interpret,   SLANG_VOID_TYPE,	SLANG_ENT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_2("afectado",	slang_IS_AFFECTED, SLANG_INT_TYPE,	SLANG_ENT_TYPE,	SLANG_INT_TYPE),
	MAKE_INTRINSIC_1("rand",	slang_rand,	   SLANG_INT_TYPE,	SLANG_INT_TYPE),
	MAKE_INTRINSIC_1("ispc",	slang_ispc,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("isevil",	slang_isevil,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("isneutral",	slang_isneutral,   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("isgood",	slang_isgood,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("isfollow",	slang_isfollow,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("isimmort",	slang_isimmort,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_2("mobhere",	slang_mobhere,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE,	SLANG_INT_TYPE),
	MAKE_INTRINSIC_1("level",	slang_level,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("nombre",	slang_nombre,	   SLANG_STRING_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_2("mob",		slang_mob,	   SLANG_VOID_TYPE,	SLANG_ENT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_2("uses",	slang_uses,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_2("wears",	slang_wears,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE, SLANG_STRING_TYPE),
	MAKE_INTRINSIC_1("class",	slang_class,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("race",	slang_race,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("clan",	slang_clan,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("money",	slang_money,	   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("race_lookup",	slang_racelookup,  SLANG_INT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_1("class_lookup",slang_classlookup, SLANG_INT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_1("clan_lookup",	slang_clanlookup,  SLANG_INT_TYPE,	SLANG_STRING_TYPE),
	MAKE_INTRINSIC_1("hastarget",	slang_hastarget,   SLANG_INT_TYPE,	SLANG_ENT_TYPE),
	MAKE_INTRINSIC_1("target",	slang_target,	   SLANG_ENT_TYPE,	SLANG_ENT_TYPE),
	SLANG_END_TABLE
};

#define blah(var,x)	const int i ## var = x;
#include "intrinvars.h"

SLang_Intrin_Var_Type Var_Intrinsecas [] =
{
#undef blah
#define blah(var,x)	MAKE_VARIABLE(#var,	&i ## var,	SLANG_INT_TYPE,	1),
#include "intrinvars.h"
	SLANG_END_TABLE
};

static int ent_push(unsigned char type, VOID_STAR ent)
{
	return _SLang_push_void_star(type, ent);
//	return _SLang_push_ref(1,ent);
}

static void ent_destroy(unsigned char unused, VOID_STAR ptr)
{
}

static int ent_pop(unsigned char type, VOID_STAR ptr)
{
	SLang_Object_Type obj;

	if (-1 == _SLang_pop_object_of_type(type, &obj, 0))
//	if (-1 == _SLang_pop_object_of_type(type, &obj))
		return -1;

	*(Entity **) ptr = obj.v.ptr_val;
//	*(Entity **) ptr = obj.v.p_val;

	return 0;
}

static int typecaster(unsigned char a_type, VOID_STAR ap, unsigned int na,
			unsigned char b_type, VOID_STAR bp)
{
	if ( b_type == SLANG_ENT_TYPE )
	{
		Entity ** b = (Entity **) bp;

		switch(a_type)
		{
			case SLANG_NULL_TYPE:
			*b = NULL;
			return 1;

			case SLANG_STRING_TYPE:
			*b = strToEnt(*(char **) ap, get_room_index(ROOM_VNUM_LIMBO) );
			return 1;
		}
	}

	if ( a_type == SLANG_INT_TYPE )
	{
		char ** b = (char **) bp;

		switch(b_type)
		{
			case SLANG_STRING_TYPE:
			*b = SLang_create_slstring(itos(*(int *)ap));
			return 1;
		}
	}

	return 0;
}

static int ent_ent_bin_op_result(int op, unsigned char a_type, unsigned char b_type,
			     unsigned char *c_type)
{
	*c_type = SLANG_INT_TYPE;
	return 1;
}

static int ent_ent_bin_op(int op,
			  unsigned char a_type, VOID_STAR ap, unsigned int na,
			  unsigned char b_type, VOID_STAR bp, unsigned int nb,
			  VOID_STAR cp)
{
	Entity *** a = (Entity ***) ap;
	Entity *** b = (Entity ***) bp;
	int * c = (int *) cp;

	if (a_type != SLANG_ENT_TYPE
	||  b_type != SLANG_ENT_TYPE)
		return -1;

	switch(op)
	{
		default:
		return 0;

		case SLANG_EQ:
		c[0] = entComparar(**a,**b) ? 1 : 0;
		break;

		case SLANG_NE:
		c[0] = entComparar(**a,**b) ? 0 : 1;
		break;
	}

	return 1;
}

void inicializar_slang(void)
{
	SLang_Class_Type *cl;

	flog( "Inicializando SLang" );
	if ( -1 == SLang_init_slang() )
		bugf( "inicializar slang : error" );
	if ( -1 == SLadd_intrin_fun_table (Intrinsecos, NULL))
		bugf( "cargar intrinsecos : error" );
	if ( -1 == SLadd_intrin_var_table (Var_Intrinsecas, NULL))
		bugf( "cargar var intrinsecas : error" );

	if (NULL == (cl = SLclass_allocate_class("Ent_Type")))
		bugf( "SLclass_allocate_class : Ent_Type" );
	SLclass_set_push_function(cl, ent_push);
	SLclass_set_pop_function(cl, ent_pop);
	SLclass_set_destroy_function(cl, ent_destroy);
	if (-1 == SLclass_register_class (cl, SLANG_ENT_TYPE, sizeof (Entity **), SLANG_CLASS_TYPE_PTR))
		bugf("SLclass_register_class : SLANG_ENT_TYPE");
	if (-1 == SLclass_add_typecast(SLANG_NULL_TYPE, SLANG_ENT_TYPE, typecaster, 1))
		bugf("SLclass_add_typecast");
	if (-1 == SLclass_add_typecast(SLANG_INT_TYPE, SLANG_STRING_TYPE, typecaster, 1))
		bugf("SLclass_add_typecast");
	if (-1 == SLclass_add_binary_op(SLANG_ENT_TYPE, SLANG_ENT_TYPE, ent_ent_bin_op, ent_ent_bin_op_result))
		bugf("SLclass_add_binary_op");
	SLang_Error_Hook = slerr_hook;
}

DO_FUN_DEC(do_temp)
{
/*	int sn;

	for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
		if ( skill_table[sn].spell_fun != NULL
		&&   skill_table[sn].spell_fun != spell_null
		&&   skill_table[sn].rating[CLASS_CLERIC] > 0
		&&   skill_table[sn].skill_level[CLASS_CLERIC] > 0
		&&   skill_table[sn].skill_level[CLASS_CLERIC] <= LEVEL_HERO )
		{
			// lo debiera tener un ranger?
			skill_table[sn].skill_level[CLASS_RANGER] = 25 +
				skill_table[sn].skill_level[CLASS_CLERIC]/2;
		} */

	int clan = clan_lookup(argument), hash;
	MOB_INDEX_DATA *mob;

	if ( clan == 0 )
		return;

	for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
			if ( mob->area == ch->in_room->area )
				mob->clan = clan;

	send_to_char("Ok.\n\r", ch );

	return;
}

SPELL_FUN_DEC(spell_slang)
{
	spell_sn = sn;

	if (-1 == SLadd_intrinsic_variable ("caster", (VOID_STAR) &caster, SLANG_ENT_TYPE, 1))
		return fFAIL;

	if (-1 == SLadd_intrinsic_variable ("victim", (VOID_STAR) &ent, SLANG_ENT_TYPE, 1))
		return fFAIL;

	if ( -1 == SLang_load_string(skill_table[sn].slangscript) )
	{
		bugf( "SLang_Error %d", SLang_Error );
		SLang_restart(1);
		SLang_Error = 0;
	}

	return TRUE;
}

void program_flow( 
        sh_int pvnum,  /* For diagnostic purposes */
	char *source,  /* the actual MOBprog code */
	Entity * ent, Entity *actor, Entity *arg1, Entity *arg2 )
{
	char buf[MSL],buf2[20];

	buf[0] = '\0';

	if (-1 == SLadd_intrinsic_variable("ent", (VOID_STAR) &ent, SLANG_ENT_TYPE, 1))
	{
		bugf("program_flow : SLadd_intrinsic_variable (ent)" );
		return;
	}

	if (-1 == SLadd_intrinsic_variable("actor", (VOID_STAR) &actor, SLANG_ENT_TYPE, 1))
	{
		bugf("program_flow : SLadd_intrinsic_variable (actor)" );
		return;
	}

	if (-1 == SLadd_intrinsic_variable("arg1", (VOID_STAR) &arg1, SLANG_ENT_TYPE, 1))
	{
		bugf("program_flow : SLadd_intrinsic_variable (arg1)" );
		return;
	}

	if (-1 == SLadd_intrinsic_variable("arg2", (VOID_STAR) &arg2, SLANG_ENT_TYPE, 1))
	{
		bugf("program_flow : SLadd_intrinsic_variable (arg2)" );
		return;
	}

	sprintf(buf2,"main_%d", pvnum);
	strcat(buf,"define ");
	strcat(buf,buf2);
	strcat(buf,"(){\n");
	strcat(buf,source);
	strcat(buf,"}\n");
	strcat(buf,buf2);
	strcat(buf,"();\n");

	if (-1 == SLang_load_string(buf))
	{
		bugf("program_flow : SLang_load_string (vnum %d)", pvnum );
		SLang_restart(1);
		SLang_Error = 0;
	}
}
