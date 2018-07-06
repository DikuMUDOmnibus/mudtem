void	give_qpoints	( CHAR_DATA *, int );

#if !defined(USAR_MACROS)
bool	tiene_qdata	( CHAR_DATA * );
int	get_qtype	( CHAR_DATA * );
int	get_qestado	( CHAR_DATA * );
int	get_qtimer	( CHAR_DATA * );
int	get_qid		( CHAR_DATA * );
bool	quest_completo	( CHAR_DATA * );
void	give_qdata	( CHAR_DATA * );
int	get_qpoints	( CHAR_DATA * );
bool	esta_en_quest	( CHAR_DATA * );
#else
#define	tiene_qdata(ch)	(IS_NPC(ch) ? FALSE : (ch)->pcdata->quest != NULL)
#define	get_qtype(ch)	(tiene_qdata(ch) ? (ch)->pcdata->quest->type : QUEST_NONE)
#define	get_qestado(ch)	(tiene_qdata(ch) ? (ch)->pcdata->quest->estado : QUEST_INCOMPLETO)
#define get_qtimer(ch)	(tiene_qdata(ch) ? (ch)->pcdata->quest->timer : 0)
#define	get_qid(ch)	(tiene_qdata(ch) ? (ch)->pcdata->quest->id : 0)
#define quest_completo(ch)	((ch)->pcdata->quest->estado == QUEST_COMPLETO)
#define give_qdata(ch)	((ch)->pcdata->quest = new_quest())
#define get_qpoints(ch)	(tiene_qdata(ch) ? (ch)->pcdata->quest->qpoints : 0)
#define esta_en_quest(ch)	(!IS_NPC(ch) && (ch)->pcdata->quest && (ch)->pcdata->quest->type != QUEST_NONE)
#endif

bool	es_quest_mob	( CHAR_DATA *, CHAR_DATA * );
void	completar_quest	( CHAR_DATA * );
void	set_qtimer	( CHAR_DATA *, int );
void	set_qtype	( CHAR_DATA *, int );
void	set_qid		( CHAR_DATA *, long );
void	set_qestado	( CHAR_DATA *, int );
void	mob_tell	( CHAR_DATA *, CHAR_DATA *, char *, Entity * );

#define QUEST_NONE	0
#define QUEST_KILLMOB	1

#define QUEST_INCOMPLETO	0
#define QUEST_COMPLETO		1
