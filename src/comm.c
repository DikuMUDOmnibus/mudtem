
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#endif

#if defined(MSDOS) || defined(__CYGWIN32__) || defined(_AIX)
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#endif

#if defined(WIN32)
#include <winsock2.h>
#include <fcntl.h>
#include <stdarg.h>	// para va_*
#include <signal.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#if !defined(WIN32)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <math.h>
/* #include <stdargs.h> */

#include "merc.h"
#include "recycle.h"
#include "events.h"
#include "plist.h"
#include "screen.h"
#include "lookup.h"
#include "tablesave.h"
#include "olc.h"
#include "tables.h"
#include "comm.h"
#include "ban.h"
#include "act_info.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_unread	);
void install_other_handlers(void);
char *condicion( int percent, int sex );
void greet_descriptor( DESCRIPTOR_DATA * );
DECLARE_DO_FUN(do_clear);
void ansi_check( EVENT * );
void UpdateOLCScreen( DESCRIPTOR_DATA * );
void UpdateBarraScreen( DESCRIPTOR_DATA * );

#define AUTOLEARN

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#if !defined(WIN32)
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#endif
#include "telnet.h"
#include <signal.h>
#if !defined( STDOUT_FILENO )
#define STDOUT_FILENO 1
#endif
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
pid_t   waitpid         args( ( pid_t pid, int *status, int options ) );
pid_t   fork            args( ( void ) );
int     kill            args( ( pid_t pid, int sig ) );
int     pipe            args( ( int filedes[2] ) );
int     dup2            args( ( int oldfd, int newfd ) );
/* int     execl           args( ( const char *path, const char *arg, ... ) ); */
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting accept and bind.
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
*/

int	close		args( ( int fd ) );
/* int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) ); */
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
/* int	listen		args( ( int s, int backlog ) ); */
/* int	read		args( ( int fd, char *buf, int nbyte ) ); */
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
/* int	write		args( ( int fd, char *buf, int nbyte ) ); */

pid_t   fork            args( ( void ) );
int     pipe            args( ( int filedes[2] ) );
int     dup2            args( ( int oldfd, int newfd ) );
int     execl           args( ( const char *path, const char *arg, ... ) );

#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
/*
int     gettimeofday    args( ( struct timeval *tp, void *tzp ) );
*/
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool		    MOBtrigger = TRUE;  /* act() switch                 */
int                 ftp_control = -1;   /* Control socket of FTP (-1 if inactive) */

typedef enum {NDESC_NORMAL, NDESC_FTP } ndesc_t;

/* variables */
#include "vars.h"

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS) || defined(WIN32)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
#if defined(WIN32)
void	game_loop_unix		args( ( u_int control ) );
#else
void	game_loop_unix		args( ( int control ) );
#endif // WIN32
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control, ndesc_t type ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
void    greet_ftp 		args( ( DESCRIPTOR_DATA *d ) );
void 	handle_ftp_data         ( DESCRIPTOR_DATA *, const char *);
void 	handle_ftp_auth         ( DESCRIPTOR_DATA *, const char *);
void 	handle_ftp_command      ( DESCRIPTOR_DATA *, const char *);
#endif




/*
 * Other local functions (OS-independent).
 */
bool 	output_buffer		args( ( DESCRIPTOR_DATA *d ) );
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );
bool	isansiarg		args( ( char * ) );

/* Needs to be global because of do_copyover */
int puerto, cControl;

int main( int argc, char **argv )
{
#if !defined(WIN32)
    struct timeval now_time;
#endif
    bool fCopyOver = FALSE;
    extern bool destruccion;

    netup	= TRUE;
    mob_recalc	= FALSE;
    destruccion	= FALSE;
    identd	= TRUE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
#if defined(WIN32)
	time(&current_time);
#else
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
#endif

    strftime( str_boot_time, MAX_INPUT_LENGTH,
     "%H:%M:%S, %A %d de %B de %Y", localtime(&current_time) );
/*  strcpy( str_boot_time, ctime( &current_time ) ); */

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    log_string( "Cargando variables." );
    leer_variables();

    /*
     * Get the port number.
     */
    puerto = 4040;

    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
		if ( !str_cmp( argv[1], "netdown" ) )
		{
			netup = FALSE;
			fprintf( stderr, "NetDown ACTIVADO.\n" );
		}
		else if ( !str_cmp( argv[1], "recalc" ) )
		{
			mob_recalc = TRUE;
			fprintf( stderr, "Recalculando stats.\n" );
		}
		else
		{
			fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
			fprintf( stderr, "       %s netdown\n", argv[0] );
			exit( 1 );
		}
	}
	else if ( ( puerto = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}

	if ( !IS_NULLSTR(argv[2]) && !str_cmp( argv[2], "netdown" ) )
	{
	    netup = FALSE;
	    fprintf( stderr, "Netdown ACTIVADO.\n" );
	}

	if ( !IS_NULLSTR(argv[2]) && !str_cmp( argv[2], "recalc" ) )
	{
	    mob_recalc = TRUE;
	    fprintf( stderr, "Recalculando stats.\n" );
	}

	/* Are we recovering from a copyover? */
 	if ( !IS_NULLSTR(argv[2]) && !str_cmp( argv[2], "copyover" ) )
 	{
 		fCopyOver = TRUE;
 		cControl = atoi(argv[3]);
 		ftp_control = atoi(argv[4]);
 	}
 	else
 		fCopyOver = FALSE;
    }

    /*
     * Run the game.
     */

#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Black Dragon Mud is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    mudport=puerto;

    if (!fCopyOver)
    {
	cControl = init_socket( puerto );
	ftp_control = init_socket( puerto + 6 );
    }
    boot_db();

    sprintf( log_buf, "Black Dragon Mud is ready to rock on port %d.", puerto );
    log_string( log_buf );

    if (fCopyOver)
    	copyover_recover();

    game_loop_unix( cControl );
    close (cControl);
    close (ftp_control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Terminacion normal del juego." );
    grabar_puntajes ( );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

#if defined(WIN32)
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 

	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
	{
		/* Tell the user that we couldn't find a usable */
		/* WinSock DLL.                                  */
		bugf("WSAStartup");
		exit(1);
	}

	if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET )
#else
    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
#endif
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

#if defined(WIN32)
	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof(ld) ) == SOCKET_ERROR )
#else
	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof(ld) ) < 0 )
#endif
	{
#if defined(WIN32)
		bugf("init_socket: SO_DONTLINGER: WSAGetLastError %d", WSAGetLastError() );
#else
		perror( "Init_socket: SO_DONTLINGER" );
#endif
		close(fd);
	    exit( 1 );
	}
    }
#endif

    sa				= sa_zero;
    sa.sin_family	= AF_INET;

#if defined(WIN32)
    sa.sin_port		= htons((u_short) port);
#else
    sa.sin_port		= htons(port);
#endif

#if defined(WIN32)
	if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == SOCKET_ERROR )
#else
    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
#endif
    {
#if defined(WIN32)
		bugf("init_socket: bind: WSAGetLastError %d", WSAGetLastError() );
#else
		perror("Init socket: bind" );
#endif
		close(fd);
		exit(1);
    }

#if defined(WIN32)
	if ( listen( fd, 3 ) == SOCKET_ERROR )
#else
    if ( listen( fd, 3 ) < 0 )
#endif
    {
#if defined(WIN32)
		bugf("init_socket: listen: WSAGetLastError %d", WSAGetLastError() );
#else
		perror("Init socket: listen");
#endif
		close(fd);
		exit(1);
    }

    return fd;
}
#endif

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;
    char buf[MIL];

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_GET_NEWANSI;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.showstr_head	= NULL;
    dcon.showstr_point	= NULL;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString	= NULL;			/* OLC */
    dcon.editor		= 0;			/* OLC */
    dcon.ident		= str_dup( "???" );	/* Ident */
    dcon.ifd		= -1;			/* Ident */
    dcon.ipid		= -1;			/* Ident */
    dcon.term		= 0;
    descriptor_list	= &dcon;

    install_other_handlers ();

    /*
     * Chequeo ANSI.
     */
    sprintf( buf, "Autodetectando ANSI...presiona ENTER o espera dos segundos.\n\r"
    		  "%s\e[c", echo_off_str );
    write_to_descriptor( 0, buf, 0 );
    desc_event_add( &dcon, 2 * PULSE_PER_SECOND, 0, ansi_check );

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        /* OLC */
	        if ( d->showstr_point && !d->pString )
			show_string( d, d->incomm );
	        else
	        if ( d->pString )
	        	string_add( d->character, d->incomm );
	        else
	            switch ( d->connected )
	            {
	                case CON_PLAYING:
	    		    substitute_alias( d, d->incomm );
			    break;
	                default:
			    nanny( d, d->incomm );
			    break;
	            }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
                    if ( d->character != NULL && getNivelPr(d->character) > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
                    if ( dcon.character != NULL && getNivelPr(d->character) > 1)
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

/* 
 * Here comes the ident driver code.
 * - Wreck
 */
#if defined(unix) && !defined(WIN32)
/*
 * Almost the same as read_from_buffer...
 */
bool read_from_ident( int fd, char *buffer )
{
    static char inbuf[MAX_STRING_LENGTH*2];
    int iStart, i, j, k;

    /* Check for overflow. */
    iStart = strlen( inbuf );
    if ( iStart >= sizeof( inbuf ) - 10 )
    {
	log_string( "Ident input overflow!!!" );
	return FALSE;
    }

    /* Snarf input. */
    for ( ; ; )
    {
	int nRead;

	nRead = read( fd, inbuf + iStart, sizeof( inbuf ) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( inbuf[iStart-2] == '\n' || inbuf[iStart-2] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_ident" );
	    return FALSE;
	}
    }

    inbuf[iStart] = '\0';

    /*
     * Look for at least one new line.
     */
    for ( i = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
    {
	if ( inbuf[i] == '\0' )
	    return FALSE;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
    {
	if ( inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii( inbuf[i] ) && isprint( inbuf[i] ) )
	    buffer[k++] = inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	buffer[k++] = ' ';
    buffer[k] = '\0';

    /*
     * Shift the input buffer.
     * mudftp: Do not compress multiple lines into just one.
     */
    while ( inbuf[i] == '\n' || inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( inbuf[j] = inbuf[i+j] ) != '\0'; j++ )
	;
    
    return TRUE;
}

/*
 * Process input that we got from the ident process.
 */
void process_ident( DESCRIPTOR_DATA *d )
{
    char buffer[MAX_INPUT_LENGTH];
    char address[MAX_INPUT_LENGTH];
    CHAR_DATA *ch=CH( d );
    char *user;
    sh_int results=0;
    int status;
    
    buffer[0]='\0';
    
    if ( !read_from_ident( d->ifd, buffer ) || IS_NULLSTR( buffer ) )
    	return;
    
    /* using first arg since we want to keep case */
    user=first_arg( buffer, address, FALSE );
    
    /* replace and set some states */
    if ( !IS_NULLSTR( user ) )
    {
        replace_string( d->ident, user );
        SET_BIT( results, 2 );
    }
    if ( !IS_NULLSTR( address ) )
    {
        replace_string( d->host, address );
        SET_BIT( results, 1 );
    }
    
    /* do sensible output */
    if ( results==1 ) /* address only */
    {
	/*
	 * Change the two lines below to your notification function...
	 * (wiznet, ..., whatever)
	 *
	sprintf( outbuf, "$n has address #B%s#b. Username unknown.", address );
	give_info( outbuf, ch, NULL, NULL, NOTIFY_IDENT, IMMORTAL );
	 */
	sprintf( log_buf, "#B$N#b tiene como direccion #B%s#b. Nombre de usuario desconocido.", address );
	wiznet( log_buf,ch,NULL,WIZ_LOGINS,0,0);
	sprintf( log_buf, "%s tiene como direccion %s.", ch->name, address );
	log_string( log_buf );
	if (getNivelPr(ch) > 0)
		update_player(ch);
    }
    else if ( results==2 || results==3 ) /* ident only, or both */
    {
	/*
	 * Change the two lines below to your notification function...
	 * (wiznet, ..., whatever)
	 *
	sprintf( outbuf, "$n is #B%s@%s#b.", user, address );
	give_info( outbuf, ch, NULL, NULL, NOTIFY_IDENT, IMMORTAL );
	 */
	sprintf( log_buf, "#B$N#b es #B%s@%s#b.", user, address );
	wiznet(log_buf,ch,NULL,WIZ_LOGINS,0,0);
	sprintf( log_buf, "%s es %s@%s.", ch->name, user, address );
	log_string( log_buf );
	if (getNivelPr(ch) > 0)
		update_player(ch);
    }
    else
    {
        sprintf( log_buf, "%s no pudo ser identificado.", ch->name );
        log_string( log_buf );
    }
    
    /* close descriptor and kill ident process */
    close( d->ifd );
    d->ifd=-1;
    /* 
     * we don't have to check here, 
     * cos the child is probably dead already. (but out of safety we do)
     * 
     * (later) I found this not to be true. The call to waitpid( ) is
     * necessary, because otherwise the child processes become zombie
     * and keep lingering around... The waitpid( ) removes them.
     */
    waitpid( d->ipid, &status, WNOHANG );
    d->ipid=-1;
    
    return;    
}

void create_ident( DESCRIPTOR_DATA *d, long ip, sh_int port )
{
    int fds[2];
    pid_t pid;
    
    /* create pipe first */
	if ( pipe( fds )!=0 )
    {
        perror( "Create_ident: pipe: " );
        return;
    }
    
    if ( (pid=fork( ))>0 )
    {
    	/* parent process */
    	d->ifd=fds[0];
    	close( fds[1] );
    	d->ipid=pid;
    }
    else if ( pid==0 )
    {
    	/* child process */
	char str_ip[64], str_local[64], str_remote[64];
        
    	if ( dup2( fds[1], STDOUT_FILENO )!=STDOUT_FILENO )
    	{
            perror( "Create_ident: dup2(stdout): " );
            return;
        }
        
	sprintf( str_local, "%d", mudport );
	sprintf( str_remote, "%d", port );
	sprintf( str_ip, "%ld", ip );
    	execl( BIN_DIR "resolve", "resolve", str_local, str_ip, str_remote, 0 );
    	/* Still here --> hmm. An error. */
    	log_string( "Exec failed; Closing child." );
    	exit( 0 );
    }
    else 
    {
    	/* error */
    	perror( "Create_ident: fork" );
    	close( fds[0] );
    	close( fds[1] );
    }
}
#endif


#if defined(unix)
#if defined(WIN32)
void game_loop_unix( u_int control )
#else
void game_loop_unix( int control )
#endif
{
    static struct timeval null_time;
#if !defined(WIN32)
    struct timeval last_time;

	signal( SIGPIPE, SIG_IGN );
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
#else
	time(&current_time);
#endif

    install_other_handlers();

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );

        if (ftp_control >= 0)
            FD_SET(ftp_control, &in_set);

        maxdesc = UMAX(control, ftp_control);

	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, (signed) d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	    if ( d->ifd!=-1 && d->ipid!=-1 )
	    {
	    	maxdesc = UMAX( maxdesc, d->ifd );
	    	FD_SET( d->ifd, &in_set );
	    }
	}

#if defined(WIN32)
	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) == SOCKET_ERROR )
	{
		bugf( "game_loop_unix: select: poll: WSAGetLastError %d", WSAGetLastError() );
		exit(1);
	}
#else
	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}
#endif

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control, NDESC_NORMAL );

	if ( FD_ISSET( ftp_control, &in_set ) )
	    init_descriptor( ftp_control, NDESC_FTP );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && getNivelPr(d->character) > 1)
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && getNivelPr(d->character) > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    /* check for input from the ident */
#if !defined(WIN32)
		if ( ( d->connected==CON_PLAYING || CH(d)!=NULL ) && 
	         d->ifd!=-1 && FD_ISSET( d->ifd, &in_set ) )
	        process_ident( d );
#endif

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point && !d->pString )
		    show_string( d, d->incomm );
		else
		if ( d->pString )
		    string_add( d->character, d->incomm );
		else
		switch ( d->connected )
		{
			case CON_PLAYING:
				substitute_alias( d, d->incomm );
				break;
			default:
			/* slight hack here so we can snarf all mudftp data in one go -O */
			while (d->incomm[0])
			{
				nanny (d, d->incomm);
	
				if (d->connected != CON_FTP_DATA)
					break;
	
				d->incomm[0] = '\0';
				read_from_buffer( d );
			}
			break;
	    	}
	        d->incomm[0]    = '\0';
	    }
	}

	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && getNivelPr(d->character) > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}

	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
#if !defined(WIN32)
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

		gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;

		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
			if ( errno != EINTR )
			{ 
				perror( "Game_loop: select: stall" );
				exit( 1 );
			}
	    }
	}
#endif

#if !defined(WIN32)
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
#else
	time(&current_time);
#endif
    }

    last_command[0] = '\0'; 

    return;
}
#endif

void ansi_check(EVENT *ev)
{
	DESCRIPTOR_DATA *desc = ev->item.desc;

	if (desc->connected == CON_GET_NEWANSI)
	{
		desc->connected = CON_GET_NAME;
		write_to_buffer(desc, echo_on_str, 0);
		greet_descriptor(desc);
	}
}

#if defined(unix)
void init_descriptor( int control, ndesc_t type )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int size;
#if defined(WIN32)
	SOCKET desc;
#else
	int desc;
#endif

    size = sizeof(sock);

    getsockname( control, (struct sockaddr *) &sock, &size );
    
#if defined(WIN32)
	if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) == SOCKET_ERROR )
	{
		bugf( "new_descriptor: accept: WSAGetLastError %d", WSAGetLastError() );
		return;
	}
#else
	if ( ( desc	= accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }
#endif

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

/*  if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 ) */
#if !defined(WIN32)
	if ( fcntl( desc, F_SETFL, O_NONBLOCK ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }
#endif

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();
    dnew->descriptor	= desc;

    size = sizeof(sock);

    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

#if !defined(WIN32)
	if ( identd )
		create_ident( dnew, sock.sin_addr.s_addr, ntohs( sock.sin_port ) );
#endif

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
/*	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf ); */
	if ( netup )
		from = gethostbyaddr( (char *) &sock.sin_addr,
					sizeof(sock.sin_addr), AF_INET );
	else
		from = NULL;
	dnew->host = str_dup( from ? from->h_name : buf );
        /* Use just IP address for now. */
	dnew->port=ntohs( sock.sin_port );
	sprintf( log_buf, "Creando socket %d para %s", dnew->descriptor, buf );
	log_string( log_buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( desc,
	    "Tu sitio ha sido baneado de este mud.\n\r", 0 );
	close( desc );
	free_descriptor(dnew);
	return;
    }

    /*
     * Chequeo ANSI.
     */
    if (type == NDESC_NORMAL)
    {
	sprintf( buf, "Autodetectando ANSI...presiona ENTER o espera dos segundos.\n\r"
    		  "%s\e[c", echo_off_str );
	write_to_descriptor( desc, buf, 0 );
	desc_event_add( dnew, 2 * PULSE_PER_SECOND, 0, ansi_check );
    }
    else
    if (type == NDESC_FTP)
    	greet_ftp(dnew);

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    return;
}
#endif

/*
 * Send the greeting.
 */
void greet_descriptor( DESCRIPTOR_DATA *dnew )
{
	extern char * help_greeting;
	extern char * help_greeting2;
	extern char * help_greeting3;
	extern char * help_greeting4;
	char *	temp = "";

	switch(number_range(1,4))
	{
		case 1:	temp = help_greeting;	break;
		case 2: temp = help_greeting2;	break;
		case 3: temp = help_greeting3;	break;
		case 4: temp = help_greeting4;  break;
	}

	if ( temp[0] == '.' )
	    write_to_buffer( dnew, temp + 1, 0 );
	else
	    write_to_buffer( dnew, temp , 0 );
}

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

#if !defined(WIN32)
    if ( dclose->ipid>-1 ) 
    {
		int status;

		kill( dclose->ipid, SIGKILL );
		waitpid( dclose->ipid, &status, WNOHANG );
    }
#endif

    if ( dclose->ifd>-1 )
#if defined(WIN32)
		_close( dclose->ifd );
#else
    	close( dclose->ifd );
#endif

    if ( dclose->outtop > 0 )
		process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
		write_to_buffer( dclose->snoop_by,
			"Tu victima ha dejado el juego.\n\r", 0 );
    }

    {
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->snoop_by == dclose )
				d->snoop_by = NULL;
		}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Cerrando link a %s.", ch->name );
	log_string( log_buf );
	if ( !IS_NPC(ch) && ch->pcdata->corpse )
	{
		save_corpse(ch, FALSE);
		extract_obj(ch->pcdata->corpse, FALSE);
	}
	if ( ch->pet && (ch->pet->in_room == NULL) )
	{
		char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
		extract_char( ch->pet, TRUE );
	}
	/* cut down on wiznet spam when rebooting */
	if ( dclose->connected == CON_PLAYING && !merc_down)
	{
	    act( "$n ha perdido su link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	}
	else
	{
	    free_char(dclose->original ? dclose->original : 
		dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** TRANQUILIZATE!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	/* There is no more space in the input buffer for now */
	if (sizeof(d->inbuf) - 10 - iStart == 0)
		break;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encontrado al leer." );
	    return FALSE;
	}
#if !defined(MSDOS) && !defined(WIN32)
	else if ( errno == EWOULDBLOCK )
	    break;
#endif
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;
    bool got_n, got_r;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Linea demasiado larga.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if (++d->repeat >= 25
	    &&  d->character
	    &&  d->connected == CON_PLAYING)
	    {
		flog( "%s!%s@%s flood de lectura! ('%s')",
			CH(d)->name, d->ident, d->host,
			d->incomm[0] != '!' ? d->incomm : d->inlast );
		wiznet("Flood flood flood $N flood flood flood!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		if ( d->character )
		{
			write_to_descriptor( d->descriptor,
				"\n\r*** DEJATE DE REPETIR ***\n\rCuenta hasta cinco.\n\r", 0 );
			WAIT_STATE( d->character, 5*PULSE_PER_SECOND );
		}

		d->repeat = 0;
/*
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
*/
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
/*    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++; */
     got_n = got_r = FALSE;

     for (;d->inbuf[i] == '\r' || d->inbuf[i] == '\n';i++)
          {
               if (d->inbuf[i] == '\r' && got_r++)
                       break;

               else if (d->inbuf[i] == '\n' && got_n++)
                       break;
          }
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    /*
     * Bust a prompt.
     */
    if ( !merc_down )
    {
	if ( d->showstr_point && !d->pString )
	    write_to_buffer( d, "[Presiona #BReturn#b para continuar]\n\r", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
	else if ( fPrompt && d->connected == CON_PLAYING )
	{
		CHAR_DATA *ch;
		CHAR_DATA *victim;

		ch = d->character;

	        /* battle prompt */
	        if ((victim = ch->fighting) != NULL && can_see(ch,victim))
	        {
			int percent;
			char buf[MAX_STRING_LENGTH];
			char buf2[MAX_STRING_LENGTH];
	
			if (victim->max_hit > 0)
				percent = victim->hit * 100 / victim->max_hit;
			else
				percent = -1;

			sprintf( buf, "%s %s ", SELFPERS(victim),
				condicion(percent, victim->sex) );
		
			if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_STAT) )
			{
				if (ch->max_hit > 0)
					percent = ch->hit * 100 / ch->max_hit;
				else
					percent = -1;

				sprintf( buf2, "%s %s", SELFPERS(ch),
					condicion(percent, ch->sex) );
				strcat( buf, buf2 );
			}
		
			if ( !IS_NPC(ch) && number_percent() < get_skill(ch,gsn_evaluar) )
			{
				sprintf( buf2, "(#B%d#b/#B%d#b) ",
					victim->hit, victim->max_hit );
				strcat( buf, buf2 );
			}

			strcat( buf, "\n\r" );
			buf[0] = UPPER(buf[0]);
			write_to_buffer( d, buf, 0);
		}

		ch = d->original ? d->original : d->character;
		if (!IS_SET(ch->comm, COMM_COMPACT) )
		    write_to_buffer( d, "\n\r", 2 );

	        if ( IS_SET(ch->comm, COMM_PROMPT) )
	            bust_a_prompt( d->character );

		if (IS_SET(ch->comm,COMM_TELNET_GA))
		    write_to_buffer(d,go_ahead_str,0);

		if (IS_SET(ch->comm,COMM_OLCX)
		&&  d->editor != ED_NONE
		&&  d->pString == NULL)
			UpdateOLCScreen(d);
	    }
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
/*    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    } */
    /*
     * OS-dependent output.
     *
     * now done at output_buffer( ) to deal with color codes.
     * - Wreck
     */
    return output_buffer( d );

}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *direc_name[] = {"N","E","S","W","U","D","I","O"};
 
    point = buf;

    str = IS_NPC(ch) ? PROMPT_ALL : ch->pcdata->prompt;

    if (IS_NULLSTR(str))
    {
	sprintf( buf, "<%dhp %dm %dmv> %s",
		ch->hit,
		ch->mana,
		ch->move,
		IS_NPC(ch) ? "" : ch->pcdata->prefix );
	send_to_char(buf,ch);
	return;
    }

   if (IS_SET(ch->comm,COMM_AFK))
   {
	send_to_char("<#BAFK#b> ",ch);
	return;
   }

   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
	case 'e':
	    found = FALSE;
	    doors[0] = '\0';
	    for (pexit = ch->in_room->exits; pexit; pexit = pexit->next)
	    {
		if ( pexit ->u1.to_room != NULL
		&&  (can_see_room(ch,pexit->u1.to_room)
		||   (IS_AFFECTED(ch,AFF_INFRARED) 
		&&    !IS_AFFECTED(ch,AFF_BLIND)))
		&&  !IS_SET(pexit->exit_info,EX_CLOSED))
		{
		    found = TRUE;
		    strcat(doors,direc_name[pexit->direccion]);
		}
	    }
	    if (!found)
		strcat(buf2,"ninguna");
	    sprintf(buf2,"%s",doors);
	    i = buf2; break;
 	 case 'c' :
	    sprintf(buf2,"%s","\n\r");
	    i = buf2; break;
         case 'h' :
            sprintf( buf2, "%d", ch->hit );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
         case 'm' :
            sprintf( buf2, "%d", ch->mana );
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
         case 'v' :
            sprintf( buf2, "%d", ch->move );
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp );
            i = buf2; break;
	 case 'X' :
	    sprintf(buf2, "%d", IS_NPC(ch) ? 0 :
	    (getNivelPr(ch) + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
	    i = buf2; break;
         case 'g' :
            sprintf( buf2, "%ld", ch->gold);
            i = buf2; break;
	 case 's' :
	    sprintf( buf2, "%ld", ch->silver);
	    i = buf2; break;
         case 'a' :
            if( getNivelPr(ch) > 9 )
               sprintf( buf2, "%d", ch->alignment );
            else
               sprintf( buf2, "%s", IS_GOOD(ch) ? "bueno" : IS_EVIL(ch) ?
                "malo" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", 
		((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
		 (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
		? ch->in_room->name : "oscuridad");
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->area->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
         case 'o' :
            sprintf( buf2, "%s", olc_ed_name(ch) );
            i = buf2; break;
         case 'O' :
            sprintf( buf2, "%s", olc_ed_vnum(ch) );
            i = buf2; break;
      }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   write_to_buffer( ch->desc, buf, point - buf );

   if (!IS_NPC(ch) && !IS_NULLSTR(ch->pcdata->prefix))
        write_to_buffer(ch->desc,ch->pcdata->prefix,0);
   return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
/*  strcpy( d->outbuf + d->outtop, txt ); */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 ); /* 4096 */
#if defined(WIN32)
	if ( ( nWrite = _write( desc, txt + iStart, nBlock ) ) < 0 )
#else
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
#endif
	{
		flog( "Write_to_descriptor : nWrite:%d, errno:%d, desc %d", nWrite, errno, desc );
		perror( "Write_to_descriptor" );
		return FALSE;
	}
    }

    return TRUE;
}

int count_host( char *host )
{
	DESCRIPTOR_DATA *d;
	int cnt = 0;

	for ( d = descriptor_list; d; d = d->next )
		if ( !str_cmp( host, d->host ) )
			cnt++;

	return cnt;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *desc_next;
    extern AREA_DATA * laberinto;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,weapon;
    bool fOld;
    bool temp;
    extern long logins;
    extern long logins_previos;
    extern void war_enter( DESCRIPTOR_DATA *, CHAR_DATA * );

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_FTP_AUTH:
    	handle_ftp_auth(d,argument);
    	break;

    case CON_FTP_COMMAND:
    	handle_ftp_command(d,argument);
    	break;

    case CON_FTP_DATA:
    	handle_ftp_data(d,argument);
    	break;

    case CON_GET_NEWANSI:
	write_to_buffer(d, echo_on_str, 0);
	if ( isansiarg(argument) )
    	{
		d->term = TERM_VT100;
		write_to_buffer( d, "ANSI detectado.\n\r", 0 );
	}
	else
		d->term = 0;
	greet_descriptor(d);
	d->connected = CON_GET_NAME;
	break;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	if ( argument[0] == '2' && argument[1] == 'c' )
		argument = &argument[2];

	if ( isansiarg(argument) )
	{
		int tmp = 0;

		d->term = TERM_VT100;

		for ( tmp = 0; ; tmp++ )
			if ( argument[tmp] == '\0' || isalpha(argument[tmp]) )
			{
				argument = &argument[tmp];
				break;
			}

		if ( tmp && *argument == 'c' ) /* primera letra es la c */
			argument++;
	}

	if ( argument[0] == '\0' )
	{
		if ( d->term == 0 )
		{
			write_to_buffer( d, "#BANSI#b detectado!\n\r", 0 );
			d->term = TERM_VT100;
		}
		free_char(d->character);
		d->character = NULL;
		return;
	}

	argument[0] = toupper(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Nombre ilegal, intenta otro.\n\rNombre: ", 0 );
	    free_char(d->character);
	    d->character = NULL;
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if (IS_SET(ch->act, PLR_DENY))
	{
	    sprintf( log_buf, "Negando acceso a %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "Acceso denegado.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
	{
	    write_to_buffer(d,"Tu sitio ha sido baneado de este mud.\n\r",0);
	    close_socket(d);
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch)) 
	    {
		write_to_buffer( d, "El juego esta en modo wizlock.\n\r", 0 );
		close_socket( d );
		return;
	    }

	    if ( !IS_IMMORTAL(ch) && (count_host(d->host) > max_host) )
	    {
		write_to_buffer( d, "Lo lamento, hay demasiadas personas jugando desde tu host.\n\r", 0 );
		write_to_buffer( d, "Intentalo mas rato.\n\r", 0 );
	    	close_socket( d );
	    	return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "El juego esta en modo newlock.\n\r", 0 );
                close_socket( d );
                return;
            }

	    if (check_ban(d->host,BAN_NEWBIES))
	    {
		write_to_buffer(d,
		    "Jugadores nuevos no son permitidos desde tu sitio.\n\r",0);
		close_socket(d);
		return;
	    }
	
	    sprintf( buf, "Esta bien, %s (S/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Password erroneo.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	write_to_buffer( d, echo_on_str, 0 );

	if (check_playing(d,ch->name))
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "%s@%s se ha conectado.", ch->name, d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 's' : case 'S':
            for ( d_old = descriptor_list; d_old != NULL; d_old = desc_next )
	    {
		desc_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Intento de reconeccion fracaso.\n\rNombre: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Nombre: ",0);
            if ( d->character != NULL )
            {
		if (d->character->pet && (d->character->pet->in_room == NULL))
		{
			char_to_room(d->character->pet, get_room_index(ROOM_VNUM_LIMBO));
			extract_char(d->character->pet, TRUE);
		}
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Por favor escribe S o N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'S': case 's':
	    sprintf( buf, "Jugador nuevo.\n\rDime un password para %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, cual ES, entonces? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Por favor escribe Si o No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password debe ser al menos de cinco caracteres de largo.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "Password nuevo no aceptable, intentalo de nuevo.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Por favor reescribe el password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords no son los mismos.\n\rReescribe password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

	write_to_buffer( d, "\n\rCual es tu sexo (#BM#b/#BF#b)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex			= SEX_MALE;
			    ch->pcdata->true_sex	= SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex			= SEX_FEMALE;
			    ch->pcdata->true_sex	= SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "Ese no es un sexo valido.\n\rCual ES tu sexo (#BM#b/#BF#b)? ", 0 );
	    return;
	}

	write_to_buffer( d,	"\n\rPuedes ser bueno, neutral o malo.\n\r"
				"Las razas y clases que puedas elegir dependeran de esta eleccion.\n\r"
				"Tu alineacion? (#BB#bueno, #BN#beutral o #BM#balo) ", 0 );
	d->connected = CON_GET_ALIGNMENT;
	break;

    case CON_GET_ALIGNMENT:
	switch( argument[0])
	{
	    case 'b' : case 'B' :
	    ch->alignment = 750;
	    ch->pcdata->true_align = ALIGN_GOOD;
	    break;

	    case 'n' : case 'N' :
	    ch->alignment = 0;
	    ch->pcdata->true_align = ALIGN_NEUTRAL;
	    break;

	    case 'm' : case 'M' :
	    ch->alignment = -750;
	    ch->pcdata->true_align = ALIGN_EVIL;
	    break;

	    default:
		write_to_buffer(d,"Esa no es una alineacion valida.\n\r",0);
		write_to_buffer(d,"Que alineacion (#BB#b/#BN#b/#BM#b)? ",0);
		return;
	}

	write_to_buffer(d,"\n\rLas siguientes razas estan disponibles:\n\r",0);
	write_to_buffer(d,"Ciertas clases no estaran disponibles, dependiendo de la raza que\n\r",0);
	write_to_buffer(d,"elijas. Usa tu sentido comun.\n\r  ",0);
	listar_razas(d, ch);
	write_to_buffer(d,"Cual es tu raza? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    write_to_buffer(d,"\n\r",0);
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"race help");
	    else
		do_help(ch,argument);
            write_to_buffer(d, "\n\rCual es tu raza? ",0);
	    break;
  	}

	race = race_lookup(argument);

	if (race == 0
	|| !race_table[race].pc_race
	|| !race_table[race].seleccionable
	|| (IS_GOOD(ch)    && IS_SET(race_table[race].noalign, NOALIGN_GOOD))
	|| (IS_EVIL(ch)    && IS_SET(race_table[race].noalign, NOALIGN_EVIL))
	|| (IS_NEUTRAL(ch) && IS_SET(race_table[race].noalign, NOALIGN_NEUTRAL))
	|| (race_table[race].remort_race && !IS_SET(ch->act, PLR_REMORT)) )
	{
	    write_to_buffer(d,"\n\rEsa no es una raza valida.\n\r",0);
            write_to_buffer(d,"Las siguientes razas estan disponibles:\n\r  ",0);
	    listar_razas(d, ch);
            write_to_buffer(d, "Cual es tu raza? ",0);
	    break;
	}

	write_to_buffer( d, "\n\r", 0 );
	do_help(ch, race_table[race].name );
	ch->race = ch->true_race = race;
	write_to_buffer( d, "\n\rEstas seguro que deseas elegir esta raza? ", 0);
	d->connected = CON_CONFIRMAR_RAZA;
	break;

	case CON_CONFIRMAR_RAZA:
	if ( LOWER(argument[0]) != 's' )
	{
		write_to_buffer( d, "\n\rLas siguientes razas estan disponibles:\n\r", 0 );
		listar_razas(d, ch);
		write_to_buffer( d, "\n\rCual es tu raza? ", 0 );
		d->connected = CON_GET_NEW_RACE;
		break;
	}
		
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
		ch->perm_stat[i] = race_table[ch->race].stats[i];
	ch->affected_by		= race_table[ch->race].aff;
	ch->affected2_by	= race_table[ch->race].aff2;
	ch->imm_flags		= race_table[ch->race].imm;
	ch->res_flags		= race_table[ch->race].res;
	ch->vuln_flags		= race_table[ch->race].vuln;
	ch->form		= race_table[ch->race].form;
	ch->parts		= race_table[ch->race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (race_table[ch->race].skills[i] == NULL)
	 	break;
	    group_add(ch,race_table[ch->race].skills[i],FALSE);
	}

	/* add cost */
	ch->pcdata->points	= race_table[ch->race].points;
	ch->size		= race_table[ch->race].size;

	temp = FALSE;

	strcpy( buf, "\n\rElige una clase [" );
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( (!class_table[iClass].remort_class ||
	          (class_table[iClass].remort_class && IS_SET(ch->act,PLR_REMORT)))
	       && ((IS_GOOD(ch)    && !IS_SET(class_table[iClass].noalign, NOALIGN_GOOD))
	        || (IS_EVIL(ch)    && !IS_SET(class_table[iClass].noalign, NOALIGN_EVIL))
	        || (IS_NEUTRAL(ch) && !IS_SET(class_table[iClass].noalign, NOALIGN_NEUTRAL)))
	       && race_table[ch->race].noclase[iClass] == FALSE )
	    {
		if ( temp == FALSE )
			temp = TRUE;
		else
			strcat( buf, " " );

		strcat( buf, class_table[iClass].name );
	    }
	}
	strcat( buf, "]: " );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	iClass = class_lookup(argument);

	if ( iClass == -1
	|| (class_table[iClass].remort_class && !IS_SET(ch->act,PLR_REMORT))
	|| (IS_GOOD(ch)    && IS_SET(class_table[iClass].noalign, NOALIGN_GOOD))
        || (IS_EVIL(ch)    && IS_SET(class_table[iClass].noalign, NOALIGN_EVIL))
        || (IS_NEUTRAL(ch) && IS_SET(class_table[iClass].noalign, NOALIGN_NEUTRAL))
	|| race_table[ch->race].noclase[iClass] == TRUE )
	{
	    write_to_buffer( d,
		"Esa no es una clase.\n\rCual ES tu clase? ", 0 );
	    return;
	}

	setClasePr(ch, iClass, 0);

        group_add(ch,"rom basics",FALSE);
        group_add(ch,class_table[getClasePr(ch)].base_group,FALSE);
        ch->pcdata->learned[gsn_recall] = 50;
	group_add(ch,class_table[getClasePr(ch)].default_group,TRUE);

	sprintf( log_buf, "%s@%s jugador nuevo.", ch->name, d->host );
	log_string( log_buf );
	wiznet("Alerta de Newbie!  #B$N#b en la mira.",ch,NULL,WIZ_NEWBIE,0,0);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	write_to_buffer( d, "\n\rAhora vas a elegir tus stats iniciales.\n\r",0);

	for (i = 0; i < MAX_STATS; i++)
		ch->perm_stat[i] = roll_stat(ch,i);

	write_to_buffer( d, "#U          Fuerza Inteli Sabidu Destre Consti#u\n\r", 0 );

	sprintf( buf,         "Maximos     %2d     %2d     %2d     %2d     %2d\n\r",
		get_max_train(ch, STAT_STR),
		get_max_train(ch, STAT_INT),
		get_max_train(ch, STAT_WIS),
		get_max_train(ch, STAT_DEX),
		get_max_train(ch, STAT_CON) );
	write_to_buffer( d, buf, 0 );

	sprintf( buf,         "Actual      %2d     %2d     %2d     %2d     %2d   Aceptas(S/N)? ",
		ch->perm_stat[STAT_STR],
		ch->perm_stat[STAT_INT],
		ch->perm_stat[STAT_WIS],
		ch->perm_stat[STAT_DEX],
		ch->perm_stat[STAT_CON] );
	write_to_buffer( d, buf, 0 );

	d->connected = CON_GET_STATS;
	break;

    case CON_GET_STATS:
	switch ( argument[0] )
	{
	case 's': case 'S':
		if ( class_table[getClasePr(ch)].multiclase == FALSE )
		{
			write_to_buffer(d, "\n\rPor favor elige un arma de las siguientes opciones:\n\r",0);
			buf[0] = '\0';
			for ( i = 0; weapon_table[i].name != NULL; i++)
				if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
				{
				    strcat(buf,weapon_table[i].name);
				    strcat(buf," ");
				}
			strcat(buf,"\n\rTu opcion? ");
			write_to_buffer(d,buf,0);
			d->connected = CON_PICK_WEAPON;
		}
		else
		{
			write_to_buffer( d, "\n\rDeseas ser multiclase (S/N)? ", 0 );
			d->connected = CON_GET_MULTICLASE;
		}
		break;

	default:
		for (i = 0; i < MAX_STATS; i++)
		    ch->perm_stat[i] = roll_stat(ch,i);
		sprintf( buf,         "Actual      %2d     %2d     %2d     %2d     %2d   Aceptas(S/N)? ",
			ch->perm_stat[STAT_STR],
			ch->perm_stat[STAT_INT],
			ch->perm_stat[STAT_WIS],
			ch->perm_stat[STAT_DEX],
			ch->perm_stat[STAT_CON] );
		write_to_buffer( d, buf, 0 );
		break;
	}
	break;

    case CON_GET_MULTICLASE:
	switch( argument[0] )
	{
		default:
		write_to_buffer( d,	"Esa no es una respuesta valida.\n\r"
					"Quieres ser multiclase (S/N)? ", 0);
		break;

		case 'n': case 'N':
		write_to_buffer(d, "\n\rPor favor elige un arma de las siguientes opciones:\n\r",0);
		buf[0] = '\0';
		for ( i = 0; weapon_table[i].name != NULL; i++)
			if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
			{
			    strcat(buf,weapon_table[i].name);
			    strcat(buf," ");
			}
		strcat(buf,"\n\rTu opcion? ");
		write_to_buffer(d,buf,0);
		d->connected = CON_PICK_WEAPON;
		break;
        
		case 's': case 'S':
		write_to_buffer( d,	"\n\rCual sera tu segunda clase?\n\r"
					"Tus opciones son:\n\r", 0 );

		for ( i = 0; i < MAX_CLASS; i++ )
			if ( i != getClasePr(ch)
			&&   puede_ser_clase( getClasePr(ch), ch->race, i ) )
			{
				sprintf( buf, "%d. %s\n\r", i, class_table[i].name );
				write_to_buffer( d, buf, 0 );
			}

		write_to_buffer( d, "\n\rTu opcion? ", 0 );
		d->connected = CON_GET_SEGUNDA_CLASE;
		break;
	}
	break;

    case CON_GET_SEGUNDA_CLASE:
	{
		i = class_lookup(argument);
		if ( i == -1
		||   i == getClasePr(ch)
		||  !puede_ser_clase(getClasePr(ch), ch->race, i) )
		{
			write_to_buffer( d,	"Esa no es una clase valida.\n\r"
						"Cual sera tu segunda clase? \n\r", 0 );
			break;
		}

		addClase(ch, i, 1);
		group_add(ch,class_table[i].base_group,FALSE);
		group_add(ch,class_table[i].default_group,TRUE);
		write_to_buffer(d, "\n\rPor favor elige un arma de las siguientes opciones:\n\r",0);
		buf[0] = '\0';
		for ( i = 0; weapon_table[i].name != NULL; i++)
			if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
			{
			    strcat(buf,weapon_table[i].name);
			    strcat(buf," ");
			}
		strcat(buf,"\n\rTu opcion? ");
		write_to_buffer(d,buf,0);
		d->connected = CON_PICK_WEAPON;
	}
	break;

    case CON_PICK_WEAPON:
	write_to_buffer(d,"\n\r",2);
	weapon = weapon_lookup(argument);
	if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
	{
	    write_to_buffer(d,
		"Esa no es una seleccion valida. Opciones son:\n\r",0);
            buf[0] = '\0';
            for ( i = 0; weapon_table[i].name != NULL; i++)
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                {
                    strcat(buf,weapon_table[i].name);
		    strcat(buf," ");
                }
            strcat(buf,"\n\rTu opcion? ");
            write_to_buffer(d,buf,0);
	    return;
	}

	ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
	do_help(ch,"NEWBIE INFO");
	d->connected = CON_READ_NMOTD;
	break;

    case CON_BEGIN_REMORT:
	write_to_buffer( d, "Empezando el proceso de remort.\n\r\n\r", 0 );
	write_to_buffer( d, "Las siguientes razas estan disponibles:\n\r  ", 0 );
	for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race || !race_table[race].seleccionable)
		break;
	    write_to_buffer(d,race_table[race].name,0);
	    write_to_buffer(d," ",1);
	}
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"Cual es tu raza (help para mas informacion)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
        do_help( ch, "motd" );
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_NMOTD:
	write_to_buffer(d,"\n\r",2);
	do_help(ch,"motd");
	write_to_buffer(d,"\n\r",2);
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
        if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
        {
            write_to_buffer( d, "#B#FALERTA#b#f! Password nulo!\n\r",0 );
            write_to_buffer( d, "Por favor reporta el password antiguo con un bug.\n\r",0);
            write_to_buffer( d,
                "Escribe '#Bpassword null <password nuevo>#b' para arreglarlo.\n\r",0);
        }

	write_to_buffer( d, "\n\rBienvenido a #BROM#b 2.4.  Por favor no alimenten a los mobiles.\n\r", 0 );

	if ( war == TRUE )
		war_enter(d, ch);

	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

	logins++;
	reset_char(ch);

	printf_to_char( ch, "Han habido #B%d#b conecciones desde Diciembre de 1998.\n\r",
		logins );
	printf_to_char( ch, "Han habido #B%d#b conecciones desde el inicio del sistema.\n\r",
		logins - logins_previos );

	if ( ch->was_in_room && ch->was_in_room->area == laberinto )
	{
		extern void entrar_laberinto(CHAR_DATA *);

		if ( laberinto->nplayer == 0 )
			entrar_laberinto(ch);
		else
			ch->was_in_room = get_room_index(ROOM_VNUM_TEMPLE);
	}

	if ( getNivelPr(ch) == 0 )
	{
	    setNivelPr(ch,1);
	    ch->exp		= exp_per_level(ch,ch->pcdata->points);
	    ch->hit		= ch->max_hit;
	    ch->mana		= ch->max_mana;
	    ch->move		= ch->max_move;
	    ch->train		= 3;
	    ch->practice	= 5;
            ch->pcdata->who_text = str_dup( "@" );
#if defined (AUTOLEARN)                         /* Set default language */
            ch->pcdata->speaking = race_table[ch->race].race_lang;
            ch->pcdata->language[race_table[ch->race].race_lang] = 100;
            ch->pcdata->language[COMMON] = 100;
            ch->pcdata->learn = 5;
#endif
	    sprintf( buf, "%s", ch->sex == SEX_MALE ? "el novato" : "la novata" );
	    set_title( ch, buf );

	    do_outfit(ch,"");
	    obj_to_char(create_object( get_obj_index(OBJ_VNUM_MAP),0),ch);
	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    send_to_char("\n\r",ch);
	}
	else if ( ch->was_in_room != NULL )
	{
	    char_to_room( ch, ch->was_in_room );
	}
	else if ( ch->in_room != NULL )
	{
		bugf( "nanny : ch->in_room == %d, char %s",
			ch->in_room->vnum, ch->name );
		char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	act( "$n ha entrado al juego.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );

	wiznet("$N ha dejado atras la vida real.",ch,NULL,
		WIZ_LOGINS,WIZ_SITES,get_trust(ch));

	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n ha entrado al juego.",ch->pet,NULL,NULL,TO_ROOM);
	}

	do_unread(ch,"");
	if ( is_clan(ch) && CLAN_STATUS(ch) >= CLAN_MINISTRO )
		check_new_peticiones(ch);
	if ( !check_peticiones() )
		grabar_peticiones();
	update_player(ch);
	break;
    }

    return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_name( name, 
	"all auto immortal self someone something the you demise balance circle honor none quit exit") )
	return FALSE;
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    if (clan_lookup(name) > 0)
    	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (signed) (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		if (d->character->pet && d->character->pet->in_room == NULL)
		{
			char_to_room(d->character->pet, get_room_index(ROOM_VNUM_LIMBO) );
			extract_char(d->character->pet, TRUE);
		}

		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;

		send_to_char(
		    "Reconectando. Escribe 'replay' para leer tells recibidos.\n\r", ch );
		act( "$n se ha reconectado.", ch, NULL, NULL, TO_ROOM );

		sprintf( log_buf, "%s@%s se ha reconectado.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N revive su coneccion.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   dold->connected != CON_GET_NEWANSI
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "Ese jugador ya esta en el juego.\n\r",0);
	    write_to_buffer( d, "Deseas conectarte de todas maneras (S/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n ha vuelto del Vacio.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
    {
    	if (ch->desc->pString) /* esta editando un string */
    	{
    		char * blah;
    		int length = 0;

		if ( ch->desc->showstr_head )
			length = strlen(ch->desc->showstr_head) + 1;

		length += strlen(txt) + 1;

		blah = alloc_mem( length );
		if ( ch->desc->showstr_head )
			strcpy ( blah, ch->desc->showstr_head );
		else
			blah[0] = '\0';
		strcat( blah, txt );
		if ( ch->desc->showstr_head )
			free_mem( ch->desc->showstr_head, strlen(ch->desc->showstr_head) + 1 );
		ch->desc->showstr_head = blah;
		ch->desc->showstr_point = ch->desc->showstr_head;
	}
	else
		write_to_buffer( ch->desc, txt, strlen(txt) );
    }

    return;
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return; /* fix? */

    if (IS_NPC(ch) || (ch->pcdata->lines == 0) )
    {
	send_to_char(txt,ch);
	return;
    }

#if defined(macintosh)
	send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
/*	    free_mem(d->showstr_head,strlen(d->showstr_head)); */
	    free_mem(d->showstr_head,strlen(d->showstr_head) + 1); /* fix? */
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (IS_NPC(d->character) || d->character == NULL)
    	show_lines = 0;
    else
    	show_lines = d->character->pcdata->lines;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_mem(d->showstr_head,strlen(d->showstr_head));
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void new_act( const char *format, Entity *ent, Entity * arg1,
	      Entity * arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "eso",	"el",	"ella"	};
    static char * const him_her [] = { "eso",	"el",	"ella"	};
    static char * const his_her [] = { "su",	"su",	"su"	};
    static char * const o_a	[] = { "o",	"o",	"a"	};
    static char * const o_a_u	[] = { "O",	"O",	"A"	};
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    char tmpbuf[MIL];
    ROOM_INDEX_DATA *room;
    CHAR_DATA *to;
    CHAR_DATA *ch = NULL, *vch = NULL;
    const char *str;
    const char *i;
    char *point;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    if ( ent == NULL )
	return;

    if ( (room = entWhereIs(ent)) == NULL )
    {
    	bugf( "new_act : ent %s(%s) en cuarto NULL, format %s",
    		entidadToString(ent), entidadGetTipo(ent), format );
    	return;
    }

    ch	= entidadEsCh(ent) ? entidadGetCh(ent) : NULL;
    vch = entidadEsCh(arg2) ? entidadGetCh(arg2) : NULL;
    to = room->people;

    if ( type == TO_VICT )
    {
	if ( arg2 == NULL )
	{
		bugf( "Act: null vch with TO_VICT, ent %s, arg1 %s, format %s.",
			entToStringExt(ent),
			entToStringExt(arg1),
			format );
		return;
	}

	if ( (room = entWhereIs(arg2)) == NULL)
	{
		bugf( "new_act : arg2 %s(%s) en cuarto NULL, format %s",
			entidadToString(arg2), entidadGetTipo(arg2),
			format );
		return;
	}

	to = room->people;
    }

	for ( ; to != NULL; to = to->next_in_room )
	{
		if ( ( to->desc == NULL && ( !IS_NPC(to) || !HAS_TRIGGER(to, TRIG_ACT) ) )
		||	(to->position < min_pos) )
			continue;

		if ( type == TO_CHAR )
		{
			if ( !entEsCh(ent) )
				continue;
			if (ch != NULL
			&&  to != ch)
				continue;
		}

		if ( type == TO_VICT
		&&   vch != NULL
		&& ( to != vch || (ch != NULL && to == ch ) ) )
			continue;

		if ( type == TO_ROOM
		&&   ch != NULL
		&&   to == ch )
			continue;

		if ( type == TO_NOTVICT
		&& ((ch && to == ch) || (vch && to == vch) || IS_SET(to->comm, COMM_NOSPAM)) )
			continue;

		point   = buf;
		str     = format;

		while ( *str != '\0')
		{
			if ( *str != '$' )
			{
				*point++ = *str++;
				continue;
			}
			++str;

			if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
			{
				bugf( "Act: missing arg2 for code %d, format %s.", *str, format );
				i = " <@@@> ";
			}
			else
			{
				switch ( *str )
				{
					default:  bugf( "Act: bad code %d, format %s.", *str, format );
					i = " <@@@> ";
					break;

                			/* Thx alex for 't' idea */
                			case 't': i = arg1 ? entidadToString(arg1) : "ERROR";		break;
                			case 'T': i = arg2 ? entidadToString(arg2) : "ERROR";		break;
                			case 'n': i = newPERS( ent, to );				break;
                			case 'N': i = arg2 ? newPERS( arg2, to ) : "ERROR";		break;
                			case 'e': i = ch ? he_she   [URANGE(0, ch  ->sex, 2)] : "";	break;
                			case 'E': i = vch ? he_she  [URANGE(0, vch ->sex, 2)] : "";	break;
                			case 'm': i = ch ? him_her  [URANGE(0, ch  ->sex, 2)] : "";	break;
                			case 'M': i = vch ? him_her [URANGE(0, vch ->sex, 2)] : "";	break;
                			case 's': i = ch ? his_her  [URANGE(0, ch  ->sex, 2)] : "";	break;
                			case 'S': i = vch ? his_her [URANGE(0, vch ->sex, 2)] : "";	break;
                			case 'o': i = ch ? o_a      [URANGE(0, ch  ->sex, 2)] : "";	break;
                			case 'O': i = vch ? o_a     [URANGE(0, vch ->sex, 2)] : "";	break;
                			case 'k': i = ch ? o_a_u    [URANGE(0, ch  ->sex, 2)] : "";	break;
                			case 'K': i = vch ? o_a_u   [URANGE(0, vch ->sex, 2)] : "";	break;

                			case 'p':
                    			i = arg1 ? newPERS(arg1, to) : "ERROR";
                    			break;

                			case 'P':
                    			i = arg2 ? newPERS(arg2, to) : "ERROR";
                    			break;

                			case 'd':
                    			if ( arg2 == NULL
                    			||  !entidadEsString(arg2)
                    			||   entidadGetString(arg2)[0] == '\0' )
                    			{
                        			i = "puerta";
                    			}
                    			else
                    			{
						strcpy(tmpbuf, entidadToString(arg2) );
                        			one_argument( tmpbuf, fname );
                        			i = fname;
                    			}
                    			break;
                		}
            		}

            		++str;
            		while ( ( *point = *i ) != '\0' )
            		    ++point, ++i;
        	}
 
        	*point++ = '\n';
        	*point++ = '\r';
 		*point   = '\0';
        	buf[0]   = UPPER(buf[0]);

 		if ( to->desc )
			send_to_char( buf, to );

 		if ( IS_NPC(to)
 		&&   HAS_TRIGGER(to, TRIG_ACT)
 		&&   MOBtrigger )
 			mp_act_trigger( buf, chToEnt(to), chToEnt(ch), arg1, arg2, TRIG_ACT );
    	}

	return;
}

/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

/*
 * output_buffer( descriptor )
 * this function sends output down a socket. Color codes are stripped off
 * is the player is not using color, or converted to ANSI color sequences
 * to provide colored output.
 * When using ANSI, the buffer can become a lot larger due to the (sometimes)
 * lengthy ANSI sequences, thus potentially overflowing the buffer. Therefor
 * *new* buffer is send in chunks.
 * The 'bzero's may seem unnecessary, but i didn't want to take risks.
 *
 * - Wreck
 */

bool output_buffer( DESCRIPTOR_DATA *d )
{
    char	buf[MAX_STRING_LENGTH];
    char	buf2[128];
    const char 	*str;
    char 	*i;
    char 	*point;
    bool	flash=FALSE, o_flash, 
    		bold=FALSE, o_bold,
    		reverse=FALSE, o_reverse,
    		underline=FALSE, o_underline;
    bool	do_act=FALSE, ok=TRUE, color_code=FALSE;
    int		color=ANSI_WHITE, o_color;

    /* discard NULL descriptor */
    if ( d==NULL )
    	return FALSE;

    bzero( buf, MAX_STRING_LENGTH ); 
    point=buf;
    str=d->outbuf;
    o_color=color;
    o_bold=bold;
    o_flash=flash;
    o_reverse=reverse;
    o_underline=underline;

    while ( *str != '\0' && (str-d->outbuf)<d->outtop )
    {
    	if ( (int)(point-buf)>=MAX_STRING_LENGTH-32 )
    	{
    	    /* buffer is full, so send it through the socket */
    	    *point++='\0';
    	    if ( !(ok=write_to_descriptor( d->descriptor, 
    	    				   buf, 
    	    				   strlen( buf ) )) )
    	        break;
    	    bzero( buf, MAX_STRING_LENGTH ); 
    	    point=buf;
    	}

    	if ( *str != '#' )
    	{
    	    color_code=FALSE;
    	    *point++ = *str++;
    	    continue;
    	}

    	if ( !color_code && *(str+1)!='<' )
    	{
    	    o_color=color;
    	    o_bold=bold;
    	    o_flash=flash;
    	    o_reverse=reverse;
    	    o_underline=underline;
    	}
    	color_code=TRUE;

    	do_act=FALSE;
    	str++;
    	switch ( *str )
    	{
    	    default:    sprintf( buf2, "#%c", *str ); 		       break;
    	    case 'x': 	sprintf( buf2, "#" );		     	       break;
    	    case '-': 	sprintf( buf2, "~" );		     	       break;
    	    case '<': 	color=o_color; bold=o_bold;
			flash=o_flash; reverse=o_reverse;
			underline=o_underline;		     do_act=TRUE; break;
    	    case '0':	color=0;	 		     do_act=TRUE; break;
    	    case '1':	color=1; 			     do_act=TRUE; break;
    	    case '2':	color=2; 			     do_act=TRUE; break;
    	    case '3':	color=3;	 		     do_act=TRUE; break;
    	    case '4':	color=4;		 	     do_act=TRUE; break;
    	    case '5':	color=5; 			     do_act=TRUE; break;
    	    case '6':	color=6;		 	     do_act=TRUE; break;
    	    case '7':	color=7;			     do_act=TRUE; break;
    	    case 'B':	bold=TRUE;   			     do_act=TRUE; break;
    	    case 'b':	bold=FALSE;  			     do_act=TRUE; break;
    	    case 'F':	flash=TRUE; 		 	     do_act=TRUE; break;
    	    case 'f':	flash=FALSE; 			     do_act=TRUE; break;
    	    case 'R':   reverse=TRUE;			     do_act=TRUE; break;
	    case 'r':   reverse=FALSE;			     do_act=TRUE; break;    	    
	    case 'U':	underline=TRUE;			     do_act=TRUE; break;
	    case 'u':	underline=FALSE;		     do_act=TRUE; break;
    	    case 'n':	if ( d->term > 0)
				sprintf( buf2, term_table[d->term].clearattr );
			else
				buf2[0]='\0';
			do_act = underline = reverse = flash = bold = FALSE;
			color = ANSI_WHITE;
			break;
    	}
	if ( do_act )
	{
	    if ( d->term > 0 )
	    {
		sprintf( buf2, "%s",
			color_value_string( d->term,
			IS_SET(term_table[d->term].tipo, TIPO_COLOR) ? color : -1,
			bold, flash, reverse, underline ) );
		color_code=TRUE;
	    }
	    else
	        buf2[0]='\0';
        }

        i=buf2;
        str++;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
    }

    *point++='\0'; 
    ok=ok && (write_to_descriptor( d->descriptor, buf, strlen( buf ) ));
    d->outtop=0;

    return ok;
}

/* source: EOD, by John Booth <???> */
/* stick this in in comm.c somewhere */
/* Remember to include <stdargs.h> */

void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	send_to_char (buf, ch);
}

char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}

void write_last_command (void)
{
    int fd;

    /* Return if no last command - set before normal exit */
    if (!last_command[0])
        return;

    fd = open (LAST_COMMAND_FILE, O_WRONLY|O_CREAT);

    if (fd < 0)
        return;

    write (fd, last_command, strlen(last_command));
    write (fd, "\n", 1);
    close (fd);
}

void nasty_signal_handler (int no)
{
    write_last_command();
    return;
}


/* Call this before starting the game_loop */
void install_other_handlers ()
{
/*    last_command [0] = NULL; */
    last_command [0] = '\0';

    if (atexit (write_last_command) != 0)
    {
        perror ("install_other_handlers:atexit");
        exit (1);
    }

    /* should probably check return code here */
    signal (SIGSEGV, nasty_signal_handler);

    /* Possibly other signals could be caught? */
}

/*
 * Write to all in the room.
 */
void send_to_room( const char *txt, ROOM_INDEX_DATA *room )
{
    DESCRIPTOR_DATA *d;
    
    for ( d = descriptor_list; d; d = d->next )
        if ( d->character != NULL )
	    if ( d->character->in_room == room )
	        act( txt, d->character, NULL, NULL, TO_CHAR );
}

void flog (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	log_string (buf);
}

void clanlog (int clan, char * fmt, ...)
{
	char buf [2*MSL];
	char fname[128];
	va_list args;

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	sprintf( fname, CLANLOG_DIR "%s", get_clan_table(clan)->name );

	append_file( NULL, fname, buf );
}

void bugf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	bug (buf, 0);
}

char *condicion( int percent, int sex )
{
	if (percent >= 100)
		return "esta en excelente condicion.";
	else if (percent >= 90)
		return "tiene unos pocos rasgunos.";
	else if (percent >= 75)
		return "tiene algunas heridas y moretones.";
	else if (percent >= 50)
		return "tiene unas cuantas heridas.";
	else if (percent >= 30)
		return "tiene grandes heridas y rasgunos.";
	else if (percent >= 15)
	{
		if ( sex == SEX_FEMALE )
			return "se ve muy herida.";
		else
			return "se ve muy herido.";
	}
	else if (percent >= 0)
		return "esta en pesima condicion.";
	else
		return "se desangra a morir.";
}

void InitScreenMap(DESCRIPTOR_DATA *d)
{
	int i;

	if (d->screenmap == NULL)
		d->screenmap = calloc(80*(d->character->pcdata->lines - 3) + 1, sizeof(char));
	if (d->oldscreenmap == NULL)
		d->oldscreenmap = calloc(80*(d->character->pcdata->lines - 3) + 1, sizeof(char));
	for (i = 0; i < 80*(d->character->pcdata->lines - 3); i++)
		d->screenmap[i] = d->oldscreenmap[i] = ' ';
}

const struct olc_show_table_type * tiene_olcx( int editor )
{
	switch(editor)
	{
		default:	return NULL;
		case ED_MOBILE:	return medit_olc_show_table;
		case ED_OBJECT: return oedit_olc_show_table;
		case ED_ROOM:	return redit_olc_show_table;
	}

	return NULL;
}

void UpdateOLCScreen(DESCRIPTOR_DATA *d)
{
	char buf[MSL*2], buf2[MSL*2];
	void * point;
const	struct olc_show_table_type * tabla = tiene_olcx(d->editor);
	int blah, i, largo, x, y, j;
extern	ROOM_INDEX_DATA xRoom;
	STRFUNC * func;
	char * tmpstr;
const	struct flag_type * flagt;

	if (tabla == NULL)
		return;

	if (d->screenmap == NULL || d->oldscreenmap == NULL)
		InitScreenMap(d);

	switch(d->editor)
	{
		default:	return;
		case ED_ROOM:	blah = (int) &xRoom;
				break;
		case ED_MOBILE:	blah = (int) &xMob;
				break;
		case ED_OBJECT:	blah = (int) &xObj;
				break;
	}

	write_to_buffer( d, VT_CURSAVE, 0 );

	tmpstr		= d->oldscreenmap;
	d->oldscreenmap	= d->screenmap;
	d->screenmap	= tmpstr;

	for ( i = 0; tabla[i].nombre != NULL; i++ )
	{
		point = (void *) ((int) tabla[i].point - (int) blah + (int) d->pEdit);

		if (tabla[i].pagina != d->pagina)
			continue;

		switch(tabla[i].tipo)
		{
			default:break;
			case OLCS_STRING:
			strcpy(buf, *(char **) point);
			break;

			case OLCS_INT:
			strcpy(buf, itos(*(int *)point));
			break;

			case OLCS_SHINT:
			strcpy(buf, itos(*(sh_int *)point));
			break;

			case OLCS_STRFUNC:
			func = (STRFUNC *) tabla[i].func;
			tmpstr = (*func) (point);
			strcpy(buf, tmpstr ? tmpstr : "" );
			break;

			case OLCS_FLAGSTR_INT:
			flagt = (const struct flag_type *) tabla[i].func;
			strcpy(buf, flag_string(flagt, *(int *)point));
			break;

			case OLCS_FLAGSTR_SHINT:
			flagt = (const struct flag_type *) tabla[i].func;
			strcpy(buf, flag_string(flagt, *(sh_int *) point));
			break;

			case OLCS_BOOL:
			strcpy(buf, *(bool *) point == TRUE ? "S" : "N");
			break;

			case OLCS_TAG:
			buf[0] = '\0';
			break;
		}

		strcpy(buf2, tabla[i].desc);
		strcat(buf2, colorstrip(buf));
		largo = strlen(buf2);
		x = tabla[i].x;
		y = tabla[i].y;
		for ( j = 0; j < largo; j++ )
		{
			if (buf2[j] == '\r')
			{
				x = tabla[i].x;
				continue;
			}
			if (buf2[j] == '\n')
			{
				y++;
				continue;
			}
			if ((tabla[i].largox < 1 && x > 79)
			||  (tabla[i].largox > 0 && x >= tabla[i].x + tabla[i].largox + strlen(tabla[i].desc)))
			{
				y++;
				x = tabla[i].x;
			}
			if ((tabla[i].largoy < 1 && y > d->character->pcdata->lines-3)
			||  (tabla[i].largoy > 0 && y >= tabla[i].y + tabla[i].largoy))
				break;
			if (((y-1)*80+(x-1)) >= 80*(d->character->pcdata->lines-3))
				break;
			d->screenmap[(y-1)*80+(x++-1)] = buf2[j];
		}
	}

	// mandamos solo las diferencias
	largo = strlen(d->screenmap);
	i = j = 0;
	buf[0] = '\0';
	while(i<largo)
	{
		if ( d->screenmap[i] == d->oldscreenmap[i] )
		{
			i++;
			continue;
		}
		sprintf(buf2, VT_CURSPOS "%c", i/80+1, i%80+1, d->screenmap[i++] );
		strcat(buf, buf2);
		j += strlen(buf2);
		while ( d->screenmap && d->screenmap[i] != d->oldscreenmap[i] )
			buf[j++] = d->screenmap[i++];
		buf[j] = '\0';
	}

	write_to_buffer( d, buf, j );
	write_to_buffer( d, VT_CURREST, 0 );
}

void UpdateBarraScreen(DESCRIPTOR_DATA *d)
{
	char buf[MIL];
	int size;
	CHAR_DATA *ch = d->character;

	if ( d->term == 0 || !ch )
		return;

	size = IS_NPC(ch) ? PAGELEN : (ch->pcdata->lines + 2);

	write_to_buffer(d, VT_CURSAVE, 0);

	sprintf(buf, VT_CURSPOS, size, 1);
	write_to_buffer(d, buf, 0);

	sprintf(buf, "#R Hp:%4d/%4d M:%3d/%3d Mv:%3d/%3d Exp:%6d Gold:%5ld Silver:%5ld %9s #r",
		UMIN(ch->hit, 9999),
		UMIN(ch->max_hit, 9999),
		UMIN(ch->mana, 999),
		UMIN(ch->max_mana, 999),
		UMIN(ch->move, 999),
		UMIN(ch->max_move, 999),
		UMIN(ch->exp, 999999),
		UMIN(ch->gold, 99999),
		UMIN(ch->silver, 99999),
		ch->name );
	write_to_buffer( d, buf, 0 );

	write_to_buffer( d, VT_CURREST, 0 );
}
 
void InitScreen(DESCRIPTOR_DATA *d)
{
	char buf[MIL];
	int size;
	CHAR_DATA * ch = d->character;

	if (!IS_SET(ch->comm, COMM_OLCX))
		return;

	size = IS_NPC(ch) ? PAGELEN : (ch->pcdata->lines + 2);

	send_to_char(VT_HOMECLR, ch);

	if ( d->editor != ED_NONE
	&&   tiene_olcx(d->editor) )
	{
		InitScreenMap(d);
		sprintf(buf, VT_MARGSET, size - 4, size);
		send_to_char(buf, ch);
		sprintf(buf, VT_CURSPOS "OK!\n\r", size - 4, 0);
		send_to_char(buf, ch);
	}
}

void do_netstat( CHAR_DATA *ch, char *argument )
{
	char buf[MIL];

	netup = !netup;
	
	sprintf( buf, "Gethostbyaddr esta #B%s#b.\n\r", netup ? "ACTIVADO" : "DESACTIVADO" );
	send_to_char( buf, ch );
}

bool isansiarg( char *argument )
{
    	if ( argument[0] != '\0'
	&& ( ( argument[0] == '['
    	    && argument[1] == '?' )
    	  || ( argument[0] == '^'
    	    && argument[1] == '[' ) ) )
		return TRUE;

	return FALSE;
}
