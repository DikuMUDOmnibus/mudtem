#include "include.h"
#include "command.h"
#include "smart.h"
#include "events.h"
#include "recycle.h"

bool pc_in_room(ROOM_INDEX_DATA *, CHAR_DATA *);
bool can_cast_spell(CHAR_DATA *, int);

int spell_dam_mage( CHAR_DATA *, CHAR_DATA * );
int spell_aff_mage( CHAR_DATA *, CHAR_DATA * );

#if !defined(USAR_MACROS)
bool is_mob_safe( CHAR_DATA *ch )
{
	if ( ch->in_room == NULL )
		return TRUE;

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
		return TRUE;

	if ( ch->pIndexData->pShop != NULL
	||   ch->pIndexData->pRepair != NULL )
		return TRUE;

	if ( IS_SET(ch->act, ACT_IS_HEALER)
	||   IS_SET(ch->act, ACT_PRACTICE)
	||   IS_SET(ch->act, ACT_TRAIN)
	||   IS_SET(ch->act, ACT_IS_CHANGER)
	||   IS_SET(ch->act, ACT_BANKER)
	||   IS_SET(ch->act, ACT_PROTOTIPO) )
		return TRUE;

	return FALSE;
}
#endif

bool can_cast( CHAR_DATA *ch, int sn, int clase )
{
	if ( sn < 1 )
	{
		bugf( "can_cast : spell %d invalido", sn );
		return FALSE;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_CONE_OF_SILENCE)
	&&   clase != CLASS_PSI )
		return FALSE;

	return (mana_cost(sn, getNivelPr(ch), clase) < ch->mana);
}

bool can_disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	OBJ_DATA *weapon;

	if ( get_eq_char(ch, WEAR_WIELD) == NULL
	&&   get_skill(ch, gsn_hand_to_hand) == 0 )
		return FALSE;

	if ( (weapon = get_eq_char(victim, WEAR_WIELD)) == NULL
	 &&  (weapon = get_eq_char(victim, WEAR_SECONDARY)) == NULL )
		return FALSE;

	if ( IS_OBJ_STAT(weapon, ITEM_NOREMOVE) )
		return FALSE;

	return TRUE;
}

#if !defined(USAR_MACROS)
bool can_circle( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_NPC(ch) && get_eq_char(ch, WEAR_WIELD) == NULL )
		return FALSE;

	if ( victim->fighting == ch )
		return FALSE;

	return TRUE;
}
#endif

int count_friends( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *gch;
	int cnt = 0;

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
		if ( gch != ch && gch->fighting == victim )
			cnt++;

	return cnt;
}

bool will_flee( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance = (getNivelPr(victim) - getNivelPr(ch))*5;

	if ( IS_AFFECTED2(ch, AFF_HOLD) )
		return FALSE;

	if ( IS_MOB_CASTER(ch)
	&&   IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
		return TRUE;

	if ( ch->hit < (ch->max_hit / 4)
	&&   IS_SET(ch->act, ACT_WIMPY) )
		chance += 20;

	if ( IS_MOB_CASTER(ch) && ch->mana < ch->max_mana / 6 )
		chance += 20;

	if ( IS_MOB_MAGIC(ch) && !CAN_SAY(ch) )
		chance += 30;

	if ( ES_THIEF(ch)
	&&   ch->hit < ch->max_hit / 3 )
		chance += 20;

	if ( ch->hit > ch->max_hit / 2 )
		chance -= 50;

	if ( IS_AFFECTED(ch, AFF_SANCTUARY) )
		chance -= 10;
	else
		chance += 10;

	if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
		chance += 10;
	else
		chance -= 10;

	if ( victim->hit > victim->max_hit / 2 )
		chance += getNivelPr(victim) - getNivelPr(ch);

	if ( ES_WARRIOR(ch) )
		chance /= 2;

	if ( IS_SET(ch->act, ACT_SENTINEL) )
		chance /= 2;

	if ( ES_THIEF(ch)
	&&   count_friends(ch, victim) > 0 )
		chance /= (count_friends(ch,victim) + 1);

	return CHANCE(chance);
}

void mob_fight( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;
	CHAR_DATA *victim = get_char_id_room( ch, (long) ev->param );

	if ( !victim || !ch->fighting )
		return;

	if ( ch->wait )
	{
		char_event_add( ch, ch->wait, (void *) victim->id, mob_fight );
		return;
	}

	if ( ch->position == POS_RESTING )
	{
		do_stand( ch, "" );
		char_event_add( ch, 1, (void *) victim->id, mob_fight );
		return;
	}

	if ( IS_AFFECTED2(ch, AFF_HOLD)
	&&   CHANCE(getNivelPr(ch)) )
	{
		do_untangle(ch, "");
		char_event_add( ch, ch->wait, (void *) victim->id, mob_fight );
		return;
	}

	switch(getClasePr(ch))
	{
		case CLASS_WARRIOR:	act_warrior(ch, victim);	break;
		case CLASS_THIEF:	act_thief(ch, victim);		break;
		case CLASS_MAGE:	act_mage(ch, victim);		break;
		case CLASS_CLERIC:	act_cleric(ch, victim);		break;
		case CLASS_PSI:		act_psi(ch, victim);		break;
	}

	if ( !char_died(ch) && !event_pending(ch->events, mob_fight) )
		char_event_add( ch, ch->wait ? ch->wait : 1, (void *) victim->id, mob_fight );
}

void act_mage( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int spell = -1;
	char buf[MIL];

	if ( !IS_PET(ch)
	&&  ((ch->hit > ch->max_hit / 2) || CHANCE(20)) )
		spell = spell_aff_mage(ch, victim);

	if ( spell == -1 )
		spell = spell_dam_mage(ch, victim);

	if ( spell > 0 && CAN_SAY(ch) )
	{
		sprintf( buf, "'%s'", skill_table[spell].name );
		do_cast( ch, buf );
		return;
	}

	if ( ES_WARRIOR(ch) )
		act_warrior( ch, victim );
	else
	if ( ES_THIEF(ch) )
		act_thief( ch, victim );
	else
	if ( will_flee(ch, victim) )
	{
		do_flee( ch, "" );
		if ( ch->position == POS_STANDING )
		{
			if ( !IS_AFFECTED(ch, AFF_SNEAK) )
				do_sneak( ch, "" );
			if ( !IS_AFFECTED(ch, AFF_HIDE) )
				do_hide( ch, "" );
			if ( IS_MOB_CASTER(ch) )
			{
				if ( can_cast_spell( ch, gsn_teleport ) )
				{
					sprintf( buf, "teleport %s", ch->name );
					do_cast( ch, buf ); /* mejor perderse */
				}
				else
				if ( can_cast_spell( ch, gsn_invis ) )
					do_cast( ch, "invisibility" );
			}
		}
	}
}

void act_cleric( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MIL];
	int spell = 0;

	if ( ch->hit < ch->max_hit / 2
	||   victim->hit < victim->max_hit / 2
	||   CHANCE(getNivelPr(ch)) )
	{
		if (((ES_WARRIOR(ch) && getNivelPr(ch) > 22)
		 ||   getNivelPr(ch) > 20)
		&&   ch->hit + 100 < ch->max_hit
		&&   can_cast( ch, gsn_heal, CLASS_CLERIC )
		&&   CHANCE(50) )
			spell = gsn_heal;
		else
		if ( getNivelPr(ch) > 35
		&&   ch->hit < ch->max_hit / 4
		&&  !IS_NEUTRAL(ch)
		&& ( (IS_GOOD(ch) && IS_EVIL(victim))
		  || (IS_EVIL(ch) && IS_GOOD(victim)) )
		&&   can_cast( ch, gsn_holy_word, CLASS_CLERIC )
		&&   CHANCE(80) )
			spell = gsn_holy_word;
		else
		if ( getNivelPr(ch) > 34
		&&   IS_GOOD(ch)
		&&  !IS_GOOD(victim)
		&&   can_cast( ch, gsn_ray_of_truth, CLASS_CLERIC )
		&&   CHANCE(80) )
			spell = gsn_ray_of_truth;
		else
		if ( getNivelPr(ch) > 33
		&&   IS_EVIL(ch)
		&&   can_cast( ch, gsn_demonfire, CLASS_CLERIC )
		&&   CHANCE(70) )
			spell = gsn_demonfire;
		else
		if ( getNivelPr(ch) > 19
		&&  !ES_IMMUNE(victim, IMM_FIRE)
		&&   can_cast( ch, gsn_flamestrike, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_flamestrike;
		else
		if ( getNivelPr(ch) > 17
		&&   IS_OUTSIDE(ch)
		&&   weather_info.sky >= SKY_RAINING
		&&   can_cast( ch, gsn_call_lightning, CLASS_CLERIC )
		&&   CHANCE(70) )
			spell = gsn_call_lightning;
		else
		if ( IS_EVIL(victim)
		&&   getNivelPr(ch) > 15
		&&   can_cast( ch, gsn_dispel_evil, CLASS_CLERIC )
		&&   CHANCE(50) )
			spell = gsn_dispel_evil;
		else
		if ( IS_GOOD(victim)
		&&   getNivelPr(ch) > 15
		&&   can_cast( ch, gsn_dispel_good, CLASS_CLERIC )
		&&   CHANCE(50) )
			spell = gsn_dispel_good;
		else
		if ( getNivelPr(ch) > 22
		&&   can_cast( ch, gsn_lightning_bolt, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_lightning_bolt;
		else
		if ( getNivelPr(ch) > 22
		&&   can_cast( ch, gsn_harm, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_harm;
		else
		if ( getNivelPr(ch) > 12
		&&   can_cast( ch, gsn_cause_critical, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_cause_critical;
		else
		if ( getNivelPr(ch) > 9
		&&   can_cast( ch, gsn_earthquake, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_earthquake;
		else
		if ( getNivelPr(ch) > 6
		&&   can_cast( ch, gsn_cause_serious, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_cause_serious;
		else
		if ( can_cast(ch, gsn_cause_light, CLASS_CLERIC) )
			spell = gsn_cause_light;
	}
	else
	{
		if ( !IS_AFFECTED2(victim, AFF_MUTE)
		&&   getNivelPr(ch) > 18
		&&  !saves_spell(getNivelPr(ch), victim, DAM_OTHER)
		&&  (es_caster(victim) || CHANCE(getNivelPr(victim)/2))
		&&   can_cast( ch, gsn_mute, CLASS_CLERIC ) )
			spell = gsn_mute;
		else
		if ( !IS_AFFECTED(victim, AFF_FAERIE_FIRE)
		&&    getNivelPr(ch) > 2
		&&    can_cast( ch, gsn_faerie_fire, CLASS_CLERIC )
		&&    CHANCE(90) )
			spell = gsn_faerie_fire;
		else
		if ( getNivelPr(ch) > 23
		&& ( IS_AFFECTED(victim, AFF_SANCTUARY)
		 || (IS_AFFECTED(victim, AFF_HASTE) && (!IS_NPC(victim) || !IS_SET(victim->off_flags, OFF_FAST)))
		 || (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
		 || (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch)))
		&&  !saves_spell(getNivelPr(ch), victim, DAM_OTHER)
		&&   can_cast( ch, gsn_dispel_magic, CLASS_CLERIC ) )
			spell = gsn_dispel_magic;
		else
		if ( getNivelPr(ch) > 16
		&&   victim->carrying
		&&  !saves_spell(getNivelPr(ch), victim, DAM_FIRE)
		&&   CHANCE(70)
		&&   can_cast( ch, gsn_heat_metal, CLASS_CLERIC ) )
			spell = gsn_heat_metal;
		else
		if ( !IS_AFFECTED(victim, AFF_BLIND)
		&&   getNivelPr(ch) > 7
		&&  !saves_spell(getNivelPr(ch), victim, DAM_LIGHT)
		&&   CHANCE(80)
		&&   can_cast( ch, gsn_blindness, CLASS_CLERIC ) )
			spell = gsn_blindness;
		else
		if ( getNivelPr(ch) > 30
		&&  !IS_AFFECTED(victim, AFF_SLOW)
		&&   CHANCE(40)
		&&   can_cast( ch, gsn_slow, CLASS_CLERIC )
		&&  !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
			spell = gsn_slow;
		else
		if ( !IS_AFFECTED(victim, AFF_PLAGUE)
		&&    getNivelPr(ch) > 16
		&&    can_cast( ch, gsn_plague, CLASS_CLERIC )
		&&   !saves_spell(getNivelPr(ch), victim, DAM_DISEASE)
		&&    CHANCE(60) )
			spell = gsn_plague;
		else
		if ( !IS_AFFECTED(victim, AFF_POISON)
		&&    getNivelPr(ch) > 12
		&&    can_cast( ch, gsn_poison, CLASS_CLERIC )
		&&   !saves_spell(getNivelPr(ch), victim, DAM_POISON)
		&&    CHANCE(60) )
			spell = gsn_poison;
		else
		if ( !IS_AFFECTED2(victim, AFF_ESTUPIDEZ)
		&&    getNivelPr(ch) > 25
		&&    can_cast( ch, gsn_estupidez, CLASS_CLERIC )
		&&   !saves_spell(getNivelPr(ch), victim, DAM_MENTAL)
		&&    CHANCE(30) )
			spell = gsn_estupidez;
		else
		if ( !IS_AFFECTED(victim, AFF_CURSE)
		&&   getNivelPr(ch) > 17
		&&   can_cast( ch, gsn_curse, CLASS_CLERIC )
		&&   CHANCE(60) )
			spell = gsn_curse;
		else
		if ( !IS_AFFECTED(victim, AFF_CALM)
		&&    getNivelPr(ch) > 29
		&&    can_cast( ch, gsn_calm, CLASS_CLERIC )
		&&    CHANCE(50) )
			spell = gsn_calm;
		else
		if ( can_cast(ch, gsn_cause_light, CLASS_CLERIC) )
			spell = gsn_cause_light;
	}

	if (spell < 0)
	{
		bugf( "Act_cleric : spell %d invalido", spell );
		return;
	}

	if ( spell > 0 && CAN_SAY(ch) )
	{
		sprintf(buf,"'%s'", skill_table[spell].name );
		do_cast( ch, buf );
		return;
	}

	if ( ES_WARRIOR(ch) || ES_THIEF(ch) )
		while(TRUE)
			switch(number_range(0,1))
			{
				case 0:	if ( ES_WARRIOR(ch) )
					{
						act_warrior(ch, victim);
						return;
					}
					break;
				case 1: if ( ES_THIEF(ch) )
					{
						act_thief(ch, victim);
						return;
					}
					break;
			}

	if ( will_flee(ch, victim) )
		do_flee( ch, "" );
}

void act_warrior( CHAR_DATA *ch, CHAR_DATA *victim )
{
	switch( number_range(0,4) )
	{
		case 0:
		if ( IS_PART(ch, PART_TAIL)
		&&   victim->position >= POS_FIGHTING )
		{
			do_tail( ch, "" );
			return;
		}
		break;

		case 1:
		if ( !IS_AFFECTED(ch, AFF_BERSERK)
		&&   !IS_AFFECTED(ch, AFF_CALM)
		&&    ch->mana > 50
		&&    IS_SET(ch->off_flags, OFF_BERSERK) )
		{
			do_berserk( ch, "" );
			return;
		}
		break;

		case 2:
		if ( IS_SET(ch->off_flags, OFF_DISARM)
		&&   can_disarm(ch, victim) )
		{
			do_disarm( ch, "" );
			return;
		}
		break;

		case 3:
		if ( IS_SET(ch->off_flags, OFF_BASH)
		&&   victim->position >= POS_FIGHTING )
		{
			do_bash( ch, "" );
			return;
		}
		break;

		case 4:
		if ( IS_SET(ch->off_flags, OFF_KICK) )
		{
			do_kick( ch, "" );
			return;
		}
		break;
	}

	if ( ES_THIEF(ch) )
		act_thief( ch, victim );
	else
	if ( will_flee(ch, victim) )
		do_flee( ch, "" );
}

void act_thief( CHAR_DATA *ch, CHAR_DATA *victim )
{
	switch(number_range(0,5))
	{
		case 0:
		if ( IS_SET(ch->off_flags, OFF_CIRCLE)
		&&   can_circle(ch, victim)
		&& (!IS_PET(ch) || IS_NPC(victim)) )
		{
			do_circle( ch, "" );
			return;
		}
		break;

		case 1:
		if ( !IS_AFFECTED2(victim, AFF_HOLD)
		&&   !IS_SET(victim->form, FORM_ANIMAL)
		&&   !IS_PET(ch) )
		{
			do_snare( ch, "" );
			return;
		}
		break;

		case 2:
		if ( victim->position >= POS_FIGHTING
		&&  !IS_AFFECTED(victim, AFF_FLYING)
		&&   IS_SET(ch->off_flags, OFF_TRIP) )
		{
			do_trip( ch, "" );
			return;
		}
		break;

		case 3:
		if ( !IS_AFFECTED(victim, AFF_BLIND)
		&&    IS_SET(ch->off_flags, OFF_KICK_DIRT)
		&&   !IS_PET(ch) )
		{
			do_dirt( ch, "" );
			return;
		}
		break;

		case 4:
		if ( IS_SET(ch->off_flags, OFF_KICK) )
		{
			do_kick( ch, "" );
			return;
		}
		break;

		case 5:
		if ( IS_SET(ch->off_flags, OFF_DISARM)
		&&   can_disarm(ch, victim) )
		{
			do_disarm( ch, "" );
			return;
		}
		break;
	}

	if ( will_flee(ch, victim) )
		do_flee( ch, "" );
}

void act_psi( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int spell = 0;
	char buf[MIL];

	if ( getNivelPr(ch) > 29
	&&   IS_EVIL(ch)
	&&   can_cast( ch, gsn_death_field, CLASS_PSI )
	&&   CHANCE(40) )
		spell = gsn_death_field;
	else
	if ( getNivelPr(ch) > 29
	&&   can_cast( ch, gsn_hellspawn, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_hellspawn;
	else
	if ( getNivelPr(ch) > 24
	&&   can_cast( ch, gsn_ultrablast, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_ultrablast;
	else
	if ( getNivelPr(ch) > 19
	&&   can_cast( ch, gsn_detonate, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_detonate;
	else
	if ( getNivelPr(ch) > 16
	&&   can_cast( ch, gsn_psionic_blast, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_psionic_blast;
	else
	if ( getNivelPr(ch) > 13
	&&   can_cast( ch, gsn_energy_drain, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_energy_drain;
	else
	if ( getNivelPr(ch) > 12
	&&   can_cast( ch, gsn_ego_whip, CLASS_PSI )
	&&   CHANCE(80) )
		spell = gsn_ego_whip;
	else
	if ( getNivelPr(ch) > 8
	&&   can_cast( ch, gsn_project_force, CLASS_PSI )
	&&   CHANCE(75) )
		spell = gsn_project_force;
	else
	if ( getNivelPr(ch) > 7
	&&   can_cast( ch, gsn_psychic_crush, CLASS_PSI )
	&&   CHANCE(75) )
		spell = gsn_psychic_crush;
	else
	if ( getNivelPr(ch) > 5
	&&   can_cast( ch, gsn_agitation, CLASS_PSI )
	&&   CHANCE(75) )
		spell = gsn_agitation;
	else
	if ( getNivelPr(ch) > 3
	&&   can_cast( ch, gsn_psychic_drain, CLASS_PSI )
	&&   CHANCE(75) )
		spell = gsn_psychic_drain;
	else
	if ( can_cast( ch, gsn_mind_thrust, CLASS_PSI ) )
		spell = gsn_mind_thrust;

	if (spell < 0)
	{
		bugf( "Act_psi : spell %d invalido", spell );
		return;
	}

	if (spell > 0)
	{
		sprintf(buf,"'%s'", skill_table[spell].name );
		do_cast( ch, buf );
		return;
	}

	if ( ES_WARRIOR(ch) )
		act_warrior( ch, victim );
	else
	if ( ES_THIEF(ch) )
		act_thief( ch, victim );
	else
	if ( will_flee(ch, victim) )
		do_flee( ch, "" );
}

MF_DATA *mfd_list;

bool is_mob_fighting( CHAR_DATA *ch )
{
	MF_DATA *mfd;

	for ( mfd = mfd_list; mfd; mfd = mfd->next )
		if ( mfd->mob == ch )
			return TRUE;

	return FALSE;
}

void stop_mob_fight( CHAR_DATA *ch )
{
	MF_DATA *mfd, *prev;

	if ( mfd_list == NULL )
	{
		bug( "Stop_mob_fight : mfd_list NULL, mob %d", ch->pIndexData->vnum );
		return;
	}

	if ( !is_mob_fighting(ch) )
	{
		bug( "Stop_mob_fight : mob %d no esta peleando", ch->pIndexData->vnum );
		return;
	}

	if ( mfd_list->mob == ch )
	{
		mfd		= mfd_list;
		mfd_list	= mfd_list->next;
		free_mfd(mfd);
	}
	else
	{
		for ( prev = mfd_list; prev; prev = prev->next )
			if ( prev->next && prev->next->mob == ch )
				break;

		if ( !prev )
		{
			bug( "Stop_mob_fight : prev NULL, mob %d", ch->pIndexData->vnum );
			return;
		}

		mfd		= prev->next;
		prev->next	= mfd->next;
		free_mfd( mfd );
	}
}

void set_mob_fight( CHAR_DATA *ch )
{
	MF_DATA *mfd;

	mfd		= new_mfd();
	mfd->mob	= ch;
	mfd->next	= mfd_list;
	mfd_list	= mfd;
}

void mf_update( void )
{
	MF_DATA *mfd;

	for ( mfd = mfd_list; mfd; mfd = mfd->next )
	{
		if ( mfd->mob->wait > 0 )
			mfd->mob->wait--;
		if ( mfd->mob->daze > 0 )
			mfd->mob->daze--;
	}
}

int getmin( int *arr, int n )
{
	int x = 100, j, k = -1;

	for ( j = 0; j < n; ++j )
		if ( x == -1 || x > arr[j] )
		{
			k = j;
			x = arr[j];
		}

	return k;
}

bool can_cast_spell( CHAR_DATA *ch, int sn )
{
	static int levarr[5];
	int levmage	= LEVEL_HERO,
	    levpsi	= LEVEL_HERO,
	    levcleric	= LEVEL_HERO,
	    levwar	= LEVEL_HERO,
	    levth	= LEVEL_HERO,
	    clase, level, cual;

	if ( sn < 1 )
	{
		bugf( "Can_cast_spell : spell %d inexistente", sn );
		return FALSE;
	}

	if ( is_affected(ch, sn) )
		return FALSE;

	if ( ES_MAGE(ch) )
		levmage = skill_table[sn].skill_level[CLASS_MAGE];
	if ( ES_CLERIC(ch) )
		levcleric = skill_table[sn].skill_level[CLASS_CLERIC];
	if ( ES_PSI(ch) )
		levpsi = skill_table[sn].skill_level[CLASS_PSI];
	if ( ES_WARRIOR(ch) )
		levwar = skill_table[sn].skill_level[CLASS_WARRIOR];
	if ( ES_THIEF(ch) )
		levth = skill_table[sn].skill_level[CLASS_THIEF];

	levarr[0]	= levmage;
	levarr[1]	= levcleric;
	levarr[2]	= levpsi;
	levarr[3]	= levwar;
	levarr[4]	= levth;

	cual		= getmin( levarr, 5 );

	switch(cual)
	{
		default:
		bugf("Can_cast_spell : cual == -1, vnum %d", CHARVNUM(ch) );
		return FALSE;

		case 0: clase = CLASS_MAGE;	break;
		case 1: clase = CLASS_CLERIC;	break;
		case 2: clase = CLASS_PSI;	break;
		case 3: clase = CLASS_WARRIOR;	break;
		case 4: clase = CLASS_THIEF;	break;
	}

	level = levarr[cual];

	if ( getNivelPr(ch) < level )
		return FALSE;

	if ( clase != CLASS_PSI && !CAN_SAY(ch) )
		return FALSE;

	if ( !can_cast( ch, sn, clase ) )
		return FALSE;

	return TRUE;
}

bool selfspell( CHAR_DATA *ch )
{
	int spell = 0;
	OBJ_DATA *obj = NULL;

	if ( (IS_AFFECTED(ch, AFF_POISON)
	   || IS_AFFECTED(ch, AFF_CURSE))
	&&   ES_PSI(ch)
	&&   can_cast_spell(ch, gsn_cell_adjustment) )
		spell = gsn_cell_adjustment;
	else
	if ( IS_AFFECTED(ch, AFF_FAERIE_FIRE)
	&&   can_cast_spell(ch, gsn_cancellation) )
		spell = gsn_cancellation;
	else
	if ( IS_AFFECTED(ch, AFF_PLAGUE)
	&&   can_cast_spell(ch, gsn_cure_disease) )
		spell = gsn_cure_disease;
	else
	if ( IS_AFFECTED(ch,AFF_POISON)
	&&   can_cast_spell(ch, gsn_cure_poison) )
		spell = gsn_cure_poison;
	else
	if ( IS_AFFECTED(ch,AFF_BLIND)
	&&  !is_affected(ch, gsn_fire_breath)
	&&  !is_affected(ch, gsn_dirt)
	&&   can_cast_spell(ch, gsn_cure_blindness) )
		spell = gsn_cure_blindness;
	else
	if ( IS_AFFECTED(ch, AFF_SLOW)
	&&   is_affected(ch, gsn_slow)
	&&   can_cast_spell(ch, gsn_cancellation) )
		spell = gsn_cancellation;
	else
	if ( ES_PSI(ch)
	&&  !IS_AFFECTED(ch, AFF_SANCTUARY)
	&&   can_cast_spell(ch, gsn_biofeedback) )
		spell = gsn_biofeedback;
	else
	if ( can_cast_spell(ch, gsn_sanctuary) )
		spell = gsn_sanctuary;
	else
	if ( ch->hit + 100 < ch->max_hit
	&&   can_cast_spell(ch, gsn_heal) )
		spell = gsn_heal;
	else
	if ( ch->mana > ch->max_mana / 2 )
	{
		obj = NULL;
		switch(number_range(1,24))
		{
			case 1:		spell = gsn_armor;		break;
			case 2:		spell = gsn_bless;		break;
			case 3:		spell = gsn_shield;		break;
			case 4:		if ( !IS_SET(ch->off_flags, OFF_FAST) )
						spell = gsn_haste;
					break;
			case 5:		if ( !is_affected( ch, gsn_enhanced_strength ) )
						spell = gsn_giant_strength;
					break;
			case 6:		if ( !IS_AFFECTED(ch, AFF_DETECT_HIDDEN) )
						spell = gsn_detect_hidden;
					break;
			case 7:		if ( (obj = get_eq_char(ch, WEAR_WIELD)) != NULL
					&&    obj->item_type == ITEM_WEAPON
					&&   !IS_WEAPON_STAT(obj, WEAPON_POISON)
					&&   !IS_WEAPON_STAT(obj, WEAPON_FLAMING)
					&&   !IS_WEAPON_STAT(obj, WEAPON_FROST)
					&&   !IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
					&&   !IS_WEAPON_STAT(obj, WEAPON_SHARP)
					&&   !IS_WEAPON_STAT(obj, WEAPON_VORPAL)
					&&   !IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
					&&   !IS_OBJ_STAT(obj, ITEM_BLESS)
					&&   !IS_OBJ_STAT(obj, ITEM_BURN_PROOF) )
						spell = gsn_poison;
					break;
			case 8:		if ( !is_affected( ch, gsn_flesh_armor ) )
						spell = gsn_stone_skin;
					break;
			case 9:		if ( !IS_AFFECTED2(ch, AFF_FLAMING_SHIELD) )
						spell = gsn_flaming_shield;
					break;
			case 10:	spell = gsn_concentracion;	break;
			case 11:	spell = gsn_enhance_health;	break;
			case 12 :	spell = gsn_adrenaline_control;	break;
			case 13 : 	if ( (obj = get_eq_char(ch, WEAR_WIELD)) != NULL
					&&   !IS_OBJ_STAT(obj, ITEM_BLESS) )
						spell = gsn_bless;
					break;
			case 14 : 	if ( !is_affected(ch, gsn_fireproof) && !ES_IMMUNE(ch, IMM_FIRE) )
						spell = gsn_fireproof;
					break;
			case 15 :	spell = gsn_displacement;		break;
			case 16 :	spell = gsn_energy_containment;		break;
			case 17 :	if ( !is_affected( ch, gsn_giant_strength ) )
						spell = gsn_enhanced_strength;
					break;
			case 18 :	if ( !is_affected( ch, gsn_stone_skin ) )
						spell = gsn_flesh_armor;
					break;
			case 19 :	spell = gsn_mental_barrier;	break;
			case 20 :	spell = gsn_thought_shield;	break;
			case 21 :	spell = gsn_concentracion;	break;
			case 22 :	spell = gsn_combat_mind;	break;
			case 23 :	if ( !IS_AFFECTED(ch, AFF_PROTECT_EVIL)
					&&   !IS_AFFECTED(ch, AFF_PROTECT_GOOD) )
						spell = gsn_inertial_barrier;
					break;
			case 24 :	spell = gsn_intellect_fortress;	break;
			case 25 :	if ( IS_EVIL(ch) )
						spell = gsn_protect_good;
					else
						spell = gsn_protect_evil;
					break;
		}
	}

	if ( spell > 0 && can_cast_spell(ch, spell) )
	{
		char buf[MIL];

		if ( obj && can_see_obj(ch, obj) )
		{
			unequip_char(ch, obj);
			sprintf( buf, "'%s' %s", skill_table[spell].name, obj->name );
			do_cast(ch, buf);
			equip_char(ch, obj, WEAR_WIELD);
		}
		else
		{
			sprintf( buf, "'%s'", skill_table[spell].name );
			do_cast( ch, buf );
		}

		return TRUE;
	}

	return FALSE;
}

void mob_cast( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;

	if ( !ch->fighting )
	{
		if ( ch->position != POS_STANDING )
			do_wake(ch, "");
		else
			selfspell(ch);
	}

	if ( ch->mana > ch->max_mana / 4
	&&   ch->in_room->area->nplayer > 0 )
		char_event_add( ch, UMAX(ch->wait, 2*PULSE_PER_SECOND), 0, mob_cast );
}

void try_to_steal( CHAR_DATA *ch )
{
	CHAR_DATA *pc;
	char buf[MIL];

	for ( pc = ch->in_room->people; pc; pc = pc->next_in_room )
		if (  ch != pc
		&&   !IS_NPC(pc)
		&&   !IS_IMMORTAL(pc)
		&&   !IS_BUILDER(pc, pc->in_room->area)
		&&   !is_friend( ch, pc )
		&&    CHANCE(getNivelPr(ch) + (getNivelPr(pc) - getNivelPr(ch))*10)
		&&    can_see( ch, pc )
		&&   !is_safe( ch, pc )
		&&    pc->position != POS_FIGHTING
		&&    ENTRE_I(getNivelPr(pc)-7,getNivelPr(ch),getNivelPr(pc)+7)
		&&  (!is_clan(ch) || (is_clan(pc) && !is_same_clan(ch, pc))) )
		{
			if (CHANCE(50))
				sprintf( buf, "coins %s", pc->name );
			else
			{
				OBJ_DATA * obj;

				for ( obj = pc->carrying; obj; obj = obj->next_content )
					if ( obj->wear_loc == WEAR_NONE
					&&   can_drop_obj( pc, obj )
					&&  !IS_SET( obj->extra_flags, ITEM_INVENTORY )
					&&   obj->level <= getNivelPr(ch) )
					sprintf( buf, "'%s' %s", obj->name, pc->name );
			}
			do_steal( ch, buf );
			return;
		}
}

void smart_event( EVENT *ev )
{
	CHAR_DATA *ch = ev->item.ch;

	if ( is_hunting(ch) )
		return;

	if ( ch->in_room->area->nplayer > 0 )
	{
		if ( ch->pIndexData->default_pos != POS_SLEEPING
		&&   ch->position == POS_SLEEPING
		&& ((IS_MOB_CASTER(ch) && !IS_AFFECTED(ch, AFF_SANCTUARY))
		||   pc_in_room(ch->in_room, ch) ) )
			do_wake( ch, "" );

		if ( ES_THIEF(ch)
		&&   CHANCE(getNivelPr(ch)) )
		{
			try_to_steal( ch );

			if (char_died(ch))
				return;
		}
	}

	if ( ch->position == POS_STANDING )
	{
		if ( IS_AFFECTED2(ch, AFF_HOLD)
		&&  !is_affected(ch, gsn_web) )
			do_untangle(ch, "");

		if ( ( (ch->hit < ch->max_hit * 0.5)
		  ||   (IS_MOB_CASTER(ch) && ch->mana < ch->max_mana*0.75) )
		&& (!IS_MOB_CASTER(ch) || IS_AFFECTED(ch, AFF_SANCTUARY))
		&&   ch->in_room->area->nplayer == 0 )
			do_sleep(ch, "");
	}

	char_event_add( ch, 2*PULSE_PER_SECOND, 0, smart_event );
}

int spell_dam_mage( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->race == RACE_VAMPIRE
	&&   getNivelPr(ch) > 40
	&&  !ES_IMMUNE(victim, DAM_POISON)
	&&   can_cast( ch, gsn_vampiric_bite, CLASS_MAGE )
	&&   CHANCE(50) )
		return gsn_vampiric_bite;
	else
	if ( getNivelPr(ch) > 26
	&&  !IS_PET(ch)
	&&   IS_NPC(victim)
	&&  !ES_IMMUNE(victim, IMM_HOLY)
	&&   can_cast( ch, gsn_power_word_of_kill, CLASS_MAGE ) )
		return gsn_power_word_of_kill;
	else
	if ( getNivelPr(ch) > 39
	&&  !ES_IMMUNE(victim, IMM_FIRE)
	&&  !ES_RES(victim, RES_FIRE)
	&&   can_cast( ch, gsn_fire_breath, CLASS_MAGE )
	&&   ch->mana > ch->max_mana * 2 / 3
	&&   CHANCE(80) )
		return gsn_fire_breath;
	else
	if ( getNivelPr(ch) > 33
	&&  !ES_IMMUNE(victim, IMM_COLD)
	&&  !ES_RES(victim, RES_COLD)
	&&   can_cast( ch, gsn_frost_breath, CLASS_MAGE )
	&&   ch->mana > ch->max_mana * 2 / 3
	&&   CHANCE(70) )
		return gsn_frost_breath;
	else
	if ( getNivelPr(ch) > 30
	&&  !ES_IMMUNE(victim, IMM_ACID)
	&&  !ES_RES(victim, RES_ACID)
	&&   can_cast( ch, gsn_acid_breath, CLASS_MAGE )
	&&   ch->mana > ch->max_mana * 2 / 3
	&&   CHANCE(60) )
		return gsn_acid_breath;
	else
	if ( getNivelPr(ch) > 27
	&&  !ES_IMMUNE(victim, IMM_ACID)
	&&   can_cast( ch, gsn_acid_blast, CLASS_MAGE )
	&&   CHANCE(80) )
		return gsn_acid_blast;
	else
	if ( getNivelPr(ch) > 30
	&&   can_cast( ch, gsn_hellspawn, CLASS_MAGE )
	&&   CHANCE(80) )
		return gsn_hellspawn;
	else
	if ( getNivelPr(ch) > 21
	&&  !ES_IMMUNE(victim, IMM_FIRE)
	&&   can_cast( ch, gsn_fireball, CLASS_MAGE )
	&&   CHANCE(70) )
		return gsn_fireball;
	else
	if ( getNivelPr(ch) > 18
	&&   can_cast( ch, gsn_energy_drain, CLASS_MAGE )
	&&   CHANCE(60) )
		return gsn_energy_drain;
	else
	if ( getNivelPr(ch) > 15
	&&   can_cast( ch, gsn_colour_spray, CLASS_MAGE )
	&&   CHANCE(60) )
		return gsn_colour_spray;
	else
	if ( getNivelPr(ch) > 9
	&&   can_cast( ch, gsn_shocking_grasp, CLASS_MAGE )
	&&   CHANCE(60) )
		return gsn_shocking_grasp;
	else
	if ( getNivelPr(ch) > 6
	&&   can_cast( ch, gsn_burning_hands, CLASS_MAGE )
	&&   CHANCE(60) )
		return gsn_burning_hands;
	else
	if ( getNivelPr(ch) > 3
	&&   can_cast( ch, gsn_chill_touch, CLASS_MAGE )
	&&   CHANCE(60) )
		return gsn_chill_touch;
	else
	if ( can_cast( ch, gsn_magic_missile, CLASS_MAGE )
	&&   (getNivelPr(ch) < 10 || ch->mana < ch->max_mana / 10) )
		return gsn_magic_missile;

	return -1;
}

int spell_aff_mage( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( getNivelPr(ch) > 27
	&&  !IS_AFFECTED2(victim, AFF_MUTE)
	&&   can_cast( ch, gsn_mute, CLASS_MAGE )
	&&  (es_caster(victim) || CHANCE(20))
	&&  !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
		return gsn_mute;
	else
	if ( getNivelPr(ch) > 15
	&&  can_cast( ch, gsn_dispel_magic, CLASS_MAGE )
	&&  (IS_AFFECTED(victim, AFF_SANCTUARY)
	 ||  IS_AFFECTED(victim, AFF_HASTE)
	 || (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
	 || (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch)))
	&& !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
		return gsn_dispel_magic;
	else
	if ( getNivelPr(ch) > 23
	&& !IS_AFFECTED(victim, AFF_SLOW)
	&&  can_cast( ch, gsn_slow, CLASS_MAGE )
	&& !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
		return gsn_slow;
	else
	if ( getNivelPr(ch) > 11
	&&  !IS_AFFECTED(victim, AFF_BLIND)
	&&   can_cast( ch, gsn_blindness, CLASS_MAGE )
	&&  !saves_spell(getNivelPr(ch), victim, DAM_LIGHT) )
		return gsn_blindness;
	else
	if ( getNivelPr(ch) > 10
	&&  !IS_AFFECTED(victim, AFF_WEAKEN)
	&&   can_cast( ch, gsn_weaken, CLASS_MAGE )
	&&  !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
		return gsn_weaken;
	else
	if ( getNivelPr(ch) > 17
	&&  !IS_AFFECTED(victim, AFF_CURSE)
	&&   can_cast( ch, gsn_curse, CLASS_MAGE )
	&&  !saves_spell(getNivelPr(ch), victim, DAM_NEGATIVE) )
		return gsn_curse;
	else
	if ( !IS_AFFECTED2(victim, AFF_HOLD)
	&&    can_cast( ch, gsn_web, CLASS_MAGE )
	&&   !saves_spell(getNivelPr(ch), victim, DAM_OTHER) )
		return gsn_web;
	else
	if ( can_cast( ch, gsn_magic_missile, CLASS_MAGE )
	&&  (getNivelPr(ch) < 10 || ch->mana < ch->max_mana / 10) )
		return gsn_magic_missile;
	else
		return -1;
}
