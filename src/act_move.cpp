/**************************************************************************/
// act_move.cpp - handles moving players between rooms etc
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
#include "track.h"
#include "msp.h"
#include "lockers.h"
#include "prototyp.h" //arena_broadcast use in recall 

// command procedures needed
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_recall	);
DECLARE_DO_FUN( do_stand	);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN( do_map		); // Kal
DECLARE_DO_FUN( do_dmap		); // Kal
void landchar( char_data *ch);
bool trapcheck_open( char_data *ch, OBJ_DATA *obj );
bool trapcheck_move( char_data *ch, int door );
void arena_broadcast(char_data *ch, char * fmt, ...); //used in recall 
/**************************************************************************/
char *const	dir_shortname[]= {"N","E","S","W","U","D","Ne","Se","Sw","Nw"};

char *const dir_name[]=
{
    "north", "east", "south", "west", "up", "down", "northeast", "southeast",
    "southwest", "northwest"
};

const   sh_int  rev_dir         []              =
{
    2, 3, 0, 1, 5, 4, 8, 9, 6, 7
};

const   sh_int  movement_loss   [SECT_MAX]      =
{
    1, 2, 2, 3, 4, 6, 4, 7, 10, 10, 6, 4, 6
};

/**************************************************************************/
/*
 * Local functions.
 */
int     find_door       args( ( char_data *ch, char *arg, char *action ));// action = NULL/lock/unlock/open/close etc
bool    has_key         args( ( char_data *ch, int key ) );
bool    has_locker_key  args( ( char_data *ch) );
void    mount           args( ( char_data *, char_data *) );
void    dismount        args( ( char_data *) );
void    tame_a_little   args( (char_data * ch, char_data * victim));     

/**************************************************************************/
void move_char( char_data *ch, int door, bool)
{
	char_data *fch;
	char_data *fch_next;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room = NULL;
	EXIT_DATA *pexit;
	OBJ_DATA  *obj;

	if ( door < 0 || door >=MAX_DIR )
	{
		bugf( "do_move(): bad door number %d.", door );
		return;
	}

    // Exit trigger, if activated, bail out. Only PCs are triggered.
	if ( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
		return;

	in_room = ch->in_room;

	if ( ( pexit   = in_room->exit[door] ) == NULL
		||   ( to_room = pexit->u1.to_room   ) == NULL 
		||   !can_see_room(ch,pexit->u1.to_room))
    {
		ch->println( "`cAlas, you cannot go that way.`x" );
		return;
    }

	if (IS_SET(pexit->exit_info, EX_CLOSED)
		&&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
		&&   !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
    {
		ch->printlnf("`cThe %s is closed.`x", IS_NULLSTR(pexit->keyword)?"door":pexit->keyword);
		return;
    }

	if ( trapcheck_move( ch, door )){
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM)
	&&   ch->master != NULL
	&&   in_room == ch->master->in_room )
	{
		ch->println( "`cWhat?  And leave your beloved master?`x" );
		return;
    }

	if ( is_affected( ch, gsn_cause_fear )
	&& IS_SET(to_room->room_flags, ROOM_OOC))
	{
		ch->println( "`cYou can't enter OOC while under the affects of cause fear.`x" );
		return;
	}

	if ( is_affected( ch, gsn_fear_magic )
	&& IS_SET(to_room->room_flags, ROOM_OOC))
	{
		ch->println( "`cYou can't enter OOC while under the affects of fear magic.`x" );
		return;
	}

	if (is_room_private_to_char( to_room, ch ))
    {
		if ( !IS_IMMORTAL( ch ))
		{
			if(!IS_NULLSTR(to_room->owner)){
				if(!is_room_owner(ch, to_room)
					&& !player_on_rooms_invite_list(ch, to_room))
				{
					ch->println( "`cThat room is owned by someone, and you are not on the room invitation list.`x" );
					return;
				}
			}
			ch->println( "`cThat room is private right now.`x" );
			return;
		}
		else if ( IS_SET( to_room->room_flags, ROOM_OOC )
			|| INVIS_LEVEL(ch)< LEVEL_IMMORTAL )
		{
			ch->println( "`cThat room is private right now.`x" );
			return;
		}
    }

	if (( IS_SET( to_room->room_flags, ROOM_NOFLY ))
		&& IS_AFFECTED( ch, AFF_FLYING ))
	{
		ch->println( "`cThe winds are too strong, you must be on the ground to get there.`x" );
		return;
	}

	if (ch->ridden_by){
		ch->println("`cYou must go where your rider leads you.`x");
		return;
    }
   
	if (ch->tethered){
		ch->println("`cYou cannot move while tethered.`x");
		return;
    }

    // premove trigger, activated only on mobs with the trigger 
	// if mp_premove_trigger() returns false, movement was canceled
	if ( IS_NPC(ch) 
		&& HAS_TRIGGER(ch, TRIG_PREMOVE) 
		&& mp_premove_trigger( ch, to_room->vnum, door ) )
	{
		ch->println("`cMovement changed/canceled by premove mobprog trigger.`x");
		return;
	}
	
	// basic mount checks - dont move ch's yet
	if (ch->mounted_on){
		if IS_SET(to_room->room_flags, ROOM_OOC){
			ch->println( "`cYou cannot bring mounts into OOC rooms.`x" );
			return;
		}

		// No Mounts UNDERWATER unless they are Otterlunged
		if (( to_room->sector_type==SECT_UNDERWATER 
			|| IS_SET( to_room->affected_by, ROOMAFF_UNDERWATER ))
			&& !IS_AFFECTED( ch->mounted_on, AFF_OTTERLUNGS ))
		{
			ch->println( "`cYour mount cannot travel underwater.`x" );
			return;
		}

		// lava check - darksun/daos
		if(to_room->sector_type==SECT_LAVA
			&& !IS_AFFECTED( ch->mounted_on, AFF_FLYING))
		{
			ch->println( "`cYour mount cannot travel on lava!!!`x");
			return;
		}

		if (IS_SET(to_room->room_flags,ROOM_LAW)
			&&  (IS_NPC(ch->mounted_on) 
			&& IS_SET(ch->mounted_on->act,ACT_AGGRESSIVE)))
		{
			act("`cYour aggressive mount $N refuses to ride there.`x",
				ch,NULL,ch->mounted_on,TO_CHAR);
			act("`cYou aren't allowed there.`x",
				ch->mounted_on,NULL,NULL,TO_CHAR);
			return;
		}
	
		if IS_SET(to_room->room_flags, ROOM_INDOORS){
			ch->println("`cYou cannot bring mounts inside.`x");
			ch->println("`cYou will either have to `Rdismount`c or `Rtether`c your mount.`x");
			return;
		}

		if (ch->mounted_on->bucking){
			ch->println("`cYour mount is out of your control!`x");
			return;
		}
    }
    
	// do all checks that only affect pc's 
	// and then movement calculations
    if ( !IS_NPC(ch)){
		int move;
/*		// the guild code is performed by mobprogs now
		int iGuild;
		
		if (IS_SET(to_room->room_flags, ROOM_CLASS_RESTRICTION) 
			&& !IS_SET(ch->act,PLR_HOLYWALK)
			&& !IS_IN_REDIT(ch))
		{
			bool NOT_ALLOWED= true;
			
			for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++) 
			{
				if (class_table[ch->clss].guild[iGuild] == to_room->vnum)
				{
					NOT_ALLOWED=false;
				}
			}
			
			if (NOT_ALLOWED)
			{
				ch->println("`cYou can't go in there, tis not your guild.`x");
				return;
			}
			
		}
*/
		// UNDERWATER checking
		if ( to_room->sector_type==SECT_UNDERWATER 
			|| IS_SET( to_room->affected_by, ROOMAFF_UNDERWATER ))
		{
			OBJ_DATA *obj2;

		    obj2 = ( get_eq_char( ch, WEAR_LIGHT ));

			if(!IS_AFFECTED( ch, AFF_OTTERLUNGS ) 
				&& !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
			{
				if ( !obj2 || (obj2 && !IS_SET(obj2->extra_flags, OBJEXTRA_OTTERLUNGS )))
				{
					ch->println( "`cYou would drown there.`x" );
					return;
				}
			}
		}


		// Lava check - Daos/darksun
		if(	to_room->sector_type==SECT_LAVA
			&& !IS_AFFECTED(ch, AFF_FLYING)
			&& !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
		{
			ch->println("`cYou would burn there.`x");
			return;
		}
		// can't go into ooc rooms with a noquit timer on
		if (IS_SET(to_room->room_flags, ROOM_OOC) 
			&& !IS_OOC(ch) && ch->pknoquit>0)
		{
			ch->println( "`cYou cannot enter an ooc room while you have a pknoquit timer.`x" );
			return;
		}


		if (!ch->mounted_on)
		{			
			if (in_room->sector_type == SECT_AIR 
				|| to_room->sector_type == SECT_AIR
				|| in_room->sector_type == SECT_LAVA
				|| to_room->sector_type == SECT_LAVA)
			{
				if ( !IS_AFFECTED(ch, AFF_FLYING) 
					&& !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK)){
					ch->println( "`cYou can't fly.`x" );
					return;
				}
			}
			
			if (( in_room->sector_type == SECT_WATER_NOSWIM
				||to_room->sector_type == SECT_WATER_NOSWIM )
				&& !IS_AFFECTED(ch,AFF_FLYING))
			{
				OBJ_DATA *obj;
				bool found;
				
				// Look for a boat
				found = false;
				if (IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
					found = true;
				
				for ( obj = ch->carrying; obj; obj = obj->next_content ){
					if ( obj->item_type == ITEM_BOAT )
					{
						found = true;
						break;
					}
				}
				if ( !found ){
					ch->println( "`cYou need a boat to go there.`x" );
					return;
				}
			} 
		}else{ // mounted movement
			if (in_room->sector_type == SECT_AIR 
				|| to_room->sector_type == SECT_AIR
				|| in_room->sector_type == SECT_LAVA
				|| to_room->sector_type == SECT_LAVA)
			{
				if ( !IS_AFFECTED(ch->mounted_on, AFF_FLYING))
				{
					ch->println( "`cYour mount can't fly.`x" );
					return;
				}
			}
			
			if (( in_room->sector_type == SECT_WATER_NOSWIM
				|| to_room->sector_type == SECT_WATER_NOSWIM )
				&&   !IS_AFFECTED(ch->mounted_on,AFF_FLYING))
			{
				ch->println( "`cYour mount cannot swim.`x" );
				return;
			}
		} // endif for is mounted? 
		
		// get the average for moving between sector types
		move = (movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
			+ movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)])/2;

		// No movement loss in OOC
		if ( IS_OOC(ch)){
			move = 0;
		}
				
		// conditional effects
		if (!ch->mounted_on){
			if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
				move /= 2;
			
			if (IS_AFFECTED(ch,AFF_SLOW))
				move *= 2;
			
			if (IS_AFFECTED(ch, AFF_SNEAK))
				move = (move* number_range(15,35))/10;

			//Y: Walking is painful if you have a headache
			if (is_affected( ch, gsn_cause_headache ))
				move += (move/2);

			/*if ( ch->move < move && !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
			{
				ch->println("`cYou are too exhausted.`x");
				return;
			}
			
			if (!IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK) 
				&& !IS_IN_REDIT(ch))
				ch->move -= move;
		}else{

			if (IS_AFFECTED(ch->mounted_on,AFF_FLYING) 
				|| IS_AFFECTED(ch->mounted_on,AFF_HASTE))
				move /= 2;
			
			if (IS_AFFECTED(ch->mounted_on,AFF_SLOW))
				move *= 2;

			if (IS_AFFECTED(ch->mounted_on, AFF_SNEAK))
				move = (move* number_range(15,35))/10;
			
			if ( ch->mounted_on->move < move )
			{
				ch->println("`cYour mount is too exhausted.`x");
				return;
			}
			
			ch->mounted_on->move -= move;*/
		}
	} // endif (!IS_NPC(ch))
    
	// Check if the character is Treeformed and if so, strip the spell
	if ( IS_SET( ch->affected_by2, AFF2_TREEFORM ))
	{
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;

		REMOVE_BIT( ch->affected_by2, AFF2_TREEFORM );
		for (paf = ch->affected; paf != NULL; paf = paf_next)
		{
			paf_next = paf->next;
			if ( paf->type == gsn_treeform )
			{
				affect_remove( ch, paf );
				ch->println("`cYou assume your original shape.`x");
				act( "`cA tree suddenly transforms into $n.`x", ch, NULL, NULL, TO_ROOM );
				break;
			}
		}
	}

    // OOC/IC room system
	if ( (IS_SET(in_room->room_flags, ROOM_OOC) != IS_SET(to_room->room_flags, ROOM_OOC))
		  && (!IS_SET(ch->comm, COMM_BUILDING)			// dont use system on builders
		  && !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK)))	// or those with holywalk on		
	{
		// check for going into ooc from ic
		if (IS_SET(to_room->room_flags, ROOM_OOC))
		{
			ch->last_ic_room = in_room;
			if ( !IS_AFFECTED(ch, AFF_SNEAK) && INVIS_LEVEL(ch)<=LEVEL_HERO)
				act( "$n leaves $T [ENTERS AN OOC ROOM].", ch, NULL, dir_name[door], TO_ROOM );
		}
		else // leaving ooc rooms
		{
			if (ch->last_ic_room)
			{
				to_room = ch->last_ic_room;
			}
			ch->last_ic_room= NULL;
			act( "`c$n leaves $T [LEAVES OOC AREA].`x", ch, NULL, dir_name[door], TO_ROOM );
		}
	}else{
		if (ch->mounted_on)
		{
			act( "`c$n rides $T.`x", ch, NULL, dir_name[door], TO_ROOM );  
		}
		else if (IS_AFFECTED(ch, AFF_SNEAK) && IS_IC(ch)) // no sneaking in ooc
		{
			int old_invis_level;
			old_invis_level=ch->invis_level;
			// hide them to mortals
			ch->invis_level = UMAX(ch->invis_level, LEVEL_IMMORTAL);
			// inform the imms in the room someone snuck in
			act( "`c$n leaves $T. [SNEAKING]`x", ch, NULL, dir_name[door], TO_ROOM );
			// restore their wizi
			ch->invis_level=old_invis_level;
		}
		else
		{
			if ( !IS_SET(ch->act, ACT_IS_UNSEEN ))
			{
				act( "`c$n leaves $T.`x", ch, NULL, dir_name[door], TO_ROOM );
			}
		}
	}

	// take damage from lodged objects
	obj=get_eq_char( ch, WEAR_LODGED_LEG );
	if(obj){
		act( "`c$p twists in your leg as you walk, hurting you greatly.`x", ch, obj, NULL, TO_CHAR );
		ch->hit  -= obj->level / 15;
		ch->move -= obj->level / 8;
	}
	obj=get_eq_char( ch, WEAR_LODGED_RIB );
	if(obj){
		act( "`c$p twists in your ribs as you walk, hurting you greatly.`x", ch, obj, NULL, TO_CHAR );
		ch->hit  -= obj->level / 12;
	}    

	// make the tracks
	ch->in_room->tracks->add_track(ch, door, TRACKTYPE_MOVE);

    //move the character from room to new room 
    char_from_room( ch );
    char_to_room( ch, to_room );

	if (ch->mounted_on){
		char_from_room(ch->mounted_on);
		char_to_room(ch->mounted_on, to_room);
		act( "`c$n has arrived on $N.`x", ch, NULL, ch->mounted_on, TO_ROOM );
    }else{
		if (IS_AFFECTED(ch, AFF_SNEAK)){
			int old_invis_level;
			// back up the wizi level
			old_invis_level= ch->invis_level;
			// hide them to mortals
			ch->invis_level = UMAX(ch->invis_level, LEVEL_IMMORTAL);
			// inform the imms in the room someone snuck in
			act( "$n arrives. [SNEAKING]", ch, NULL, dir_name[door], TO_ROOM );
			// restore their wizi
			ch->invis_level=old_invis_level;
		}else{
			act( "`c$n has arrived.`x", ch, NULL, NULL, TO_ROOM );
		}
    }
	do_look( ch, "auto" );	
    
	// check PC's falling off their mount
    if (ch->mounted_on && !IS_NPC(ch))
    {
		if ((get_skill(ch, gsn_riding)==0)&&(number_range(1, 100)<=3))
		{
			ch->println("`cYou fall off your mount!`x");
			ch->println("`cYou should really learn to ride.`x");
			act( "`c$n falls off of $N, how embarrassing.`x", ch, NULL, ch->mounted_on, TO_ROOM );
			ch->position=POS_RESTING;
			dismount(ch);
		}
		if (get_skill(ch, gsn_riding)<75){
			check_improve(ch,gsn_riding,true,2);   
		}		
	}
    
    
    if(!to_room){ // no following into NULL 
		return;
	}

    if (in_room == to_room){ // no circular follows
		return;
	}

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
		fch_next = fch->next_in_room;
		
		if ( !IS_NPC(fch) )
		{
			if( fch->pcdata->is_trying_aware && !IS_IMMORTAL(ch) && 
				number_percent()<get_skill(fch, gsn_awareness) &&
				fch->position==POS_SLEEPING &&
				!IS_AFFECTED(fch,AFF_SLEEP))
			{
				fch->position=POS_RESTING;
				fch->println("`cYou are awakened suddenly by a presence.`x");
				do_stand(fch,"");
				check_improve(fch,gsn_awareness,true,14);
			}
		}
		
		if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
			&&   fch->position < POS_STANDING)
			do_stand(fch,"");
		
		if ( fch->master == ch && fch->position == POS_STANDING
			&&   can_see_room(fch,to_room))
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
			
			{
				char fbuf[MIL];
				sprintf(fbuf,"`cYou follow $N `Y%s`x.`x", dir_name[door]);
				act( fbuf, fch, NULL, ch, TO_CHAR );
			}
			move_char( fch, door, true );
		}
    }

    // If someone is following the char, these triggers get activated
    // for the followers before the char, but it's safer this way...
    if( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) ){
		mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
	}

    if( !IS_NPC( ch ) ){
		mp_greet_trigger( ch );
	}
    return;
}

/**************************************************************************/
void do_northeast( char_data *ch, char *)
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_NORTHEAST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_southeast( char_data *ch, char *)
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_SOUTHEAST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_southwest( char_data *ch, char *)
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_SOUTHWEST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_northwest( char_data *ch, char *)
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_NORTHWEST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );
		
		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_north( char_data *ch, char *)
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_NORTH, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_east( char_data *ch, char * )
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_EAST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_south( char_data *ch, char * )
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_SOUTH, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_west( char_data *ch, char * )
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_WEST, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_up( char_data *ch, char * )
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_UP, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}
/**************************************************************************/
void do_down( char_data *ch, char * )
{
	ROOM_INDEX_DATA *was_room = ch->in_room;

    move_char( ch, DIR_DOWN, false );

	if ( was_room == ch->in_room )
		free_speedwalk( ch->desc );

		if(!IS_IMMORTAL(ch))
		{WAIT_STATE(ch, 3);}

    return;
}


/**************************************************************************/
int find_door( char_data *ch, char *arg, char *action )// action = NULL/lock/unlock/open/close etc
{
    EXIT_DATA *pexit;
    int door;
	
	if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 7;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 8;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 9; 
    else
    {
		for ( door = 0; door < MAX_DIR; door++ )
		{
			if ( ( pexit = ch->in_room->exit[door] ) != NULL
				&&   pexit->exit_info>0
				&&   pexit->keyword != NULL
				&&   is_name( arg, pexit->keyword ) )
				return door;
		}
		ch->printlnf("`cI see no %s here to %s.`x", arg, action);
		return -1;
    }
	
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
		if(IS_NULLSTR(action)){
			ch->printlnf( "`cI see no door %s.`x", arg);
		}else{
			ch->printlnf( "`cI see no door %s here to %s.`x", arg, action);
		}
		return -1;
    }
	
    if ( pexit->exit_info==0 )
    {
		ch->printlnf( "`cYou can't %s the %s exit.`x", action, dir_name[door]);
		return -1;
    }
	
    return door;
}

/**************************************************************************/
void do_unlock( char_data *ch, char *argument );
/**************************************************************************/
void do_open( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;
	
    one_argument( argument, arg );
	
    if(IS_NULLSTR(arg)){
		ch->println("`cOpen what?`x");
		return;
    }
	
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
		// open portal 
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1], EX_ISDOOR))
			{
				act( "`cYou can't open $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (!IS_SET(obj->value[1], EX_CLOSED))
			{
				act( "`c$p is already open.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (IS_SET(obj->value[1], EX_LOCKED))
			{
				act( "`c$p is locked.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			REMOVE_BIT(obj->value[1], EX_CLOSED);
			act("`cYou open $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n opens $p.`x",ch,obj,NULL,TO_ROOM);
			msp_to_room(MSPT_ACTION, MSP_SOUND_OPEN_DOOR, 0, ch, false, true);
			return;
		}
		
		// 'open object' 
		if ( obj->item_type != ITEM_CONTAINER){ 
			act( "`c$p is not a container.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSED) ){ 
			act( "`c$p is already open.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) ){ 
			act( "`c$p can't be opened.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}

		// traps
		if (trapcheck_open( ch, obj )){
			return;
		}

		// attempt to automatically unlock it
		if(IS_SET(obj->value[1], CONT_LOCKED)){
			do_unlock(ch, argument);
		}
		if ( IS_SET(obj->value[1], CONT_LOCKED) ){ 
			act( "`c$p is locked.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		
		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act("`cYou open $p.`x",ch,obj,NULL,TO_CHAR);
		act( "`c$n opens $p.`x", ch, obj, NULL, TO_ROOM );
		return;
    }
	
    if ( ( door = find_door( ch, arg, "open" ) ) >= 0 )
    {
		// 'open door' 
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev= NULL;
		
		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{ ch->println("`cIt's already open.`x"); return; }
		if (  IS_SET(pexit->exit_info, EX_LOCKED) )
		{ ch->println("`cIt's locked.`x");		return; }
		
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act( "`c$n opens the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );
		msp_to_room(MSPT_ACTION, MSP_SOUND_OPEN_DOOR, 0, ch, false, true);
		ch->println("`cYou have done so successfully`x");

		{ 
			char_data *rch;

			// update peoples mappers
			for ( rch = ch->in_room->people; rch; rch = rch->next_in_room ){		
				if (!IS_NPC(rch) && IS_AWAKE(rch)){
					if(IS_SET(rch->act,PLR_AUTOMAP)){
						do_map(rch,"");
					}
    			}
			}

			// open the other side
			if ( ( to_room   = pexit->u1.to_room            ) != NULL
				&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
				&&   pexit_rev->u1.to_room == ch->in_room )
			{
		
				REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
				for ( rch = to_room->people; rch; rch = rch->next_in_room ){
					act( "`cThe $d opens.`x", rch, NULL, pexit_rev->keyword, TO_CHAR );				
					if (!IS_NPC(rch) && IS_AWAKE(rch)){
						if(IS_SET(rch->act,PLR_AUTOMAP)){
							do_map(rch,"");
						}
    				}
				}
			}
		}
    }
    return;
}



/**************************************************************************/
void do_close( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;
	
	one_argument( argument, arg );
	
	if(IS_NULLSTR(arg)){
		ch->println("`cClose what?`x");
		return;
	}
	
	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		// portal stuff 
		if (obj->item_type == ITEM_PORTAL)
		{
			
			if (!IS_SET(obj->value[1],EX_ISDOOR)
				||   IS_SET(obj->value[1],EX_NOCLOSE))
			{
				act( "`cYou can't close $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (IS_SET(obj->value[1],EX_CLOSED))
			{
				act( "`c$p is already closed.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			SET_BIT(obj->value[1],EX_CLOSED);
			act("`cYou close $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n closes $p.`x",ch,obj,NULL,TO_ROOM);
			return;
		}
		
		// 'close object' 
		if ( obj->item_type != ITEM_CONTAINER ){ 
			act( "`c$p is not a container.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( IS_SET(obj->value[1], CONT_CLOSED) ){ 
			act( "`c$p is already closed.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) ){ 
			act( "`c$p can't be closed.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		
		SET_BIT(obj->value[1], CONT_CLOSED);
		act("`cYou close $p.`x",ch,obj,NULL,TO_CHAR);
		act( "`c$n closes $p.`x", ch, obj, NULL, TO_ROOM );
		return;
	}
	
	if ( ( door = find_door( ch, arg, "close" ) ) >= 0 )
	{
		// 'close door' 
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev= NULL;
		
		pexit   = ch->in_room->exit[door];
		if ( IS_SET(pexit->exit_info, EX_CLOSED) ){ 
			ch->println("`cIt's already closed.`x"); 
			return; 
		}
		if(IS_SET(pexit->exit_info, EX_NOCLOSE) )
		{ ch->println("`cThat door may not be closed once opened.`x"); return; }
		
		SET_BIT(pexit->exit_info, EX_CLOSED);
		act( "`c$n closes the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );
		act( "`cYou close the $d.`x", ch, NULL, pexit->keyword, TO_CHAR );

		msp_to_room(MSPT_ACTION, MSP_SOUND_CLOSE_DOOR, 0, ch, false, true);
		{
			char_data *rch;

			// update peoples mappers
			for ( rch = ch->in_room->people; rch; rch = rch->next_in_room ){		
				if (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOMAP)){
					do_map(rch,"");
    			}
			}
			
			// close the other side
			if ( ( to_room   = pexit->u1.to_room            ) != NULL
				&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
				&&   pexit_rev->u1.to_room == ch->in_room )
			{
				char_data *rch;
				
				SET_BIT(pexit_rev->exit_info, EX_CLOSED);
				for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
				{
					act( "`cThe $d closes.`x", rch, NULL, pexit_rev->keyword, TO_CHAR );
					if (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOMAP)){
						do_map(rch,"");
    				}
				}
			}
		}
    }
	
    return;
}


/**************************************************************************/
bool has_key( char_data *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return true;
    }

    return false;
}


/**************************************************************************/
void do_lock( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
	OBJ_INDEX_DATA *key;
    int door;
	
    one_argument( argument, arg );
	
	if(IS_NULLSTR(arg)){
		ch->println("`cLock what?`x");
		return;
	}
	
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
		// portal stuff 
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR)
				||  IS_SET(obj->value[1],EX_NOCLOSE))
			{
				act( "`cYou can't lock $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}

			if (!IS_SET(obj->value[1],EX_CLOSED)){
				act( "`cTry closing $p first.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
			{
				act( "`c$p can't be locked.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (!has_key(ch,obj->value[4]))// locking, portal key
			{
				act( "`cYou lack the key to lock $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (IS_SET(obj->value[1],EX_LOCKED))
			{
				act( "`c$p is already locked.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			SET_BIT(obj->value[1],EX_LOCKED);

			// check if key exists (should be 100% at this point but better to be safe)
			if (( key = get_obj_index( obj->value[4])) == NULL )
			{
				ch->printlnf( "do_lock: vnum %d not found, report this to admin.", obj->value[4] );
				act("`cYou lock $p.`x", ch, obj, NULL, TO_CHAR );
			}
			else
			{
				act("`cYou lock $p with $T.`x", ch, obj, key->short_descr, TO_CHAR );
			}
			act("`c$n locks $p.`x",ch,obj,NULL,TO_ROOM);
			msp_to_room(MSPT_ACTION, MSP_SOUND_LOCK, 0, ch, false, true);
			return;
		}
		
		// 'lock object' 
		if ( obj->item_type != ITEM_CONTAINER ){ 
			act( "`c$p is not a container.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		// attempt to automatically close it
		if (!IS_SET(obj->value[1], CONT_CLOSED)){
			do_close(ch, argument);
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSED) ){ 
			act( "`c$p is not closed.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( obj->value[2] < 0 ){ 
			act( "`c$p can't be locked.`x", ch, obj, NULL, TO_CHAR );
			return;
		}
		if ( IS_SET(obj->value[1], CONT_LOCKED) ){
			act( "`c$p is already locked.`x", ch, obj, NULL, TO_CHAR );
			return;
		}
		// check for the 'key'
		if ( IS_SET(obj->value[1], CONT_LOCKER) ){
			if ( !lockers->has_access( ch, obj) ) // locking, locker object key
			{ ch->println("`cYou don't have access to lock this locker.`x"); return; }		
		}else{
			if ( !has_key( ch, obj->value[2] ) ){ // locking, object key
				act( "`cYou lack the key to $p.`x", ch, obj, NULL, TO_CHAR );
				return; 
			}
		}
		
		SET_BIT(obj->value[1], CONT_LOCKED);
		msp_to_room(MSPT_ACTION, MSP_SOUND_LOCK, 0, ch, false, true);

		if (( key = get_obj_index( obj->value[2])) != NULL ){
			act("`cYou lock $p with $T.`x", ch, obj, key->short_descr, TO_CHAR );
		}else{
			act("`cYou lock $p.`x", ch, obj, NULL, TO_CHAR );
		}
		act( "`c$n locks $p.`x", ch, obj, NULL, TO_ROOM );
		return;
    }
	
    if ( ( door = find_door( ch, arg, "lock" ) ) >= 0 )
    {
		// 'lock door' 
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev=NULL;
		
		pexit   = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{ ch->println("`cIt's not closed.`x");		return; }
		if ( pexit->key < 0 || IS_SET(pexit->exit_info, EX_NOLOCK) )
		{ ch->println("`cIt can't be locked.`x");	return; }
		if ( !has_key( ch, pexit->key) ) // locking, door key
		{ ch->println("`cYou lack the key.`x");		return; }
		if ( IS_SET(pexit->exit_info, EX_LOCKED) )
		{ ch->println("`cIt's already locked.`x");	return; }
		
		SET_BIT(pexit->exit_info, EX_LOCKED);

		if (( key = get_obj_index( pexit->key )) == NULL )
		{
			ch->printlnf( "`cdo_lock: vnum %d not found, report this to admin.`x", pexit->key );
			act("`cYou lock $p.`x", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			act("`c*CLICK* You've locked the $t with $T.`x", ch, pexit->keyword, key->short_descr, TO_CHAR );
		}
		act( "`c$n locks the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );
		
		msp_to_room(MSPT_ACTION, MSP_SOUND_LOCK, 0, ch, false, true);
		
		// lock the other side 
		if ( ( to_room   = pexit->u1.to_room            ) != NULL
			&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
			&&   pexit_rev->u1.to_room == ch->in_room )
		{
			SET_BIT(pexit_rev->exit_info, EX_LOCKED);
		}
	}
	
	return;
}


/**************************************************************************/
void do_unlock( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
	OBJ_INDEX_DATA *key;
    int door;
	
    one_argument( argument, arg );
	
    if(IS_NULLSTR(arg)){
		ch->println("`cUnlock what?`x");
		return;
    }
	
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
		// portal stuff
		if (obj->item_type == ITEM_PORTAL)
		{
			if (IS_SET(obj->value[1],EX_ISDOOR))
			{
				act( "`cYou can't unlock $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (!IS_SET(obj->value[1],EX_CLOSED))
			{
				act( "`c$p isn't closed.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (obj->value[4] < 0)
			{
				act( "`c$p can't be unlocked.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (!has_key(ch,obj->value[4])) // unlocking, portal key
			{
				act( "`cYou lack the key to unlock $p.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			if (!IS_SET(obj->value[1],EX_LOCKED))
			{
				act( "`c$p is already unlocked.`x", ch, obj, NULL, TO_CHAR );
				return;
			}
			
			REMOVE_BIT(obj->value[1],EX_LOCKED);
//			msp_to_room(MSPT_ACTION, MSP_SOUND_UNLOCK, 0, ch, false, true);
	
			if (( key = get_obj_index( obj->value[4])) != NULL ){
				act("`cYou unlock $p with $T.`x", ch, obj, key->short_descr, TO_CHAR );
			}else{
				act("`cYou unlock $p.`x", ch, obj, NULL, TO_CHAR );
			}
			act("`c$n unlocks $p.`x", ch, obj, NULL, TO_ROOM );
			return;
		}

		// 'unlock object'
		if ( obj->item_type != ITEM_CONTAINER ){ 
			act( "`c$p is not a container.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSED) ){ 
			act( "`c$p is not closed.`x", ch, obj, NULL, TO_CHAR );
			ch->println( "`cIt's not closed.`x" );		 
			return; 
		}
		if ( obj->value[2] < 0 ){ 
			act( "`c$p can't be unlocked.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		if ( !IS_SET(obj->value[1], CONT_LOCKED) ){ 
			act( "`c$p is already unlocked.`x", ch, obj, NULL, TO_CHAR );
			return; 
		}
		// check for the 'key'
		if ( IS_SET(obj->value[1], CONT_LOCKER) ){
			if ( !lockers->has_access( ch, obj) ) // unlocking, locker object key
			{ ch->println( "`cYou don't have access to unlock this locker.`x" ); return; }		
		}else{
			if ( !has_key( ch, obj->value[2] ) ){ // unlocking, object key
				act( "`cYou lack the key to unlock $p.`x", ch, obj, NULL, TO_CHAR );
				return; 
			}
		}
		
		REMOVE_BIT(obj->value[1], CONT_LOCKED);
//		msp_to_room(MSPT_ACTION, MSP_SOUND_UNLOCK, 0, ch, false, true);

		if (( key = get_obj_index( obj->value[2])) != NULL ){
			act("`cYou unlock $p with $T.`x", ch, obj, key->short_descr, TO_CHAR );
		}else{
			act("`cYou unlock $p.`x", ch, obj, NULL, TO_CHAR );
		}
		act("`c$n unlocks $p.`x", ch, obj, NULL, TO_ROOM );
		return;
    }
	
    if ( ( door = find_door( ch, arg, "unlock" ) ) >= 0 )
    {
		// 'unlock door'
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev=NULL;
		
		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{ ch->println("`cIt's not closed.`x");		return; }
		if ( pexit->key < 0 )
		{ ch->println("`cIt can't be unlocked.`x");	return; }
		if ( !has_key( ch, pexit->key) ) // unlocking, door key
		{ ch->println("`cYou lack the key.`x");		return; }
		if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
		{ ch->println("`cIt's already unlocked.`x");	return; }
		
		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
//		msp_to_room(MSPT_ACTION, MSP_SOUND_UNLOCK, 0, ch, false, true);

		// check if key exists (should be 100% at this point but better to be safe)
		if (( key = get_obj_index( pexit->key )) == NULL )
		{
			ch->printlnf( "`cdo_unlock: vnum %d not found, report this to admin.`x", pexit->key );
			act("`cYou unlock $p.`x", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			act("`c*CLICK* You've unlocked the $t with $T.`x", ch, pexit->keyword, key->short_descr, TO_CHAR );
		msp_to_room(MSPT_ACTION, MSP_SOUND_UNLOCK, 0, ch, false, true);
		}
		act( "`c$n unlocks the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );
		
		// unlock the other side 
		if ( ( to_room   = pexit->u1.to_room            ) != NULL
			&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
			&&   pexit_rev->u1.to_room == ch->in_room )
		{
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
		}
	}
	
    return;
}


/**************************************************************************/
void do_pick( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *gch;
	OBJ_DATA *obj;
    int door;
	
    one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println("`cPick what?`x");
		return;
	}

    if (get_skill(ch,gsn_pick_lock)==0)
    {
		if (number_range(1, 10)==1)
		{
			ch->println("`cYou pick your nose when you think no one is looking.`x");
			act("`c$n picks $s nose discretely.`x", ch, NULL, NULL, TO_ROOM);
			return;
		}
		ch->println( "`cYou failed.`x" );
		return;
    }

	if(ch->fighting){
		ch->println( "`cYou failed.`x" );
		return;
	}
	
	// look for guards 
	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	{
		if ( IS_NPC(gch) && !IS_SET(gch->act, ACT_IS_UNSEEN) && 
			IS_AWAKE(gch) && ch->level + 5 < gch->level )
		{
			act( "`c$N is standing too close to the lock.`x",
				ch, NULL, gch, TO_CHAR );
			return;
		}
	}

	// can't pick locks by spamming
	if (ch->desc && ch->desc->repeat>15)
	{
		WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
		ch->println("`cYou failed.`x");
		return;
	}
	
	
	if ( number_percent( ) > get_skill(ch,gsn_pick_lock)+10 || IS_NPC(ch))
	{
		WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
		ch->println("`cYou failed.`x");
		check_improve(ch,gsn_pick_lock,false,2);
		return;
	}
	
	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		// portal stuff 
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR))
			{   
				ch->println("`cYou can't do that.`x");
				return;
			}
			
			if (!IS_SET(obj->value[1],EX_CLOSED))
			{
				ch->println("`cIt's not closed.`x");
				return;
			}
			
			if (obj->value[4] < 0)
			{
				ch->println("`cIt can't be unlocked.`x");
				return;
			}
			
			if (IS_SET(obj->value[1],EX_PICKPROOF))
			{
				WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
				ch->println("`cYou failed.`x");
				return;
			}
			
			WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
			REMOVE_BIT(obj->value[1],EX_LOCKED);
			act("`cYou pick the lock on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n picks the lock on $p.`x",ch,obj,NULL,TO_ROOM);
			msp_skill_sound(ch, gsn_pick_lock);
			check_improve(ch,gsn_pick_lock,true,2);
			return;
		}

		// 'pick object' 
		if ( obj->item_type != ITEM_CONTAINER )
		{ ch->println( "`cThat's not a container.`x" ); return; }
		if ( !IS_SET(obj->value[1], CONT_CLOSED) )
		{ ch->println( "`cIt's not closed.`x" );		 return; }
		if ( obj->value[2] < 0 )
		{ ch->println( "`cIt can't be unlocked.`x" );	 return; }
		if ( !IS_SET(obj->value[1], CONT_LOCKED) )
		{ ch->println( "`cIt's already unlocked.`x" );	 return; }
		// lockers are 10 times harder to pick than normal containers
		if ( IS_SET(obj->value[1], CONT_LOCKER) && number_range(1,10)!=1){
			WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
			act("`cYou fail to pick the lock on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n attempts to pick the lock on $p but fails.`x",ch,obj,NULL,TO_ROOM);
			return;
		}
		if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
		{
			WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
			ch->println( "`cYou failed.`x" );
			return;
		}
		
		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act("`cYou pick the lock on $p.`x",ch,obj,NULL,TO_CHAR);
		act("`c$n picks the lock on $p.`x",ch,obj,NULL,TO_ROOM);
		WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
		msp_skill_sound(ch, gsn_pick_lock);
		check_improve(ch,gsn_pick_lock,true,2);
		return;
    }
	
	if ( ( door = find_door( ch, arg, "pick" ) ) >= 0 )
	{
		/* 'pick door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev=NULL;
		
		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info,EX_CLOSED) )
		{ ch->println("`cIt's not closed.`x");		return; }
		if ( pexit->key < 0 && !IS_IMMORTAL(ch))
		{ ch->println("`cIt can't be picked.`x");	return; }
		if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
		{ ch->println("`cIt's already unlocked.`x"); return; }
		if ((IS_SET(pexit->exit_info,	 EX_PICKPROOF) && !IS_IMMORTAL(ch))
			|| (IS_SET(pexit->exit_info, EX_HARD) && number_range(1,4)!=1)
			|| (IS_SET(pexit->exit_info, EX_INFURIATING) && number_range(1,20)!=1)
			)
		{
			WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
			ch->println("`cYou failed.`x");
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );	
		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		ch->println("`c*Click*`x");
		msp_skill_sound(ch, gsn_pick_lock);
		act( "`c$n picks the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );
		check_improve(ch,gsn_pick_lock,true,2);
		
		/* pick the other side */
		if ( ( to_room   = pexit->u1.to_room            ) != NULL
			&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
			&&   pexit_rev->u1.to_room == ch->in_room )
		{
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
		}
    }
	
    return;
}

/**************************************************************************/
void do_stand( char_data *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
	
    if (ch->mounted_on!=NULL)
	{
		ch->println("`cYou cannot do that while mounted.`x");
		return;
	}
	
    if ( argument[0] != '\0' )
    {
		if (ch->position == POS_FIGHTING)
		{
			ch->println("`cMaybe you should finish fighting first?`x");
			return;
		}
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
			ch->println("`cYou don't see that here.`x");
			return;
		}
		if (obj->item_type != ITEM_FURNITURE
			||  (!IS_SET(obj->value[2],STAND_AT)
			&&   !IS_SET(obj->value[2],STAND_ON)
			&&	 !IS_SET(obj->value[2],STAND_UNDER)
			&&   !IS_SET(obj->value[2],STAND_IN)))
		{
			ch->println("`cYou can't seem to find a place to stand.`x");
			return;
		}
		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_new("`cThere's no room to stand on $p.`x", ch, obj, NULL, TO_ROOM, POS_DEAD);
			return;
		}
	}
	
	switch ( ch->position )
	{
	case POS_SLEEPING:
		ch->println("`cStanding in the dreamworld does not make you stand here!`x");
		return;
		
	case POS_RESTING: 
	case POS_SITTING:
	case POS_KNEELING:
		if (obj == NULL)
		{
			ch->println("`cYou stand up.`x");
			act( "`c$n stands up.`x", ch, NULL, NULL, TO_ROOM );
			ch->on = NULL;
			ch->is_trying_sleep=false;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
			act("`cYou stand at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n stands at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
			act("`cYou stand on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n stands on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],STAND_UNDER))
		{
			act("`cYou stand under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n stands under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou stand in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n stands in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_STANDING;
		break;
		
	case POS_STANDING:
		ch->println("`cYou are already standing.`x");
		ch->is_trying_sleep=false;
		break;
		
    case POS_FIGHTING:
		ch->println("`cYou are already fighting!`x");
		break;
    }
	
	return;
}

/**************************************************************************/
void do_kneel( char_data *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
	
    if(ch->mounted_on!=NULL)
	{
		ch->println( "`cYou cannot kneel while mounted, dismount first.`x" );
		return;
	}
	
    if(!IS_NULLSTR(argument))
    {
		if(ch->position == POS_FIGHTING){
			ch->println( "`cMaybe you should finish fighting first?`x" );
			return;
		}
		
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if(!obj){
			ch->printlnf( "`cYou don't see any '%s' here to kneel on.`x", argument );
			return;
		}

		if (obj->item_type != ITEM_FURNITURE
			||  (!IS_SET(obj->value[2],KNEEL_AT)
			&&   !IS_SET(obj->value[2],KNEEL_ON)
			&&   !IS_SET(obj->value[2],KNEEL_UNDER)
			&&   !IS_SET(obj->value[2],KNEEL_IN)))
		{
			ch->println("`cYou can't seem to find a place to kneel.`x");
			return;
		}

		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_new("`cThere's no room to kneel on $p.`x", ch,obj,NULL,TO_ROOM,POS_DEAD);
			return;
		}
	}
	
	switch ( ch->position )
	{
	case POS_SLEEPING:
		ch->println( "`cYou can't wake up!`x" );
		return;
		
	case POS_RESTING: 
	case POS_SITTING:
	case POS_STANDING:
		if (obj == NULL)
		{
			ch->println( "`cYou kneel on the floor.`x" );
			act( "`c$n kneels on the floor.`x", ch, NULL, NULL, TO_ROOM );
			ch->on = NULL;
			ch->is_trying_sleep=false;
		}
		else if (IS_SET(obj->value[2],KNEEL_AT))
		{
			act("`cYou kneel at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n kneels at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],KNEEL_ON))
		{
			act("`cYou kneel on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n kneels on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],KNEEL_UNDER))
		{
			act("`cYou kneel under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n kneels under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou kneel in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n kneels in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_KNEELING;
		break;
				
    case POS_FIGHTING:
		ch->println("`cYou are fighting!`x");
		break;
    }
	return;
}

/**************************************************************************/
void do_rest( char_data *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
	
    if (ch->mounted_on!=NULL)
	{
		ch->println("`cYou cannot do that while mounted.`x");
		return;
    }
    
	if (!ch->fighting && ch->position == POS_FIGHTING)
	{
		bug("do_rest: !ch->fighting && ch->position == POS_FIGHTING");
		ch->position = POS_STANDING;
	}
    
	if (ch->position == POS_FIGHTING)
    {
		ch->println("`cYou are already fighting!`x");
		return;
	}
	
    // okay, now that we know we can rest, find an object to rest on 
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
			ch->println("`cYou don't see that here.`x");
			return;
		}
	}else{
		obj = ch->on;
	}
	

    if (obj != NULL)
    {
		if (obj->item_type != ITEM_FURNITURE 
			||  (!IS_SET(obj->value[2],REST_ON)
			&&   !IS_SET(obj->value[2],REST_IN)
			&&   !IS_SET(obj->value[2],REST_UNDER)
			&&   !IS_SET(obj->value[2],REST_AT)))
		{
			ch->println("`cYou can't rest on that!`x");
			return;
		}
		
		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_new("`cThere is no room on $p for you.`x",ch,obj,NULL,TO_CHAR,POS_DEAD);
			return;
		}
		
		ch->on = obj;
    }

    switch ( ch->position )
	{
	case POS_SLEEPING:
		ch->println("`cYou can't wake yourself up!!`x");
		break;
		
	case POS_RESTING:
		ch->println("`cYou are already resting.`x");
		break;
		
	case POS_STANDING:
	case POS_KNEELING:
		if (obj == NULL)
		{
			ch->println("`cYou rest.`x");
			act( "`c$n sits down and rests.`x", ch, NULL, NULL, TO_ROOM );
		}
		else if (IS_SET(obj->value[2],REST_AT))
		{
			act("`cYou sit down at $p and rest.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits down at $p and rests.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_ON))
		{
			act("`cYou sit on $p and rest.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits on $p and rests.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_UNDER))
		{
			act("`cYou sit under $p and rest.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits under $p and rests.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou rest in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n rests in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;
		
    case POS_SITTING:
		if (obj == NULL)
		{
			ch->println("`cYou rest.`x");
			act("`c$n rests.`x",ch,NULL,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_AT))
		{
			act("`cYou rest at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n rests at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_ON))
		{
			act("`cYou rest on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n rests on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_UNDER))
		{
			act("`cYou rest under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n rests under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou rest in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n rests in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;
    }

	if ( IS_AFFECTED(ch, AFF_FLYING) && HAS_CONFIG(ch, CONFIG_AUTOLANDONREST)){
		landchar(ch);
		ch->println("`cYou stop flying.`x");
	}

    return;
}

/**************************************************************************/
void do_sit (char_data *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
	
    if(ch->mounted_on){
		ch->println("`cYou are already sitting on something (your mount).`x");
		return;
	}
    
    if(ch->position == POS_FIGHTING){
		ch->println("`cMaybe you should finish this fight first?`x");
		return;
    }
	
	// okay, now that we know we can sit, find an object to sit on
    if(!IS_NULLSTR(argument)){
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
			ch->printlnf("`cYou don't see any %s here to sit on/in/under/at.`x", argument);
			return;
		}
	}else{
		obj = ch->on;
	}
	
    if (obj != NULL)                                                              
    {
		if (!IS_SET(obj->item_type,ITEM_FURNITURE)
			||  (!IS_SET(obj->value[2],SIT_ON)
			&&   !IS_SET(obj->value[2],SIT_IN)
			&&   !IS_SET(obj->value[2],SIT_UNDER)
			&&   !IS_SET(obj->value[2],SIT_AT)))
		{
			ch->println("`cYou can't sit on that.`x");
			return;
		}
		
		if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_new("`cThere's no more room on $p.`x",ch,obj,NULL,TO_CHAR,POS_DEAD);
			return;
		}
		
		ch->on = obj;
    }
	switch (ch->position)
    {
	case POS_SLEEPING:
		ch->println("`cYou must wake up first.`x");
		break;
	case POS_RESTING:
		if (obj == NULL)
			ch->println("You stop resting.`x");
		else if (IS_SET(obj->value[2],SIT_AT))
		{
			act("`cYou sit at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_UNDER))
		{
			act("`cYou sit under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_IN))
		{
			act("`cYou sit in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou sit on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;
	case POS_KNEELING:
		if (obj == NULL)
			ch->println("`cYou stop kneeling.`x");
		else if (IS_SET(obj->value[2],SIT_AT))
		{
			act("`cYou sit at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_UNDER))
		{
			act("`cYou sit under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_IN))
		{
			act("`cYou sit in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou sit on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;
	case POS_SITTING:
		ch->println("`cYou are already sitting down.`x");
		break;
	case POS_STANDING:
		if (obj == NULL)
		{
			ch->println("`cYou sit down.`x");
			act("`c$n sits down on the ground.`x",ch,NULL,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_AT))
		{
			act("`cYou sit down at $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits down at $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_IN))
		{
			act("`cYou sit in $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits in $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_UNDER))
		{
			act("`cYou sit under $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits under $p.`x",ch,obj,NULL,TO_ROOM);
		}
		else
		{
			act("`cYou sit down on $p.`x",ch,obj,NULL,TO_CHAR);
			act("`c$n sits down on $p.`x",ch,obj,NULL,TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;
    }
    return;
}


/**************************************************************************/
void do_sleep( char_data *ch, char *argument )
{
	OBJ_DATA *obj = NULL;
	
    if (ch->mounted_on!=NULL)
	{
		ch->println( "`cYou cannot do that while mounted.`x" ); 
		return;
	}
	
	// dont spam message
	if (ch->desc && ch->desc->repeat>5)
	{
		ch->println( "`cSpamming the sleep command isn't going to make you go to`x" );
		ch->println( "`csleep any faster.  You will fall asleep when you fall asleep.`x" );
		return;
	}
	
	switch ( ch->position )
	{
    case POS_SLEEPING:
		ch->println( "`cYou are already sleeping.`x" );
		break;
		
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
    case POS_KNEELING: 
		if (argument[0] == '\0' && ch->on == NULL)
		{
			ch->println( "`cYou attempt to go to sleep.`x" );
			ch->is_trying_sleep=true;
			ch->position=POS_RESTING;
			if(IS_NEWBIE(ch)){
				ch->println("`cnote: you won't fall asleep instantly unless you are very tired.`x");
			}
		}
		else  // find an object and sleep on it 
		{
			if (argument[0] == '\0'){
				obj = ch->on;
			}else{
				obj = get_obj_list( ch, argument,  ch->in_room->contents );
			}
			
			if (obj == NULL){
				ch->printlnf( "`cYou don't see any '%s' in the room to sleep on.`x", argument );
				if(IS_NEWBIE(ch)){
					ch->println("`cNote: you must drop carried items such as bedrolls before sleeping on/in them.`x");
				}
				return;
			}
			if (obj->item_type != ITEM_FURNITURE
				||  (!IS_SET(obj->value[2],SLEEP_ON)
				&&   !IS_SET(obj->value[2],SLEEP_IN)
				&&   !IS_SET(obj->value[2],SLEEP_UNDER)
				&&   !IS_SET(obj->value[2],SLEEP_AT)))
			{
				ch->printlnf( "`cYou can't sleep on %s!`x", obj->short_descr);
				return;
			}
			
			if (ch->on != obj && count_users(obj) >= obj->value[0]){
				act_new("`cThere is no room on $p for you.`x",ch,obj,NULL,TO_CHAR,POS_DEAD);
				return;
			}
			
			ch->on = obj;
			if (IS_SET(obj->value[2],SLEEP_AT))
			{
				act("`cYou go to sleep at $p.`x",ch,obj,NULL,TO_CHAR);
				act("`c$n tries to sleep at $p.`x",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SLEEP_UNDER))
			{
				act("`cYou try to go to sleep under $p.`x",ch,obj,NULL,TO_CHAR);
				act("`c$n tries to go to sleep under $p.`x",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SLEEP_ON))
			{
				act("`cYou try to go to sleep on $p.`x",ch,obj,NULL,TO_CHAR);
				act("`c$n tries to go to sleep on $p.`x",ch,obj,NULL,TO_ROOM);
			}
			else
			{
				act("`cYou try to go to sleep in $p.`x",ch,obj,NULL,TO_CHAR);
				act("`c$n tries to go to sleep in $p.`x",ch,obj,NULL,TO_ROOM);
			}
			ch->is_trying_sleep=true;
			ch->position=POS_RESTING;
		}
		break;
		
    case POS_FIGHTING:
		ch->println( "`cYou are already fighting!`x" );
		break;
    }

	if ( IS_AFFECTED(ch, AFF_FLYING) && HAS_CONFIG(ch, CONFIG_AUTOLANDONREST)){
		landchar(ch);
		ch->println( "`cYou stop flying.`x" );
	}

					
	// instant sleep for mobs
	if(IS_NPC(ch) && ch->is_trying_sleep)
	{
		ch->position=POS_SLEEPING;
		ch->println("`cYou drift off into the dreamscape.`x");
		act( "`c$n goes to sleep.`x", ch, NULL, NULL, TO_ROOM );
	}
    return;
}


/**************************************************************************/
void do_wake( char_data *ch, char *argument )
{
	char arg[MIL];
    char_data *victim;
	
    one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
	    if(ch->mounted_on || ch->fighting || ch->position==POS_FIGHTING )
		{
			ch->println( "`cYou are already awake.`x" ); 
			return;
		}
    
		if(!IS_NPC(ch))
		{
			if(ch->pcdata->tired>30 || IS_AFFECTED(ch,AFF_SLEEP) )
			{
				// dont spam message
				if (ch->desc && ch->desc->repeat>10)
				{
					ch->println( "`cSpamming the wake command isn't going to help you wakeup.`x" );
					return;
				}
				ch->println( "`cYou cannot wake yourself.`x" );
			}
			else
			{
				if(ch->position==POS_STANDING)
				{
					ch->println( "`cYou are already standing.`x" );
					ch->is_trying_sleep=false;
				}
				else
				{
					// if you wake up while drunk, you'll have a nasty little headache
					if ( ch->pcdata->condition[COND_DRUNK] )
					{
						AFFECT_DATA af;
						
						af.where		= WHERE_MODIFIER;
						af.type			= gsn_cause_headache;
						af.level		= ch->level;
						af.duration		= 3;
						af.location		= APPLY_SD;
						af.modifier		= - ch->level/5;
						af.bitvector	= 0;
						affect_to_char( ch, &af );
						ch->println("`cYou wake up with a major hangover.`x");
						ch->pcdata->condition[COND_DRUNK] = 0;
					}

					ch->position=POS_RESTING;
					do_stand(ch,"");
				}
			}
		}
		else
		{
			if(ch->position==POS_STANDING)
			{
				ch->println("`cYou are already standing.`x");
				ch->is_trying_sleep=false;
			}else{
				ch->position=POS_RESTING;
				do_stand(ch,"");
			}
		}       
		return;
	}
	
	if ( !IS_AWAKE(ch) )
	{ ch->println("`cYou are asleep yourself!`x");	return; }
	
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ ch->println("`cThey aren't here.`x");			return; }
	
	if ( IS_AWAKE(victim) )
	{ act( "`c$N is already awake.`x", ch, NULL, victim, TO_CHAR ); return; }
	
	if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "`cYou can't wake $M!`x",   ch, NULL, victim, TO_CHAR );  return; }
	
	act_new( "`c$n wakes you.`x", ch, NULL, victim, TO_VICT,POS_SLEEPING );
	victim->position=POS_RESTING;
	victim->subdued= false;


	// if you wake up while drunk, you'll have a nasty little headache
	if ( !IS_NPC( victim ))
	{
		if ( victim->pcdata->condition[COND_DRUNK] )
		{
			AFFECT_DATA af;

			af.where		= WHERE_MODIFIER;
			af.type			= gsn_cause_headache;
			af.level		= victim->level;
			af.duration		= 3;
			af.location		= APPLY_SD;
			af.modifier		= - victim->level/5;
			af.bitvector	= 0;
			affect_to_char( victim, &af );
			victim->println("`cYou wake up with a major hangover.`x");
			victim->pcdata->condition[COND_DRUNK] = 0;
		}
	}

	do_stand(victim,"");
	return;
}


/**************************************************************************/
void do_sneak( char_data *ch, char *)
{
	AFFECT_DATA af;
	
	if (ch->mounted_on!=NULL)
	{
		ch->println( "`cYou cannot do that while mounted.`x" );
		return;
	}

	ch->println( "`cYou attempt to move silently.`x" );
	affect_strip( ch, gsn_sneak );

	if (IS_AFFECTED(ch,AFF_SNEAK))
		return;

	if ( number_percent( ) < get_skill(ch,gsn_sneak))
	{
		check_improve(ch,gsn_sneak,true,3);
		af.where     = WHERE_AFFECTS;
		af.type      = gsn_sneak;
		af.level     = ch->level;
		af.duration  = ch->level;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_SNEAK;
		affect_to_char( ch, &af );
	}else{
		check_improve(ch,gsn_sneak,false,3);
	}
	return;
}


/**************************************************************************/
void do_hide( char_data *ch, char *)
{
	if (ch->mounted_on!=NULL){
		ch->println( "`cYou cannot do that while mounted.`x" ); 
		return;
	}
    
    ch->println( "`cYou attempt to hide.`x" );

    if( IS_AFFECTED(ch, AFF_HIDE) ){
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
	}

    if ( number_percent( ) < get_skill(ch,gsn_hide)){
		SET_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch,gsn_hide,true,3);
    }else{
		check_improve(ch,gsn_hide,false,3);
	}

    return;
}

/**************************************************************************/
void do_vanish( char_data *ch, char *)
{
	AFFECT_DATA af;

	if (IS_NPC(ch))
	{
		ch->println( "Players only." );
		return;
	}
	
	if (get_skill(ch, gsn_vanish) < 1)
	{
		ch->println( "`cYou put your hands in front of your eyes, hoping no one will see you.`x" );
		return;
	}


	if (ch->mounted_on!=NULL)
	{
		ch->println( "`cYou cannot do that while mounted.`x" ); 
		return;
	}

    if ( IS_AFFECTED2(ch, AFF2_VANISH) )
	{
		ch->println( "`cYou are already out of sight.`x" );
		return;
	}

    if (ch->pcdata->next_vanish>current_time)
    {
		ch->println( "`cYou are not able to vanish just yet.`x" );
		return;
	}

    ch->println( "`cYou attempt to vanish from plain sight.`x" );

    if ( number_percent( ) < get_skill(ch,gsn_vanish))
    {
		af.where     = WHERE_AFFECTS2;
		af.type      = gsn_vanish;
		af.level     = ch->level;
		af.duration  = (ch->level/20)+1;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF2_VANISH;
		affect_to_char( ch, &af );
		check_improve(ch,gsn_vanish,true,3);
		ch->pcdata->next_vanish = current_time + (150 - ch->level)*8;
		act( "`c$n vanishes from plain sight.`x", ch, NULL, NULL, TO_ROOM );
    }
    else
	{
		check_improve(ch,gsn_vanish,false,3);
		ch->pcdata->next_vanish = current_time + (150 - ch->level);
	}

    return;
}

/**************************************************************************/
void do_sscan( char_data *ch, char *argument )
{
	if ( number_percent( ) < get_skill(ch,gsn_scan) || IS_CONTROLLED(ch))
	{
		do_scan( ch, argument);
		check_improve(ch,gsn_scan,true,1);
	}
	else
	{
		check_improve(ch,gsn_scan,false,1);
		ch->println("`cYour scanning is insufficient.`x");
	}
	return;
}

/**************************************************************************/
// by Alander
void do_visible( char_data *ch, char *argument)
{
    affect_strip ( ch, gsn_invisibility				);
    affect_strip ( ch, gsn_mass_invis				);
    affect_strip ( ch, gsn_sneak					);
    REMOVE_BIT   ( ch->affected_by,	AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);

	if ( IS_SET( ch->affected_by2, AFF2_TREEFORM ))
	{
		REMOVE_BIT( ch->affected_by2, AFF2_TREEFORM );
		act( "`cA tree suddenly transforms into $n.`x", ch, NULL, NULL, TO_ROOM );
		ch->println( "`cYou assume your normal form.`x" );
	}

	if ( IS_SET( ch->affected_by2, AFF2_VANISH ))
	{
		REMOVE_BIT( ch->affected_by2, AFF2_VANISH );
		affect_strip ( ch, gsn_vanish );
		act( "`cA swirl of dust reveals $n.`x", ch, NULL, NULL, TO_ROOM );
		ch->println( "`cYou shake the `#`Bfaerie-dust`^ from your body.`x" );
	}

	if(strcmp(argument,"auto"))
	{
		ch->println( "`cYou make yourself visible.`x" );
	}
	return;
}
/**************************************************************************/
// find the players recall vnum in order of priority
// 1st - a manually set vnum
// 2nd - level 1 to 5 newbie recall vnum
// 3rd - court location
// 4th - clan location if not on pkill port
// 5th - class recall point (guild[0] for now)
// 6th - default racial location
int get_recallvnum(char_data *ch)
{
	int vnum=ROOM_VNUM_OOC;
	
	if (ch->recall_inn_room>0) {
		vnum=ch->recall_inn_room;
	} else if (ch->recall_room > 0 ){
		vnum= ch->recall_room;
	// using IS_SET to check for the court flag since when switched it 
	// shouldn't get the court status of the controling player, but the mob
	}else if (IS_COURT(ch)){
		vnum=ROOM_VNUM_COURT_RECALL;
	}else if (ch->level<6 && ROOM_VNUM_NEWBIE_RECALL){       
		vnum=ROOM_VNUM_NEWBIE_RECALL;
	}else if (ch->clan && !GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){       
		vnum=ch->clan->recall_room();
	}else if ( class_table[ch->clss].recall > 0 ){
		vnum= class_table[ch->clss].recall;
	} else {
		vnum=race_table[ch->race]->recall_room;
	}
	return vnum;
};
/**************************************************************************/
void do_recall( char_data *ch, char *arg)
{
    char_data *victim;
    ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *innlocation;
    static int pvnum;
    int recall_vnum;
    char buf[MIL]; //for arena broadcast 
    char_data *wch; 	

//Arena recall situation 
    if ( IS_SET(ch->act2, ACT2_PLR_ARENA)) 
    { 
      if(ch->position == POS_FIGHTING){ 
         ch->println( "`cMaybe you should finish fighting first?`x\n\r"); 
            return; 
      } 

      if(ch->position != POS_STANDING){ 
         ch->position = POS_STANDING; 
      } 
      if ( IS_SET(ch->affected_by, AFF_CURSE)){ 
          REMOVE_BIT( ch->affected_by, AFF_CURSE ); 
      } 
      if(ch->pknorecall > 0){ 
         ch->pknorecall = 0; 
      } 
   } 
    
    if ( IS_SET(ch->act2, ACT2_PLR_EVENT)) 
    { 
      if(ch->position == POS_FIGHTING){ 
         ch->println( "`cMaybe you should finish fighting first?`x\n\r"); 
            return; 
      } 

      if(ch->position != POS_STANDING){ 
         ch->position = POS_STANDING; 
      } 
      if ( IS_SET(ch->affected_by, AFF_CURSE)){ 
          REMOVE_BIT( ch->affected_by, AFF_CURSE ); 
      } 
      if(ch->pknorecall > 0){ 
         ch->pknorecall = 0; 
      } 
   } 

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET)) 
    { 
      ch->println("`cOnly players can recall.`x"); 
      return; 
    }

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
		ch->println("`cOnly players can recall.`x");
		return;
    }

	// Recall reset resets the recal_inn_room to zero. This means the player recalls to his
	// default recall spot.
	if (!str_cmp("reset", arg)) {
		// The inn recall room is set.
		if ( ch->recall_inn_room!=0 ) {
			// Check the room is valid before printing the name of the room to the player.
			if ( ((innlocation=get_room_index(ch->recall_inn_room)) != NULL) &&
				  (innlocation->name != NULL) ) {
				// Inform the player which room is no longer his recall room.
				ch->printlnf("You no longer recall to %s.", innlocation->name);
			}
		} else {
			// Inn recall room wasn't set.
			ch->println("`cYou weren't recalling anywhere specific.`x");
		}

		// Reset the inn recall variables.
		ch->recall_inn_room		= 0;
		ch->expire_recall_inn	= 0;

		return;
	}

	// stop pets recalling if they are asleep
	if(!IS_AWAKE(ch)){
		ch->println( "`cYou can't recall when you are sleeping!`x" );
		return;
	}
	
	// Check to see if ch is charmed and being ordered to cast
    if ( IS_AFFECTED(ch,AFF_CHARM) && !IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		ch->println( "`cYou must wait for your master to tell you to recall.`x" );
		return;
	}
	
    location = ch->last_ic_room;
	
/*	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		ch->println( "`cYou pray for transportation but nothing happens.`x" );
		return;
	}
*/	
    if (IS_OOC(ch) && IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
	{
        act( "`c$n prays for transportation!`x", ch, 0, 0, TO_ROOM );
        ch->wrapln("`cAs you pray for transportation, You feel an inward`x "
			"`ctugging but realise you haven't gone anywhere.`x");
		return;
	}
	
    if (IS_OOC(ch) && location)
    {
		act( "`c$n decides to go back to the IC realm.`x", ch, 0, 0, TO_ROOM );
		ch->last_ic_room= NULL;
    }
    else 
    {
		act( "`c$n prays for transportation!`x", ch, 0, 0, TO_ROOM );
		
		
		// pets recall to their masters
		if (IS_NPC(ch) && ch->master){
			recall_vnum = ch->master->in_room->vnum;
		}else{
			recall_vnum=get_recallvnum(ch);
		}
		
		if ( ( location = get_room_index( recall_vnum ) ) == NULL )
		{
			ch->println( "`cYou are completely lost.`x" );
			
			if ( ( location = get_room_index( ROOM_VNUM_OOC ) ) == NULL)
			{
				ch->printlnf( "`cBUG: Cant find the main ooc room (vnum = %d)`x\r\n"
					"`cPlease report this to an admin.`x", ROOM_VNUM_OOC);
				return;
			}
			else
			{
				if (IS_SET(location->room_flags, ROOM_OOC))
				{
					ch->printlnf( "`cTaking you to the main OOC room since your normal recall (%d) doesnt exist.`x", recall_vnum);
				}
				else
				{
					ch->printlnf( "`cBUG: Taking you to the main ooc room (vnum = %d)`x\r\n"
						"`cThis room SHOULD be an OOC room - please report this bug to an admin.`x", ROOM_VNUM_OOC);
				}
			}	
		}
		
		if (ch->pknorecall>0){
			ch->println("`cYou may not recall so soon after conflict.`x");
			return;
		}
		
		if(IS_NPC(ch)){
			location = get_room_index(pvnum);
		}
		
		if ( ch->in_room == location ){
			ch->println("`cThere would be no point in recalling, since you are already at your recall location.`x");
			return;
		}
		
		if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
			||   IS_AFFECTED(ch, AFF_CURSE))
		{
			ch->println("`cYou pray for transportation, but nothing appears to happen.`x");
			return;
		}
		
		OBJ_DATA *city_flag;
	 
		for (city_flag = ch->carrying; city_flag; city_flag = city_flag->next_content ) 
    		{if (city_flag->pIndexData->vnum == OBJ_VNUM_ATHENS_FLAG)
		{ch->printlnf("`RYou can't recall while carrying %s!!!`x", city_flag->description);return;}}

                for (city_flag = ch->carrying; city_flag; city_flag = city_flag->next_content )
                {if (city_flag->pIndexData->vnum == OBJ_VNUM_SPARTA_FLAG)
                {ch->printlnf("`RYou can't recall while carrying %s!!!`x", city_flag->description);return;}}

		for(city_flag = ch->carrying; city_flag; city_flag = city_flag->next_content )
		{if (city_flag->pIndexData->vnum == OBJ_VNUM_MEGARA_FLAG)
                {ch->printlnf("`RYou can't recall while carrying %s!!!`x", city_flag->description);return;}}



		if ( ( victim = ch->fighting ) != NULL )
		{
			int lose,skill;
			
			skill = get_skill(ch,gsn_recall);
			
			if ( number_percent() < 80 * skill / 100 )
			{
				check_improve(ch,gsn_recall,false,6);
				WAIT_STATE( ch, 4 );
				ch->println("You failed!");
				return;
			}
			lose = (ch->desc != NULL) ? 25 : 50;
			gain_exp( ch, 0 - lose );
			if ( IS_HERO( ch ))
				do_heroxp( ch, 0 - lose );
			
			check_improve(ch,gsn_recall,true,4);
			ch->printlnf( "`cYou recall from combat!  You lose `C%d`c exps.`x", lose );
			stop_fighting( ch, true );
			
		}
		
		if (ch->move>0 && IS_IC(ch) && ch->level>5){
			ch->move /= 2;
		}
		
		if (ch->mounted_on!=NULL){
			ch->mounted_on->move/=2;
		}
    }
	
    if (ch->mounted_on){
		act( "`c$n and $N disappear.`x", ch, NULL, ch->mounted_on, TO_ROOM );
		char_from_room(ch->mounted_on);
		char_to_room(ch->mounted_on, location);
    }else{
		act( "`c$n disappears.`x", ch, NULL, NULL, TO_ROOM );  
	}
    
    char_from_room( ch );
    char_to_room( ch, location );
	
    if (ch->mounted_on){
		act( "`c$n appears in the room riding $N.`x", ch, NULL, ch->mounted_on, TO_ROOM ); 
    }else{
		act( "`c$n appears in the room.`x", ch, NULL, NULL, TO_ROOM );
	}
	 
    do_look( ch, "auto" );

if ( IS_SET(ch->act2, ACT2_PLR_EVENT)) 
   { 
      REMOVE_BIT(ch->act2, ACT2_PLR_EVENT);
	ch->printlnf( "`CYou have forfeited your entry in this event!");
       return;
   }
    
if ( IS_SET(ch->act2, ACT2_PLR_ARENA)) 
   { 
      REMOVE_BIT(ch->act2, ACT2_PLR_ARENA);       
        
      if(ch->challenged) 
      { 
         ch->challenged = NULL; 
         if(ch->pcdata->bet) 
         { 
         ch->pcdata->bet = NULL; 
         } 

         //Arena clean up system 
             
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
                  if(wch->position != POS_STANDING){ 
                     wch->position = POS_STANDING; 
                  } 
                  if(wch->pcdata->bet){ 
                     wch->pcdata->bet = NULL; 
                  } 

                  do_recall(wch, "");                      
               } 
                
            } 
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
               if(can_see(wch, ch) && wch->challenger){ 
                  wch->challenger = NULL; //If they had not accepted yet and outside the arena. 
               } 
            } 
         sprintf(buf,"`W%s `Rhas cancelled the contest by recall.`x\n\r",ch->name); 
         arena_broadcast( ch, buf );             
      } 
       
      if(ch->challenger) 
      { 
         ch->challenger = NULL; 
         if(ch->pcdata->bet) 
         { 
         ch->pcdata->bet = NULL; 
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
                  if(wch->position != POS_STANDING){ 
                     wch->position = POS_STANDING; 
                  } 
                  if(wch->pcdata->bet){ 
                     wch->pcdata->bet = NULL; 
                  } 

                  do_recall(wch, "");                
               } 
            } 
         for( wch = player_list; wch; wch = wch->next_player ) 
            { 
                                     
               if(can_see(wch, ch) && wch->pcdata->bet)    
               { 
                  //if bets were made give back.                   
                  wch->gold += wch->pot; 
                  wch->pot = 0; 
                  wch->println("`R[ARENA]:`x Bet cancelled! You get your `Ygold`x back!\n\r"); 
                  wch->pcdata->bet = NULL; 
                  REMOVE_BIT(wch->act2, ACT2_PLR_BET);                                                 
               } 
            } 
         for( wch = player_list; wch; wch = wch->next_player ) 
            { 
               if(can_see(wch, ch) && wch->challenger){ 
                  wch->challenger = NULL; //Good to be careful. 
               } 
            } 
         sprintf(buf,"`W%s `Rhas cancelled the contest by recall.`x\n\r",ch->name); 
         arena_broadcast( ch, buf );          
      } 
                
   }//main if 
    

    if ( ch->pet && (ch->mounted_on!=ch->pet))  
    {
		pvnum = location->vnum;
		SET_BIT( ch->pet->dyn, DYN_IS_BEING_ORDERED );
		do_recall(ch->pet,"");
		REMOVE_BIT( ch->pet->dyn, DYN_IS_BEING_ORDERED );
    }
	
    return;
}

/**************************************************************************/
void do_train( char_data *ch, char *argument )
{
	char buf[MSL];
    char_data *mob;
    sh_int stat = - 1;
	char *pOutput = NULL;
	int cost, amount, stat_diff;
	
    if ( IS_NPC(ch) ){
		ch->println( "`cOnly players can train sorry.`x" );
		return;
	}

	// Check for trainer.
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
			break;
    }
	
    if ( !mob )
    {
		ch->println("`cYou can't train here.`x");
		return;
    }

    // pretrain trigger, activated only on mobs with the trigger 
	// if the command 'mob preventtrain' is called, then the
	// training is aborted
	if ( IS_NPC(mob) && HAS_TRIGGER(mob, TRIG_PRETRAIN)){
		mobprog_preventtrain_used=false;
		if(mp_percent_trigger( mob, ch, NULL, NULL, TRIG_PRETRAIN)){
			if(mobprog_preventtrain_used){
				mob->printlnf("Training prevented on %s", PERS(ch, NULL));
				return;
			}
		}
	}

	
    if ( argument[0] == '\0' )
    {
		ch->printlnf( "`cYou have `C%d`c training sessions.`x", ch->train );
		argument = "foo";
	}
	
    cost = 1;
	
    if ( !str_prefix( argument, "strength" ) ){
		stat        = STAT_ST;
		pOutput     = "strength";
    }else if ( !str_prefix( argument, "quickness" ) ){
		stat        = STAT_QU;
		pOutput     = "quickness";
	} else if ( !str_prefix( argument, "presence" ) ){
		stat        = STAT_PR;
		pOutput     = "presence";
	} else if ( !str_prefix( argument, "empathy" ) ){
		stat        = STAT_EM;
		pOutput     = "empathy";
	} else if ( !str_prefix( argument, "intuition" ) ){
		if(!str_cmp(argument, "in") || !str_cmp(argument, "int")){
			ch->println("Note: 'int' on this mud is short for intuition (NOT intelligence)");
			ch->println("      Intelligence is combination of memory and reasoning.");
			ch->println("      type 'train intu' if you really want to train intuition.");
			ch->println("      type 'train mem' to train memory.");
			ch->println("      type 'train rea' to train reasoning.");
			return;
		}
		stat        = STAT_IN;
		pOutput     = "intuition";
	} else if ( !str_prefix( argument, "constitution" ) ){
		stat        = STAT_CO;
		pOutput     = "constitution";
	} else if ( !str_prefix( argument, "agility" ) ){
		stat        = STAT_AG;
		pOutput     = "agility";
	} else if ( !str_cmp( argument, "sd" ) || !str_prefix( argument, "self-discipline" ) ){
		stat        = STAT_SD;
		pOutput     = "self-discipline";
	} else if ( !str_prefix( argument, "memory" ) ){
		stat        = STAT_ME;
		pOutput     = "memory";
	} else if ( !str_prefix( argument, "reasoning" ) ){
		stat        = STAT_RE;
		pOutput     = "reasoning";
/*	}else if ( !str_cmp(argument, "hp" ) || !str_prefix( argument, "hitpoints" ) ){
		cost = 1;
	}else if ( !str_cmp(argument, "mana" ) ){
		cost = 1;
	}else if ( !str_cmp(argument, "moves" )|| !str_cmp( argument, "mv" )  ){
		cost = 1;
*/
	}else{
		strcpy( buf, "`cYou can train:`x\n\r" );
		if ( ch->perm_stats[STAT_ST] < ch->potential_stats[STAT_ST] ) {
			strcat( buf, "`c   strength`x\n\r" );
		}
		if ( ch->perm_stats[STAT_QU] < ch->potential_stats[STAT_QU] ){
			strcat( buf, "`c   quickness`x\n\r" );
		}		
		if ( ch->perm_stats[STAT_PR] < ch->potential_stats[STAT_PR] ){
			strcat( buf, "`c   presence`x\n\r" );
		}
		if ( ch->perm_stats[STAT_EM] < ch->potential_stats[STAT_EM] ){
			strcat( buf, "`c   empathy`x\n\r" );
		}
		if ( ch->perm_stats[STAT_IN] < ch->potential_stats[STAT_IN] ){
			strcat( buf, "`c   intuition`x\n\r" );
		}
		if ( ch->perm_stats[STAT_CO] < ch->potential_stats[STAT_CO] ){
			strcat( buf, "`c   constitution`x\n\r" );
		}
		if ( ch->perm_stats[STAT_AG] < ch->potential_stats[STAT_AG] ){
			strcat( buf, "`c   agility`x\n\r" );
		}				
		if ( ch->perm_stats[STAT_SD] < ch->potential_stats[STAT_SD] ){
			strcat( buf, "`c   self-discipline`x\n\r" );
		}
		if ( ch->perm_stats[STAT_ME] < ch->potential_stats[STAT_ME] ){
			strcat( buf, "`c   memory`x\n\r" );
		}				
		if ( ch->perm_stats[STAT_RE] < ch->potential_stats[STAT_RE] ){
			strcat( buf, "`c   reasoning`x\n\r" );
		}
		
/*		if ( ch->pcdata->perm_hit < race_table[ch->race]->max_hp ){
			strcat( buf, "`c   hitpoints`x\n\r" );
		}	

		strcat( buf, "`c   moves`x");
*/
		ch->printlnf( "%s", buf );
		return;
	}
/*	
	if (!str_cmp("hp",argument) || !str_prefix( argument, "hitpoints" ))
	{
		if ( cost > ch->train )
		{
			ch->println( "`cYou don't have enough training sessions.`x" );
			return;
		}
		
		if(ch->pcdata->perm_hit>=race_table[ch->race]->max_hp)
		{
			ch->println( "`cYour hps are already maxed.`x" );
			return;
		}
		
		ch->train -= cost;
		
		amount = number_range(class_table[ch->clss].hp_min,
			class_table[ch->clss].hp_max);
		ch->printlnf( "Your durability increases from %d to %d!",
			ch->pcdata->perm_hit, ch->pcdata->perm_hit+amount);
		ch->pcdata->perm_hit += amount;
		ch->pcdata->perm_hit=UMIN(ch->pcdata->perm_hit, race_table[ch->race]->max_hp);

		ch->max_hit += amount;
		ch->hit += amount;
		act( "`c$n's durability increases!`x",ch,NULL,NULL,TO_ROOM);
		ch->printlnf( "`cYou now have `C%d`c training session%s left.`x",
				ch->train, ch->train==1?"":"s");
		return;
	}

	if (!str_cmp("moves",argument) || !str_cmp( argument, "mv" ))
	{
		int	value = number_range(7, 15);

		if ( cost > ch->train )
		{
			ch->println("`cYou don't have enough training sessions.`x");
			return;
		}
		
		ch->train -= cost;
		ch->printlnf( "Your moves increase from %d to %d!",
			ch->pcdata->perm_move,
			ch->pcdata->perm_move + value);
		ch->pcdata->perm_move += value;
		ch->max_move += value;
		ch->move += value;
		act( "`c$n's endurance increases!`x",ch,NULL,NULL,TO_ROOM);
		ch->printlnf( "`cYou now have `C%d`c training session%s left`x.",
				ch->train, ch->train==1?"":"s");
		return;
	}

	
	if (!str_cmp("mana",argument))
	{
		if ( cost > ch->train )
		{
			ch->println("`cYou don't have enough training sessions.`x");
			return;
		}
		
		ch->train -= cost;
		int increase;
		if(class_table[ch->clss].fMana){
			increase=number_range(6,15);
		}else{
			increase=number_range(3,6);
		}
		ch->printlnf( "`cYour power increases from `C%d`c to`C %d!`x",
			ch->pcdata->perm_mana, ch->pcdata->perm_mana+increase);
		ch->pcdata->perm_mana += increase;
		ch->max_mana += increase;
		ch->mana += increase;
		act( "`c$n's power increases!`x",ch,NULL,NULL,TO_ROOM);
		ch->printlnf( "`cYou now have `C%d`c training session%s left.`x",
				ch->train, ch->train==1?"":"s");
		return;
	}
*/	
	if ( ch->perm_stats[stat]  >= ch->potential_stats[stat] )
	{
		act( "`cYour $T is already at maximum.`x", ch, NULL, pOutput, TO_CHAR );
		return;
	}
	
	if ( cost > ch->train )
	{
		ch->println("`cYou don't have enough training sessions.`x");
		return;
	}
	
	ch->train-= cost;
	
	stat_diff = ch->potential_stats[stat] - ch->perm_stats[stat];
	amount = stat_diff;
	amount = amount * number_range(1,100);
	amount = amount / 100; 
	
	// miniums for large stat differences 
	if (stat_diff>60 && amount<5){
		amount =5;
	}
	if (stat_diff>45 && amount<4){
		amount =4;
	}else if (stat_diff>30 && amount<3){
		amount =3;
	}else if (stat_diff>15 && amount<2){
		amount =2;
	}
	
	if(amount<1) {
		amount=1;
	}
	if(amount>9) {
		amount=9;
	}

	ch->printlnf( "`cYour `C%s`c increases from `C%d`c to `C%d/%d`c!", pOutput, 
		ch->perm_stats[stat], ch->perm_stats[stat]+amount, 
		ch->potential_stats[stat]);
	act( "`c$n's $T increases!`x", ch, NULL, pOutput, TO_ROOM );

	ch->perm_stats[stat] += amount;
	ch->printlnf( "`cYou now have `C%d`c training session%s left.`x",
			ch->train, ch->train==1?"":"s");		
	// patch a bug where a level 1 newbie could be raged 
	// then train to increase stats
	if(ch->pcdata->last_level==0){
		ch->pcdata->last_level=1;
	}
	reset_char(ch);
}

/**************************************************************************/
void do_bank( char_data *ch, char *argument)
{
	char arg[MIL];
	char arg2[MIL];
	int number;
	
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	
	if(arg[0]=='\0')
	{
		ch->println("`cSyntax:  bank <deposit|balance|withdraw> <amount>`x");
		ch->println("`c         There is a fee of 5 gold on any deposit or withdraw.`x");
		ch->println("`c         The bank only deals in gold.`x");
		ch->println("`cNote:    If you are in an army with a bank account setup, your army bank will be used.`x");
		return;
	}

	// check if this is a clan bank, it is, it takes prority over a normal bank
	// note: clan banks shouldn't have the bank room flag set
	if( ch->in_room && ch->clan && ch->clan->m_BankRoom== ch->in_room->vnum)
	{
		// argument has been destroyed previously, so we'll send it piecemeal
		clan_bank( ch, arg, arg2 );
		return;
	}
	
	if( !ch->in_room || !IS_SET(ch->in_room->room_flags,ROOM_BANK) )
	{
		ch->println( "`cYou must be in a bank to make a transaction.`x" );
		return;
	}
	
	if(!str_prefix(arg,"balance") )
	{
		ch->printlnf( "`cYou have an account balance of %ld gold.`x", ch->bank );
		return;
	}     
	
	if(!is_number(arg2))
	{
		ch->println( "`cThe second argument must be a number.`x" );
		return;
	}
	number=atoi(arg2);
	if(number<=0)
	{
		ch->println( "`cThe second argument must be a number greater than 0.`x" );
		return;
	}

	if(number>1250000){
		ch->println( "`cSorry, we dont deal in such large amounts.`x" );
		return;
	}
	
	if(!str_prefix(arg,"deposit"))
	{
		if( number+5>ch->gold )
		{
			
			if( number>ch->gold ){
				ch->println( "`cYou do not have that much money.`x" );
			}else{
				ch->println( "`cWe charge a fee of 5 gold per deposit, banking all that money`x\r\n"
				"`cwill not leave you with enough to pay our deposit fee.`x" );
			}
			return;
		}
		ch->gold-=number+5;
		ch->bank+=number;             
		ch->println( "`cDeposit made.`x\r\n`cThank you for your patronage.`x" );
		save_char_obj(ch); // resave them
		WAIT_STATE(ch, PULSE_PER_SECOND*3);
		return;
	}
	
    if(!str_prefix(arg,"withdraw"))
	{
		if(number<6)
		{
			ch->println("`cDue to the surcharge of 5 gold you can not withdraw that small of an amount.`x");
			return;
		}
		
		if(number>ch->bank-5)
		{
			ch->println("`cYou do not have that much in the bank.`x");
			return;
		}
		ch->gold+=number;
		ch->bank-=number+5;
		ch->println( "`cWithdrawal made.\r\nThank you for your patronage.`x" );
		save_char_obj(ch); // resave them		
		WAIT_STATE(ch, PULSE_PER_SECOND*3);
		return;
	}
	
	ch->printlnf( "`c'%s' is not a valid bank transaction.`x", arg);
	
	return;
}

/**************************************************************************/
// Kalahn - July 98
void do_goooc( char_data *ch, char * )
{
    ROOM_INDEX_DATA *location;

	if (IS_OOC(ch)){
        ch->println("`cYou are already in an OOC room.`x");
        return;
    }

    if (IS_NPC(ch))
    {
		ch->println("`cOnly players can use GoOOC.`x");
		return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
	{
		ch->println("`cYou can't use GoOOC from rooms you can't recall out of.`x");
		return;
	}

	if (ch->pknorecall>0 || ch->pknoquit>0)
	{
		ch->println("`cYou can't use GoOOC while you have a pkill timer.`x");
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CURSE))
	{
		ch->println("`cYou can't use GoOOC while cursed.`x");
		return;
	}


    if ( IS_SWITCHED(ch))
    {
		ch->println("`cGoooc is disabled while you are switched, If you really must move the`x");
		ch->printlnf( "`cmob you are controlling to the ooc rooms type trans self %d.`x", ROOM_VNUM_OOC );
        return;
    }
    
	// find the room and check it is there
	if ( ( location = get_room_index( ROOM_VNUM_OOC )) == NULL )
    {
		ch->println("`cFor some reason the main ooc room is missing!`x");
		ch->printlnf( "`cPlease note the admin (room vnum = %d)`x", ROOM_VNUM_OOC);
        return;
    }

    if (ch->in_room==location)
    {
        ch->println("`cYou are already in that room.`x");
        return;
    }

    if ( ch->fighting )
    {
        ch->println("`cYou can't use GoOOC while fighting!`x");
        return;
    }

    if (ch->mounted_on)
    {
        ch->println("`cClimb off your mount first.`x");
        return;	
	}

	// check for going into ooc from ic
	if (IS_SET(location->room_flags, ROOM_OOC)){
		ch->last_ic_room = ch->in_room;
	}else{
        ch->println("`cFor some reason the main ooc room isnt marked as an OOC room!`x");
        ch->println("`cYou can not be transfered there as such.`x");
		ch->printlnf( "`cPlease note the bug admin (room vnum = %d)`x", ROOM_VNUM_OOC);
		return;
	}
    
    act( "`c$n DISAPPEARS INTO THE OOC ROOMS`x", ch, 0, 0, TO_ROOM );
    ch->println("`cTransferring you to the main OOC room now.`x");
	char_from_room( ch );
    char_to_room( ch, location );
    act( "`c$n APPEARS FROM THE IC REALM`x", ch, 0, 0, TO_ROOM );

    do_look( ch, "auto" );
    return;
}
/**************************************************************************/
// Kal - August 99
void do_fly( char_data *ch, char * )
{
	int launch_cost=10;

	// it is easier for some races to take off
	// originally designed for faeries
	if(IS_SET(race_table[ch->race]->flags, RACEFLAG_LOWCOST_LAUNCH)){
		launch_cost=2;
	}

	if(!IS_SET(race_table[ch->race]->aff,AFF_FLYING)){
		ch->println( "`cYour race can't naturally fly, sorry.`x" );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLY	)){
		ch->println( "`cThe powerful winds prevent you from taking to air.`x" );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_FLYING) ){
		ch->println( "`cYou are already flying!`x" );
		return;
	}

	if (ch->fighting) {
		ch->println( "`cYou attempt to take off the ground, but your opponent prevents you!`x" );
		WAIT_STATE(ch, PULSE_PER_SECOND*3);
		return;
	}

	if(ch->move<launch_cost*3/2){
		ch->println( "`cYou dont feel you have enough energy to take off.`x" );
		return;
	}
			
	do_visible(ch,"auto");

	ch->move-=launch_cost;
	
	act( "`c$n launches $mself into the air.`x", ch, NULL, NULL, TO_ROOM );
	ch->println("You launch yourself into the air.");
	SET_BIT(ch->affected_by,AFF_FLYING);
	SET_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);
}
/**************************************************************************/
void landchar( char_data *ch)
{
	// loop thru removing all fly affects
    AFFECT_DATA *paf, *paf_next;   
    for ( paf= ch->affected; paf; paf = paf_next )
    {
		paf_next=paf->next;
        if ( paf->bitvector== AFF_FLYING){
			affect_remove( ch, paf );
		}
    }

	REMOVE_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);	
	REMOVE_BIT(ch->affected_by, AFF_FLYING);
}
/**************************************************************************/
DECLARE_SPELL_FUN( spell_null );
/**************************************************************************/
// Kal - August 99
void do_land( char_data *ch, char * )
{
	if ( !IS_AFFECTED(ch, AFF_FLYING) ){
		ch->println("`cYou arent currently flying!`x");
		return;
	}

	landchar(ch);

	act( "`c$n returns to the ground.`x", ch, NULL, NULL, TO_ROOM );
	ch->println("`cYou land on the ground.`x");
}
/**************************************************************************/
// the below code could be used to give ideas for how to make the magic
// system prevent players from casting the same spell_function on others
// twice under different names
/*	// loop thru all the spells finding any that use spell_fly,
	// and if we find any remove them if ch is affected by that spell
	SPELL_FUN * spell_fun=skill_table[gsn_fly].spell_fun;
	if(spell_fun && spell_fun!=spell_null){	
		for ( int sn = FIRST_SPELL; sn < LAST_SPELL+1; sn++ )
		{
			if ( skill_table[sn].name == NULL )
				break;

			// we have a spell that uses the spell_fly spell_function
			if(skill_table[sn].spell_fun==spell_fun)
			{
				if(is_affected(ch, sn)){
					paf=affect_find(ch->affected, sn);
					affect_remove( ch, paf );
					break;
				}
			}				
		}
	}				
	spell_fun=skill_table[gsn_animal_essence].spell_fun;
	if(spell_fun && spell_fun!=spell_null){	
		for ( int sn = FIRST_SPELL; sn < LAST_SPELL+1; sn++ )
		{
			if ( skill_table[sn].name == NULL )
				break;

			// we have a spell that uses the spell_animal_essence spell_function
			if(skill_table[sn].spell_fun==spell_fun)
			{
				if(is_affected(ch, sn)){
					paf=affect_find(ch->affected, sn);
					affect_remove( ch, paf );
					break;
				}
			}				
		}
	}				
*/
/**************************************************************************/
void do_speedwalk( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg[MIL];
	char *direction;
	bool found = false;

	if ( !ch->desc || IS_NULLSTR( argument ))
	{
		ch->println("`cYou must include directions.  Read `Rhelp speedwalk`c for more information.`x");
		return;
	}

	buf[0] = '\0';

	while ( *argument != '\0' )
	{
		argument = one_argument( argument, arg );
		strcat( buf, arg );
	}

	for ( direction = buf + str_len(buf)-1; direction >= buf; direction-- )
	{
		if ( !is_digit( *direction))
		{
			switch ( *direction )
			{
			case 'n':
			case 'e':
			case 's':
			case 'w':
			case 'r':		// Northwest
			case 't':		// Northeast
			case 'g':		// Southeast
			case 'f':		// Southwest
			case 'u':
			case 'd':
				found = true;
				break;
			case 'o':
				break;
			default:
				ch->println("`cInvalid direction!`x");
				return;
			}
		}
		else if (!found)
			*direction ='\0';
	}

	if ( !found )
	{
		ch->println("`cNo directions specified.`x");
		return;
	}

	ch->desc->speedwalk_buf  = str_dup( buf );
	ch->desc->speedwalk_head = ch->desc->speedwalk_buf;
	ch->println("`cYou start to walk...`x");
	return;
}

/**************************************************************************/
void do_knock( char_data *ch, char *argument )
{
	char arg[MIL];
	int door;

	one_argument( argument, arg );

	if ( IS_NULLSTR( arg ))
	{
		ch->println( "`cKnock on what?`x" );
		return;
	}

	if ( ch->fighting ) // no knocking while fighting
	{
		ch->println( "`cYou have better things to do with your knuckles right now.`x" );
		return;
	}

	if (( door = find_door( ch, arg, "knock on")) >= 0 )
	{
		ROOM_INDEX_DATA	*to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev=NULL;
		
		pexit = ch->in_room->exit[door];

		if ( !IS_SET( pexit->exit_info, EX_CLOSED )) // door already open
		{
			ch->println( "`cWhy knock?  It's open.`x" );
			return;
		}
		// door closed, so lets knock
		act( "`cYou knock loudly on the $d.`x", ch, NULL, pexit->keyword, TO_CHAR );
		act( "`c$n knocks loudly on the $d.`x", ch, NULL, pexit->keyword, TO_ROOM );

		if ((    to_room = pexit->u1.to_room ) != NULL
			&& ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
			&&   pexit_rev->u1.to_room == ch->in_room )
		{
			char_data	*rch;

			for ( rch = to_room->people; rch; rch = rch->next_in_room )
			{
				act( "`cSomeone knocks loudly from the other side of the $d.`x",
					rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
		}
	}
	else
	{
		act( "`cYou knock on your forehead but nobody answers.`x", ch, NULL, NULL, TO_CHAR );
		act( "`c$n knocks on $s forehead but nobody answers.`x", ch, NULL, NULL, TO_ROOM );
	}

	return;
}
/**************************************************************************/
void do_invitelist(char_data *ch, char *argument)
{
	if(IS_NPC(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}

	if(GAMESETTING4(GAMESET4_ROOM_INVITES_DISABLED)){
		ch->println("`cThe room invites system is currently disabled.`x");
		return;
	}

	ROOM_INDEX_DATA *room=ch->in_room;

    if(IS_NULLSTR(room->owner)){
		if(IS_ADMIN(ch)){
			ch->println("`cThere are no owners in this room, therefore invite lists are ignored.`x");
		}else{
			ch->println("`cYou are not the owner of this room, so you can't modify its invite list.`x");
		}	
		return;
	}

	if(!is_exact_name(ch->name, room->owner) && !IS_ADMIN(ch)){
		ch->println("`cYou are not the owner of this room, so you can't modify its invite list.`x");
		return;
	}

	char arg[MIL];
	one_argument(argument, arg);
	smash_tilde(arg);

	if(IS_NULLSTR(arg)){	
		if(IS_ADMIN(ch)){
			ch->printlnf("`cCurrent room %d owner = '%s'`x", room->vnum, room->owner);
		}
		ch->printlnf("`cCurrent room invite list = '%s'`x", ltrim_string(rtrim_string(room->invite_list)));
		ch->println("`cType 'invitelist <playername>' to add or remove it from this list.`x");
		ch->println("`cType 'invitelist clan=<clanname>' to add or remove clan from this list.`x");
		return;
	}

	if(has_colour(arg)){
		ch->println("You can't use colour codes here.");
		return;
	}

	if(is_exact_name(arg, room->invite_list)){
		ch->printlnf("`cRemoving '%s' from room invite list.`x", arg);
		// remove them from the rooms invite list
		room->invite_list=string_replace_all(room->invite_list, FORMATF(" %s ", arg), " ");
		room->invite_list=string_replace_all(room->invite_list, "  ", " ");
		ch->printlnf("`xRoom invite list now: '%s'`x", ltrim_string(rtrim_string(room->invite_list)));
		SET_BIT( room->area->olc_flags, OLCAREA_INVITELISTCHANGED );
		return;
	}else{
		// add them to the rooms invite list
		if(str_len(room->invite_list)> MIL){
			ch->println("`cToo many names listed in room invite list, remove some first.`x");
			ch->printlnf("`cRoom invite list now: '%s'`x", ltrim_string(rtrim_string(room->invite_list)));
			return;
		}

		ch->printlnf("`cAdding '%s' to room invite list.`x", arg);
		if(IS_NULLSTR(room->invite_list)){
			replace_string(room->invite_list, FORMATF(" %s ", arg));
		}else{
			char *f=FORMATF("%s%s ", room->invite_list, arg);
			replace_string(room->invite_list, f);
		}
		ch->printlnf("`cRoom invite list now: '%s'`x", ltrim_string(rtrim_string(room->invite_list)));
		SET_BIT( room->area->olc_flags, OLCAREA_INVITELISTCHANGED );
		return;
	}
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
