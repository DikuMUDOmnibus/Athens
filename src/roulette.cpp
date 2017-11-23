/**************************************************************************/ 
// roulette.cpp 
/*************************************************************************** 
* The Dawn of Time v1.69q (c)1997-2002 Michael Garratt                    * 
* >> A number of people have contributed to the Dawn codebase, with the   * 
*    majority of code written by Michael Garratt - www.dawnoftime.org     * 
* >> To use this source code, you must fully comply with the dawn license * 
*    in licenses.txt... In particular, you may not remove this copyright  * 
*    notice.                                                              * 
*************************************************************************** 
*  This file contributed and made by Mark Fall aka Grimnard Drand         * 
*                                                                         * 
**************************************************************************/ 

#include "include.h" 
#include "global.h" 
#include "gameset.h" 

//local functions 
void do_save_gamesettings(char_data *ch, char *); 
void show_roulette_table(char_data *ch, OBJ_DATA *obj); 
void remove_roulette(char_data *ch); 
void roulette_win(char_data *ch, OBJ_DATA *obj); 

/**************************************************************************/ 
void do_roulette(char_data *ch, char *argument) 
{ 
   char arg1[MIL]; 
   char arg2[MIL]; 
   char arg3[MIL]; 
   OBJ_DATA *obj; 

   argument = one_argument(argument,arg1); 
   argument = one_argument(argument,arg2); 
   argument = one_argument(argument,arg3); 

   if(IS_NPC(ch)) 
   { 
      do_huh(ch,""); 
      return; 
   } 
  /* if(!IS_LETGAINED(ch)) 
   { 
      ch->println("This is set up for RP porpose. You must be letgained to play."); 
      return; 
   }*/ 
   if(IS_NULLSTR(arg1) || IS_NULLSTR(arg2)) 
   { 
      ch->println("Roulete Commands,"); 
      ch->println("   <table name> <bet amount(all bets in gold)> <bet placement> - Places your bet on the table(only one bet per player)."); 
      ch->println("   <table name> show      - show's you the table."); 
      ch->println("   <table name> clear      - clear your bet before wheel is spun."); 
      ch->println("   <table name> win      - show's the odd's chart for the table."); 
      return; 
   } 
   if((obj=get_obj_here(ch,arg1))==NULL) 
   { 
      act("I see no $T here.",ch,NULL,arg1,TO_CHAR); 
      return; 
   }    
   if(!IS_ROULETTE_TABLE(obj)) 
   { 
      ch->println("You must be at a roulette table."); 
      return; 
   } 
   if(!str_cmp(arg2, "show")) 
   { 
      show_roulette_table(ch,obj); 
      return; 
   }    
   if(!str_cmp(arg2, "win")) 
   { 
      ch->println("All win's are subject to the house's fund's"); 
      ch->println("Payouts are calculated like this <bet>*<rate shown below>"); 
      ch->println("Bottom table(1-9,10-18,19-27,`Rred`x,`Sblack`x,`Yyellow`x) = 2"); 
      ch->println("Rows(1,2,3)                                    = 3"); 
      ch->println("Center table(1-27)                             = 10"); 
      ch->println("Zero's(0, D0)                                  = 15"); 
      return; 
   }
   if(is_number(arg2)) 
   { 
      int bet=atoi(arg2); 
      if(ch->gold < bet) 
      { 
         ch->println("You don't have enough gold!!"); 
         return; 
      } 
	
	if(!IS_IMMORTAL(ch) && bet > 10000)
	{ch->printlnf("The house limit on this table is 10,000 gold!");return;}

      int bid=-1; 
      if(is_number(arg3)) 
      { 
         bid=atoi(arg3); 
         if(bid < 0 || bid > 27) 
         { 
            ch->println("Please look at Help Roulette for more info on bet placement."); 
            return; 
         } 
      } 
      if(bid==0) 
      { 
         bid=29; 
      } 
      if(!str_cmp(arg3,"d0") || !str_cmp(arg3, "D0")) 
      { 
         bid=28; 
      } 
      if(!str_cmp(arg3, "row1")) 
      { 
         bid=30; 
      } 
      if(!str_cmp(arg3,"row2")) 
      { 
         bid=31; 
      } 
      if(!str_cmp(arg3,"row3")) 
      { 
         bid=32; 
      } 
      if(!str_cmp(arg3,"thru9")) 
      { 
         bid=33; 
      } 
      if(!str_cmp(arg3,"thru18")) 
      { 
         bid=34; 
      } 
      if(!str_cmp(arg3,"thru27")) 
      { 
         bid=35; 
      } 
      if(!str_cmp(arg3,"red")) 
      { 
         bid=36; 
      } 
      if(!str_cmp(arg3,"black")) 
      { 
         bid=37; 
      } 
      if(!str_cmp(arg3,"yellow")) 
      { 
         bid=38; 
      } 
      if(bid < 0 || bid > 38) 
      { 
         ch->println("Please look at Help Roulette for more info on bet placement."); 
         return; 
      } 
      if(!TABLE_FULL(obj)) 
      { 
         if(TABLE_PLAYER1(obj) == NULL) 
         { 
            SET_TABLE_P1(ch,obj); 
            ch->roulette_tp=1; 
         } 
         else if(TABLE_PLAYER2(obj) == NULL) 
         { 
            SET_TABLE_P2(ch,obj); 
            ch->roulette_tp=2; 
         } 
         else if(TABLE_PLAYER3(obj) == NULL) 
         { 
            SET_TABLE_P3(ch,obj); 
            ch->roulette_tp=3; 
         } 
         R_MARK = bid; 
         R_PULSE=PULSE_ROULETTE; 
         ch->gold -= bet; 
         switch(ch->roulette_tp) 
         { 
         case 1: 
            obj->roulette_bet_1_1=bet; 
            break; 
         case 2: 
            obj->roulette_bet_1_2=bet; 
            break; 
         case 3: 
            obj->roulette_bet_1_2=bet; 
            break; 
         default: 
            ch->gold +=bet; 
            ch->println("This table is experiancing problems please see an imm and report this."); 
            return; 
         } 
         SET_CURRENT_TABLE(ch,obj); 
         show_roulette_table(ch,obj); 
         return; 
      } 
      ch->println("Sorry but i have to many players at this table already please wait for someone to leave."); 
      return; 
   } 
   if(!str_cmp(arg2,"clear")) 
   { 
      if(!IS_CURRENT_TABLE(ch,obj)) 
      { 
         ch->println("Your not playing at this table."); 
         return; 
      } 
      if(roulette->play >= 8) 
      { 
         ch->println("Sorry but after the wheel is spinning all bets are final."); 
         return; 
      } 
      R_MARK=0; 
      REMOVE_CURRENT_TABLE(ch); 
      switch(ch->roulette_tp) 
      { 
      case 1: 
         ch->gold += obj->roulette_bet_1_1; 
         obj->roulette_bet_1_1=0; 
         SET_TABLE_P1(NULL,obj); 
         break; 
      case 2: 
         ch->gold += obj->roulette_bet_1_2; 
         obj->roulette_bet_1_2=0; 
         SET_TABLE_P2(NULL,obj); 
         break; 
      case 3: 
         ch->gold += obj->roulette_bet_1_3; 
         obj->roulette_bet_1_3=0; 
         SET_TABLE_P3(NULL,obj); 
         break; 
      case 0: 
         ch->println("Your bet's have been cleared"); 
         return; 
      default: 
         ch->println("This table is experiancing problems please see an imm and report this."); 
         return; 
      } 
      ch->roulette_tp=0; 
      ch->println("All bets removed"); 
      return; 
   } 
   ch->println("Roulete Commands,"); 
   ch->println("   <table name> <bet amount(all bets in gold)> <bet placement> - Places your bet on the table(only one bet per player)."); 
   ch->println("   <table name> show      - show's you the table."); 
   ch->println("   <table name> clear      - clear your bet before wheel is spun."); 
   ch->println("   <table name> win      - show's the odd's chart for the table."); 
   return; 
} 
/**************************************************************************/ 
void show_roulette_table(char_data *ch, OBJ_DATA *obj) 
{ 

   char row1[MSL]; 
   char row2[MSL]; 
   char row3[MSL]; 
   char row4[MSL]; 
   char row5[MSL]; 
   strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____|____|____|____|____|_________|`c@`x"); 
   strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|____|____|____|____|____|_________|`c@`x"); 
   strcpy(row3,"`c@`s_________|___|____|____|____|____|____|____|____|____|____|_________|`c@`x"); 
   strcpy(row4,"`c@@@@@@@`s|`c@@`s|   |______________|______________|______________|          `c@`x"); 
   strcpy(row5,"`c@@@@@@@`s|`c@@`s|   |______________|______________|______________|          `c@`x"); 
    
   if(IS_CURRENT_TABLE(ch,obj)) 
   { 
      int place=R_MARK; 
      switch(place) 
      { 
      case 29: 
         strcpy(row3,"`c@`s_________|`?X`s__|____|____|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 1: 
         strcpy(row3,"`c@`s_________|___|`?X`s___|____|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 2: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|`?X`s___|____|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 3: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |`?X`s___|____|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 4: 
         strcpy(row3,"`c@`s_________|___|____|`?X`s___|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 5: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|`?X`s___|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 6: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|`?X`s___|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 7: 
         strcpy(row3,"`c@`s_________|___|____|____|`?X`s___|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 8: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|`?X`s___|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 9: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|`?X`s___|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 10: 
         strcpy(row3,"`c@`s_________|___|____|____|____|`?X`s___|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 11: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|`?X`s___|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 12: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|`?X`s___|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 13: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|`?X`s___|____|____|____|____|_________|`c@`x"); 
         break; 
      case 14: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|`?X`s___|____|____|____|____|_________|`c@`x"); 
         break; 
      case 15: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|`?X`s___|____|____|____|____|_________|`c@`x"); 
         break; 
      case 16: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|____|____|`?X`s___|____|____|_________|`c@`x"); 
         break; 
      case 17: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|____|____|`?X`s___|____|____|_________|`c@`x"); 
         break; 
      case 18: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____||`?X`s_______|____|____|_________|`c@`x"); 
         break; 
      case 19: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|`?X`s___|____|____|____|____|_________|`c@`x"); 
         break; 
      case 20: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|`?X`s___|____|____|____|____|_________|`c@`x"); 
         break; 
      case 21: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____|____|`?X`s___|____|____|_________|`c@`x"); 
         break; 
      case 22: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|____|____|`?X`s___|____|____|_________|`c@`x"); 
         break; 
      case 23: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|____|____|`?X`s___|____|____|_________|`c@`x"); 
         break; 
      case 24: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____|____|____|`?X`s___|____|_________|`c@`x"); 
         break; 
      case 25: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|____|____|____|____|`?X`s___|_________|`c@`x"); 
         break; 
      case 26: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|____|____|____|____|`?X`s___|_________|`c@`x"); 
         break; 
      case 27: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____|____|____|____|`?X`s___|_________|`c@`x"); 
         break; 
      case 28: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|`?X`s  |____|____|____|____|____|____|____|____|____|_________|`c@`x"); 
         break; 
      case 30: 
         strcpy(row1,"`c@`m&`s---`y*`s---`m&`s|   |____|____|____|____|____|____|____|____|____|`?X`s________|`c@`x"); 
         break; 
      case 31: 
         strcpy(row2,"`c@ `m & `s| `m&  `s|  `G0`s|____|____|____|____|____|____|____|____|____|`?X`s________|`c@`x"); 
         break; 
      case 32: 
         strcpy(row3,"`c@`s_________|___|____|____|____|____|____|____|____|____|____|`?X`s________|`c@`x"); 
         break; 
      case 33: 
         strcpy(row4,"`c@@@@@@@`s|`c@@`s|   |__`?X`#`s__`^X`s__`^X`s__`^X`s__|______________|______________|          `c@`x"); 
         break; 
      case 34: 
         strcpy(row4,"`c@@@@@@@`s|`c@@`s|   |______________|__`?X`#`s__`^X`s__`^X`s__`^X`s__|______________|          `c@`x"); 
         break; 
      case 35: 
         strcpy(row4,"`c@@@@@@@`s|`c@@`s|   |______________|______________|__`?X`#`s__`^X`s__`^X`s__`^X`s__|          `c@`x"); 
         break; 
      case 36: 
         strcpy(row5,"`c@@@@@@@`s|`c@@`s|   |__`?X`#`s__`^X`s__`^X`s__`^X`s__|______________|______________|          `c@`x"); 
         break; 
      case 37: 
         strcpy(row5,"`c@@@@@@@`s|`c@@`s|   |______________|__`?X`#`s__`^X`s__`^X`s__`^X`s__|______________|          `c@`x"); 
         break; 
      case 38: 
         strcpy(row5,"`c@@@@@@@`s|`c@@`s|   |______________|______________|__`?X`#`s__`^X`s__`^X`s__`^X`s__|          `c@`x"); 
         break; 
      default: 
         break; 
      } 
   } 
   ch->println("\r\n`c@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@`x"); 
   ch->println("`c@   `m&&&&  `s|   |                                                       `c@`x"); 
   ch->println("`c@  `m& `s| `m/& `s|___|______________________________________________________ `c@`x"); 
   ch->println("`c@`m & `s\\ /  `m&`s| `GD0`s|   `Y3`s|   `Y6`s|   `Y9`s|  `S12`s|  `S15`s|  `S18`s|  `R21`s|  `R24`s|  `R27`s| `xall row1`s|`c@`x"); 
   ch->printlnf("%s",row1); 
   ch->println("`c@ `m& `s/ \\ `m& `s|___|   `R2`s|   `R5`s|   `S8`s|  `S11`s|  `Y14`s|  `Y17`s|  `R20`s|  `S23`s|  `Y26`s| `xall row2`s|`c@`x"); 
   ch->printlnf("%s",row2); 
   ch->println("`c@  `m &&&   `s|   |   `S1`s|   `Y4`s|   `R7`s|  `S10`s|  `Y13`s|  `R16`s|  `S19`s|  `Y22`s|  `R25`s| `xall row3`s|`c@`x"); 
   ch->printlnf("%s",row3); 
   ch->println("`c@@@@@@@`s|`c@@`s|   |   `x1 thru 9`s   |  `x10 thru 18`s  |  `x19 thru 27`s  |          `c@`x"); 
   ch->printlnf("%s",row4); 
   ch->println("`c@@@@@@@`s|`c@@`s|   |     `RRED      `s|     `SBLACK    `s|    `YYellow    `s|          `c@`x"); 
   ch->printlnf("%s",row5); 
   ch->println("`c@@@@@@@`s|`c@@`s|                                                           `c@`x"); 
   ch->println("`c@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@`x"); 
   return; 
} 
/**************************************************************************/ 
void update_roulette(void) 
{ 
   if(--roulette->pulse <= 0) 
   { 
      roulette->pulse = PULSE_ROULETTE; 
      connection_data *d; 
      for ( d = connection_list; d != NULL; d = d->next ) 
      { 
         if (   d->connected_state == CON_PLAYING ) 
         { 
            char_data *ch=d->character; 
            bool found; 
            found = false; 
             
            if(ch->roulette_table != NULL) 
            { 
               OBJ_DATA *obj; 
               for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content ) 
               { 
                  if(obj==ch->roulette_table) 
                  { 
                     found = true; 
                  } 
               } 
               if(!found) 
               { 
                  remove_roulette(ch); 
               } 
               ch->roulette_play+=1; 
               OBJ_DATA *table=ch->roulette_table; 
               switch(ch->roulette_play) 
               { 
               case 1:case 5: 
                  ch->println("The Croupier says 'Place Your bet's.... Place Your bet's'"); 
                  ch->println("Rember walking away from table forfits bet's"); 
                  break; 
               case 8: 
                  ch->println("The Croupier says 'Spining the wheel all bets are final'"); 
                  ch->println("The Croupier says 'No more bets please'"); 
                  ch->println("The Croupier spins the wheel and drops in the ball with a pinch."); 
                  break; 
               case 10: 
                  roulette_win(ch,table); 
                  ch->roulette_play=0; 
                  break; 
               default: 
                  break; 
               } 
            } 
         } 
      } 
   } 
} 
/**************************************************************************/ 
void remove_roulette(char_data *ch) 
{ 
   OBJ_DATA *obj; 
   obj=ch->roulette_table; 
   R_MARK=0; 
   REMOVE_CURRENT_TABLE(ch); 
   if(ch->roulette_tp >=1) 
   { 
      switch(ch->roulette_tp) 
      { 
      case 1: 
         ch->gold += obj->roulette_bet_1_1; 
         obj->roulette_bet_1_1=0; 
         SET_TABLE_P1(NULL,obj); 
         break; 
      case 2: 
         ch->gold += obj->roulette_bet_1_2; 
         obj->roulette_bet_1_2=0; 
         SET_TABLE_P2(NULL,obj); 
         break; 
      case 3: 
         ch->gold += obj->roulette_bet_1_3; 
         obj->roulette_bet_1_3=0; 
         SET_TABLE_P3(NULL,obj); 
         break; 
      default: 
         return; 
      } 
   } 
   ch->roulette_tp=0; 
   ch->roulette_play=0; 
   ch->println("You walked away from table all bets cleared."); 
   return; 
} 
/**************************************************************************/ 
void roulette_win(char_data *ch, OBJ_DATA *obj) 
{ 
   int bid; 
   int bet; 
   int ball; 
    
   if(R_MARK==0) 
   { 
      ch->println("This table is broke please notify an imm!!"); 
      return; 
   } 
   bid=R_MARK; 
   switch(ch->roulette_tp) 
   { 
   case 1: bet=obj->roulette_bet_1_1;break; 
   case 2: bet=obj->roulette_bet_1_2;break; 
   case 3: bet=obj->roulette_bet_1_3;break; 
   default:ch->println("This table is broke please notify an imm!!");return; 
   } 
   ball=number_range(1,29); 
   char win[MIL]; 
   switch(ball) 
   { 
   case 29: 
      strcpy(win,"`GGreen Zero`x."); 
      break; 
   case 1: 
      strcpy(win,"`SBlack One`x."); 
      break; 
   case 2: 
      strcpy(win,"`RRed Two`x."); 
      break; 
   case 3: 
      strcpy(win,"`YYellow Three`x."); 
      break; 
   case 4: 
      strcpy(win,"`YYellow Four`x."); 
      break; 
   case 5: 
      strcpy(win,"`RRed Five`x."); 
      break; 
   case 6: 
      strcpy(win,"`YYellow Six`x."); 
      break; 
   case 7: 
      strcpy(win,"`RRed Seven`x."); 
      break; 
   case 8: 
      strcpy(win,"`SBlack Eight`x."); 
      break; 
   case 9: 
      strcpy(win,"`YYellow Nine`x."); 
      break; 
   case 10: 
      strcpy(win,"`SBlack Ten`x."); 
      break; 
   case 11: 
      strcpy(win,"`SBlack Eleven`x."); 
      break; 
   case 12: 
      strcpy(win,"`SBlack Twelve`x."); 
      break; 
   case 13: 
      strcpy(win,"`YYellow Thirteen`x."); 
      break; 
   case 14: 
      strcpy(win,"`YYellow Fourteen`x."); 
      break; 
   case 15: 
      strcpy(win,"`SBlack Fifteen`x."); 
      break; 
   case 16: 
      strcpy(win,"`RRed Sixteen`x."); 
      break; 
   case 17: 
      strcpy(win,"`YYellow Seventeen`x."); 
      break; 
   case 18: 
      strcpy(win,"`SBlack Eighteen`x."); 
      break; 
   case 19: 
      strcpy(win,"`SBlack Nineteen`x."); 
      break; 
   case 20: 
      strcpy(win,"`RRed Twenty`x."); 
      break; 
   case 21: 
      strcpy(win,"`RRed Twenty-One`x."); 
      break; 
   case 22: 
      strcpy(win,"`YYellow Twenty-Two`x."); 
      break; 
   case 23: 
      strcpy(win,"`SBlack Twenty-Three`x."); 
      break; 
   case 24: 
      strcpy(win,"`RRed Twenty-Four`x."); 
      break; 
   case 25: 
      strcpy(win,"`RRed Twenty-Five`x."); 
      break; 
   case 26: 
      strcpy(win,"`YYellow Twenty-Six`x."); 
      break; 
   case 27: 
      strcpy(win,"`RRed Twenty-Seven`x."); 
      break; 
   case 28: 
      strcpy(win,"`GGreen Double Zero`x."); 
      break; 
   default: 
      ch->println("This machine has broke please contact an imm!!"); 
      return; 
   } 
   int payout; 
   if(bid==ball) 
   { 
      if(ball==29 || ball==28) 
      { 
         payout= bet*15; 
      } 
      else 
      { 
         payout= bet*10; 
      } 
   }else 
   { 
      payout=0; 
   } 
   if(R_WIN_ALL_1(ball) && bid == 30) 
   { 
      payout = (payout + bet*3); 
   } 
   if(R_WIN_ALL_2(ball) && bid == 31) 
   { 
      payout = (payout + bet*3); 
   } 
   if(R_WIN_ALL_3(ball) && bid == 32) 
   { 
      payout = (payout + bet*3); 
   } 
   if(R_WIN_1_9(ball) && bid == 33) 
   { 
      payout = (payout + bet*2); 
   } 
   if(R_WIN_10_18(ball) && bid == 34) 
   { 
      payout = (payout + bet*2); 
   } 
   if(R_WIN_19_27(ball) && bid == 35) 
   { 
      payout = (payout + bet*2); 
   } 
   if(R_WIN_RED(ball) && bid == 36) 
   { 
      payout = (payout + bet*2); 
   } 
   if(R_WIN_BLACK(ball) && bid == 37) 
   { 
      payout = (payout + bet*2); 
   } 
   if(R_WIN_YELLOW(ball) && bid == 38) 
   { 
      payout = (payout + bet*2); 
   } 
   ch->printlnf("%s",win); 
   ch->printlnf("paying out for %s",win); 
   if(payout==0) 
   { 
      ch->printlnf("You have lost your bet of %d gold!.", bet);
	ch->gold = ch->gold - bet; 
   } 
   else 
   { 
      ch->printlnf("You have won your bet of %d on %s and win %d gold back!", bet, win, payout); 
      ch->gold = ch->gold - bet;
	ch->gold = ch->gold + payout; 
   } 
   R_MARK=0; 
   REMOVE_CURRENT_TABLE(ch); 
   if(ch->roulette_tp >=1) 
   { 
      switch(ch->roulette_tp) 
      { 
      case 1: 
         ch->gold += obj->roulette_bet_1_1; 
         obj->roulette_bet_1_1=0; 
         SET_TABLE_P1(NULL,obj); 
         break; 
      case 2: 
         ch->gold += obj->roulette_bet_1_2; 
         obj->roulette_bet_1_2=0; 
         SET_TABLE_P2(NULL,obj); 
         break; 
      case 3: 
         ch->gold += obj->roulette_bet_1_3; 
         obj->roulette_bet_1_3=0; 
         SET_TABLE_P3(NULL,obj); 
         break; 
      default: 
         return; 
      } 
   } 
   ch->roulette_tp=0; 
   return; 
} 
/**************************************************************************/ 
/**************************************************************************/ 
/**************************************************************************/ 

