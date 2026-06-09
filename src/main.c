#include "../include/ft_ascii_caster.h"
#include <stdio.h>
#include <string.h>

int validate_args(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Usage: ./ft_ascii_caster [--orientation|-o] <map_file>.map\n");
        return 0;
    }

    const char *map_path = argv[argc - 1];
    if (!has_map_extension(map_path))
    {
        fprintf(stderr, "Error: map file must end with .map\n");
        return 0;
    }
    return 1;
}

int get_render_mode(int argc, char **argv)
{
    for (int i = 1; i < argc - 1; ++i)
    {
        if ((strcmp(argv[i], "--orientation") == 0) || (strcmp(argv[i], "-o") == 0))
            return RENDER_ORIENTATION;
    }
    return RENDER_DISTANCE;
}

const char *get_map_path(int argc, char **argv)
{
    return argv[argc - 1];
}

int has_map_extension(const char *path)
{
    const char *ext = ".map";
    size_t plen = strlen(path);
    size_t elen = strlen(ext);
    if (plen < elen)
        return 0;
    return (strcmp(path + plen - elen, ext) == 0);
}

int main(int argc, char **argv)
{
    if (!validate_args(argc, argv))
        return 1;

    int render_mode = get_render_mode(argc, argv);
    const char *map_path = get_map_path(argc, argv);

    t_map *map = parse_map(map_path);
    if (!map)
        return 1;

    render_loop(map, render_mode);
    free_map(map);
    return 0;
}
