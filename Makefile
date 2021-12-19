CLASSES = Channel Client Command ListenSocket Socket
INTERFACES = types Reply Parser
SOURCES_WS = $(addsuffix .cpp, $(CLASSES)) \
	main.cpp commands/channels.cpp commands/messages.cpp commands/monitoring.cpp commands/oper.cpp commands/pingpong.cpp \
	commands/register.cpp commands/utils.cpp
HEADERS_WS = $(addsuffix .hpp, $(CLASSES)) $(addsuffix .hpp, $(INTERFACES)) \

SOURCES = $(addprefix srcs/, $(SOURCES_WS))
HEADERS = $(addprefix includes/, $(HEADERS_WS))

NAME = ircserv

CC = clang++ -std=c++98 -I. -I includes -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) -o $(NAME)

clean:

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re