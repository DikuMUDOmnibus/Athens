/**************************************************************************/
// score.cpp - The score table
/***************************************************************************
 * The Dawn of Time v1.69r (c)1997-2004 Michael Garratt                    *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Michael Garratt - www.dawnoftime.org     *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, & Katja Nyboe.   *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/
#include "include.h"
#include "ictime.h"
#include "cust_col.h"

/**************************************************************************/
extern char randomColours[16];
int get_sublevels_for_level(int level);
void do_affects(char_data *ch, char * argument);

/**************************************************************************/
#define ch_custom_colour(cc_code) (ch->desc->colour_memory.custom_colour[(cc_code)])
#define ch_template_colour(cc_code) (ch->desc->colour_memory.colour_template->template_colour[(cc_code)])
#define ch_effective_colour(cc_code) (ch_custom_colour(cc_code)=='.'? \ch_template_colour(cc_code):ch_custom_colour(cc_code))

/**************************************************************************/
//By Kalahn - Based on original oblivion score table
void show_score( char_data *ch, char_data *v)
{
	// figure out if remort is shown
	char rbuf[MIL];
	if(!IS_NPC(v) 
		&& GAMESETTING(GAMESET_REMORT_SUPPORTED) 
		&& GAMESETTING(GAMESET_REMORT_IN_SCORE))		
	{
		sprintf(rbuf,"\n  `cRemort: `C%d`x", v->remort);
	}else{
		strcpy(rbuf,"`x");
	}

	// determine what to show for the players name 
	char fullname[MSL];
	if(v->pcdata && !IS_NULLSTR(v->pcdata->surname)){
		sprintf(fullname, "`C%s %s`x", capitalize(v->name), capitalize(v->pcdata->surname));
	}else{
		strcpy(fullname, capitalize(v->name));
	}

	// first section

	char leveltxt[MIL*2];

	ch->printlnf("`C%-36s`x", fullname);
	ch->printlnf("`cLevel:   `C%3d\t`cExperience:`C%9d\t`cRPS:  `C%7ld\t`cGender:     `C%-7s`x", 
		v->level, v->exp, v->pcdata->rp_points, v->sex==0 ? "sexless" : v->sex==1 ? "Male  " : "Female" );
	if(v->pcdata->spouse > 0)
		{ch->printlnf("`cRemort:    `C%d`x\t`cXP For Lvl:     `C%5d\t`cFame: `C%7ld\t`cSpouse:     `C%s`x", 
			v->remort, (v->level+1)*exp_per_level
			(v,v->pcdata->points)-(v->exp), v->pcdata->fame, v->pcdata->spouse );}
	else
		{ch->printlnf("`cRemort:    `C%d`x\t`cXP For Lvl:     `C%5d\t`cFame: `C%7ld\t `cSpouse:     `CNoone`x", 
			v->remort, (v->level+1)*exp_per_level
			(v,v->pcdata->points)-(v->exp), v->pcdata->fame);}
	ch->printlnf("`cPracs: `C%4d\t`cGold Coins:   `C%6ld\t`cClout:`C%7ld\t`cCitizen:    `C%-11s`x", 
			v->practice, v->gold, v->pcdata->clout, race_table[v->race]->name );

	ch->printlnf("`cTrains:  `C%6d\t`cSilver Coins:`C%6ld\t`cDivin: `C%7ld\t`cStatus:     `C%-12s`x", 
			v->train, v->silver, v->pcdata->divinity, class_table[v->clss].name );
	if (ch->clan)
        	{ch->printlnf("`cItems:`C%3d/%4d\t`cArena Wins:     `C%d\t`cMps:   `C%7ld\t`cEnlisted:  `CYes`x", 
			v->carry_number/10, can_carry_n(v)/10, v->win, v->pcdata->military_points);}
        else
		{ch->printlnf("`cItems:`C%3d/%4d\t`cArena Wins:     `C%d\t`cMps:   `C%7ld\t`cEnlisted:  `CNo`x", 
			v->carry_number/10, can_carry_n(v)/10, v->win, v->pcdata->military_points);}
	if(v->deity > 0)
		{ch->printlnf("`cEncumbrance:`C%7d/%7d\t`cArena Losses:   `C%d\t`cWorship: `C%s`x", 
			(int) get_carry_weight(v)/10, (int) can_carry_w(v)/10, v->lost, v->deity->name );}
	else
		{ch->printlnf("`cEncumbrance:`C%7d/%7d\t`cArena Losses:   `C%d`x\t`cWorship:  Noone.`x", 
			(int) get_carry_weight(v)/10, (int) can_carry_w(v)/10, v->lost); 
      ch->printlnf("`cFor more information, see `Cattributes`c.`x");
	if(IS_IMMORTAL(v))
	{		ch->printlnf("`c  %10s%3d   %5s%3d   %5s%3d   %5s%3d  Trust:%3d `x        ",
			"Wizi:",	v->invis_level, 
			"OWizi:",	v->owizi, 
			"IWizi:",	v->iwizi, 
			"Incognito:", v->incog_level, v->trust);
	}

	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS) && IS_IMMORTAL(ch)){
		if(v==ch){
			do_affects(ch,"");
		}else{ // imm version - checking out another player
			do_affects(ch,v->name);
		}
}}	
}

/**************************************************************************/
void do_score( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    one_argument( argument, arg );

	if (IS_IMMORTAL(ch) && !IS_NULLSTR(arg))
  	{
	if ((victim = get_char_world(ch, arg)) == NULL )
	{ch->println("They aren't here.");return;}

	else if(IS_NPC(ch) || IS_NPC(victim))
	{ch->printlnf("No peeking at mobiles scores!!!");return;}

	else if(ch->level <= victim->level)
        {ch->printlnf("%s is of a higher level then you, you can't do that", victim->name);
        victim->printlnf("%s just tried to view your score!", ch->name);return;}
    	}
	else
	{
		victim=ch;
	}
   	
	show_score(ch, victim);
}
/**************************************************************************/

void show_attributes( char_data *ch, char_data *v)
{

	// determine what to show for the players name 
	char fullname[MSL];
	if(v->pcdata && !IS_NULLSTR(v->pcdata->surname)){
		sprintf(fullname, "%s %s", capitalize(v->name), capitalize(v->pcdata->surname));
	}else{
		strcpy(fullname, capitalize(v->name));
	}


	// first section
	ch->printlnf("`x  Attributes for:  `=g%-36s", 
		fullname);

	// attribute and info section
	ch->println("");  

	ch->printlnf("  `xSt:%3d(%3d)/%+3d%sCo:%3d(%3d)/%+3d%s "
				 "`xHit Points: `R%5d`x/`r%5d  `xAlliance: `B%-2d",
		v->perm_stats[STAT_ST], v->potential_stats[STAT_ST], 
			v->modifiers[STAT_ST], abs(v->modifiers[STAT_ST])>99?"":" ",
		v->perm_stats[STAT_CO], v->potential_stats[STAT_CO], 
			v->modifiers[STAT_CO],abs(v->modifiers[STAT_CO])>99?"":" ",
		v->hit,  v->max_hit,  IS_NPC(v) ? 0 : v->alliance);

	ch->printlnf("  `xQu:%3d(%3d)/%+3d%sAg:%3d(%3d)/%+3d%s "
				 "`xMana:       `R%5d`x/`r%5d  `xTendency: `B%-2d",
		v->perm_stats[STAT_QU], v->potential_stats[STAT_QU], 
			v->modifiers[STAT_QU],abs(v->modifiers[STAT_QU])>99?"":" ",
		v->perm_stats[STAT_AG], v->potential_stats[STAT_AG], 
			v->modifiers[STAT_AG],abs(v->modifiers[STAT_AG])>99?"":" ",
		v->mana, v->max_mana,  IS_NPC(v) ? 0 : v->tendency);

	ch->printlnf("  `xPr:%3d(%3d)/%+3d%sSd:%3d(%3d)/%+3d%s "
				 "`xHitroll:  `R%1s %3d `xDamroll: `R%1s %3d       ",
		v->perm_stats[STAT_PR], v->potential_stats[STAT_PR], 
			v->modifiers[STAT_PR],abs(v->modifiers[STAT_PR])>99?"":" ",
		v->perm_stats[STAT_SD], v->potential_stats[STAT_SD], 
			v->modifiers[STAT_SD],abs(v->modifiers[STAT_SD])>99?"":" ",
		(GET_HITROLL(v)<0) ? "-" : (GET_HITROLL(v)==0) ? " " : "+", abs(GET_HITROLL(v)),
		(GET_DAMROLL(v)<0) ? "-" : (GET_DAMROLL(v)==0) ? " " : "+", abs(GET_DAMROLL(v)));


	ch->printlnf("  `xEm:%3d(%3d)/%+3d%sMe:%3d(%3d)/%+3d%s "
		"`xACpierce: `B%1s%3d  `xACbash:  `B%1s%3d        ",
		v->perm_stats[STAT_EM], v->potential_stats[STAT_EM], 
			v->modifiers[STAT_EM],abs(v->modifiers[STAT_EM])>99?"":" ",
		v->perm_stats[STAT_ME], v->potential_stats[STAT_ME], 
			v->modifiers[STAT_ME],abs(v->modifiers[STAT_ME])>99?"":" ",
		(GET_AC(v,AC_PIERCE)/20<0) ? "-": (GET_AC(v,AC_PIERCE)/20)==0 ? " " : "+", 
		abs(GET_AC(v,AC_PIERCE)/20),(GET_AC(v,AC_BASH)/20<0) ? "-": 
		(GET_AC(v,AC_BASH)/20)==0 ? " " : "+", abs(GET_AC(v,AC_BASH)/20));

	ch->printlnf("  `xIn:%3d(%3d)/%+3d%sRe:%3d(%3d)/%+3d%s "
				   "`xACslash:  `B%1s%3d  `xACmagic: `B%1s%3d        ",
		v->perm_stats[STAT_IN], v->potential_stats[STAT_IN], 
			v->modifiers[STAT_IN],abs(v->modifiers[STAT_IN])>99?"":" ",
		v->perm_stats[STAT_RE], v->potential_stats[STAT_RE], 
			v->modifiers[STAT_RE],abs(v->modifiers[STAT_RE])>99?"":" ",
	  (GET_AC(v,AC_SLASH)/20<0) ? "-" : (GET_AC(v,AC_SLASH)/20)==0 ? " " : "+",
	  abs(GET_AC(v,AC_SLASH)/20), (GET_AC(v,AC_EXOTIC)/20<0) ? "-" : 
	  (GET_AC(v,AC_EXOTIC)/20)==0 ? " " : "+",   abs(GET_AC(v,AC_EXOTIC)/20));
      ch->println("");	
}
/**************************************************************************/
void do_attributes( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    one_argument( argument, arg );

    if (IS_IMMORTAL(ch) && !IS_NULLSTR(arg))
  	{
	if ((victim = get_char_world(ch, arg)) == NULL )
	{ch->println("They aren't here.");return;}

	else if(ch->level <= victim->level)
        {ch->printlnf("%s is of a higher level then you, you can't do that", victim->name);
        victim->printlnf("%s just tried to view your attributes!", ch->name);return;}
    	}
	else
	{
		victim=ch;
	}
   	
	show_attributes(ch, victim);
}
/**************************************************************************/
/**************************************************************************/

