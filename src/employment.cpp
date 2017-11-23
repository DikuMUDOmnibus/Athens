/**************************************************************************/
/* employment.cpp - its all about having jobs!!!                          */
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
 ***************************************************************************
 *This file written by Zeus of Athens Mud at athens.boue.ca:9000 make any  *
 *changes you need or want but remember to keep this intact!               *
 **************************************************************************/

#include "include.h" // dawn standard includes
#include "olc.h"
#include "nanny.h"
#include "msp.h"
#include "macros.h"

void do_appoint( char_data *ch, char *argument)
{
	int count = 0;
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if(IS_NPC(ch))
	{do_huh(ch,"");return;}

	if(!IS_LT_GUARD(ch) && !IS_IMMORTAL(ch))
	{do_huh(ch,"");return;}
	
	if(IS_NULLSTR(arg1))
	{ch->printlnf("APPOINT (person) (position)");
	ch->printlnf("Note: for valid options leave the POSITION field blank.");
	return;}

		if(IS_LT_GUARD(ch) && IS_NULLSTR(arg2))
		{ch->printlnf ("Valid Options: city_guard");return;}

		if(IS_IMMORTAL(ch) && IS_NULLSTR(arg2))
		{
			ch->titlebar("");
			ch->printlnf("Valid options:");
			                for(count=0; !IS_NULLSTR(job_flags[count].name); count++){
                        ch->printf("  %s  ", job_flags[count].name);}
                
			ch->print_blank_lines(1);
                	ch->titlebar("");
                	return;
		}	

	if((victim = get_whovis_player_world(ch, arg1)) == NULL)
	{ch->println( "That person is not here to appoint." );return;}
      	 
    	if(IS_NPC(victim))
	{ch->println("Not on NPC's.");return;}

	if(!str_cmp(arg2, "city_guard"))
	{
		if(!IS_LT_GUARD(ch) && !IS_IMMORTAL(ch))
		{do_huh(ch,"");return;}

		if(IS_CITY_GUARD(victim))
		{ch->printlnf("%s is already a City Guard", victim->name);
		 ch->printlnf("Use the DISMISS command to remove them");return;}
		
		if(IS_JOB(victim))
                {ch->printlnf("%s already is already employed.", victim->name);
                victim->printlnf("%s has tried to appoint you to %s but you are already employed.", ch->name, arg2);
                return;}

		else
	
		{SET_BIT(victim->job,JOB_CITY_GUARD);
		SET_BIT(victim->job,JOB_JOB);
	 	ch->printlnf("%s is now a City Guard", victim->name);
	 	victim->printlnf("By the will of %s you are now a City Guard", ch->name);
		return;}
	}

	if(!str_cmp(arg2, "lt_guard"))
        {
               	if(!IS_IMMORTAL(ch))
               	{do_huh(ch,"");return;}

           	if(IS_LT_GUARD(victim))
               	{ch->printlnf("%s is already a Lieutenant Guard", victim->name);
               	ch->printlnf("Use the DISMISS command to remove them");return;}

         	if(IS_JOB(victim))
                {ch->printlnf("%s already has a job.", victim->name);
                victim->printlnf("%s has tried to appoint you to %s but you are already employed.", ch->name, arg2);
                return;}

	      	else
                
		{SET_BIT(victim->job,JOB_LT_GUARD);
		SET_BIT(victim->job,JOB_JOB);
               	ch->printlnf("%s is now a Lieutenant Guard", victim->name);
               	victim->printlnf("By the will of %s you are now a Lieutenant Guard", ch->name);
               	return;}
	}

	if(!str_cmp(arg2, "cartographer"))
	
	{
               	if(!IS_IMMORTAL(ch))
               	{do_huh(ch,"");return;}

		if(IS_SET(victim->job,JOB_CARTOGRAPHER))
		{ch->printlnf("%s is already a Cartographer", victim->name);
		 ch->printlnf("Use the DISMISS command to remove them");return;}
		
		if(IS_JOB(victim))
                {ch->printlnf("%s already is already employed.", victim->name);
                victim->printlnf("%s has tried to appoint you to %s but you are already employed.", ch->name, arg2);
                return;}

		else
		
		{SET_BIT(victim->job,JOB_JOB);
		SET_BIT(victim->job,JOB_CARTOGRAPHER);
		ch->printlnf("You have appointed %s to the position of Cartographer", victim->name);
		victim->printlnf("By the will of %s you are now a Cartographer", ch->name);
		return;}
	}
}

void do_dismiss( char_data *ch, char *argument)
{
	int count = 0;
        char arg1[MIL];
        char arg2[MIL];
        char_data *victim;
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        if(IS_NPC(ch))
        {do_huh(ch,"");return;}

        if(!IS_LT_GUARD(ch) && !IS_IMMORTAL(ch))
	{do_huh(ch,"");return;}

        if(IS_NULLSTR(arg1))
        {ch->printlnf("DISMISS (person) (position)");
	ch->printlnf("NOTE: for a valid list leave the POSITION field blank.");
	return;}

		if(IS_LT_GUARD(ch) && IS_NULLSTR(arg2))
		{ch->printlnf("Valid Options: city_guard");return;}
	
		if(IS_IMMORTAL(ch) && IS_NULLSTR(arg2))
                {
                        ch->titlebar("");
                        ch->printlnf("Valid options:");
                                        for(count=0; !IS_NULLSTR(job_flags[count].name); count++){
                        ch->printf("  %s  ", job_flags[count].name);}

                        ch->print_blank_lines(1);
                        ch->titlebar("");
                        return;
                }


        if((victim = get_whovis_player_world(ch, arg1)) == NULL)
        {ch->println( "That person is not here to appoint." );return;}

        if(IS_NPC(victim))
        {ch->println("Not on NPC's.");return;}

        if(!str_cmp(arg2, "city_guard"))
        {
                if(!IS_LT_GUARD(ch) && !IS_IMMORTAL(ch))
                {do_huh(ch,"");return;}

                if(!IS_CITY_GUARD(victim))
                {ch->printlnf("%s is not a City Guard", victim->name);
                 ch->printlnf("Use the APPOINT command to appoint them");return;}

                else

                {REMOVE_BIT(victim->job,JOB_CITY_GUARD);
		REMOVE_BIT(victim->job,JOB_JOB);
                ch->printlnf("%s is no longer a City Guard", victim->name);
                victim->printlnf("By the will of %s you are no longer a City Guard", ch->name);

	       }

	}
	
        if(!str_cmp(arg2, "lt_guard"))
        {
                if(!IS_IMMORTAL(ch))
                {do_huh(ch,"");return;}

                if(!IS_LT_GUARD(victim))
                {ch->printlnf("%s is not a Lieutenant Guard", victim->name);
                ch->printlnf("Use the APPOINT command to appoint them");return;}

                else

                {REMOVE_BIT(victim->job,JOB_LT_GUARD);
		REMOVE_BIT(victim->job,JOB_JOB);
                ch->printlnf("%s is no longer a Lieutenant Guard", victim->name);
                victim->printlnf("By the will of %s you are no longer a Lieutenant Guard", ch->name);
                return;}
        }

	if(!str_cmp(arg2, "cartographer"))
	
	{
                if(!IS_IMMORTAL(ch))
                {do_huh(ch,"");return;}

                if(!IS_SET(victim->job, JOB_CARTOGRAPHER))
                {ch->printlnf("%s is not a Cartograher", victim->name);
                ch->printlnf("Use the APPOINT command to appoint them");return;}

                else

                {REMOVE_BIT(victim->job,JOB_CARTOGRAPHER);
		REMOVE_BIT(victim->job,JOB_JOB);
                ch->printlnf("%s is no longer a Cartographer", victim->name);
                victim->printlnf("By the will of %s you are no longer a Cartographer", ch->name);
                return;}
	}

}

