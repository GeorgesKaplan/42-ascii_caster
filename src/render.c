#include "../include/ft_ascii_caster.h"
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define MOVE_SPEED 0.12
#define ROT_SPEED 0.08
#define MINIMAP_WIDTH 15
#define MINIMAP_HEIGHT 10

static void restore_terminal(const struct termios *saved)
{
    if (saved)
        tcsetattr(0, TCSAFLUSH, saved);
}

static int enable_raw_mode(struct termios *orig)
{
    if (tcgetattr(0, orig) != 0)
        return 0;
    struct termios raw = *orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSAFLUSH, &raw) != 0)
        return 0;
    return 1;
}

static int read_input_char(void)
{
    char c;
    ssize_t n = read(0, &c, 1);
    if (n <= 0)
        return 0;
    return (int)(unsigned char)c;
}

static int get_arrow_key(void)
{
    char seq[2];
    if (read(0, seq, 1) != 1)
        return 0;
    if (read(0, seq + 1, 1) != 1)
        return 0;
    if (seq[0] != '[')
        return 0;
    if (seq[1] == 'A')
        return 'U';
    if (seq[1] == 'B')
        return 'D';
    if (seq[1] == 'C')
        return 'R';
    if (seq[1] == 'D')
        return 'L';
    return 0;
}

static char choose_shade(int side, int stepX, int stepY)
{
    char face;
    if (side == 0)
        face = (stepX == -1) ? 'W' : 'E';
    else
        face = (stepY == -1) ? 'N' : 'S';

    return face;
}

static void draw_minimap(char *screen, t_map *map, double px, double py, double dirX, double dirY)
{
    int map_width = (map->cols < MINIMAP_WIDTH) ? map->cols : MINIMAP_WIDTH;
    int map_height = (map->rows < MINIMAP_HEIGHT) ? map->rows : MINIMAP_HEIGHT;

    for (int y = 0; y < map_height; ++y)
    {
        for (int x = 0; x < map_width; ++x)
        {
            char cell = map->grid[y][x];
            char display = (cell == '1') ? '#' : ' ';
            screen[y * (SCREEN_WIDTH + 1) + x] = display;
        }
    }

    int player_map_x = (int)px;
    int player_map_y = (int)py;
    if (player_map_x >= 0 && player_map_x < map_width &&
        player_map_y >= 0 && player_map_y < map_height)
    {
        char dir_char = 'P';
        if (dirX > 0.5)
            dir_char = '>';
        else if (dirX < -0.5)
            dir_char = '<';
        else if (dirY > 0.5)
            dir_char = 'v';
        else if (dirY < -0.5)
            dir_char = '^';
        screen[player_map_y * (SCREEN_WIDTH + 1) + player_map_x] = dir_char;
    }
}

static void draw_screen(const char *screen)
{
    write(1, "\x1b[2J\x1b[H", 7);
    write(1, screen, SCREEN_HEIGHT * (SCREEN_WIDTH + 1));
}

static int is_wall(t_map *map, int x, int y)
{
    if (x < 0 || x >= map->cols || y < 0 || y >= map->rows)
        return 1;
    return map->grid[y][x] == '1';
}

static void render_frame(t_map *map, double px, double py, double dirX, double dirY, double planeX, double planeY)
{
    static char screen[SCREEN_HEIGHT * (SCREEN_WIDTH + 1)];
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            if (y > SCREEN_HEIGHT / 2)
                screen[y * (SCREEN_WIDTH + 1) + x] = '.';
            else
                screen[y * (SCREEN_WIDTH + 1) + x] = ' ';
        }
        screen[y * (SCREEN_WIDTH + 1) + SCREEN_WIDTH] = '\n';
    }

    for (int x = 0; x < SCREEN_WIDTH; ++x)
    {
        double cameraX = 2.0 * x / (double)SCREEN_WIDTH - 1.0;
        double rayDirX = dirX + planeX * cameraX;
        double rayDirY = dirY + planeY * cameraX;

        int mapX = (int)px;
        int mapY = (int)py;

        double sideDistX;
        double sideDistY;
        double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0 / rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0 / rayDirY);
        int stepX;
        int stepY;
        int hit = 0;
        int side = 0;

        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (px - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - px) * deltaDistX;
        }
        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (py - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - py) * deltaDistY;
        }

        while (!hit)
        {
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (mapX >= 0 && mapX < map->cols && mapY >= 0 && mapY < map->rows)
                if (map->grid[mapY][mapX] == '1')
                    hit = 1;
        }

        double perpWallDist;
        if (side == 0)
            perpWallDist = (mapX - px + (1 - stepX) / 2.0) / rayDirX;
        else
            perpWallDist = (mapY - py + (1 - stepY) / 2.0) / rayDirY;
        if (perpWallDist <= 0.0)
            perpWallDist = 0.001;

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0)
            drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT)
            drawEnd = SCREEN_HEIGHT - 1;

        char shade = choose_shade(side, stepX, stepY);
        for (int y = drawStart; y <= drawEnd; ++y)
            screen[y * (SCREEN_WIDTH + 1) + x] = shade;
    }

    draw_minimap(screen, map, px, py, dirX, dirY);
    draw_screen(screen);
}

static void move_player(t_map *map, double *px, double *py, double dirX, double dirY, double planeX, double planeY, int command)
{
    double nx = *px;
    double ny = *py;
    if (command == 'W' || command == 'w')
    {
        nx = *px + dirX * MOVE_SPEED;
        ny = *py + dirY * MOVE_SPEED;
    }
    else if (command == 'S' || command == 's')
    {
        nx = *px - dirX * MOVE_SPEED;
        ny = *py - dirY * MOVE_SPEED;
    }
    else if (command == 'A' || command == 'a')
    {
        nx = *px - planeX * MOVE_SPEED;
        ny = *py - planeY * MOVE_SPEED;
    }
    else if (command == 'D' || command == 'd')
    {
        nx = *px + planeX * MOVE_SPEED;
        ny = *py + planeY * MOVE_SPEED;
    }
    if (!is_wall(map, (int)nx, (int)*py))
        *px = nx;
    if (!is_wall(map, (int)*px, (int)ny))
        *py = ny;
}

void render_loop(t_map *map)
{
    struct termios orig;
    if (!enable_raw_mode(&orig))
    {
        write(2, "Error: unable to set terminal raw mode\n", strlen("Error: unable to set terminal raw mode\n"));
        return;
    }

    double posX = map->player_x + 0.5;
    double posY = map->player_y + 0.5;
    double dirX;
    double dirY;
    double planeX;
    double planeY;

    if (map->player_dir == 'N')
    {
        dirX = 0.0; dirY = -1.0;
        planeX = 0.66; planeY = 0.0;
    }
    else if (map->player_dir == 'S')
    {
        dirX = 0.0; dirY = 1.0;
        planeX = -0.66; planeY = 0.0;
    }
    else if (map->player_dir == 'E')
    {
        dirX = 1.0; dirY = 0.0;
        planeX = 0.0; planeY = 0.66;
    }
    else
    {
        dirX = -1.0; dirY = 0.0;
        planeX = 0.0; planeY = -0.66;
    }

    int running = 1;
    while (running)
    {
        render_frame(map, posX, posY, dirX, dirY, planeX, planeY);
        int c = read_input_char();
        if (c == 0)
        {
            usleep(20000);
            continue;
        }
        if (c == 'q' || c == 'Q')
            running = 0;
        else if (c == 'W' || c == 'w' || c == 'S' || c == 's' || c == 'A' || c == 'a' || c == 'D' || c == 'd')
            move_player(map, &posX, &posY, dirX, dirY, planeX, planeY, c);
        else if (c == 27)
        {
            int arrow = get_arrow_key();
            if (arrow == 'L' || arrow == 'R')
            {
                double rot = (arrow == 'L') ? -ROT_SPEED : ROT_SPEED;
                double oldDirX = dirX;
                dirX = dirX * cos(rot) - dirY * sin(rot);
                dirY = oldDirX * sin(rot) + dirY * cos(rot);
                double oldPlaneX = planeX;
                planeX = planeX * cos(rot) - planeY * sin(rot);
                planeY = oldPlaneX * sin(rot) + planeY * cos(rot);
            }
            else
                running = 0;
        }
    }

    restore_terminal(&orig);
    write(1, "\x1b[2J\x1b[H", 7);
}
