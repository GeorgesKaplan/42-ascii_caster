CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
NAME = bin/ft_ascii_caster

.PHONY: all clean fclean re run

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(dir $(NAME))
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: all
	./$(NAME) maps/classic.map
