struct lang_type
{
    char *     name;
};

extern  const   struct  lang_type       lang_table      [ MAX_LANGUAGE ];

/* language.c */
void    do_language     args( ( CHAR_DATA *ch, char *argument, int language) );
int     lang_lookup     args( ( const char *name ) );
