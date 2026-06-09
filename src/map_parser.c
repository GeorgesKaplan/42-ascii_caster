#include "../include/ft_ascii_caster.h"
#include <stdio.h>

// Placeholder for the map parser. Implement strict validation later.

int parse_map(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("fopen");
        return 0;
    }
    // TODO: read and validate the map fully.
    fclose(f);
    return 1;
}
