/**************************************************************************/
// act_ente.cpp - portal related code
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

// command procedures needed
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_stand         );

/**************************************************************************/
// random room generation procedure
ROOM_INDEX_DATA  *get_random_room(char_data *ch)
{
    ROOM_INDEX_DATA *room;
	
	int count;	// look for up to 1000 rooms -  places like olc have
				//						trouble finding a valid room
    for ( count=0; count<1000; count++)
    {
		room = get_room_index(number_range(0,65535));

		// room must exist
		if (!room)
			continue;
	
		if ( !can_see_room(ch,room)
			|| IS_SET(room->area->area_flags, AREA_NOTELEPORT)
			|| IS_SET(room->area->area_flags, AREA_NOSCRY)
			|| IS_SET(room->area->area_flags, AREA_OLCONLY)
			|| IS_SET(room->room_flags, ROOM_PRIVATE)
			|| IS_SET(room->room_flags, ROOM_SOLITARY) 
			|| IS_SET(room->room_flags, ROOM_SAFE) 
			|| IS_SET(room->room_flags, ROOM_OOC) 
			|| IS_SET(room->room_flags, ROOM_NOSCRY)
			|| IS_SET(room->room_flags, ROOM_NO_RECALL)
			)
			continue;

		// no aggressive mobs in law rooms
		if (IS_NPC(ch) 
			&& IS_SET(ch->act,ACT_AGGRESSIVE)
			&& IS_SET(room->room_flags,ROOM_LAW)
			)
			continue;

		// no mobs in nomob rooms
		if (IS_NPC(ch) 
			&& IS_SET(room->room_flags,ROOM_NO_MOB)
			)
			continue;

		break;
    }
	if(count>=1000){
		bugf("get_random_room had over 1000 attempts at finding a suitable room for '%s', "
			"returning limbo.",ch->name);
		ch->printlnf( "get_random_room had over 1000 attempts at finding a suitable room for '%s', "
			"returning you to limbo, talk or note an imm to fix you up.", ch->name );
		flush_char_outbuffer(ch);
		room = get_room_index(ROOM_VNUM_LIMBO);
		assert(room!=NULL);// if we are going to crash, we might as well do it in a helpful way
	}
    return room;
}

/**************************************************************************/
// RT Enter portals 
void do_enter( char_data *ch, char *argument)
{    
    ROOM_INDEX_DATA *location; 

    if ( ch->fighting != NULL ){
		return;
	}

    // nifty portal stuff 
    if (!IS_NULLSTR(argument))
    {
		ROOM_INDEX_DATA *old_room;
		OBJ_DATA *portal;
		char_data *fch, *fch_next;
		
		old_room = ch->in_room;
		
		portal = get_obj_list( ch, argument,  ch->in_room->contents );
		
		if (portal == NULL)
		{
			ch->println("`cYou don't see that here.`x");
			return;
		}
		
		if (portal->item_type != ITEM_PORTAL 
			||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,ANGEL)))
		{
			ch->println("`cYou can't seem to find a way in.`x");
			return;
		}
		
		if (!IS_TRUSTED(ch,ANGEL) && !IS_SET(portal->value[2],GATE_NOCURSE)
			&&  IS_AFFECTED(ch,AFF_CURSE))
		{
			ch->println("`cSomething prevents you from leaving...`x");
			return;
		}

		// can't use takeable portals in norecall rooms
		if (CAN_WEAR(portal, OBJWEAR_TAKE) && 
			(IS_SET(old_room->room_flags,ROOM_NO_RECALL)
			|| IS_SET( old_room->room_flags, ROOM_ANTIMAGIC) ))
		{
			act("`c$p doesn't appear to be working.`x",	ch,portal,NULL,TO_CHAR); 
			return;
		}

		if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
		{
			location = get_random_room(ch);
			portal->value[3] = location->vnum; // for record keeping :)
		}
		else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
			location = get_random_room(ch);
		else
			location = get_room_index(portal->value[3]);
		
		if (location == NULL
			||  location == old_room
			||  !can_see_room(ch,location) 			
			||  is_room_private_to_char(location, ch))
		{
			act("`c$p doesn't seem to go anywhere.`x",ch,portal,NULL,TO_CHAR);
			return;
		}
		
		if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
			&&  IS_SET(location->room_flags,ROOM_LAW))
		{
			ch->println("`cSomething prevents you from leaving...`x");
			return;
		}
		
		if (ch->mounted_on!=NULL)
		{
			ch->println("`cYour mount cannot fit inside.`x");
			return;
		}
		
		if (ch->ridden_by!=NULL)
		{
			ch->println("`cOnly if your rider wants to go there.`x");
			return;
		}
		
		act("`c$n steps into $p.`x",ch,portal,NULL,TO_ROOM);
		
		if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
			act("You enter $p.`x",ch,portal,NULL,TO_CHAR);
		/*else
			act("You walk through $p and find yourself somewhere else...`x",
			ch,portal,NULL,TO_CHAR);*/
		
		char_from_room(ch);
		char_to_room(ch, location);
		
		if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
		{
			obj_from_room(portal);
			obj_to_room(portal,location);
		}
		
		if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
			act("`c$n has arrived.`x",ch,portal,NULL,TO_ROOM);
		else
			act("`c$n has arrived through $p.`x",ch,portal,NULL,TO_ROOM);
		
		do_look(ch,"auto");
		
		// negative below -1
		if (portal->value[0]<-1)
			portal->value[0]=1;

		// charges 
		if (portal->value[0] > 0)
		{
			portal->value[0]--;
			//			if (portal->value[0] == 0)
			//				portal->value[0] = -1;
		}
		
		// protect against circular follows 
		if (old_room == location)
			return;
		
		for ( fch = old_room->people; fch != NULL; fch = fch_next )
		{
			fch_next = fch->next_in_room;
			
			if (portal == NULL || portal->value[0] == 0) 
				// no following through dead portals 
				continue;
			
			if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
				&&   fch->position < POS_STANDING)
				do_stand(fch,"");
			
			if ( fch->master == ch && fch->position == POS_STANDING)
			{
				
				if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
					&&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
				{
					act("`cYou can't bring $N there.`x",
						ch,NULL,fch,TO_CHAR);
					act("`cYou aren't allowed there.`x",
						fch,NULL,NULL,TO_CHAR);
					continue;
				}
				
				act( "`cYou follow $N.`x", fch, NULL, ch, TO_CHAR );
				do_enter(fch,argument);
			}
		}
		
		if (portal != NULL && portal->value[0] == 0)
		{
			act("`c$p fades out of existence.`x",ch,portal,NULL,TO_CHAR);
			if (ch->in_room == old_room)
				act("`c$p fades out of existence.`x",ch,portal,NULL,TO_ROOM);
			else if (old_room->people != NULL)
			{
				act("`c$p fades out of existence.`x", 
					old_room->people,portal,NULL,TO_CHAR);
				act("`c$p fades out of existence.`x",
					old_room->people,portal,NULL,TO_ROOM);
			}
			extract_obj(portal);
			
		}
		/* 
		* If someone is following the char, these triggers get activated
		* for the followers before the char, but it's safer this way...
		*/
		if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
			mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
		if ( !IS_NPC( ch ) )
			mp_greet_trigger( ch );
		
		return;
    }

    ch->println("`cNope, can't do it.`x");
    return;
}
/**************************************************************************/
