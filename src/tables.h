/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

#if !defined(_TABLES_H)
#define _TABLES_H

extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];

/* flag tables */
extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern  const	struct	flag_type	furniture_flags[];
extern  const	struct	flag_type	trap_flags[];
extern	const	struct	flag_type	trapd_flags[];
extern	const	struct	recval_type	recval_table[];
extern	const	struct	flag_type	target_table[];
extern	const	struct	flag_type	dam_classes[];
extern	const	struct	flag_type	log_flags[];
extern	const	struct	flag_type	show_flags[];
extern	const	struct	flag_type	oprog_flags[];
extern	const	struct	flag_type	rprog_flags[];
extern	const	struct	flag_type	stat_table[];
extern	const	struct	flag_type	petstat_table[];
extern	const	struct	flag_type	con_table[];

extern	const	struct	flag_type 	area_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	affect2_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	vuln_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	bit_type	bitvector_type[];
extern	const	struct	flag_type	mem_types[];
extern	const	struct	flag_type	clan_flags[];
extern	const	struct	flag_type	detect_table[];
extern	const	struct	taxi_dest_type	taxi_dest[];
extern	const	struct	event_type	event_table[];
extern	const	struct	flag_type	skill_flags[];
extern	const	struct	flag_type	noalign_table[];
extern	const	struct	flag_type	mat_table[];

struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
};

struct recval_type
{
	int numhit;
	int typhit;
	int bonhit;
	int ac;
	int numdam;
	int typdam;
	int bondam;
};

struct taxi_dest_type
{
	char *	lugar;
	int	vnum;
};

#define OLCS_STRING		1
#define OLCS_INT		2
#define OLCS_SHINT		3
#define OLCS_STRFUNC		4
#define OLCS_FLAGSTR_INT	5
#define OLCS_FLAGSTR_SHINT	6
#define OLCS_BOOL		7
#define OLCS_TAG		8

struct olc_show_table_type
{
	char *		nombre;
	void *		point;
	char *		desc;
	sh_int		tipo;
	sh_int		x;
	sh_int		y;
	sh_int		largox;
	sh_int		largoy;
	sh_int		pagina;
	const void *	func;
};

extern const struct olc_show_table_type redit_olc_show_table[];
extern const struct olc_show_table_type medit_olc_show_table[];
extern const struct olc_show_table_type oedit_olc_show_table[];

typedef char * STRFUNC ( void * );

extern STRFUNC areaname;

/* handler.c */
int flagtablepos( const struct flag_type *, int );

#endif // _TABLES_H
