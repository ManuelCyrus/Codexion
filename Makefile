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

# ==========================================
# COLORS
# ==========================================

GREEN  = \033[0;32m
RED    = \033[0;31m
YELLOW = \033[1;33m
BLUE   = \033[0;34m
CYAN   = \033[0;36m
RESET  = \033[0m

# ==========================================
# BUILD
# ==========================================

all: $(NAME)

$(NAME): $(OBJS)
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"
	@printf "$(GREEN) Building $(NAME)...$(RESET)\n"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN) Build complete!$(RESET)\n"
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(OBJS_DIR)
	@printf "$(BLUE)📦 Compiling $<$(RESET)\n"
	@$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# CLEAN
# ==========================================

clean:
	@printf "$(YELLOW)🧹 Cleaning objects...$(RESET)\n"
	@rm -rf $(OBJS_DIR)

fclean: clean
	@printf "$(RED) Removing executable...$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

# ==========================================
# TESTS
# ==========================================

run_fifo: all
	@printf "$(BLUE)▶ Running FIFO test...$(RESET)\n"
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"
	@./$(NAME) 5 800 200 200 200 5 50 fifo

run_edf: all
	@printf "$(BLUE)▶ Running EDF test...$(RESET)\n"
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"
	@./$(NAME) 5 800 200 200 200 5 50 edf

run_one: all
	@printf "$(BLUE)▶ Running single coder test...$(RESET)\n"
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"
	@./$(NAME) 1 800 200 200 200 5 50 fifo

stress: all
	@printf "$(RED) Running stress test...$(RESET)\n"
	@printf "$(CYAN)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)\n"
	@./$(NAME) 200 800 200 200 200 3 10 edf

valgrind: all
	@printf "$(YELLOW)🛠 Running Valgrind...$(RESET)\n"
	@valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		./$(NAME) 5 800 200 200 200 5 50 fifo

norm:
	@printf "$(CYAN)📏 Running Norminette...$(RESET)\n"
	@norminette

# ==========================================
# PHONY
# ==========================================

.PHONY: all clean fclean re \
		run_fifo run_edf run_one \
		stress valgrind norm