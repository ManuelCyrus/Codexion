
NAME        = codexion

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread -Iinclude

SRCS_DIR    = src
OBJS_DIR    = objs
SRCS        = $(SRCS_DIR)/main.c \
              $(SRCS_DIR)/heap.c \
              $(SRCS_DIR)/simulation.c \
              $(SRCS_DIR)/utils.c

OBJS        = $(SRCS:$(SRCS_DIR)/%.c=$(OBJS_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJS_DIR)
fclean: clean
	rm -f $(NAME)
re: fclean all

.PHONY: all clean fclean re
