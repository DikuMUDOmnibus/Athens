/**************************************************************************/
// affects.cpp - see below
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
/***************************************************************************
 *  FILE: affects.cpp - written mainly by Kalahn                           *
 *                                                                         *
 *  affects_update() - called by update_handler in update.c                *
 ***************************************************************************/
#include "include.h" // standard dawn includes

/********************************/
/* START OF FUNCTION PROTOTYPES */
DECLARE_DO_FUN(do_bug);
void rand_to_char(char_data *ch, char *str1, char *str2, char *str3, char *str4, char *str5);
void mount( char_data *, char_data *);
void dismount( char_data *);


/**** local prototypes below ****/
void do_fearful(char_data *);
void do_fearmagic(char_data *);
void do_animal_train(char_data *);

/*  END OF FUNCTION PROTOTYPES  */
/********************************/


/************************************************************************/
/*
 * Control the active affects going on.
 * Called periodically by update_handler.
 */
void affects_update( void )
{
	char_data *ch;

	for ( ch = char_list; ch; ch = ch->next )
	{
		// added stuff fear - Gwynn
		if (IS_AFFECTED(ch, AFF_FEAR) && (number_range(1,3)==1))
		{ 
			do_fearful(ch);
		}          

		if (IS_AFFECTED2(ch, AFF2_FEAR_MAGIC) && (number_range(1,3)==1))
		{ 
			do_fearmagic(ch);
		}          

		// check for missing mounts	
		if (ch->mounted_on)
		{
			if (ch->mounted_on->in_room!=ch->in_room)
			{
				ch->println("`cYour mount has inexplicably disappeared.`x");
				dismount(ch);
				bug("BUG affects_update(): Mount in different room than rider.");
			}
		}

		if (ch->bucking){
			do_animal_train(ch);
		}
	}
	return;
}

/************************************************************************/
// for spell_fear_magic - Tibault
void do_fearmagic(char_data *ch)
{
    int attempt;
    ROOM_INDEX_DATA *was_in;
    bool found;
    EXIT_DATA *pexit;
    int door;
	
	if IS_OOC(ch)
		return;
	
	// record the room they start in
    was_in = ch->in_room;
	
    switch(ch->position)
    {
	case POS_SLEEPING:
		rand_to_char(ch,
			"`cYou have nightmares about bolts of lightning striking you.`x\r\n",
			"`cYou have nightmares about being transformed into something hideous.`x\r\n",
			"`cYou have a nightmare about people plotting revenge against you.`x\r\n",
			"`cYou have nightmares about a life full of strife.`x\r\n",
			"`cYou have nightmares about drowning in seas' violent storms.`x\r\n" );
		
		ch->println("`cYou awaken, a powerful fear of the gods still vibrant in your mind.`x");
		ch->position = POS_RESTING;
		break;
		
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
		ch->println("`cYou can't help but imagine the gods laughing from above.`x");
		ch->position = POS_STANDING;
		
	case POS_FIGHTING:
	case POS_STANDING:
		if (ch->mounted_on!=NULL)
		{
			ch->println("`cYour mount senses your fears and bucks, startled.`x");
			dismount(ch);
		}
		ch->println("`cIs this the work of the gods, or just your imagination running wild?`x");
    }
    
    if (ch->position < POS_STANDING)
		return;
	
	rand_to_char(ch,
		"`cSmall clouds gather here momentarily.  Is a thunderbolt on its way?`x",
		"`cA tidal wave washes over everything in sight, but nothing happens.`x",
		"`cA loud and threatening laugh roars down from Mount Olympus.`x",
		"`cA dead hand appears from the Underworld then disappears suddenly.`x",
		"`cA dark omen, a shadow-shaped like Cerberus streaks your line of vision.`x");
	
	ch->println("\r\n`cYou can not continue to fight while a fear of the gods envelopes you.`x");
	
	// do a 'flee' from room 
	for (attempt = 0; attempt < 10; attempt++ )
	{
		found = false;
		
		door = number_door( );
		
		if ( ( pexit = was_in->exit[door] ) == 0
			||   pexit->u1.to_room == NULL
			||   IS_SET(pexit->exit_info, EX_CLOSED)
			||   number_range(0,ch->daze) != 0
			||  (IS_NPC(ch)          
			&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB )))
			continue;
		
		act("`c$n screams and runs out the room in a fit of terror!`x", ch, NULL, NULL, TO_ROOM );
		move_char( ch, door, false );
		
		// keep going till we get out of the room
		if(ch->in_room != was_in )
			continue;
		
		if (!IS_NPC(ch))
			ch->println("`cYou scream and run away in an involuntary fit of terror!`x");
		
		stop_fighting( ch, true ); // incase they were fighting 
		return;
	}
	ch->println("`cYou can't seem to escape!`x");
	return;
}

/************************************************************************/
// for spell_cause_fear and dragonfear - Rathern 
void do_fearful(char_data *ch)
{
    int attempt;
    ROOM_INDEX_DATA *was_in;
    bool found;
    EXIT_DATA *pexit;
    int door;
	
	if IS_OOC(ch)
		return;
	
	// record the room they start in
    was_in = ch->in_room;
	
    switch(ch->position)
    {
	case POS_SLEEPING:
		rand_to_char(ch,
			"`cYou have nightmares about vicious city guards.`x\r\n",
			"`cYou have nightmares about being imprisioned.`x\r\n",
			"`cYou have a nightmare about a cruel oligarchy taking control of your city.`x\r\n",
			"`cYou have nightmares about monsters that are invincible.`x\r\n",
			"`cYou have nightmares about a beautiful woman with snakes for hair.`x\r\n" );
		
		ch->println("`cYou awaken, a powerful fear of the world still vibrant in your mind.`x");
		ch->position = POS_RESTING;
		break;
		
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
		ch->println("You stand up, an inexplicable fear distracting you.");
		ch->position = POS_STANDING;
		
	case POS_FIGHTING:
	case POS_STANDING:
		if (ch->mounted_on!=NULL)
		{
			ch->println("`cYour mount senses your fears and bucks, startled.`x");
			dismount(ch);
		}
		ch->println("`cIs the world really out to get you, or is it your imagination?");
    }
    
    if (ch->position < POS_STANDING)
		return;
	
	rand_to_char(ch,
		"`cWill your taxes keep going up?`x",
		"`cEveryone is out to get you.`x",
		"`cAny food given to you might be poisoned!`x",
		"`cIs that really water?  Is it tainted?`x",
		"`cThe world is so large.  Surely will get lost!.`x");
	
	ch->println("\r\n`cHow can you fight when you are so fearful?\r\nYou flee from combat.`x");
	
	// do a 'flee' from room 
	for (attempt = 0; attempt < 10; attempt++ )
	{
		found = false;
		
		door = number_door( );
		
		if ( ( pexit = was_in->exit[door] ) == 0
			||   pexit->u1.to_room == NULL
			||   IS_SET(pexit->exit_info, EX_CLOSED)
			||   number_range(0,ch->daze) != 0
			||  (IS_NPC(ch)          
			&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB )))
			continue;
		
		act("`c$n screams and runs out the room in terror!`x", ch, NULL, NULL, TO_ROOM );
		move_char( ch, door, false );
		
		// keep going till we get out of the room
		if(ch->in_room != was_in )
			continue;
		
		if (!IS_NPC(ch))
			ch->println("`cYou scream and run away in an involuntary terror!`x");
		
		stop_fighting( ch, true ); // incase they were fighting 
		return;
	}
	ch->println("`cYou can't seem to escape the horror!`x");
	return;
}

/************************************************************************/
void do_animal_train(char_data *ch)  
{
	int chance=number_range(1,100);
	char_data * rider = ch->ridden_by;
   
	if (IS_SET(ch->act, ACT_PET)) 
		chance+=5;

	if (!rider)
	{
		bug("Error in do_animal_train, riderless ch");
		ch->bucking=false;
		return;
	}
   
	if (IS_NPC(rider))
	{
		bug("Error in do_animal_train, NPC rider");
		dismount(ch->ridden_by);
		ch->bucking=false;
		return;
	}
   
	// check to see if rider is bucked
	if ((chance<10)
		|| (chance>=get_skill(rider,gsn_animal_training))
		|| (chance>=get_skill(rider,gsn_riding)))
		ch->bucking = false;

	if (!ch->bucking)
	{
		dismount(rider);
		act("`c$N bucks you off!`x", rider, NULL, ch, TO_CHAR); 
		act("`c$n is bucked off of $N`x", rider, NULL, ch, TO_ROOM);
		check_improve(rider, gsn_animal_training, false, 3);
		rider->position = POS_RESTING;
		if (number_range(1,3)==1)
		{
			rider->println("`cThis is really wearing you out.`x");
			rider->pcdata->tired+=number_fuzzy(3);
		}
		
		// see if mount gets really pissed off
		if (ch->level+5 > rider->level)
			multi_hit(ch, rider, TYPE_UNDEFINED);
	}
	else
	{
		act("`c$N tries to buck you off, but you hold on!`x", rider, NULL, ch, TO_CHAR); 
		act("`c$N tries to buck off $n, but $e holds on!`x", rider, NULL, ch, TO_ROOM);
		if (number_range(1,3)==1)
		{
			rider->println("`cThis is really wearing you out.`x");
			rider->pcdata->tired++;
		}
		chance = 1;
		if (rider->level < ch->level)
			chance = 0;
		else 
			chance += (rider->level - ch->level) / 5;
		if (IS_SET(ch->act, ACT_PET))
			chance+=2;
		
		ch->will -= chance;

		if (ch->will <= 0)
		{
			act("`c$N stops bucking, you have broken $M!`x", rider, NULL, ch, TO_CHAR); 
			act("`c$N stops bucking, $n has broken $M!`x", rider, NULL, ch, TO_ROOM);
			ch->will = 0;
			SET_BIT(ch->act, ACT_DOCILE);
			ch->bucking = false;
		}
		check_improve(rider, gsn_animal_training, true, 3); 
		check_improve(rider, gsn_riding, true, 1);
		return;
	}
	return;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
