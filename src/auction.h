typedef struct  auction_data            AUCTION_DATA; /* auction data */

struct auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

extern AUCTION_DATA * auction;
