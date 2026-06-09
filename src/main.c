#include "../include/ft_ascii_caster.h"
#include <stdio.h>
#include <string.h>

int validate_args(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./ft_ascii_caster <map_file>.map\n");
        return 0;
    }
    if (!has_map_extension(argv[1]))
    {
        fprintf(stderr, "Error: map file must end with .map\n");
        return 0;
    }
    return 1;
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

    t_map *map = parse_map(argv[1]);
    if (!map)
        return 1;

    render_loop(map);
    free_map(map);
    return 0;
}
