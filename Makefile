NAME = ircserv
BOT_NAME = ircbot

CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
DEPFLAGS = -MMD -MP

INCLUDE = include

SRC_DIR = src
SRC_FILES = main.cpp \
			Util.cpp \
			Server.cpp \
			ServerEvents.cpp \
			ServerConnections.cpp \
			ServerClientIO.cpp \
			ServerCommands.cpp \
			ServerUtils.cpp \
			Signal.cpp \
			Channel.cpp \
			command/PASS.cpp \
			command/NICK.cpp \
			command/USER.cpp \
			command/JOIN.cpp \
			command/PRIVMSG.cpp \
			command/INVITE.cpp \
			command/MODE.cpp \
			command/KICK.cpp \
			command/TOPIC.cpp \
			command/PART.cpp \
			command/PING.cpp \
			command/PONG.cpp \
			command/NOTICE.cpp \
			command/QUIT.cpp
BOT_DIR = src/bot
BOT_FILES = main.cpp \
			Bot.cpp

SRC =$(addprefix $(SRC_DIR)/, $(SRC_FILES))
BOT_SRC = $(addprefix $(BOT_DIR)/, $(BOT_FILES))

OBJ_DIR = obj
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
BOT_OBJ_DIR = obj_bot
BOT_OBJ = $(patsubst $(BOT_DIR)/%.cpp, $(BOT_OBJ_DIR)/%.o, $(BOT_SRC))

DEPS = $(OBJ:.o=.d) $(BOT_OBJ:.o=.d)

RED = \033[31m
GREEN = \033[1;32m
YELLOW = \033[33m
RESET = \033[0m

all : $(NAME)
	@printf "$(GREEN)✨ $(NAME) ready! ✨$(RESET)\n"
bonus: $(BOT_NAME)
	@printf "$(GREEN)✨ $(BOT_NAME) ready! ✨$(RESET)\n"

$(NAME) : $(OBJ)
	@printf "$(YELLOW)Linking $(NAME)...$(RESET)\n"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
$(BOT_NAME) : $(BOT_OBJ)
	@printf "$(YELLOW)Linking $(BOT_NAME)...$(RESET)\n"
	@$(CC) $(CFLAGS) $(BOT_OBJ) -o $(BOT_NAME)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)Compiling $<...$(RESET)\n"
	@$(CC) -I $(INCLUDE) $(DEPFLAGS) $(CFLAGS) -c -g $< -o $@
$(BOT_OBJ_DIR)/%.o : $(BOT_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)Compiling $<...$(RESET)\n"
	@$(CC) -I $(INCLUDE) $(DEPFLAGS) $(CFLAGS) -c -g $< -o $@

clean :
	@printf "$(RED)Cleaning object folder... 🧹$(RESET)\n"
	@rm -rf $(OBJ_DIR) $(BOT_OBJ_DIR)

fclean : clean
	@printf "$(RED)Deleting $(NAME) $(BOT_NAME) executable... 🧹⚠️$(RESET)\n"
	@rm -f $(NAME) $(BOT_NAME)

re:	fclean all

-include $(DEPS)

.PHONY:	all bonus clean fclean re
