/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/***************************************************************************
*   ROM 2.4 is copyright 1993-1996 Russ Taylor            *
*   ROM has been brought to you by the ROM consortium         *
*       Russ Taylor (rtaylor@efn.org)               *
*       Gabrielle Taylor                     *
*       Brian Moore (zump@rom.org)                  *
*   By using this code, you have agreed to follow the terms of the      *
*   ROM license, in the file Rom24/doc/rom.license            *
***************************************************************************/
/***************************************************************************
*   MARRY.C written by Ryouga for Vilaross Mud (baby.indstate.edu 4000)*
*   Please leave this and all other credit include in this package.    *
*   Email questions/comments to ryouga@jessi.indstate.edu         *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "include.h"

void do_marry( char_data *ch, char *argument)
{

    char arg1[MIL],arg2[MIL];
    char_data *victim;
    char_data *victim2;
    //char buf[MSL];
   
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

   if ((ch->level >= LEVEL_IMMORTAL))
   {
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->println("Syntax: marry <char1> <char2>\n\r");
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->println("The first person mentioned isn't playing.\n\r");
        return;
    }
   
    if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
    {
    ch->println( "The second person mentioned isn't playing.\n\r");
   return;
    }
   
    if ( IS_NPC(victim) || IS_NPC(victim2))
    {
      ch->println("I don't think they want to be Married to the Mob.\n\r");
     return;
    }       
   
    if (!IS_SET(victim->act, PLR_CONSENT) || !IS_SET(victim2->act, PLR_CONSENT))
    {
      ch->println("They do not give consent.\n\r");
     return;
    }
   
      if ((!IS_NULLSTR(victim->pcdata->spouse  )) &&
(!IS_NULLSTR( victim2->pcdata->spouse  )))
    {
        ch->println("They are both already married! \n\r");
       return;
    }
else       if (!IS_NULLSTR(victim->pcdata->spouse  ))
    {
        ch->printlnf("%s is already married! \n\r",victim->name);
       return;
    }
else       if (!IS_NULLSTR(victim2->pcdata->spouse  ))
    {
        ch->printlnf("%s is already married! \n\r",victim2->name);
       return;
    }


   

    if (victim->level < 12 && victim2->level < 12)
      {
        ch->println( "They are not of the proper level to marry.\n\r");
       return;
      }
else     if (victim->level < 12)
      {
        ch->printlnf( "%s is not of the proper level to marry.\n\r",victim->name);
       return;
      }
else     if (victim2->level < 12)
      {
        ch->printlnf( "%s is not of the proper level to marry.\n\r",victim2->name);
       return;
      }
   
    ch->printlnf( "You pronounce %s and %s man and wife!\n\r",victim->name,victim2->name);
   /*  ch->println("You pronounce them man and wife");*/
    act( "You say I do to $N!\n\r", victim, NULL, victim2, TO_CHAR  );
    act( "You say I do to $N!\n\r", victim2, NULL, victim, TO_CHAR  );
victim->pcdata->spouse=strdup(victim2->name);
victim2->pcdata->spouse=strdup(victim->name);
    REMOVE_BIT(victim->act, PLR_CONSENT);
    REMOVE_BIT(victim2->act, PLR_CONSENT);

    return;

   }
   else
   {
     ch->println( "You do not have marrying power.\n\r");
    return;
   }
}

void do_divorce( char_data *ch, char *argument)
{

    char arg1[MIL],arg2[MIL];
    char_data *victim;
    char_data *victim2;
    //char buf[MSL];
   
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

   if (ch->level >= LEVEL_IMMORTAL)
   {
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
         ch->println( "Syntax: divorce <char1> <char2>\n\r");
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
         ch->println("The first person mentioned isn't playing.\n\r");
        return;
    }
   
    if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
    {
         ch->println( "The second person mentioned isn't playing.\n\r");
   return;
    }
   
    if ( IS_NPC(victim) || IS_NPC(victim2))
    {
      ch->println("I don't think they're Married to the Mob...\n\r");
     return;
    }
           
    if (!IS_SET(victim->act, PLR_CONSENT) || !IS_SET(victim2->act, PLR_CONSENT))
    {
      ch->println( "They do not give consent.\n\r");
     return;
    }

      if ((IS_NULLSTR(victim->pcdata->spouse  )) &&
(IS_NULLSTR( victim2->pcdata->spouse  )))
    {
      ch->println( "They aren't even both married!!\n\r");
       return;
    }
else       if (IS_NULLSTR(victim->pcdata->spouse  ))
    {
        ch->printlnf("%s isn't married! \n\r",victim->name);
       return;
    }
else       if (IS_NULLSTR(victim2->pcdata->spouse  ))
    {
        ch->printlnf("%s isn't married! \n\r",victim2->name);
       return;
    }

    ch->printlnf( "You hand %s and %s their divorce papers!\n\r",victim->name,victim2->name);
    act( "You are now divorced from $N!\n\r", victim, NULL, victim2, TO_CHAR  ); 
    act( "You are now divorced from $N!\n\r", victim2, NULL, victim, TO_CHAR  );     

free_string(victim->pcdata->spouse);
free_string(victim2->pcdata->spouse);
victim->pcdata->spouse=NULL;
victim2->pcdata->spouse=NULL;
    return;

   }
   else
   {
     ch->println( "You do not have divorcing power.\n\r");
    return;
   }
}

void do_consent( char_data *ch, char*argument)
{
   if (IS_NPC(ch))
    return;
   
   if ( IS_SET(ch->act, PLR_CONSENT) )
   {
     ch->println( "You no longer give consent.\n\r");
    REMOVE_BIT(ch->act, PLR_CONSENT);
    return;
   }
                           
    ch->println( "You now give consent to be married!\n\r");
   SET_BIT(ch->act, PLR_CONSENT);
   return;
}


void do_spousetalk( char_data *ch, char *argument )
{
    char buf[MSL];
    
    connection_data *d;

if (IS_NULLSTR(ch->pcdata->spouse  ))
{
      ch->println("You talk to your imaginary spouse who ignores you like a real one\n\r");
return;
}

    if (argument[0] == '\0' )
    {
      ch->println("What do you wish to tell your other half?\n\r");
     return;
    }
    else  /* message sent */
    {
bool found=false;
      for ( d = connection_list; d != NULL; d = d->next )
      {
        char_data *victim;

        victim = d->original ? d->original : d->character;
        if ( d->connected_state == CON_PLAYING &&
             d->character != ch &&
             !strcmp(d->character->name , ch->pcdata->spouse))
        {
found=true;
break;
        }
      }


if (found)
{
      sprintf( buf, "`R(`WSpouse >`R) `Y'%s'`X\n\r", /*ch->pcdata->spouse,*/ argument );
       ch->println( buf );
          act_new ( "`R(`WSpouse < `R) `Y'$t'`X",ch,argument,d->character,TO_VICT,POS_SLEEPING);
}
else
{
          ch->println("Your spouse is not here.\n\r");
}
         return;
    }
} 
