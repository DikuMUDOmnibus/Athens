/**************************************************************************/
// act_comm.cpp - primarily code relating to player communications
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

#include "include.h" // dawn standard includes
#include "intro.h"
#include "channels.h"
#include "msp.h"
#include "pload.h"
#include "lockers.h"
#include "shop.h"
#include "olc.h"

// command procedures needed
DECLARE_DO_FUN(do_quit	);
DECLARE_DO_FUN(do_amote	);
DECLARE_DO_FUN(do_pmote	);
DECLARE_DO_FUN(do_smote	);
DECLARE_DO_FUN(do_flee	);
DECLARE_DO_FUN( do_recall); //arena 
DECLARE_DO_FUN(do_advertise);
void saymote( language_data *language, char_data *ch, char *argument, int sayflags);
void laston_player_deleting(char_data * ch);
void quit_char(char_data *ch, char *argument, bool character_deleting );
char_data* find_innkeeper(char_data* ch);
void arena_broadcast(char_data *ch, char * fmt, ...); //arena
/**************************************************************************/
void do_delet( char_data *ch, char *)
{
	 ch->println( "`cYou must type the full command to delete yourself.`x" );
}
/**************************************************************************/
void do_delete( char_data *ch, char *argument)
{
	char strsave[MIL];
#ifndef WIN32
	char buf[MSL];
#endif
	bool remove_pfile = false;
	
	if (IS_NPC(ch))
		return;
	
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}
	
	if (ch->in_room->vnum==ROOM_VNUM_JAIL)
	{
        ch->println( "`cDeleting in this room is not an option, if you really want to be deleted,`x\r\n"
			"`csend a note to admin asking so, and then logoff.`x" );
		return;
	}
	
	if (ch->pcdata->confirm_delete)
	{
		if (argument[0] != '\0')
		{
			ch->println( "`cDelete status removed.`x" );
			ch->pcdata->confirm_delete = false;
			return;
		}
		else
		{
			sprintf( strsave, pfilename( ch->name, get_pfiletype(ch)) );
			wiznet("`c$N deletes $Mself.`x",ch,NULL,0,0,0);
			
			if (ch->level < 5)
				remove_pfile = true;
			
			laston_player_deleting(ch);				// remove character from laston list
			
			intro_player_delete(ch);		// remove them from the intro database

	
			quit_char(ch, "", true);		// let character delete
			
			// on dedicated pkill muds, players just use the delete 
			// command to logout when they have no quit timers...
			// so their pfile is not deleted.
            if(!GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
            {
#ifdef WIN32
				unlink(strsave);	         // delete player file
#else			
                if (remove_pfile)
                {
                    unlink(strsave);         // delete player file
                }
                else // move them to the delete directory
                {        
                    sprintf(buf,"mv %s %s &", strsave, DELETE_DIR);
                    system(buf);
                }
#endif
            }
			return;
		}
	}
	
    if (argument[0] != '\0')
    {
		ch->println( "`cJust type delete. No argument.`x" );
		return;
    }
	
    ch->println( "`cType delete again to confirm this command.`x" );
    ch->println( "`RWARNING: this command is irreversible.`x" );
	ch->println( "`cTyping delete with an argument will undo delete status.`x" );
	ch->pcdata->confirm_delete = true;
	wiznet("`c$N is contemplating deletion.`x",ch,NULL,0,0,get_trust(ch));
}
	    

/**************************************************************************/
// RT code to display channel status
void do_channels( char_data *ch, char *)
{
	char buf[MSL];
	
	/* lists all channels and their status */
	ch->println( "   `cchannel     status`x" );
	ch->println( "`c---------------------`x" );
	ch->printlnf( "`s%-15s%s", "Newbie", HAS_CONFIG(ch, CONFIG_NONEWBIE)?"OFF":"ON");
	ch->printlnf( "`c%-15s%s", "OOC (.)", HAS_CHANNELOFF(ch, CHANNEL_OOC)?"OFF":"ON");
	//ch->printlnf( "`g%-15s%s", "Q/A", HAS_CHANNELOFF(ch, CHANNEL_QA)?"OFF":"ON");
	
	if(IS_NEWBIE_SUPPORT(ch))
	{
	ch->printlnf("`Y%-15s%s", "NSchat", HAS_CHANNELOFF(ch, CHANNEL_NSCHAT)?"OFF":"ON");
	}	

	if (IS_IMMORTAL(ch))
	{
		ch->printlnf( "`G%-15s%s", "Immtalk (:)", HAS_CHANNELOFF(ch, CHANNEL_IMMTALK)?"OFF":"ON");
		ch->printlnf("`Y%-15s%s", "NSchat", HAS_CHANNELOFF(ch, CHANNEL_NSCHAT)?"OFF":"ON");
	}

	if (IS_IBUILDER(ch))
	{
		ch->printlnf("`B%-15s%s", "Btalk (-)", HAS_CHANNELOFF(ch, CHANNEL_BUILDTALK)?"OFF":"ON");
	}

	if (IS_CODER(ch))
	{
		ch->printlnf( "`M%-15s%s", "Codetalk (=)", HAS_CHANNELOFF(ch, CHANNEL_CODETALK)?"OFF":"ON");
	}

	if (IS_JRADMIN(ch))
	{
		ch->printlnf("`y%-15s%s", "Admintalk ([)", HAS_CHANNELOFF(ch, CHANNEL_ADMINTALK)?"OFF":"ON");
	}

	if(IS_CREATOR(ch))
	{
		ch->printlnf( "`c%-15s%s`x", "Hightalk (])", HAS_CHANNELOFF(ch, CHANNEL_HIGHTALK)?"OFF":"ON");
	}

	ch->printlnf( "`g%-15s%s`x", "Quiet mode", HAS_CHANNELOFF(ch, CHANNEL_QUIET)?"ON":"OFF");
	if (IS_SET(ch->comm,COMM_AFK))
		ch->println( "You are AFK." );
	
	if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
		ch->println( "You are immune to snooping." );
	
	if (ch->lines != PAGELEN)
	{
		if (ch->lines){
			ch->printlnf( "You display %d lines of scroll.", ch->lines+2 );
		}else{
			ch->println( "Scroll buffering is off." );
		}
	}
	
	if (!IS_NULLSTR(ch->prompt))
    {
		sprintf(buf,"Your current prompt is: %s",ch->prompt);
		ch->printlnbw(buf);
    }
	
	if (!IS_NULLSTR(ch->olcprompt))
	{
		sprintf(buf,"Your current olc prompt is: %s",ch->olcprompt);
		ch->printlnbw(buf);
	}
	
	if (IS_SET(ch->comm,COMM_NOSHOUT))
		ch->println( "You cannot yell." );
	
    if (IS_SET(ch->comm,COMM_NOTELL))
		ch->println( "You cannot use tell." );
	
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
		ch->println( "You cannot use channels." );
	
	if (IS_SET(ch->comm,COMM_NOEMOTE))
		ch->println( "You cannot show emotions." );
	
}

/**************************************************************************/
// quiet blocks out all communication 
void do_quiet ( char_data *ch, char *)
{
	if (HAS_CHANNELOFF(ch, CHANNEL_QUIET))
	{
		ch->println( "`cQuiet mode removed.`x" );
		REMOVE_CHANNELOFF(ch, CHANNEL_QUIET);
	}
	else
	{
		ch->println( "`cFrom now on, you will only hear says and emotes.`x" );
		SET_CHANNELOFF(ch, CHANNEL_QUIET);
	}
}

// prototype

#if defined(unix)
const   char    echo_off_str    [] = { '\0', '\0', '\0' };
const   char    echo_on_str [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
/*const   char    echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str   [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str  [] = { IAC, GA, '\0' };
*/
#endif

/**************************************************************************/
void do_nspeak( char_data *ch, char *argument)
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

	if(IS_NPC(ch) || ch->pcdata->diplomacy==0 && !IS_IMMORTAL(ch))
	{
		ch->println( "`cYou must be a noble to speak.`x" );
		return;
	}

	if (IS_NULLSTR(argument))
	{
		ch->println( "`cWhat do you wish to proclaim?`x" );
		return;
	}

	ch->printlnf( "`YYou Noble Speak: '%s'`x", argument);
	broadcast(ch,"`Y<noble:> '%s`Y'`x\r\n", argument);
}
	
/**************************************************************************/
// by Kalahn - Sept 98
void do_ntalk( char_data *ch, char *argument )  
{
    char_data *nch, *victim;

	// unswitched mobs can't ooc
	if (IS_UNSWITCHED_MOB(ch))
	{
		ch->println( "`cPlayers or switched players only.`x" );
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

	if (IS_NULLSTR(argument) )
	{
		ch->println( "`cWhat do you wish to ntalk?`x" );
		return;
	}

	if (IS_SET(TRUE_CH(ch)->comm,COMM_NOCHANNELS))
	{
		ch->println( "`cThe gods have revoked your channel priviliges.`x" );
		return;
	}

	ch->printlnf( "`=NYou ntalk: `=n%s`x", argument);

	// log all ntalk to a single file
	// Let's see what they say when we're not there :)
	{
		char logbuf[MSL];
		sprintf(logbuf,"%s`x", argument);
		append_datetime_ch_to_file( ch, NTALK_LOGFILE, logbuf);
	}

    for ( nch = player_list; nch; nch = nch->next_player )
    {
		if (TRUE_CH(ch)==nch){
			continue;
		}

		victim=nch;
		// ntalk going thru switch
		if (victim->controlling && victim->controlling->desc)
		{
			victim=victim->controlling;
		}

		if (IS_NOBLE(nch)){
			if (!IS_NOBLE(ch))
			{
				victim->printlnf( "`=N<%s [not a noble] noble talks>: `=n'%s`=n'`x", 
					TRUE_CH(ch)->name, argument);
			}else{
				if (IS_IMMORTAL(nch)){
					victim->printlnf( "`=N<%s noble talks>: `=n'%s`=n'`x",
						TRUE_CH(ch)->name, argument);
				}else{
					victim->printlnf( "`=N<noble talk>: `=n'%s`=n'`x", argument);
				}
			}
		}
	}
}
/**************************************************************************/
void do_afk ( char_data *ch, char *argument)
{
    // unswitched mobs can't be afk
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("players only sorry");
        return;
    }

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

	if(!IS_NULLSTR(argument)){
		// this makes it so you can change your afk message without turning afk off
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}
	
    if (IS_SET(TRUE_CH(ch)->comm,COMM_AFK))	{
		act("`c$n has returned from being AFK.`x",ch,NULL,NULL,TO_ROOM);
		ch->println( "`cAFK mode removed. Type 'replay' to see tells.`x" );
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}else{
		if(IS_NULLSTR(argument)){
			replace_string(TRUE_CH(ch)->pcdata->afk_message, "");
		}else{
			// idiot checks
			if(c_str_len(argument)==-1){
				ch->println("`cYou can't have newline colour codes in your afk message`x");
				return;
			}
			if(c_str_len(argument)>50){
				ch->println("`cYour afk message can't be more than 50 visible characters.`x");
				return;
			}

			// set their afk away message
			replace_string(TRUE_CH(ch)->pcdata->afk_message, argument);
		}
		if(IS_NULLSTR(argument)){
			act("`c$n just went AFK.`x",ch,NULL,NULL,TO_ROOM);
			ch->println( "`cYou are now in AFK mode.`x");
		}else{
			act("`c$n just went AFK ($t).`x",ch,argument,NULL,TO_ROOM);
			ch->printlnf( "`cYou are now in AFK mode (%s).`x", argument );
		}
		SET_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}
}

/**************************************************************************/
void do_requestooc (char_data *ch, char *argument)
{
    char_data *victim;

    if (!IS_IMMORTAL(ch) && !IS_OOC(ch))
    {
        ch->println( "`cYou must be in the chat rooms to use requestooc.`x" );
        return; 
    }

    if (argument[0]=='\0')
    {
        ch->println( "`cRequest for whom to go to ooc chat rooms.`x" );
        return; 
    }

    if ((victim=get_whovis_player_world(ch, argument))==NULL)
    {
        ch->println( "`cThey are not playing.`x" );
        return;
    }

    ch->printlnf( "`=OSending ooc chat request to %s.`x", victim->name);
    victim->wraplnf("`=O%s has requested that you go to the ooc chat "
        "rooms to talk ooc.`x", ch->name);
    victim->println("`=OYou can get there by typing `=Cgoooc`x." );
}

/**************************************************************************/
void do_pray( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println( "Pray What?" );
		return;
	}

		ch->printlnf( "`MYou pray to any who would listen: '%s'`x",	argument);
	// put it on the prayers wiznet channel
	if (!IS_SET(ch->comm, COMM_NOPRAY)){
		sprintf(buf,"`C%s `cprays '%s' `R[`wroom %d`R](`w%s`R)`x\r\n", 
			ch->name, argument, ch->in_room?ch->in_room->vnum:0,
			position_table[ch->position].name);
		wiznet(buf,NULL,NULL,WIZ_PRAYERS_DREAMS,0,AVATAR);  
	};
	
	for(char_data *c=player_list; c; c = c->next_player){
		if( c != ch && ch->in_room==get_room_index(c->temple))
		{
			if(!IS_NPC(ch)){
				ch->pcdata->did_ic=true;
			}
			act_new("`M$n prays '$t'`x",ch, argument,c,TO_VICT,POS_DEAD);
		}
	}
}
/**************************************************************************/
void do_sayto( char_data *ch, char *argument )
{
    char arg[MSL], buf[MSL];
	char_data *victim;

    argument = one_argument(argument,arg);
    
    if (IS_NULLSTR(arg))
    {
        ch->println( "`cSayto whom what?`x" );
		ch->println( "`cnote: you can actually use normal say and put a > symbol `x\r\n"
			"`cfollowed by a players name as an abbreviation of say to...`x\r\n"
			"`ceg `=C '>kal hello `x or `=C say >kalahn hello`x" );
        return;
    }

	victim = get_char_room( ch, arg);
	if (!victim)
	{
		ch->printlnf( "`xYou can't find '%s' to 'sayto'`x", arg );
		return;
	}

	if (victim==ch)
	{
		ch->println( "`cYou can't direct sayto at yourself.`x" );
		return;
	}

	sprintf(buf,">%s %s", arg, argument);
    saymote( ch->language, ch, buf, 0);
}
/**************************************************************************/
void do_say( char_data *ch, char *argument )
{
	int sayflags=0;
	if(HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)){
		SET_BIT(sayflags, SAYFLAG_NO_SAYMOTE);
	}
	if(HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)){
		SET_BIT(sayflags, SAYFLAG_CONVERT_COLOUR);
	}

	saymote( ch->language, ch, argument, sayflags);	
}
/**************************************************************************/
void do_saymote( char_data *ch, char *argument )
{
    saymote( ch->language, ch, argument, 0);
}
/**************************************************************************/
void do_rsay( char_data *ch, char *argument )
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

	if(!IS_NPC(ch)){
		if(ch->pcdata->security<1){
			ch->println("`crsay is for mobprogs only, it is a say seen only to the person who is remembered.`x");
		}else{
			ch->println("`crsay is for use in mobprogs only, it is short for 'remembered say'.`1`x"
				"`cA mob must first 'mob remember $n', then 'rsay whatever text' will only be heard by $q.`x");
			return;
		}
		return;
	}
    saymote( ch->language, ch, argument, A);
}

/**************************************************************************/
// Kal July99
void do_saycolour(char_data *ch, char *argument)
{
	char arg[MSL];
	char newcol;
    argument = first_arg(argument,arg, false); // first_arg keeps case
    
    if (IS_NULLSTR(arg))  
	{
        ch->println( "`cUse saycolour to set your default colour when talking.`x");
        ch->println( "`csyntax: saycolour <single color code character>`x");
		ch->println( "`ce.g. `Rsaycol G`c would make all the words you spoke to be `Ggreen`x");
		return;
    }

	if(str_len(arg)>1)
	{
		ch->println( "`csaycolour: You can't have more than a single character for a colour code!`x");
		return;
	}

	newcol=arg[0];
	{// check it is a valid code
		char buf[MSL];
		sprintf(buf, "`%c", newcol);
		if (c_str_len(buf)!=0){
			ch->println( "`csaycolour: You can't have a colour code that is a control code!`x");
			return;
		}else{
			ch->printlnf("`csay colour code set to '%c'`x", newcol);
			ch->saycolour=newcol;
		}
	}
}
/**************************************************************************/
// Kal July99
void do_motecolour(char_data *ch, char *argument)
{
	char arg[MSL];
	char newcol;
    argument = first_arg(argument,arg, false); // first_arg keeps case
    
    if (IS_NULLSTR(arg))  
	{
        ch->println( "`xUse motecolour to set your default colour of motes in saymotes." );
        ch->println( "syntax: motecolour <single color code character>" );
		ch->println( "e.g. `=Cmotecol G`x would make all the motes in your saymotes to be `Ggreen`x" );
		return;
    }

	if(str_len(arg)>1)
	{
		ch->println( "motecolour: You can't have more than a single character for a colour code!");
		return;
	}

	newcol=arg[0];
	{// check it is a valid code
		char buf[MSL];
		sprintf(buf, "`%c", newcol);
		if (c_str_len(buf)!=0){
			ch->println( "motecolour: You can't have a colour code that is a control code!");
			return;
		}else{
			ch->printlnf( "mote colour code set to '%c'", newcol);
			ch->motecolour=newcol;
		}
	}
}
/**************************************************************************/
// Kalahn August 97 
void do_whisper( char_data *ch, char *argument )
{
    char_data *victim, *overhear;
    char target[MIL];
	char message[MSL];

	// dont allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

    // wiznet whispers NO MOBS
	if( !IS_UNSWITCHED_MOB(ch )) {
		wiznet(FORMATF("`cwhisper %s: \"%s\"`x", ch->name, argument), 
			ch ,NULL,WIZ_WHISPERS,0,get_trust(ch));
	}

    argument = one_argument( argument, target);

    if( IS_NULLSTR(target)){
        ch->println( "`cWhisper to whom?`x" );
        return;
    }

    if(!str_cmp(target,"all")){
        ch->println( "`cIf you are going to tell everyone just use say.`x" );
        return;
    }

    // find the person in the room to whisper them 
    victim = get_char_room( ch, target);
    if(!victim){
        ch->printlnf( "`cYou can't find '%s' to whisper to`x", target);
        return;
    }

    if (victim == ch){
        ch->println( "`cIf you start whispering to yourself, people will think you're strange.`x" );
        return;
    }

    if(!IS_AWAKE(victim)){
        ch->printlnf( "`cYou had better wake '%s' first.`x", PERS(victim, ch));
        return;		
    }

    argument = ltrim_string(argument);
    if(IS_NULLSTR(argument)){
        ch->printlnf( "`cWhat do you want to whisper to %s?`x", PERS(victim, ch));
        return;
    }

    // record the whisper as ic or ooc, or maybe both 
	if(!IS_NPC(ch)){
		ch->pcdata->did_ic=true;
        if(room_is_private(ch->in_room)){
            ch->pcdata->did_ooc=true;
		}
	}

    // go thru all in the room - whispering
    // to target, and those who over hear 
    for ( overhear=ch->in_room->people; overhear; overhear = overhear->next_in_room )
    {
        if ( overhear == ch || !IS_AWAKE(overhear)){
			continue;
		}

        translate_language(ch->language, true, ch, overhear, argument, message);

        // direct it to whoever is necessary 
        if(overhear == victim){
			RECORD_TO_REPLAYROOM=true;
			if ( !is_affected( victim, gsn_deafness ) || IS_OOC(victim)) {
	            act("`c$n whispers to you '$t`x'", ch, message, victim, TO_VICT);
			}
            act("`cYou whisper '$t`x' to $N`x.", ch, argument, victim,TO_CHAR);
			RECORD_TO_REPLAYROOM=false;
			continue;
        }


        if (IS_AWAKE(overhear) && IS_TRUSTED(overhear, INVIS_LEVEL(ch)))
        {
			// the higher the chance value, the more likely 'overhear' will hear
            int chance = 45 - ch->perm_stats[STAT_IN]/3; // approx range 45 -> 75 
            chance -= ch->modifiers[STAT_IN]/3;      // can take it up to > 100 
            chance += overhear->modifiers[STAT_QU]/4;
            chance += overhear->modifiers[STAT_IN]/3;

			// awareness helps you overhear
			chance += get_skill(overhear,gsn_awareness)/5;
            
			chance-= number_range(1,100);

            if (( chance > 0
			|| IS_IMMORTAL(overhear)
			|| is_affected(overhear, gsn_augment_hearing ))) // overheard 
            {
				if ( IS_OOC(overhear) || !is_affected( overhear, gsn_deafness) ) {
		            overhear->printlnf("`cYou overhear %s whispering '%s' to %s`x.",
						PERS(ch, overhear), message, PERS(victim, overhear));
					overhear->record_replayroom_event(
							FORMATF("`cYou overhear %s whispering '%s' to %s`x.",
								PERS(ch, overhear), message, PERS(victim, overhear))
						);
				}
            } else if (chance+45 > 0){ // notice but not hear 
				overhear->printlnf("`cYou notice %s whispering to %s`x.",
					PERS(ch, overhear), PERS(victim, overhear));
				overhear->record_replayroom_event(
						FORMATF("`cYou notice %s whispering to %s`x.",
							PERS(ch, overhear), PERS(victim, overhear))
					);
			}
        }
	}

    return;
}

/**************************************************************************/
// return true if for any reason if the sender can't send a tell
// - this function sends a message to the sender stating the reason
bool tell_cant_send_tell(char_data *ch)
{
    // unswitched mobs can't send tells
    if (IS_UNSWITCHED_MOB(ch))
    {
		ch->println( "`cUncontrolled mobs can't tell.`x" );
		return true;
    }

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master ){
			ch->master->println( "`cNot going to happen.`x" );
		}
		ch->println( "`cYou can't be ordered to send tells.`x" );
		return true;
	}

    if(IS_SET(TRUE_CH(ch)->comm, COMM_NOTELL) ){
		ch->println( "`cThe gods have removed your power to send tells.`x" );
		return true;
	}

	return false;
}
/**************************************************************************/
void deliver_tell(char_data *ch, char * fmt, ...)
{
	assert(!IS_NPC(ch));
	char_data *target=ch->controlling?ch->controlling:ch;

    char buf[MSL];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, MSL, fmt, args);
	va_end (args);

	char recordbuf[MSL];
	sprintf(recordbuf, "%s> %s",
		shorttime(NULL),
		buf);

	replace_string(	// record it in their replay buffer
		ch->pcdata->replaytell_text[ch->pcdata->next_replaytell], 
		recordbuf);
	++ch->pcdata->next_replaytell%=MAX_REPLAYTELL;
	if(HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS)){
		target->wrapln(buf); // send to the character
	}else{
		target->print(buf); // send to the character
	}
}
/**************************************************************************/
enum sendtext_type {ST_TELL, ST_RETELL, ST_REPLY};
/**************************************************************************/
// - Kalahn, Jan 00
void tell_sendtext(char_data *from, char_data *to, char *text, sendtext_type type)
{
	assertp(from);
	assertp(to);
	// get the characters they are connected to 
	// - so messages can be transperant to switch etc
	char_data *sender=from->controlling?from->controlling:from;

	if(IS_NPC(from) || IS_NPC(to)){ // TRUE_CH() must be used in the calling function
		sender->println("Tells can't be exchanged with non playing characters.");
		return;
	} 

	if(channel_colour_disabled(to,CHANNEL_TELLS)){
		text=strip_colour(text);
	}
		
	// support restricting tells between players
	if(GAMESETTING2(GAMESET2_TELL_RESTRICTIONS))
	{
		if( !(IS_IMMORTAL(from) || IS_IMMORTAL(to)
			||(IS_NEWBIE_SUPPORT(from) && !IS_LETGAINED(to)) 
			||(IS_NEWBIE_SUPPORT(to) && !IS_LETGAINED(from))))
		{
			if (IS_NEWBIE_SUPPORT(from)){
				sender->println( "Tells may only be exchanged with an immortal or new players.");
			}else if (!IS_LETGAINED(from)){
				sender->println( "`xTells may only be exchanged with an immortal or newbie support players\r\n"
					"(ie has +++ in who).\r\n"
					"If you want to talk to another player about Out of Character (OOC) way\r\n"
					"you can use goto the ooc rooms using the `=Cgoooc`x command, and then request\r\n"
					"using the `=Crequestooc`x command that the person you are wanting to talk meets\r\n"
					"meets you in the ooc rooms.");
			}else{
				sender->println( "Tells may only be exchanged with an immortal." );
			}
			return;
		}
	}

	// get the name of the person the message is being sent to from 
	// the senders perspective
	char to_name[MIL];
	char to_capname[MIL];
	int to_gender;
	bool to_unknown;
	if(type==ST_REPLY && IS_SET(from->dyn,DYN_UNKNOWN_REPLY_NAME)){
		to_unknown=true;
		strcpy(to_name,"A god"); // the default name
		to_gender=SEX_NEUTRAL;
	}else{ 
		to_unknown=false;
		strcpy(to_name, mxp_create_tag(from, FORMATF("ch-uid_name %d", to->uid), capitalize(to->name)));
		to_gender=URANGE(0, to->pcdata->true_sex, 2);
	}
	strcpy(to_capname,capitalize(to_name));

	// leave imms in peace and quiet from mortal tells :)
    if( HAS_CHANNELOFF(to, CHANNEL_QUIET) && !IS_IMMORTAL(from)) {
        sender->printlnf( "`c%s is not receiving tells.`x", to_name);
        return;
    }

	// check for nothing to say - including empty colour codes
    if ( IS_NULLSTR(text) || c_str_len(text)==0) {
        sender->printlnf( "`c%s %s what?`x",
			(type==ST_TELL)?"Tell":	((type==ST_RETELL)?"Retell":"Reply to"), to_name);
		return;
	}

	// If the sender knows the name of the person they are talking to or they can't 
	// see the person they are talking to on the wholist, they can be informed
	// of the linkdead status of the person they are speaking with
    if ( !(to->controlling?to->controlling:to)->desc 
		  && (!to_unknown || !can_see_who(from,to)))
    {
		sender->wraplnf("`x%s seems to have misplaced %s link... `x"
			"`cthe tell will be recorded in %s replay which they will`x "
			"`cautomatically see if they reconnect.`x",
			to_capname, his_her[to_gender], his_her[to_gender]);
    }

    // keep a track of what newbie support is doing
    if ( (IS_NEWBIE_SUPPORT(from) && !IS_LETGAINED(to))
        ||(IS_NEWBIE_SUPPORT(to) && !IS_LETGAINED(from)) )
    {
        char tbuf[MSL];
		char nbuf[MSL];
        sprintf (tbuf,"Tell %s %s", to->name, text);
        append_newbie_support_log(from, tbuf);
		if ( !IS_IMMORTAL( from ) && !IS_IMMORTAL( to )) {
			sprintf (nbuf, "`W%s newbietells %s '%s'", 
				from->name, to->name, text);
			wiznet(nbuf,from,NULL,WIZ_NEWBIETELL,0,LEVEL_IMMORTAL);
		}
	}

	// get the name of the person sending the message 
	// from the receivers perspective
	char from_name[MIL];
	bool from_unknown=true;;
	int from_gender;	
	switch(type){
	case ST_TELL:	// 'new conversation' sender known if can be seen on the wholist
		if(can_see_who(to, from) || IS_SET(from->dyn, DYN_USING_KTELL) ){
			from_unknown=false;
			REMOVE_BIT(from->dyn, DYN_HAS_ANONYMOUS_RETELL);
		}else{
			SET_BIT(from->dyn, DYN_HAS_ANONYMOUS_RETELL);
		}
		break;

	case ST_REPLY:	// For the sender to be replying, the receiver has to have 
		// sent the sender a tell earlier, therefore the receiver could see them 
		// then, so this conversation isn't anonymous
		from_unknown=false;
		break;

	case ST_RETELL: // determined on the anonymous status of original tell
		if(!IS_SET(from->dyn, DYN_HAS_ANONYMOUS_RETELL)){
			from_unknown=false;
		}
		break;
	}
	if(from_unknown){
		strcpy(from_name,"A god"); // name unknown to the receiver
		from_gender=SEX_NEUTRAL;
	}else{	
		strcpy(from_name, mxp_create_tag(to, FORMATF("ch-uid_name %d", from->uid), capitalize(from->name)));
		from_gender=URANGE(0, from->pcdata->true_sex, 2);
	}

	// send the messages
	deliver_tell( from, "%sYou %s%s %s '%s%s'`x\r\n", 
		(type==ST_REPLY?"`=M":"`=m"),
		(from_unknown?"anonymously ":""),
		(to_unknown && type==ST_REPLY)|| !can_see_who(from, to)? mxp_create_tag(from, "tl_rp", "tell"):
			mxp_create_tag(from, (FORMATF("%s %s", from->retell==to?"tl-nm_rp_tlnm":"tl-nm_rt_tlnm", to->name)), "tell"),
		to_name,
		text,
		(type==ST_REPLY?"`=M":"`=m"));

	deliver_tell(to, "%s%s %s you %s'%s%s'`x\r\n", // tells you
		(to->retell!=from || from_unknown?"`=M":"`=m"),
		from_name,
		(from_unknown || !can_see_who(to, from))?mxp_create_tag(to, "tl_rp", "tells"):
			mxp_create_tag(to, (FORMATF("%s %s", to->retell==from?"tl-nm_rt_tlnm":"tl-nm_rp_tlnm", from->name)), "tells"),
		(to_unknown?"(someone) ":""),
		text,
		(to->retell!=from?"`=M":"`=m"));

	// reply locks onto all anonymous tells 
	// if the tell isn't anonymous then to those who you arent retelling you
	// also when the receiver doesn't have any current reply
	if(from_unknown || to->retell!=from || !to->reply){
		if(!to_unknown){
			to->reply=from;
		}
		if(from_unknown){
			SET_BIT(to->dyn,DYN_UNKNOWN_REPLY_NAME);
		}else{
			REMOVE_BIT(to->dyn,DYN_UNKNOWN_REPLY_NAME);
		}
	}

	if(type==ST_TELL){
		from->retell=to;
	}

	// inform the sender of receiver afk status on the message they just sent
	// (assuming it wont give away the identity of an imm that is a 'someone')
	if (IS_SET(to->comm,COMM_AFK) && (!to_unknown || !can_see_who(from,to)))
	{  
		from->wraplnf( 
			"`c%s is currently AFK and therefore might not have seen your message.  Your`x "
			"`cmessage has been displayed to their screen and recorded in %s tell replay buffer.`x", 
			to_capname,
			his_her[to_gender]);
    }
}
/**************************************************************************/
void do_tell( char_data *ch, char *argument )
{
    char arg[MIL];
	char_data *victim;
	
	if(tell_cant_send_tell(ch)){ 
		return;
	};

    if( HAS_CHANNELOFF(ch, CHANNEL_QUIET) ) {
        ch->println( "`cYou must turn off quiet mode before sending a tell.`x" );
        return;
    }

	// find out who the tell is going to
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->println( "`cTell whom what?`x" );
        return;
    }

    // remove a , from the name field if required
	if (arg[str_len(arg)-1]==',')
        arg[str_len(arg)-1]=0;

    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL) 
    {
        ch->printlnf( "`c'%s' couldn't be found to send a tell to.`x", arg);
        return;
    }

	if(IS_SET(TRUE_CH(victim)->comm, COMM_BUSY))
	{
		ch->printlnf("`c%s is busy, perhaps you should try again later.`x", victim->name);
		return;
	}

	if(TRUE_CH(ch)==victim){
        ch->println( "`cTry talking to someone other than yourself.`x" );
        return;
	}
	
	tell_sendtext(TRUE_CH(ch), victim, argument, ST_TELL);
}
/**************************************************************************/
void do_atell( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println( "`catell <who> what you want to say - anonymous tell`x" );
		return;
	}
	int whovis = IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS);
    REMOVE_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);

	do_tell(ch, argument);

	if (whovis)
	{
		SET_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
	}
}
/**************************************************************************/
void do_ktell( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println( "`cktell <who> what you want to say - known tell`x" );
		return;
	}

	SET_BIT(TRUE_CH(ch)->dyn, DYN_USING_KTELL);
	do_tell(ch, argument);
	REMOVE_BIT(TRUE_CH(ch)->dyn, DYN_USING_KTELL);
}
/**************************************************************************/
void do_retell( char_data *ch, char *argument )
{
	if(tell_cant_send_tell(ch)){ 
		return;
	};

	if (!TRUE_CH(ch)->retell)
    {
		ch->println( "`cThey aren't here.`x" );
        ch->println( "`c(you have to send someone a tell before using retell.)`x" );
        return;
    }
	tell_sendtext(TRUE_CH(ch), TRUE_CH(ch)->retell, argument, ST_RETELL);
}

/**************************************************************************/
void do_reply( char_data *ch, char *argument )
{
	if(tell_cant_send_tell(ch)){ 
		return;
	};
	
    if ( TRUE_CH(ch)->reply == NULL )
    {
        ch->println( "`cThey aren't here.`x" );
        return;
    }

	tell_sendtext(TRUE_CH(ch), TRUE_CH(ch)->reply, argument, ST_REPLY);
}
/**************************************************************************/
struct sectyellinfo
{
	short sectindex;
	float yellreduction; // amount of yell remaining after the sound 
						 // travelled thru the room
};

// return the id of the next yell
time_t get_yell_id(void)
{
	static int lastyellid=0;
	lastyellid++;
    return lastyellid;
}

sectyellinfo sectyelltable[]={
	{SECT_INSIDE		,(float)0.90}, // reduction amounts on the high side
	{SECT_CITY			,(float)0.85}, // to make the yell command actually useable
	{SECT_FIELD			,(float)0.75},
	{SECT_FOREST		,(float)0.70},
	{SECT_HILLS			,(float)0.70}, 
	{SECT_MOUNTAIN		,(float)0.65}, 
	{SECT_WATER_SWIM	,(float)0.55},
	{SECT_WATER_NOSWIM	,(float)0.45},
	{SECT_SWAMP			,(float)0.45},
	{SECT_AIR			,(float)0.65},
	{SECT_DESERT		,(float)0.65},
	{SECT_CAVE			,(float)0.80},
	{SECT_UNDERWATER	,(float)0.35},
	{SECT_SNOW			,(float)0.80},
	{SECT_ICE			,(float)0.80},
	{SECT_TRAIL			,(float)0.75},
	{SECT_LAVA			,(float)0.40},
	{SECT_INSIDE		,(float)0.90},
};
/**************************************************************************/
float get_yellreduction(short sector)
{
	if(sector<0 || sector>=SECT_MAX){
		bugf("get_yellreduction(): Out of range sector value %d!!!", sector);
		return (float)0.50; // use low value
	}

	if(sectyelltable[sector].sectindex==sector){
		return sectyelltable[sector].yellreduction;
	}else{ // gotta go look for it
		bugf("get_yellreduction(): Table out of order for index %d, sectyelltable[sector].sectindex=%d!!", 
			sector, sectyelltable[sector].sectindex);
		return (float)0.65; // use low value
	}
}
/**************************************************************************/
void recurse_yell(ROOM_INDEX_DATA *to_room, float amplitude, 
				  int direction, time_t yellindex)
{
	if(!to_room){
		bug("recurse_yell(): to_room==NULL!");
		return;
	}

	// reduce the amplitude of the yell to travel this room
	amplitude*=get_yellreduction(to_room->sector_type);

	// yell has died out
	if(amplitude<5.0){
		return;
	} 

	// if the room we are currently in already has a yell that is greater 
	// or equal to our current amplitude, leave this path
	if(to_room->yellindex==yellindex && to_room->yell_amplitude>=amplitude){
		return; 
	}

	// add our yell here
	to_room->yellindex=yellindex;
	to_room->yell_amplitude=amplitude;
	
	to_room->yell_enteredindir=direction;

	// broadcast the yell in all directions
	int dir;
    for (dir=0;dir<MAX_DIR;dir++)
	{
		if(!to_room->exit[dir] || !to_room->exit[dir]->u1.to_room)
			continue;
		
		if(IS_SET(to_room->exit[dir]->exit_info,EX_CLOSED)){
			amplitude*=(float)0.55; // lose 45% going thru doors
		}	
		
		if(dir!=direction){
			amplitude*=(float)0.70; // lose 30% changing direction
		}	

		// yells dont bleed between OOC and IC
		if((to_room->exit[dir]->u1.to_room->room_flags&ROOM_OOC) 
			!=(to_room->room_flags&ROOM_OOC)){
			continue;
		}

		if(   !to_room->exit[dir]->u1.to_room->exit[rev_dir[dir]]
			|| to_room->exit[dir]->u1.to_room->exit[rev_dir[dir]]->u1.to_room!=to_room){
			// exits missed matched, wipe directional component
			recurse_yell(to_room->exit[dir]->u1.to_room, amplitude, -1, yellindex);
		}else{
			// send to the next room
			recurse_yell(to_room->exit[dir]->u1.to_room, amplitude, dir, yellindex);
		}
	}
}
/**************************************************************************/
char *get_yellreversedir(short dir, int amplitude)
{
	static char buf[MIL];

	if(dir==-1){
		if(amplitude>50){
			sprintf(buf,"`cfrom somewhere in the near surroundings`x");
		}else if (amplitude>40){
			sprintf(buf,"`cfrom somewhere not far from here`x");
		}else if (amplitude>30){
			sprintf(buf,"`cfrom somewhere in the near distance`x");
		}else if (amplitude>20){
			sprintf(buf,"`cfrom somewhere in the distance`x");
		}else if (amplitude>10){
			sprintf(buf,"`cfrom somewhere in the far distance`x");
		}else if (amplitude>4){
			sprintf(buf,"`cfrom somewhere in the far far distance`x");
		}else{
			sprintf(buf,"`cfaintly`x");
		}
	}else{
		if(amplitude>50){
			sprintf(buf,"`cfrom somewhere in the near surroundings to the %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>40){
			sprintf(buf,"`cfrom somewhere not far from here to the %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>30){
			sprintf(buf,"`cfrom somewhere in the near distant %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>20){
			sprintf(buf,"`cfrom somewhere in the distant %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>10){
			sprintf(buf,"`cfrom somewhere in the far distance %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>5){
			sprintf(buf,"`cfrom somewhere in the far far distant %s`x",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>2){
			sprintf(buf,"`cfaintly from the %s`x", dir_name[rev_dir[dir]]);
		}else{
			sprintf(buf,"`cfaintly`x");
		}
	}
	return buf;
}
/**************************************************************************/
bool ftp_reconnect(char *name);
/**************************************************************************/
void do_testyell(char_data *ch, char *)
{
	ch->println("===Yell test info");
	ch->printlnf( "Room vnum = %d",ch->in_room->vnum);
	ch->printlnf( "Yellindex = %d",(int)ch->in_room->yellindex);
	ch->printlnf( "Yell_enteredindir = %d",ch->in_room->yell_enteredindir);
	ch->printlnf( "Yell_amplitude    = %f",ch->in_room->yell_amplitude);

	if(ch->desc){
		ch->println("===Colour memory test dump");
		ch->printlnf("current = %c", ch->desc->colour_memory.current);
		ch->printlnf("saved index = %d", ch->desc->colour_memory.saved_index);
		for(int i=0; i<MAX_SAVED_COLOUR_ARRAY; i++){
			ch->printlnf("%d] saved =%c", i, ch->desc->colour_memory.saved[i]);
		}
	}

	// manual room count
	{
		int rcount=0;
		int i;
		ROOM_INDEX_DATA *pRoomIndex;

		for(i=0; i<MAX_KEY_HASH; i++){
			for ( pRoomIndex  = room_index_hash[i];
			pRoomIndex != NULL;
			pRoomIndex  = pRoomIndex->next )
			{		  
				rcount++;
			}
		}
		ch->printlnf("manual room count=%d", rcount);
	}

	if(HAS_MSP(ch)){
		ch->println("Testing MSP by sending potion sound using msp_to_room()...");
		msp_to_room(MSPT_ACTION, MSP_SOUND_QUAFF, 
						0,
						ch,
						false,
						true);	
		ch->println("Test finished.");
	}else{
		ch->println("Currently your msp setting is disabled, so no msp test sound is sent to you.");
	}	

	ch->println("Telling your dawnftp client to reconnect (if you have one).");
	ftp_reconnect(ch->name);
}
/**************************************************************************/
// Kal - July 99
void do_yell(char_data *ch, char *argument )
{
	char buf2[MSL];
	static int rpmon=0;

	char buf[MSL];
	time_t yellid= get_yell_id();
	float start_amp=race_table[ch->race]->high_size*(float)1.1;

	if ( IS_SET(ch->comm, COMM_NOSHOUT) )
	{
		ch->println( "`cYou can't yell.`x" );
		return;
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "`cYell what?`x" );
		return;
	}

	if(IS_SET(ch->in_room->room_flags, ROOM_NOSPEAK)
	&& !HAS_CONFIG(ch, CONFIG_HOLYSPEECH)) {
		ch->println( "`cYou feel the air escape your lungs, and yet... no sound.`x" );
		return;
	}
	
    // wiznet RPMONITOR 
    if (!IS_NPC(ch) && !IS_OOC(ch)) 
    {
        rpmon++;
        if (rpmon>=5)
        {
            sprintf (buf2, "Wiznet rpmonitor: %s yells '%s'`x", ch->name, argument);
            wiznet(buf2,ch,NULL,WIZ_RPMONITOR,0,get_trust(ch));
            rpmon=0;
        }
    }
	
    if(!IS_NPC(ch)){
        ch->pcdata->did_ic=true;
	}
				// fromroom ,  amplitude, no direction
	recurse_yell(ch->in_room, start_amp,  -1,		yellid);

	// now transmit to all connections what they heard
	{
		ROOM_INDEX_DATA *this_room;
		connection_data *d;     
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING 
				&& d->character && d->character->in_room
				&& IS_AWAKE(d->character))
			{
				this_room=d->character->in_room;
				if(this_room->yellindex==yellid){
					// they heard it					
					translate_language(ch->language, true, ch, d->character, argument, buf);
										
					if(this_room== ch->in_room)
					{
						act("`c$n yells '$t'`x",ch,buf,d->character,TO_VICT);
					}
					else
					{
						if(IS_IMMORTAL(CH(d)))
						{
							d->character->printlnf( "`cYou hear a %s voice(%s) yell %s '%s', %f`x",
								(ch->sex==2?"female":"male"),
								ch->name,
								get_yellreversedir(this_room->yell_enteredindir,
								(int)this_room->yell_amplitude),
								buf, this_room->yell_amplitude);
						}
						else
						{
							if(IS_OOC(ch))
							{
								d->character->printlnf( "`cYou hear a %s voice(%s) yell %s '%s'`x", 
									(ch->sex==2?"female":"male"), ch->name,
									get_yellreversedir(this_room->yell_enteredindir,
									(int)this_room->yell_amplitude), buf);
							}
							else
							{
								d->character->printlnf( "`cYou hear a %s voice yell %s '%s'`x", 
									(ch->sex==2?"female":"male"),
									get_yellreversedir(this_room->yell_enteredindir,
									(int)this_room->yell_amplitude), buf);
							}
						}
					}
				}
			}
		}
	}
	act("`cYou yell '$t'`x",ch,argument,NULL,TO_CHAR);
}

/**************************************************************************/
void do_emote( char_data *ch, char *argument )
{
	char buf2[MSL];
	static int rpmon=0;
	
	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		ch->println( "`cYou can't show your emotions.`x" );
		return;
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "`cEmote what?`x" );
		return;
	}

	// Integrated emote code 
	{
		int starcount = count_char(argument, '*');
		int atcount = count_char(argument, '@');
		
		if(starcount>0)
		{
			if( atcount>0){
				do_amote(ch,argument);
			}else{
				do_smote(ch,argument);
			}
			return;
		}
		if( atcount>0){
			do_pmote(ch,argument);
			return;
		}
		
	}

	if(!IS_OOC(ch))
	{
		// dont allow messages that could be defrauding 
		if(check_defrauding_argument(ch, argument)){
			return;
		}
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    // wiznet RPMONITOR
    if (!IS_NPC(ch))
	{
		if (!IS_OOC(ch)) 
		{
			rpmon++;
			if (rpmon>=15)
			{
				sprintf (buf2, "`cWiznet rpmonitor: %s emotes '%s`x'`x", ch->name, argument);
				wiznet(buf2,ch,NULL,WIZ_RPMONITOR,0,get_trust(ch));
				rpmon=0;
			}
			if(rps_candiate){
				ch->pcdata->did_ic=true;
			}
		}
		
		if(room_is_private(ch->in_room) && rps_candiate){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
		
		// anti autorps abuse system
		if(IS_IC(ch) && rps_candiate)
		{
			++ch->pcdata->emote_index%=RPS_AUDIT_SIZE;
			free_string(ch->pcdata->emotecheck[ch->pcdata->emote_index]);
			ch->pcdata->emotecheck[ch->pcdata->emote_index]=str_dup(argument);
			ch->pcdata->emotetimes[ch->pcdata->emote_index]=current_time;
		}
	}
	
	MOBtrigger = false;
	RECORD_TO_REPLAYROOM=true;
	act( "`c$n, $T`x`x", ch, NULL, argument, TO_ROOM );
	act( "`c$n, $T`x`x", ch, NULL, argument, TO_CHAR );
	RECORD_TO_REPLAYROOM=false;
	MOBtrigger = true;
	
	return;
}

/**************************************************************************/
// written by Kalahn
void do_amote( char_data *ch, char *argument )
{
    char *letter;
    char act_text[MIL];
    char vict_text[MIL];

    char ch_arg[MIL];
    char_data *victim;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        ch->println( "`cYou can't show your emotions.`x" );
        return;
    }

    argument = one_argument( argument, ch_arg );

    if ( ch_arg[0] == '\0' || argument[0] == '\0')
    {
        ch->println( "`cSyntax: amote <at> <text>.`x" );
        ch->println( "`c<at> is the target who the amote is direct at.`x" );
        ch->println( "`cThe text must include * for your name and a @ where you want the targets name.`x" );
        ch->println( "`ceg amote guard Jumping from out of the shadows, * grabs @ by the neck.`x" );
        return;
    }

    if (( victim = get_char_room( ch, ch_arg ) ) == NULL )
    {
        ch->printlnf( "`cYou can't seem to find the player '%s'.`x", ch_arg);
        return;
    }

    if (victim == ch)
    {
        ch->println( "`cYou can't direct amotes at yourself... use smote.`x");
        return;
    }

    if ( argument[0] == '\0' )
    {
        ch->printlnf( "`cAmote what towards %s.`x", PERS(victim, ch));
        return;
    }

	if ( (count_char(argument,'@') != 1) || (count_char(argument,'*') != 1) )
    {
        ch->println( "`cSyntax: amote <target> <text>.`x" );
        ch->println( "`c The text must include a single * for your name and a single @ where you want the targets name.`x" );
        ch->println( "`c * must appear before @.`x" );
        ch->println( "`c eg amote guard Jumping from out of the shadows, * grabs @ by the neck.`x" );
        return;
    }

    if (strstr(argument,"@") < strstr(argument,"*"))
    {
        ch->println( "`cAMOTE: * must appear before the @.`x" );
        return;
    }

	// dont allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    act_text[0]= '\0';
    vict_text[0]= '\0';

    letter = argument;

    for (; *letter != '\0'; letter++)
    {
        if (*letter == '@')
        {
            if ( *(letter+1) == '\'' && *(letter+2) == 's')
            {
                strcat(act_text, "`#`x$N's`&");
                strcat(vict_text, "your");
                letter += 2;
            }
            else if ( *(letter+1) == 'r' || *(letter+1) == 's') 
            {
                strcat(act_text, "`#`x$N's`&");
                strcat(vict_text, "your");
                letter++;
            }
            else
            { 
                strcat(act_text, "`#`x$N`&");
                strcat(vict_text, "you");
            }
        }
        else if (*letter == '*')
        {
            strcat(act_text, "`#`W$n`&");
            strcat(vict_text, "`#`W$n`&");
        }
        else
        {
            strncat(act_text, letter,1);
            strncat(vict_text, letter,1);
        }
    }


    MOBtrigger = false;
    act( act_text,  ch, NULL, victim, TO_NOTVICT );
    act( act_text,  ch, NULL, victim, TO_CHAR );
    act( vict_text, ch, NULL, victim, TO_VICT );
    MOBtrigger = true;

	if(!IS_NPC(ch) && rps_candiate){
        ch->pcdata->did_ic=true;
		if(room_is_private(ch->in_room)){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
	}

}


/**************************************************************************/
// modified by Kalahn to use @ in place of name and use act
void do_pmote( char_data *ch, char *argument )
{
    char *letter;
    char act_text[MIL];
    char vict_text[MIL];
	
    char ch_arg[MIL];
    char_data *victim;
	
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        ch->println( "`cYou can't show your emotions.`x" );
        return;
    }
	
    argument = one_argument( argument, ch_arg );
	
    if ( ch_arg[0] == '\0' || argument[0] == '\0')
    {
        ch->println( "`cSyntax: pmote <target> <text>.`x" );
        ch->println( "`cThe text must include a single @ where you want the targets name.`x" );
        ch->println( "`ceg pmote guard smiles at @.`x" );
        return;
    }
	
    if (( victim = get_char_room( ch, ch_arg ) ) == NULL )
    {
        ch->printlnf( "`cYou can't seem to find the player '%s'.`x", ch_arg);
        return;
    }
	
    if (victim == ch)
    {
        ch->println( "`cYou can't direct pmotes at yourself... use emote.`x" );
        return;
    }
	
    if ( argument[0] == '\0' )
    {
        ch->printlnf( "`cPmote what towards %s.`x", PERS(victim, ch));
        return;
    }
	
	if (count_char(argument,'@') != 1)
    {
        ch->println( "`cSyntax: pmote <target> <text>.`x" );
        ch->println( "`cThe text must include a single @ where you want the targets name.`x" );
        ch->println( "`ceg pmote guard smiles at @.`x" );
        return;
    }

	// dont allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};
	
    strcpy (act_text, "$n ");
    strcpy (vict_text, "$n ");
	
    letter = argument;
	
    for (; *letter != '\0'; letter++)
    {
        if (*letter == '@')
        {
            if ( *(letter+1) == '\'' && *(letter+2) == 's')
            {
                strcat(act_text, "`#`x$N's`&");
                strcat(vict_text, "your");
                letter += 2;
            }
            else if ( *(letter+1) == 'r' || *(letter+1) == 's') 
            {
                strcat(act_text, "`#`x$N's`&");
                strcat(vict_text, "your");
                letter++;
            }
            else 
            { 
                strcat(act_text, "`#`x$N`&");
                strcat(vict_text, "you");
            }
        }
        else
        {
            strncat(act_text, letter,1);
            strncat(vict_text, letter,1);
        }
    }
	
    MOBtrigger = false;
    act( act_text,  ch, NULL, victim, TO_NOTVICT );
    act( act_text,  ch, NULL, victim, TO_CHAR );
    act( vict_text, ch, NULL, victim, TO_VICT );
    MOBtrigger = true;
	
	if(!IS_NPC(ch) && rps_candiate){
		ch->pcdata->did_ic=true;
		if(room_is_private(ch->in_room)){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
	}
}

/**************************************************************************/
//  modified by Kalahn to use * in place of name and use act 
void do_smote(char_data *ch, char *argument )
{
    char *letter;
    char act_text[MSL];
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) && !IS_AWAKE(ch))
    {
        ch->println( "`cYou can't show your emotions.`x" );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        ch->println( "`cSmote what?`x" );
        return;
    }
    
    if (count_char(argument,'*') != 1)
    {
        ch->println( "`cYou must include a single * where you want your name to appear in a smote.`x" );
        return;
    }
   
	// dont allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    act_text[0]='\0';
    letter = argument;

    for (; *letter != '\0'; letter++)
    {
        if (*letter == '*'){
            strcat(act_text, "`#`W$n`&");
        }else{
            strncat(act_text, letter,1);
		}     
    }

    MOBtrigger = false;
    act(act_text, ch, NULL, NULL, TO_ROOM );
    act(act_text, ch, NULL, NULL, TO_CHAR );
    MOBtrigger = true;

    return;
}


/**************************************************************************/
void do_bug( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println("`cThis command is used for reporting bug in any of code in the game.`x");
		ch->println("`cYou must type something as a parameter.`x");
		return;
	}

	sprintf(buf,"do_bug: %s reported the bug '%s' [room %d]\r\n", ch->name, argument, ch->in_room->vnum);
	wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel

	append_logentry_to_file( ch, BUG_FILE, argument );
	ch->println( "`cBug logged. (bug automatically records the room you are in by the way)`x" );

	 return;
}

/**************************************************************************/
void do_typo( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println( "`cThis command is used for reporting typos in any of the text anywhere in the game.`x" );
		ch->println( "`cYou must type something as a parameter, there is also the 'type' social.`x" );
		return;
	}
	
	sprintf(buf,"do_typo: %s reported the typo '%s' [room %d]\r\n", ch->name, argument, ch->in_room->vnum);
	wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel

     append_logentry_to_file( ch, TYPO_FILE, argument );
     ch->println( "`cTypo logged. (typo automatically records the room you are in by the way)`x" );
	 return;
}
/**************************************************************************/
/*
void do_event( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println("This command is used for reporting daily events.");
		ch->println("You must type something as a parameter.");
		return;
	}

	sprintf(buf,"%s\r\n", argument);

	append_event_to_file( ch, EVENT_FILE, argument );
	ch->println( "Event logged." );

	 return;
}
*/
/**************************************************************************/
void do_qui( char_data *ch, char *)
{
    ch->println( "`xIf you want to `RQUIT`c, you have to spell it out.`x" );
    return;
}
/**************************************************************************/
void mp_logout_trigger( char_data *ch);
/**************************************************************************/
void quit_char(char_data *ch, char *argument, bool character_deleting )
{
	connection_data *d,*c_next;
    int id;

	

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

	if(IS_SET(ch->dyn,DYN_USING_ATLEVEL)){
		ch->println( "`cYou can't quit while using at level!`x" );
		return;
	}

   //arena quiter 
   if ( IS_SET(ch->act2, ACT2_PLR_ARENA) && ch->position == POS_FIGHTING) 
    {              
       ch->println( "`cMaybe you should finish fighting first?`x\n\r"); 
            return; 
   } 
   if(IS_SET(ch->act2, ACT2_PLR_ARENA)) 
   { 
         char buf[MIL]; 
         char_data *wch; 

         //Tell spectators the dastardly deed. Not all quits are unwarranted. 
         //example: if the challenged never accepts or declines. 
         sprintf( buf, "`W%s `rcancels the contest by quitting`x", ch->name);    
         arena_broadcast( ch, buf ); 
          
         //Arena clean up system 
            for( wch = player_list; wch; wch = wch->next_player ) 
            { 
                                     
               if(can_see(wch, ch) && wch->pcdata->bet)    
               { 
                  //if bets were made pay back.                   
                  wch->gold += wch->pot; 
                  wch->pot = 0; 
                  wch->println("`R[ARENA]:`x Bet cancelled! You get your `Ygold`x back!\n\r"); 
                  wch->pcdata->bet = NULL; 
                  REMOVE_BIT(wch->act2, ACT2_PLR_BET);                                                 
               } 
            } 
            for( wch = player_list; wch; wch = wch->next_player ) 
            { 
               if(can_see(wch, ch) && IS_SET(wch->act2, ACT2_PLR_ARENA)){ 

                  REMOVE_BIT(wch->act2, ACT2_PLR_ARENA);              
                  if(wch->challenged != NULL){ 
                  wch->challenged = NULL; 
                  } 
                  if(wch->challenger != NULL){ 
                  wch->challenger = NULL; 
                  } 
                  //go through possible recall problems. 
                  //DO NOT SET ARENA ROOM FLAGS TO NORECALL 
                  if ( IS_SET(wch->affected_by, AFF_CURSE)){ 
                  REMOVE_BIT( wch->affected_by, AFF_CURSE ); 
                  } 
                  if(wch->pknorecall > 0){ 
                  wch->pknorecall = 0; 
                  } 
                  do_recall(wch, "");                
               } 
            } 
            for( wch = player_list; wch; wch = wch->next_player ) 
            { 
               if(can_see(wch, ch) && wch->challenger){ 
                  wch->challenger = NULL; //If they had not accepted yet and outside the arena. 
               } 
            }             
       
   }//end of arena quitter 

    if ( IS_SWITCHED(ch) )
    {
		if (IS_IMMORTAL(ch)){
			ch->println( "`cYou are currently controlling a mob... type `Rreturn`c first!`x" );
		}else{
			ch->println( "`cYou are not in your mortal body, type `Rreturn`c to get back to it then try quitting.!`x" );
		}
        return;
    }

    if ( IS_NPC(ch) )
        return;

	// ploaded players can't quit
	if(ch->pload && ch->pload->dont_save){
		if(ch->pload->loaded_by){
			ch->pload->loaded_by->printlnf( "quit_char(): character %s can't 'quit' cause it was ploaded.", ch->name);
		}			
		logf( "quit_char(): character %s can't 'quit' cause it was ploaded.", ch->name);
		return;
	}
	
	OBJ_DATA *nrflag;
	nrflag = ch->carrying;
	while ( nrflag )
	{
	OBJ_DATA *nrflagHolder;
	nrflagHolder = nrflag->next_content;
	if(IS_SET(nrflag->extra2_flags,OBJEXTRA2_STRIP_ON_QUIT))
	{obj_from_char(nrflag);
//	ch->printlnf("`cYou have dropped %s.`x", nrflag->description);
	}
	nrflag = nrflagHolder;}
	
	

	mp_logout_trigger( ch);

	if ( !character_deleting && !IS_LINKDEAD(ch) && ch->pnote && str_cmp("confirm", argument)) {
		
		switch(ch->pnote->type) {
			default:
				break;
			case NOTE_NOTE:
				ch->println( "`cYou have a Note in progress.`x\r\n" );
				break;
			case NOTE_IDEA:
				ch->println( "`cYou have an Idea in progress.`x\r\n" );
				break;
			case NOTE_PENALTY:
				ch->println( "`cYou have a Penalty note in progress.`x\r\n" );
				break;
			case NOTE_NEWS:
				ch->println( "`cYou have a News in progress.`x\r\n" );
				break;
			case NOTE_CHANGES:
				ch->println( "`cYou have a Change in progress.`x\r\n" );
				break;
			case NOTE_ANOTE:
				ch->println( "`cYou have an Anote in progress.`x\r\n" );
				break;
			case NOTE_INOTE:
				ch->println( "`cYou have an Inote in progress.`x\r\n" );
				break;
		}
		ch->println( "`cYou may either post it, clear it, or type `Rquit confirm`c to quit now.`x" );
		return;		
	}

    if (!( character_deleting
			|| ( IS_IMMORTAL(ch) && !str_cmp("confirm", argument))
		  )
	    ) // if they are deleting they can quit or if they are an imm typing 'quit confirm' 
    {
        if ( ch->position == POS_FIGHTING )
        {
            ch->println( "`cNo way! You are fighting.`x" );
            return;
        }

#ifndef unix
		if (moot->moot_type<0){
			moot->moot_type =0;
		}
#endif 
	
        if(ch->pcdata->diplomacy && moot->moot_type){
            ch->println( "`cYou can not quit while a moot is in progress.`x" );
            return;
        }
    
        if(moot->moot_victim==ch){
            ch->println( "`cYou can not quit while a moot is called against you.`x" );
            return;
        }

		if(!GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
			if(ch->pknorecall>0){
				if (IS_IMMORTAL(ch)){
					ch->println( "`cYou have a PK norecall timer (been in a pk fight).`x" );
					ch->println( "`cIf you wish to quit you must type `Rquit confirm`c.`x" );
				}else{
					ch->println( "`cYou may not leave this world so soon after conflict.`x" );
				}
				return;
			}
		}

        if(ch->pknoquit>0){
			if (IS_IMMORTAL(ch)){
				ch->println( "`cYou have a PK noquit timer (been in a pk fight).`x" );
				ch->println( "`cIf you wish to quit you must type `Rquit confirm`c.`x" );
			}else{
				ch->println( "`cYou may not leave this world so soon after PK interactions.`x" );
				ch->println( "`c(you have a pknoquit timer)`x" );
			}
            return;
        }
		
        if ( ch->position  < POS_STUNNED  )
        {
            ch->println( "`cYou're not DEAD yet.`x" );
            return;
        }
    }
	
    act( "`C$n`c leaves behind this mythological existence.`x", ch, NULL, 
NULL, TO_ROOM );
	
	// send the info broadcast about the person leaving
	//info_broadcast(ch, "%s leaves behind this mythological existence.", ch->name);

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
	{
		if(!IS_IMMORTAL(ch)){
			pkill_broadcast("%s fades from the realm of death! [Pk=%d.Pd=%d]\r\n", 
				ch->name,			
				TRUE_CH(ch)->pcdata->p9999kills,
				TRUE_CH(ch)->pcdata->p9999defeats);

		}
	}

    if (ch->desc){
        logf( "%s has left this existence. (connected_socket = %d), (left in room=%d, lasticroom=%d)", 
			ch->name, ch->desc->connected_socket, 
			ch->in_room?ch->in_room->vnum:0,
			ch->last_ic_room?ch->last_ic_room->vnum:0);
    }else{
        logf( "%s has left this existence. (descriptor = linkless) (left in room=%d, lasticroom=%d)", 
			ch->name, 
			ch->in_room?ch->in_room->vnum:0,
			ch->last_ic_room?ch->last_ic_room->vnum:0);
	}
    
    sprintf( log_buf, "%s has left this existence. (left level %d, remort %d), room=%d, lasticroom=%d", 
		ch->name, ch->level, ch->remort,
		ch->in_room?ch->in_room->vnum:0,
		ch->last_ic_room?ch->last_ic_room->vnum:0);
    wiznet(log_buf,ch,NULL,WIZ_LOGINS,0, UMIN(get_trust(ch), MAX_LEVEL));

	// record their exit into the game into their plog if they have one
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)){
        append_playerlog( ch, log_buf);
    }

    if (ch->desc){
		if (IS_IRC(ch)){
			sprintf( log_buf, "was %s@%s (IRC).", ch->name, ch->desc->remote_hostname);
		}else{
			sprintf( log_buf, "was %s@%s", ch->name, ch->desc->remote_hostname);
		}
		wiznet(log_buf,NULL,NULL,WIZ_SITES,0,UMIN(get_trust(ch),MAX_LEVEL));
	}

    // After extract_char the ch is no longer valid!
    save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, true );
    if ( d){
        connection_close( d );
	}else{
		log_string("do_quit: linkless player quiting");
	}

    // toast evil cheating bastards 
    for (d = connection_list; d != NULL; d = c_next)
    {
        char_data *tch;   
        c_next = d->next;
        tch = d->original ? d->original : d->character;
        if (tch && tch->id == id)
        {
            extract_char(tch,true);
            connection_close(d);
        } 
    }
  	if IS_IMMORTAL(ch)
	{
	ch->printlnf("`cThank you for your hard work here at `CAthens`c.");
	}
	else
	{
	ch->printlnf("`cThe mythological world awaits your return.`x");
 	WAIT_STATE(ch,3*PULSE_VIOLENCE);
	}
	return;
}
/**************************************************************************/
void do_quit( char_data *ch, char *argument )
{	
	if(!IS_SET(ch->in_room->room_flags, ROOM_OOC) && !IS_SET(ch->in_room->room2_flags, ROOM2_QUITTABLE) && 
!IS_IMMORTAL(ch))
	{
	ch->println("`cYou can't quit here, this room isn't a safe haven.`x\r\n");

    	if (IS_NEWBIE(ch)){
        ch->println("`cType `RRECALL`c or `RQUIT`c to go somewhere suitable.`x\r\n");
    	}
	return;
	}
	else
	{quit_char(ch, argument, false);}
	
}

/**************************************************************************/
void do_save( char_data *ch, char *)
{
	 if ( IS_NPC(ch) )
	return;

	 save_char_obj( ch );
     ch->printlnf("`cYour character has been saved.`x");
     ch->printlnf( "`RRemember that %s has automatic saving.`X", MUD_NAME );

#ifdef unix
     //WAIT_STATE(ch,1 * PULSE_VIOLENCE);
#endif
    return;
}


/**************************************************************************/
// RT changed to allow unlimited following and follow the NOFOLLOW rules 
void do_follow( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
	
    one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "`cFollow whom?`x" );
		return;
	}
	
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->println( "`cThey aren't here.`x" );
		return;
    }
	
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
		act( "`cBut you'd rather follow $N!`x", ch, NULL, ch->master, TO_CHAR );
		return;
    }
	
    if ( victim == ch )
    {
		if ( ch->master == NULL )
		{
			ch->println( "`cYou already follow yourself.`x" );
			return;
		}
		stop_follower(ch);
		return;
    }
	
    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
		act("`c$N doesn't seem to want any followers.`x\r\n",
			ch,NULL,victim, TO_CHAR);
		return;
    }
	
    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
		stop_follower( ch );
	
    add_follower( ch, victim );
	return;
}

/**************************************************************************/
void add_follower( char_data *ch, char_data *master )
{
	if ( ch->master != NULL )
	{
		bug("Add_follower: non-null master.");
		return;
	}

	ch->master = master;
	ch->leader = NULL;

	if ( can_see( master, ch ))
	{
		if ( !IS_SET(ch->dyn,DYN_SILENTLY))
		{
			act( "`c$n now follows you.`x", ch, NULL, master, TO_VICT );
		}
	}

    act( "`cYou now follow $N.`x",  ch, NULL, master, TO_CHAR );

    return;
}


/**************************************************************************/
void stop_follower( char_data *ch )
{
    if ( ch->master == NULL )
    {
		bug("Stop_follower: null master.");
		return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
		REMOVE_BIT( ch->affected_by, AFF_CHARM );
		affect_strip( ch, gsn_charm_person );

		// remove loophole where players would tell a mob to attack then 
		// do nofollow leaving the mob to fight to the death
		if(IS_NPC(ch)){
			if(ch->fighting && IS_NPC(ch->fighting)){
				do_flee(ch,"");
				do_flee(ch,"");
				do_flee(ch,"");

				// if they dont suceed in fleeing they become worthless
				if(ch->fighting){
					ch->no_xp=true;
					ch->hitroll=-30;
					ch->damroll=-30;
					ch->hit=1;
					ch->max_hit=1;
				}
			}
		}
    }

	if ( ch->in_room && can_see( ch->master, ch ))
    {
		if ( IS_SET( ch->dyn, DYN_SILENTLY ))
		{
			act( "`c$n stops following you.`x", ch, NULL, ch->master, TO_VICT );
			act( "`cYou stop following $N.`x",	ch, NULL, ch->master, TO_CHAR );
		}
	}
	if (ch->master->pet == ch)
		ch->master->pet = NULL;

	ch->master = NULL;
	ch->leader = NULL;
    return;
}
/**************************************************************************/
bool is_character_loaded_into_room(char_data *ch, ROOM_INDEX_DATA *room);
/**************************************************************************/
// nukes charmed monsters and pets
void nuke_pets( char_data *ch)
{    
    char_data *pet;

    if ((pet = ch->pet) != NULL)
    {
		// if they arent in the game yet (haven't logged in, and this 
		//   isn't a reconnect)  set pet->room to NULL
		if((ch->desc && ch->desc->connected_state!=CON_PLAYING) || !ch->desc){
			if(!is_character_loaded_into_room(ch->pet,ch->pet->in_room)){
				ch->pet->in_room=NULL;
			};
		}
		stop_follower(pet);

		if (pet->in_room != NULL)
			 act("`c$N slowly fades away.`x",ch,NULL,pet,TO_NOTVICT);
		extract_char(pet,true);
	 }
	 ch->pet = NULL;

    return;
}



/**************************************************************************/
void die_follower( char_data *ch )
{
    char_data *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

	 ch->leader = NULL;

	 for ( fch = char_list; fch != NULL; fch = fch->next )
    {
		 if(ch->pet==fch){ // pets arent affected by nofollow
			 continue;
		 }
		if ( fch->master == ch )
			 stop_follower( fch );
		if ( fch->leader == ch )
			fch->leader = fch;
    }

    return;
}


/**************************************************************************/
void do_order( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg[MIL], arg2[MIL], arg3[MIL];
	char_data *victim;
	char_data *recipient;
	char_data *och;
	char_data *och_next;
	bool found;
	bool fAll;
	
	argument = one_argument( argument, arg );
	argument = one_argument(argument,arg2);
	
	if ( IS_NULLSTR(arg) || IS_NULLSTR(arg2))
	{
		ch->println( "`cOrder whom to do what?`x" );
		return;
	}

	if ( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		ch->println( "`cYou feel like taking, not giving, orders.`x" );
		return;
	}
	
	if ( !str_cmp( arg, "all" ) )
	{
		fAll   = true;
		victim = NULL;
	}   
	else
    {
        fAll   = false;

		// allow order pet
		if(ch->pet && !str_cmp(arg ,"pet")){
			victim=ch->pet;
			if(ch->in_room!=victim->in_room){
				ch->println( "`cYour pet isn't here.`x" );
				return;
			}
		}else{
			if (( victim = get_char_room( ch, arg )) == NULL )
			{
				ch->println( "`cThey aren't here.`x" );
				return;
			}
		}
		
        if ( victim == ch )
        {
			ch->println( "`cAye aye, right away!`x" );
            return;
        }
		
        if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
			||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
        {
            ch->println( "`cDo it yourself!`x" );
            return;
        }
    }
	
	strcat(arg2, " ");
	strcat(arg2, argument);
    found = false;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
        och_next = och->next_in_room;
		
        if ( IS_AFFECTED(och, AFF_CHARM)
			&&   och->master == ch
			&& ( fAll || och == victim ) )
        {
            if ( !str_prefix("master", arg2))
            {
				argument = one_argument(argument, arg3);
				
                if  (!IS_SET(och->act, ACT_PET))
				{
                    ch->println( "`cThat is not your pet.`x" );
                    return;
                }
				
                if ( IS_NULLSTR(arg3))
                {
                    ch->println( "`cWho would you like to give your pet to?`x" );
                    return;
                }
                
                if ( ( recipient = get_char_room( ch, arg3 ) ) == NULL )
                {
                    ch->println( "`cThere is nobody like that here to take the pet.`x" );
                    return;
                }
				
				if (IS_NPC(recipient)) 
                {
                    ch->println( "`cYou may only give pets to other players.`x" );
                    return;
                }
				
				if (recipient->level < och->level)   
                {
                    act( "`cIt appears as though your pet would be the master of $N.`x", 
						ch, NULL, recipient, TO_CHAR);
                    return;
                }
				
				if (recipient->pet){
                    act( "`cIt appears as though $N already has a pet.`x", 
						ch, NULL, recipient, TO_CHAR);
                    return;
				}
				
				stop_follower(och);
				ch->pet = NULL; 
				
				SET_BIT(och->act, ACT_PET);
				SET_BIT(och->affected_by, AFF_CHARM);
				add_follower( och, recipient);
				och->leader = recipient;
				recipient->pet = och;
				
				act("`c$n is now $N's pet.`x", och, NULL, recipient, TO_ROOM);
				act("`cYou are now $N's pet.`x", och, NULL, recipient, TO_CHAR);  			
				return;
            }
            else
            {
				// PC being ordered to do this - 3% chance of breaking the charm
				if ( !IS_NPC(och) && number_range(1,33)==1 && !IS_IMMORTAL(ch))
				{
					sprintf( buf, "`c$n orders you to '%s', you manage to come to your senses and break the charm!`x", arg2 );
					affect_parentspellfunc_strip( och, gsn_charm_person);
					act( buf, ch, NULL, och, TO_VICT );
					ch->printlnf( "`c%s doesn't appear to have heard you!`x", PERS(och, ch));
					continue;
				}

				found = true;
				// Toggle the Ordered Bit
				SET_BIT( och->dyn, DYN_IS_BEING_ORDERED );

				sprintf( buf, "`c$n tells you to '%s'.\nYou can't resist.`x", arg2 );
				act( buf, ch, NULL, och, TO_VICT );
				interpret( och, arg2 );

				// Remove the Ordered Bit
				REMOVE_BIT( och->dyn, DYN_IS_BEING_ORDERED );
            }
        }
    }
	
    if ( found )
    {
        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->println( "`cOk.`x" );
    }
    else
        ch->println( "`cYou have no followers here.`x" );
    return;
}


/**************************************************************************/
void do_group( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		char_data *gch;
		char_data *leader;
		
		leader = (ch->leader != NULL) ? ch->leader : ch;
		ch->printlnf( "`c%s's group:`x", icapitalize( PERS( leader, ch )));

		for ( gch = char_list; gch; gch = gch->next )
		{
			if ( is_same_group( gch, ch ) )
			{
				// has hacks to prevent div by 0 
				ch->printlnf( "[ %3d%% hp - %3d%% mana - %3d%% mv ] %s",
					(int) gch->hit*100/(gch->max_hit==0?1:gch->max_hit),
					(int) gch->mana*100/(gch->max_mana==0?1:gch->max_mana),
					(int) gch->move*100/(gch->max_move==0?1:gch->max_move),
					capitalize( PERS(gch, ch) ));
			}
		}
		return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->println( "`cThey aren't here.`x" );
		return;
	}

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
		ch->println( "`cBut you are following someone else!`x" );
		return;
	}

    if ( victim->master != ch && ch != victim )
    {
		act( "`c$N isn't following you.`x", ch, NULL, victim, TO_CHAR );
		return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
		ch->println( "`cYou can't remove charmed mobs from your group.`x" );
        return;
    }
    
	if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act("`cYou like your master too much to leave $m!`x",ch,NULL,victim,TO_VICT);
    	return;
    }

	if(GAMESETTING(GAMESET_RESTRICTED_GROUPING)){
		if (victim->level - ch->level > 8)
		{
			ch->println( "`cThey are too high of a level for your group.`x" );
			return;
		}
 
		if (victim->level - ch->level < -8)
		{
			ch->println( "`cThey are too low of a level for your group.`x" );
			return;
		}
	}

	if ( IS_NPC( victim ))
	{
		ch->println( "`cYou can't use this command to group mobs. `x");
		return;
	}

	if ( is_same_group( victim, ch ) && ch != victim )
    {
		victim->leader = NULL;
		act( "`c$n removes you from $s group.`x",  ch, NULL, victim, TO_VICT    );
		act( "`cYou remove $N from your group.`x", ch, NULL, victim, TO_CHAR    );
		return;
    }

    victim->leader = ch;
	act( "`cYou join $n's group.`x", ch, NULL, victim, TO_VICT    );
    act( "`c$N joins your group.`x", ch, NULL, victim, TO_CHAR    );
    return;
}
/**************************************************************************/
void do_report(char_data *ch, char *)
{
	// has hacks to prevent div by 0 
	do_say(ch, 
		FORMATF( "`cI have `C%3d%%`c hp.`x",
			(int) ch->hit*100/(ch->max_hit==0?1:ch->max_hit)));		
}
/**************************************************************************/
void do_split( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg1[MIL],arg2[MIL];
	char_data *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	argument = one_argument( argument, arg1 );
	one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		ch->println( "`cSplit how much?`x" );
		return;
	}

	amount_silver = atoi( arg1 );

	if (arg2[0] != '\0')
		amount_gold = atoi(arg2);

	if ( amount_gold < 0 || amount_silver < 0)
	{
		ch->println( "`cYour group wouldn't like that.`x" );
		return;
	}

	if ( amount_gold == 0 && amount_silver == 0 )
	{
		ch->println( "`cYou hand out zero coins, but no one notices.`x" );
		return;
	}

	if ( ch->gold <  amount_gold || ch->silver < amount_silver)
	{
		ch->println( "`cYou don't have that much to split.`x" );
		return;
	}

	members = 0;
	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
			members++;
	}
	
	if ( members < 2 )
	{
		ch->println( "`cJust keep it all.`x" );
		return;
	}
	
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;
	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if ( share_gold == 0 && share_silver == 0 )
	{
		ch->println( "`cDon't even bother, cheapskate.`x" );
		return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;
    
	if (share_silver > 0)
    {
		ch->printlnf( "`cYou split %d silver coins. Your share is %d silver.`x",
			amount_silver,share_silver + extra_silver);
    }

	if ( share_gold > 0)
	{
		ch->printlnf( "`cYou split %d gold coins. Your share is %d gold.`x",
			amount_gold,share_gold + extra_gold);
	}

	if (share_gold == 0)
	{
		sprintf(buf,"`c$n splits %d silver coins. Your share is %d silver.`x",
			amount_silver,share_silver);
	}
	else if (share_silver == 0)
	{
		sprintf(buf,"`c$n splits %d gold coins. Your share is %d gold.`x",
			amount_gold,share_gold);
	}
	else
	{
		sprintf(buf,
			"`c$n splits %d silver and %d gold coins, giving you %d silver and %d gold.`x\r\n",
			amount_silver,amount_gold,share_silver,share_gold);
	}

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
		{
			act( buf, ch, NULL, gch, TO_VICT );
			gch->gold += share_gold;
			gch->silver += share_silver;
		}
	}
	return;
}


/**************************************************************************/
void do_gtell( char_data *ch, char *argument )
{
    char buf[MSL];
    char_data *gch;

    if ( argument[0] == '\0' )
    {
		ch->println( "`cTell your group what?`x" );
		return;
	}

	if ( IS_SET( ch->comm, COMM_NOTELL ) )
	{
		ch->println( "`cYour message didn't get through!`x" );
		return;
	}

	// Note use of gch->printlnf so gtell works on sleepers.
	sprintf( buf, "`c%s tells the group '`C%s`c'`x", ch->name, argument );
	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
		if ( is_same_group( gch, ch ) )
			gch->printlnf( "%s", buf );
	}

	return;
}

/**************************************************************************/
void do_resetroom(char_data *ch, char *)
{
	reset_room( ch->in_room, true );
	ch->printlnf( "Room '%s' [%d] reset.",
		ch->in_room->name, ch->in_room->vnum);
    return;
}

/**************************************************************************/
void do_resetarea(char_data *ch, char *)
{
	reset_area( ch->in_room->area);
	reset_room( ch->in_room, true );
	ch->printlnf( "`cArea '%s' [%d-%d] reset.`x",
		ch->in_room->area->name, ch->in_room->area->min_vnum,
		ch->in_room->area->max_vnum);
    return;
}

/**************************************************************************/
void do_crashloop (char_data *ch, char * argument)
{
	if (IS_NPC(ch))
	{
		do_huh(ch,"");
        return;
	}

    if (str_cmp("confirm", argument))
    {
        ch->println( "`xType `=Ccrashloop confirm`x if you want to put the mud into" );
        ch->println( "`xan endless loop to test the signal handling!" );
		return;
	}

	for (;;);  // loop endlessly ( Now that's some awesome code :) -- Ker )
}
/**************************************************************************/
/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( char_data *ach, char_data *bch )
{
	if ( ach == NULL || bch == NULL)
		return false;
    
	if ( ach->leader != NULL ) ach = ach->leader;
	if ( bch->leader != NULL ) bch = bch->leader;
	return ach == bch;
}

/**************************************************************************/
void do_ntell( char_data *ch, char *argument )  
{
	char arg[MIL], buf[MSL];
    char_data *nch, *victim, *nvictim;

    // unswitched mobs can't send ntells
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

	if (!IS_NOBLE(ch)){
		do_huh(ch,"");
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "`cNot going to happen.`x" );
		return;
	}

    argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		ch->println( "`cNoble tell whom what?`x" );
		return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL
		||  IS_NPC(victim) ) 
	{
		ch->println( "`cThey aren't here.`x" );
		return;
	}
	// Put in these lines from do_tell in because it made some sense. <shrug>
	if (victim->controlling && victim->controlling->desc)
	{
		victim=victim->controlling;
	}
	
	if(ch==victim)
	{
		ch->println( "`cNo point in sending an ntell to yourself.`x" );
		return;
	};

	// Also from do_tell, looked good to put.
	if ( victim->desc == NULL)
	{
		act("`c$N seems to have misplaced $S link...try again later.`x", ch,NULL,victim,TO_CHAR);
		
		sprintf(buf,"`cA noble ntells you '%s'`x\r\n`x", argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	if (IS_SET(victim->comm,COMM_AFK))
	{
		if (IS_NPC(victim))
		{
			act("`c$E is AFK, and not receiving tells.`x",ch,NULL,victim,TO_CHAR);
			return;
		}

		act("`c$E is AFK, but your tell will go through when $E returns.`x", ch,NULL,victim,TO_CHAR);
		
		sprintf(buf,"`cA noble ntells you '%s'`x\r\n", argument );
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
    
		victim->printlnf( "`cA noble just tried to ntell you '%s' (you are marked as afk)`x", argument);
		return;
	}

	if(!IS_NPC(ch))
		ch->pcdata->did_ooc=true;

	// Send the tell to the player.
	victim->printlnf("`M%s ntells you '%s`M'"
		"(you can use nreply to talk back if you need to)`x\r\n", 
		IS_IMMORTAL(victim)?PERS(ch,victim):"A noble", argument );

	// Start the broadcast of the ntell to all nobles.
	for ( nch = player_list; nch; nch = nch->next_player )
	{
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou ntell %s '%s`M'", victim->name, argument);
			continue;
		}

		// dont see both the noble broadcast and the personal message
		if(	nch==victim)
		{
			continue;
		};

		nvictim=nch;
		// ntalk going thru switch
		if (nvictim->controlling && nvictim->controlling->desc)
			nvictim=nvictim->controlling;

		if (IS_NOBLE(nch))
		{
			if (IS_IMMORTAL(nch))
			{
				nvictim->printlnf( "`M<%s ntells %s>: '%s`M'`x", 
					TRUE_CH(ch)->name, victim->name, argument);
			}else{
				nvictim->printlnf( "`M<A noble ntells %s>: '%s`M'`x", victim->name, argument);
			}
		}
	}
	return;
}

/**************************************************************************/
void do_nreply( char_data *ch, char *argument )
{
	char_data *nch, *victim;

    // unswitched mobs can't send nreplies
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

    if ( IS_NULLSTR(argument) )
    {
        ch->println( "`cNreply what?`x" );
        return;
    }

	//Start the broadcast of the nreply to all nobles.
    for ( nch = player_list; nch; nch = nch->next_player )
    {
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou nreply '%s`M'", argument);
			continue;
		}

		victim=nch;

		// ntalk going thru switch
		if (victim->controlling && victim->controlling->desc)
			victim=victim->controlling;

		if (IS_NOBLE(nch))
		{
			victim->printlnf( "`M<%s nreplies> '%s`M'`x",
					TRUE_CH(ch)->name, argument);
		}
	}
	return;
}

/**************************************************************************/
void do_declineooc(char_data *ch, char *argument)
{
    char_data *victim;
	char arg[MIL];
	int minutes = 0;

	argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ))
    {
        ch->println( "`cWho do you want to decline?`x" );
        return; 
    }
	if ( !IS_NULLSTR( argument ))
	{
		minutes = atoi( argument );
	}

    if (( victim = get_whovis_player_world( ch, arg )) == NULL )
    {
        ch->println( "`cThey are not playing.`x" );
        return;
    }

    ch->printlnf( "`cSending ooc chat decline to %s.`x", victim->name);
	if ( minutes > 0 )
	{
		ch->printlnf( "`c  You have specified that you will be available in %d minute(s).`x", minutes );
	}
    victim->printlnf( "`c%s has declined an ooc chat with you.", ch->name );
	if ( minutes > 0 )
	{
		victim->printlnf( "`c  %s has specified that they will be available in approximately %d minute(s).`x",
			ch->name, minutes );
		victim->println( "`c  Please respect their wishes and be patient.`x" );
	}
}

/**************************************************************************/
extern bool hotreboot_in_progress;
/**************************************************************************/
// turn mccp on/off
void do_mccp( char_data *ch, char *argument )
{
	connection_data *d=TRUE_CH(ch)->desc;
#ifndef MCCP_ENABLED
	ch->wrapln("Mud Client Compression Protocol (MCCP) support has not "
		"been enabled for this compile of the mud therefore unavailable.");
	ch->wraplnf("There have been a total of %d uncompressed bytes sent to your connection", 
		d?d->bytes_sent:0);
	return;
#else
    if (!d) {
        ch->println("You don't have an descriptor!?!");
        return;
    }
	
	if(IS_NULLSTR(argument)){
		unsigned int bs=d->bytes_sent;
		unsigned int bsbc=d->bytes_sent_before_compression;
		unsigned int bsac=d->bytes_sent_after_compression;
		ch->titlebar("MUD CLIENT COMPRESSION PROTOCOL SYNTAX");
		ch->println("  Syntax: `=Cmccp on2`x    - to force mccp on (v2 startup).");
		ch->println("  Syntax: `=Cmccp on1`x    - to force mccp on (v1 startup).");
		ch->println("  Syntax: `=Cmccp off`x    - to force mccp off.");
		ch->titlebar("MCCP CONNECTION STATISTICS");
		if(d->out_compress){
			ch->printlnf("  `WYour connection is currently connected with mccp%d.`x",
				d->mccp_version);
		}else{
			ch->println("  Your connection is currently not making use of mccp.");
		}
		ch->printlnf("  Total bytes sent to your connection: %d", bs);

		if(d->out_compress){
			ch->printlnf("  Bytes sent to your connection after compression: %7d", bsac);
			ch->printlnf("  Bytes sent to your connection before compression:%7d", bsbc);
			if(bsbc){
				ch->printlnf("  Compression ratio: %0.2f%%`x", (float)bsac*100/ (float)bsbc );
			}
		}
		ch->titlebar("");
		return;
	}

	if(hotreboot_in_progress){
		ch->println("You can't change your mccp status while a hotreboot is in progress");
		return;
	}

	if(!str_cmp("on2", argument)){
		if(d->out_compress){
			ch->println("Your connection is already compressed.");
		}else{
			ch->println("Manually starting mccp2 compression.");
			d->mccp_version=2;
			d->begin_compression();
		}
		return;
	}

	if(!str_cmp("on1", argument)){
		if(d->out_compress){
			ch->println("Your connection is already compressed.");
		}else{
			ch->println("Manually starting mccp1 compression.");
			d->mccp_version=1;
			d->begin_compression();
		}
		return;
	}

	if(!str_cmp("off", argument)){
		if(d->out_compress){
			ch->printlnf("Stopping mccp%d compression.", d->mccp_version);
			d->end_compression();
		}else{
			ch->println("You don't currently have a compressed connection to turn off.");
		}
		return;
	}
	ch->printlnf("Unrecognised command '%s'", argument);
	do_mccp(ch,"");
#endif // MCCP_ENABLED
}
/**************************************************************************/
void do_advertise( char_data *ch, char * )
{
   OBJ_DATA *obj;
	if (!IS_NPC(ch))
 	{ 
		for ( obj = ch->carrying; obj; obj = obj->next_content )
    		{
        	if ( obj->item_type == ITEM_FLYER && obj->wear_loc == WEAR_HOLD )
	            break;
    		}
    
		if ( !obj )
    		{
        		ch->println("`cYou are not holding a flyer.`x");
        		return;
    		}else{ 

		ch->println( "`cYou advertise to anyone who would listen.`x" );
		act("`c$n advertises about an upcoming play.`x",ch,NULL,NULL,TO_ROOM);
			return;
		}
	}
	else
	{
		ch->println( "`cMobiles don't need to advertise.`x" );
	}
}
/**************************************************************************/
void do_assassinate( char_data *ch, char *argument )
{
    char_data *victim;
    char arg[MIL];
    one_argument( argument, arg  );

    	if ( arg[0] == '\0')
    	{
		ch->println( "`cAssassinate whom?`x" );
		return;
	}

	if(!GAMESETTING2(GAMESET2_ASSASSINS))
	{
		ch->println( "`cThe assassins of Nemesis are not available at this time.`x" );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "`cThere is no one in Greece with by that name right now.`x" );
		return;
	}

    	if ( ch == victim )
    	{
		ch->println( "`cYou're better off attacking a monster to do that.`x" );
		return;
    	}

   	if ( IS_NPC(victim))
		{
			ch->println( "`cYou can not hire an assassin for them.`x" );
			return;
		}
	if ( IS_IMMORTAL(victim))
		{
			ch->println( "`cYou can not hire an assassin for them.`x" );
			return;
		}

	if ( ch->gold < 1000 )
	{
		ch->println( "`cYou must have a thousand gold to hire an assassin!`x" );
		return;
	}
	ch->gold -= 1000;
	ch->println( "`cYou have hired an assassin.`x" );
	victim->println("`cA contracted assassin creeps out of the shadows to kill you.`x");
	raw_kill( victim, ch);
        act( "`c$n APPEARS FROM THE LAND OF THE LIVING.`x", victim, NULL, NULL, TO_ROOM );
	return;
}
/**************************************************************************/ 
void do_wager(char_data *ch, char *argument) 
{ 
    char arg1 [MIL];  
    char arg2 [MIL]; 
    int adjust; 
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 ); 
  
   if (IS_NPC(ch)) 
   { 
      ch->println("Mobiles don't have a need to wager.\r\n"); 
      return; 
   } 
  
	if(!GAMESETTING2(GAMESET2_APPLEBET))
	{
		ch->println( "`cThe apple toss is not being wagered on right now.`x" );
		return;
	}
if(!str_cmp(arg2, "aphrodite") || !str_cmp(arg2, "hera") || !str_cmp(arg2, "athena"))
   {

    	if (arg1[0] == '\0' || arg2[0] == '\0') 
    	{ 
        	ch->println("`cSyntax: wager <amount> <goddess>.`x"); 
      		ch->println("`cWagers limited to 100 gold.`x"); 
        	return; 
    	} 
    
   	if (!strcmp(arg2, "gold")) 
   	{ 
      		ch->println("`cDon't type gold, type the goddess' name.`x"); 
        	return; 
   	} 
 
   
   	int value = atoi(arg1); 
    
   	if(value == 0) 
   	{ 
      		ch->println("`cPlace a wager higher than zero!`x\r\n"); 
      		return; 
   	} 

   	if(value > 100) 
   	{ 
      		ch->println("`cEnter a value between 1 and 100.`x\n\r"); 
      		return; 
   	} 

   	if (value * 100 > (ch->gold *100 + ch->silver) ) 
    	{ 
      		ch->println("`cYou do not have enough gold on you!`x\n\r"); 
      		return; 
    	} 
  
  
    	if(ch->apot > 0){ 
     		ch->printlnf("`cA bookie gives your old wager %d gold back, then...`x", ch->pot); 
     		adjust = ch->pot; 
     		ch->gold += adjust; 
     		ch->pot = 0;     
    	} 
  
    	ch->pcdata->wager = str_dup(arg2); 
    	ch->printlnf("`cA bookie takes your wager of %d `Ygold`x coins on %s.`x\r\n", value, arg2); 

    	SET_BIT(ch->act2, ACT2_PLR_WAGER); 
    	deduct_cost(ch,value * 100);             
    	ch->apot += value; 
    	if (ch->apot > 100){ 
      		adjust = ch->apot - 100; 
      		ch->gold += adjust; 
       		ch->apot = 100; 
      	} 
  }else{
    	ch->printlnf("`cYou must chose amoung Aphrodite, Athena and Hera.`x"); 
	return;
  }  
} 
/**************************************************************************/
 void do_trade( char_data *ch, char *argument, int cmdnumber )
{
/*
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  OBJ_DATA *obj;
  char buf[MSL];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );


if(IS_SET(TRUE_CH(ch)job, JOB_TRADER))
  {

*/
    ch->printlnf( "Trading commodities is limited to Traders.\n\r");
    ch->printlnf( "See: HELP TRADER for more information.\n\r");
    return;
/*
  }

 
  if ( arg1[0] == '\0' )
  {
    ch->printlnf( "Syntax:   TRADE [<COMMAND>] [<SUB-COMMAND>]\n\r");
    ch->printlnf( "Commands: START, STOP, PRICE, SELL, REMOVE\n\r");
    return;
  }

  if ( !str_cmp( arg1, "remove" ) )
  {
    if ( arg2[0] == '\0' )
    {
      ch->printlnf("Remove which item from your trading list?\n\r");
      return; 
    }

    if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL )
    {
       ch->printlnf("You haven't got that item!\n\r");
       return;
    }

    if ( !IS_OBJ_STAT( obj, ITEM_FOR_TRADE ) )
    {
       ch->printlnf("This item isn't for trade to start with.\n\r");
       return;
    }
    if ( !obj->item_type == ITEM_COMMODITY )
       ch->printlnf("You can only trade commodities.\n\r");
       return;
    }
    REMOVE_BIT(obj->extra2_flags, ITEM_FOR_TRADE);
    sprintf(buf,"You remove %s from your trading list!\n\r",
     obj->short_descr);
    ch->printlnf(buf,ch);
  }

  if ( !str_cmp( arg1, "price" ) )
  { 
    if ( arg2[0] == '\0' )
    {
      ch->printlnf("Price for what, and at what price?\n\r");
      return;
    } 

    if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL )
    {
       ch->printlnf("You haven't got that item!\n\r");
       return;
    }

    if ( !IS_OBJ_STAT( obj, ITEM_FOR_TRADE ) )
    {
       ch->printlnf("This item isn't for trade to start with.\n\r");
       return;
    }
 
    if ( arg3[0] == '\0' )
    {
       ch->printlnf("How much for it?\n\r");
       return;
    }
 
    if ( !is_number(arg3) )
    {
       ch->printlnf("That is not a valid price!\n\r");
       return;
    }

    obj->player_cost = atoi(arg3);
    sprintf(buf,"You reprice %s for %d gold.\n\r",obj->short_descr,obj->player_cost);
    ch->printlnf(buf,ch);
    return;
  }

  if ( !str_cmp( arg1, "sell" ) )
  {
    if ( arg2[0] == '\0' )
    {
       ch->printlnf("Sell what and for how much?\n\r");
       return;
    }

    if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL )
    {
       ch->printlnf("You haven't got that item!\n\r");
       return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_FOR_TRADE ) )
    {
       ch->printlnf("This item is already for trade.\n\r");
       return;
    }
    
    if ( arg3[0] == '\0' )
    {
       ch->printlnf("How much for it?\n\r");
       return;
    }
  
    if ( !is_number(arg3) )
    {
       ch->printlnf("That is not a valid price!\n\r");
       return;
    }

    obj->player_cost = atoi(arg3); 
    sprintf(buf,"You begin selling %s for an opening value of %d.\n\r",obj->short_descr,obj->player_cost);
    ch->printlnf(buf,ch);
    SET_BIT(obj->extra2_flags, ITEM_FOR_TRADE);
    return;      
  }

  if ( !str_cmp( arg1, "start" ) )
  {
    if ( IS_SET(ch->act, PLR_TRADING) )
    {
      ch->printlnf("You are already trading.\n\r");
      return;
    }
 else
    ch->printlnf("You start trading.\n\r");
    SET_BIT(ch->act, PLR_TRADING);
    return;
  }

  if ( !str_cmp( arg1, "stop" ) )
  {
    if ( !IS_SET(ch->act, PLR_TRADING) )
    {
      ch->printlnf("You are not currently trading.\n\r");
      return;
    }
 
    ch->printlnf("You stop trading.\n\r");
    REMOVE_BIT(ch->act, PLR_TRADING);
    return;
  }

  return;
*/
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

