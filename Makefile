NAME	= BlockBlast

CC	= gcc

RM	= rm -f

SRCS	= ./src/main.c ./src/bb.c

OBJS	= $(SRCS:.c=.o)

CFLAGS = -iquote ./include/
CFLAGS += -Wall -Wextra

all: $(NAME)

$(NAME): $(OBJS)
	 $(CC) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

gen:
	python3 genblock.py

.PHONY: all clean fclean re gen
