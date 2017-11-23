/**************************************************************************/
// global.h - Global variable system details see below
/***************************************************************************
 * The Dawn of Time v1.69r (c)1997-2004 Michael Garratt                    *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Michael Garratt - www.dawnoftime.org     *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: global.h - To add a global just add a SINGLE entry into 
 *        global.h prefixed with the word EXTERN (all uppercase)... 
 *        compiler macros do the rest.
 ***************************************************************************/
#ifndef GLOBAL_H
#define GLOBAL_H

EXTERN  MOOT_DATA *moot;
EXTERN  ROULETTE_DATA *roulette;

EXTERN  char EXE_FILE[30];
EXTERN  char MACHINE_NAME[MSL];
EXTERN  char PLATFORM_INFO[2048];

// for do_count - act_info.c 
EXTERN  int max_on;
EXTERN  int true_count;
EXTERN  time_t          maxon_time;				// Time of master maxon
EXTERN  int				hotrebootmaxon;         // maxon since last hotreboot
EXTERN  time_t          hotrebootmaxon_time;    // maxon since last hotreboot 
                                                // was at what time

EXTERN  connection_data *   connection_free;	// Free list for connections
EXTERN  connection_data *   connection_list;    // All open connections
EXTERN  connection_data *   c_next; // the next connection used in a number of loops

EXTERN  char_data *			player_list;
EXTERN  race_data *			race_list;

// Reserved file handles
// - Secondary is reserved so we can do 2 lots of file io - in handler.c
EXTERN  FILE *          fpReserve;
EXTERN  FILE *          fpAppend2FilReserve;
EXTERN  FILE *          fpReserveFileExists;

EXTERN  RUNLEVEL_TYPE	runlevel;				// Used to mark bootup, mainloop and shutdown
EXTERN  char            str_boot_time[MIL];
EXTERN  time_t          boot_time;              // Time we last hotrebooted
EXTERN  time_t          lastreboot_time;         // Time we started up 
EXTERN  time_t          current_time;           // time of this pulse
EXTERN  long            tick_counter;           // counter of the ticks
EXTERN  int             note_notify_counter;    // notification of new mail
EXTERN  char            shutdown_filename[MSL];
EXTERN  bool            MOBtrigger;             // act() switch
EXTERN  int				mainport;               // the mainport value specified on startup
EXTERN  int				parsed_mainport;               // the mainport value specified on startup

EXTERN  int				resolver_stdinout;			// Pipe to hostname lookup resolver stdin/stdout
EXTERN  int				resolver_stderr;		// Pipe to hostname lookup resolver stderr
EXTERN  int				resolver_version;		// resolver version, * 1000
EXTERN  int				resolver_running;

EXTERN  bool            fBootTestOnly;	// used to check if the mud will bootup

EXTERN	DEITY_DATA	*	deity_first;
EXTERN	DEITY_DATA	*	deity_last;

// area related
EXTERN  AREA_DATA *     area_first;
EXTERN  AREA_DATA *     area_vnumsort_first;
EXTERN  AREA_DATA *     area_levelsort_first;
EXTERN  AREA_DATA *     area_arealist_first;
EXTERN  AREA_DATA *     area_last;
EXTERN  SHOP_DATA *     shop_last;
EXTERN	cInnData*		pLastInn;
EXTERN  int	resaveCounter;

// moons code - affects casting level of mages
EXTERN  int		moon_day;	// 1->28
EXTERN  int     moon_month;	// 1-12
EXTERN  int     moon_cast_modifier;

// MPINFO stuff - program_flow() callstack stuff
EXTERN  vn_int callstack_pvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_mvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_rvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_line[MAX_CALL_LEVEL];
EXTERN  bool callstack_aborted[MAX_CALL_LEVEL];
EXTERN  int call_level; // Keep track of nested "mpcall"s
EXTERN  bool mobprog_preventtrain_used;
EXTERN  bool mobprog_preventprac_used;


EXTERN  sh_int		top_helpfile;

// WEBSERVER related
//EXTERN  bool webRunning;
EXTERN  int  webHits;
EXTERN  int  webHelpHits;
EXTERN  int  webWhoHits;

// DEFAULT CHARACTER TEMPLATES
EXTERN  char_data * chImmortal;

// mudftp code
EXTERN  int ftp_control;

// pkill port details
EXTERN  int p9999maxpk, p9999maxpklevel;
EXTERN  char p9999maxpkname[50];
EXTERN  int p9999maxpd, p9999maxpdlevel;
EXTERN  char p9999maxpdname[50];

// GSN numbers - being moved into here
#include "gsn.h"

// races stuff
EXTERN  int total_npcracescount;
EXTERN  int total_npcareacount;

// log memory
EXTERN  bool log_memory;
EXTERN  int free_mem_count;
EXTERN  int alloc_mem_count;

// linked list of all pMobIndex records
EXTERN  MOB_INDEX_DATA *pMobIndexlist;

// disabled commands system
EXTERN  DISABLED_DATA *disabled_first; 

// new dynamic skill system
EXTERN  sh_int	FIRST_SPELL;
EXTERN  sh_int	LAST_SPELL;
EXTERN  sh_int	SKILL_TABLE_FLAGS;
EXTERN  sh_int	TOP_SKILL;

// supports letgain system
EXTERN  letgain_data *letgain_list;

// dyn command related
EXTERN	sh_int	COM_TABLE_FLAGS;
EXTERN	sh_int	DEITY_FLAGS;
EXTERN	sh_int	HERB_FLAGS;
EXTERN	sh_int	MIX_FLAGS;
EXTERN	sh_int	CLAN_FLAGS;
EXTERN	sh_int	SKILLGROUPEDIT_FLAGS;

// olc based resave flags
EXTERN	bool	LANGUAGE_NEEDS_SAVING;

// classedit related globals
EXTERN  sh_int	CLASS_TABLE_FLAGS;

// DEBUG system - COMMANDS IN DEBUG TO ALLOW YOU TO SET THE ROOM, OBJ, MOB
//                THEN USE 'MAKECOREFILE' TO HAVE A LOOKY AT WHAT YOU HAVE SET
EXTERN	ROOM_INDEX_DATA *DEBUG_ROOM;
EXTERN	char_data		*DEBUG_MOB;
EXTERN	OBJ_DATA		*DEBUG_OBJECT;

EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_ROOM_VNUM;
EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_OBJECT_VNUM;
EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_MOBILE_VNUM;

// check_immtalk system
EXTERN	char check_immtalk_replay_text[MAX_CHECK_IMMTALK][MIL+30];
EXTERN	int check_immtalk_replay_index;

// use command tail to record the last commands leading up to a crash 
// - olc or interp based
EXTERN char inputtail[MAX_INPUTTAIL][MIL+250]; // 250 bytes for details
EXTERN int inputtail_index;

EXTERN char temp_HSL_workspace[HSL];
EXTERN int mobprog_count;

EXTERN mob_index_data *limbo_mob_index_data; // used when we need a temp mob template 

// Array of containers read for proper re-nesting of objects.
EXTERN OBJ_DATA *   rgObjNest[MAX_NEST];

// raceedit stuff
EXTERN  sh_int	RACEEDIT_FLAGS;
EXTERN  class race_data** race_table; // dynamically allocated

EXTERN  bool EXECUTING_SOCIAL;
EXTERN  bool RECORD_TO_REPLAYROOM;

// channeloff macros use the following variable to avoid following a null pointer
EXTERN  long __CHANNEL_OFF_CRASH_PROTECTOR_VARIABLE;

EXTERN  continent_type *continent_list;

EXTERN  int lockers_total_count;
EXTERN  int lockers_object_count;

// system languages
EXTERN  language_data *language_unknown;
EXTERN  language_data *language_native;
EXTERN  language_data *language_alwaysunderstood;
EXTERN  language_data *language_reverse;

// for roulette 
#define IS_ROULETTE_1(obj)               (IS_SET(obj->extra2_flags, OBJEXTRA2_ROULETTE_1)) 
#define IS_ROULETTE_2(obj)               (IS_SET(obj->extra2_flags, OBJEXTRA2_ROULETTE_2)) 
#define IS_ROULETTE_TABLE(obj)            (IS_ROULETTE_1(obj) || IS_ROULETTE_2(obj)) 
#define IS_CURRENT_TABLE(ch,obj)         (ch->roulette_table == obj) 
#define R_MARK                        (ch->roulette_mark) 
#define SET_CURRENT_TABLE(ch,obj)         (ch->roulette_table = obj) 
#define REMOVE_CURRENT_TABLE(ch)         (ch->roulette_table = NULL) 
#define TABLE_FULL(obj)                  (obj->roulette_player1 != NULL && obj->roulette_player2 != NULL && obj->roulette_player3 != NULL)
#define TABLE_PLAYER1(obj)               (obj->roulette_player1)          
#define TABLE_PLAYER2(obj)               (obj->roulette_player2) 
#define TABLE_PLAYER3(obj)               (obj->roulette_player3) 
#define SET_TABLE_P1(ch,obj)            (obj->roulette_player1 = ch) 
#define SET_TABLE_P2(ch,obj)            (obj->roulette_player2 = ch) 
#define SET_TABLE_P3(ch,obj)            (obj->roulette_player3 = ch) 
#define R_WIN_ALL_1(a)                  (a==3 || a==6 || a==9 || a==12 || a==15 || a==18 || a==21 || a==24 || a==27) 
#define R_WIN_ALL_2(a)                  (a==2 || a==5 || a==8 || a==11 || a==14 || a==17 || a==20 || a==23 || a==26) 
#define R_WIN_ALL_3(a)                  (a==1 || a==4 || a==7 || a==10 || a==13 || a==16 || a==19 || a==22 || a==25) 
#define R_WIN_1_9(a)                  (a>=1 && a<=9) 
#define R_WIN_10_18(a)                  (a>=10 && a<=18) 
#define R_WIN_19_27(a)                  (a>=19 && a<=27) 
#define R_WIN_YELLOW(a)                  (a==3||a==6||a==9||a==14||a==17||a==26||a==4||a==13||a==22) 
#define R_WIN_BLACK(a)                  (a==12||a==15||a==18||a==8||a==11||a==23||a==1||a==10||a==19) 
#define R_WIN_RED(a)                  (a==21||a==24||a==27||a==2||a==5||a==20||a==7||a==16||a==25) 
#define R_PULSE                        (roulette->pulse)   


#endif // GLOBAL_H
