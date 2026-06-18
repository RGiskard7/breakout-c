CC=gcc
CFLAGS=-g -Wall -pedantic

ALLEGRO_VERSION=5.2.9.1
MINGW_VERSION=14.1.0
FOLDER=C:
FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib
INCLUDE_ALLEGRO=\include

FLAGS=-I breakout/include -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) $(CFLAGS)
LIBS=-L $(PATH_ALLEGRO)$(LIB_ALLEGRO) -lallegro_monolith -lallegro_main -lallegro_primitives -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec

OBJS=breakout/src/main.o breakout/src/game.o breakout/src/paddle.o breakout/src/ball.o breakout/src/brick.o
TARGET=breakout/breakout.exe

all: $(TARGET)

breakout/src/main.o: breakout/src/main.c breakout/include/game.h breakout/include/config.h
	$(CC) $(FLAGS) -c breakout/src/main.c -o breakout/src/main.o

breakout/src/game.o: breakout/src/game.c breakout/include/game.h breakout/include/config.h breakout/include/paddle.h breakout/include/ball.h breakout/include/brick.h
	$(CC) $(FLAGS) -c breakout/src/game.c -o breakout/src/game.o

breakout/src/paddle.o: breakout/src/paddle.c breakout/include/paddle.h breakout/include/config.h
	$(CC) $(FLAGS) -c breakout/src/paddle.c -o breakout/src/paddle.o

breakout/src/ball.o: breakout/src/ball.c breakout/include/ball.h breakout/include/config.h
	$(CC) $(FLAGS) -c breakout/src/ball.c -o breakout/src/ball.o

breakout/src/brick.o: breakout/src/brick.c breakout/include/brick.h breakout/include/config.h
	$(CC) $(FLAGS) -c breakout/src/brick.c -o breakout/src/brick.o

$(TARGET): $(OBJS)
	@echo ----------------------------------------------------------
	@echo Makefile Breakout
	@echo ----------------------------------------------------------
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	del /q $(OBJS) $(TARGET) 2>nul
