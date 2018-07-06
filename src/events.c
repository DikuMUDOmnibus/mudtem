/* Rudimentary event code
 *
 * Copyright 1997 Oliver Jowett <oliver@sa-search.massey.ac.nz>. You can
 * freely use this code, so long as this notice remains intact.
 */

#include "include.h"
#include "events.h"
#include "tables.h"

int event_lookup_callback( ev_callback );

/* much tweakage will be needed here :)
 *
 * stuff not supplied here:
 *  new_event, free_event (just allocate/free the structure)
 *  typedef struct _event EVENT; (I have this in my merc.h so I don't have
 *   to pull in events.h there)
 *  a 'EVENT *events;' field in CHAR_DATA/ROOM_INDEX_DATA/OBJ_DATA
 *
 * in handler.c, extract_obj and extract_char need to remove any pending
 * events on that obj/char, ie:
 *
 * EVENT *ev, *ev_next;
 *
 * ...
 *
 * for (ev=ch->events; ev; ev=ev_next)
 * {
 *   ev_next=ev->nextitem;
 *   event_delete(ev);
 * }
 *
 * Also be sure that clone_object etc. set the events pointer of the new
 * object etc. to NULL.
 *
 */

EVENT *event_list[MAX_KEY_HASH];
int events_pending=0;
int events_total=0;
int events_hw=0;
long pulseclock=0;
int pulsebucket=0;

/* remove an event from an itemlist */
static void event_delete_itemlist(EVENT *ev)
{
  EVENT *search;
  EVENT **last;

  if (ev->start)
  {
    for (last=ev->start, search=*last;
	 search && search!=ev;
	 last=&search->nextitem, search=*last)
      ;
    
    if (!search)
    {
	int sn = event_lookup_callback(ev->callback);

	bugf("event_delete_itemlist: can't find event (%s) in itemlist for removal",
		sn != -1 ? event_table[sn].name : "???" );
    }
    else
      *last=ev->nextitem;
    
    ev->start=NULL;
  }
}

void event_update(void)
{
  EVENT *ev;

  pulseclock++; /* new stuff queued will queue in the future */

  while ((ev=event_list[pulsebucket])!=NULL &&
	 ev->when<pulseclock)
  {
    event_list[pulsebucket]=ev->next; /* remove from main list */
    ev->next=NULL;

    /* extract from the itemlist */
    event_delete_itemlist(ev);

    /* Event extracted, dispatch it */

    if (!ev->callback)
      bugf("event_update: null callback?!");
    else
      (*ev->callback)(ev);

    free_event(ev);
    events_pending--;
  }

  pulsebucket=(pulsebucket+1)%MAX_KEY_HASH;

  /* all done, move on to the next pulse */
}

void event_add(EVENT *ev)
{
  EVENT *search;
  EVENT **last;
  int bucket;

  if (!ev)
  {
    bugf("event_add: NULL");
    return;
  }

  /* ev should already be in the itemlist if necessary */

  if (ev->when<pulseclock)
  {
    bugf("event_add: timewarp!");
    event_delete_itemlist(ev);
    return;
  }

  bucket=ev->when % MAX_KEY_HASH;

  for (last=&event_list[bucket], search=*last;
       search && search->when<ev->when;
       last=&search->next, search=*last)
    ;

  ev->next=search;
  *last=ev;

  events_total++;
  events_pending++;
  if (events_pending > events_hw)
    events_hw=events_pending;
}

void event_delete(EVENT *ev)
{
  EVENT *search;
  EVENT **last;
  int bucket;

  if (!ev)
  {
    bugf("event_delete: NULL");
    return;
  }

  bucket=ev->when % MAX_KEY_HASH;

  for (last=&event_list[bucket], search=*last;
       search && search!=ev;
       last=&search->next, search=*last)
    ;

  if (!search)
    bugf("event_delete: event not in list");
  else
    *last=ev->next;

  event_delete_itemlist(ev);

  free_event(ev);
  events_pending--;
}

EVENT *event_pending(EVENT *list, ev_callback fn)
{
  EVENT *search;

  for (search=list;
       search;
       search=search->nextitem)
    if (search->callback==fn)
      return search;

  return NULL;
}

void char_event_add(CHAR_DATA *ch, int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev=new_event();
  ev->when=pulseclock+when;
  ev->item.ch=ch;
  ev->param=p;
  ev->callback=fn;

  /* hook into itemlist */
  ev->start=&ch->events;
  ev->nextitem=ch->events;
  ch->events=ev;

  event_add(ev);
}

void ent_event_add(Entity *ent, int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev		= new_event();
  ev->when	= pulseclock + when;
  ev->item.ent	= ent;
  ev->param	= p;
  ev->callback	= fn;
  ev->start	= NULL;

  event_add(ev);
}

void generic_event_add(int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev		= new_event();
  ev->when	= pulseclock+when;
  ev->param	= p;
  ev->callback	= fn;
  ev->item.ch	= NULL;
  ev->start	= NULL;

  event_add(ev);
}

void desc_event_add(DESCRIPTOR_DATA *desc, int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev=new_event();
  ev->when=pulseclock+when;
  ev->item.desc = desc;
  ev->param=p;
  ev->callback=fn;

  /* hook into itemlist */
  ev->start=&desc->events;
  ev->nextitem=desc->events;
  desc->events=ev;

  event_add(ev);
}

void obj_event_add(OBJ_DATA *obj, int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev=new_event();
  ev->when=pulseclock+when;
  ev->item.obj=obj;
  ev->param=p;
  ev->callback=fn;

  /* hook into itemlist */
  ev->start=&obj->events;
  ev->nextitem=obj->events;
  obj->events=ev;

  event_add(ev);
}

void room_event_add(ROOM_INDEX_DATA *room, int when, void *p, ev_callback fn)
{
  EVENT *ev;

  ev=new_event();
  ev->when=pulseclock+when;
  ev->item.room=room;
  ev->param=p;
  ev->callback=fn;

  /* hook into itemlist */
  ev->start=&room->events;
  ev->nextitem=room->events;
  room->events=ev;

  event_add(ev);
}

const char *event_stats(void)
{
  static char buf[300];

  sprintf(buf,
	  "Event statistics:\n\r"
	  "\n\r"
	  " Pending events:       %d\n\r"
	  " Max pending events:   %d\n\r"
	  " Total events queued:  %d\n\r"
	  " Pulseclock:           %ld\n\r",
	  events_pending,
	  events_hw,
	  events_total,
	  pulseclock);

  return buf;
}

int event_lookup_callback( ev_callback cb )
{
	int sn;

	for ( sn = 0; event_table[sn].name; sn++ )
		if ( event_table[sn].callback == cb )
			return sn;

	return -1;
}

void show_event_list( CHAR_DATA * ch, EVENT * pEv )
{
	EVENT * ev = pEv;
	int sn;

	for ( ; ev; ev = ev->nextitem )
	{
		sn = event_lookup_callback( ev->callback );

		printf_to_char( ch, "Evento %s, when %d\n\r",
			sn != -1 ? event_table[sn].name : "INDEF",
			ev->when );
	}
}

void event_get_obj( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	OBJ_DATA *obj;
	
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
		if ( obj->item_type == (int) ev->param && obj->level <= getNivelPr(ch) )
		{
			obj_from_room( obj );
			obj_to_char( obj, ch );
			act( "$n toma $p.", ch, objToEnt(obj), NULL, TO_ROOM );
			return;
		}
	}
}

void event_wield( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	OBJ_DATA *obj, *arma = NULL;
	int promedio = 0;

	for ( obj = ch->carrying; obj; obj = obj->next_content )
	{
		if ( (obj->wear_loc == WEAR_NONE) && (obj->item_type == ITEM_WEAPON) )
		{
			if ( PROMEDIO_OBJ(obj) > promedio )
			{
				promedio	= PROMEDIO_OBJ(obj);
				arma		= obj;
			}
		}
	}

	if ( arma && (PROMEDIO_CH(ch) < PROMEDIO_OBJ(arma)))
		wear_obj( ch, arma, FALSE );
}

void stc_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	char *msg = (char *) ev->param;

	send_to_char( msg, ch );
}

void ent_stc_event( EVENT *ev)
{
	Entity * ent = ev->item.ent;
	char *msg = (char *) ev->param;

	send_to_ent( msg, ent );
}

void to_room_act_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	char *msg = (char *) ev->param;
	
	act( msg, ch, NULL, NULL, TO_ROOM );
	return;
}

void room_update_event( EVENT *ev )
{
	ROOM_INDEX_DATA *room = ev->item.room;
	AFFECT_DATA *paf, *paf_next;
	CHAR_DATA *ch;

	/* go through affects and decrement */
	for ( paf = room->affected; paf != NULL; paf = paf_next )
	{
		paf_next	= paf->next;
		if ( paf->duration > 0 )
		{
			paf->duration--;
			if (number_range(0,4) == 0 && paf->level > 0)
				paf->level--;  /* spell strength fades with time */
		}
		else if ( paf->duration < 0 )
			;
		else
		{
			if ( paf_next == NULL
			||   paf_next->type != paf->type
			||   paf_next->duration > 0 )
				if ( paf->type > 0
				&& !IS_NULLSTR(skill_table[paf->type].msg_room)
				&& (ch = room->people) )
					act( skill_table[paf->type].msg_room, ch, 0, 0, TO_ALL );

			affect_remove_room( room, paf );
		}
	}

	if ( room->affected )
		room_event_add( room, PULSE_TICK, 0, room_update_event );
}

void autosave_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;

	if ( IS_NPC(ch) )
		return;

	if ( is_in_room(ch, ch->in_room) )
	{
		send_to_char( "#BG#brabando #BA#butomaticamente.\n\r", ch );
		save_char_obj( ch );
	}

	char_event_add( ch, MINUTOS(5)*PULSE_PER_SECOND, (void *) ch->id, autosave_event );
}

bool event_borrable( EVENT * ev )
{
	int pos;

	pos = event_lookup_callback( ev->callback );

	if ( pos != -1
	&&   event_table[pos].borrable == FALSE )
		return FALSE;

	return TRUE;
}

void actremove( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	int arg = (int) ev->param;
	
	REMOVE_BIT(ch->act, arg);
}

void actset( EVENT * ev )
{
	CHAR_DATA * ch = ev->item.ch;
	int arg = (int) ev->param;

	SET_BIT(ch->act, arg);
}

void obj_extract_event( EVENT * ev )
{
	OBJ_DATA * obj = ev->item.obj;

	extract_obj(obj, TRUE);
}
