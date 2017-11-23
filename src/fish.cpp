
/*fish.cpp
Upon not being able to find a snippet being publicly shared to do this,
I decided to try to write something simple myself.
- Hera, of Athens - The Mud athens.boue.ca port 9000 
  hera_of_athens@yahoo.com*/

 
#include "include.h"

void do_fish( char_data *ch, char * )
{
   OBJ_DATA *obj;
   OBJ_DATA *fishcatch;
   int fishevent;


	if(!IS_SET(ch->in_room->room2_flags, ROOM2_FISH))
	{
        	ch->println("`cYou are not at a fishing site.`x");
        	return;
	}

 	if (!IS_NPC(ch)) 
 	{ 
		for ( obj = ch->carrying; obj; obj = obj->next_content )
    		{
        	if ( obj->item_type == ITEM_POLE && obj->wear_loc == WEAR_HOLD )
            break;
    		}
    
		if ( !obj )
    		{
        		ch->println("`cYou are not holding a fishing pole.`x");
        		return;
    		}else{ 

			for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
			{
				if ( obj->item_type == ITEM_FISH )
				extract_obj( obj );
				break;
			}
		
			if ( obj == NULL )
			{
				ch->println( "`cThere are no fish to catch here.`x" );
				return;
			}


   			fishevent = dice(1,3); 
   			switch(fishevent) 
				{
				case 1:
	    			ch->println("`cYou get a nibble but the fish leaves without taking the bait.`x\n\r"); 
				break;
    					
				case 2:
	    			ch->println("`cA fish steals your bait without getting hooked!`x\n\r"); 
				break;    
	
				case 3:
	    			if (get_obj_index(OBJ_VNUM_FISHCATCH)) {
					fishcatch = create_object(get_obj_index(OBJ_VNUM_FISHCATCH));
					ch->println("A fish takes the bait and becomes hooked.\n\r");
					obj_to_char(fishcatch,ch);
				}else{
        			ch->println( "BUG: No available catch object for fishing - please report!" );
				return;
				}



				
				}
   		}
	}
	else
		ch->println( "Mobiles don't need to fish." );

}
