# compiling stuff
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17
FLAG_OPTIMIZING = -O2
FLAG_DEBUG = -g -O0

# header files
INCLUDE = ./inc ./inc/config ./inc/config_parser ./inc/config_server
CXXFLAGS += $(addprefix -I,$(INCLUDE))

# program name
NAME = webserv

# source files
VPATH =	src \
		src/config_parser \
		src/config_server

SRCS =	main.cpp \
		Lexer.cpp \
		Parser.cpp \
		ps.cpp \
		pss.cpp \
		pls.cpp \
		memberfkParser.cpp \
		Server.cpp \
		Server_epoll.cpp \
		Server_io.cpp \
		Server_sockets.cpp


# obejct files
OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

all: CXXFLAGS += $(FLAG_OPTIMIZING)
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

# .cpp zu .o
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CXXFLAGS += $(FLAG_DEBUG)
debug: $(NAME)

run: $(NAME)
	./$(NAME) conf/example_1.conf

valgrind: fclean debug
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(NAME)

.PHONY: all clean fclean re run debug valgrind
