/*
flag.cpp
- Hera, of Athens - The Mud athens.boue.ca port 9000 
  hera_of_athens@yahoo.com
*/

#include "include.h" 

void do_aflag(char_data *ch, char *argument)
{ 
    OBJ_DATA *obj;
	if (IS_NULLSTR(argument))
	{
		ch->println( "`cWhich aflag command?  Raise, Recover, Swipe, Display?`x" );
		return;
    	}
	if(!str_cmp("raise", argument))
	{	
		if(IS_SET(ch->in_room->room2_flags, ROOM2_ATHENSFLAG)) 
		{
			for ( obj = ch->carrying; obj; obj = obj->next_content )
			{
     				if ( obj->item_type == ITEM_ATHENSFLAG && obj->wear_loc == WEAR_HOLD )
           			break;
			}
			if ( !obj )
			{
     			ch->println("You are not holding the Athenian flag.");
     			return;
			}else{ 
				ch->println( "`cYou proudly fly your city flag.`x" );
				act( "$n proudly raises to Athens City Flag.",ch,NULL,NULL,TO_ROOM);
				return;
			}
		}
    			ch->println( "`cYou are not at the Athenian Flag Pole.`x" );
			return;
	}
	if(!str_cmp("recover", argument))
	{	
		if(GAMESETTING(GAMESET2_MEGARAFLAG_ATHENS))
		{

			ch->println( "`cYou attempt to recover the Megara City Flag.`x" );
			act( "$n attempts to recover the Megara City Flag.",ch,NULL,NULL,TO_ROOM);
			return;
		}
    			ch->println( "`cThe Megarian City Flag is not in the possession of Athens.`x" );
			return;
	}
}
