/* Rudimentary event code
 *
 * Copyright 1997 Oliver Jowett <oliver@sa-search.massey.ac.nz>. You can
 * freely use this code, so long as this notice remains intact.
 */

#ifndef _EVENTS_H
#define _EVENTS_H

/* Event queueing/dequeueing/etc */

typedef void (*ev_callback)(struct _event *ev);

struct _event {
  union {
    CHAR_DATA *ch;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    DESCRIPTOR_DATA *desc;
    Entity *ent;
  } item;                   /* object/char etc. associated with this       */

  void *param;              /* param for the callback                      */

  ev_callback callback;     /* callback to call                            */
  int when;                 /* when event is going to happen (pulseclock)  */

  struct _event *next;      /* next in priority queue                      */
  struct _event *nextitem;  /* next in item queue                          */
  struct _event **start;    /* pointer to pointer to start of this sublist */
};

struct event_type
{
	char *		name;
	ev_callback	callback;
	bool		borrable;
};

/* event globals */
extern EVENT *event_list[MAX_KEY_HASH]; /* global event list                  */
extern int events_pending;         /* # of pending events                */
extern int events_total;           /* total events ever queued           */
extern int events_hw;              /* high-water-mark for events_pending */
extern long pulseclock;            /* global pulseclock                  */

/* low-level manipulation functions */
EVENT *new_event(void);            /* grab a new event structure */
void free_event(EVENT *ev);        /* free an event structure */
void event_update(void);           /* run any pending events */
void event_add(EVENT *ev);         /* queue a new event */
void event_delete(EVENT *ev);      /* delete an already queued event */
EVENT *event_pending(EVENT *list, ev_callback fn);

/* higher-level manipulation functions */
void char_event_add(CHAR_DATA *ch, int when, void *p, ev_callback fn);
void obj_event_add (OBJ_DATA *obj, int when, void *p, ev_callback fn);
void room_event_add(ROOM_INDEX_DATA *room, int when, void *p, ev_callback fn);
void desc_event_add(DESCRIPTOR_DATA *, int, void *, ev_callback);
void generic_event_add( int, void *, ev_callback );
void ent_event_add(Entity *, int, void *, ev_callback );

/* stat collection output */
const char *event_stats(void);
void	show_event_list	( CHAR_DATA *, EVENT * );
bool	event_borrable ( EVENT * );

/* eventos genericos */
void event_get_obj		( EVENT * );
void event_wield		( EVENT * );
void stc_event			( EVENT * );
void to_room_act_event		( EVENT * );
void room_update_event		( EVENT * );
void char_update_event		( EVENT * );
void obj_update_event		( EVENT * );
void autosave_event		( EVENT * );
void mob_fight			( EVENT * );
void mob_cast			( EVENT * );
void smart_event		( EVENT * );
void hunt_event			( EVENT * );
void quest_update		( EVENT * );
void mem_check_event		( EVENT * );
void save_plist_event		( EVENT * );
void eat_msg			( EVENT * );
void afilar_arma		( EVENT * );
void obj_quemar			( EVENT * );
void reboot_event		( EVENT * );
void ent_free_event		( EVENT * );
void ent_stc_event		( EVENT * );
void ent_timer			( EVENT * );
void recall_event		( EVENT * );
void room_aggress_event		( EVENT * );
void actset			( EVENT * );
void actremove			( EVENT * );
void check_strength		( EVENT * );
void obj_extract_event		( EVENT * );

#endif
