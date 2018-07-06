void load_plist( void );
void update_player( CHAR_DATA * );
void player_delete( char * );
PLIST * plist_lookup( const char * );
PLIST * new_plist( void );
PLIST * plist_lookup_id( long );
void free_plist(PLIST *);
#define HASHKEY(x) (LOWER(x) - 'a')
#if defined(_EVENTS_H)
void save_plist_event( EVENT * );
#endif
