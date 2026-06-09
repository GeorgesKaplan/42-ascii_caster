#ifndef FT_ASCII_CASTER_H
#define FT_ASCII_CASTER_H

#include <unistd.h>
#include <stdlib.h>

typedef struct s_map
{
	char **grid;
	int rows;
	int cols;
	int player_x;
	int player_y;
	char player_dir;
}               t_map;

/* map parser */
t_map *parse_map(const char *path);
void free_map(t_map *map);

/* renderer */
#define RENDER_DISTANCE 0
#define RENDER_ORIENTATION 1

void render_loop(t_map *map, int render_mode);

int validate_args(int argc, char **argv);
int has_map_extension(const char *path);

#endif
