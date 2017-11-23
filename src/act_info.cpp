/**************************************************************************/
// act_info.cpp - primarily code showing players information
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

#include "include.h" // dawn standard includes
#include "magic.h"
#include "olc.h"
#include "duel.h"
#include "math.h"
#include "msp.h"
#include "ictime.h"
#include "lockers.h"
#include "nanny.h"
#include <time.h>


/* command procedures needed */
DECLARE_DO_FUN( do_exits    );
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_map); // Kal
DECLARE_DO_FUN( do_tracks); // Kal
DECLARE_DO_FUN( do_flags); // Hera

int get_birthmonth( char_data *ch );
char *get_weapontype(OBJ_DATA *obj);

void where_char    args((char_data *victim, char_data *ch,
                            sh_int depth, sh_int door));

char *const where_distance[2] = {
    "in the same room.",
    "directly to the %s."
};
void letter_read( char_data *ch, OBJ_DATA *letter );

DECLARE_DO_FUN( do_count        );

char *	const	where_name	[] =
{
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
	"`c<lodged in arm>`x     ",
	"`c<lodged in leg>`x     ",
	"`c<lodged in rib>`x     ",
	"`c<sheathed>`x          ",
	"`c<concealed>`x         ",
    "",
    "",
    "",
    "",
    "",
    "",
	""
};


/*
 * Local functions.
 */
void	show_list_to_char	args( ( OBJ_DATA *list, char_data *ch, char *filter,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( char_data *victim, char_data *ch ) );
void	show_char_to_char_1	args( ( char_data *victim, char_data *ch ) );
void	show_char_to_char	args( ( char_data *list, char_data *ch ) );
bool	check_blind		args( ( char_data *ch ) );

/**************************************************************************/
char *get_canwear_colour( OBJ_DATA *obj, char_data *ch)
{
    int can_wear=0; // if not 0, it can't be worn

	
		
    if(  (  IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
		    && HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY)
		  )
      ||(!IS_NPC(ch) && obj->pIndexData->relative_size<
                           race_table[ch->race]->low_size &&
            obj->item_type != ITEM_LIGHT &&
            !CAN_WEAR( obj, OBJWEAR_HOLD)  &&
            !CAN_WEAR( obj, OBJWEAR_FLOAT) )
      ||(!IS_NPC(ch) && obj->pIndexData->relative_size>
                           race_table[ch->race]->high_size &&
            obj->item_type != ITEM_LIGHT &&
            !CAN_WEAR( obj, OBJWEAR_FLOAT) )
      )
    {
        can_wear=1;
    }

    if(!IS_NPC(ch) && !can_wear)
    {
		can_wear = DISALLOWED_OBJECT_FOR_CHAR(obj, ch)?1:0;
    }

	// restrictions for objects
    if(!IS_NPC(ch) && !can_wear)
    {
		if(obj->pIndexData && ch->pcdata
			&& HAS_CONFIG(ch,CONFIG_OBJRESTRICT) 
			&& ((ch->pcdata->objrestrict& obj->pIndexData->objrestrict)>0))
		{
			can_wear=2;
		}

	}

	switch(can_wear){
	case 0:
		return "`=d";
	case 1:
		return "`=k";
	case 2:
		return "`=K";
	}

	bugf("get_canwear_colour(): unknown value %d", can_wear);
	return "`x";
}
/**************************************************************************/
char *format_obj_to_char_new( OBJ_DATA *obj, char_data *ch, bool fShort )
{
    static char buf[MSL];
    buf[0] = '\0';
	char name[MSL];
    name[0] = '\0';


	// get the objects name into 'name'
    if ( fShort ){
		if ( !IS_NULLSTR(obj->short_descr) ){
			strcpy( name, obj->short_descr );
		}
    }else{
		if ( !IS_NULLSTR(obj->description) ){
			strcpy( name, obj->description );
		}
    }

    if ( ( fShort && IS_NULLSTR(obj->short_descr) )
		|| (!fShort && IS_NULLSTR(obj->description) ) )
	{
		if (HAS_HOLYLIGHT(ch))
		{
			char buf2[MSL];

			sprintf(buf2," `#`m(no short|long%s%d)", 
				obj->pIndexData?" ":"", obj->pIndexData?obj->pIndexData->vnum:0);
			strcat(buf,buf2);
			if (obj->pIndexData && 
				!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM))
			{
				sprintf(buf2," `#`g[%d,%d]", obj->pIndexData->vnum, obj->level);
				strcat(buf,buf2);
			}
			strcat(buf,"`&");			
		}
		return buf;
	}

	
	
	
    if ( IS_OBJ_STAT(obj, OBJEXTRA_INVIS)   )		strcat( buf, game_settings->aura_invis);
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
			&& IS_OBJ_STAT(obj, OBJEXTRA_EVIL) )	strcat( buf, game_settings->aura_evil);
    if ( IS_AFFECTED(ch, AFF_DETECT_GOOD)
		 &&  IS_OBJ_STAT(obj,OBJEXTRA_BLESS))		strcat( buf, game_settings->aura_good);
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, OBJEXTRA_MAGIC))		strcat( buf, game_settings->aura_magical);
    if ( IS_OBJ_STAT(obj, OBJEXTRA_GLOW)    )		strcat( buf, game_settings->aura_glowing);
    if ( IS_OBJ_STAT(obj, OBJEXTRA_CHAOS)	)		strcat( buf, game_settings->aura_chaos);	
	if ( IS_OBJ_STAT(obj, OBJEXTRA_HUM)		)		strcat( buf, game_settings->aura_hum);
	if ( IS_SET( obj->extra2_flags, OBJEXTRA2_BURIED))	strcat( buf, game_settings->aura_buried);
//	if ( IS_TRAPPED( obj )
//		&& IS_AFFECTED2( ch, AFF2_DET_TRAPS ))	strcat( buf, "{#{r(Trapped){& "		);

	if ( IS_WEAPON_STAT(obj, WEAPON_HOLY)     
		&&  IS_OBJ_STAT(obj,OBJEXTRA_ANTI_EVIL))   strcat(buf,game_settings->aura_holy);

	if(CAN_WEAR(obj, OBJWEAR_WIELD))
	{
		if ( IS_WEAPON_STAT(obj, WEAPON_HOLY) &&  IS_OBJ_STAT(obj,OBJEXTRA_ANTI_GOOD)){
			strcat(buf,game_settings->aura_unholy);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_FLAMING)){
			strcat(buf,game_settings->aura_flaming);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)){
			strcat(buf,game_settings->aura_vampric);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_SHOCKING)){
			strcat(buf,game_settings->aura_shocking);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_FROST)){
			strcat(buf,game_settings->aura_frost);
		}
    }

	// mxp the object
	strcat(buf, mxp_create_tag(ch, mxp_tag_for_object(ch, obj), name));
	
	if (!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM)){
		char buf2[MSL];

		if (obj->pIndexData)
		{
			sprintf(buf2," `#`g[%d,%d]`&", obj->pIndexData->vnum, obj->level);
			strcat(buf,buf2);
		}
	}
    return buf;
}

/**************************************************************************/
char *format_obj_to_char( OBJ_DATA *obj, char_data *ch, bool fShort )
{
    static char buf[MSL];
	sprintf(buf,"`#%s%s`&", 
		get_canwear_colour( obj, ch),
		format_obj_to_char_new( obj, ch, fShort ));
    return buf;
}
/**************************************************************************/
/* Show a list to a character.
 * Can coalesce duplicated items.
 * filter support added by Kalahn Feb 01.
 */
void show_list_to_char( OBJ_DATA *list, char_data *ch, char *filter, bool fShort, bool fShowNothing )
{
    char buf[MSL];
    BUFFER *output;
    char **prgpstrShow;
    char **prgpstrShowMXP=NULL;
    char **prgpstrColour;
    int *prgnShow;
    char *pstrShow;
	OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
	bool fCombine;

    if ( ch->desc == NULL ){
		return;
	}

	// pre count the number of items we will be displaying
    count = 0;
    for ( obj = list; obj; obj = obj->next_content ){
		count++;
	}
	if(count<1){
		return; // dont do anything if nothing to display
	}

	// record if they have MXP, if they do, initially work with non MXP versions
	// of the object list so we can compact easily without MXP UID's confusing
	// the compact system.
	bool player_has_mxp;
	if(HAS_MXP(ch)){
		TRUE_CH(ch)->pcdata->mxp_enabled=false;
		player_has_mxp=true;
	}else{
		player_has_mxp=false;
	}
	
	// Alloc space for output lines.
	output = new_buf();
	
	// allocate memory to handle the sorting of this list etc
	prgnShow      = (int *)alloc_mem( count * sizeof(int)    );
	prgpstrShow	  = (char **)alloc_mem( count * sizeof(char *) );
	prgpstrColour = (char **)alloc_mem( count * sizeof(char *) );
	if(player_has_mxp){
		prgpstrShowMXP =(char **)alloc_mem( count * sizeof(char *) );
	}
	
	nShow	= 0;
	
	// Format the list of objects.
	bool matched;
	for ( obj = list; obj != NULL; obj = obj->next_content )
	{ 
		// filter support for inventory list etc
		if(!IS_NULLSTR(filter)){
			matched=false;
			if(is_name( filter, obj->name)) {
				matched=true;
			}else if(is_name( filter, obj->short_descr )) {
				matched=true;
			}else if(is_name( filter, obj->description )) {
				matched=true;
			}else if(flag_value( item_types, filter) == obj->item_type){
				matched=true;
			}
			
			if(!matched){
				continue;
			}
		}
		
		if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
		{
			pstrShow = format_obj_to_char_new( obj, ch, fShort );
			fCombine = false;
			
			if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			{
				//  Look for duplicates, case sensitive.
				//  Matches tend to be near end so run loop backwords.
				for ( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow]++;
						fCombine = true;
						break;
					}
				}
			}
			
			// Couldn't combine, or didn't want to.
			if ( !fCombine )
			{
				prgpstrShow [nShow] = str_dup( pstrShow );
				prgnShow    [nShow] = 1;
				
				// record the colour code of this object 
				prgpstrColour[nShow]=str_dup(get_canwear_colour( obj, ch));
				
				// record MXP version if necessary
				if(player_has_mxp){
					TRUE_CH(ch)->pcdata->mxp_enabled=true;
					prgpstrShowMXP[nShow]=str_dup(format_obj_to_char_new( obj, ch, fShort ));
					TRUE_CH(ch)->pcdata->mxp_enabled=false;
				}
				nShow++;
			}
		}
	}
	
	// Output the formatted list.
	for ( iShow = 0; iShow < nShow; iShow++ )
	{
		if (prgpstrShow[iShow][0] == '\0')
		{
			free_string(prgpstrShow[iShow]);
			continue;
		}
		
		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
		{
			add_buf(output, "`#");
			add_buf(output, prgpstrColour[iShow]);
			
			if ( prgnShow[iShow] != 1 ){
				sprintf( buf, "(%2d) ", prgnShow[iShow] );
				add_buf(output,buf);
			}else{
				add_buf(output,"     ");
			}
		}
		
		if(player_has_mxp){
			add_buf(output,prgpstrShowMXP[iShow]);
			free_string( prgpstrShowMXP[iShow] );
		}else{
			add_buf(output,prgpstrShow[iShow]);
		}
		add_buf(output,"`&\r\n");
		
		free_string( prgpstrShow[iShow] );
		free_string( prgpstrColour[iShow] );
	}
	
	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			ch->print("     ");
		ch->println( "Nothing." );
	}
	
	ch->sendpage(buf_string(output));
	free_buf(output);
	
	// Clean up.
	free_mem( prgnShow,    count * sizeof(int)    );
	free_mem( prgpstrShow, count * sizeof(char *) );
	free_mem( prgpstrColour, count * sizeof(char *) );
	if(player_has_mxp){
		// free the memory allocated to store the MXP version of the list
		free_mem( prgpstrShowMXP, count * sizeof(char *) );
		// turn MXP back on for connection
		TRUE_CH(ch)->pcdata->mxp_enabled=true;
	}		
	return;
}


/**************************************************************************/
void show_char_to_char_0( char_data *victim, char_data *ch )
{
    char buf[MSL], buf2[MSL];
	
	//prototype
	char * char_position_text( char_data *ch); 

	if (IS_UNSWITCHED_MOB(ch))
	{
		return;
	}

    buf[0] = '\0';
    if( IS_SET(victim->comm,COMM_AFK        )   ) strcat( buf, "[AFK] "        );

    if(IS_LINKDEAD(victim)){
		if(victim->pload){
			strcat( buf, "`Y[PLOAD]`x ");
		}else{
			strcat( buf, "`Y[LINKDEAD]`x ");
		}
	}
	if( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    
	// wizi stuff
	if (INVIS_LEVEL(victim)&&  IS_IMMORTAL(ch)){
		if (INVIS_LEVEL(victim)==LEVEL_IMMORTAL){
			strcat( buf, "(Wizi) " );
		}else{
			buf2[0]='\0';
			sprintf(buf2, "(Wizi %d) ", INVIS_LEVEL(victim));
			strcat( buf, buf2);
			buf2[0]='\0';
		}
	}
/*
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "`#`g(Hide)`& "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "`#`c(Charmed)`& "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "`#`C(Translucent)`& ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "`#`r(Pink Aura)`& "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "`#`R(Red Aura)`& "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "`#`Y(Golden Aura)`& ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "`#`W(White Aura)`& " );
	// should only ever been seen by imms (because of can_see)
	if ( IS_AFFECTED2(victim, AFF2_TREEFORM)  ) strcat( buf, "`#`G(Tree)`& "	   );
	// imms and faeries
	if ( IS_AFFECTED2(victim, AFF2_VANISH)    ) strcat( buf, "`#`S(Vanished)`& "   );
*/	
    if ( IS_AFFECTED(victim, AFF2_GNAT)   ) strcat( buf, "`#`cTransformed into a gnat`& ");
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_IS_UNSEEN)){
		if (HAS_HOLYLIGHT(ch)){
			strcat( buf, "(UNSEEN) " );
		}else{
			return;
		}
    }

/////////////////////////////////////////////////////////////////////
// Below displays chars in positions that dont use victim->postion //
/////////////////////////////////////////////////////////////////////
	// subdued players
	if (IS_SUBDUED(victim))
	{
		ch->printlnf( "%s%s is here looking a little subdued.",
			buf, capitalize( PERS( victim, ch )));
        return;
	}

	// mounted players
	if (IS_MOUNTED(victim))
	{
		if (IS_RIDING(ch)==victim)
		{
			// we are riding the victim
			ch->printlnf( "`c%s`#`s`cYou are riding on %s.`x`&",
				buf, PERS( victim, ch ));
			return;
		}
		else
		{
			// the victim is being ridden, but not by us,
			// show them with the rider (not by themselves)
			return;
		}
	}

	// mob is tethered (NPC's only at this stage)
	if (IS_TETHERED(victim))
	{
		ch->printlnf( "`c%s`#`s`c%s is tethered here.`x`&",
			buf, capitalize( PERS( victim, ch )));
        return;
	}

	// characters riding others
	if (IS_RIDING(victim))
	{
		ch->printlnf( "`c%s`#`s`c%s is here riding `&`c%s.`x`B",
			buf,
			capitalize( PERS( victim, ch )),
			PERS( IS_RIDING(victim), ch ));
		return;
	}

/* NOTE: need to add 
* - subdued and tethered?
* - subdued and being ridden?
*/
//////////////////////////////////////////////////////////////
//    Above postions that dont use victim->postion above    //
//////////////////////////////////////////////////////////////

	// mobs that are in default position and have look descripts
	if (IS_NPC(victim)
		&& victim->position == victim->start_pos 
		&& !IS_NULLSTR(victim->long_descr))
	{
		ch->printlnf( "`c%s%s", buf, LONGPERS(victim, ch));
		return;
	}

	// players and mobs in positions defined with victim->pos
	strcat ( buf, PERS(victim, ch));
	switch ( victim->position )
	{
	case POS_DEAD:
		strcat( buf, "`c is DEAD!!`x" );
		break;
	case POS_MORTAL:
		strcat( buf, "`c is mortally wounded.`x" );
		break;
	case POS_INCAP:
		strcat( buf, "`c is incapacitated.`x" );
		break;
	case POS_STUNNED:
		strcat( buf, "`c is lying here stunned.`x" );
		break;
	case POS_SLEEPING:
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
	case POS_STANDING:
		strcat( buf, char_position_text(victim));
		break;
	case POS_FIGHTING:
		strcat( buf, "`c is here, fighting`x" );
		if ( victim->fighting == NULL )
			strcat( buf, "`c thin air??`x" );
		else if ( victim->fighting == ch )
			strcat( buf, "`c YOU!`x" );
		else if ( victim->in_room == victim->fighting->in_room )
		{
			strcat( buf, PERS( victim->fighting, ch ) );
			strcat( buf, "`c.`x" );
		}
		else
			strcat( buf, "`c someone who left??`x" );
		break;
	} // end of switch (victim->postion)

	buf[0]= UPPER(buf[0]);
	ch->println(buf);
	return;
}

/**************************************************************************/
void show_char_to_char_1( char_data *victim, char_data *ch )
{
	char buf[MSL];
	OBJ_DATA *obj;
	int iWear;
	int percent;
	bool found;
	bool peeking;

	peeking = ( victim != ch && !IS_NPC(ch)
		&& IS_SET(ch->act,PLR_AUTOPEEK)
		&&   number_percent( ) < get_skill(ch,gsn_peek));

	// can't peek at IC immortals unless you are an imm
	if (!IS_NPC(victim) && victim->level>= LEVEL_IMMORTAL)
	{
		peeking= false;
	}
	// ic imms always exceed at peeking
	if (!IS_NPC(ch) && (ch->level>= LEVEL_IMMORTAL) && IS_SET(ch->act,PLR_AUTOPEEK))
	{
		peeking= true;
	}

		
	if ( can_see( victim, ch ) )
	{
		if (ch == victim)
			act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
		else
		{
			if (peeking || number_percent( )<=5)
			{
				act( "`c$n looks at you thoroughly.`x", ch, NULL, victim, TO_VICT    );
				act( "`c$n looks at $N thoroughly.`x",  ch, NULL, victim, TO_NOTVICT );
			}
			else
			{
				act( "`c$n looks at you.`x", ch, NULL, victim, TO_VICT    );
				act( "`c$n looks at $N.`x",  ch, NULL, victim, TO_NOTVICT );
			}
		}
	}
	

	if ( victim->description[0] != '\0' )
	{
        ch->print(victim->description );
	}
	else
	{
        act( "`cYou see nothing special about $M.`x", ch, NULL, victim, TO_CHAR );
	}
	
	if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
	else
        percent = -1;
	
	strcpy( buf, PERS(victim, ch) );
	
	if (percent >= 100)
        strcat( buf, "`c is in excellent condition.`x\r\n");
	else if (percent >= 90)
        strcat( buf, "`c has a few scratches.`x\r\n");
	else if (percent >= 75)
        strcat( buf,"`c has some small wounds and bruises.`x\r\n");
	else if (percent >=  50)
        strcat( buf, "`c has quite a few wounds.`x\r\n");
	else if (percent >= 30)
        strcat( buf, "`c has some big nasty wounds and scratches.`x\r\n");
    else if (percent >= 15)
        strcat ( buf, "`c looks pretty hurt.`x\r\n");
    else if (percent >= 0 )
        strcat (buf, "`c is in `rawful`c condition.`x\r\n");
    else
        strcat(buf, "`c is `rbleeding`c to death.`x\r\n");
    
    buf[0] = UPPER(buf[0]);
    ch->print( buf );
    
    if (victim->mounted_on)
	{
		ch->printlnf( "`c%s is mounted on %s.`x",
			capitalize( victim->short_descr),
			victim->mounted_on->short_descr );
	}

	// flying status
	if ( IS_AFFECTED(victim, AFF_FLYING) ){
		ch->printlnf( "`c%s appears to be airborne.`x", PERS(victim,ch) );
	}

	// money changers
	if ( IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER)){
		ch->printlnf( "`c%s appears to be willing to exchange`x "
			"`cyour silver coins for gold.`x", PERS(victim,ch) );
    }

    
    found = false;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( iWear == WEAR_CONCEALED
		&&   ch != victim
		&&  !HAS_HOLYLIGHT( ch ))
			continue;

        if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
			&&   can_see_obj( ch, obj ) )
        {
			if ( !found )
			{
                ch->println( "" );
                act( "`c$N is using:`x", ch, NULL, victim, TO_CHAR );
                found = true;
			}
			ch->printlnf( "`=\xab""`c%s%s`x",
				where_name[iWear],
				format_obj_to_char( obj, ch, true ));
		}
	}
	
	if (peeking)
	{
		ch->printlnf( "\r\n`cYou peek at the inventory of %s:`x`=\xab",
			PERS(victim,ch));
		check_improve(ch,gsn_peek,true,4);
		show_list_to_char( victim->carrying, ch, "", true, true );
		ch->print( "`x" );
	}
	return;
}

/**************************************************************************/
void show_char_to_char( char_data *list, char_data *ch )
{
	char_data *rch;
	
	for ( rch = list; rch; rch = rch->next_in_room )
	{
		if ( rch == ch )
			continue;
		
		if ( get_trust(ch) < INVIS_LEVEL(rch))
			continue;
		
		if ( can_see( ch, rch ) )
		{
			show_char_to_char_0( rch, ch );
		}
		else if ( room_is_dark( ch->in_room )
			&&        IS_AFFECTED(rch, AFF_INFRARED ) &&
			(IS_NPC(rch) && !IS_SET(rch->act, ACT_IS_UNSEEN)) )
		{
			ch->println( "`cYou see glowing eyes in the darkness.`x" );
		}
	}
	
	return;
}


/**************************************************************************/
// Kalahn - June 98
void do_peek(char_data *ch, char *argument)
{
	char_data *victim;
	char arg[MIL];
	
	bool peeking; 
	
	argument = one_argument( argument, arg );

	victim = get_char_room( ch, arg );
	
	if (victim)
	{	
		if (victim==ch)
		{
			ch->println( "`cUse the inventory command to see your inventory.`x" );
		}

		peeking = (!IS_NPC(ch) && (number_percent( ) < get_skill(ch,gsn_peek)));

		// can't peek at IC immortals unless an IC imm yourself
		if (!IS_NPC(victim) && (victim->level>= LEVEL_IMMORTAL))
		{
			peeking= false;
		}

		if (peeking)
		{
			act( "`c$n looks at you thoroughly.`x", ch, NULL, victim, TO_VICT    );
			act( "`c$n looks at $N thoroughly.`x",  ch, NULL, victim, TO_NOTVICT );
		}
		else
		{
			act( "`c$n looks at you.`x", ch, NULL, victim, TO_VICT    );
			act( "`c$n looks at $N.`x",  ch, NULL, victim, TO_NOTVICT );
		}

		// ic imms always exceed at peeking
		if (!IS_NPC(ch) && ch->level>= LEVEL_IMMORTAL)
		{
			peeking= true;
		}

		ch->printlnf( "`cYou peek at the inventory of %s:`x`=\xab",
			PERS(victim,ch));
		check_improve(ch,gsn_peek,true,4);

		if (peeking){
			show_list_to_char( victim->carrying, ch, "", true, true );
		}else{
			ch->println( "`cYou didn't manage to see anything.`x" );
		}

		ch->print("`x" );
	}
	else
	{
		ch->println( "`cYou can't see them here.`x" );
	}
	return;
}


/**************************************************************************/
bool check_blind( char_data *ch )
{
    if (HAS_HOLYLIGHT(ch))
		return true;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
		ch->println( "`cYou can't see a thing!`x" );
		return false;
    }
	return true;
}

/**************************************************************************/
void do_consider( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    char msg[MIL];
    
    int diffhp;
    int difflevel;
    
    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        ch->println( "`cConsider killing whom?`x" );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		// do an object check 
		// get_obj_here() supports uid
		OBJ_DATA *obj= get_obj_here( ch, arg );
		if ( obj ){
			act( "`cYou can't fight $p.`x", ch, obj, NULL, TO_CHAR );
			return;
		}

        ch->println( "`cThey're not here.`x" );
        return;
    }

    if (is_safe(ch,victim))
	{
        ch->println( "`cDon't even think about it.`x" );
		return;
	}

    if ( victim == ch )
	{
		ch->println( "`cWhy are you considering hurting yourself?`x" );
		return;
	}


    diffhp = number_range(victim->hit* 7/10, victim->hit* 13/10) - 
             number_range(ch->hit* 8/10, ch->hit* 12/10);
    

         if ( diffhp <= -100 ) strcpy(msg, "`c$N looks very wimpy to you.\n\r`x");
    else if ( diffhp <=  -50 ) strcpy(msg, "`c$N is much weaker than you.\n\r`x");
    else if ( diffhp <=  -20 ) strcpy(msg, "`c$N is weaker than you.\n\r`x");
    else if ( diffhp <=   10 ) strcpy(msg, "`c$N is about as tough as you.\n\r`x");
    else if ( diffhp <=   40 ) strcpy(msg, "`c$N is tougher than you.\n\r`x");
    else if ( diffhp <=   90 ) strcpy(msg, "`c$N is quite intimidating to you.\n\r`x");
         else                  strcpy(msg, "`c$N is very intimidating in comparison to you.\n\r`x");

    if (IS_NPC(victim) && !IS_CONTROLLED(victim)){
		difflevel = victim->level - ch->level;
		difflevel = number_range(difflevel-2, difflevel+2); 
    
			 if ( difflevel <=  -11 ) strcat(msg, "`cYou think $E could be snapped like a twig.`x");
		else if ( difflevel <=  -8 ) strcat(msg, "`cYou think $E looks like an easy kill.`x"); 
		else if ( difflevel <=  -5 ) strcat(msg, "`cYou think $E could barely offer any resistance.`x"); 
		else if ( difflevel <=  -2 ) strcat(msg, "`cYou think $E would give you little trouble.`x");
		else if ( difflevel <=   2 ) strcat(msg, "`cYou think $E is a perfect match for you.`x");
		else if ( difflevel <=   5 ) strcat(msg, "`cYou think $E would put up a good fight.`x");
		else if ( difflevel <=   8 ) strcat(msg, "`cYou think $E would be difficult to defeat.`x");
		else if ( difflevel <=  11 ) strcat(msg, "`cYou think you had better get some help for this one.`x");      
			 else                  strcat(msg, "`cYou think $E would surely send you to the Underworld.`x");
	}
    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}

/**************************************************************************/
/* changes your scroll */
void do_scroll(char_data *ch, char *argument)
{
	char arg[MIL];
	int lines;

	one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		if (ch->lines == 0)
			ch->println( "`cYou do not page long messages.`x" );
		else
		{
			ch->printlnf( "`cYou currently display %d lines per page.`x", ch->lines + 2);
		}
		return;
	}

	if (!is_number(arg))
	{
		ch->println( "`cYou must provide a number.`x" );
		return;
	}

	lines = atoi(arg);
	
	if (lines == 0)
	{
		ch->println( "`cPaging disabled.`x" );
		ch->lines = 0;
		return;
	}

	if (lines < 10 || lines > 100)
	{
		ch->println( "`cYou must provide a reasonable number.`x" );
		return;
	}

	ch->printlnf( "`cScroll set to %d lines.", lines );
	ch->lines = lines - 2;
}

/**************************************************************************/
void do_motd(char_data *ch, char *)
{
    do_help(ch,"motd");
}

/**************************************************************************/
void do_imotd(char_data *ch, char *)
{
	 do_help(ch,"imotd");
}

/**************************************************************************/
void do_rules(char_data *ch, char *)
{
    do_help(ch,"rules");
}

/**************************************************************************/
void do_story(char_data *ch, char *)
{
    do_help(ch,"story");
}
/**************************************************************************/
void do_autoreformat(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

    if (IS_SET(ch->act,PLR_AUTOREFORMAT))
	{
        ch->println( "`cYour notes will no longer be automatically reformatted when posting.`x" );
        REMOVE_BIT(ch->act,PLR_AUTOREFORMAT);
	}
	else
	{
        ch->println( "`cYour notes will now be automatically reformatted when posting.`x" );
        SET_BIT(ch->act,PLR_AUTOREFORMAT);
	}
}
/**************************************************************************/
void do_autowraptells(char_data *ch, char *)
{
    if (HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS))
    {
		ch->println( "`cTells you send and receive will no longer be`x "
			"`cautomatically wordwraped.`x" );
		REMOVE_CONFIG(ch, CONFIG_AUTOWRAPTELLS);
    }
    else
    {
		ch->println( "`cTells you send and receive will now be`x "
			"`cautomatically wordwraped.`x" );
		SET_CONFIG(ch, CONFIG_AUTOWRAPTELLS);
    }
}

/**************************************************************************/
void do_autoself(char_data *ch, char *argument)
{
	char_data *target=TRUE_CH(ch);

	char arg[MIL];
	one_argument(argument, arg);

	if(IS_NULLSTR(arg)){
		if (IS_SET(target->comm,COMM_AUTOSELF))
		{
			ch->println( "`cYou can refer to yourself with your name, description and the keyword 'self'.`x" );
			REMOVE_BIT(target->comm,COMM_AUTOSELF);
		}
		else
		{
			ch->println( "`cYou can only refer to yourself with the word 'self'.`x" );
			SET_BIT(target->comm,COMM_AUTOSELF);
		}
	}else{
		if(!str_cmp(arg,"on")){
			SET_BIT(target->comm,COMM_AUTOSELF);
			ch->println( "`cAutoself is now ON.`x" );
		}
		if(!str_cmp(arg,"off")){
			REMOVE_BIT(target->comm,COMM_AUTOSELF);
			ch->println( "`cAutoself is now OFF.`x" );
		}
	}
}

/**************************************************************************/
void do_autowizilogin(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN)){
		ch->println( "`cYou wizi status will be unmodified during login.`x" );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN);
    }else{
		ch->println( "`cYou will automatically be made wizi on login.`x");
		SET_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN);
    }
}
/**************************************************************************/
void do_autokeepalive(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE)){
		ch->println( "`cAutokeepalive turned off.`x" );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE);
    }else{
		ch->println( "`cAutokeepalive turned on.`x");
		SET_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE);
    }

	ch->titlebar("ABOUT AUTOKEEPALIVE");
	ch->wrapln("`cAutokeepalive is a system which can in some cases`x "
		"`cwork around some ADSL/Cable routers which 'forget' about`x "
		"`ca connection after a certain time of inactivity - say`x "
		"`c30 minutes.  With this option turned on, the mud will send`x "
		"`cto your mud client a 'blank message' every minute if you`x "
		"`chave been inactive for more than 5 minutes... this should keep`x "
		"`cthe connection active in the view of the ADSL/Cable router.`x "
		"`cSome mud clients will incorrectly display 2 characters (one`x "
		"`clooks like a 'y' with a horizontal colon on top, and the other an 'n'`x "
		"`cwith a tilde (`-) on top.  Regardless of if you see this or`x "
		"`cnot, the system will in some cases keep your connection alive.`x  "
		"`cThe system is entirely optional, and will NOT prevent your`x "
		"`ccharacter being automatically logged out for inactivity if the`x "
		"`cmud is configured to do so.`x");
	ch->titlebar("");
}

/**************************************************************************/
void do_autowhoinvislogin(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN))
    {
		ch->println( "`cYou whoinvis status will be unmodified during login.`x" );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN);
    }else{
		ch->println( "`cYou will automatically be made whoinvis on login.`x");
		SET_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN);
    }
}

/**************************************************************************/
void do_autolist(char_data *ch, char *)
{
	char *alon="`CON`x";
	char *aloff="`cOFF`x";
	
	if (IS_NPC(ch)){
		ch->println("`cautolist: players only.`x");
		return;
	}
	
	ch->println( "   `caction     status`x"  );
	ch->println( "`=j---------------------`x" );
	
	ch->printlnf( "autoanswer     %s", !HAS_CONFIG(ch, CONFIG_NOAUTOANSWER)?alon:aloff);	
	ch->printlnf( "autoassist     %s", IS_SET(ch->act,PLR_AUTOASSIST)?alon:aloff);	
	if(!GAMESETTING2(GAMESET2_NO_AUTODAMAGE_COMMAND)){
		ch->printlnf( "autodamage     %s", HAS_CONFIG2(ch, CONFIG2_AUTODAMAGE)?alon:aloff);
	}
	ch->printlnf( "`cautoexamine`x    %s", HAS_CONFIG(ch, CONFIG_AUTOEXAMINE)?alon:aloff);	
	ch->printlnf( "`cautoexit`x       %s", IS_SET(ch->act,PLR_AUTOEXIT)?alon:aloff);	
	ch->printlnf( "`cautogold`x       %s", IS_SET(ch->act,PLR_AUTOGOLD)?alon:aloff);	
	ch->printlnf( "`cautokeepalive`x  %s", HAS_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE)?alon:aloff);	
	ch->printlnf( "`cautolandonrest`x %s", HAS_CONFIG(ch, CONFIG_AUTOLANDONREST)?alon:aloff);	
	ch->printlnf( "`cautoloot`x       %s", IS_SET(ch->act,PLR_AUTOLOOT)?alon:aloff);
	ch->printlnf( "`cautomap`x        %s", HAS_CONFIG(ch, CONFIG_AUTOMAP )?alon:aloff);	
	ch->printlnf( "`cautopeek`x       %s", IS_SET(ch->act,PLR_AUTOPEEK)?alon:aloff);	
	ch->printlnf( "`cautopkassist`x   %s", IS_SET(ch->dyn,DYN_AUTOPKASSIST)?alon:aloff);	
	ch->printlnf( "`cautorecall`x     %s", HAS_CONFIG(ch,CONFIG_AUTORECALL)?alon:aloff);	
	ch->printlnf( "`cautoreformat`x   %s", IS_SET(ch->act,PLR_AUTOREFORMAT)?alon:aloff);	
	ch->printlnf( "`cautosaycolourc`x %s", HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)?alon:aloff);	
	ch->printlnf( "`cautosaymote`x    %s", HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)?aloff:alon);	
	ch->printlnf( "`cautosplit`x      %s", IS_SET(ch->act,PLR_AUTOSPLIT)?alon:aloff);	
	ch->printlnf( "`cautosubdue`x     %s", IS_SET(ch->act,PLR_AUTOSUBDUE)?alon:aloff);	
	ch->printlnf( "`cautotrack`x      %s", HAS_CONFIG(ch, CONFIG_AUTOTRACK)?alon:aloff);	
	ch->printlnf( "`cautowraptells`x  %s", HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS)?alon:aloff);	
	ch->printlnf( "`cbattlelag`x      %s", !HAS_CONFIG2(ch,CONFIG2_NO_BATTLELAG_PROMPT)?alon:aloff);	
	ch->printlnf( "`ccombine items`x  %s", IS_SET(ch->comm,COMM_COMBINE)?alon:aloff);	
	ch->printlnf( "`ccompact mode`x   %s", IS_SET(ch->comm,COMM_COMPACT)?alon:aloff);	
	if ( ch->level > 89 ){
		ch->printlnf( "`chero messages`x  %s", HAS_CONFIG(ch,CONFIG_NOHEROMSG)?alon:aloff);
	}
	ch->printlnf( "`cnonames`x        %s", HAS_CONFIG(ch, CONFIG_NONAMES)?alon:aloff);	
	ch->printlnf( "`cnoteach`x        %s", IS_SET(ch->act,PLR_NOTEACH)?alon:aloff);	
	ch->printlnf( "`cprompt`x         %s", IS_SET(ch->comm,COMM_PROMPT)?alon:aloff);	
	ch->printlnf( "`cspecify self`x   %s  (to toggle type autoself)", HAS_AUTOSELF(ch)?alon:aloff);	

	if(IS_IMMORTAL(ch)){
		ch->printlnf( "`cautowizilogin`x     %s", HAS_CONFIG2(ch,CONFIG2_AUTOWIZILOGIN)?alon:aloff);
		ch->printlnf( "`cautowhoinvislogin`x %s", HAS_CONFIG2(ch,CONFIG2_AUTOWHOINVISLOGIN)?alon:aloff);
	}

	if (IS_UNSWITCHED_MOB(ch) && ch->desc){
		if(ch->desc->colour_mode==CT_NOCOLOUR){
			ch->println( "`cYou are playing in monochrome.`x");	
		}else{
			ch->println( "`cYou are playing in `?colour`c.`x");	
		}
	}

	ch->printlnf( "`cYou can%s be summoned.`x", IS_SET(ch->act,PLR_NOSUMMON)?"not":"");
	ch->printlnf( "`cYou can%s be charmed.`x",  HAS_CONFIG(ch,CONFIG_NOCHARM)?"not":"");
	ch->printlnf( "`cYou %saccept followers.`x", IS_SET(ch->act,PLR_NOFOLLOW)?"do not ":"");
/*	
	if(HAS_CONFIG(ch, CONFIG_SHOWMISC)){
		ch->println( "`cMisc notes are shown in unread.`x" );
	}else{
		ch->println( "`cDetails of unread misc notes are not shown in unread.`x" );
	}

	ch->printlnf( "`cYou have misc note reading %sbled.`x", HAS_CONFIG(ch,CONFIG_NOMISC)?"disa":"ena");
	ch->printlnf( "`cYou are speaking %s.`x", ch->language->name );
*/	
    if (ch->seeks)
    {
        ch->printlnf( "`cYou are currently seeking the clan `C%s`c.`x",
            ch->seeks->name());
    }
}

/**************************************************************************/
void do_name_b4short( char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}

    if ( HAS_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT))
    {
        REMOVE_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT);
		ch->println("`cKnown names are now displayed after short descriptions.`x");
    }
    else
    {
        SET_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT);
		ch->println("`cKnown names are now displayed before short descriptions.`x");
    }
    return;
}
/**************************************************************************/
void do_name_only_4known( char_data *ch, char *)
{	
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if ( HAS_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN)){
		ch->println("`cIf you know someones name, you will see both their name and short description.`x");
        REMOVE_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN);
    }else{
		ch->println("`cIf you know someones name, you wont see their short description.`x");
        SET_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN);
    }
    return;
}

/**************************************************************************/
void do_autoassist(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOASSIST))
	{
		ch->println( "`cAutoassist removed.`x" );
		REMOVE_BIT(ch->act,PLR_AUTOASSIST);
	}
	else
	{
		ch->println( "`cYou will now assist when needed.`x" );
		SET_BIT(ch->act,PLR_AUTOASSIST);
	}
}

/**************************************************************************/
void do_autopkassist(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->dyn,DYN_AUTOPKASSIST))
	{
		ch->println( "`cAutopkassist removed.`x" );
		REMOVE_BIT(ch->dyn,DYN_AUTOPKASSIST);
	}
	else
	{
		ch->println( "`cYou will now assist in pk fights when needed.`x" );
		SET_BIT(ch->dyn,DYN_AUTOPKASSIST);
	}
}


/**************************************************************************/
void do_autoexit(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOEXIT))
	{
		ch->println( "`cExits will no longer be displayed.`x" );
		REMOVE_BIT(ch->act,PLR_AUTOEXIT);
	}
	else
	{
		ch->println( "`cExits will now be displayed.`x" );
		ch->println( "`cnote: use `Rfullexits`c to have the name of each exit displayed.`x" );
		SET_BIT(ch->act,PLR_AUTOEXIT);
	}
}
/**************************************************************************/
void do_fullexits(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}

    if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
		ch->println( "`cFull exit info will no longer be shown automatically.`x");
		REMOVE_CONFIG2(ch,CONFIG2_FULL_EXITS);
	}else{
		ch->println( "`cFull exit info will now automatically be shown.`x");
		SET_CONFIG2(ch,CONFIG2_FULL_EXITS);
    }
}
/**************************************************************************/
void do_detect_oldstyle_note_writing(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if(HAS_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING)){
		ch->println( "`cThe note system will now automatically detect the use of {}`x "
			"`cand convert it to a newline code.`x");
		REMOVE_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING);
	}else{
		ch->println( "`cThe note system will no longer automatically detect the use`x "
			"`cof {} and convert it to a newline code.`x");
		SET_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING);
    }
}
/**************************************************************************/
void do_autoexamine(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_AUTOEXAMINE))
    {
		ch->println( "`cAutoexamining removed.`x" );
		REMOVE_CONFIG(ch, CONFIG_AUTOEXAMINE);
    }
	else
	{
		ch->println( "`cAutomatic corpse examining set.`x" );
		SET_CONFIG(ch, CONFIG_AUTOEXAMINE);
    }
}
/**************************************************************************/
void do_autolandonrest(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_AUTOLANDONREST))
    {
		ch->println( "`cAutomatic landing when you rest disabled.`x" );
		REMOVE_CONFIG(ch, CONFIG_AUTOLANDONREST);
    }
	else
	{
		ch->println( "`cYou will automatically land when you rest if flying.`x" );
		SET_CONFIG(ch, CONFIG_AUTOLANDONREST);
    }
}
/**************************************************************************/
// Kal
void do_nonames(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_NONAMES)){
		ch->println( "`cYou will now see the names players have introduced themselves to you.`x" );
		REMOVE_CONFIG(ch, CONFIG_NONAMES);
    }else{
		ch->println( "`cYou will no longer see playernames beside their short descriptions.`x" );
		SET_CONFIG(ch, CONFIG_NONAMES);
    }
}
/**************************************************************************/
// Kal
void do_autoanswer(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_NOAUTOANSWER)){
		ch->println( "`cYou will now automatically introduce yourself to someone who asks your name.`x" );
		REMOVE_CONFIG(ch, CONFIG_NOAUTOANSWER);
    }else{
		ch->println( "`cYou will no longer automatically introduce yourself to someone who asks your name.`x" );
		SET_CONFIG(ch, CONFIG_NOAUTOANSWER);
    }
}
/**************************************************************************/
void do_autogold(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOGOLD))
	{
		ch->println( "`cAutogold removed.`x" );
		REMOVE_BIT(ch->act,PLR_AUTOGOLD);
	}
	else
	{
		ch->println( "`cAutomatic gold looting set.`x" );
		SET_BIT(ch->act,PLR_AUTOGOLD);
	}
}

/**************************************************************************/
void do_autoloot(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOLOOT))
	{
		ch->println( "`cAutolooting removed.`x" );
		REMOVE_BIT(ch->act,PLR_AUTOLOOT);
	}
	else
	{
		ch->println("`cAutomatic corpse looting set.`x" );
		SET_BIT(ch->act,PLR_AUTOLOOT);
	}
}

/**************************************************************************/
void do_autosubdue(char_data *ch, char *)
{
	// unswitched mobs can't ooc
	if (IS_UNSWITCHED_MOB(ch))
	{
		ch->println( "`cPlayers or switched players only.`x" );
		return;
	}
	// link dead players can't ooc (ie can't force ooc from a linkdead)
	if (!TRUE_CH(ch))
	{
		ch->println( "`cLinkdead players can't use this command.`x" );
        return;
	}
	
    if (IS_SET(TRUE_CH(ch)->act,PLR_AUTOSUBDUE))
    {
        ch->println( "`cYou will fight other players to the death.`x" );
        REMOVE_BIT(TRUE_CH(ch)->act,PLR_AUTOSUBDUE);
    }
    else
    {
        ch->println( "`cWhen you fight other players, you will fight to subdue them.`x" );
        SET_BIT(TRUE_CH(ch)->act,PLR_AUTOSUBDUE);
    }
}
/**************************************************************************/
void do_autopeek(char_data *ch, char *)
{
    if (IS_NPC(ch))
	{
        ch->println( "Players only." );
        return;
	}
 
    if (IS_SET(ch->act,PLR_AUTOPEEK))
    {
        ch->println( "`cYou will NOT automatically peek at people when you look at them.`x" );
        REMOVE_BIT(ch->act,PLR_AUTOPEEK);
    }
    else
    {
        ch->println( "`cYou will automatically peek at people when you look at them.`x" );
        SET_BIT(ch->act,PLR_AUTOPEEK);
    }
}

/**************************************************************************/
void do_autorecall(char_data *ch, char *)
{
    if (IS_NPC(ch))
		return;
	
    if (HAS_CONFIG(ch,CONFIG_AUTORECALL))
    {
		ch->println( "`cYou will no longer automatically recall if`x "
			"`clinkdead when you are attacked.`x" );
		REMOVE_CONFIG(ch,CONFIG_AUTORECALL);
	}
    else
    {
		ch->println( "`cYou will automatically recall if linkdead when`x "
			"`cyou are attacked.`x" );
		SET_CONFIG(ch,CONFIG_AUTORECALL);
    }
}

/**************************************************************************/
void do_autosaymote(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}

	char colprefix[3];
	if(ch->colour_prefix==COLOURCODE){
		sprintf(colprefix, "%c%c",ch->colour_prefix,ch->colour_prefix);
	}else{
		sprintf(colprefix, "%c", ch->colour_prefix);
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)){
		ch->println( "`cSay's will automatically be treated as saymotes.`x");
		REMOVE_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE);
	}else{
		ch->wrapln( "`cSay's will no longer automatically be treated as saymotes,`x "
			"`cyou can use saymote manually to get a saymote.`x");
		SET_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE);
    }
	ch->wraplnf("`cnote: You can also use `=Cautosaycolourcodes`c to make your`x "
		"`csays automatically insert %s in front of colour codes (so you don't`x "
		"`ctalk with colour).`x", colprefix);
}
/**************************************************************************/
void do_autosaycolourcodes(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}
	char colprefix[3];
	if(ch->colour_prefix==COLOURCODE){
		sprintf(colprefix, "%c%c",ch->colour_prefix,ch->colour_prefix);
	}else{
		sprintf(colprefix, "%c", ch->colour_prefix);
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)){
		ch->println( "`cSay's will now allow colours in them.`x");
		REMOVE_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES);
	}else{
		ch->wraplnf( "`cSay's will automatically convert the colour code character (%s) into %s%s`x "
			"`cso you will be able to talk colour codes easily.`x",
			colprefix, colprefix, colprefix);			
		ch->println( "`cNote: you can bypass this behaviour for individual says by using saymote.`x");
		SET_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES);
    }
}
/**************************************************************************/
// Kal - June 01
void do_autodamage(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_AUTODAMAGE)){
		ch->println( "`cNumeric damage values will no longer be shown during combat.`x");
		REMOVE_CONFIG2(ch,CONFIG2_AUTODAMAGE);
	}else{
		ch->println( "`cNumeric damage values will now be shown during combat.`x");
		SET_CONFIG2(ch,CONFIG2_AUTODAMAGE);
    }
}
/**************************************************************************/
void do_autosplit(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOSPLIT))
	{
		ch->println( "`cAutosplitting removed.`x" );
		REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
	}
	else
	{
		ch->println( "`cAutomatic gold splitting set.`x" );
		SET_BIT(ch->act,PLR_AUTOSPLIT);
	}
}
/**************************************************************************/
void do_brief(char_data *ch, char *)
{
	if (IS_SET(ch->comm,COMM_BRIEF))
	{
		ch->println( "`cFull descriptions activated.`x" );
		REMOVE_BIT(ch->comm,COMM_BRIEF);
	}
	else
	{
		ch->println( "`cShort descriptions activated.`x" );
		SET_BIT(ch->comm,COMM_BRIEF);
	}
}

/**************************************************************************/
void do_compact(char_data *ch, char *)
{
	if (IS_SET(ch->comm,COMM_COMPACT))
	{
		ch->println( "`cCompact mode removed.`x" );
		REMOVE_BIT(ch->comm,COMM_COMPACT);
	}
	else
	{
		ch->println( "`cCompact mode set.`x" );
		SET_BIT(ch->comm,COMM_COMPACT);
	}
}

/**************************************************************************/
void do_showaffects(char_data *ch, char *)
{
	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
	{
		ch->println( "`cAffects will no longer be shown in score.`x" );
		REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
	else
	{
		ch->println( "`cAffects will now be shown in score.`x" );
		SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
}
/**************************************************************************/
void do_battlelag(char_data *ch, char *argument)
{
	if(!TRUE_CH_PCDATA(ch)){
		ch->println("`cOnly players or controlled mobs can use the battlelag command.`x");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->wrapln("`cBattlelag text (when set), is displayed in front of`x "
			"`cyour prompt when you are currently affected by combat lag.`x");
		ch->println("`csyntax: battlelag <lagtext/symbols>    (to set/change the current battle lag text)`x");
		ch->println("`csyntax: battlelag off   (to turn off battle lag)`x");
		ch->println("`csyntax: battlelag on    (to enable the displaying of battlelag text)`x");
		ch->println("`csyntax: battlelag clear (to clear your battle lag text and use mudwide default)`x");

		if(IS_NULLSTR(TRUE_CH_PCDATA(ch)->battlelag)){
			ch->printlnf("`cYou are currently using the mudwide default battle lag text of '%s'.`x",
				game_settings->mud_default_battlelag_text);
		}else{
			ch->printlnf("`cYou are currently have your battle lag text set to '%s'.`x",
				TRUE_CH_PCDATA(ch)->battlelag);
		}
		return;
	}

	if(!str_cmp(argument, "on")){
		ch->println("`cBattle lag prompt enabled.`x");
		REMOVE_CONFIG2(ch, CONFIG2_NO_BATTLELAG_PROMPT);
		return;
	}

	if(!str_cmp(argument, "off")){
		ch->println("`cBattle lag prompt disabled.`x");
		SET_CONFIG2(ch, CONFIG2_NO_BATTLELAG_PROMPT);
		return;
	}

	if(!str_cmp(argument, "clear")){
		argument="";
	}

	ch->printlnf("`cBattle lag prompt changed from '%s' to '%s'.`x",
		TRUE_CH_PCDATA(ch)->battlelag,
		argument);
	replace_string(TRUE_CH_PCDATA(ch)->battlelag, argument);
}
/**************************************************************************/
void do_prompt(char_data *ch, char *argument)
{
	char buf[MSL];

	if (IS_NPC(ch))
	{
		ch->println( "`cPlayers only.`x" );
		return;	
	}

	if ( IS_NULLSTR(argument))
	{
		if (IS_SET(ch->comm,COMM_PROMPT))
		{
			if (ch->desc->editor) // working in olc
			{
				if (IS_NULLSTR(ch->olcprompt))
				{
					sprintf(buf,"`cYour OLCprompt was not defined...`x "
						"`cusing your normal prompt.`x\r\n%s\r\n",ch->prompt );
				}
				else
				{
					sprintf(buf,"`cYour OLCprompt was '%s'`x\r\n",ch->olcprompt );
				}

			}
			else
			{
				if(IS_NULLSTR(ch->prompt)){
					sprintf(buf,"`cYour prompt was the system wide default (%s`x)\r\n", 
						game_settings->default_prompt);
				}else{
					sprintf(buf,"`cYour prompt was '%s'`x\r\n", ch->prompt);
				}
			}
			ch->printbw(buf);


      	    ch->println( "`cYou will no longer see prompts.`x" );
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
			ch->println( "`cYou will now see prompts.`x" );
			if (ch->desc->editor) // working in olc
			{
				if (IS_NULLSTR(ch->olcprompt))
				{
					sprintf(buf,"`cYour current OLCprompt is not defined...`x "
						"`cusing your normal prompt`x\r\n%s\r\n",ch->prompt );
				}
				else
				{
					sprintf(buf,"`cYour current OLCprompt is %s`x\r\n",ch->olcprompt );
				}

			}
			else
			{
				sprintf(buf,"`cYour current prompt is %s`x\r\n",ch->prompt );
			}
			ch->printbw(buf);

			SET_BIT(ch->comm,COMM_PROMPT);
    	}
		ch->println("`cNote: you can change your group prompt using `=Cgprompt`x");
		return;
	}

    if( !strcmp( argument, "basic" ) )
        strcpy( buf, "`c| %h/%Hhp | ");
	else if( !strcmp( argument, "all" ) )
		strcpy( buf, "`c[%h/%Hhp %m/%Mm %vmv %Xxp> ");
    else if( !strcmp( argument, "xp" ) )
        strcpy( buf, "`c| %h/%Hhp %x/%Xxp %P | ");
    else if( !strcmp( argument, "shop" ) )
        strcpy( buf, "`c| %h/%Hhp %ggold %ssilver| ");
    else if( !strcmp( argument, "explore" ) )
        strcpy( buf, "`c| %h/%Hhp %e %z| ");
    else if( !strcmp( argument, "rp" ) )
        strcpy( buf, "`c| %b in %z at %t | ");
    else if( !strcmp( argument, "default" ) )
        buf[0]='\0';
    else if( !strcmp( argument, "immortal" ) )
        strcpy( buf, "`c| %hhp room:%R zone:%Z ictime: %t | %d");
    else if( !strcmp( argument, "olc" ) ){
		if (ch->desc->editor){  // in olc editor
			strcpy( buf, "[`#`m%e`^ in `R%o`mv`R%O`g%Z`^ - %T`^ - %t%d>");
		}else{ // not in olc
			strcpy( buf, "[`#`m%e`^ in `mv`R%R `g%Z`^ - %T`^ - %t%d>");
		}

	}else if( !strcmp( argument, "olc2" ) ){
		if (ch->desc->editor){  // in olc editor
			strcpy( buf, "[`#`m%e`^ in `R%o`mv`R%O`g%Z`^ - %T`^ - %t - %d>");
		}else{ // not in olc
			strcpy( buf, "[`#`m%e`^ in `mv`R%R `g%Z`^ - %T`^ - %t - %d>");
		}

	}
	else if( !strcmp( argument, "build1" ) )
        strcpy( buf, "[--= %hhp %mm %vmv %R %z =-->");
    else if( !strcmp( argument, "build2" ) )
        strcpy( buf, "[--= %hhp %mm %vmv %R %z(%Z) %e =-->");
	else
	{
		if ( str_len(argument) > 160 ){
			argument[160] = '\0';
		}
		strcpy( buf, argument );
		smash_tilde( buf );
		if (str_suffix("%c",buf)){
			strcat(buf," ");
		}
	}

	if (ch->desc->editor) // working in olc - set the olc prompt
	{		
		replace_string(ch->olcprompt, buf);
		sprintf(buf,"`cOLCprompt set to '%s'`x\r\n",ch->olcprompt );
	}else{		
		replace_string(ch->prompt, buf);
		if(IS_NULLSTR(ch->prompt)){
			sprintf(buf,"`cPrompt cleared, now using the system default prompt of '%s'`x\r\n",
				game_settings->default_prompt);
		}else{
			sprintf(buf,"`cPrompt set to '%s'`x\r\n",ch->prompt );
		}
	}
	ch->printbw(buf);
	return;
}
/**************************************************************************/
// GroupPrompt - Kal, Dec 2001
void do_gprompt(char_data *ch, char *argument)
{
	char buf[MSL];

	if (IS_NPC(ch))
	{
		ch->println("`cPlayers only.`x");
		return;	
	}

	if ( IS_NULLSTR(argument) )
	{
		if(!IS_SET(ch->comm,COMM_NOGPROMPT))
		{
			sprintf(buf,"`cYour group prompt was '%s'`x\r\n",ch->gprompt );
			ch->printbw(buf);
			
			ch->println( "`cYou will no longer see group prompts.`x" );
			SET_BIT(ch->comm,COMM_NOGPROMPT);
		}
		else
		{
			ch->println( "`cYou will now see group prompts.`x" );
			sprintf(buf,"`cYour current group prompt is '%s'`x\r\n",ch->gprompt );
			ch->printbw(buf);
			
			REMOVE_BIT(ch->comm,COMM_NOGPROMPT);
		}

		ch->printlnf("`cType `=C%s`c for help on the group prompt %% codes.`x",
			mxp_create_send(ch, "gprompt help"));
		return;
	}

	if( !str_cmp(argument,"help") )
	{
		ch->titlebar("`cGROUP PROMPT PERCENTAGE CODES`x");
		ch->println(
			"`c  %g - begin group section`x"
			"`c  %G - end group section`x"
			"`c  %h - lowest hitpoints % for group members in the room`x"
			"`c  %m - lowest mana % for group members in the room`x"
			"`c  %v - lowest move % for group members in the room`x"
			"`c  %p - begin pet section`x"
			"`c  %P - end pet section`x"
			"`c  %q - pet hitpoints %`x"
			"`c  %r - pet mana %`x"
			"`c  %s - pet move %`x" 
			"`c  %N - number of group members in the current room`x"
			"`c  %c - carriage return`x"
			"`c  %C - carriage return only if there is preceeding text`x"
			"`c  %x - number of charmies in the current room (excluding pet)`x"
			"`x");
		ch->println("`cAnything between %p and %P is 'eaten' when you dont have a pet in the room.`x");
		ch->println("`cAnything between %g and %G is 'eaten' when you dont have a group member in the room.`x");
		ch->println("`cGroup prompts only work on pets and other players - not charmies (except %x).`x");
		ch->println("`cThe default group prompt is:`x");
		ch->printlnbw("  '`#%g[`xgrp `R%hhp `B%mm `M%vmv`&]%G%p[`spet `r%qhp `b%rm `m%smv`&>%P%c'");
		ch->titlebar("");
		return;
	}

	
	{
		if ( str_len(argument) > 160 ){
			argument[160] = '\0';
		}
		strcpy( buf, argument );
		smash_tilde( buf );
		if (str_suffix("%c",buf)){
			strcat(buf," ");
		}
	}
	
	ch->printfbw("`cPrompt changed from '%s' to '%s'`x\r\n",ch->gprompt, buf );
	replace_string( ch->gprompt, buf );
	REMOVE_BIT(ch->comm,COMM_NOGPROMPT);
	return;
}
/**************************************************************************/
void do_combine(char_data *ch, char *)
{
	if (IS_SET(ch->comm,COMM_COMBINE))
	{
		ch->println( "`cLong inventory selected.`x" );
		REMOVE_BIT(ch->comm,COMM_COMBINE);
	}
	else
	{
		ch->println( "`cCombined inventory selected.`x" );
		SET_BIT(ch->comm,COMM_COMBINE);
	}
}

/**************************************************************************/
void do_nofollow(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	// Check to see if ch is charmed and being ordered to cast
	if ( IS_AFFECTED(ch,AFF_CHARM) && !IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		ch->println( "`cYou must wait for your master to tell you to do that.`x" );
		return;
	}

	if (IS_SET(ch->act,PLR_NOFOLLOW))
	{
		ch->println( "`cYou now accept followers.`x" );
		REMOVE_BIT(ch->act,PLR_NOFOLLOW);
	}
	else
	{
		ch->println( "`cYou no longer accept followers.`x" );
		SET_BIT(ch->act,PLR_NOFOLLOW);
		die_follower( ch );
	}
}

/**************************************************************************/
void do_noteach(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_NOTEACH))
    {
		ch->println( "`cYou now can be taught.`x" );
		REMOVE_BIT(ch->act,PLR_NOTEACH);
	}
	else
	{
		ch->println( "`cYou no longer can be taught.`x");
		SET_BIT(ch->act,PLR_NOTEACH);
	}
}

/**************************************************************************/
void do_nosummon(char_data *ch, char *)
{
	if (IS_NPC(ch))
	{
		if (IS_SET(ch->imm_flags,IMM_SUMMON))
		{
			ch->println( "`cYou are no longer immune to summon.`x" );
			REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
		}
		else
		{
			ch->println( "`cYou are now immune to summoning.`x" );
			SET_BIT(ch->imm_flags,IMM_SUMMON);
		}
	}
	else
	{
		if (IS_SET(ch->act,PLR_NOSUMMON))
		{
			ch->println( "`cYou are no longer immune to summon.`x" );
			REMOVE_BIT(ch->act,PLR_NOSUMMON);
		}
		else
		{
			ch->println( "`cYou are now immune to summoning.`x" );
			SET_BIT(ch->act,PLR_NOSUMMON);
		}
	}
}

/**************************************************************************/
void do_nocharm(char_data *ch, char *)
{
    if (IS_NPC(ch))
    {
		if (IS_SET(ch->imm_flags,IMM_CHARM))
		{
			ch->println( "`cYou are no longer immune to charm.`x" );
			REMOVE_BIT(ch->imm_flags,IMM_CHARM);
		}
		else
		{
			ch->println( "`cYou are now immune to charming.`x" );
			SET_BIT(ch->imm_flags,IMM_CHARM);
		}
    }
    else
    {
		if(GAMESETTING2(GAMESET2_NOCHARM_HAS_NOAFFECT)){
			ch->println("`cNOTE: This setting has no affect while the affects of nocharm \n\rhave been disabled in the gamesettings.`x");
		}
		if (HAS_CONFIG(ch,CONFIG_NOCHARM))
		{
			ch->println( "`cYou are no longer immune to charm.`x" );
			REMOVE_CONFIG(ch,CONFIG_NOCHARM);
		}
		else
		{
			ch->println( "`cYou are now immune to charming.`x" );
			SET_CONFIG(ch,CONFIG_NOCHARM);
		}
    }
}

/**************************************************************************/
void do_glance( char_data *ch, char *argument )
{
	char_data *victim;
	char buf[MSL];
	int percent;

	if(IS_NULLSTR(argument)){
		ch->println( "Glance at whom?" );
		return;
	}

	victim = get_char_room( ch, argument );

	if(!victim){
		ch->printlnf( "`cYou can't seem to see any '%s' in the room.`x", argument);
		return;
	}

	if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
	else
        percent = -1;
	
	strcpy( buf, "`c");
	strcat( buf, PERS(victim, ch) );
	
	if (percent >= 100)
        strcat( buf, "`c is in excellent condition.`x\r\n");
	else if (percent >= 90)
        strcat( buf, "`c has a few scratches.`x\r\n");
	else if (percent >= 75)
        strcat( buf,"`c has some small wounds and bruises.`x\r\n");
	else if (percent >=  50)
        strcat( buf, "`c has quite a few wounds.`x\r\n");
	else if (percent >= 30)
        strcat( buf, "`c has some big nasty wounds and scratches.`x\r\n");
    else if (percent >= 15)
        strcat ( buf, "`c looks pretty hurt.`x\r\n");
    else if (percent >= 0 )
        strcat (buf, "`c is in `rawful condition`c.`x\r\n");
    else
        strcat(buf, "`c is `rbleeding`c to death.`x\r\n");
	
    ch->printlnf( "`c%s`x", capitalize( buf ));

	// flying status
	if ( IS_AFFECTED(victim, AFF_FLYING) ){
		ch->println( "`cThey appear to be airborne.`x" );
	}
}
/**************************************************************************/
void send_zmudmap_to_char(char_data *ch);
void do_lockers( char_data *ch, char *argument );
/**************************************************************************/
// supports uid 
void do_look( char_data *ch, char *argument )
{
	char buf  [MSL];
	char arg1 [MIL];
	char arg2 [MIL];
	char arg3 [MIL];
	EXIT_DATA *pexit;
	char_data *victim;
	OBJ_DATA *obj;
	char *pdesc;
	int door;
	int number,count;
	
	if ( ch->desc == NULL )
		return;
	
	if ( ch->position < POS_SLEEPING )
	{
		ch->println( "`cYou can't see anything but stars!`x" );
		return;
	}
	
	if ( ch->position == POS_SLEEPING )
	{
		ch->println( "`cYou can't see anything here while in the dreamworld!`x" );
		return;
	}
	
	if ( IS_IC(ch) && !check_blind( ch ) )
		return;
	
	if ( !IS_NPC(ch)
		&& !HAS_HOLYLIGHT(ch)
		&& room_is_dark( ch->in_room ) )
	{
		ch->println( "`cIt is pitch black ... `x" );

		
		if(!IS_SET(ch->in_room->room_flags, ROOM_NOAUTOMAP))
			{
				if (USING_AUTOMAP(ch)) 
					{
						do_map(ch,"");
    					}
			}

		// attempt to display room contents, glowing objects should be seen
		show_list_to_char( ch->in_room->contents, ch, "", false, false );
		show_char_to_char( ch->in_room->people, ch );


        if ( (IS_SET(TRUE_CH(ch)->act, PLR_AUTOEXIT) || HAS_CONFIG2(ch,CONFIG2_FULL_EXITS))
			&& !IS_SET(ch->in_room->room_flags, ROOM_NOAUTOEXITS))
		{
			ch->println( "" );
			if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
				do_exits( ch, "fullexits" );
			}else{
				do_exits( ch, "auto" );
			}			
		}


		return;
	}
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	number = number_argument(arg1,arg3);
	count = 0;
	
	if ( IS_NULLSTR(arg1) || !str_cmp( arg1, "auto" ) || !str_cmp( arg1, "brief" ))
	{
		if (IS_SET(ch->dyn,DYN_MAPCLEAR) && USING_AUTOMAP(ch)){			        
			do_map(ch,"");
    	}
		// 'look' or 'look auto'
		ch->printf( "`=B%s`x", mxp_create_tag(ch, "RName", ch->in_room->name));


		if(!GAMESETTING2(GAMESET2_HIDE_AREA_SHORTNAMES) 
			&& !IS_NULLSTR(ch->in_room->area->short_name))
		{
			ch->printf(" `#[%s%s`^]", 
				colour_table[(ch->in_room->area->vnum%14)+1].code,
				ch->in_room->area->short_name);
		}
		
		if ( (IS_IMMORTAL(ch) && HAS_HOLYLIGHT(ch))
			|| (IS_SET(ch->comm, COMM_BUILDING) && ch->in_room 
			&& IS_BUILDER(ch, ch->in_room->area, BUILDRESTRICT_ROOMS)) )
		{ 
			ch->printf( " [Room %d]",ch->in_room->vnum );
		}
		
		if (IS_OOC(ch)){
			ch->print(" `=^(OOC ROOM)" );
		}
		
		if (IS_OLCAREA(ch->in_room->area)){
			ch->print( " `=&(OLC AREA)" );
		}

		ch->println( "`x" ); 
		if(!IS_SET(ch->in_room->room_flags, ROOM_NOAUTOMAP)){
			if (!IS_SET(ch->dyn,DYN_MAPCLEAR) && USING_AUTOMAP(ch)){
				do_map(ch,"");
    		}
		}
		
		if ( IS_NULLSTR(arg1) 
			|| !IS_SET(TRUE_CH(ch)->comm,COMM_BRIEF)){
			if ( ch->desc && !ch->desc->speedwalk_buf && str_cmp( arg1, "brief" )){   
				ch->printf( "`=b  %s",  mxp_create_tag(ch, "RDesc", ch->in_room->description));
			}

		}

        if( (IS_SET(TRUE_CH(ch)->act, PLR_AUTOEXIT) || HAS_CONFIG2(ch,CONFIG2_FULL_EXITS))
			&& !IS_SET(ch->in_room->room_flags, ROOM_NOAUTOEXITS))
		{
			ch->println( "" );
			if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
				do_exits( ch, "fullexits" );
			}else{
				do_exits( ch, "auto" );
			}			
		}
		
		
		show_list_to_char( ch->in_room->contents, ch, "", false, false );

		// if there are lockers in this room, display them
		if(lockers->room_has_lockers(ch->in_room))
		{		
			if ( !IS_OUTSIDE( ch )
			|| ch->in_room->sector_type == SECT_CITY
			|| ch->in_room->sector_type == SECT_TRAIL )
			{
				ch->printlnf("     `=\x8d%s", 
				mxp_create_send(ch,"look lockers",
					FORMATF("""`CThis place has %d mailbox%s.`x", 
					ch->in_room->lockers->quantity,
					ch->in_room->lockers->quantity==1?"":"es")));

			}else{

			ch->printlnf("     `=\x8d%s", 
				mxp_create_send(ch,"look lockers",
					FORMATF("""`CThis room has %d locker%s.`x", 
					ch->in_room->lockers->quantity,
					ch->in_room->lockers->quantity==1?"":"s")));
			}
		}
		ch->print( "`=D" );
		show_char_to_char( ch->in_room->people,   ch );

		ch->print( "`x" );


		// do track if they arent following anyone and have the skills and arent speedwalking
		if(HAS_CONFIG(ch, CONFIG_AUTOTRACK) && !ch->master ){
			if(get_skill(ch, gsn_fieldtrack)>1 
				|| get_skill(ch, gsn_citytrack)>1)
			{
				if(!(ch->desc && ch->desc->speedwalk_buf)){
					do_tracks(ch,"");
				}
			}
		}
        return;
	}
	
	if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
	{
		// 'look in'
		if ( IS_NULLSTR(arg2) )
		{
			ch->println( "`cLook in what?`x" );
			return;
		}

		// get_obj_here() supports uid
		if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			ch->println( "`cYou do not see that here.`x" );
			return;
		}
		
		switch ( obj->item_type )
		{
		default:
			ch->println( "`cThat is not a container.`x" );
			break;
			
		case ITEM_DRINK_CON:
			if ( obj->value[1] <= 0 )
			{
				ch->println( "`cIt is empty.`x" );
				break;
			}
			
			ch->printlnf( "`cIt's %sfilled with a %s liquid.`x",
				obj->value[1] <     obj->value[0] / 4 ?
				"less than half-" :	obj->value[1] < 3 * obj->value[0] / 4 ?
				"about half-"     : "more than half-",
				liq_table[obj->value[2]].liq_color );
			break;

		case ITEM_PORTAL:
			{
				bool autolook = false;
			    ROOM_INDEX_DATA *location;
				int lvnum;

				if (IS_SET(obj->value[2],GATE_OPAQUE)) {
					ch->println( "`cThe portal is opaque, you can't see anything inside it.`x" );
					return;
				};

				if( IS_SET( ch->act,  PLR_AUTOEXIT )) {
					autolook = true;
					REMOVE_BIT( ch->act, PLR_AUTOEXIT );
				}

				if(IS_SET(obj->value[2],GATE_SHORT_LOOKINTO)){
					act( "`cYou look through $p and see`x", ch, obj, NULL, TO_CHAR );
				}else{
					act( "`cYou gaze through the portal and see:`x", ch, NULL, NULL, TO_CHAR );
				}

				if (IS_SET(obj->value[2],GATE_RANDOM) || obj->value[3] == -1)
				{
					location = get_random_room(ch);
					lvnum = location->vnum; // for record keeping :)
				}else{
					lvnum = obj->value[3];
				
				}
				sprintf( buf, "%d look", lvnum);
				do_at( ch, buf );
				if( autolook )
					SET_BIT( ch->act, PLR_AUTOEXIT );
			}
			break;
			
		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:
                case ITEM_PAN:
                case ITEM_POT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if ( IS_SET(obj->value[1], CONT_CLOSED) )
			{
				ch->println( "`cIt is closed.`x" );
				break;
			}
			
			act( "`c$p holds:`x", ch, obj, NULL, TO_CHAR );
			show_list_to_char( obj->contains, ch, "", true, true );
			break;
		}
		return;
    }

	// get_char_room() supports uid
    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
		show_char_to_char_1( victim, ch );
		return;
    }

	// support UID
	int uid=get_uid(arg3);
	if(uid){
		// uid looking at objects in inventory 
		for ( obj = ch->carrying; obj; obj = obj->next_content )
		{		
			if(uid==obj->uid && can_see_obj( ch, obj ) ){
				// parchments are displayed differently 
				if ( obj->item_type == ITEM_PARCHMENT ){
					letter_read( ch, obj );
					return;
				}

				// take the first name field, and if it is longer than
				// 2 characters, see if we can find an extended description 
				// that matches it... if so show that instead of the objects 
				// description		
				{
					char firstname[MIL];
					one_argument(obj->name, firstname);

					if(str_len(firstname)>2){
						// use only one of the sets of extended descriptions 
						if (obj->extra_descr)
						{ // unique objects extended descriptions 
							pdesc = get_extra_descr( firstname, obj->extra_descr );
							if( pdesc ){
								ch->sendpage(pdesc);
								return;
							}				
						}
						else // vnums extended descriptions
						{        
							pdesc = get_extra_descr( firstname, obj->pIndexData->extra_descr );
							if ( pdesc )
							{
								ch->sendpage(pdesc);
								return;
							}
						}
					}
				}

				ch->println( obj->description );
				return;
			}
		}
		// uid looking at objects in room
		for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
		{
			if(uid==obj->uid && can_see_obj( ch, obj ) ){
				// parchments are displayed differently 
				if ( obj->item_type == ITEM_PARCHMENT ){
					letter_read( ch, obj );
					return;
				}

				// take the first name field, and if it is longer than
				// 2 characters, see if we can find an extended description 
				// that matches it... if so show that instead of the objects 
				// description		
				{
					char firstname[MIL];
					one_argument(obj->name, firstname);

					if(str_len(firstname)>2){
						// use only one of the sets of extended descriptions 
						if (obj->extra_descr)
						{ // unique objects extended descriptions 
							pdesc = get_extra_descr( firstname, obj->extra_descr );
							if( pdesc ){
								ch->sendpage(pdesc);
								return;
							}				
						}
						else // vnums extended descriptions
						{        
							pdesc = get_extra_descr( firstname, obj->pIndexData->extra_descr );
							if ( pdesc )
							{
								ch->sendpage(pdesc);
								return;
							}
						}
					}
				}

				ch->println( obj->description );
				return;
			}
		}
		ch->println( "`cYou do not see that here.`x" );
		return;
	}

	// support lockers
    if( !str_prefix(arg3, "lockers") ){
		do_lockers(ch, "look");
        return;
    }
    if( !str_prefix(arg3, "mailbox") ){
		do_lockers(ch, "look");
        return;
    }
    if( !str_prefix(arg3, "mailboxes") ){
		do_lockers(ch, "look");
        return;
    }

	for ( obj = ch->carrying; obj; obj = obj->next_content )
    {		
        if ( can_see_obj( ch, obj ) )
        {  // player can see object
			
			// parchments take highest priority
            if ( obj->item_type == ITEM_PARCHMENT && is_name( arg3, obj->name ) )
            {
                if (++count == number)
                {
					letter_read( ch, obj );
                    return;
                }
            }
            // use only one of the sets of extended descriptions 
            if (obj->extra_descr)
            { // unique objects extended descriptions 
                pdesc = get_extra_descr( arg3, obj->extra_descr );
                if( pdesc ){
                    if (++count == number){
                        ch->sendpage(pdesc);
                        return;
                    }
					continue;
                }				
            }
            else // vnums extended descriptions
            {        
                pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
                if ( pdesc )
                {
                    if (++count == number){ 
                        ch->sendpage(pdesc);
                        return;
                    }
					continue;
                }
            }
			
            if ( is_name( arg3, obj->name ) )
            {
                if (++count == number){
                    ch->println( obj->description );
                    return;
                }
            }
        }
		
    }
	
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            // use only one of the sets of extended descriptions 
            if (obj->extra_descr)
            { // unique objects extended descriptions 				
                pdesc = get_extra_descr( arg3, obj->extra_descr );
                if ( pdesc ){
                    if (++count == number) {
						ch->sendpage(pdesc);
						return;
                    }
					continue;
				}
            }
            else // vnums extended descriptions
            {      
                pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
                if ( pdesc != NULL )
                {
                    if (++count == number)
                    {
                        ch->sendpage(pdesc);
                        return;
                    }
					continue;
                }
            }
        }
		
        if ( is_name( arg3, obj->name ) )
        {
            if (++count == number){
                ch->println( obj->description );
                return;
            }
        }
    }
	
	// match room extended descriptions
    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc )
    {
        if (++count == number){
            ch->sendpage(pdesc);
            return;
        }
    }
    
	// tell them how many they matched on
    if (count > 0 && count != number)
    {
		if (count == 1){
			ch->printlnf( "`cYou only see one %s here.`x", arg3 );
		}else{
			ch->printlnf( "`cYou only see %d of those here.`x", count );
		}
		return;
    }
	

	// try looking at a direction
	door = dir_lookup( arg1 );
	if ( door == -1 ){
		ch->println( "`cYou do not see that here.`x" );
        return;
    }
	
    // 'look direction'
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
        ch->println( "`cNothing special there.`x" );
        return;
    }
	
    if ( !IS_NULLSTR(pexit->description)){
        ch->print( pexit->description );
    }else{
        ch->println( "`cNothing special there.`x" );
	}
	
    if ( pexit->keyword    != NULL
        && pexit->keyword[0] != '\0'
        && pexit->keyword[0] != ' ' )
    {
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            act( "`cThe $d is closed.`x", ch, NULL, pexit->keyword, TO_CHAR );
        }
        else
        {
            if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
            {
                act( "`cThe $d is open.`x",   ch, NULL, pexit->keyword, TO_CHAR );
            }
        }
    }
    return;
}

/**************************************************************************/
void do_read (char_data *ch, char *argument )
{
    do_look(ch,argument);
}

/**************************************************************************/
// supports uid
void do_examine( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg[MIL];
	OBJ_DATA *obj;
    
	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "`cExamine what?`x" );
		return;
	}

	do_look( ch, arg );

	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		switch ( obj->item_type )
		{
		default:
			break;
		
		case ITEM_JUKEBOX:
			//        do_play(ch,"list");
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0)
			{
				if (obj->value[1] == 0)
					ch->println( "`cOdd...there are no coins in the pile.`x" );
				else if (obj->value[1] == 1)
					ch->println( "`cWow. One gold coin.`x" );
				else
					ch->printlnf( "`cThere are %d gold coins in the pile.`x", obj->value[1]);
			}
			else if (obj->value[1] == 0)
			{
				if (obj->value[0] == 1)
					ch->println( "`cWow. One silver coin.`x" );
				else
					ch->printlnf( "`cThere are %d silver coins in the pile.`x", obj->value[0]);
			}
			else
			{
				ch->printlnf( "`cThere are %d gold and %d silver coins in the pile.`x",
					obj->value[1],obj->value[0]);
			}
			break;

		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:		
		case ITEM_DRINK_CON:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			sprintf(buf,"`cin %s`x",argument);
			do_look( ch, buf );
		}
	}
    return;
}

/**************************************************************************/
void do_exits( char_data *ch, char *argument )
{
	EXIT_DATA *pexit;
    char buf[MSL];
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto){
        sprintf(buf,"`c[Exits:");
    }else{
        if (IS_IMMORTAL(ch)){
            sprintf(buf,"`CObvious exits from room %d:`x\r\n",ch->in_room->vnum);
        }else{
            sprintf(buf,"`CObvious exits:`x\r\n");
		}
	}

	if(!str_cmp( argument, "fullexits" )){
		strcat(buf, "`c");
	}

    found = false;
    for ( door = 0; door<MAX_DIR; door++ )
    {
        if ( ( pexit = ch->in_room->exit[door] ) != NULL
            &&   pexit->u1.to_room != NULL
            &&   can_see_room(ch,pexit->u1.to_room) 
            &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            found = true;
            if ( fAuto ){
                strcat( buf, mxp_create_tagf(ch, "Ex", " %s",dir_name[door]));
            }
            else
            {
                sprintf( buf + str_len(buf), "%-5s - %s",
                    capitalize( dir_name[door] ),
                    (!HAS_HOLYLIGHT(ch)
                     && room_is_dark( pexit->u1.to_room) )
                    ?  "`cToo dark to tell`x"
                    : pexit->u1.to_room->name
                    );
            if (IS_IMMORTAL(ch))
                sprintf(buf + str_len(buf), 
                " (room %d)\r\n",pexit->u1.to_room->vnum);
            else
                sprintf(buf + str_len(buf), "\r\n");
            }
        }
    }

    // hidden exits for those with holylight 
    if (HAS_HOLYLIGHT(ch))
    {
        for ( door = 0; door < MAX_DIR; door++ )
        {
    
            if ( ( pexit = ch->in_room->exit[door] ) != NULL
                &&   pexit->u1.to_room != NULL
                &&   can_see_room(ch,pexit->u1.to_room) 
                &&   IS_SET(pexit->exit_info, EX_CLOSED) )
            {
                found = true;
                if ( fAuto )
                {
                    strcat( buf, " (" );
                    strcat( buf, dir_name[door] );
                    strcat( buf, ")" );
                }
                else
                {
                    sprintf( buf + str_len(buf), "(%s) - %s",
                        capitalize( dir_name[door] ),
                        pexit->u1.to_room->name);
                    if (IS_IMMORTAL(ch))
                        sprintf(buf + str_len(buf), 
                            " (room %d)\r\n",pexit->u1.to_room->vnum);
                    else
                        sprintf(buf + str_len(buf), "\r\n");
                }
            }
        }
    }

    if ( !found )
        strcat( buf, fAuto ? " none" : "`cNone.`x" );

    if ( fAuto ){
        strcat( buf, "`c]`x");
		strcpy( buf, mxp_create_tag(ch, "Exits", buf ));
	}
    ch->printlnf( "`=a`c%s`x", buf);
	                
    return;
}

/**************************************************************************/
void do_worth( char_data *ch, char * )
{
    char buf[MSL], message[MSL];

	ch->titlebar("WORTH");

	if (IS_NPC(ch))
	{
		ch->printlnf( "`cYou have %ld gold and %ld silver.`x",
			ch->gold,ch->silver);
		return;
	}
	if(ch->bank){
		ch->printlnf( "`c Bank Account:  %ld", ch->bank);
		}
	ch->printlnf(
		" `cYou have %ld gold and %ld silver, \r\n %ld Rps, and %d experience (%d exp to level).`x",
		ch->gold, ch->silver, ch->pcdata->rp_points, ch->exp,
		(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
	
	ch->printlnf( "`c Wimpy set to %d hitpoint%s.`x", ch->wimpy, ch->wimpy==1?"":"s");
	ch->printlnf( "`c Panic set to %d hitpoint%s.`x", ch->pcdata->panic, ch->pcdata->panic==1?"":"s");
//	ch->printlnf( "`c You're currently speaking in %s.`x", ch->language->name);

	if (HAS_CLASSFLAG( ch, CLASSFLAG_TOTEMS )){
		int month = get_birthmonth( ch );

		if ( month >= 0 )
		{
			ch->printlnf( " `cYour totemic spirit is the %s.`x",
			totem_table[month].name );
		}
	}

	if (HAS_CLASSFLAG( ch, CLASSFLAG_DEITIES )){
		if ( ch->deity && ch->deity->name)
		{
			ch->printlnf( "`c You worship `W%s`c.`x", ch->deity->name );
		}
	}

    if(GET_SECURITY(ch)){
        ch->printlnf( " `cYou have an OLC security rating of `C%d`c.`x", GET_SECURITY(ch));
	}

	if ( IS_IMMORTAL(ch)) {
		ch->printlnf( " `cCouncil: %s`x", flag_string( council_flags, ch->pcdata->council ));
	}

	if (IS_NEWBIE_SUPPORT(ch)){
		ch->println( " `cYou are a newbie support member.`x" );
	}

	if (HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER))
	{
		ch->println( " `cYou are tester of the new prac system.`x" );
	}

/*	if (IS_NOBLE(ch))
	{
		ch->printlnf(" `cYou are a noble with a diplomacy of `C%d`c and have currently `C%d`c vote%s,\r\n"
			" Your autovote setting is currently at `C%d`c.`x",
			TRUE_CH(ch)->pcdata->diplomacy,
			TRUE_CH(ch)->pcdata->dip_points,
			TRUE_CH(ch)->pcdata->dip_points==1?"":"s",
			TRUE_CH(ch)->pcdata->autovote);
	}
*/	
	if(HAS_CONFIG(ch, CONFIG_COURTMEMBER))
	{
		ch->println( " `cYou are a member of some court.`x" );
	}

	if (HAS_CONFIG(ch, CONFIG_DISALLOWED_PKILL))
	{
		ch->println( " `cYou have a restriction on you that prevents you from pkilling!`x" );
	}

	if (IS_SET(ch->comm, COMM_CANNOCHANNEL))
	{
		ch->println( " `cYou can nochannel other players.`x" );
	}

	if(!IS_NPC(ch)){
/*		ch->printlnf( " `cCreation points: %d  XPPerLvl: %d`x",
			ch->pcdata->points,
			exp_per_level(ch, ch->pcdata->points));
*/
		if(IS_SET(ch->act, PLR_QUESTER)){
			ch->println(" `cYou have your quester status enabled.`x");
		}else{
			ch->println(" `cYou have your quester status disabled.`x");
		}
		
		if(GAMESETTING(GAMESET_PEACEFUL_MUD)){
			if(HAS_CONFIG(ch, CONFIG_ACTIVE)){
				ch->println(" `cYou are flagged as an active player - but the game is locked to peaceful.`x");
			}else{
				ch->println(" `cYou are a peaceful player - the game is locked as peaceful.`x");
			}
		}else{
			if(HAS_CONFIG(ch, CONFIG_ACTIVE)){
				ch->println(" `cYou are an active player.`x");
			}else{
				ch->println(" `cYou are a peaceful player.`x");
			}
		}

		if(IS_SET(ch->act, PLR_NOREDUCING_MAXKARN)){
			ch->println(" `cYour maxkarns is permanently 5.`x");
		}

		if (HAS_CONFIG(ch, CONFIG_NOTE_ONLY_TO_IMM)){
			ch->println("`c You can only write your notes to the immortals.`x");
		}

		if(ch->pcdata->autoafkafter){
			ch->printlnf(" `cYou automatically go afk after %d minute%s.`x",
				ch->pcdata->autoafkafter,
				ch->pcdata->autoafkafter==1?"":"s");
		}
	}
	

	sprintf(buf, " `cYou are`x");
	if ( ch->pcdata->tired > 16 ) strcat( buf, " `ctired,`x" );
	( ch->pcdata->condition[COND_HUNGER] == 0 ) ?
		strcat( buf, " `chungry,`x" )	: strcat( buf, " `cfull,`x" );
	( ch->pcdata->condition[COND_DRUNK] == 0 ) ?
		strcat( buf, " `csober`x" )	: strcat( buf, " `cdrunk`x" );
	( ch->pcdata->condition[COND_THIRST] == 0 ) ?
		strcat( buf, ", `cthirsty.`x" )	: strcat( buf, "`c.`x" );
	ch->printlnf( "`c%s`x", buf );

    sprintf(buf, " `cYou are currently `x");
    if (ch->subdued)
        strcat( buf, "`csubdued.`x" );
    else
    {
        switch ( ch->position )
		{
        case POS_DEAD:     strcat( buf, "`cDEAD!!`x" );              break;
        case POS_MORTAL:   strcat( buf, "`cmortally wounded.`x" );   break;
        case POS_INCAP:    strcat( buf, "`cincapacitated.`x" );      break;
        case POS_STUNNED:  strcat( buf, "`clying here stunned.`x" ); break;
        case POS_SLEEPING: 
        if (ch->on != NULL)
		{
			if ( IS_SET( ch->on->value[2], SLEEP_AT ))
			{
				sprintf( message, "`csleeping at %s.`x", ch->on->short_descr );
				strcat( buf, message );
			}
			else if ( IS_SET( ch->on->value[2], SLEEP_ON ))
			{
				sprintf( message, "`csleeping on %s.`x", ch->on->short_descr);
				strcat( buf, message );
			}
			else if ( IS_SET( ch->on->value[2], SLEEP_UNDER ))
			{
				sprintf(message, "`csleeping under %s.`x", ch->on->short_descr );
				strcat( buf, message );
			}
			else
			{
				sprintf(message, "`csleeping in %s.`x", ch->on->short_descr);
				strcat(buf,message);
			}
        }
        else 
             strcat(buf,"`csleeping.`x");
        break;

        case POS_RESTING:  
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], REST_AT ))
                {
                    sprintf( message, "`cresting at %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
                else if ( IS_SET( ch->on->value[2], REST_ON ))
                {
                    sprintf( message, "`cresting on %s.`x", ch->on->short_descr );
					strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], REST_UNDER ))
				{
					sprintf( message, "`cresting under %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
                else 
                {
                    sprintf( message, "`cresting in %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
            }
            else
                strcat( buf, "`cresting.`x" );

            if (ch->is_trying_sleep){
                strcat( buf, " `c(You are trying to go to sleep)`x" );
            }
			break;
			
        case POS_SITTING:
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], SIT_AT ))
                {
					sprintf( message, "`csitting at %s.`x", ch->on->short_descr );
					strcat( buf, message );
                }
                else if (IS_SET(ch->on->value[2],SIT_ON))
                {
                    sprintf( message, "`csitting on %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], SIT_UNDER ))
				{
					sprintf( message, "`csiiting under %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
                else
                {
					sprintf( message, "`csitting in %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
			}
            else
				strcat( buf, "`csitting.`x" );
			break;

		case POS_KNEELING:
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], KNEEL_AT ))
                {
					sprintf( message, "`ckneeling at %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
                else if ( IS_SET( ch->on->value[2], KNEEL_ON ))
                {
                    sprintf( message, "`ckneeling on %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], KNEEL_UNDER ))
				{
					sprintf( message, "`ckneeling under %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
                else
                {
					sprintf( message, "`ckneeling in %s.`x", ch->on->short_descr );
                    strcat( buf, message );
                }
			}
            else
				strcat(buf, "`ckneeling.`x");
			break;

		case POS_STANDING:
			if (ch->on != NULL)
			{
				if ( IS_SET( ch->on->value[2], STAND_AT ))
				{
					sprintf( message, "`cstanding at %s.`x", ch->on->short_descr);
					strcat( buf, message );
				}
				else if ( IS_SET( ch->on->value[2], STAND_ON ))
				{
					sprintf( message, "`cstanding on %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
				else if ( IS_SET( ch->on->value[2], STAND_UNDER ))
				{
					sprintf( message, "`cstanding under %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
				else
				{
					sprintf( message, "`cstanding in %s.`x", ch->on->short_descr );
					strcat( buf, message );
				}
			}else{
				strcat( buf, "`cstanding.`x" );
			}
			break;
        
		case POS_FIGHTING:
            strcat( buf, "`cfighting.`x" );
			break;
		}
	}
    ch->printlnf( "`c%s`x", buf );

	if (ch->pnote)
	{
		switch(ch->pnote->type)
		{
		default:
			break;
		case NOTE_NOTE:
			ch->println("`c You are working on a note.`x");
			break;
		case NOTE_IDEA:
			ch->println("`c You are working on an idea.`x");
			break;
		case NOTE_PENALTY:
			ch->println("`c You are working on a penalty!");
			break;
		case NOTE_NEWS:
			ch->println("`c You are working on a news.");
			break;
		case NOTE_CHANGES:
			ch->println("`c You are working on a change.");
			break;
		case NOTE_ANOTE:
			ch->println("`c You are working on an anote.");
			break;
		case NOTE_INOTE:
			ch->println("`c You are working on an inote.");
			break;
		}
	}
	ch->titlebar("");

    return;
}

/**************************************************************************/
char *full_affect_loc_name( AFFECT_DATA *paf);
/**************************************************************************/
void do_affects(char_data *ch, char * argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	
	// support using aff on players by imms
    char_data *v;
	char arg[MIL];
    one_argument( argument, arg );
    if (IS_IMMORTAL(ch) && !IS_NULLSTR(arg)){
		if ( ( v = get_char_world( ch, arg ) ) == NULL )
		{
			ch->printlnf( "`c'%s' isnt here.`x", arg);
			return;
		}
    }else{
		v=ch;
	}
   
	if ( v->affected)
	{
		if(v==ch){
			ch->println("`cYou are affected by the following spells:`x");
		}else{
			ch->printlnf( "`c%s is affected by the following spells:`x", PERS(v, ch));
		}
		for ( paf = v->affected; paf; paf = paf->next )
		{
			if(paf->where==WHERE_OBJECTSPELL){
				continue;
			}
			if (paf_last && paf->type == paf_last->type)
			{
				if (ch->level >= 20){
					ch->printf( "                          ");
				}else{
					continue;
				}
			}
			else
			{
				ch->printf( "`cSpell: %-19s`x", skill_table[paf->type].name );
			}			
			
			if ( ch->level >= 20 ){
				ch->printf( "`c: modifies %s by %d `x",
					full_affect_loc_name( paf), paf->modifier);

				if ( paf->duration == -1 ){
					ch->printf( "`cpermanently`x" );
				}else{ // time shown is approximately the correct ic hours
					if(ch->level>=50){ 
						ch->printf( "`cfor %0.1f hours`x", (float)paf->duration/11.4 ); 
					}else{
						ch->printf( "`cfor %0.0f hours`x", (float)paf->duration/11.4 ); 
					}				
				}
			}else{
				if ( ch->level >= 10 ){
					ch->printf( "`c: modifies %s by %d `x",
						full_affect_loc_name( paf), paf->modifier);
				}else{
					ch->printf( "`c: modifies %s`x",full_affect_loc_name( paf));
				}
			}

			
			ch->println("");
			paf_last = paf;
		}
	}else{
		if(v==ch){
			ch->println("`c  You are not affected by any spells.`x");
		}else{
			ch->printlnf( " `c%s is not affected by any spells.`x", PERS(v, ch));
		}
	}

	if (IS_AFFECTED(v, AFF_HIDE))
	{
		if(v==ch){
			ch->println("`cYou are currently trying to hide with your surroundings.`x");
		}else{
			ch->printlnf( "`c%s is currently trying to hide with their surroundings.`x", PERS(v, ch));
		}
	}

/*	if (IS_AFFECTED2(v, AFF2_VANISH))
	{
		if(v==ch){
			ch->println("`cYou are currenty invisible to anyone but the faeries.`x");
		}else{
			ch->println("`c%s if currently invisible to anyone but the faeries.`x");
		}
	}*/
	return;
}

/**************************************************************************/
char *	const	day_name	[] =
{
    "Heliou", "Selenes", "Areos", "Hermu", "Dios",
    "Aphrodites", "Khronu"
};


// now have 12 months 36 days
char *	const	month_name	[] =
{
    "Gamelion", "Anthesterion", "Elaphebolion", 
	"Mounichion","Thargelion","Skirophorion",
	"Hekatombaion",  "Metageitnion", "Boedromion", 
	"Pyanopsion", "Maimakterion", "Poseideon", 
};
/**************************************************************************/
void do_time( char_data *ch, char * )
{
    char *suf;
    int day;
	
	time_t rawtime;
  	tm * ptm;
	time ( &rawtime );
	ptm = localtime ( &rawtime );
    
	day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

//	ch->printlnf("`WIn Character Time:`x\n");
    ch->printlnf(
        "`cIt is %d o'clock %s:\nThe day of %s, %d%s the Month of %s in the year 500 BC.\r\n",
        (time_info.hour % (ICTIME_HOURS_PER_DAY/2) == 0) ?
			(ICTIME_HOURS_PER_DAY/2) : time_info.hour %(ICTIME_HOURS_PER_DAY/2),
        time_info.hour >= (ICTIME_HOURS_PER_DAY/2) ? "pm" : "am",
        day_name[day % ICTIME_DAYS_PER_WEEK],
        day, suf,
        month_name[time_info.month]
		/*time_info.year*/);

	// system time
/*
	ch->printlnf("`#`gPacfic US time is: %s`^", asctime (ptm));

	ptm->tm_hour++;
mktime (ptm);
	ch->printlnf("`#`gMountain US time is: %s`^", asctime (ptm));

	ptm->tm_hour++;
mktime (ptm);
	ch->printlnf("`#`gCentral US time is: %s`^", asctime (ptm));

	ptm->tm_hour++;
mktime (ptm);
	ch->printlnf("`#`gEastern US time is: %s`^", asctime (ptm));

	ptm->tm_hour = ptm->tm_hour +5;
mktime (ptm);
	ch->printlnf("`#`gGreenwich time is: %s`^", asctime (ptm));
*/
	ch->println("`#`cTo see a list of contests and group quests, type `YContests`^.");
    	return;
}

/**************************************************************************/
void do_uptime( char_data *ch, char * )
{
    extern time_t boot_time;

    timediff(boot_time, current_time);

	// system time
	ch->printf( "`cThe system time is             %s", (char *) ctime( &current_time ));

    // dawn startup time
	ch->printlnf( "`c%s last rebooted at %s"
		        "Which was %s ago.", 
				MUD_NAME,
				ctime( &lastreboot_time),
				timediff(lastreboot_time, current_time));

	if (!IS_NPC(ch))
	{
		// online time
		ch->printlnf("`cYou connected at               %s"
					"Which was %s ago.",
					 (char *) ctime( &ch->logon), 
					 timediff(ch->logon, current_time));
		// creation time
		ch->printlnf("`cYou first created at           %s"
					"Which was %s ago.",
					 (char *) ctime( (time_t *)&ch->id), 
					 short_timediff(ch->id, current_time));
		ch->printlnf("`cYou have played approximately %d.%02d hours.`x",
            (int) GET_SECONDS_PLAYED(ch)/ 3600, 
			(int) (GET_SECONDS_PLAYED(ch)/36)%100);

		ch->printlnf("`cWhich is %0.03f%% of the time since you created.`x",
			((double)GET_SECONDS_PLAYED(ch)/(double)(current_time-ch->id))*100.0);
	}		

	// hotreboot
	ch->printlnf("`cLast hotreboot was at          %s`x"
            "`cWhich was %s ago.`x", str_boot_time, 
			timediff(boot_time, current_time));

	if (IS_IMMORTAL(ch))
	{
		int i = get_magecastmod()*3/2;

		ch->printlnf( "`cMage spell casting modifier %d`x", 
			weather_info[ch->in_room->sector_type].mage_castmod );
		ch->printlnf( "`cMax mod for day %d/36 month %d/12 is %d`x",
			time_info.day+1, time_info.month+1, i);
	}
	ch->println("`cTo see game time related info use the ictime command.`x");

    return;
}


/**************************************************************************/
void do_weather( char_data *ch, char * )
{
    char buf[MSL];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

	if ( IS_OOC(ch) )
    {
		ch->println("`cWeather is an IC command, you can't use it in OOC rooms.`x");
		return;
    }

    if ( !IS_OUTSIDE(ch) || ch->in_room->sector_type == SECT_CAVE )
    {
		ch->println("`cYou can't see the sky from here.`x");
		return;
    }

    ch->printlnf( "`cThe sky is %s and %s.`x",
		sky_look[weather_info[ch->in_room->sector_type].sky],
		weather_info[ch->in_room->sector_type].change >= 0 ?
		"`ca warm southerly breeze blows`x" : "`ca cold northern gust blows`x" );

	{
		int i = (int)get_magecastmod()*3/2;

		if (weather_info[ch->in_room->sector_type].moon_getting_brighter)
		{
			switch (i)
			{
			case - 12:
			case - 11:
			case - 10: // rare only once per game year (1 week real time)
sprintf( buf, "`cThe sky is so dark, it is like the moon doesn't exist.`x\r\n"); break;
			case - 9:
			case - 8:
sprintf( buf, "`cThe sky is pitch black as the moon can not be seen.`x\r\n"); break;
			case - 7: // about once per month for half of the year
sprintf( buf, "`cThe moon is engulfed in shadow.`x\r\n"); break;
			case - 6: 
sprintf( buf, "`cThe sky grows dark as the moon obscures the light of the sun.`x\r\n"); break;
			case - 5:
sprintf( buf, "`cThe Dark Moon rules the sky not a ray of light can be seen.`x\r\n"); break;
			case - 4:
sprintf( buf, "`cA faint sliver of moon light slices the sky.`x\r\n"); break;			
			case - 3:
sprintf( buf, "`cA thin sliver of moonlight glimmers in the heavens.`x\r\n"); break;
			case - 2:
sprintf( buf, "`cThe crescent moon glows softly in the sky.`x\r\n"); break;
			case - 1:
sprintf( buf, "`cThe moonbeams gleam more brightly.`x\r\n"); break;
			case   0:
sprintf( buf, "`cThe moon, half alight, shines in the heavens.`x\r\n"); break;
			case   1:
sprintf( buf, "`cThe moonbeams gleam more brightly.`x\r\n"); break;
			case   2:
sprintf( buf, "`cThe moon light dances as the waxing engulfs darkness.`x\r\n"); break;			
			case   3:
sprintf( buf, "`cThe moon radiates soft light as it waxes towards full.`x\r\n"); break;
			case   4:
sprintf( buf, "`cThe pale sickly glow of the three quarter moon washes the land in its light.`x\r\n"); break;
			case   5:
sprintf( buf, "`cOnly a small sliver of darkness dampens the moon's splendor.`x\r\n"); break;
			case   6: 
sprintf( buf, "`cThe moon's glory is nearly at its zenith.`x\r\n"); break;
			case   7: // about once per month for half of the year
sprintf( buf, "`cThe full moon dazzles you with its brightness.`x\r\n"); break;
			case   8:
			case   9: // anything above here is very rare 
			case  10:
sprintf( buf, "`cThe full moon DAZZLES you with its brightness.`x\r\n"); break;
			case  11: // 11 is for about 4minutes every week real time!
sprintf( buf, "`cThe full moon is BLINDING with its brightness!`x\r\n"); break;
			default:
sprintf( buf, "`cUnknown moon value mc=%d, i=%d, p=1 - please report this to Admin.`x\r\n", 
		weather_info[ch->in_room->sector_type].mage_castmod, i);
				break;
			}
		}
		else // moon getting darker
		{
			switch (i)
			{
			case - 13:
sprintf( buf, "`cThe sky is the darkest you can actually remember, it is like the sky has never seen the moon!`x\r\n"); break;
			case - 12:
			case - 11:
			case - 10: // rare only once per game year (1 week real time)
sprintf( buf, "`cThe sky is so dark, it is like the moon doesn't exist.`x\r\n"); break;
			case - 9: 
			case - 8:
sprintf( buf, "`cThe sky is pitch black as the moon can not be seen.`x\r\n"); break;
			case - 7: // about once per month for half of the year
sprintf( buf, "`cThe moon is engulfed in shadow.`x\r\n"); break;
			case - 6: 
sprintf( buf, "`cThe sky grows dark as the moon obscures the light of the sun.`x\r\n"); break;
			case - 5:
sprintf( buf, "`cThe Dark Moon rules the sky not a ray of light can be seen.`x\r\n"); break;
			case - 4:
sprintf( buf, "`cA dim sliver of moon light slices the sky.`x\r\n"); break;
			case - 3:
sprintf( buf, "`cOnly the edge of moon light still shines upon the world.`x\r\n"); break;
			case - 2:
sprintf( buf, "`cA crescent moon hangs shimmering in the sky.`x\r\n"); break;
			case - 1:
sprintf( buf, "`cThe moonlight fades to a soft glow.`x\r\n"); break;
			case   0:
sprintf( buf, "`cThe half-lit moon washes the land.`x\r\n"); break;
			case   1:
sprintf( buf, "`cDarkness comes forth, but the moon yet shines in the night.`x\r\n"); break;
			case   2:
sprintf( buf, "`cThe moon light has its last dance as its light is engulfed.`x\r\n"); break;
			case   3:
sprintf( buf, "`cThe moon radiates soft light as it wans towards darkness.`x\r\n"); break;
			case   4:
sprintf( buf, "`cThe moon is sliced by the encroaching darkness.`x\r\n"); break;
			case   5:
sprintf( buf, "`cThe moon's glory is slightly faded in the night sky.`x\r\n"); break;
			case   6: 
			case   7: // about once per month for half of the year
sprintf( buf, "`cThe full moon dazzles you with its brightness.`x\r\n"); break;
			case   8:
			case   9: // anything above here is very rare 
			case  10:
sprintf( buf, "`cThe full moon DAZZLES you with its brightness.`x\r\n"); break;
			case  11: // 11 is for about 4minutes every week real time!
sprintf( buf, "`cThe full moon is BLINDING with its brightness!`x\r\n"); break;
			default:
sprintf( buf, "`cUnknown moon value mc=%d, i=%d, p=2 - please report this to Admin`x\r\n", 
		weather_info[ch->in_room->sector_type].mage_castmod, i);
				break;
			}
		}

		if (i>-7) // can't see the moon really at this time of night
		{
			switch (time_info.hour)
			{
				case 20:
					ch->println("`cThe tip of the moon can be seen on the surface of the horizon.`x");
					break;
				case 21:
					ch->println("`cThe moon rises just above the the horizon.`x");
					break;
				case 22:
					ch->println("`cThe moon has risen midway up the sky.`x");
					break;
				case 23:
					ch->println("`cThe moon is almost directly above you.`x");
					break;
				case 0:
					ch->println("`cThe moon is directly above.`x");
					break;
				case 1:
					ch->println("`cThe moon is starting to fall.`x");
					break;
				case 2:
					ch->println("`cThe moon is about midway down the sky.`x");
					break;
				case 3:
					ch->println("`cThe moon is falling just below the the horizon.`x");
					break;
				case 4:
					ch->println("`cThe tip of the moon is disappearing below the surface of the horizon.`x");
					break;
				default: // day time
					break;
			}
		}
		if (time_info.hour<5 || time_info.hour>20)
			ch->printf( "`c%s`x", buf );
	}
    return;
}
/**************************************************************************/
// Airius 
void do_lore(char_data *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf=NULL;
    int skill;
	
    // find out what 
    if (argument == '\0')
    {
		ch->println("`cLore what item?`x");
		return;
    }
	
	if (ch->position == POS_FIGHTING) {
		ch->println("`cNot while fighting you don't!`x");
		return;
	}
	
    obj =  get_obj_list(ch,argument,ch->carrying);
	
    if (obj== NULL)
    {
        ch->println("`cYou don't have that item.`x");
        return;
    }
	
    if ((skill = get_skill(ch,gsn_lore)) < 1)
    {
        ch->println("`cYeah right, like you have even ever looked at a book.`x");
        return;
    }
	
/*    if (ch->mana<40){
		ch->println("`cYou have not enough mana.`x");
        return;
    }else{
        ch->mana-=40;
	}
*/
    if (number_percent()>= skill - obj->level + ch->level)  
    { 
		// failure at lore
	    ch->wrapln("`cYou try to think back into your memory, even after a period of deep`x "
		    "`cconcentration, nothing useful surfaces from your thoughts.`x");
		return;
	}
		
	// success! 	
	// Herbs can't be lored
	if ( obj->item_type == ITEM_HERB ) {
		ch->println("`cYou can't learn anything important about this item.`x");
//		ch->mana+=40;
		return;
	}
	
	ch->printlnf(
		"`cObject '%s' is type %s,`x\r\n"
		"`cExtra flags %s.`x\r\n"
		"`cExtra2 flags %s.`x\r\n"
		"`cWeight is %0.1f lbs, value is %d, level is %d.`x",
        obj->name, 
		item_type_name( obj ),
		extra_bit_name( obj->extra_flags ),
		extra2_bit_name( obj->extra2_flags ),
		((double)obj->weight) / 10,
		obj->cost,
		obj->level );
	
    switch ( obj->item_type )
    {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ch->printf( "`cLevel %d spells of:`x", obj->value[0] );
		
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
		{
			ch->printlnf( " `c'%s'`x", skill_table[obj->value[1]].name  );
		}
		
		if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
		{
			ch->printlnf( " `c'%s'`x", skill_table[obj->value[2]].name );
		}
        
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printlnf( " `c'%s'`x", skill_table[obj->value[3]].name );
		}
        
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
			ch->printlnf( " `c'%s'`x", skill_table[obj->value[4]].name );
		}
        break;
		
	case ITEM_WAND:
	case ITEM_STAFF:
		ch->printf( "`cHas %d charges of level %d`x", obj->value[2], obj->value[0] );

		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printlnf( " `c'%s'.`x", skill_table[obj->value[3]].name );
		}

        break;
		
	case ITEM_DRINK_CON:
		ch->printlnf( "`cIt holds %s-colored %s.`x",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
        break;

	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		ch->printlnf( "`cMaximum combined weight: %d lbs, Capacity for an individual item: %d lbs.`x",
			obj->value[0], obj->value[3]);
		ch->printlnf( "`cFlags: %s.`x", cont_bit_name(obj->value[1]));
		if (obj->value[4] != 100){
			ch->printf( "`cWeight multiplier: %d%%`x\r\n", obj->value[4]);
		}
		break;
		
	case ITEM_WEAPON:
		ch->printlnf( "`cWeapon type is %s`x", get_weapontype( obj ));

		ch->printlnf( "`cDamage is %dd%d (average %d).`x",
			obj->value[1],obj->value[2],
			(1 + obj->value[2]) * obj->value[1] / 2);
		if(obj->value[4])  // weapon flags 
		{
			ch->printlnf( "`cWeapons flags: %s`x",weapon_bit_name(obj->value[4]));
		}
        break;
		
        case ITEM_ARMOR:
            ch->printlnf(
				"`cArmor class is %d pierce, %d bash, %d slash, and %d vs. mystical.`x",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
			break;
    }
	
    for ( paf = OBJECT_AFFECTS(obj); paf; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {       
			if(paf->duration > -1){
				ch->printf( "`cAffects %s by %d for %d hour%s.%s`x\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					paf->duration, paf->duration==1?"":"s",
					(paf->level>ch->level?" `x (above your level)`x":""));
			}else{
				ch->printf( "`cAffects %s by %d.%s`x\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					(paf->level>ch->level?" `c (above your level)`x":""));
			}
			
			if (paf->bitvector)
			{
				ch->printlnf( "`c%s`x", to_affect_string( paf, obj->level ));
			}
		}
	}
	check_improve(ch,gsn_lore,true,2);
    return;
}

/**************************************************************************/
int count_active_players(void);
// displays info to the player about the mud - by Kalahn 98
void do_mudstats( char_data *ch, char * )
{
	int w=70; // width
	
    centerf_to_char( ch, w,"Area Count          = %5d\r\n", top_area      );
    centerf_to_char( ch, w,"Unique Room count   = %5d\r\n", top_room      );
    centerf_to_char( ch, w,"Shop count          = %5d\r\n", top_shop      );
	centerf_to_char( ch, w,"Inn count           = %5d\r\n", top_inn       );
    centerf_to_char( ch, w,"Unique Mobile count = %5d\r\n", top_mob_index );
    centerf_to_char( ch, w,"Mobile inuse count  = %5d\r\n", mobile_count  );
    centerf_to_char( ch, w,"Mobprog count       = %5d\r\n", mobprog_count );
	centerf_to_char( ch, w,"Unique Object count = %5d\r\n", top_obj_index );
    centerf_to_char( ch, w,"Help Entries count  = %5d\r\n", top_help      );
	centerf_to_char( ch, w,"Social count        = %5d\r\n", social_count );
/*
	ch->println( "" );
	centerf_to_char( ch, w,"`^-=====================`G Server Info`^ ======================-`x\r\n");
	centerf_to_char( ch, w,"The Current System time is %.24s\r\n",
                 (char *) ctime( &current_time ));
    centerf_to_char( ch, w,"The mud is running from %s.\r\n", MACHINE_NAME);
    centerf_to_char( ch, w,"Accepting telnet connections on port %d.\r\n", mainport);
    if (IS_IRC(ch) || IS_IMMORTAL(ch)){
		centerf_to_char( ch, w,"IRC DCC chat connections are on port %d.\r\n", ircport);
  }
	centerf_to_char( ch, w,"Integrated webserver connections are on port %d.\r\n", webport);
	centerf_to_char( ch, w,"MudFtp connections are on port %d.\r\n", mudftpport); 
    centerf_to_char( ch, w,"Webhits %d, WebHelpHits %d, WebWhohits %d\r\n", 
		webHits, webHelpHits, webWhoHits);


	centerf_to_char( ch, w,"`^-=====================`B Last Reboot`^ ======================-`x\r\n");
	centerf_to_char( ch, w,"%s last rebooted at %.24s\r\n", MUD_NAME, ctime( &lastreboot_time));
	centerf_to_char( ch, w,"Which was %s ago,\r\n", timediff(lastreboot_time, current_time));
	centerf_to_char( ch, w, "since then the maxon has been `B%d`x\r\n", true_count);
	centerf_to_char( ch, w,"This was `B%s`x ago\r\n", timediff(maxon_time, current_time));
	centerf_to_char( ch, w,"at %.24s systime.\r\n", (char *) ctime( &maxon_time ));

	centerf_to_char( ch, w,"`^ -=======================`R HotReboot`^ =========================-`x\r\n");
	centerf_to_char( ch, w,"%s last hotrebooted at %.24s\r\n", MUD_NAME, str_boot_time);			
	centerf_to_char( ch, w,"Which was %s ago,\r\n", timediff(boot_time, current_time));
	centerf_to_char( ch, w,"since then the maxon has been `R%d`x\r\n", hotrebootmaxon);
	centerf_to_char( ch, w,"This was `R%s`x ago\r\n", timediff(hotrebootmaxon_time, current_time));
	centerf_to_char( ch, w,"at %.24s systime.\r\n", (char *) ctime( &hotrebootmaxon_time));
	ch->println( "" );
	do_compile_time(ch,"");
	centerf_to_char( ch, w,"Active Player Count = %d, (>5 <%d, on in the last week)\r\n", 
		count_active_players(), LEVEL_IMMORTAL);
*/
}

/**************************************************************************/
void do_inventory( char_data *ch, char *argument )
{
	ch->println( "`cYou are carrying:`x" );
	show_list_to_char( ch->carrying, ch, argument, true, true );
	ch->print( "`x" );

	if(IS_NULLSTR(argument) && IS_NEWBIE(ch)){
		ch->println("`cYou can filter the inventory list by text and item type...`x");
		ch->println("`cexamples: 'inv skin', 'inv mace', 'inv food'...`x");
	}

	return;
}

/**************************************************************************/
void do_equipment( char_data *ch, char * )
{
	OBJ_DATA *obj;
	int iWear;
	bool found;
	
	ch->println( "`cYou are wearing:`=\xab");
	found = false;

	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if (( iWear >= WEAR_LODGED_ARM - 1 && iWear <= WEAR_LODGED_RIB )
		&& get_eq_char( ch, iWear) == NULL )
			continue;	
		
		if ( iWear == WEAR_SHEATHED
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		if ( iWear == WEAR_CONCEALED
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		ch->print( where_name[iWear] );
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL ){
//			ch->println( "");
			continue;
		}
		
		if ( can_see_obj( ch, obj ) )
		{
			ch->printlnf( "     `c%s`=\xab", format_obj_to_char( obj, ch, true ));

		}
		else
		{
			ch->println( "`csomething.`x" );
		}
		found = true;
	}
	ch->print( "`x" );
	return;
}
/**************************************************************************/
// Find an obj in player's inventory.
OBJ_DATA *get_obj_carry_of_type_excluding( char_data *ch, char *argument, OBJ_DATA *exclude)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
		if(obj==exclude || obj->item_type!=exclude->item_type){
			continue;
		}
		if( obj->wear_loc == WEAR_NONE
		&&   (can_see_obj( ch, obj ) ) 
		&&   is_name( arg, obj->name ) )
		{
			if( ++count == number )
			return obj;
		}
    }
    return NULL;
}
/**************************************************************************/
enum COMPARE_RESULT{COMPARE_WORSE, COMPARE_ABOUT_SAME, COMPARE_BETTER};
/**************************************************************************/
COMPARE_RESULT calc_compare( obj_data *obj1, obj_data *obj2)
{
	assertp(obj1);
	assertp(obj2);

	int value1=0;
	int value2=0;
	switch ( obj1->item_type )
	{
	default:
		bug("`cdisplay_compare(): How did we get here?`x");
		do_abort();
		break;
	case ITEM_ARMOR:
		value1 = obj1->value[0] + obj1->value[1] + obj1->value[2] + obj1->value[3];
		value2 = obj2->value[0] + obj2->value[1] + obj2->value[2] + obj2->value[3];
		break;
	case ITEM_WEAPON:
		value1 = (1 + obj1->value[2]) * obj1->value[1];
		value2 = (1 + obj2->value[2]) * obj2->value[1];
		break;
	}

	int r=value1-value2; 
	if(r<-1){ 
	  return COMPARE_WORSE; 
	}else if (r>1){ 
	  return COMPARE_BETTER;
	} 
	// r is either -1, 0 or 1
	return COMPARE_ABOUT_SAME; 
}

/**************************************************************************/
// Compare an object to everything comparable a player carries.
// - I know this is far from the most efficient way to write this
//   but it is a quick hack to give nice the functionality - Kal, Feb 2001.
void do_compare( char_data *ch, char *argument )
{
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int count;

	if ( IS_NULLSTR(argument) )
	{
		ch->println( "`cCompare what to everything you carry?`x" );
		return;
	}

	if ( ( obj1 = get_obj_carry( ch, argument ) ) == NULL )
	{
		ch->printlnf( "`cYou do not have any '%s' item.`x", argument);
		return;
	}

	if(obj1->item_type!=ITEM_ARMOR && obj1->item_type!=ITEM_WEAPON){
		ch->printlnf("`cYou can only compare weapons or armour and '%s' is neither.`x", 
			obj1->short_descr);
		return;
	}

	// we have an item - compare it with everything we can
	name_linkedlist_type* list[3], *plist, *plist_next;
	for(count=0;count<3; count++){
		list[count]=NULL;
	}

	bool found=false;
	char vnumtext[MIL];
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
		if( can_see_obj(ch,obj2)
			&&  obj1->pIndexData!=obj2->pIndexData
			&&  obj1->item_type == obj2->item_type			
			&&  (obj1->wear_flags & obj2->wear_flags & ~OBJWEAR_TAKE) != 0 )
		{
			sprintf(vnumtext, "%s%07d", obj2->short_descr, obj2->pIndexData->vnum);
			addlist(&list[calc_compare(obj1, obj2)],vnumtext, (int)*(get_canwear_colour(obj2, ch)+2), false, false);
			found=true;
		}
	}
	char objprefix[MIL];
	sprintf(objprefix,"%s%s `xis ", get_canwear_colour(obj1, ch), obj1->short_descr);	

	for(count=0; count<3; count++){
		bool first=true;
		char *comp="";
		for(plist=list[count];plist; plist=plist_next){
			plist_next=plist->next;
			if(first){
				switch(count){
					case COMPARE_WORSE: comp="`cworse than`x"; break;
					case COMPARE_ABOUT_SAME: comp="`cabout the same as`x"; break;
					case COMPARE_BETTER: comp="`cbetter than`x"; break;
				}
				first=false;
			}	
			size_t i=str_len(plist->name)-7;
			plist->name[i]='\0'; // hide the number at the end
			ch->print(objprefix);
			ch->printlnf("%s `=%c%s`x", comp, (char)plist->tag, plist->name);
			plist->name[i]='0';
			free_string(plist->name);
			delete plist;
		}
	}
	
	if(!found){		
		ch->printlnf("`cYou have nothing that is comparable to %s.`x", 
			obj1->short_descr);
	}

	return;
}

/**************************************************************************/
void do_credits( char_data *ch, char * )
{
    do_help( ch, "credits" );
	return;
}

/**************************************************************************/
void do_where( char_data *ch, char *argument )
{
	char arg[MIL];
    char_data *victim=NULL;
    connection_data *d;
    bool found;

    one_argument( argument, arg );
	
    if ( arg[0] == '\0' )
    {
		ch->println( "`cPlayers near you:`x" );
		found = false;
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING
				&& ( victim = d->character ) != NULL
				&&   !IS_NPC(victim)
				&&   victim->in_room != NULL
				&&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
				&&   is_room_private_to_char( victim->in_room, ch )
				&&   victim->in_room->area == ch->in_room->area
				&&   can_see( ch, victim ) )
			{
				found = true;
				
				ch->printlnf( "`c[%s%5d`x] %-12s is in %s (%s)`x",
					colour_table[(victim->in_room->vnum%14)+1].code,
					victim->in_room->vnum, victim->name,
					victim->in_room->name, victim->in_room->area->name);
			}
		}
		if ( !found )
			ch->println( "`cNone.`x" );
    }
    else
    {
		victim = get_char_world( ch, arg );

		if ( victim )
		{
			ch->printlnf( "`c[%s%5d`x] %-12s is in %s (%s)`x",
				colour_table[(victim->in_room->vnum%14)+1].code,
				victim->in_room->vnum, victim->name,
				victim->in_room->name, victim->in_room->area->name);
		}
		else
		{
			act( "`cYou didn't find any $T.`x", ch, NULL, arg, TO_CHAR );
		}
	}
	return;
}


/**************************************************************************/
void where_char( char_data *victim, char_data *ch,
   sh_int depth, sh_int door )
{
    char buf[MIL], buf2[MIL];
    buf[0] = '\0';

    strcat( buf, PERS( victim, ch ));
    strcat( buf, ", " );
    sprintf( buf2, where_distance[depth], dir_name[door] );
    strcat( buf, buf2 );
    ch->println( buf );
    return;
}

/**************************************************************************/
void do_append( char_data *ch, char * )
{
	ch->println("`cTo edit your description type `Rdescript`c.`x" );
    return;
}
/**************************************************************************/
void do_description( char_data *ch, char * )
{
	if(IS_NPC(ch))
        return;

	ch->println("`cEntering edit mode for you own description.`x" );
	ch->println("`cType @ to finish editing your description.`x" );
	string_append(ch, &ch->description);

	return;

}
/**************************************************************************/
void do_pkinfo( char_data *ch, char *argument )
{
	char_data *victim;

    if (IS_IMMORTAL(ch) && !IS_NULLSTR(argument)){
		if ( ( victim = get_char_world( ch, argument ) ) == NULL )
		{		
			ch->printlnf( "`c'%s' couldn't be found.`x", argument);
			return;
		}else{
			ch->printlnf( "`cShowing pkinfo for %s.`x", PERS(victim, ch));			
		}
    }else{
		victim=ch;
	}

	int w=79;

	ch->print("`=t`#");
	if(GAMESETTING(GAMESET_PEACEFUL_MUD)){
		ch->titlebar("THE GAME IS LOCKED IN PEACEFUL MODE");
	}

	ch->titlebar("PLAYER KILL INFO");
	

	centerf_to_char(ch, w,"`cYou have player killed %d time%s.`x\r\n",
			  victim->pkkills, victim->pkkills==1 ? "" : "s");

	centerf_to_char(ch, w, "`cYou have been player killed %d time%s.`x\r\n",
			  victim->pkdefeats, victim->pkdefeats==1 ? "" : "s");

	centerf_to_char(ch, w,"`cYou are safe from being pkilled for %d more hour%s.`x\r\n",
			  victim->pksafe, victim->pksafe==1 ? "" : "s");

	centerf_to_char(ch, w,"`cYou may not OOL for %d hour%s.`x\r\n",
			  victim->pkool, victim->pkool==1 ? "" : "s");

	centerf_to_char(ch, w,"`cYou can not recall for %d hour%s.`x\r\n\r\n",
			  victim->pknorecall, victim->pknorecall==1 ? "" : "s");


	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
		centerf_to_char(ch, w,"`cYou can not quit for %d hour%s.`x\r\n",
				  victim->pknoquit, victim->pknoquit==1 ? "" : "s");
		if(!IS_NPC(ch)){
			centerf_to_char(ch, w,"P9999Kills %d, P9999Defeats %d\r\n",
					victim->pcdata->p9999kills, victim->pcdata->p9999defeats);
		}
	}else{
		centerf_to_char(ch, w,"`cYou can not quit for %d hour%s.`x\r\n",
				  UMAX(victim->pknoquit,victim->pknorecall) , 
				  UMAX(victim->pknoquit,victim->pknorecall)==1 ? "" : "s");
	}

	// mkill info and stealing consquences
	if (!IS_NPC(victim))
	{
		ch->titlebar("MOBILE DEFEAT COUNTER");
		centerf_to_char(ch, w,"`cIn your lifetime you have been defeated by %d mob%s.`x\r\n",
			  victim->pcdata->mdefeats, victim->pcdata->mdefeats==1 ? "" : "s");


		if(victim->pcdata->unsafe_due_to_stealing_till>current_time){
			int t=(int)((victim->pcdata->unsafe_due_to_stealing_till-current_time)/60)+1;
			ch->print_blank_lines(1);
			ch->titlebar("STEALING CONSEQUENCES");
			centerf_to_char(ch, w,"`cYou will currently automatically accept all duels for the next %d minute%s.`x\r\n",
				t, t==1?"":"s");
		} 
	}

	// display duel info if duel system is enabled (on by default)
	if(!GAMESETTING2(GAMESET2_NO_DUEL_REQUIRED)){		
		if(victim->duels){
			victim->duels->display_pkinfo(ch);
		}
	}

	ch->titlebar("");
}

/**************************************************************************/
void show_practice_list(char_data *showto, char_data *victim)
{
    BUFFER *output;
    char buf[MSL];
    int sn;
    int col;

    output = new_buf();
    col    = 0;

	if(HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER)){
		sprintf(buf,"`?%s", makef_titlebar("PRACTICING FOR NEW PRAC SYSTEM TESTING"));
	}else{
		sprintf(buf,"`?%s", makef_titlebar("PRACTICING"));
	}
	buf[str_len(buf)-2]= '\0';
	strcat(buf,"`x\r\n");
	add_buf( output, buf);
	add_buf( output, percent_colour_codebar());

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
		if ( IS_NULLSTR(skill_table[sn].name))
            break;
		
        if (victim->pcdata->learned[sn] < 1) // skill is not known 
            continue;
		
        if ( !IS_SPELL(sn) 
			&& (victim->level < skill_table[sn].skill_level[victim->clss]) )
            continue;

		// level zero skills - imm only 
		if ((skill_table[sn].skill_level[victim->clss]==0)
				&& victim->level<LEVEL_IMMORTAL)  
			continue;
		
        if ( col % 3 == 1 ){
	        sprintf( buf, "  %s%-17.17s `x%3d%%   ",
				percent_colour_code(victim->pcdata->learned[sn]),
				skill_table[sn].name, victim->pcdata->learned[sn] );
		}else{
	        sprintf( buf, " %s%-18.18s `x%3d%%  ",
				percent_colour_code(victim->pcdata->learned[sn]),
				skill_table[sn].name, victim->pcdata->learned[sn] );
		}

        add_buf( output, buf );
        if ( ++col % 3 == 0 )
            add_buf( output, "\r\n" );
    }
	
    if ( col % 3 != 0 )
        add_buf( output, "\r\n" );
    sprintf(buf, "\r\n                       You have %d practice session%s left.\r\n",
        victim->practice, victim->practice==1?"":"s");
	add_buf( output, buf );

    showto->sendpage(buf_string(output));
    free_buf(output);		
}

/**************************************************************************/
void new_practice( char_data *ch, int sn, int number_of_times)
{
	if (sn==-1 || number_of_times<1){
		return;
	}
	// can't improve in ooc rooms
    if (IS_OOC(ch)) {
		ch->println("BUG: You shouldn't be able to prac in an OOC ROOM - please report to admin!");
		return;
	}

	new_practice( ch, sn, number_of_times-1); // recursively call it

	// calculate the maxprac and maxlearn values
	int maxlearn=skill_table[sn].maxprac_percent[ch->clss];
	if(maxlearn<1){
		maxlearn=65; // default max learn - for unskilled
	}
	int maxprac=skill_table[sn].maxprac_percent[ch->clss];
	if(maxprac<1){
		maxprac=50; // default max prac
	}
	// can't prac better than the max prac amount for your class 
	if(ch->pcdata->learned[sn]>=maxprac){
		return; 
	}

	// store the per thousand improve table
	static short improve_chance_lookup_table[102]; // stored in range 1 ->10000
	static bool initialise_table=true;
	if(initialise_table){ // calculate the improvement chances once, first time
		int i;
		double f;
		for(i=0; i<=100; i++){
	//		f=sqrt((100.0-i)/100);
			f=(100.0-i)/100;
			f=pow(100.0,f);
			f*=100; // put in 1->10000 range - increased precision
			improve_chance_lookup_table[i]=(short)f;
//			logf("improve_chance_lookup_table[%d]=%d", i, improve_chance_lookup_table[i]);
		}
		improve_chance_lookup_table[101]=1;
		initialise_table=false;
	}

	int chance=improve_chance_lookup_table[ch->pcdata->learned[sn]];
	int max_percent=UMAX(maxlearn, maxprac);
	if(max_percent>0 && max_percent<100){ // scale it to work in the range for your class
		chance=chance* max_percent/100;
		if(chance<1){
			chance=1;
		}
	}
	if (number_range(1,10000)> chance){
		return;
	}
	ch->pcdata->learned[sn]++;
}
/**************************************************************************/
void do_practice( char_data *ch, char *argument )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
	char arg1[MSL];
    int sn;
	int maxprac;
	
    if ( IS_NPC(ch))
        return;
	
    if ( IS_NULLSTR(argument ))
	{
		show_practice_list(ch, ch);
		return;
    }

    char_data *mob;
    int adept;
	
	if (!str_prefix("'",argument)){  // ' symbol optional
		argument = one_argument( argument, arg1 );
	}else{
		strcpy(arg1, argument);
	}
	
    if ( !IS_AWAKE(ch) )
    {
        ch->println("`cWhile in the dreamworld?`x");
        return;
	}
	
	// Find the practice mob in the room
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
            break;
    }
    if( !mob){			
        ch->println( "`cYou can't do that here.`x" );
        return;
    }

    // preprac trigger, activated only on mobs with the trigger 
	// if the command 'mob preventprac' is called, then the
	// pracing is aborted
	if ( IS_NPC(mob) && HAS_TRIGGER(mob, TRIG_PREPRAC)){
		mobprog_preventprac_used=false;
		if(mp_percent_trigger( mob, ch, NULL, NULL, TRIG_PREPRAC)){
			if(mobprog_preventprac_used){
				mob->printlnf("Pracing prevented on %s", PERS(ch, NULL));
				return;
			}
		}
	}

	if(!IS_ICIMMORTAL(ch)){
		if(!can_see(mob, ch)){
			ch->printlnf("`cIt looks as though %s would be able to help you practice,`x\r\n"
				"`cgo visible then try again.`x", PERS(mob, ch));
			return;
		}
	}

    if ( ch->practice <= 0 )
    {
        ch->println("`xYou have no practice sessions left.`x");
        return;
    }
	
    if ( ( sn = find_spell( ch,arg1, false) ) < 0)
    {
        ch->printlnf( "`cNo such skill/spell '%s' exists.`x", arg1);
        return;
    }
	
    // all ch's down here have pcdata
    if (ch->pcdata->learned[sn] < 1) // skill is not known
    {
        ch->printlnf( "`cYou know nothing of the skill/spell `C%s `cyet.`x", 
			skill_table[sn].name);
        return;
    }
	
	if ( !IS_SPELL(sn) && (ch->level < skill_table[sn].skill_level[ch->clss]) )
    {
        ch->printlnf( "`cYou haven't reached the level required to use the skill `C%s`c yet.`x", 
			skill_table[sn].name);
        return;
    }

	// level zero skills - imm only 
	if (skill_table[sn].skill_level[ch->clss]==0) 
	{ 
        ch->printlnf( "`cYou can't practice %s.`x", skill_table[sn].name);
        return;
	}

    if(skill_table[sn].rating[ch->clss]<1){
        ch->printlnf( "`cYou can't practice %s.`x", skill_table[sn].name);
        return;
    }

	// check for no prac flag on skill
	if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_PRAC)){
		ch->printlnf( "`c%s can't be practiced here.`x", skill_table[sn].name);
		return;
	}	
	
	maxprac=skill_table[sn].maxprac_percent[ch->clss];
	adept=maxprac;
	if(adept==0){
		if(HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
			adept=50;
		}else{
			adept=class_table[ch->clss].skill_adept;
		}
	}

	// messages when greater than adept
    if ( ch->pcdata->learned[sn] >= adept )
    {
		if(skill_table[sn].get_maxprac(ch)){
			ch->printlnf( "`cYou can't practice %s any further than you already have.`x",
				skill_table[sn].name );
		}else{
			ch->printlnf( "`cYou are already learned at %s.`x",
				skill_table[sn].name );
		}
		return;
    }

	// do the prac
    ch->practice--;
	// Use the new system if appropriate
	if(IS_SET(skill_table[sn].flags,SKFLAGS_NEW_IMPROVE_SYSTEM)
		|| HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
		int pracamount= ((ch->modifiers[STAT_ME]
					   +ch->modifiers[STAT_RE]
					   +(ch->modifiers[STAT_IN]/2)
					   +30)*4)/skill_table[sn].rating[ch->clss];

		act( "`c$n receives lessons from $N.`x",
			ch, NULL, mob, TO_ROOM );
		
		int old_ability=ch->get_display_skill(sn);
		new_practice(ch, sn, pracamount); // calls itself recursively

		if(old_ability==ch->pcdata->learned[sn]){
			ch->printlnf( "`cYou attempt to practice %s.`x", skill_table[sn].name);
			ch->printlnf( "`cYou failed to learn anything about %s you didn't already know.`x",
				skill_table[sn].name);
		}else{
			if(ch->get_display_skill(sn)>=maxprac){
				if(ch->get_display_skill(sn)>skill_table[sn].get_maxprac(ch)){
					ch->wraplnf("`c%s instructs you that %s has taught you all you can`x "
						"`clearn from another about %s, the rest you will need to teach`x "
						"`cby yourself with use.`x",
						CPERS(mob,ch), he_she[URANGE(0, mob->sex , 2)], skill_table[sn].name);
				}else{
					ch->wraplnf("`c%s informs you that you have been`x "
						"`ctaught all you learn about %s.`x",
						CPERS(mob,ch), skill_table[sn].name);
				}
			}else{
				ch->printlnf("`c%s gives you lessons in %s.`x",
					PERS(mob,ch), skill_table[sn].name);
			}
			ch->printlnf("`cYour `C%s`c is now at `C%d%%`c (%s max prac is %d%%, %s learn scale is %d%%).`x", 
				skill_table[sn].name, ch->pcdata->learned[sn],
				class_table[ch->clss].name, skill_table[sn].get_maxprac(ch),
				class_table[ch->clss].name, skill_table[sn].get_learnscale(ch));
		}
		ch->printlnf( "`cYou now have `C%d`c practice session%s left.`x",
			ch->practice, ch->practice==1?"":"s");
		return;
	}

	// use the old system
    ch->pcdata->learned[sn] +=
        (ch->modifiers[STAT_ME]+ch->modifiers[STAT_RE]+30) /
		skill_table[sn].rating[ch->clss];
    if ( ch->pcdata->learned[sn] < adept )
	{
        act( "`cYou practice $T.`x",
            ch, NULL, skill_table[sn].name, TO_CHAR );
		ch->printlnf( "`cYour `C%s`c is now at `C%d%%`c.`x",
			skill_table[sn].name, ch->pcdata->learned[sn]);
		act( "`c$n practices $T.`x",
			ch, NULL, skill_table[sn].name, TO_ROOM );
    }
    else
	{
        ch->pcdata->learned[sn] = adept;
        act( "`cYou are now learned at $T.`x",
            ch, NULL, skill_table[sn].name, TO_CHAR );
        act( "`c$n is now learned at $T.`x",
            ch, NULL, skill_table[sn].name, TO_ROOM );
    }
	ch->printlnf( "`cYou now have `C%d `cpractice session%s left.`x",
		ch->practice, ch->practice==1?"":"s");
}
 
/**************************************************************************/
// 'Wimpy' originally by Dionysos.
void do_wimpy( char_data *ch, char *argument )
{
	char arg[MIL];
	int wimpy;

	one_argument( argument, arg );

	if(IS_NULLSTR(arg)){
		wimpy = ch->max_hit / 5;
    }else{
		wimpy = atoi( arg );
	}
	
	if ( wimpy < 0 )
	{
		ch->println("`cYour courage exceeds your wisdom.`x");
		return;
	}

	if ( wimpy > ch->max_hit/2 )
	{
		ch->println("`cSuch cowardice ill becomes you.`x");
		return;
	}

	ch->wimpy	= wimpy;
    ch->printlnf( "`cWimpy set to `C%d`c hit points.`x", wimpy );
	if(IS_NULLSTR(arg)){
		ch->println( "`cNote: You can set your wimpy to a specific number of hitpoints - use wimpy <number>.`x");
	}
}

/**************************************************************************/
void do_panic( char_data *ch, char *argument )
{
	char arg[MIL];
	int panic;

	argument=one_argument( argument, arg );
	
	if(IS_NPC(ch))
		return;

	if ( arg[0] == '\0' )
		panic = ch->max_hit / 10;
	else if(is_number(arg))
	{
		panic = atoi( arg );
	}
	else
	{
		ch->println("`cYou must specify a number.`x");
		return;
	}

    if ( panic < 0 )
	{
		ch->println("`cYou courage exceeds your wisdom.`x");
		return;
	}

	if ( panic > ch->max_hit/5 )
	{
		ch->println("`cSuch cowerdice is not possible in this realm.`x");
		return;
	}

	ch->pcdata->panic   = panic;
	ch->printlnf( "`cYou will `CPANIC`c at `C%d`c hit points.`x", panic );
	return;
}

/**************************************************************************/
void do_password( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char *pArg;
    char *pwdnew;
	 char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	{
		ch->println("Players only.");	
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println("`cNot going to happen.`x");
		return;
	}

    /*
	 * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( is_space(*argument) )
	{
		argument++;
	}

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	{
		cEnd = *argument++;
	}

    while ( *argument != '\0' )
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( is_space(*argument) )
	{
		argument++;
	}

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	{
		cEnd = *argument++;
	}

	while ( *argument != '\0' )
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
	*pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
		ch->println("`cSyntax: password <old> <new>.`x");
		return;
    }

	if (!(ch->pcdata->overwrite_pwd && !str_cmp( "-", arg1)))
	{
		if (str_cmp( "-", ch->pcdata->pwd ) 
			&& !is_valid_password(arg1, ch->pcdata->pwd, ch->desc))			
		{
			WAIT_STATE( ch, 40 );
			ch->println("`cWrong password.  Wait 10 seconds.`x");
			return;
		}
	}

	if ( str_len(arg2) < 5 )
    {
		ch->println("`cNew password must be at least five characters long.`x");
		return;
	}

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = dot_crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
			ch->println( "`cNew password not acceptable, try again.`x" );
			return;
		}
	}

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
	ch->pcdata->overwrite_pwd= false;
	save_char_obj( ch );
    ch->println("Ok.");
    return;
}

/**************************************************************************/
/*void do_lag( char_data *ch, char * argument) 
{   
	if ( IS_NULLSTR(argument) )
	{
		ch->println( "`xThis command turns lag prevention on or off." );
		ch->println( "Type 'lag on' to turn lag prevention on." );
		ch->println( "Type 'lag off' to turn lag prevention off." );
		return;
	}

	if (!str_prefix(argument, "on")) 
	{
		ch->println( "Lag prevention has now been enabled." );
		return; 
	}

	if (!str_prefix(argument, "off")) 
	{
		ch->println( "Lag prevention has now been disabled." );
		return;
	}

	ch->println( "`xThis command turns lag prevention on or off." );
	ch->println( "Type 'lag on' to turn lag prevention on." );
	ch->println( "Type 'lag off' to turn lag prevention off." );
	return;
}

*/
/**************************************************************************/
void do_huh( char_data *ch, char * argument)
{
/*
#ifdef unix
	msp_to_room(MSPT_ACTION, MSP_SOUND_HUH, 0, ch, true, false );
#endif
*/
	ch->println( "`cThat doesn't seem to do anything.`x" );

	// Record the moblog command
	if( IS_NPC(ch) && IS_SET(ch->act,ACT_MOBLOG) ){
		append_timestring_to_file( MOBLOG_LOGFILE, 
			FORMATF("[%5d] in room %d do_huh() - from '%s'", 
			(ch->pIndexData?ch->pIndexData->vnum:0),
			(ch->in_room? ch->in_room->vnum:0),
			argument));
	}
    return;
}

/**************************************************************************/
void do_dawnftp( char_data *ch, char *argument )
{
	pc_data *pcdata=TRUE_CH(ch)->pcdata; // the characters pcdata we want to work on
	if(!pcdata){
		ch->println("Players can only use this command");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->titlebar("DAWNFTP OPTIONS");
		ch->println("syntax: `=Cdawnftp off`x  - dawnftp is permanately off.");
		ch->wrapln( "syntax: `=Cdawnftp auto`x - mud will use dawnftp if your dawnftp is currently connected.");
		ch->wrapln( "syntax: `=Cdawnftp on`x   - dawnftp will always be used, even if "
			"you don't have a dawnftp client connected");
		ch->printlnf("Your dawnftp preference is currently set to %s", 
			preference_word(pcdata->preference_dawnftp));
		ch->wrapln("dawnftp is a superset of the mudftp system, a mudftp client can be used "
			"on dawn based muds which have the separate mudftp port enabled.");
		ch->println("DawnFtp unlike mudftp is automatically reconnected upon hotreboots.");
		ch->println("The DawnFtp client can be downloaded from http://www.dawnoftime.org/dawnftp");
		ch->titlebar("");
		return;
	}

	PREFERENCE_TYPE pt;
	if(!str_prefix(argument, "off")){
		pt=PREF_OFF;
	}else if(!str_prefix(argument, "autosense")){
		pt=PREF_AUTOSENSE;
	}else if(!str_prefix(argument, "on")){
		pt=PREF_ON;
	}else{
		ch->printlnf("Unsupported dawnftp option '%s'", argument);
		do_dawnftp(ch,"");
		return;
	}
	if(pcdata->preference_dawnftp==pt){
		ch->printlnf("Your dawnftp preference is already set to %s", preference_word(pt));
		return;
	}

	ch->printlnf("dawnftp preference changed from %s to %s", 
		preference_word(pcdata->preference_dawnftp),
		preference_word(pt));
	pcdata->preference_dawnftp=pt;
}

/**************************************************************************/
void do_objrestrict(char_data *ch, char *)
{
	OBJ_DATA *obj;
	// check for any objects that have restrictions on them, if ch has some
	// then they can't change their objrestrict status till they remove them.
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc != WEAR_NONE)
        {
			if(obj->pIndexData && obj->pIndexData->objrestrict){
				ch->wraplnf("You can't use this command while you are equiped with "
					"objects that have the objectrestriction groups on them.  "
					"Remove the objects first (%s - '%s').", 
					obj->name, obj->short_descr);
				return;
			}
		}
	}

    if (HAS_CONFIG(ch, CONFIG_OBJRESTRICT))
    {
		ch->println( "IC object restrictions disabled." );
		REMOVE_CONFIG(ch, CONFIG_OBJRESTRICT);
    }
    else
    {
		ch->println("IC object restrictions enabled.");
		SET_CONFIG(ch, CONFIG_OBJRESTRICT);
    }
}

/**************************************************************************/
void do_quester(char_data *ch, char *argument)
{
    if (IS_NPC(ch))
		return;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println("`cNot going to happen.`x");
		return;
	}

	if(IS_SET(ch->act, PLR_QUESTER)){
		ch->println("`cYou already have your quester status enabled.`x");
		return;
	};

	if ( str_cmp("confirm", argument)) {
		if(!codehelp(ch, "do_quester_noargument", CODEHELP_ALL_BUT_PLAYERS )){
			ch->println("`cIf you want to enable your quester status type:`x");
			ch->println("  `Cquester confirm`x.");
			ch->println("`RBE WARNED: `cThe quester status once on can not be be removed.`x");
		};
		return;
	}

	SET_BIT(ch->act, PLR_QUESTER);	
	if(!codehelp(ch, "do_quester_enabled", CODEHELP_ALL_BUT_PLAYERS )){
		ch->println("`cYour quester status has been activated.  This can not be turned off.`x");
		ch->println("`cQuests that are programmed to work with only questers will recognise you now.`x");
	}
	
	save_char_obj(ch);
}

/**************************************************************************/
// Celrion - Oct 99 
void do_losepet( char_data *ch, char *argument)
{
    if(!ch->pet)
    {
        ch->println("`cYou have no pet to lose!`x");
        return;
    }
    
	if(ch->fighting)
    {
        ch->println("`cYou can't lose your pet while fighting.`x");
        return;
	}

	if (str_cmp("confirm", argument))
    {
        ch->println("`cType `Rlosepet confirm`c to get rid of your pet.`x");
		return;
	}

    REMOVE_BIT(ch->pet->act, ACT_PET);
    stop_follower(ch->pet); // removes charm and sets:
							//		ch->pet->master->pet to NULL
							//		ch->pet->master to NULL
	ch->println("`cYou have managed to lose your pet.`x");

}

/**************************************************************************/
void do_noheromsg( char_data *ch, char * )
{
	if ( HAS_CONFIG( ch, CONFIG_NOHEROMSG ))
	{
		ch->println( "Hero message turned on." );
		REMOVE_CONFIG( ch, CONFIG_NOHEROMSG );
	}
	else
	{
		ch->println( "Hero message turned off." );
		SET_CONFIG( ch, CONFIG_NOHEROMSG );
	}
}

/**************************************************************************/
void do_hide_hidden_areas( char_data *ch, char * )
{
	if ( HAS_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS))
	{
		ch->println( "Hidden areas will now be seen on the area list." );
		REMOVE_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS);
	}
	else
	{
		ch->println( "Hidden areas will no longer be seen on the area list." );
		SET_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS);
	}
}

/**************************************************************************/
void do_history( char_data *ch, char * )
{
	if(IS_NPC(ch)){
		ch->println("`cPlayers only.`x");
		return;
	}

  	ch->println("`cUse `RHLOOK`c to view another PC's history.`x" );
 	ch->println("`cEntering edit mode for you own history.`x" );
 	ch->println("`cType `R@`c to finish editing your character history.`x" );
	string_append(ch, &ch->pcdata->history);
  	return;
}

/**************************************************************************/
void do_charhistory( char_data *ch, char * argument)
{
	char_data *victim;
	char arg[MIL];

    if (!IS_IMMORTAL(ch) && !IS_NEWBIE_SUPPORT(ch))
    {
		do_huh(ch,"");
		return;	
    }

    // keep a track of what newbie support is doing 
    if (IS_NEWBIE_SUPPORT(ch)){
        append_newbie_support_log(ch, argument);
    }

	argument = one_argument( argument, arg );

	if(IS_NULLSTR(arg)){
        ch->println( "`cSyntax: charhistory <player>`x" );
        ch->println( "`cUse `RHLOOK`c to look at a players history.`c" );
        return;
	}

    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "`cThey aren't here.`x" );
        return;
    }

    if (!IS_TRUSTED(ch,MAX_LEVEL-4) && !IS_NPC(victim) 
		&& IS_LETGAINED(victim)
		&& IS_TRUSTED(victim,20))
    {
        ch->println( "`cSorry, you can't set edit the history of a player who is letgained.`x "
			"`cGet an admin immortal to do it.`x" );
        return;
    }

    if (IS_NEWBIE_SUPPORT(ch) && IS_NPC(victim)){
        ch->println( "`cSorry, you can't set the history of a mob.`x" );
        return;
    }


    if ((get_trust(victim)>= get_trust(ch))&& (ch != victim))
    {
        ch->println( "`cYou can't set/edit the history of someone a higher level or equal to you.`x" );
        return;
    }

    string_append(ch, &victim->pcdata->history);
	ch->printlnf("`cEditing the history of `C%s.`x", PERS(victim, ch));
    return;
}
/**************************************************************************/
// Kal - Jan 04
void do_classinfo( char_data *ch, char *argument )
{
	int i, race;
	race_data *pRace;
	char titlebar[MIL];	
	bool magic_antipathy=false;
	bool not_selectable=false;
	bool customization_disabled=false;
	char col;

	if(!ch->desc){
		ch->println("`cYou have to be connected to use this command.`x");
		return;
	}

	col=0;
	if(IS_NULLSTR(argument)){

		if(ch->desc->connected_state==CON_WEB_REQUEST){			
			ch->println("  `cSelect class information for one of the following citizenships:`x");
		}else{
			ch->println(" `cSyntax: classinfo <citizenship>`x");
			ch->println(" `cWhere citizenship is one of the following:`x");
		}
		for(pRace=race_list; pRace; pRace=pRace->next){
			if(!pRace->pc_race()){
				continue;
			}
			if(!pRace->creation_selectable()){
				continue;
			}
			if(IS_SET(pRace->flags, RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO)){
				continue;
			}

			if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
				&& ch->desc 
				&& ch->desc->connected_state == CON_PLAYING
				&& IS_SET(pRace->flags, RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT)
				&& pRace->remort_number > ch->remort)
			{
				continue;
			}
			if(ch->desc && ch->desc->connected_state != CON_PLAYING
				&& ch->desc->connected_state!=CON_WEB_REQUEST
				&& pRace->remort_number > ch->desc->creation_remort_number)
			{
					continue;
			}

			if(ch->desc->connected_state==CON_WEB_REQUEST){
				ch->print(
					mxp_create_tag_core(
						FORMATF("a href=\"../classinfo/%s\"", pRace->name), 
						FORMATF("     %-18s", url_encode_post_data(capitalize(pRace->name)))
						)
					);
			}else{
				ch->printf("  %-18s", pRace->name);
			}
			if(++col%3==0){
				ch->println("");
			}
		}
		if(col%3!=0){
			ch->println("");
		}
		return;
	}

	race=pcrace_lookup(argument);
	if(race==-1){
		ch->printlnf("`cClassInfo: Couldn't find any citizenship '%s'.`x", argument);
		return;
	}
	pRace=race_table[race];

	if(!IS_IMMORTAL(ch)){
		if(!pRace->creation_selectable()){
			ch->printlnf("`cClassInfo: Couldn't find any citizenship '%s'.`x", argument);
			return;
		}
		if(IS_SET(pRace->flags, RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO)){
			ch->printlnf("`cClassInfo: citizenship '%s' is not available in classinfo.`x", argument);
			return;
		}

		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
			&& ch->desc 
			&& ch->desc->connected_state == CON_PLAYING
			&& IS_SET(pRace->flags, RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT)
			&& pRace->remort_number > ch->remort)
		{
			ch->printlnf("`cClassInfo: citizenship '%s' is not available for your remort level.`x", argument);
			return;
		}
	}
	if(ch->desc && ch->desc->connected_state != CON_PLAYING
		&& ch->desc->connected_state!=CON_WEB_REQUEST
		&& pRace->remort_number > ch->desc->creation_remort_number)
	{
		ch->printlnf("`cClassInfo: citizenship '%s' is not available for your remort level.`x", argument);
		return;
	}


	// by this stage, we have the race selected and into pRace

	ch->titlebarf("CLASSINFO RELATING TO THE %s CITIZENS", uppercase(pRace->name));
	sprintf(titlebar, "`=t-=%s`=t===`=Tclass name`=t====`=Tbase xp`=t===="
		"`=Tprime attribute 1`=t==`=Tprime attribute 2`=t====-`x",
		(GAMESETTING(GAMESET_REMORT_SUPPORTED)?"`=Tremort":"====="));	
	ch->println(titlebar);

	col=0;
	for ( i= 0; !IS_NULLSTR(class_table[i].name); i++)
	{	
		class_type *cl;
		cl=&class_table[i];
		bool magic_flag=false;
		bool customization_flag=false;
		bool unselectable_flag=false;

		if(!IS_IMMORTAL(ch)){
			if(!cl->creation_selectable){
				continue;
			}
			if(pRace->class_exp[i]<1000){
				continue;
			}

			if(IS_SET(cl->flags, CLASSFLAG_ALWAYS_HIDDEN_FROM_MORTAL_CLASSINFO)){
				continue;
			}

			if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
				&& ch->desc 
				&& ch->desc->connected_state == CON_PLAYING
				&& IS_SET(cl->flags, CLASSFLAG_HIDDEN_FROM_MORTAL_CLASSINFO_WHEN_ABOVE_THEIR_REMORT)
				&& cl->remort_number > ch->remort)
			{
				continue;
			}			
		}

		if(ch->desc && ch->desc->connected_state != CON_PLAYING){
			if(cl->remort_number > ch->desc->creation_remort_number)
				continue;
		}


		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)){
			ch->printf("    `s%d",cl->remort_number);
		}else{
			ch->print( "     ");
		}

		if(pRace->class_exp[i]<1000){
			unselectable_flag=true;
			not_selectable=true;			
		}
		
		if(!GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION) 
			&& IS_SET(cl->flags, CLASSFLAG_MAGIC_ANTIPATHY))
		{
			magic_antipathy=true;
			magic_flag=true;
		}
		if(!GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION) 
			&& IS_SET(cl->flags, CLASSFLAG_NO_CUSTOMIZATION))
		{
			customization_disabled=true;
			customization_flag=true;
		}
		
		ch->printlnf("`x%16s%s%s%s%s%s%6d       %-16s   %-16s", 
			capitalize(cl->name),
			unselectable_flag?"`Y#`x":"",
			magic_flag?"`Y*`x":"",
			customization_flag?"`Y-`x":" ",
			unselectable_flag?"":" ", // put the space which we skipped above
			magic_flag?"":" ", // put the space which we skipped above
			pRace->class_exp[i],
			capitalize(stat_flags[cl->attr_prime[0]].name),
			capitalize(stat_flags[cl->attr_prime[1]].name)
			);
	}
	if(not_selectable){
		ch->println("  `Y#`x = class can't be selected in creation (base xp<1000).");
	}
	if(magic_antipathy ){
		ch->println("  `Y*`x = this class has no god powers.");
	}
	if(customization_disabled ){
		ch->println("  `Y-`x = this class has no advanced customization available.");
	}
	ch->println("  `cNote: The prime attributes are the same for every class/citizenship combination.`x");
	ch->titlebar("");
	

	if(ch->desc->connected_state==CON_WEB_REQUEST){
		ch->print_blank_lines(1);
		do_classinfo(ch, "");
	}

}
/**************************************************************************/
void do_flags( char_data *ch, char * )
{
		if(GAMESETTING(GAMESET2_ATHENSFLAG_ATHENS)){
			ch->wrapln("`cThe Athens City Flag belongs to `CAthens`c.`x");}
		if(GAMESETTING(GAMESET2_ATHENSFLAG_MEGARA)){
			ch->wrapln("`cThe Athens City Flag belongs to `CMegara`c.`x");}
		if(GAMESETTING(GAMESET2_MEGARAFLAG_ATHENS)){
			ch->wrapln("`cThe Megara City Flag belongs to `CAthens`c.`x");}
		if(GAMESETTING(GAMESET2_MEGARAFLAG_MEGARA)){
			ch->wrapln("`cThe Megara City Flag belongs to `CMegara`c.`x");}

}
/**************************************************************************/
/**************************************************************************/

