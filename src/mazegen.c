/*
 * MazeGen.c -- Mark Howell -- 8 May 1991
 *
 * Usage: MazeGen vnum [width [height [seed]]]
 */
#include "include.h"
#include <time.h>

char *fwrite_flag( long flags, char buf[] );

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

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

    stack = (maze_t *) mud_calloc (width * height, sizeof (maze_t));
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


void PrintMaze (FILE *fp, maze_t maze, int width, int height)
{
    int w, h;
    char *line, *lp;

    line = (char *) mud_calloc ((width + 1) * 2, sizeof (char));
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
    (void) fputs (line, fp);
    (void) fputs ("\n", fp);
    for (h = 0; h < height; h++) {
        for (lp = line, w = 0; w < width; w++) {
            if ((maze + w)->left)
                *lp++ = ((maze + w)->path && (maze + w - 1)->path) ? '.' : ' ';
            else
                *lp++ = '|';
            *lp++ = ((maze + w)->path) ? '.' : ' ';
        }
        *lp++ = '|';
        (void) fputs (line, fp);
	(void) fputs ("\n", fp);
        for (lp = line, w = 0; w < width; w++) {
            *lp++ = '+';
            if ((maze + w)->down)
                *lp++ = ((maze + w)->path && (h == height - 1 ||
                         (maze + w + width)->path)) ? '.' : ' ';
            else

                *lp++ = '-';
        }
        *lp++ = '+';
        (void) fputs (line, fp);
        (void) fputs ("\n", fp);
        maze += width;
    }
    free (line);

}/* PrintMaze */

void Maze2Area (FILE *fp, maze_t maze, int width, int height, int startvnum)
{
    int w, h;
	int vnum=0;
	int num_exit;

	fprintf(fp, "#ROOMS\n");

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
			fprintf(fp, "#%d\n", startvnum+vnum);
			switch (num_exit)
			{
				case 0 :
					fprintf(fp, "Perdido en el Laberinto~\n");
					fprintf(fp, "Estas perdido en el laberinto de retorcidos pasajes sin tener donde ir.\n~\n");
					break;
				case 1 :
					fprintf(fp, "Sin salida~\n");
					fprintf(fp, "#B#FJAJAJAJAJA#b#f!.  Ahora tienes que volver!\n~\n");
					break;
				case 2 :
					fprintf(fp, "Tunel oscuro~\n");
					fprintf(fp, "Estas en un laberinto de pasajes oscuros, todos parecidos.\n~\n");
					break;
				case 3:
					fprintf(fp, "Pasaje oscuro~\n");
					fprintf(fp, "Estas en un laberinto de pasajes oscuros, todos parecidos.\n~\n");
					break;
				case 4 :
					fprintf(fp, "Cuarto oscuro~\n");
					fprintf(fp, "Estas en un laberinto de pasajes oscuros, todos parecidos.\n~\n");
					break;
			}
			fprintf(fp, "0 %d 0\n", ROOM_NO_RECALL|ROOM_NOWHERE|ROOM_INDOORS);
			if ((maze + w)->up)
				fprintf(fp, "D0\n~\n~\n0 0 %d\n", startvnum + vnum - width );
			if ((maze + w)->right)
				fprintf(fp, "D1\n~\n~\n0 0 %d\n", startvnum + vnum + 1 );
			if ((maze + w)->down)
				fprintf(fp, "D2\n~\n~\n0 0 %d\n", startvnum + vnum + width );
			if ((maze + w)->left)
				fprintf(fp, "D3\n~\n~\n0 0 %d\n", startvnum + vnum - 1 );
			fprintf(fp, "S\n");
			++vnum;
        }
        maze += width;
    }
	fprintf(fp, "#0\n\n");
	fprintf(fp, "#RESETS\n");

	fprintf(fp, "S\n\n");
}/* Maze2Area */

void crear_laberinto (FILE *fp, int startvnum, int width, int height)
{
	maze_t maze;
	char buf[MIL];

	srand ((int) time ((time_t *) NULL));

	if (width <= 0 || height <= 0)
	{
		bug( "Crear_laberinto : Ancho o largo ilegal!", 0 );
	        exit (EXIT_FAILURE);
	}

	maze = (maze_t) mud_calloc (width * height, sizeof (cell_t));
	if (maze == NULL)
	{
		bug( "Crear_laberinto : no se pudo asignar memoria!", 0 );
        	exit (EXIT_FAILURE);
	}

	fprintf(fp, "#AREADATA\n");
	fprintf(fp, "Name Laberinto~\n");
	fprintf(fp, "Builders None~\n");
	fprintf(fp, "Security 9\n");
	fprintf(fp, "VNUMs %d %d\n", startvnum, startvnum+(width*height) );
	fprintf(fp, "Prototipo 0\n");
	fprintf(fp, "Low 5\n");
	fprintf(fp, "High 60\n");
	fprintf(fp, "Credits { 50+ } Birdie  Laberinto~\n");
	fprintf(fp, "Recalc 1\n");
	fprintf(fp, "End\n\n");

	CreateMaze (maze, width, height);

	fprintf(fp, "#OBJECTS\n");
	fprintf(fp, "#%d\n", startvnum );
	fprintf(fp, "mapa laberinto~\n");
	fprintf(fp, "mapa del laberinto~\n");
	fprintf(fp, "Un mapa del laberinto.~\n");
	fprintf(fp, "paper~\n");
	fprintf(fp, "map 0 A\n");
	fprintf(fp, "0 0 0 0 0\n");
	fprintf(fp, "0 1500 0 P\n");
	fprintf(fp, "E\n");
	fprintf(fp, "mapa~\n");
	PrintMaze (fp, maze, width, height);
	fprintf(fp, "~\n");

	SolveMaze (maze, width, height);

	fprintf(fp, "E\n");
	fprintf(fp, "solucion~\n");
	PrintMaze (fp, maze, width, height);
	fprintf(fp, "~\n");
	fprintf(fp, "#0\n\n");

	/* Make the mob 	*/

	fprintf(fp, "#MOBILES\n");
	fprintf(fp, "#%d\n", startvnum );
	fprintf(fp, "guerrero laberinto~\n");
	fprintf(fp, "~\n");
	fprintf(fp, "Un guerrero da vueltas por el lugar, con los ojos desorbitados.\n~\n");
	fprintf(fp, "Parece llevar anos tratando de salir de aqui.\n~\n");

	fprintf(fp, "human~\n");
	fprintf(fp, "%s 40 0 1000 0\n", fwrite_flag( ACT_IS_NPC|ACT_SENTINEL|ACT_STAY_AREA|ACT_SCAVENGER|ACT_AGGRESSIVE, buf ) );
	fprintf(fp, "50 40 4d175+1400 3d3+33 5d5+10 bite\n");
	fprintf(fp, "0 0 0 0\n");
	fprintf(fp, "0 0 0 0\n");
	fprintf(fp, "stand stand male 0\n");
	fprintf(fp, "0 0 large carne\n");
	fprintf(fp, "#0\n\n");

	/* now generate the rooms */
	Maze2Area (fp, maze, width, height, startvnum);

	fprintf(fp, "#SPECIALS\n" );
	fprintf(fp, "M %d spec_rat\n", startvnum );
	fprintf(fp, "S\n\n" );

	/* And the trailer		*/
	fprintf(fp, "#$\n");

	free (maze);
/*	exit (EXIT_SUCCESS); */

	return;
}/* main */
