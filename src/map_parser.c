#include "../include/ft_ascii_caster.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static void write_error_and_free(const char *msg, char *buf, char **lines, t_map *map)
{
    if (msg)
        write(2, "Error: ", 7), write(2, msg, strlen(msg)), write(2, "\n", 1);
    if (buf)
        free(buf);
    if (lines)
    {
        for (int i = 0; lines[i]; ++i)
            free(lines[i]);
        free(lines);
    }
    if (map)
        free_map(map);
}

static char *dup_range(const char *src, int start, int len)
{
    char *s = malloc(len + 1);
    if (!s)
        return NULL;
    for (int i = 0; i < len; ++i)
        s[i] = src[start + i];
    s[len] = '\0';
    return s;
}

t_map *parse_map(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return NULL;
    }

    size_t cap = 4096;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf)
    {
        close(fd);
        return NULL;
    }

    ssize_t r;
    while ((r = read(fd, buf + len, cap - len)) > 0)
    {
        len += (size_t)r;
        if (cap - len == 0)
        {
            size_t ncap = cap * 2;
            char *n = malloc(ncap);
            if (!n)
            {
                free(buf);
                close(fd);
                return NULL;
            }
            for (size_t i = 0; i < len; ++i)
                n[i] = buf[i];
            free(buf);
            buf = n;
            cap = ncap;
        }
    }
    if (r < 0)
    {
        perror("read");
        free(buf);
        close(fd);
        return NULL;
    }
    close(fd);

    if (len == 0)
    {
        write(2, "Error: empty map\n", strlen("Error: empty map\n"));
        free(buf);
        return NULL;
    }

    /* count lines */
    int lines_count = 0;
    for (size_t i = 0; i < len; ++i)
        if (buf[i] == '\n')
            lines_count++;
    /* account for last line if no trailing newline */
    if (buf[len - 1] != '\n')
        lines_count++;

    /* split into lines */
    char **lines = malloc((lines_count + 1) * sizeof(char*));
    if (!lines)
    {
        free(buf);
        return NULL;
    }
    int li = 0;
    size_t start = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (buf[i] == '\n')
        {
            int l = (int)(i - start);
            if (l == 0)
            {
                write(2, "Error: empty line in map\n", strlen("Error: empty line in map\n"));
                write_error_and_free(NULL, buf, lines, NULL);
                return NULL;
            }
            lines[li++] = dup_range(buf, (int)start, l);
            start = i + 1;
        }
    }
    if (start < len)
    {
        int l = (int)(len - start);
        if (l == 0)
        {
            write(2, "Error: empty line in map\n", strlen("Error: empty line in map\n"));
            write_error_and_free(NULL, buf, lines, NULL);
            return NULL;
        }
        lines[li++] = dup_range(buf, (int)start, l);
    }
    lines[li] = NULL;

    /* ensure rectangular and validate chars */
    int rows = li;
    int cols = (int)strlen(lines[0]);
    for (int y = 0; y < rows; ++y)
    {
        if ((int)strlen(lines[y]) != cols)
        {
            write(2, "Error: map must be rectangular\n", strlen("Error: map must be rectangular\n"));
            write_error_and_free(NULL, buf, lines, NULL);
            return NULL;
        }
        if (cols == 0)
        {
            write(2, "Error: empty map line\n", strlen("Error: empty map line\n"));
            write_error_and_free(NULL, buf, lines, NULL);
            return NULL;
        }
    }

    /* validate edges and characters, find player */
    int player_count = 0;
    t_map *map = malloc(sizeof(t_map));
    if (!map)
    {
        write_error_and_free(NULL, buf, lines, NULL);
        return NULL;
    }
    map->rows = rows;
    map->cols = cols;
    map->grid = malloc(rows * sizeof(char*));
    if (!map->grid)
    {
        write_error_and_free(NULL, buf, lines, map);
        return NULL;
    }

    for (int y = 0; y < rows; ++y)
    {
        char *line = lines[y];
        /* first and last columns must be '1' */
        if (line[0] != '1' || line[cols - 1] != '1')
        {
            write(2, "Error: map not closed by walls\n", strlen("Error: map not closed by walls\n"));
            write_error_and_free(NULL, buf, lines, map);
            return NULL;
        }
        for (int x = 0; x < cols; ++x)
        {
            char c = line[x];
            if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
            {
                player_count++;
                map->player_x = x;
                map->player_y = y;
                map->player_dir = c;
            }
            else if (c == '0' || c == '1')
            {
                /* ok */
            }
            else
            {
                write(2, "Error: invalid character in map\n", strlen("Error: invalid character in map\n"));
                write_error_and_free(NULL, buf, lines, map);
                return NULL;
            }
        }
        map->grid[y] = dup_range(line, 0, cols);
    }

    if (player_count != 1)
    {
        write(2, "Error: map must contain exactly one player\n", strlen("Error: map must contain exactly one player\n"));
        write_error_and_free(NULL, buf, lines, map);
        return NULL;
    }

    /* top and bottom rows must be all '1' */
    for (int x = 0; x < cols; ++x)
    {
        if (lines[0][x] != '1' || lines[rows - 1][x] != '1')
        {
            write(2, "Error: map not closed by walls\n", strlen("Error: map not closed by walls\n"));
            write_error_and_free(NULL, buf, lines, map);
            return NULL;
        }
    }

    /* cleanup */
    for (int i = 0; i < rows; ++i)
        free(lines[i]);
    free(lines);
    free(buf);

    return map;
}

void free_map(t_map *map)
{
    if (!map)
        return;
    if (map->grid)
    {
        for (int i = 0; i < map->rows; ++i)
            free(map->grid[i]);
        free(map->grid);
    }
    free(map);
}
