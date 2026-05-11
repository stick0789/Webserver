
NAME = webserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
HEADERS    =  -I inc \
              -I inc/CGI \
              -I inc/mime \
              -I inc/parser \
              -I inc/request \
              -I inc/response \
              -I inc/server \
              -I inc/utils

SRCS        = main.cpp \
              src/CGI/CGIHandler.cpp \
              src/mime/MimeTypes.cpp \
              src/response/HTTPResponse.cpp \
              src/response/ResponseBuilder.cpp \
              src/utils/Utils.cpp \
              src/utils/Debug.cpp \
              src/utils/HTTPStatus.cpp

OBJS        = $(SRCS:.cpp=.o)

GREEN       = \033[0;32m
RED         = \033[0;31m
RESET       = \033[0m

# Regla principal
all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) compilat amb èxit!$(RESET)"

%.o: %.cpp
	@echo "Compilant $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
    
clean:
	@rm -f $(OBJS)
	@echo "$(RED)✗ Objectes eliminats.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)✗ Executable $(NAME) eliminat.$(RESET)"

re: fclean all

.PHONY: all clean fclean re