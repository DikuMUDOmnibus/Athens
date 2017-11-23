/*
mine.cpp
Upon not being able to find a snippet being publicly shared to do this,
I decided to try to write something simple myself.
- Hera, of Athens - The Mud athens.boue.ca port 9000 
  hera_of_athens@yahoo.com
*/
 
#include "include.h" 

void do_mine( char_data *ch, char * )
{ 
   OBJ_DATA *obj;
   OBJ_DATA *gemstone;
   int mineevent;

	if(!IS_SET(ch->in_room->room2_flags, ROOM2_MINE))
	{
        	ch->println("You are not in a mine.");
        	return;
	}

 	if (!IS_NPC(ch)) 
 	{ 
		for ( obj = ch->carrying; obj; obj = obj->next_content )
    		{
        	if ( obj->item_type == ITEM_PICK && obj->wear_loc == WEAR_HOLD )
            break;
    		}
    
		if ( !obj )
    		{
        		ch->println("You are not holding a pick.");
        		return;
    		}else{ 

			for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
			{
				if ( obj->item_type == ITEM_ORE )
				extract_obj( obj );
				break;
			}
		
			if ( obj == NULL )
			{
				ch->println( "This is not a suitable place to mine." );
				return;
			}


   			mineevent = dice(1,9); 
   			switch(mineevent) 
				{
				case 1:
	    			ch->println("Your pick strikes a soft ore.  It is gold!\n\r"); 
				ch->gold=ch->gold+1;
				break;
    					
				case 2:
	    			ch->println("Your pick strikes a soft ore.  It is silver!\n\r"); 
				ch->silver=ch->silver+50;
				break; 
   
                                case 3:
                                ch->println("Your pick strikes a soft ore.  It is gold!\n\r");
                                ch->gold=ch->gold+2;
                                break;

                                case 4:
                                ch->println("Your pick strikes a soft ore.  It is silver!\n\r");
                                ch->silver=ch->silver+25;
                                break;

                                case 5:
                                ch->println("Your pick strikes a soft ore.  It is gold!\n\r");
                                ch->gold=ch->gold+3;
                                break;

                                case 6:
                                ch->println("Your pick strikes a soft ore.  It is silver!\n\r");
                                ch->silver=ch->silver+75;
                                break;

                                case 7:
                                ch->println("Your pick strikes a soft stone.  It contains nothing of value.\n\r");
                                break;

                                case 8:
                                ch->println("Your pick strikes a hard stone.  It contains nothing of value.\n\r");
                                break;
	
				case 9:
	    			if (get_obj_index(OBJ_VNUM_GEMSTONE)) {
					gemstone = create_object(get_obj_index(OBJ_VNUM_GEMSTONE));
					ch->println("Your pick strikes a very hard stone.  It is a gem!\n\r");
					obj_to_char(gemstone,ch);
				}else{
        			ch->println( "BUG: No available gemstone object for mining - please report!" );
				return;
				}



				
				}
   		}
	}
	else
		ch->println( "Mobiles don't need to mine." );

}
