NAME = irc

CC = c++

FLAGS = -std=c++98 -g -MMD -Werror -Wextra -Wall 

SRC = main.cpp Server.cpp Client.cpp Channel.cpp

OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

-include $(DEP)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(DEP)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: clean fclean all re