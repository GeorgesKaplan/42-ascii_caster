CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude
LDFLAGS = -lm
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
NAME = bin/ft_ascii_caster

.PHONY: all clean fclean re run run-distance run-orientation run-o

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(dir $(NAME))
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: all
	./$(NAME) maps/classic.map

run-distance: all
	./$(NAME) maps/classic.map

run-orientation: all
	./$(NAME) --orientation maps/classic.map

run-o: all
	./$(NAME) -o maps/classic.map
