/*
olympics.cpp
- Hera, of Athens - The Mud athens.boue.ca port 9000 
  hera_of_athens@yahoo.com
*/

#include "include.h" 
DECLARE_DO_FUN( do_race2); 
DECLARE_DO_FUN( do_race3); 
DECLARE_DO_FUN( do_race4); 
DECLARE_DO_FUN( do_race5); 
DECLARE_DO_FUN( do_race6); 

void do_race( char_data *ch, char * )
{ 
   OBJ_DATA *obj;
   int raceevent;

if (!IS_NPC(ch))
{ 
   	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
   	{
	if ( obj->item_type == ITEM_FINISH )
		break;
	}		
	if ( obj == NULL )
	{
		ch->println( "There is not a race occuring here." );
		return;
	}
		ch->println( "With the drop of flag, the race starts!" );
	raceevent = dice(1,3); 
   	switch(raceevent) 
		{
		case 1:
	    		ch->println(3,"You push yourself forward as your feet slide into the earth.\n\r"); 
			do_race2(ch,"");
			break;
    					
		case 2:
	    		ch->println(2,"Using your feet to spring forward, you accelerate quickly.\n\r"); 
			do_race2(ch,"");
			break;
 	
		case 3:
	    		ch->println(1,"You leap ahead from the finish line with gusto!\n\r"); 
			do_race2(ch,"");
			return;
		}
}
	else
		ch->println( "Mobiles don't need to compete in races." );

}

/***************************************************************************/
void do_race2( char_data *ch, char * )
{ 
   int raceevent2;

if (!IS_NPC(ch))
{ 
	raceevent2 = dice(1,3); 
   	switch(raceevent2) 
		{
		case 1:
	    		ch->println(6,"You struggle to right yourself from a crouched start.\n\r"); 
			do_race3(ch,"");
			break;
    					
		case 2:
	    		ch->println(5,"As you stand more upright, your stride evens out.\n\r"); 
			do_race3(ch,"");
			break;
 	
		case 3:
	    		ch->println(4,"Quickly straightening yourself out, you start off well.\n\r"); 
			do_race3(ch,"");
				return;
		}
}
	else
		ch->println( "Mobiles don't need to compete in races." );

}
/***************************************************************************/
void do_race3( char_data *ch, char * )
{ 
   int raceevent3;

if (!IS_NPC(ch))
{ 
	raceevent3 = dice(1,3); 
   	switch(raceevent3) 
		{
		case 1:
	    		ch->println(9,"As your arms swing loosely, you catch wind.\n\r"); 
			do_race4(ch,"");
			break;
    					
		case 2:
	    		ch->println(8,"You tuck your arms at your sides, creating less drag.\n\r"); 
			do_race4(ch,"");
			break;
 	
		case 3:
	    		ch->println(7,"Your arms pump back and forth steadily, gaining you momentum.\n\r"); 
			do_race4(ch,"");
				return;
		}
}
	else
		ch->println( "Mobiles don't need to compete in races." );

}
/***************************************************************************/
void do_race4( char_data *ch, char * )
{ 
   int raceevent4;

if (!IS_NPC(ch))
{ 
	raceevent4 = dice(1,3); 
   	switch(raceevent4) 
		{
		case 1:
	    		ch->println(12,"Pushing your legs to go faster is tiring you.\n\r"); 
			do_race5(ch,"");
			break;
    					
		case 2:
	    		ch->println(11,"You've found a good, steady pace at which to run.\n\r"); 
			do_race5(ch,"");
			break;
 	
		case 3:
	    		ch->println(10,"The momentum of your running seems to grow.\n\r"); 
			do_race5(ch,"");
				return;
		}
}
	else
		ch->println( "Mobiles don't need to compete in races." );

}
/***************************************************************************/
void do_race5( char_data *ch, char * )
{ 
   int raceevent5;

if (!IS_NPC(ch))
{ 
	raceevent5 = dice(1,3); 
   	switch(raceevent5) 
		{
		case 1:
	    		ch->println(15,"The finish line appears ahead of you.\n\r"); 
			do_race6(ch,"");
			break;
    					
		case 2:
	    		ch->println(14,"You quickly approach the finish line.\n\r"); 
			do_race6(ch,"");
			break;
 	
		case 3:
	    		ch->println(13,"The finish line seems to race to meet you.\n\r"); 
			do_race6(ch,"");
				return;
		}

}
	else
		ch->println( "Mobiles don't need to compete in races." );

}

/***************************************************************************/
void do_race6( char_data *ch, char * )
{ 

if (!IS_NPC(ch))
{ 
			act(16,"$n crosses the finish line!`x", ch ); 

}
	else
		ch->println( "Mobiles don't need to compete in races." );

}

