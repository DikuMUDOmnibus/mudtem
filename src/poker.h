#define NUMERO(x) ( poker->poker[(x)] / 4 )
#define FIGURA(x) ( poker->poker[(x)] % 4 )
#define MAX_CARTAS (52)

typedef struct poker_data POKER_DATA;

struct poker_data
{
      POKER_DATA  *next;
      CHAR_DATA   *jugador;
      sh_int       poker[5];
      long         cambiar;
      bool         turno;
};

POKER_DATA  *poker_list;
POKER_DATA  *poker_free;
bool         cartas[MAX_CARTAS];
int          poker_stat = POKER_NADA;
long         pozo;
long	     ultap;
int          numero_jugadores;
int	     numero_cartas;

long         pot2 (int i);
bool         es_jugador (CHAR_DATA * ch);
bool         todos_cambiaron (void);
POKER_DATA  *get_poker_data (CHAR_DATA * ch);
int          cartas_camb (POKER_DATA * poker);
void         inscrib_char (CHAR_DATA * ch);
POKER_DATA  *new_poker_data (void);
void         elim_char (CHAR_DATA * ch);
void         free_poker (POKER_DATA * poker);
void         limpiar_poker (void);
void	     mensaje_jugadores( char *buf );
int	     valor_cartas (POKER_DATA *poker);
