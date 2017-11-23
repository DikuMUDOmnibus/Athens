/**************************************************************************/ 
// vehicle.cpp - Vehicle movement & Combat by Ixliam 
/*************************************************************************** 
 * Whispers of Times Lost (c)1998-2004 Brad Wilson (ixliam@wotl.org)       * 
 * >> If you use this code you must give credit in your help file as well  * 
 *    as leaving this header intact.                                       * 
 * >> To use this source code, you must fully comply with all the licenses * 
 *    in licenses below. In particular, you may not remove this copyright  * 
 *    notice.                                                              * 
 *************************************************************************** 
 * The Dawn of Time v1.69r (c)1997-2004 Michael Garratt                    * 
 * >> A number of people have contributed to the Dawn codebase, with the   * 
 *    majority of code written by Michael Garratt - www.dawnoftime.org     * 
 * >> To use this source code, you must fully comply with the dawn license * 
 *    in licenses.txt... In particular, you may not remove this copyright  * 
 *    notice.                                                              * 
 **************************************************************************/ 

#include "include.h" 
#include "vehicle.h" 

DECLARE_DO_FUN( do_look      ); 
bool laston_find(char *argument); 

/**************************************************************************/ 
// semilocalized globals 
VEHICLE_DATA   *vehicle_list; 
/**************************************************************************/ 
// create vehicle GIO lookup table 

GIO_START(VEHICLE_DATA) 
GIO_INTH(reset,      "Reset ") 
GIO_INTH(vnum,      "Vnum ") 

GIO_FINISH_STRDUP_EMPTY 
/**************************************************************************/ 
// loads in the vehicle database 
void load_vehicle_db(void) 
{ 
   VEHICLE_DATA *node; 
   OBJ_DATA *obj; 
   room_index_data *room=NULL; 

   logf("===Loading vehicle database from %s...", VEHICLE_FILE); 
   GIOLOAD_LIST(vehicle_list, VEHICLE_DATA, VEHICLE_FILE); 

   for (node = vehicle_list; node; node= node->next) 
   { 
      room = get_room_index(node->reset); 
      if(room) 
      { 
         obj = create_object(get_obj_index(node->vnum)); 
         obj_to_room(obj, room); 
      } 
   } 
   log_string ("load_vehicle_db(): finished"); 
} 
/**************************************************************************/ 
// saves the vehicle database 
void save_vehicle_db( void) 
{ 
   logf("===save_vehicle_db(): saving vehicle database to %s", VEHICLE_FILE); 
   GIOSAVE_LIST(vehicle_list, VEHICLE_DATA, VEHICLE_FILE, true); 
} 
/**************************************************************************/ 
// lists vehicles 
void do_vehiclelist( char_data *ch, char *) 
{ 
   VEHICLE_DATA *node; 
   int count; 

   ch->titlebar("-=VEHICLE RESETS=-"); 

   count=0; 
   for (node = vehicle_list; node; node= node->next) 
   { 
       ch->printlnf("`s%d> `MRoom: `m%d  `MVnum: `m%d", ++count, node->reset, node->vnum); 
   } 
   return; 
} 
/**************************************************************************************/ 
void queue_vehicle(int reset, int vnum) 
{ 
   VEHICLE_DATA *node = new VEHICLE_DATA; 
   node->reset = reset; 
   node->vnum = vnum; 
   node->next = vehicle_list; 
   vehicle_list = node; 
   save_vehicle_db(); 
} 
/**************************************************************************/ 
EXTRA_DESCR_DATA *ed_prefix(EXTRA_DESCR_DATA *ed, char *keyword, char *description); 
/**************************************************************************/ 
/**************************************************************************/ 
void do_vehicle_add(char_data *ch, char *argument) 
{ 
    OBJ_DATA *obj = NULL; 
    char arg1[MIL]; 

    argument = one_argument( argument, arg1 ); 

    obj = get_obj_list( ch, arg1, ch->in_room->contents ); 

    if (obj == NULL ) 
    { 
   ch->println("No vehicle here to add to database."); 
   return; 
    } 

    if ( obj->item_type!=ITEM_VEHICLE) 
    { 
   ch->println("That is not a vehicle."); 
   return; 
    } 

    queue_vehicle(ch->in_room->vnum, obj->pIndexData->vnum); 
    ch->printlnf("%s `W[`S%d`W] `wadded in room %d.",obj->short_descr, obj->pIndexData->vnum, ch->in_room->vnum); 
    return; 
} 
/**************************************************************************/ 
int get_direction( char *arg ) 
{ 
   if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) return 0; 
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) return 1; 
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) return 2; 
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) return 3; 
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) return 4; 
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) return 5; 
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) return 6; 
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) return 7; 
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) return 8; 
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) return 9; 
    return -1; 
} 
/**************************************************************************/ 
void travelecho( char_data *ch, OBJ_DATA *vehicle ) 
{ 
   int chance = 0; 
   for(char_data *vch = player_list; vch; vch = vch->next_player) 
   { 
   if(vch==ch) 
   { 
         if(IS_IMMORTAL(vch) || IS_SET(ch->comm,COMM_BUILDING)) 
         { 
         vch->print("`#`STravelEcho: `^"); 
         } 
         vch->print(get_extra_descr("_TRAVEL", vehicle->pIndexData->extra_descr)); 
         vch->println("`x"); 
      continue; 
   } 

   if(ch->in_room->area==vch->in_room->area && vch->in_room->sector_type == SECT_VEHICLE) 
   { 
      chance = number_range(1,2); 
      if(vch!=ch && chance==1) 
      { 
           if(IS_IMMORTAL(vch) || IS_SET(vch->comm,COMM_BUILDING)) 
            { 
            vch->print("`#`STravelEcho: `^"); 
            } 
            vch->print(get_extra_descr("_TRAVEL", vehicle->pIndexData->extra_descr)); 
            vch->println("`x"); 
      } 
   } 
   } 
   return; 
} 
/**************************************************************************/ 
void do_drive( char_data *ch, char *argument ) 
{ 
    char_data *vch; 
    ROOM_INDEX_DATA *in_room; 
    ROOM_INDEX_DATA *to_room; 
    EXIT_DATA *pexit; 
    char arg1[MIL]; 
    OBJ_DATA *controls; 
    OBJ_DATA *vehicle = NULL; 
    int direction; 
    int dir_name_2; 

    for ( controls = ch->in_room->contents; controls != NULL; controls = controls->next_content ) 
    { 
   if ( controls->item_type == ITEM_VEHICLE_CONTROLS ) 
       break; 
    } 

    if ( controls == NULL ) 
    { 
   ch->println("Nothing to drive here."); 
   return; 
    } 

    if ( ( vehicle = get_obj_type( get_obj_index( controls->value[0] ) ) ) == NULL ) 
    { 
   ch->println("Error, can't find vehicle for control panel." ); 
   return; 
    } 

    if ( vehicle->item_type != ITEM_VEHICLE ) 
    { 
   do_huh(ch, ""); 
   return; 
    } 

    if ( vehicle->in_room == NULL ) 
    { 
   ch->println("Error, vehicle room is null." ); 
   return; 
    } 

    in_room = vehicle->in_room; 

    argument = one_argument( argument, arg1 ); 

    direction = get_direction( arg1 ); 

    if ( arg1[0] == '\0' ) 
   direction = -1; 

    if ( direction == -1 ) 
    { 
   ch->println("Drive where ?" ); 
   return; 
    } 

    if ( ( pexit   = in_room->exit[direction] ) == NULL 
   ||   ( to_room = pexit->u1.to_room   ) == NULL 
   ||   !can_see_room(ch,pexit->u1.to_room)) 
    { 
   ch->println("There is no way to travel in that direction." ); 
   return; 
    } 

    if (IS_SET(pexit->exit_info, EX_ISDOOR) && to_room->sector_type!=SECT_CITY) 
    { 
   ch->println("You cannot drive through there." ); 
   return; 
    } 

    if (IS_SET(to_room->room_flags, ROOM_INDOORS) && !IS_SET(vehicle->value[2], VEHICLE_CAVE)) 
    { 
   ch->println("You cannot drive indoors." ); 
   return; 
    } 

    if (IS_SET(pexit->exit_info, EX_LOCKED)) 
    { 
   ch->println("That direction seems to be barred to you." ); 
   return; 
    } 

   if( to_room->sector_type==SECT_LAVA && !IS_SET(vehicle->value[2], VEHICLE_LAVA)) 
   { 
      ch->println("You would burn there."); 
      return; 
   } 

   if( to_room->sector_type==SECT_WATER_NOSWIM && !IS_SET(vehicle->value[2], VEHICLE_WATER)) 
   { 
      ch->println("You are not in a boat."); 
      return; 
   } 

   if( to_room->sector_type==SECT_AIR && !IS_SET(vehicle->value[2], VEHICLE_AIR)) 
   { 
      ch->println("This vehicle cannot fly."); 
      return; 
   } 

   if( to_room->sector_type==SECT_CAVE && !IS_SET(vehicle->value[2], VEHICLE_CAVE)) 
   { 
      ch->println("This vehicle cannot travel underground."); 
      return; 
   } 

   if( to_room->sector_type==SECT_INSIDE) 
   { 
      ch->println("You cannot go there."); 
      return; 
   } 

   if ( to_room->sector_type==SECT_UNDERWATER ||IS_SET( to_room->affected_by, ROOMAFF_UNDERWATER )) 
   { 
      ch->println("This vehicle cannot go underwater."); 
      return; 
   } 

   if((to_room->sector_type==SECT_CITY || to_room->sector_type==SECT_FIELD || 
       to_room->sector_type==SECT_FOREST || to_room->sector_type==SECT_HILLS || 
       to_room->sector_type==SECT_MOUNTAIN || to_room->sector_type==SECT_SWAMP || 
       to_room->sector_type==SECT_DESERT || 
       to_room->sector_type==SECT_SNOW || to_room->sector_type==SECT_ICE || 
       to_room->sector_type==SECT_TRAIL || to_room->sector_type==SECT_GATEWAY ) && 
       !IS_SET(vehicle->value[2], VEHICLE_LAND)) 
   { 
      ch->println("This vehicle cannot travel on land."); 
      return; 
   } 

    for ( vch = in_room->people; vch != NULL; vch = vch->next_in_room ) 
    { 
   if ( can_see_obj( vch, vehicle ) ) 
       vch->printlnf( "%s leaves %s.",  capitalize(vehicle->short_descr), dir_name[direction] ); 
    } 

    obj_from_room( vehicle ); 
    obj_to_room( vehicle, to_room ); 

    dir_name_2 = rev_dir[direction]; 

    for ( vch = to_room->people; vch != NULL; vch = vch->next_in_room ) 
    { 
   if ( can_see_obj( vch, vehicle ) ) 
       vch->printlnf( "%s has arrived from %s.", capitalize(vehicle->short_descr), dir_name[dir_name_2] ); 
    } 

    bool travel = false; 

    if( to_room->sector_type==SECT_AIR && IS_SET(vehicle->value[2], VEHICLE_AIR) && travel==false) 
    { 
      ch->printlnf( "You fly %s %s.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (IS_SET(vehicle->value[2], VEHICLE_LAND) && to_room->sector_type==SECT_SNOW && travel==false) 
    { 
       ch->printlnf( "You drive %s %s through the snow.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (IS_SET(vehicle->value[2], VEHICLE_LAND) && to_room->sector_type==SECT_SNOW && travel==false) 
    { 
       ch->printlnf( "You drive %s %s through the ice.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (IS_SET(vehicle->value[2], VEHICLE_LAND) && to_room->sector_type!=SECT_WATER_NOSWIM && travel==false) 
    { 
       ch->printlnf( "You drive %s %s.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (IS_SET(vehicle->value[2], VEHICLE_WATER) && to_room->sector_type==SECT_WATER_NOSWIM && travel==false)    
    { 
   ch->printlnf( "You sail %s %s.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (IS_SET(vehicle->value[2], VEHICLE_WATER) && to_room->sector_type==SECT_WATER_SWIM && travel==false)    
    { 
   ch->printlnf( "You sail %s %s through the shallow water.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    if (travel==false) 
    { 
       ch->printlnf( "You move %s %s.", vehicle->short_descr, dir_name[direction]  ); 
   travel = true; 
    } 

    travelecho(ch, controls); 
    in_room = ch->in_room; 
    char_from_room( ch ); 
    char_to_room( ch, to_room ); 
    do_look( ch, "" ); 
    char_from_room( ch ); 
    char_to_room( ch, in_room ); 

    return; 
} 
/**************************************************************************/ 
void do_board( char_data *ch, char *argument ) 
{ 
    char arg1[MIL]; 
    ROOM_INDEX_DATA *to_room; 
    OBJ_DATA *building, *building_next = NULL; 
    OBJ_DATA *vehicle = NULL; 

    argument = one_argument( argument, arg1 ); 

    if ( IS_NULLSTR(arg1)) 
    { 
   for ( building = ch->in_room->contents; building != NULL; building = building_next ) 
       { 
      building_next = building->next_content; 

      if ( building->item_type!=ITEM_VEHICLE_EXIT) 
         continue; 
      if ( building->item_type==ITEM_VEHICLE_EXIT) 
          { 
             if ( ( vehicle = get_obj_type( get_obj_index( building->value[0] ) ) ) == NULL ) 
             { 
            ch->println("Error, can't find ship for vehicle exit." ); 
            return; 
             } 
             act("You leave $p.", ch, vehicle, NULL, TO_CHAR ); 
             act("$n leaves $p.", ch, vehicle, NULL, TO_ROOM ); 

             char_from_room( ch ); 
             char_to_room( ch, get_room_index(vehicle->in_room->vnum)); 
             do_look( ch, "" ); 

             act("$n has arrived from $p.", ch, vehicle, NULL, TO_ROOM); 
         return; 
      } 
   } 
    } 

    building = get_obj_list( ch, arg1, ch->in_room->contents ); 
    if ( building == NULL ) 
    { 
   ch->println("Nothing like that to board here."); 
   return; 
    } 

   if ( building->item_type==ITEM_VEHICLE_EXIT) 
       { 
          if ( ( vehicle = get_obj_type( get_obj_index( building->value[0] ) ) ) == NULL ) 
          { 
         ch->println("Error, can't find vehicle for this exit." ); 
         return; 
          } 
          act("You leave $p.", ch, vehicle, NULL, TO_CHAR ); 
          act("$n leaves $p.", ch, vehicle, NULL, TO_ROOM ); 
          char_from_room( ch ); 
      char_to_room( ch, get_room_index(vehicle->in_room->vnum)); 
      do_look( ch, "" ); 
      act("$n has arrived from $p.", ch, vehicle, NULL, TO_ROOM); 
      return; 
   } 

    if ( building->item_type!=ITEM_VEHICLE) 
    { 
        ch->println("Nothing to board here." ); 
   return; 
    } 

    if ( ( to_room = get_room_index( building->value[0] ) ) == NULL ) 
    { 
   ch->println("Error, bad room pointed to by building object." ); 
   return; 
    } 

    act("You board $p.", ch, building, NULL, TO_CHAR ); 
    act("$n boards $p.", ch, building, NULL, TO_ROOM ); 

    char_from_room( ch ); 
    char_to_room( ch, to_room); 
    do_look( ch, "" ); 

    act("$n has arrived from outside.", ch, NULL, NULL, TO_ROOM); 

    return; 
} 
/**************************************************************************/ 
void do_vehicle_delete( char_data *ch, char *argument ) 
{ 
   VEHICLE_DATA *node; 
   VEHICLE_DATA *prevnode=NULL; 
   int target, count=1; 
    
   if(vehicle_list==NULL){ 
      ch->println("There are no vehicles in the database."); 
      return; 
   } 
    
   if(IS_NULLSTR(argument)){ 
      ch->println("syntax: vehicledelete <vehicle_number>"); 
      return; 
   }; 
    
   if(!is_number(argument)){ 
      ch->print("Argument to vehicledelete must be a number."); 
      return; 
   } 
    
   target=atoi(argument); 
    
   for (node=vehicle_list;node;node=node->next, ++count){ 
      if(count==target) 
         break; 
      prevnode = node; 
   } 
    
   if(node==NULL){ 
      ch->printf(0, "Vehicle not found."); 
      return; 
   } 
    
   ch->printf(0, "Vehicle Vnum #%d has been deleted.\r\n", node->vnum); 
    
   if(!prevnode){ // delete the head 
      vehicle_list = vehicle_list->next; 
   }else{ 
      prevnode->next=node->next; 
   } 
    
   delete node; 
   save_vehicle_db(); 
} 
/**************************************************************************/ 
/**************************************************************************/ 
/**************************************************************************/ 
/**************************************************************************/ 
