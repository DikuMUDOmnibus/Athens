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

#include "include.h" 
#include "channels.h" 


int     atoi            args( ( const char *string ) ); 

// command procedures needed 
DECLARE_DO_FUN( do_recall ); 
DECLARE_DO_FUN( do_look ); 
DECLARE_DO_FUN( do_dismount ); 

void    dismount        args( ( char_data *) ); 

/********************************************************************************/ 

void do_challenge(char_data *ch, char *argument) 
{ 

   char_data *victim; 
   char_data *wch; 
   char buf[MIL]; 
   char arg [MIL]; 
   int inthere = 0; 

    one_argument(argument,arg); 


    if(IS_NPC(ch)) 
    { 
        return; 
    } 

   if(ch->pknoquit>0 || ch->pknorecall>0 || ch->pksafe>0 || ch->pkool>0){ 
      ch->println( "`R[ARENA]:You have been or are involved in PK action and must wait" ); 
      return; 
    } 
   if ((ch->level<6) || (!IS_LETGAINED(ch) && GAMESETTING_LETGAINING_IS_REQUIRED)) 
   { 
      ch->println("`R[ARENA]:`x You can not player kill, steal from players etc,"); 
      ch->println("till level 6 and you have been letgained."); 
      ch->println("(See HELP DEATH and HELP LETGAIN)"); 
      return; 
   } 
    
    if (arg[0] == '\0') 
    { 
      ch->println( "`R[ARENA]:`x Syntax: challenge <name>\n\r" ); 
      return; 
    } 

   if(ch->challenged 
      && IS_SET(ch->act2, ACT2_PLR_ARENA)) 
   { 
      ch->println("`R[ARENA]:`xYou have already challenged and you are in the arena."); 
return; 
   } 


    if ( ( victim = get_char_world( ch, arg ) ) == NULL ) 
    {    
      ch->println("Can't see them."); 
      return;          
    } 

   if ((victim->level<6) || (!IS_LETGAINED(victim) && GAMESETTING_LETGAINING_IS_REQUIRED)) 
   { 
      ch->println("`R[ARENA]:`x They cannot player kill"); 
      ch->println("till level 6 AND have been letgained."); 
      return; 
   } 

   if(victim->pknoquit>0 || victim->pknorecall>0 || victim->pksafe>0 || victim->pkool>0){ 
      ch->println( "`R[ARENA]:They have been or are involved in PK action and must wait" ); 
      return; 
    }       
    
   //prevents abusers from using to get out of a jam. 
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) 
    ||   IS_AFFECTED(ch, AFF_CURSE)) 
    {        
      ch->println( "`R[ARENA]:`x You are either cursed or in a norecall room.\n\r" ); 
        return; 
    } 

   //player challenged is himself. 
    if (victim == ch) 
    { 
   ch->println( "`R[ARENA]:`x You're going to fight yourself in the arena? Verrry funny :P.\n\r" ); 
   return; 
    } 

   //Get_char_world does cough up NPC's. Could consider the possibility 
   if(IS_NPC(victim)) 
   { 
      ch->printlnf( "`R[ARENA]:`x %s does not do arenas.\n\r", victim->short_descr); 
      return; 
   } 

   //player challenged has challenged the challenger first. 
   if(ch->name == victim->challenged){ 
   ch->printlnf("`R[ARENA]:`x %s has challenged YOU, now you must decide if to accept.", ch->challenger); 
   ch->printlnf( "`R[ARENA]:`x Syntax: accept %s.\n\r", ch->challenger ); 
   return; 
   } 
    
   //player challenged has a challenger 
   if(victim->challenger) 
   { 
      ch->printlnf( "`R[ARENA]: `Y%s has challenged %s, you must wait.", victim->challenger, victim->name ); 
      return; 
   } 

   //player challenged has made his own challenge with another 
   if(victim->challenged) 
   { 
      ch->printlnf( "`R[ARENA]: `Y%s has challenged %s, you must wait.", victim->name, victim->challenged ); 
      return; 
    }else{ 

        for( wch = player_list; wch; wch = wch->next_player ) 

       {    
             if(can_see(wch, ch) && IS_SET(wch->act2, ACT2_PLR_ARENA) ){          
               ch->printlnf("`R[ARENA]: `W%s is in there.", wch->name);    
              inthere += 1; 
              } 
       } 
       if(inthere > 0){ 
          ch->println("`YSorry, the arena is busy. Please try after the fight.`x"); 
          return; 
       } 
   } 
   // this can get confusing 
   ch->challenged = victim->name; //ch has_challenged 
   victim->challenger = ch->name; //victim has_challenger 

    SET_BIT( ch->act2, ACT2_PLR_ARENA ); 

    
   if (ch->mounted_on){ 
      dismount(ch); 
      act( "$n dismounts.", ch, 0, 0, TO_ROOM ); 
   } 
//Transfer them to the arena. Supply your vnum. 
   ROOM_INDEX_DATA *room; 
   room = get_room_index(700); 
   char_from_room(ch); 
   char_to_room(ch, room); 
   do_look( ch, "auto" );

    sprintf( buf, "`W%s `Yhas just challenged `W%s `bin the `Rarena!`x", ch->name, victim->name );    
    arena_broadcast( ch, buf ); 
   sprintf( buf, "`W%s's `barena stats: `WWon:  %d    `WLost:  %d   `x\n\r" , ch->name, ch->win, ch->lost );    
    arena_broadcast( ch, buf ); 
    
   victim->printlnf( "`R[ARENA]`x `Y%s, you have been challenged in the arena!", victim->name ); 
   victim->println( "`R[ARENA]:`x Please, decline if you wish not to fight." ); 
   victim->printlnf( "`R[ARENA]:`x Options: decline %s or accept %s.", ch->name, ch->name ); 

} 

/**********************************************************************************/ 

void do_accept(char_data *ch, char *argument) 
{ 
   char_data *victim; 
   char buf[MIL]; 
   char arg [MIL]; 
  
    
    one_argument(argument,arg); 

      
    if(IS_NPC(ch)) 
    { 
        return; 
    } 

   if( ch->pknoquit>0 || ch->pknorecall>0 || ch->pksafe>0 || ch->pkool>0 ){ 
      ch->println( "`R[ARENA]:You have been or are involved in PK action and must wait" ); 
      return; 
    } 

    if (arg[0] == '\0'){ 

      if(ch->challenger){ 
      ch->println( "`R[ARENA]:`x Syntax: accept <name>.\n\r" ); 
        return; 
      }else{ 
      ch->println( "`R[ARENA]:`x But, you do not have a challenger!" ); 
      return; 
      } 
   } 

   //Stop spam to the arena broadcast. 
   if(ch->challenger 
      && IS_SET(ch->act2, ACT2_PLR_ARENA)) 
   { 
      ch->println("`R[ARENA]:`xYou have already accepted and you are in the arena."); 
            return; 
   } 

   //Stop abusers from using the CANSEE bit to see who's online. 
   if(!ch->challenger){ 
      ch->println("`R[ARENA]:`xBut, they have not challenged you."); 
      return; 
   }       

   //make him a can_see in case the guy waiting in the arena went undetectable 
   //lookers with this bit set can_see all except immortals. For this application. 
   // This is to avoid the delay of asking the challenger to become visible so 
   //they can accept. 
   SET_BIT( ch->act2, ACT2_PLR_CANSEE ); 
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL ) 
    {       
      ch->println("They aren't here."); 
      REMOVE_BIT(ch->act2, ACT2_PLR_CANSEE); //Take it away from them 
      return; 
    } 

   //Remove CANSEE bit at every return. They still have it. 
    if (victim == ch) 
    { 
       ch->println( "`R[ARENA]:`x You graciously accept yourself. Verrry funny.\n\r" ); 
      REMOVE_BIT(ch->act2, ACT2_PLR_CANSEE); //They have aquired a victim and still have the bit. 
        return; 
    } 
     //also, if if victim->challenged != ch name  print "they did not challenge you." 
   if ( victim->name != ch->challenger || victim->challenged == NULL) 
   { 
      ch->println( "`R[ARENA]:`x They have not challenged YOU."); 
      REMOVE_BIT(ch->act2, ACT2_PLR_CANSEE);//They have aquired a victim and still have the bit. 
      return; 
   } 

   //Final removal. 
   REMOVE_BIT(ch->act2, ACT2_PLR_CANSEE); 
  
   //passed inspection, get arena bit. Can seeall in the arena but !immortals. 
    SET_BIT( ch->act2, ACT2_PLR_ARENA ); 

   if (victim->mounted_on){ 
      dismount(victim); 
      act( "$n dismounts.", ch, 0, 0, TO_ROOM ); 
   } 
//Transfer them to the arena. Supply your vnum. 
    ROOM_INDEX_DATA *room; 
    room = get_room_index(700); 
    char_from_room(ch); 
    char_to_room(ch, room); 
    do_look( ch, "auto" );

    sprintf(buf,"`W%s `Yhas just accepted the challenge.`x",ch->name); 
    arena_broadcast( ch, buf ); 
   sprintf( buf, "`W%s's `barena stats: `WWon  %d    `WLost  %d   `x" , victim->name, victim->win, victim->lost );    
    arena_broadcast( ch, buf ); 
   sprintf( buf, "`W%s's `barena stats: `WWon  %d    `WLost  %d   `x\n\r" , ch->name, ch->win, ch->lost );    
    arena_broadcast( ch, buf ); 
   sprintf( buf, "`YPlace your bets `RNOW!!! `bSyntax: `xbet <`Wamount `YGold`x> < `W%s `ROR `W%s `x>.", victim->name, ch->name );    
    arena_broadcast( ch, buf ); 



} 
  
/**************************************************************************/ 

void do_decline(char_data *ch, char *argument) 
{ 
          
   char_data *victim; 
   char_data *wch; 
   char buf[MIL]; 
   char arg [MIL]; 
  
        
    one_argument(argument,arg); 
      
    if IS_NPC(ch) 
    { 
        return; 
    } 

   if ( IS_SET(ch->act2, ACT2_PLR_ARENA)) 
    {  
   ch->println( "`R[ARENA]:`YDecline denied, you accepted the challenge`x.\n\r" ); 
   return; 
   } 
       
    if (arg[0] == '\0') 
    { 
      if(ch->challenger == NULL){ 
      ch->println( "`R[ARENA]:`Y But, you currently have no challenger`x.\n\r" ); 
      return; 
      }else{ 
      ch->println( "`R[ARENA]:`Y Syntax: decline <name>`x.\n\r" ); 
      return; 
      } 
    } 

          
     if( ( victim = get_char_world( ch, arg ) ) == NULL ) 
    { 
      if(ch->challenger == NULL){ 
      ch->println( "`R[ARENA]:`Y But, you currently have no challenger`x.\n\r" ); 
      return; 
      }else if(ch->challenger){ 
         for( wch = player_list; wch; wch = wch->next_player ) 
         {                      
            if(can_see(wch, ch) && IS_SET(wch->act2, ACT2_PLR_ARENA) ) 
            { 
                          
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
       } 
      ch->println( "`R[ARENA]:`Y Your decline is accepted.`x\n\r" ); 
        ch->challenger = NULL; 
        return; 
       
    }else if (victim == ch) 
    { 
       ch->println( "`R[ARENA]:`x You respectfully decline yourself. Verrry funny.\n\r" ); 
        return; 
    } 
    else if(ch->challenger == NULL){ 
      ch->println( "`R[ARENA]:`Y Actually, you have NO current challengers`x.\n\r" ); 
      return;        
    }else if ( victim->name != ch->challenger ){      
      ch->printlnf( "`R[ARENA]:`Y Actually, %s is your challenger`x.", ch->challenger ); 
        ch->printlnf( "`R[ARENA]:`Y Syntax: decline %s`x.\n\r", ch->challenger ); 
      return; 
   } 
   else{          
      sprintf(buf,"`W%s `Rhas declined the challenge.\n\r",ch->name); 
      arena_broadcast( ch, buf ); 
      ch->challenger = NULL; 
      REMOVE_BIT(victim->act2, ACT2_PLR_ARENA);              
      if(victim->challenged != NULL){ 
      victim->challenged = NULL; 
      } 
      if(victim->challenger != NULL){ 
      victim->challenger = NULL; 
      } 
      //go through possible recall problems. 
      //DO NOT SET ARENA ROOM FLAGS TO NORECALL 
      if ( IS_SET(victim->affected_by, AFF_CURSE)){ 
      REMOVE_BIT( victim->affected_by, AFF_CURSE ); 
      } 
      if(victim->pknorecall > 0){ 
      victim->pknorecall = 0; 
      } 
      if(victim->position != POS_STANDING){ 
      victim->position = POS_STANDING; 
      } 
      if(victim->pcdata->bet){ 
      victim->pcdata->bet = NULL; 
      } 
       
      do_recall( victim, "" ); //Decline won't trigger the "cancel by recall"    
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

   } 
    

} 

/**************************************************************************/ 


void arena_broadcast(char_data *ch, char * fmt, ...) 
{ 
    
    char buf[MSL], buf2[MSL]; 
   char_data *wch; 
    
   va_list args; 
   va_start(args, fmt); 
   vsnprintf(buf, MSL, fmt, args); 
   va_end(args); 

   sprintf(buf2, "`=\x8a`R[ARENA]:`x `=\x8b%s`=\x8a`x", buf); 

   for( wch = player_list; wch; wch = wch->next_player ) 
    { 
      if(IS_IMMORTAL(ch) && !IS_IMMORTAL(wch)){ 
         continue; 
      } 

      if(   can_see(wch, ch) ) 
      { 
         bool heard=false; 
         if(!HAS_CHANNELOFF(ch, CHANNEL_QUIET) 
            && !HAS_CHANNELOFF(ch, CHANNEL_INFO)) 
         { 
            wch->println(buf2); 
            heard=true; 
         } 

         // record the broadcast in the receivers replay channels buffer 
         if(wch->pcdata){ 
            replace_string(wch->pcdata->replaychannels_text[wch->pcdata->next_replaychannels], 
               FORMATF("%s%s%s %s\r\n", heard?"`X":"`S", shorttime(NULL), heard?">":"]", buf2)); 
            ++wch->pcdata->next_replaychannels%=MAX_REPLAYCHANNELS; 
         } 
      } 
   } 
} 
/**************************************************************************/ 

void do_bet(char_data *ch, char *argument) 
{ 
    char arg1 [MIL];  
    char arg2 [MIL]; 
    char_data *victim; 
   int adjust; 
    
    argument = one_argument( argument, arg1 );  /* Combine the arguments */ 
    argument = one_argument( argument, arg2 ); 
  
   if (IS_NPC(ch)) 
   { 
      ch->println("Please use the kitchen entrance and don't attract attention.\r\n"); 
      return; 
   } 
  
    if (arg1[0] == '\0' || arg2[0] == '\0') 
    { 
        ch->println("Syntax: bet <amount> <player name>."); 
      ch->println("Betting amount limited to 100 gold."); 
        return; 
    } 
    
   if (!strcmp(arg2, "gold")) 
   { 
      ch->println("Don't type gold, type the player's name."); 
        return; 
   } 
    
   //make ch able to detect players just for the bet 
   SET_BIT( ch->act2, ACT2_PLR_CANSEE ); 

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL ) 
    { 
      ch->println("They are not here.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
    } 

   if (victim == ch) 
   { 
      ch->println("Can't bet on yourself.\r\n"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
    }    

   if(!IS_SET(victim->act2, ACT2_PLR_ARENA) && !victim->challenger) 
   { 
      ch->println("They are not an arena contestant.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 

   if(ch->challenger) 
   { 
      ch->println("You have a challenger. Participants cannot bet. Just accept or decline.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 

   if (IS_SET(ch->act2, ACT2_PLR_ARENA)) 
   { 
      ch->println("Participants cannot prosper except for rewards.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 

   if(!IS_SET(victim->act2, ACT2_PLR_ARENA) && victim->challenger) 
   { 
      ch->println("Wait until after they accept the challenge.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 
          
   int value = atoi(arg1); 
    
   if(value == 0) 
   { 
      ch->println("Place a bet higher than zero!\r\n"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 

   if(value > 100) 
   { 
      ch->println("Enter a value between 1 and 100.\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
   } 

   if (value * 100 > (ch->gold *100 + ch->silver) ) 
    { 
      ch->println("You do not have enough gold on you!\n\r"); 
      REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 
      return; 
    } 
  
  
    if(ch->pot > 0){ 
     ch->printlnf("A bookie gives your old bet %d gold back, then...", ch->pot); 
     adjust = ch->pot; 
     ch->gold += adjust; 
     ch->pot = 0;     
    } 
  
    ch->pcdata->bet = str_dup(victim->name); 
    victim->pcdata->bet = str_dup( ch->name ); 
    ch->printlnf("A bookie takes your bet of %d `Ygold`x coins on %s.\r\n", value, victim->name); 
    REMOVE_BIT( ch->act2, ACT2_PLR_CANSEE ); 

    SET_BIT(ch->act2, ACT2_PLR_BET); 
    deduct_cost(ch,value * 100);             
    ch->pot += value; 
    if (ch->pot > 100){ 
      adjust = ch->pot - 100; 
      ch->gold += adjust; 
       ch->pot = 100; 
      } 
  
} 
/***********************************************************************************/ 
void do_join(char_data *ch, char *argument) 
{
   char arg [MIL]; 
   one_argument(argument,arg); 

    if(IS_NPC(ch)) 
    { 
        return; 
    } 
    if(!GAMESETTING(GAMESET3_DISCORD)){
			ch->wrapln("`cDiscordia is closed.`x");return;}
   if IS_SET(ch->act2, ACT2_PLR_EVENT){
			ch->wrapln("`cYou're already in an event!`x");return;}
   if(ch->pknoquit>0 || ch->pknorecall>0 || ch->pksafe>0 || ch->pkool>0){ 
      ch->println( "`cYou have been or are involved in PK action and must wait.`x" ); 
      return; 
    } 

   if ((ch->level<6) || (!IS_LETGAINED(ch) && GAMESETTING_LETGAINING_IS_REQUIRED)) 
   { 
      ch->println("`c You can not player kill, steal from players etc,`x"); 
      ch->println("`ctill level 6 and you have been letgained.`x"); 
      ch->println("`c(See HELP LETGAIN)`x"); 
      return; 
   } 

    if (arg[0] == '\0') 
    { 
      ch->println( "`cSyntax: JOIN <EVENT>`x\n\r" ); 
      ch->println( "`cThe only available event for now is the DISCORDIA`x\n\r" ); 
      return; 
    } 

   //prevents abusers from using to get out of a jam. 
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) 
    ||   IS_AFFECTED(ch, AFF_CURSE)) 
    {        
      ch->println( "`cYou are either cursed or in a norecall room.`x\n\r" ); 
        return; 
    } 

   SET_BIT( ch->act2, ACT2_PLR_EVENT ); 
   if (ch->mounted_on){ 
      dismount(ch); 
      act( "$n dismounts.", ch, 0, 0, TO_ROOM ); 
   } 

   //Transfer them to the event. Supply your vnum. 
   ROOM_INDEX_DATA *room; 
   room = get_room_index(928); 
    act( "`c$n DISAPPEARS FOR THE EVENT.`x", ch, 0, 0, TO_ROOM );
    ch->println("`cTransferring you to the event now.`x");
   char_from_room(ch); 
   char_to_room(ch, room); 
    act( "`c$n APPEARS FOR THE EVENT.`x", ch, 0, 0, TO_ROOM );
   do_look( ch, "auto" );



}
/***********************************************************************************/ 





