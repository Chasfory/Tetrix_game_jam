##
## EPITECH PROJECT, 2022
## B-OOP-400-PAR-4-1-tekspice-laetitia.bousch
## File description:
## Makefile
##

NAME			=	my_tetrix

SFML			=	-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

CFLAGS 			=	-Wall -Wextra -std=c++20 -fno-gnu-unique

CPPFLAGS	    =	$(INC)

CC				=	g++

SRC_DIR			=	src/

SRC			=	$(SRC_DIR)main.cpp			\

OBJ			=	$(SRC:.cpp=.o)

all:	$(NAME)

$(NAME):	$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(SFML) -lm

db:     CFLAGS += -g3
db:		$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(SFML) -lm

clean:
	$(RM) $(OBJ) *gc*

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

redb: fclean db

.PHONY: all tests fclean re db redb