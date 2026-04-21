CXX = c++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -g
#CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer -O1

# Target executable
TARGET = webserv



SOURCE_DIR = ./src


VPATH = $(SOURCE_DIR)
# Source files
#SRCS =	server.cpp \
#		client.cpp
SRCS =	main.cpp
		

# Object files

OBJS_Dir = objects
OBJS = $(addprefix $(OBJS_Dir)/, $(notdir $(SRCS:.cpp=.o)))


# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJS_Dir):
	@echo "Creating $(OBJS_Dir) directory"
	@mkdir -p $(OBJS_Dir)

# Compile source files into object files
$(OBJS_Dir)/%.o: %.cpp | $(OBJS_Dir)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) 

fclean: clean
	rm -f $(TARGET)
	rm -f *.txt
	rm -r -f objects

re: fclean all

# do not search for this files
.PHONY: all clean fclean re