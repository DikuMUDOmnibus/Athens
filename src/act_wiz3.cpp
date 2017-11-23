#include "include.h" // dawn standard includes
#include "olc.h"
#include "nanny.h"
#include "msp.h"
#include "magic.h"
#include "params.h"

/******************************************************************************
Idea for this function borrowed from promote command snippet on Mud 
Magic (http://www.mudmagic.com) and modified from the DOT do_advance
function act_wiz.cpp)									
******************************************************************************/

void do_immadvance(char_data *ch, char *argument)
{
    char arg1[MIL];
    char arg2[MIL];
    char_data *victim;
    int level;
    int iLevel;
	int olevel;
	int nlevel;
	bool not_letgained=false;

	if(IS_NPC(ch))
	{ch->println("Players online sorry.");return;}

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
    {ch->println( "Syntax: immadvance <char> <level>." );return;}

	if (( victim = get_whovis_player_world(ch, arg1)) == NULL)
	{ch->println( "That player is not here." );return;}

	if (IS_NPC(victim))
	{ch->println("Not on NPC's.");return;}

    if((level = atoi(arg2)) < 93 || level > 100)
    {ch->printlnf("Level must be 93 to 100.");return;}

    if(level > get_trust(ch))
    {ch->println( "Limited to your trust level." );return;}

    if(victim != ch)
	{if (get_trust(victim) >= get_trust(ch))
	{ch->println( "Sorry, no can do. Their level/trust is higher then yours.");return;}}

    /* Now we actually advance or lower the player's level. This is ripped right out of do_advance in act_wiz.cpp and modified to suit our needs */

	if (level < victim->level)
       {
	olevel = victim->level;
	nlevel = level;
	ch->printlnf("You have lowered %s's level from %d to %d.", victim->name, olevel, nlevel);
	victim->println("The Gods on Olympus look down upon you and frown.");
	victim->printlnf("\n\rYour level has been LOWERED from %d to %d", olevel, nlevel);
	
	  int temp_prac = victim->practice;
        victim->level    = 1;
        victim->exp      = exp_per_level(victim,victim->pcdata->points);
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;
	  /*advance_level( victim );*/
        victim->practice = temp_prac;
	}
	else
	{ 
	olevel = victim->level;
	nlevel = level;
	ch->printlnf("`cYou raise `X%s's `clevel from `C%d `cto `C%d`c.`x", victim->name, olevel, nlevel);
	victim->printlnf("`cThe Gods on Olympus look down upon you and smile.`x");
	victim->printlnf("\n\r`cYour level has INCREASED from `C%d `cto `C%d`c.`x", olevel, nlevel);

	if(olevel<93)
	{victim->printlnf("`#`YWelcome to the Athens' Immortal Staff. Please read `RHELP IMMRULES `Yand use the 
`RHOLYSET `Ycommand now. Also you should read the help file for your position. `R(help nsa-rules, builder-rules, 
coder-rules, jradmin-rules, sradmin-rules, head-nsa, head-builder, head-coder, or head-admin.`^");}}

	if(!IS_LETGAINED(victim))
	{
		not_letgained=true;
		SET_BIT(victim->act,PLR_CAN_ADVANCE);
	}

    for (iLevel=victim->level;iLevel<level; iLevel++)
    {
        victim->level += 1;
    }

    victim->exp = exp_per_level(victim,victim->pcdata->points) * UMAX(1, victim->level);
victim->trust = 0;

    // reset RPS score 
    if (victim->pcdata->rp_points<0)
	{victim->pcdata->rp_points = 0;}

	// move the pfile if required
	{
		PFILE_TYPE pt=get_pfiletype(victim);
		if(victim->pcdata->pfiletype!=pt)
		{
			rename(pfilename(victim->name,victim->pcdata->pfiletype),
				pfilename(victim->name,pt)); // move the file
			victim->pcdata->pfiletype=pt;
		}
	}
	if(not_letgained)
	{
		REMOVE_BIT(victim->act,PLR_CAN_ADVANCE);
		ch->printlnf("`cNote: %s isn't letgained.`x", victim->name);
	}
    save_char_obj(victim);
    return;
}

/*******************************************************************************
                      do_award 1.1 snippet for EmberMUD
                      Install sheet by Rindar (Ron Cole)
             Code by Rindar (Ron Cole) and Raven (Laurie Zenner)
Snippet is available at:
http://ftp.mudmagic.com/diku/merc/rom/ember/snippets/Award.txt
*******************************************************************************/
void do_award( char_data *ch, char *argument)
{
    char_data *victim;
    //char       buf  [MSL];
    char       arg1 [MIL];
    char       arg2 [MIL];
    char       arg3 [MIL];
    int      value;
    int	 level;
    int      levelvi;
    char     *maxed;
    long     maxlong  = 2147483647;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    
    /* have to initialize maxed or will get warnings when compiling */
    maxed = strdup( "maxed" );

  if(!IS_NPC(ch) && ch->level < 97 )
	{do_huh(ch,"");return;}

 	else{  

  if ( arg1[0] == '\0' || arg3[0] == '\0' || !is_number(arg2))
    {
 ch->println("`cSyntax: award <char> <amount> <type>.`x");
       ch->println("`cValid types: clout, divinity, experience, fame, gold, mps, or object.`x");
	 /*ch->println("Valid types: alignment, experience, gold, object.");*/
       ch->println("`YNOTE: Substitute the object's VNUM for the amount.`x");
        return;
    }
    
    //if ( strncmp(arg3, "alignment",  strlen(arg3))
    if   (strncmp(arg3, "experience", strlen(arg3))
    &&   strncmp(arg3, "gold",       strlen(arg3))
    &&   strncmp(arg3, "object",     strlen(arg3))
    &&   strncmp(arg3, "fame",       strlen(arg3))
    &&   strncmp(arg3, "mps",        strlen(arg3))
    &&   strncmp(arg3, "divinity",   strlen(arg3))
    &&	 strncmp(arg3, "clout",      strlen(arg3))
    &&	 strncmp(arg3, "practice",   strlen(arg3))
    &&   strncmp(arg3, "train",	     strlen(arg3)))

    {
        ch->println("`cValid types: clout, divinity, experience, fame, gold, mps, or object.`x");
        ch->println( "`YNOTE: Substitute the object's VNUM for the amount.`x");
        return;
    }

	if (( victim = get_whovis_player_world(ch, arg1)) == NULL)
	{ch->println( "`cThat player is not here.`x" );return;}
       
    if ( IS_NPC( victim ) )
    {
        ch->println( "`cNot on NPC's.`x");
        return;
    }

    level = get_trust(ch);
    levelvi = get_trust(victim);

    if ((level<=levelvi) && (ch != victim))
    {
        ch->println( "`cYou can only award those of lesser trust level.`x");
        return;
    }


    value = atoi(arg2);
    
    if (value == 0)
    {
       ch->println("`cThe value must not be equal to 0.`x");
       return;
    }

    /*if (arg3[0] == 'a' ) alignment
    {
       if ( value < -2000 || value > 2000 )
       {
           send_to_char( "`cYou can only alter someone's alignment by -2000 to 2000 points.`x\n\r", ch );
           return;
       }
    
       if ((victim->alignment+value) > 1000)
       {
          value = (1000 - victim->alignment);
          victim->alignment = 1000;
          maxed = strdup( "high" );
       }

       else if ((victim->alignment+value) < -1000)
       {
          value = (-1000 - victim->alignment);
          victim->alignment = -1000;
          maxed = strdup( "low" );
       }
       else
          victim->alignment += value;

       if ( value == 0 )
       {
          sprintf( buf, "%s's alignment is already as %s as it can be.\n\r", victim->name, maxed );
          send_to_char(buf, ch);
          return;
       }
       else
       {
          sprintf( buf,"You alter %s's alignment by %d points.\n\r",victim->name, value);
          send_to_char(buf, ch);
       
          if ( value > 0 )
          { 
             sprintf( buf,"Your soul feels lighter and more virtuous!");
             send_to_char( buf, victim );
             return;
          }
          else
          {
             sprintf( buf,"You shudder deeply as your soul darkens.");
             send_to_char( buf, victim );
             return;
          }
       }
    }
*/
    if (arg3[0] == 'e') /* experience */
    {
       /* Cannot use the 'gain_exp' function since it won't
          give experience if the player is in the arena and it
          takes con away if they go below minimum experience
          (which could make them self-destruct).   That's just
          too mean to do during an 'award', since it might happen
          by mistake. */
          
    	if ( victim->level >= LEVEL_HERO )
       {
	ch->printlnf("`c%s cannot receive experience bonuses.`x", victim->name);
          return;
       }
       
       if (value < -1000000 || value > 1000000)
       {
           ch->println( "`cYou can only award between -1000000 and 1000000 experience.`x");
           return;
       }

       if (victim->exp < 0 && value < 0 && victim->exp < ((-1*maxlong) - value) )
       {
          value   = (-1*maxlong) - victim->exp;
          victim->exp = (-1*maxlong);
          maxed   = strdup( "minumum" );
       }
       else if ( victim->exp > 0 && value > 0 && victim->exp > (maxlong - value) )
       {
          value   = maxlong - victim->exp;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          //victim->exp += value;
	gain_exp(victim,value);

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s experience points possible.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d experience points.`x",victim->name, value);
       }
          if (value > 0)
          { 
             victim->printlnf("`cYou receive %d experience points for your efforts!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYou are drained of %d experience points!`x", value );
             return;
          }
       }
    
    
    if (arg3[0] == 'g')  /* gold */
    {
       if ( value < -9999999 || value > 9999999 )
       {
           ch->println( "`cYou can only award between -9999999 and 9999999 gold.`x");
           return;
       }

       if ( value < 0 && victim->gold < value )
       {
          value    = -1*victim->gold;
          victim->gold = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->gold > (maxlong - value) )
       {
          value   = maxlong - victim->exp;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->gold += value;
          
       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of gold allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d gold coins.`x",victim->name, value);
       
          if ( value > 0 )
          { 
             victim->printlnf("`cYour coin pouch grows heavier! You gain %d gold coins!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYour coin pouch grows lighter! You lose %d gold coins!`x", value );
             return;
          }
       }
    }

    if (arg3[0] == 'o' )   /* objects */
    {
       OBJ_INDEX_DATA *pObjIndex;
       OBJ_DATA *obj;

       if ( ( pObjIndex = get_obj_index( atoi( arg2 ) ) ) == NULL )
       {
           ch->println( "`cThere is no object with that vnum.`x");
           return;
       }

       obj = create_object( pObjIndex );

       if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
       {
          ch->printlnf("`cAlas, %s is carrying too many items to receive that.`x",victim->name);
          extract_obj( obj );
          return;
       }

       if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
       {
          ch->printlnf( "`cAlas, that is too heavy for %s to carry.`x",victim->name);
          extract_obj( obj );
          return;
       }
       
       obj_to_char( obj, victim );
       ch->printlnf("`cYou award %s item %d.`x", victim->name, value);

       
       victim->printlnf("`cYour load seems heavier!`x");
       return;
    }

    if (arg3[0] == 'f')  /* fame */
    {
       if ( value < -1000 || value > 1000 )
       {
           ch->println( "`cYou can only award between -1000 and 1000 fame.`x");
           return;
       }

       if ( value < 0 && victim->pcdata->fame < value )
       {
          value    = -1*victim->pcdata->fame;
          victim->pcdata->fame = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->pcdata->fame > (maxlong - value) )
       {
          value   = maxlong - victim->pcdata->fame;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->pcdata->fame += value;
          
       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of fame allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d fame.`x",victim->name, value);
       
          if ( value > 0 )
          { 
             victim->printlnf("`cYour fame increases!! You gain %d fame!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYour fame wans like the full moon! You lose %d fame!`x", value );
             return;
          }
       }
    }

   if (arg3[0] == 'm')  /* military points */
    {
       if ( value < -1000 || value > 1000 )
       {
           ch->println( "`cYou can only award between -1000 and 1000 Military Points.`x");
           return;
       }

       if ( value < 0 && victim->pcdata->military_points < value )
       {
          value    = -1*victim->pcdata->military_points;
          victim->pcdata->military_points = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->pcdata->military_points > (maxlong - value) )
       {
          value   = maxlong - victim->pcdata->military_points;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->pcdata->military_points += value;

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of Military Points allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d MPs.`x",victim->name, value);

          if ( value > 0 )
          {
             victim->printlnf("`cYour Military Reputation grows! You gain %d MPs!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYour Military Reputation is tarnished! You lose %d MPs!`x", value);
             return;
          }
       }
    }

   if (arg3[0] == 'd')  /* divinity */
    {
       if ( value < -1000 || value > 1000 )
       {
           ch->println( "`cYou can only award between -1000 and 1000 Divinity Points.`x");
           return;
       }

       if ( value < 0 && victim->pcdata->divinity < value )
       {
          value    = -1*victim->pcdata->divinity;
          victim->pcdata->military_points = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->pcdata->divinity > (maxlong - value) )
       {
          value   = maxlong - victim->pcdata->divinity;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->pcdata->divinity += value;

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of Divinity allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou increase %s Divinty by %d.`x",victim->name, value);

          if ( value > 0 )
          {
             victim->printlnf("`cYour Divine Favour grows! You gain %d Divinity!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYour favour with the Gods is diminished! You lose %d Divinity!`x", value);
             return;
          }
       }
    }

if (arg3[0] == 'c')  /* clout */
    {
       if ( value < -1000 || value > 1000 )
       {
           ch->println( "`cYou can only award between -1000 and 1000 clout.`x");
           return;
       }

       if ( value < 0 && victim->pcdata->clout < value )
       {
          value    = -1*victim->pcdata->clout;
          victim->pcdata->clout = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->pcdata->clout > (maxlong - value) )
       {
          value   = maxlong - victim->pcdata->clout;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->pcdata->clout += value;

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of Clout allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d Clout.`x",victim->name, value);

          if ( value > 0 )
          {
             victim->printlnf("`cYour Clout increases!! You gain %d Clout!`x", value );
             return;
          }
          else
          {
             victim->printlnf("`cYour Clout wans like the full moon! You lose %d Clout!`x", value );
             return;
          }
       }
    }
}    

if (arg3[0] == 'p')  /* practice */
    {
       if ( value < -5 || value > 5 )
       {
           ch->println( "`cYou can only award between -5 and 5 pracs.`x");
           return;
       }

       if ( victim->practice < 0 && value < 0 && victim->practice < ((-1*maxlong) - value) )
       {
	  value = (-1*maxlong) - victim->practice;
          victim->practice = (-1*maxlong);
          maxed    = strdup( "minumum" );          
       }
       else if ( victim->practice > 0 && value > 0 && victim->practice > (maxlong - value) )
       {
          value   = maxlong - victim->practice;
          victim->practice = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->practice += value;

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of practices allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d practices.`x",victim->name, value);

          if ( value > 0 )
          {
             victim->printlnf("`cYou gain %d practices! You now have %d practices.`x", value, victim->practice );
             return;
          }
          else
          {
             victim->printlnf("`cThe Gods are displeased, you lose %d practices.`x", value );
             return;
          }
       }
    }

if (arg3[0] == 't')  /* train */
    {
       if ( value < -5 || value > 5 )
       {
           ch->println( "`cYou can only award between -5 and 5 trains.`x");
           return;
       }

       if ( victim->train < 0 && value < 0 && victim->train < ((-1*maxlong) - value) )
       {
          value = (-1*maxlong) - victim->train;
          victim->train = (-1*maxlong);
          maxed    = strdup( "minumum" );
       }
       else if ( victim->train > 0 && value > 0 && victim->train > (maxlong - value) )
       {
          value   = maxlong - victim->train;
          victim->train = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->train += value;

       if ( value == 0 )
       {
          ch->printlnf("`c%s already has the %s amount of trains allowed.`x", victim->name, maxed );
          return;
       }
       else
       {
          ch->printlnf("`cYou award %s %d trains.`x",victim->name, value);

          if ( value > 0 )
          {
             victim->printlnf("`cYou gain %d trains! You now have %d trains.`x", value, victim->train );
             return;
          }
          else
          {
             victim->printlnf("`cThe Gods are displeased, you lose %d trains.`x", value );
             return;
          }
       }
    }


    return;
}
/******************************************************************************/
void do_punish( char_data *ch, char *argument)
{
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

        victim = get_whovis_player_world(ch, arg1);
        if(!victim)
	{ch->printlnf("`cYou can't seem to find '%s' to punish.`x", arg1);
        return;}

	if(IS_NULLSTR(arg1) || IS_NULLSTR(arg2))
	{ch->printlnf("Syntax: PUNISH <player> <minor/major>");return;}

	if(IS_NPC(ch) && IS_NPC(victim))
	{ch->printlnf("Not on NPCs");return;}

	if(!IS_CITY_GUARD(ch) && !IS_LT_GUARD(ch) && !IS_IMMORTAL(ch))
	{do_huh(ch,"");return;}

	if(!str_cmp(arg2, "minor"))
	{ch->printlnf("`c%s jailed for 5 hours.`x", victim->name);
	victim->printlnf("`c%s has jailed you for 5 yours.`x", ch->name);
	char_from_room(victim);
        char_to_room(victim, get_room_index(ROOM_VNUM_JAIL));
	act("`c$n arrives to service their 5 hour sentence.`x", victim, NULL, NULL, TO_ROOM);
	get_obj_index(OBJ_VNUM_JAIL_TOKEN_5);
	obj = create_object(get_obj_index(OBJ_VNUM_JAIL_TOKEN_5));
	obj->timer = 5;
	obj_to_char(obj, victim);
	return;}

	if(!str_cmp(arg2, "major"))
	{ch->printlnf("`c%s jailed for 10 hours.`x", victim->name);
	victim->printlnf("`c%s has jailed you for 10 hours.`x", ch->name);
	char_from_room(victim);
        char_to_room(victim, get_room_index(ROOM_VNUM_JAIL));
	act("`c$n arrives to service their 10 hour sentence.`x", victim, NULL, NULL, TO_ROOM);
	get_obj_index(OBJ_VNUM_JAIL_TOKEN_10);
	obj = create_object(get_obj_index(OBJ_VNUM_JAIL_TOKEN_10));
	obj->timer = 10;
	obj_to_char(obj, victim);
	return;}
}
/**************************************************************************/
/*				God Powers - Hera April 2005                      */
/**************************************************************************/
void bless_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont bless mobs at this stage :)
		return;
	}

	affect_parentspellfunc_strip( victim, gsn_bless);
	spell_bless(	gsn_bless,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`c$n has has blessed you.`x", ch, NULL, victim, TO_VICT);
			}else{
				if (can_see_who(victim, TRUE_CH(ch))){
					victim->printlnf("`c%s has blessed you.`x", TRUE_CH(ch)->name);
				}else{
					victim->println("`cAn Olympian god has blessed you.`x");
				}
			}
		}
	}	
}
/**************************************************************************/
// Hera April 05
void do_bless( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
	{
		// bless room
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			bless_victim( ch, vch);
        }
        sprintf(buf,"$N blessed room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
		ch->println( "`cRoom blessed.`x" );
        return;
    }

	if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
	{
		// bless all players in the game
		for(victim=player_list; victim; victim=victim->next_player)
		{
			if(IS_NPC(victim))
				continue;

			bless_victim( ch, victim);
		}
		ch->println( "`cAll active players blessed.`x" );
		return;
	}

	// restore a specific player/mob
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "`cThey aren't here.`x" );
		return;
	}
	bless_victim( ch, victim);

	sprintf(buf,"$N blessed %s", IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

	if (IS_SILENT(ch)){
		ch->printlnf("`cYou have silently blessed %s.`x", victim->name );
	}else{
		ch->printlnf("`cYou have blessed %s.`x", victim->name );
	}
	return;
}
/**************************************************************************/
void do_shout( char_data *ch, char *argument )
{
	connection_data *d;
    
	if(IS_NPC(ch))
	{do_huh(ch,"");return;}

	if ( argument[0] == '\0' )
	{
	ch->println( "`cShout what?`x" );
	return;
	}

   
	if(!str_cmp(ch->name, "zeus"))
	{
		for ( d = connection_list; d != NULL; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING )
			{
			d->character->printlnf("`CZeus`c shouts forth from atop Mount Olympus:\n\r'%s'`x", argument);
			}
		}
	}
	
        else if(!str_cmp(ch->name, "hera"))
        {
                for ( d = connection_list; d != NULL; d = d->next )
                {
                        if ( d->connected_state == CON_PLAYING )
                        {
                        d->character->printlnf("`CIris`c messenger of `CHera`c proclaims on her behalf:\n\r'%s'`x", 
argument);
                        }
                }
        }

        else if(!str_cmp(ch->name, "aphrodite"))
        {
                for ( d = connection_list; d != NULL; d = d->next )
                {
                        if ( d->connected_state == CON_PLAYING )
                        {
                        d->character->printlnf("`cA pair of doves fly across the skies carrying `CAphrodite`c's message: 
'%s'`x",argument);
                        }
                }
        }

	else
	{
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING )
			{
			d->character->printlnf( "`cHermes flies by with a message from `C%s`c:\n'%s'`x", ch->name, argument );
			}
		}
	}
    return;
}
/**************************************************************************/
void do_powers( char_data *ch, char * )
{
	if (IS_NPC(ch))
	{
		ch->printlnf( "`cMobiles do not have god-powers.`x");
		return;
	}
	if ( IS_IMMORTAL(ch)) {
		ch->printlnf( "`cImmortals have the following Special Powers:\nBless\nShout`x");
	}

	if(!str_cmp(ch->name, "zeus")){
		ch->printlnf( "`cAs a high level immortal, you have these powers:\nBoon\nRestore`x");
		ch->printlnf( "`cAs Zeus, you have these powers:\nChain`x");
	}
	if(!str_cmp(ch->name, "hera")){
		ch->printlnf( "`cAs a high level immortal, you have these powers:\nBoon\nRestore`x");
	}
	if(!str_cmp(ch->name, "eris")){
		ch->printlnf( "`cAs a high level immortal, you have these powers:\nBoon\nRestore`x");
		ch->printlnf( "`cAs Eris, you have these powers:\nApple\nDiscord`x");
	}
	if(!str_cmp(ch->name, "aphrodite")){
		ch->printlnf( "`cAs Aphrodite, you have these powers:\nRomance`x");
	}
	if(!str_cmp(ch->name, "dionysus")){
		ch->printlnf( "`cAs Dionysus, you have these powers:\nDrunk\nSober`x");
	}
	if(!str_cmp(ch->name, "poseidon")){
		ch->printlnf( "`cAs Poseidon, you have these powers:\nTyphoon`x");
	}
	if(!str_cmp(ch->name, "nemesis")){
		ch->printlnf( "`cAs Nemesis, you have these powers:\nAssassins`x");
	}
}
/**************************************************************************/
void chain_lightning_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont fry mobs at this stage :)
		return;
	}

	spell_chain_lightning(	gsn_chain_lightning,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`c$n has sent a chain of lightning through you.`x", ch, NULL, victim, TO_VICT);
			}else{
				if (can_see_who(victim, TRUE_CH(ch))){
					victim->printlnf("`c%s has sent a chain of lightning through you.`x", TRUE_CH(ch)->name);
				}else{
					victim->println("`cA chain of lightning has gone through your body.`x");
				}
			}
		}
	}	
}
/**************************************************************************/
void do_chain( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if(!str_cmp(ch->name, "zeus")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			// fry room
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				chain_lightning_victim( ch, vch);
        		}
        		sprintf(buf,"$N sends chain lightning through room %d.",ch->in_room->vnum);
        		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
			ch->println( "`cRoom chain'd.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			// fry all players in the game
			for(victim=player_list; victim; victim=victim->next_player)
			{
				if(IS_NPC(victim))
				continue;

				chain_lightning_victim( ch, victim);
			}
			ch->println( "`cAll active players chain'd.`x" );
			return;
		}

		// fry a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		chain_lightning_victim( ch, victim);

		sprintf(buf,"$N chain'd %s", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		if (IS_SILENT(ch)){
			ch->printlnf("`cYou have silently chain'd %s.`x", victim->name );
		}else{
			ch->printlnf("`cYou have chain'd %s.`x", victim->name );
		}
		return;
	}else{
        	ch->println( "`cLightning is Zeus' weapon, not yours.`x" );
   		return;
	}	

}
/**************************************************************************/
void bolt_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont fry mobs at this stage :)
		return;
	}

	spell_lightning_bolt(	gsn_lightning_bolt,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`c$n has thrown a thunderbolt at you!.`x", ch, NULL, victim, TO_VICT);
			}else{
				if (can_see_who(victim, TRUE_CH(ch))){
					victim->printlnf("`c%s has thrown a thunderbolt at you!`x", TRUE_CH(ch)->name);
				}else{
					victim->println("`cA thunderbolt falls from the sky, striking you!`x");
				}
			}
		}
	}	
}
/**************************************************************************/
void do_bolt( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if(!str_cmp(ch->name, "zeus")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			// fry room
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				bolt_victim( ch, vch);
        		}
        		sprintf(buf,"$N drops a thunderbolt onto room %d.",ch->in_room->vnum);
        		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
			ch->println( "`cRoom bolt'd.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			// fry all players in the game
			for(victim=player_list; victim; victim=victim->next_player)
			{
				if(IS_NPC(victim))
				continue;

				bolt_victim( ch, victim);
			}
			ch->println( "`cAll active players bolt'd.`x" );
			return;
		}

		// fry a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		bolt_victim( ch, victim);

		sprintf(buf,"$N bolt'd %s", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		if (IS_SILENT(ch)){
			ch->printlnf("`cYou have silently bolt'd %s.`x", victim->name );
		}else{
			ch->printlnf("`cYou have bolt'd %s.`x", victim->name );
		}
		return;
	}else{
        	ch->println( "`cLightning is Zeus' weapon, not yours.`x" );
   		return;
	}	

}
/**************************************************************************/
void romance_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont love mobs at this stage :)
		return;
	}

	spell_charm_person(	gsn_charm_person,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`c$n works her magic on you.`x", ch, NULL, victim, TO_VICT);
			}else{
				victim->printlnf("`c%s works her magic on you.`x", TRUE_CH(ch)->name);
			}
		}
	}	
}
/**************************************************************************/
void do_romance( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;


	one_argument( argument, arg );
	if(!str_cmp(ch->name, "aphrodite")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			ch->println( "`cLove works best between two people, not a room.`x" );
			ch->println( "`cSave the orgy for cable.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			ch->println( "`cLove works best between two people, not the world.`x" );
			ch->println( "`cSave the orgy for cable.`x" );
			return;
		}

		// love a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		romance_victim( ch, victim);

		sprintf(buf,"$N romanced %s", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		ch->printlnf("`cYou have romanced %s.`x", victim->name );

		return;
	}else{
        	ch->println( "`cLove is Aphrodite's weapon, not yours.`x" );
   		return;
	}	

}
/**************************************************************************/
void sober_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont sober mobs at this stage :)
		return;
	}

	spell_sober(	gsn_sober,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`cBy the power of $n, you are sober.`x", ch, NULL, victim, TO_VICT);
			}else{
				victim->printlnf("`cBy the power of %s, you are sober.`x", TRUE_CH(ch)->name);
			}
		}
	}	
}
/**************************************************************************/
void do_sober( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if(!str_cmp(ch->name, "dionysus")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			// sober room
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				sober_victim( ch, vch);
        		}
        		sprintf(buf,"$N sobers %d.",ch->in_room->vnum);
        		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
			ch->println( "`cRoom sober.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			// sober all players in the game
			for(victim=player_list; victim; victim=victim->next_player)
			{
				if(IS_NPC(victim))
				continue;

				sober_victim( ch, victim);
			}
			ch->println( "`cAll active players now sober.`x" );
			return;
		}

		// sober a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		sober_victim( ch, victim);

		sprintf(buf,"$N sobers up %s", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		if (IS_SILENT(ch)){
			ch->printlnf("`cYou have silently sobered up %s.`x", victim->name );
		}else{
			ch->printlnf("`cYou have sobered up %s.`x", victim->name );
		}
		return;
	}else{
        	ch->println( "`cBooze is Dionysus' weapon, not yours.`x" );
   		return;
	}	
}
/**************************************************************************/
void drunk_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont lush mobs at this stage :)
		return;
	}

	spell_drunkeness(	gsn_drunkeness,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`cBy the power of $n, you are drunk!`x", ch, NULL, victim, TO_VICT);
			}else{
				victim->printlnf("`cBy the power of %s, you are drunk!`x", TRUE_CH(ch)->name);
			}
		}
	}	
}
/**************************************************************************/
void do_drunk( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if(!str_cmp(ch->name, "dionysus")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			// lush room
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				drunk_victim( ch, vch);
        		}
        		sprintf(buf,"$N gets %d drunk.",ch->in_room->vnum);
        		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
			ch->println( "`cRoom drunk.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			// sober all players in the game
			for(victim=player_list; victim; victim=victim->next_player)
			{
				if(IS_NPC(victim))
				continue;

				drunk_victim( ch, victim);
			}
			ch->println( "`cAll active players now drunk.`x" );
			return;
		}

		// lush a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		drunk_victim( ch, victim);

		sprintf(buf,"$N gets %s drunk", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		if (IS_SILENT(ch)){
			ch->printlnf("`cYou have silently gotten %s drunk.`x", victim->name );
		}else{
			ch->printlnf("`cYou have gotten %s drunk.`x", victim->name );
		}
		return;
	}else{
        	ch->println( "`cBooze is Dionysus' weapon, not yours.`x" );
   		return;
	}	

}
/**************************************************************************/
void typhoon_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // dont storm mobs at this stage :)
		return;
	}

	spell_earthquake(	gsn_earthquake,	ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "`cThe seas of $n rage against you!`x", ch, NULL, victim, TO_VICT);
			}else{
				victim->printlnf("`cThe seas of %s rage against you!`x", TRUE_CH(ch)->name);
			}
		}
	}	
}
/**************************************************************************/
void do_typhoon( char_data *ch, char *argument )
{
    	char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if(!str_cmp(ch->name, "poseidon")){	

		if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
		{
			// storm room
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				typhoon_victim( ch, vch);
        		}
        		sprintf(buf,"$N typhoons %d.",ch->in_room->vnum);
        		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
			ch->println( "`cRoom typhoon'd.`x" );
        		return;
    		}

		if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
		{
			// typhoon all players in the game
			for(victim=player_list; victim; victim=victim->next_player)
			{
				if(IS_NPC(victim))
				continue;

				typhoon_victim( ch, victim);
			}
			ch->println( "`cAll active players typhoon'd.`x" );
			return;
		}

		// storm a specific player/mob
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "`cThey aren't here.`x" );
			return;
		}
		typhoon_victim( ch, victim);

		sprintf(buf,"$N typhoon'd %s", IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

		if (IS_SILENT(ch)){
			ch->printlnf("`cYou have silently sent a typhoon against %s.`x", victim->name );
		}else{
			ch->printlnf("`cYou have sent a typhoon against %s.`x", victim->name );
		}
		return;
	}else{
        	ch->println( "`cThe rage of the sea is under the power of Poseidon.`x" );
   		return;
	}	
}
/***************************************************************************/
void do_powerhelp( char_data *ch, char *argument)
{
    char arg[MIL];
    one_argument( argument, arg );

	if(!str_cmp(arg, "bless"))
	{
 		ch->println("`cSyntax: bless <victim>`x");
        	ch->println("`cBlesses the victim.`x");
 		ch->println("`cSyntax: bless room`x");
        	ch->println("`cBlesses everyone in the room you are standing in.`x");
 		ch->println("`cSyntax: bless all`x");
        	ch->println("`cBlesses everyone in the game.`x");
		return;
	}
	if(!str_cmp(arg, "shout"))
	{
 		ch->println("`cSyntax: shout <message>`x");
        	ch->println("`cShouting sends a message to the whole game.`x");
		return;
	}
	if(!str_cmp(arg, "chain"))
	{
 		ch->println("`cSyntax: chain <victim>`x");
        	ch->println("`cChain sends lightning bolts to the victim and those around them.`x");
 		ch->println("`cSyntax: chain room`x");
        	ch->println("`cChains everyone in the room you are standing in.`x");
 		ch->println("`cSyntax: chain all`x");
        	ch->println("`cChains everyone in the game.`x");
		return;
	}
	if(!str_cmp(arg, "bolt"))
	{
 		ch->println("`cSyntax: bolt <victim>`x");
        	ch->println("`cStrikes the victim with a bolt of lightning.`x");
 		ch->println("`cSyntax: bolt room`x");
        	ch->println("`cBolts everyone in the room you are standing in.`x");
 		ch->println("`cSyntax: bolt all`x");
        	ch->println("`cBolts everyone in the game.`x");
		return;
	}
	if(!str_cmp(arg, "romance"))
	{
 		ch->println("`cSyntax: romance <victim1> <victim2>`x");
        	ch->println("`cCauses victim1 to become smitten with victim2.`x");
		return;
	}
	if(!str_cmp(arg, "sober"))
	{
 		ch->println("`cSyntax: sober <victim>`x");
        	ch->println("`cMakes the victim sober.`x");
		return;
	}
	if(!str_cmp(arg, "drunk"))
	{
 		ch->println("`cSyntax: drunk <victim>`x");
        	ch->println("`cMakes the victim drunk.`x");
		return;
	}
	if(!str_cmp(arg, "assassins"))
	{
 		ch->println("`cSyntax: assassins <on/off>`x");
        	ch->println("`cTurn on or off the mortal ability to hire assassins.`x");
		return;
	}
	if(!str_cmp(arg, "apple"))
	{
 		ch->println("`cSyntax: apple <throw/catch>`x");
        	ch->println("`cTurn on or off the apple betting event.`x");
		return;
	}
 		ch->println("`cSyntax: powerhelp <power>`x");
        	ch->println("`cType `RPOWERS`c to see your list of powers.`x");
		return;
}
/***************************************************************************/
void do_transform( char_data *ch, char *argument )
{

      char arg1[MIL];
      char arg2[MIL];
      char_data *victim;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

if(!str_cmp(ch->name, "hera"))
{

        if (( victim = get_whovis_player_world(ch, arg1)) == NULL)
        {ch->println( "That player is not here." );return;}

	if(ch == victim)
	{ch->printlnf("Hera, why are you trying to transform yourself?");
	return;}



                if (arg1[0] == '\0' || arg2[0] == '\0')
                {
			ch->printlnf("`cSyntax: Transform (person) (thing)`x");
			ch->printlnf("`cthing: gnat`x");
			return;
                }


                if(!str_cmp(arg2, "gnat"))
                {

                        ch->printlnf("You have transformed %s into a %s", victim->name, arg2);
                        return;
                }
 
		  else if(!str_cmp(arg2, "test"))
			{
                        victim->println( "In a fit of rage, Hera has transformed you!" );
                        ch->printlnf("You have transformed them.");
			return;
			}
   }else{
   ch->println( "`cTransformation is a torture only available to Hera.`x" );

   return;
   }
}
/***************************************************************************/
void do_multicheck(char_data *ch, char *argument)
{
	connection_data *d;
	connection_data *d2;
	bool found = false;
	#define LOOKUP_STATUS     0
	#define STATUS_DONE       1
 
	for (d = connection_list; d != NULL; d = d->next)
	{
		if (d->connected_state != CON_PLAYING || IS_IMMORTAL(ch))
		continue;
		for (d2 = d->next; d2 != NULL; d2 = d2->next)
		{
			if (!str_cmp(d->remote_hostname, d2->remote_hostname))
			{
			if (d2->connected_state != CON_PLAYING)
			continue;
			if (d2->character == NULL || d->character == NULL)
			continue;
			found = true;
			ch->printlnf("`c%s and %s are multiplaying.`x", d2->character->name, d->character->name);
			}
		}
	}
	if (!found)
	ch->printlnf("`cNoone is multiplaying currently.`x");
	return;
}
/***************************************************************************/
