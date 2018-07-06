#include "events.h"

#define entidadEsCh( ent )	( (ent) && ((ent)->tipo == ENT_CH) )
#define entidadEsObj( ent )	( (ent) && ((ent)->tipo == ENT_OBJ) )
#define entidadEsString( ent )	( (ent) && ((ent)->tipo == ENT_STRING) )
#define entidadEsRoom( ent )	( (ent) && ((ent)->tipo == ENT_ROOM) )
#define entidadEsInt( ent )	( (ent) && ((ent)->tipo == ENT_INT) )

#define entEsCh(ent)		entidadEsCh(ent)
#define entEsObj(ent)		entidadEsObj(ent)
#define entEsStr(ent)		entidadEsString(ent)
#define entEsRoom(ent)		entidadEsRoom(ent)
#define entEsInt(ent)		entidadEsInt(ent)

#define entidadGetCh( ent )	( (ent)->u.ch )
#define entidadGetObj( ent )	( (ent)->u.obj )
#define entidadGetString( ent )	( (ent)->u.string )
#define entidadGetRoom( ent )	( (ent)->u.room )
#define entidadGetInt( ent )	( (ent)->u.entero )

#define entGetCh(ent)		entidadGetCh(ent)
#define entGetObj(ent)		entidadGetObj(ent)
#define entGetRoom(ent)		entidadGetRoom(ent)

const char *	entidadToString		( Entity * );
const char *	entidadToStringNoColor	( Entity * );
const char *	entidadToStringExt	( Entity * );
int		entidadGetNivel		( Entity * );
Entity *	objToEntidad		( OBJ_DATA *, bool );
Entity *	chToEntidad		( CHAR_DATA *, bool );
Entity *	stringToEntidad		( char *, bool, ROOM_INDEX_DATA * );
Entity *	intToEntidad		( int, bool, ROOM_INDEX_DATA * );
Entity *	roomToEntidad		( ROOM_INDEX_DATA *, bool );

#define entToString(ent)		entidadToString(ent)
#define entToStringExt(ent)		entidadToStringExt(ent)
#define entGetNivel(ent)		entidadGetNivel(ent)
#define entGetTipo(ent)			entidadGetTipo(ent)

#define strToEnt(a,room)	stringToEntidad(a, FALSE, room)
#define chToEnt(a)		chToEntidad(a, FALSE)
#define objToEnt(a)		objToEntidad(a, FALSE)
#define intToEnt(a,room)	intToEntidad(a, FALSE, room)
#define roomToEnt(a)		roomToEntidad(a, FALSE)
#define entGetVnum(a)		entidadGetVnum(a)

ROOM_INDEX_DATA *	entWhereIs	( Entity * );

int		entidadGetVnum		( Entity * );
char *		entidadGetTipo		( Entity * );
char *		newPERS			( Entity *, CHAR_DATA * );

void		limpiar_ent_temp	( void );
bool		entComparar		( Entity *, Entity * );
CHAR_DATA *	ent_get_char_room	( Entity *, char * );

#if !defined(USAR_MACROS)
Entity *	entGetTarget		( Entity * );
int		entGetSex		( Entity * );
long		entGetId		( Entity * );
char *		entGetClanGod		( Entity * );
int		entGetRace		( Entity * );
#else
#define entGetTarget(ent)	(((ent) && (ent)->tipo == ENT_CH) ? (ent)->u.ch->mprog_target : NULL)
#define entGetSex(ent)		(((ent) && (ent)->tipo == ENT_CH) ? URANGE(0,(ent)->u.ch->sex,2) : SEX_NEUTRAL)
#define	entGetId(ent)		(((ent) && entEsCh(ent)) ? (ent)->u.ch->id : 0)
#define entGetClanGod(ent)	(((ent) && entEsCh(ent)) ? CLAN_GOD((ent)->u.ch) : "Mota")
#define entGetRace(ent)		(((ent) && entEsCh(ent)) ? (ent)->u.ch->race : 0)
#endif

void		send_to_ent		( char *, Entity * );
void		entSetTarget		( Entity *, Entity * );
OBJ_DATA *	ent_get_obj_world	( Entity *, char * );
CHAR_DATA *	ent_get_char_world	( Entity *, char * );
OBJ_DATA *	ent_get_obj_here	( Entity *, char * );
bool		ent_can_see		( Entity *, Entity * );
int		entGetClan		( Entity * );
bool		ent_can_see_ch		( Entity *, CHAR_DATA * );
const char *	entPERS			( Entity *, Entity * );
char *		entGetShortDescr	( Entity * );
char *		entGetName		( Entity * );
bool		entHasTarget		( Entity * );
MPROG_LIST *	entGetProgs		( Entity * );
void		affect_to_ent		( Entity *, AFFECT_DATA * );
int		entGetSavingThrow	( Entity * );
void		entSetSavingThrow	( Entity *, int );
bool		ent_is_part		( Entity *, long );
void		obj_to_ent		( OBJ_DATA *, Entity * );
void		change_health		( Entity *, Entity *, int );
void		entSetAlignment		( Entity *, int );
int		entGetAlignment		( Entity * );
int		entGetHit		( Entity * );
int		entGetMaxHit		( Entity * );
void		entSetHit		( Entity *, int );
void		ent_update_pos		( Entity * );
void		ent_wear_obj		( Entity *, OBJ_DATA *, bool );
OBJ_DATA *	ent_get_eq_char		( Entity *, int );
bool		ent_is_npc		( Entity * );
bool		ent_saves_spell		( int, Entity *, int );
bool		ent_is_affected		( Entity *, int );
bool		ent_died		( Entity * );
#define ent_is_evil(ent) 		(entGetAlignment(ent) <= -350)
#define ent_is_good(ent) 		(entGetAlignment(ent) >= 350 )
#define ent_is_neutral(ent)		(!ent_is_evil(ent) && !ent_is_good(ent))
bool		ent_is_safe_spell	( Entity *, Entity *, bool );
bool		ent_IS_AFFECTED		( Entity *, long );
bool		ent_IS_AFFECTED2	( Entity *, long );
int		ent_get_skill		( Entity *, int );
bool		ent_is_same_group	( Entity *, Entity * );
int		ent_get_curr_stat	( Entity *, int );
CHAR_DATA *	entGetPet		( Entity * );
void		ent_multi_hit		( Entity *, Entity *, int );
void		ent_event_add		( Entity *, int, void *, ev_callback );
void		ent_add_follower	( Entity *, Entity * );
CHAR_DATA *	entGetPeopleRoom	( Entity * );
ROOM_INDEX_DATA *	ent_find_location	( Entity *, char * );
Entity *	entCopiar		( Entity * );
void		poner_ent_lista		( Entity * );
void		dead_update		( void );
int		get_prog_delay		( Entity * );
char *		entGetNombre		( Entity * );
