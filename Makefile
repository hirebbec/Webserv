NAME		=	webserv

CC			=	c++
CFLAGS		=	-std=c++98 -Wall -Wextra -Werror# -fsanitize=address
RM			=	rm -f

SRCS		=	developer_files/srcs/main.cpp developer_files/srcs/utils.cpp developer_files/srcs/parser.cpp
OBJS		=	$(SRCS:.cpp=.o)
HEADERS     =   developer_files/includes/*.hpp

$(NAME)		:	$(OBJS)
				$(CC) $(CFLAGS) -o $@ $^

all			:	$(NAME)

%.o			:	%.cpp $(HEADERS)
				$(CC) $(CFLAGS) -c -o $@ $<

clean		:
				$(RM) $(OBJS)

fclean		:	clean
				$(RM) $(NAME)

re:				fclean all

.PHONY		:	all clean fclean re test