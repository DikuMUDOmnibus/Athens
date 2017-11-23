/************************************************************************
* Bounty.cpp written for Athen's Mud by Zeus of Athen's Mud		*
* This code is based off of the numerous double experience snippets 	*
* available.								*
************************************************************************/

#include "include.h"

void do_bounty(char_data *ch, char *argument)
{
        int amount;
        int obounty;
        char arg1[MSL];
        char arg2[MSL];
        char_data *vch;
	int bountycount = 0;

        argument = one_argument(argument,arg1);
        argument = one_argument(argument,arg2);


	if(!str_cmp(arg1, "show"))
	{	
		for(vch = char_list; vch != NULL; vch = vch->next)
		{
			if(vch->bounty != 0)
			{
			ch->printlnf("%s has a bounty of %d gold on their head!", vch->name, vch->bounty);
			bountycount++;
			}
		}

		if(bountycount == 0)
		{
		ch->printlnf("No one has a bounty on their head right now.");
		bountycount = 0;
		return;
		}
	return;
	}	

	if((vch = get_char_world(ch, arg1)) == NULL)
        {ch->println("`cThat character isn't online.`x");return;}


	if(!str_cmp(arg2, "clear") && IS_IMMORTAL(ch))
	{
        	if(IS_NULLSTR(arg1))
	        {ch->printlnf("Syntax Bounty <person> clear");return;}

	
	        if((vch->bounty != 0))
        	
        	{ch->printlnf("You have cleared the bounty on %s.", vch->name);
	         vch->printlnf("%s has cleared the bounty on your head.", ch->name);
	         info_broadcast(ch,"By the will of %s, the bounty on %s's head has been cleared.", ch->name, vch->name);
	         vch->bounty = 0;
        	 return;
		}
		else
		{ch->printlnf("%s does not have a bounty on their head.", vch->name);return;}
	        
	}

        if(IS_NPC(ch) || IS_NPC(vch))
        {ch->printlnf("Not on NPCs");return;}

        amount = atoi(arg2);
        if(IS_NULLSTR(arg1) || !is_number(arg2) || arg2[0] == '\0')
        {ch->printlnf("Syntax: BOUNTY (person) (amount)");
	 ch->printlnf("Syntax: BOUNTY SHOW");
	 if(IS_IMMORTAL(ch))
		{
		ch->printlnf("\n\r\t`#`C*** `RImmortal Only `C*** `^\n\rSyntax: BOUNTY (person) CLEAR");
		}
	return;}


        if(!IS_IMMORTAL(ch) && ch->gold < amount)
        {ch->printlnf("You do not have that much gold on your person.");return;}

        if(IS_IMMORTAL(vch))
        {ch->printlnf("That wasn't very bright");
         vch->printlnf("%s just tried to put a bounty of %d on you.", ch->name, amount);
         return;}

        if(ch == vch)
        {ch->printlnf("Why would you want to put a bounty on your own head?");
         info_broadcast(ch,"%s just tried to put a bounty on themselves, they must want to die!", ch->name);
        }

        	if(amount < 0)
	{
		if(amount + vch->bounty < 0)
		{ch->printlnf("You can't reduce a bounty below 0 gold.");return;}
	
		if(amount < 0 && vch->bounty > 0)
		{
		ch->printlnf("You pay %d gold towards the bounty of %d gold on %s' head.", amount, vch->bounty, vch->name);
			if(ch != vch)
			{
			obounty = vch->bounty + amount;
			vch->printlnf("%s has just paid %d gold towards the bounty of %d gold on your head. Your bounty is now at %d gold.", ch->name, amount, vch->bounty, obounty);
			}
		
			if(vch->bounty + amount == 0)
			{
			 info_broadcast(ch,"The bounty on the head of %s has been paid off!", vch->name);
			 }
		}
		 vch->bounty += amount;
		 ch->gold += amount;
		return;
	}

	if(vch->bounty != 0 && amount > 0)
	{
	vch->bounty += amount;
	vch->printlnf("%s has placed a bounty of %d gold on your head! Your bounty is now %d gold.", ch->name, amount, vch->bounty);
	ch->printlnf("You have placed a bounty of %d gold on %s's head.", amount, vch->name);
	ch->gold = ch->gold - amount;
	info_broadcast(ch,"%s has placed a bounty of %d gold on the head of %s", ch->name, amount, vch->name);
	return;
	}
	else
	{
	vch->printlnf("%s has placed a bounty of %d gold on your head!", ch->name, amount);
	ch->printlnf("You have placed a bounty of %d gold on %s's head.", amount, vch->name);
	vch->bounty += amount;
	ch->gold = ch->gold - amount;
	info_broadcast(ch,"%s has placed a bounty of %d gold on the head of %s", ch->name, amount, vch->name);
	return;
	}
}
/***********************************************************************************************************************/
