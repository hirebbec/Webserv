NAME		=	client

CC			=	c++
CFLAGS		=	-std=c++98 -Wall -Wextra -Werror
RM			=	rm -f

SRCS		=	client.cpp
OBJS		=	$(SRCS:.cpp=.o)

$(NAME)		:	$(OBJS)
				$(CC) -o $@ $^

all			:	$(NAME)

%.o			:	%.cpp
				$(CC) $(CFLAGS) -c -o $@ $<

clean		:
				$(RM) $(OBJS)

fclean		:	clean
				$(RM) $(NAME)

re:				fclean all

.PHONY		:	all clean fclean re test