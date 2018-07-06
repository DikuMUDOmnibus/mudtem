/*
 * SillyMUD Distribution V1.1b             (c) 1993 SillyMUD Developement
 * See license.doc for distribution terms.   SillyMUD is based on DIKUMUD
 *
 * Modifications by Rip in attempt to port to merc 2.1
 */

/*
 * Modified by Turtle for Merc22 (07-Nov-94)
 *
 * I got this one from ftp.atinc.com:/pub/mud/outgoing/track.merc21.tar.gz.
 * It cointained 5 files: README, hash.c, hash.h, skills.c, and skills.h.
 * I combined the *.c and *.h files in this hunt.c, which should compile
 * without any warnings or errors.
 */

/*
 * Some systems don't have bcopy and bzero functions in their linked libraries.
 * If compilation fails due to missing of either of these functions,
 * define NO_BCOPY or NO_BZERO accordingly.                 -- Turtle 31-Jan-95
 */

#if defined(WIN32)
#define NO_BCOPY
#define NO_BZERO
#endif

#include "include.h"
#include "recycle.h"
#include "events.h"
#include "lookup.h"
#include "special.h"

DECLARE_DO_FUN( do_say	);
DECLARE_DO_FUN( do_open	);
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_mproomhunt );

#if !defined(MSDOS) && !defined(linux) && !defined(__CYGWIN32__)
void bcopy(register char *s1,register char *s2,int len);
void bzero(register char *sp,int len);
#endif
void perseguir( CHAR_DATA *ch );

struct hash_link
{
  int			key;
  struct hash_link	*next;
  void			*data;
};

struct hash_header
{
  int			rec_size;
  int			table_size;
  int			*keylist, klistsize, klistlen; /* this is really lame,
							  AMAZINGLY lame */
  struct hash_link	**buckets;
};

#define WORLD_SIZE	30000
#define	HASH_KEY(ht,key)((((unsigned int)(key))*17)%(ht)->table_size)

struct room_q
{
  int		room_nr;
  struct room_q	*next_q;
};

struct nodes
{
  int	visited;
  int	ancestor;
};

#define IS_DIR		(exit_lookup(get_room_index(q_head->room_nr), (i)))
#define GO_OK		(!IS_SET( IS_DIR->exit_info, EX_CLOSED ))
#define GO_OK_SMARTER	1

#if defined( NO_BCOPY )
void bcopy(register char *s1,register char *s2,int len)
{
  while( len-- ) *(s2++) = *(s1++);
}
#endif

#if defined( NO_BZERO )
void bzero(register char *sp,int len)
{
  while( len-- ) *(sp++) = '\0';
}
#endif

void init_hash_table(struct hash_header	*ht,int rec_size,int table_size)
{
  ht->rec_size	= rec_size;
  ht->table_size= table_size;
  ht->buckets	= (void*)mud_calloc(sizeof(struct hash_link**),table_size);
  ht->keylist	= (void*)mud_malloc(sizeof(ht->keylist)*(ht->klistsize=128));
  ht->klistlen	= 0;
}

void init_world(ROOM_INDEX_DATA *room_db[])
{
  /* zero out the world */
  bzero((char *)room_db,sizeof(ROOM_INDEX_DATA *)*WORLD_SIZE);
}

// void destroy_hash_table(struct hash_header *ht,void (*gman)(void))
void destroy_hash_table(struct hash_header *ht)
{
  int			i;
  struct hash_link	*scan,*temp;

  for(i=0;i<ht->table_size;i++)
    for(scan=ht->buckets[i];scan;)
      {
	temp = scan->next;
//	(*gman)(scan->data); donothing???
	free(scan);
	scan = temp;
      }
  free(ht->buckets);
  free(ht->keylist);
}

void _hash_enter(struct hash_header *ht,int key,void *data)
{
  /* precondition: there is no entry for <key> yet */
  struct hash_link	*temp;
  int			i;

  temp		= (struct hash_link *)mud_malloc(sizeof(struct hash_link));
  temp->key	= key;
  temp->next	= ht->buckets[HASH_KEY(ht,key)];
  temp->data	= data;
  ht->buckets[HASH_KEY(ht,key)] = temp;
  if(ht->klistlen>=ht->klistsize)
    {
      ht->keylist = (void*)realloc(ht->keylist,sizeof(*ht->keylist)*
				   (ht->klistsize*=2));
    }
  for(i=ht->klistlen;i>=0;i--)
    {
      if(ht->keylist[i-1]<key)
	{
	  ht->keylist[i] = key;
	  break;
	}
      ht->keylist[i] = ht->keylist[i-1];
    }
  ht->klistlen++;
}

ROOM_INDEX_DATA *room_find(ROOM_INDEX_DATA *room_db[],int key)
{
  return((key<WORLD_SIZE&&key>-1)?room_db[key]:0);
}

void *hash_find(struct hash_header *ht,int key)
{
  struct hash_link *scan;

  scan = ht->buckets[HASH_KEY(ht,key)];

  while(scan && scan->key!=key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

int room_enter(ROOM_INDEX_DATA *rb[],int key,ROOM_INDEX_DATA *rm)
{
  ROOM_INDEX_DATA *temp;
   
  temp = room_find(rb,key);
  if(temp) return(0);

  rb[key] = rm;
  return(1);
}

int hash_enter(struct hash_header *ht,int key,void *data)
{
  void *temp;

  temp = hash_find(ht,key);
  if(temp) return 0;

  _hash_enter(ht,key,data);
  return 1;
}

ROOM_INDEX_DATA *room_find_or_create(ROOM_INDEX_DATA *rb[],int key)
{
  ROOM_INDEX_DATA *rv;

  rv = room_find(rb,key);
  if(rv) return rv;

  rv = (ROOM_INDEX_DATA *)mud_malloc(sizeof(ROOM_INDEX_DATA));
  rb[key] = rv;
    
  return rv;
}

void *hash_find_or_create(struct hash_header *ht,int key)
{
  void *rval;

  rval = hash_find(ht, key);
  if(rval) return rval;

  rval = (void*)mud_malloc(ht->rec_size);
  _hash_enter(ht,key,rval);

  return rval;
}

int room_remove(ROOM_INDEX_DATA *rb[],int key)
{
  ROOM_INDEX_DATA *tmp;

  tmp = room_find(rb,key);
  if(tmp)
    {
      rb[key] = 0;
      free(tmp);
    }
  return(0);
}

void *hash_remove(struct hash_header *ht,int key)
{
  struct hash_link **scan;

  scan = ht->buckets+HASH_KEY(ht,key);

  while(*scan && (*scan)->key!=key)
    scan = &(*scan)->next;

  if(*scan)
    {
      int		i;
      struct hash_link	*temp, *aux;

      temp	= (*scan)->data;
      aux	= *scan;
      *scan	= aux->next;
      free(aux);

      for(i=0;i<ht->klistlen;i++)
	if(ht->keylist[i]==key)
	  break;

      if(i<ht->klistlen)
	{
	  bcopy((char *)ht->keylist+i+1,(char *)ht->keylist+i,(ht->klistlen-i)
		*sizeof(*ht->keylist));
	  ht->klistlen--;
	}

      return temp;
    }

  return NULL;
}

void room_iterate(ROOM_INDEX_DATA *rb[], void (*func)(int, ROOM_INDEX_DATA *, void *), void *cdata)
{
  register int i;

  for(i=0;i<WORLD_SIZE;i++)
    {
      ROOM_INDEX_DATA *temp;
  
      temp = room_find(rb,i);
      if(temp) (*func)(i,temp,cdata);
    }
}

void hash_iterate(struct hash_header *ht, void (*func)(int, void *, void *), void *cdata)
{
  int i;

  for(i=0;i<ht->klistlen;i++)
    {
      void		*temp;
      register int	key;

      key = ht->keylist[i];
      temp = hash_find(ht,key);
      (*func)(key,temp,cdata);
      if(ht->keylist[i]!=key) /* They must have deleted this room */
	i--;		      /* Hit this slot again. */
    }
}

int exit_ok( CHAR_DATA *ch, EXIT_DATA *pexit, bool especial )
{
	ROOM_INDEX_DATA *to_room;

	if ( pexit == NULL || (to_room = pexit->u1.to_room) == NULL )
		return 0;

	if ( !can_see_room(ch, to_room) )
		return 0;

	if ( IS_NPC(ch) )
	{
		if ( IS_SET(to_room->room_flags, ROOM_NO_MOB)
		&&   especial == FALSE )
			return 0;

		if ( to_room->sector_type == SECT_HOYO
		&&  !IS_AFFECTED(ch, AFF_FLYING) )
			return 0;

		if ( to_room->clan > 0 && to_room->clan != ch->clan )
			return 0;

		if ( IS_SET(to_room->room_flags, ROOM_LAW)
		&&   IS_SET(ch->act, ACT_AGGRESSIVE)
		&&   especial == FALSE )
			return 0;
	}

	return 1;
}

void donothing(void)
{
  return;
}

int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
	       int depth, int in_zone, bool especial )
{
  struct room_q		*tmp_q, *q_head, *q_tail;
  struct hash_header	x_room;
  int			i, tmp_room, count=0, thru_doors;
  ROOM_INDEX_DATA	*herep;
  ROOM_INDEX_DATA	*startp;
  EXIT_DATA		*exitp;

  if ( depth <0 )
    {
      thru_doors = TRUE;
      depth = -depth;
    }
  else
    {
      thru_doors = FALSE;
    }

  startp = get_room_index( in_room_vnum );

  init_hash_table( &x_room, sizeof(int), 2048 );
  hash_enter( &x_room, in_room_vnum, (void *) - 1 );

  /* initialize queue */
  q_head = (struct room_q *) mud_malloc(sizeof(struct room_q));
  q_tail = q_head;
  q_tail->room_nr = in_room_vnum;
  q_tail->next_q = 0;

  while(q_head)
    {
	herep = get_room_index( q_head->room_nr );

	/* for each room test all directions */
	if( herep->area == startp->area || !in_zone )
	{
		/* only look in this zone...
		   saves cpu time and  makes world safer for players */
		for ( exitp = herep->exits; exitp; exitp = exitp->next )
		{
			i = exitp->direccion;
			if( exit_ok(ch,exitp,especial) && ( thru_doors ? GO_OK_SMARTER : GO_OK ) )
			{
				/* next room */
				tmp_room = exitp->u1.to_room->vnum;
				if( tmp_room != out_room_vnum )
				{
					/* shall we add room to queue ?
					count determines total breadth and depth */
					if( !hash_find( &x_room, tmp_room )
					&& ( count < depth ) )
					{
						count++;

						/* mark room as visted and put on queue */
						tmp_q = (struct room_q *) mud_malloc(sizeof(struct room_q));
						tmp_q->room_nr = tmp_room;
						tmp_q->next_q = 0;
						q_tail->next_q = tmp_q;
						q_tail = tmp_q;

						/* ancestor for first layer is the direction */
						hash_enter( &x_room, tmp_room,
							((int)hash_find(&x_room,q_head->room_nr)
							== -1) ? (void*)(i+1)
							: hash_find(&x_room,q_head->room_nr));
					}
				}
				else
				{
					/* have reached our goal so free queue */
					tmp_room = q_head->room_nr;
					for(;q_head;q_head = tmp_q)
					{
						tmp_q = q_head->next_q;
						free(q_head);
					}
					/* return direction if first layer */
					if ((int)hash_find(&x_room,tmp_room)==-1)
					{
						if (x_room.buckets)
						{
							/* junk left over from a previous track */
							// destroy_hash_table(&x_room, donothing);
							destroy_hash_table(&x_room);
						}
						return(i);
					}
					else
					{
						/* else return the ancestor */
						i = (int)hash_find(&x_room,tmp_room);
						if (x_room.buckets)
						{
							/* junk left over from a previous track */
							// destroy_hash_table(&x_room, donothing);
							destroy_hash_table(&x_room);
						}
						return( -1+i);
					}
				}
			}
		}
	}
      
      /* free queue head and point to next entry */
      tmp_q = q_head->next_q;
      free(q_head);
      q_head = tmp_q;
    }

  /* couldn't find path */
  if( x_room.buckets )
    {
      /* junk left over from a previous track */
      // destroy_hash_table( &x_room, donothing );
      destroy_hash_table( &x_room );
    }
  return -1;
}

void do_hunt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int direction;
  bool fArea;

  one_argument( argument, arg );

  if( arg[0] == '\0' )
  {
      send_to_char( "A quien quieres cazar?\n\r", ch );
      return;
  }

  /* only imps can hunt to different areas */
  fArea = ( get_trust(ch) < MAX_LEVEL );

  if( fArea == TRUE ) // solo en el area
	victim = get_char_area( ch, arg );
  else
  if ( (victim = get_char_area( ch, arg )) == NULL )
	victim = get_char_world( ch, arg );

  if( victim == NULL || IS_SET(victim->in_room->room_flags, ROOM_NOWHERE) )
  {
      send_to_char("Nadie por aqui con ese nombre.\n\r", ch );
      return;
  }

  if( ch->in_room == victim->in_room )
  {
	act( "$N esta aqui!", ch, NULL, chToEnt(victim), TO_CHAR );
	return;
  }

  /*
   * Deduct some movement.
   */
  if( ch->move > 2 )
    ch->move -= 3;
  else
  {
      send_to_char( "Estas demasiado cansado para cazar!\n\r", ch );
      return;
  }

  act( "$n cuidadosamente olfatea el aire.", ch, NULL, NULL, TO_ROOM );

  WAIT_STATE( ch, skill_table[gsn_hunt].beats );

  direction = find_path( ch->in_room->vnum, victim->in_room->vnum,
			ch, -40000, fArea, FALSE );

  if( direction == -1 )
  {
      act( "No puedes encontrar un camino a $N desde aqui.",
	  ch, NULL, chToEnt(victim), TO_CHAR );
      stop_hunting( ch, TRUE, FALSE, FALSE );
      return;
  }

  if( direction < 0 || direction > (MAX_DIR - 1) )
  {
      send_to_char( "Hmm... parece que algo salio mal.\n\r", ch );
      stop_hunting( ch, TRUE, FALSE, FALSE );
      return;
  }

  /*
   * Give a random direction if the player misses the die roll.
   */
  if( ( IS_NPC (ch) && number_percent () > 75)        /* NPC @ 25% */
     || (!IS_NPC (ch) && number_percent () >          /* PC @ norm */
	 ch->pcdata->learned[gsn_hunt] ) )
    {
	EXIT_DATA *pExit;

	do
	{
		direction = number_door();
	}
	while((pExit = exit_lookup(ch->in_room, direction)) == NULL
	   ||  pExit->u1.to_room == NULL );
    }

  check_improve(ch,gsn_hunt,TRUE,1);

  /*
   * Display the results of the search.
   */
  act( "$N esta a $t.", ch, strToEnt(dir_nombre[direction],ch->in_room), chToEnt(victim), TO_CHAR );

  if ( es_clase(ch, CLASS_RANGER) )
  {
	if ( !is_hunting(ch) )
		set_hunt( ch, victim, FALSE );
	move_char( ch, direction, TRUE );
  }

  return;
}

int hunt_victim( CHAR_DATA *ch )
{
	int dir, opor;
	bool fArea, roomhunt = FALSE;
	CHAR_DATA *victim = huntvictim(ch);
	ROOM_INDEX_DATA * tRoom = huntroom(ch);
	EXIT_DATA *pExit;
	FRetVal retval;

	if ( tRoom == NULL
	&&   victim == NULL )
	{
		send_to_char( "tRoom && victim NULL", ch );
		return 0;
	}

	if (!ch->in_room)
	{
		send_to_char( "ch->in_room NULL", ch );
		bugf( "hunt_victim : NULL ch->in_room (1), char %s, id %ld",
			ch->name, ch->id );
		return 0;
	}

	if ( victim )
	{
		if( !can_see( ch, victim ) )
		{
			if ( IS_NPC(ch) )
			{
				REMOVE_BIT(ch->comm, COMM_NOCHANNELS );
				do_say( ch, "Maldicion!  Mi presa huyo!!" );
				send_to_char( "Tu presa huyo.\n\r", ch );
				stop_hunting( ch, TRUE, FALSE, FALSE );
			}
			else
			{
				send_to_char( "No puedes encontrar a tu victima.\n\r", ch );
				stop_hunting( ch, TRUE, TRUE, FALSE );
			}
			return 0;
		}

		if ( ch->in_room == victim->in_room )
		{
			if ( IS_NPC(ch) )
			{
				if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
				{
					send_to_char( "Tu victima esta aqui.\n\r", ch );
					act( "$n mira con odio a $N y dice, 'Debes #BMORIR#b!!!'",
						ch, NULL, chToEnt(victim), TO_NOTVICT );
					act( "$n te mira con odio y dice, 'Debes #BMORIR#b!!!'",
						ch, NULL, chToEnt(victim), TO_VICT );
					act( "Miras con odio a $N y dices, 'Debes #BMORIR#b!!!",
						ch, NULL, chToEnt(victim), TO_CHAR);
					multi_hit( ch, victim, TYPE_UNDEFINED );
					stop_hunting( ch, TRUE, FALSE, FALSE );
					return 0;
				}
				else /* de vuelta a casa */
				{
					send_to_char( "Cuarto seguro. Vuelta a casa.\n\r", ch );
					strip_mem_char( ch, MEM_HUNTING );
					strip_mem_char( ch, MEM_HOSTILE );
					strip_mem_char( ch, MEM_AFRAID );
					give_mem( ch, MEM_HOSTILE, victim->id );
					stop_hunting( ch, TRUE, TRUE, TRUE );
					if ( ch->was_in_room )
						set_hunt_room( ch, ch->was_in_room );
					return 0;
				}
			} /* NPC */
			else
			{
				send_to_char( "Tu busqueda fue completada.\n\r", ch );
				stop_hunting( ch, TRUE, TRUE, FALSE );
				return 0;
			} /* PC */
		} /* mismo cuarto? */

		tRoom = victim->in_room;
	} /* victim */
	else
	if (tRoom) /* hunteando cuartos */
	{
		if ( ch->in_room == huntroom(ch) )
		{
			stop_hunting( ch, FALSE, FALSE, TRUE );

			if ( IS_NPC(ch) )
			{
				if ( SPEC_FUN(ch) == spec_taxi )
				{
					if ( ch->in_room != ch->was_in_room )
					{
						send_to_char( "Taxi en destino.\n\r", ch );
						act( "$n dice 'Llegamos a destino. Gracias por preferir Taxis #BBDM#b.'", ch, NULL, NULL, TO_ROOM );
						die_follower(ch, FALSE);
						set_hunt_room(ch, ch->was_in_room);
					}
				}
				else
				{
					send_to_char( "Llegamos a destino.\n\r", ch );
					if ( ch->pIndexData->script )
						ch->pIndexData->script->timer = 0;
				}
			}
			else
				send_to_char( "Tu busqueda fue completada.\n\r", ch );

			return 0;
		}
		roomhunt = TRUE;
	}

	if ( ch->move < 10 )
	{
		act( "Estas demasiado cansad$o.", ch, NULL, NULL, TO_CHAR );
		stop_hunting( ch, TRUE, IS_NPC(ch) ? FALSE : TRUE, TRUE );
		return 0;
	}
	else
		ch->move -= 2;

	fArea = !IS_NPC(ch) || IS_SET(ch->act, ACT_STAY_AREA);

	dir = find_path( ch->in_room->vnum, tRoom->vnum, ch, -40000, fArea, ch->hunt_data->especial );

	if( dir < 0 || dir >= MAX_DIR )
	{
		if ( IS_NPC(ch) )
		{
			if ( SPEC_FUN(ch) == spec_taxi )
			{
				act( "$n dice 'Whoops! me perdi.'", ch, NULL, NULL, TO_ROOM );
				act( "$n desaparece!", ch, NULL, NULL, TO_ROOM );
				die_follower(ch, FALSE);
				char_from_room(ch);
				char_to_room(ch, ch->was_in_room);
				stop_hunting( ch, FALSE, FALSE, TRUE );
			}
			else
			{
				if ( IS_FORM(ch, FORM_SENTIENT) )
					act( "$n dice 'Maldicion! Lo perdi!'", ch, NULL, NULL, TO_ROOM );
				else
					act( "$n grune y refunfuna.", ch, NULL, NULL, TO_ROOM );
				send_to_char( "Lo perdiste.\n\r", ch );
				stop_hunting( ch, TRUE, FALSE, FALSE );
			}
		}
		else
		{
			send_to_char( "No puedes encontrar un camino desde aqui.\n\r", ch );
			stop_hunting( ch, TRUE, TRUE, FALSE );
		}
		return 0;
	}

	if (!ch->in_room)
	{
		bugf( "hunt_victim : NULL ch->in_room (2), char %s, id %ld",
			ch->name, ch->id );
		return 0;
	}

	pExit = exit_lookup(ch->in_room, dir);

	if ( IS_NPC(ch) )
	{
		if ( IS_SET(ch->act, ACT_CLERIC) && IS_SET(ch->act, ACT_WARRIOR))
			opor = 95;
		else if ( IS_SET(ch->act, ACT_THIEF) )
			opor = 90;
		else if ( IS_SET(ch->act, ACT_WARRIOR) )
			opor = 85;
		else if ( IS_SET(ch->act, ACT_CLERIC) || IS_SET(ch->act, ACT_MAGE) )
			opor = 80;
		else
			opor = 70;

		if ( getNivelPr(ch) > 40 )
			opor += 10;
	}
	else
		opor = get_skill( ch, gsn_hunt );

	/*
	 * Give a random direction if the mob misses the die roll.
	 */
	if( roomhunt == FALSE
	&&  number_percent () > opor )
	{
		do
		{
			dir = number_door();
		}
		while( (pExit = exit_lookup(ch->in_room, dir)) == NULL
		||     (pExit->u1.to_room == NULL) );
	}

	if ( IS_NPC(ch)
	&&  !IS_AFFECTED(ch, AFF_PASS_DOOR)
	&&   IS_SET(pExit->exit_info, EX_CLOSED) )
	{
		if ( IS_SET(pExit->exit_info, EX_LOCKED) )
		{
			do_unlock( ch, (char *) dir_name[dir] );

			if ( IS_SET(pExit->exit_info, EX_LOCKED) ) /* si todavia tiene llave */
				do_pick( ch, (char *) dir_name[dir] );
		}

		do_open( ch, (char *) dir_name[dir] );
	}

	if ( IS_NPC(ch) && SPEC_FUN(ch) == spec_taxi )
	{
		CHAR_DATA *gch;

		for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
			if ( gch != ch && gch->master == ch )
			{
				if ( DINERO(gch) < ( (getNivelPr(gch) > 10) ? 10 : 1 ) )
				{
					act( "$n te dice 'Lo lamento amigo, pero no tienes suficiente dinero.'", ch, NULL, chToEnt(gch), TO_VICT );
					stop_follower(gch);
				}
				else
				{
					deduct_cost( gch, ( ( getNivelPr(gch) > 10 ) ? 10 : 1 ) );
				}
			}
	} /* taxi */

	retval = move_char( ch, dir , TRUE);

	if (retval != fOK || char_died(ch))
		return 0;

	return PULSE_PER_SECOND;
}

void do_stop( CHAR_DATA *ch, char *argument )
{
	if ( !es_clase(ch, CLASS_RANGER) )
	{
		send_to_char( "Este comando es para un ranger solamente.\n\r", ch );
		return;
	}
	
	stop_hunting( ch, TRUE, TRUE, FALSE );
	send_to_char( "Ok.\n\r", ch );
	return;
}

bool can_hunt( CHAR_DATA *mob, CHAR_DATA *victim, bool especial )
{
	bool fArea = IS_SET(mob->act, ACT_STAY_AREA);
	int dir;

	if ( !can_see(mob,victim)
	||    mob->pIndexData->script )
	{
		send_to_char( "No puedes verlo.\n\r", mob );
		return FALSE;
	}

	if ( mob->hit < mob->max_hit / 5 )
	{
		send_to_char( "Estas muy debil.\n\r", mob );
		return FALSE;
	}

	if ( SPEC_FUN(mob) == spec_taxi )
	{
		send_to_char( "Un taxi no hace esas cosas.\n\r", mob );
		return FALSE;
	}

	if ( mob->in_room == victim->in_room )
		return TRUE;

	if ( fArea && (mob->in_room->area != victim->in_room->area) )
	{
		send_to_char( "No esta en tu area.\n\r", mob );
		return FALSE;
	}

	dir = find_path( mob->in_room->vnum, victim->in_room->vnum, mob, -40000, fArea, especial );

	if( dir < 0 || dir > (MAX_DIR - 1) )
	{
		send_to_char( "No hay un camino hacia alla desde aqui.\n\r", mob );
		return FALSE;
	}

	return TRUE;
}

void hunt_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	int tiempo = PULSE_PER_SECOND * 10;

	if ( is_hunting(ch) )
	{
		if ( ch->position < POS_STANDING )
			do_wake( ch, "" );

		if ( IS_NPC(ch)
		&&   ch->spec_fun == spec_hunter
		&&   ch->hit < ch->max_hit / 2 )
			return;

		if ( ch->position < POS_STANDING
		||   (tiempo = hunt_victim( ch )) > 0)
			char_event_add( ch, tiempo, 0, hunt_event );
	}

	return;
}

void set_hunt( CHAR_DATA *ch, CHAR_DATA *victim, bool especial )
{
	int speed = PULSE_PER_SECOND;

	if ( is_hunting(ch)
	&&   huntvictim(ch) != NULL
	&&   huntvictim(ch) != victim )
	{
		bugf( "set_hunt : victim ya existente para char %s, id %ld",
			ch->name, ch->id );
		return;
	}

	if ( !is_hunting(ch) )
		ch->hunt_data = new_hdata( );

	ch->hunt_data->victima	= victim;
	ch->hunt_data->id	= victim->id;
	ch->hunt_data->especial	= especial;

	if ( IS_AFFECTED(ch, AFF_HASTE) || IS_SET(ch->off_flags, OFF_FAST) )
		speed /= 2;

	if ( IS_NPC(ch) )
		SET_BIT(ch->act, ACT_UPDATE_ALWAYS);

	if ( ch->position == POS_SLEEPING
	||   ch->position == POS_RESTING
	||   ch->position == POS_SITTING )
		do_wake( ch, "" );

	if ( ch->position != POS_STANDING )
		return;

	if ( !event_pending(ch->events, hunt_event) )
		char_event_add( ch, speed, 0, hunt_event );
}

void set_hunt_room( CHAR_DATA *ch, ROOM_INDEX_DATA * target )
{
	int speed = PULSE_PER_SECOND * 2;

	if ( is_hunting(ch)
	&&   huntroom(ch) != NULL
	&&   huntroom(ch) != target )
	{
		bugf( "set_hunt_room : room ya existente para char %s, id %ld, room %d, target %d",
			ch->name, ch->id, huntroom(ch)->vnum, target->vnum );
		return;
	}

	if ( !is_hunting(ch) )
		ch->hunt_data = new_hdata( );

	ch->hunt_data->room	= target;
	ch->hunt_data->especial = TRUE;

	if ( IS_AFFECTED(ch, AFF_HASTE) || IS_SET(ch->off_flags, OFF_FAST) )
		speed /= 2;

	SET_BIT(ch->act, ACT_UPDATE_ALWAYS);

	if ( ch->position == POS_SLEEPING
	||   ch->position == POS_RESTING
	||   ch->position == POS_SITTING )
		do_wake( ch, "" );

	char_event_add( ch, speed, 0, hunt_event );
}

void stop_hunting( CHAR_DATA *ch, bool vict, bool victid, bool room )
{
	if ( !is_hunting(ch) )
		return;

	if ( vict )
		ch->hunt_data->victima = NULL;
	if ( victid )
		ch->hunt_data->id = 0;
	if ( room )
		ch->hunt_data->room = NULL;

	if ( ch->hunt_data->victima == NULL
	&&   ch->hunt_data->id == 0
	&&   ch->hunt_data->room == NULL )
	{
		free_hdata( ch->hunt_data );
		ch->hunt_data = NULL;
	}
}

bool can_hunt_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	bool fArea = IS_SET(ch->act, ACT_STAY_AREA);
	int dir;

	if ( fArea && (ch->in_room->area != room->area) )
	{
		send_to_char( "No esta en tu area.\n\r", ch );
		return FALSE;
	}

	dir = find_path( ch->in_room->vnum, room->vnum, ch, -40000, fArea, TRUE );

	if( dir < 0 || dir > (MAX_DIR - 1) )
	{
		send_to_char( "No hay un camino hacia alla desde aqui.\n\r", ch );
		return FALSE;
	}

	return TRUE;
}
