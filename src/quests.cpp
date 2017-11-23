#include "include.h"
#include "channels.h"
#include "msp.h"

void do_quests(char_data *ch, char *argument)
{
	ch->printlnf("`c~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`x");
	ch->printlnf("`CCONTESTS AND GROUP QUESTS:");
	ch->printlnf("`c~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`x");
	ch->printlnf("\r\n`c   4 am -  Hunt for the Greased Pig");
	ch->printlnf("\r\n`c   6 am -  Athens and Megara invade each other");
	ch->printlnf("\r\n`c   8 am -  Apple-Picking Starts");
	ch->printlnf("\r\n`c  12 pm -  Apple-Picking Ends");
	ch->printlnf("\r\n`c   4 pm -  Missing Laurel Leaves Quest");
	ch->printlnf("\r\n`c   6 pm -  Knossos invades Athens and Megara");
	ch->printlnf("\r\n`c  12 am -  Double Experience Gains");
	ch->printlnf("\r\n`c~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`x");
	return;
}
