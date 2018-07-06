#if !defined(_SMART_H)
#define _SMART_H

typedef struct	mf_data			MF_DATA;

struct mf_data
{
	MF_DATA *	next;
	CHAR_DATA *	mob;
};

bool is_caster( CHAR_DATA * );
bool can_disarm( CHAR_DATA *, CHAR_DATA * );

#if !defined(USAR_MACROS)
bool is_mob_safe( CHAR_DATA * );
bool can_circle( CHAR_DATA *, CHAR_DATA * );
#else
#define is_mob_safe(ch)			\
(  (ch)->in_room == NULL		\
|| (ch)->pIndexData->pShop != NULL	\
|| (ch)->pIndexData->pRepair != NULL	\
|| IS_SET((ch)->act, ACT_IS_HEALER)	\
|| IS_SET((ch)->act, ACT_PRACTICE)	\
|| IS_SET((ch)->act, ACT_TRAIN)		\
|| IS_SET((ch)->act, ACT_IS_CHANGER)	\
|| IS_SET((ch)->act, ACT_BANKER)	\
|| IS_SET((ch)->act, ACT_PROTOTIPO) )
#define can_circle(ch,victim) (get_eq_char((ch), WEAR_WIELD) != NULL && (victim)->fighting != (ch))
#endif

void act_warrior( CHAR_DATA *, CHAR_DATA * );
void act_mage( CHAR_DATA *, CHAR_DATA * );
void act_cleric( CHAR_DATA *, CHAR_DATA * );
void act_thief( CHAR_DATA *, CHAR_DATA * );
void act_psi(CHAR_DATA *, CHAR_DATA *);
bool will_flee( CHAR_DATA *, CHAR_DATA * );
void set_mob_fight( CHAR_DATA * );
void stop_mob_fight( CHAR_DATA * );
bool is_mob_fighting( CHAR_DATA * );
#endif // _SMART_H
