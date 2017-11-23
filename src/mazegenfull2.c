/*
 * MazeGen.c -- Mark Howell -- 8 May 1991
 * modified 1997/04/16 to generate Envy 1.0 area files.
 *
 * Usage: MazeGen [width [height [seed]]]
 *
 * Outputs the area file to stdout.  Also makes two very useful
 * objects, a map of the maze and a solution to the maze.  This
 * algorithm can be adapted to dynamically generated mazes.  You
 * may wish to prevent the maze from changing if players are actually
 * in the maze at the time or you may wind up trapping them in an
 * unreachable portion.
 *
 * There are different room descriptions based on the number of
 * exits in the room.  Feel free to change them in the source code
 * or edit the resulting output file.  Also, you may want to modify
 * the number of different mobs and mob placement.  Current algorithm
 * it to put one mob in each room.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 39
#define HEIGHT 11

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#ifdef TRUE
#undef TRUE
#endif /* TRUE */

#define TRUE 1

#define cell_empty(a) (!(a)->up && !(a)->right && !(a)->down && !(a)->left)

typedef struct {
    unsigned int up      : 1;
    unsigned int right   : 1;
    unsigned int down    : 1;
    unsigned int left    : 1;
    unsigned int path    : 1;
    unsigned int visited : 1;
} cell_t;
typedef cell_t *maze_t;

void CreateMaze (maze_t maze, int width, int height)
{
    maze_t mp, maze_top;
    char paths [4];
    int visits, directions;

    visits = width * height - 1;
    mp = maze;
    maze_top = mp + (width * height) - 1;

    while (visits) {
        directions = 0;

        if ((mp - width) >= maze && cell_empty (mp - width))
            paths [directions++] = UP;
        if (mp < maze_top && ((mp - maze + 1) % width) && cell_empty (mp + 1))
            paths [directions++] = RIGHT;
        if ((mp + width) <= maze_top && cell_empty (mp + width))
            paths [directions++] = DOWN;
        if (mp > maze && ((mp - maze) % width) && cell_empty (mp - 1))
            paths [directions++] = LEFT;

        if (directions) {
            visits--;
            directions = ((unsigned) rand () % directions);

            switch (paths [directions]) {
                case UP:
                    mp->up = TRUE;
                    (mp -= width)->down = TRUE;
                    break;
                case RIGHT:
                    mp->right = TRUE;
                    (++mp)->left = TRUE;
                    break;
                case DOWN:
                    mp->down = TRUE;
                    (mp += width)->up = TRUE;
                    break;
                case LEFT:
                    mp->left = TRUE;
                    (--mp)->right = TRUE;
                    break;
                default:
                    break;
            }
        } else {
            do {
                if (++mp > maze_top)
                    mp = maze;
            } while (cell_empty (mp));
        }
    }
}/* CreateMaze */


void SolveMaze (maze_t maze, int width, int height)
{
    maze_t *stack, mp = maze;
    int sp = 0;

    stack = (maze_t *) calloc (width * height, sizeof (maze_t));
    if (stack == NULL) {
        (void) fprintf (stderr, "Cannot allocate memory!\n");
        exit (EXIT_FAILURE);
    }
    (stack [sp++] = mp)->visited = TRUE;

    while (mp != (maze + (width * height) - 1)) {

        if (mp->up && !(mp - width)->visited)
            stack [sp++] = mp - width;
        if (mp->right && !(mp + 1)->visited)
            stack [sp++] = mp + 1;
        if (mp->down && !(mp + width)->visited)
            stack [sp++] = mp + width;
        if (mp->left && !(mp - 1)->visited)
            stack [sp++] = mp - 1;

        if (stack [sp - 1] == mp)
            --sp;

        (mp = stack [sp - 1])->visited = TRUE;
    }
    while (sp--)
        if (stack [sp]->visited)
            stack [sp]->path = TRUE;

    free (stack);

}/* SolveMaze */


void PrintMaze (maze_t maze, int width, int height)
{
    int w, h;
    char *line, *lp;

    line = (char *) calloc ((width + 1) * 2, sizeof (char));
    if (line == NULL) {
        (void) fprintf (stderr, "Cannot allocate memory!\n");
        exit (EXIT_FAILURE);
    }
    maze->up = TRUE;
    (maze + (width * height) - 1)->down = TRUE;

    for (lp = line, w = 0; w < width; w++) {
        *lp++ = '+';
        if ((maze + w)->up)
            *lp++ = ((maze + w)->path) ? '.' : ' ';
        else
            *lp++ = '-';
    }
    *lp++ = '+';
    (void) puts (line);
    for (h = 0; h < height; h++) {
        for (lp = line, w = 0; w < width; w++) {
            if ((maze + w)->left)
                *lp++ = ((maze + w)->path && (maze + w - 1)->path) ? '.' : ' ';
            else
                *lp++ = '|';
            *lp++ = ((maze + w)->path) ? '.' : ' ';
        }
        *lp++ = '|';
        (void) puts (line);
        for (lp = line, w = 0; w < width; w++) {
            *lp++ = '+';
            if ((maze + w)->down)
                *lp++ = ((maze + w)->path && (h == height - 1 ||
                         (maze + w + width)->path)) ? '.' : ' ';
            else

                *lp++ = '-';
        }
        *lp++ = '+';
        (void) puts (line);
        maze += width;
    }
    free (line);

}/* PrintMaze */

void Maze2Area (maze_t maze, int width, int height)
{
    int w, h;
	int vnum=0;
	int num_exit;

	printf("#ROOMS\n\n");

    maze->up = 0;
    (maze + (width * height) - 1)->down = 0;

    for (h = 0; h < height; h++) 
	{
        for (w = 0; w < width; w++) 
		{
			num_exit = 0;
			if ((maze + w)->up)
				++num_exit;
			if ((maze + w)->right)
				++num_exit;
			if ((maze + w)->down)
				++num_exit;
			if ((maze + w)->left)
				++num_exit;
			printf("#QQ%3.3d\n", vnum);
			switch (num_exit)
			{
				case 0 :
					printf("Labyrinth of the Minotaur~\n");
					printf("The labyrinth of the minotaur stretches out ominously.\n~\n");
					break;
				case 1 :
					printf("Dead end in the Labyrinth~\n");
					printf("The labryinth of the minotaur stretches out ominously. Perhaps another route would be more beneficial.\n~\n");
					break;
				case 2 :
					printf("Labyrinth of the Minotaur~\n");
					printf("The labyrinth of the minotaur stretches out ominously and does not seem to have an exit.\n~\n");
					break;
				case 3:
					printf("Labyrinth of the Minotaur       ~\n");
					printf("The labyrinth of the minoatur stretches out ominiously.\n~\n");
					break;
				case 4 :
					printf("Labyrinth of the Minotaur~\n");
					printf("The labyrinth of the minoatur stretches out ominiously.\n~\n");
					break;
			}
			printf("0 524608 0\n");
			if ((maze + w)->up)
				printf("D0\n~\n~\n0 0 QQ%3.3d\n", vnum - width );
			if ((maze + w)->right)
				printf("D1\n~\n~\n0 0 QQ%3.3d\n", vnum + 1 );
			if ((maze + w)->down)
				printf("D2\n~\n~\n0 0 QQ%3.3d\n", vnum + width );
			if ((maze + w)->left)
				printf("D3\n~\n~\n0 0 QQ%3.3d\n", vnum - 1 );
			printf("S\n");
			++vnum;
        }
        maze += width;
    }
	printf("#0\n\n");
	printf("#RESETS\n\n");

    for (vnum = 0; vnum < width * height; vnum++) 
	{
		printf("M 0 QQ000 %d QQ%3.3d\n", width*height, vnum);
	}

	printf("S\n\n");
	printf("#$\n\n");


}/* PrintMaze */


void do_mazecode (int argc, char *argv [])
{
    int width = WIDTH;
    int height = HEIGHT;
    maze_t maze;

    if (argc >= 2)
        width = atoi (argv [1]);

    if (argc >= 3)
        height = atoi (argv [2]);

    if (argc >= 4)
        srand (atoi (argv [3]));
    else
        srand ((int) time ((time_t *) NULL));

    if (width <= 0 || height <= 0) {
        (void) fprintf (stderr, "Illegal width or height value!\n");
        exit (EXIT_FAILURE);
    }
    maze = (maze_t) calloc (width * height, sizeof (cell_t));
    if (maze == NULL) {
        (void) fprintf (stderr, "Cannot allocate memory!\n");
        exit (EXIT_FAILURE);
    }

	printf("#AREA	{ 5 50} Envy   Terrible Maze~\n\n");


    CreateMaze (maze, width, height);

	printf("#OBJECTS\n\n");

	printf("#QQ000\n");
	printf("map maze~\n");
	printf("a map of the labyrinth.~\n");
	printf("A map of the labyrinth.~\n");
	printf("~\n");
	printf("12 0 1\n");
	printf("0~ 0~ 0~ 0~\n");
	printf("0 1500 0\n");

	printf("E\n");
	printf("map~\n");
    PrintMaze (maze, width, height);
	printf("~\n");

    SolveMaze (maze, width, height);

	printf("E\n");
	printf("solution~\n");
    PrintMaze (maze, width, height);
	printf("~\n");

	printf("#0\n\n");

	/* Make the mob 	*/

	printf("#MOBILES\n\n");
	printf("#QQ000\n");
	printf("large rat~\n");
	printf("a large rat~\n");
	printf("A large rat, fat from feasting on the lost stands here.\n~\n");
	printf("This rat is quite large and has had many large feasts from those who have become lost in the labyrinth.\n~\n");

	printf("1 40 1000 S\n");
	printf("50 40 -100 4d175+1400 3d3+33\n");
	printf("500 0\n");
	printf("18 28 1\n");
	printf("#0\n");

	/* now generate the rooms */

    Maze2Area (maze, width, height);

	/* And the trailer		*/

	printf("#$\n");

    free (maze);
    exit (EXIT_SUCCESS);

    return;

}/* main */
