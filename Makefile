NAME = Codexion

SRC = 

CC = CC

OBJ = $(SRC:.c = .o)


CFLAGS = -Wall -Wextra -Werror

$(NAME):$(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.C
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(NAME)
.PHONY:


