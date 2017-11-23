/**************************************************************************/ 
// vehicle.h - Vehicle movement & Combat 
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

#ifndef VEHICLE_H 
#define VEHICLE_H 

/**************************************************************************/ 
// data structures first 
typedef struct   vehicle_type VEHICLE_DATA; 

struct vehicle_type 
{ 
  struct vehicle_type * next; 
   int reset; 
   int vnum; 
}; 
/**************************************************************************/ 
//prototypes 
void load_vehicle_db( void ); 
void save_vehicle_db( void ); 

// semilocalized globals 
extern VEHICLE_DATA *vehicle_list; 

#endif // VEHICLE_H 
