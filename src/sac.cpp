/*
sac.cpp
- Hera, of Athens - The Mud athens.boue.ca port 9000 
  hera_of_athens@yahoo.com
*/
 
#include "include.h" 

void do_sacrifice( char_data *ch, char *argument ) 
{ 
    char arg[MIL]; 
    OBJ_DATA *obj; 

    one_argument( argument, arg ); 

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) ) 
    { 
   act( "`c$n offers $mself to the gods, who graciously decline.`x", 
       ch, NULL, NULL, TO_ROOM ); 
   act( 
       "`cThe gods appreciate your offer and may accept it later.`x", ch, NULL, NULL, TO_CHAR ); 
   return; 
    } 

    obj = get_obj_list( ch, arg, ch->in_room->contents ); 
    if ( obj == NULL ) 
    { 
   ch->printlnf( "`cYou can't find it.`x\n\r"); 
   return; 
    } 

    if ( obj->item_type == ITEM_CORPSE_PC ) 
    { 
   if (obj->contains) 
        { 
      ch->printlnf( 
        "`cThe gods would not appreciate that.`x"); 
      return; 
        } 
    } 


    if ( !obj->item_type == ITEM_FOOD ) 
    { 
   ch->printlnf( "`cThat is not an acceptable sacrifice.`x" ); 
   return; 
    } 


     if ( obj->item_type == ITEM_FOOD ) 
	{   
        ch->printlnf( 
       	"`mYou send up a burnt offering to the gods`m.`x"); 
   	act( "`c$n's offering sends a spiral of smoke to the gods.`x", 
       		ch, NULL, NULL, TO_ROOM );     
   	wiznet("$N sends up $p as a burnt offering.", ch,obj,0,0,0); 
    	extract_obj( obj ); 
    	return; 
	}else{
   	ch->printlnf( "`cThat is not an acceptable sacrifice.`x" ); 
   	return; 
    	} 
}