CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -DNDEBUG
LDFLAGS = -lGL -lGLU -lglut -lpng -lm

# Remove -DNDEBUG to enable FPS counter
#CFLAGS  = -std=c99 -Wall -Wextra -g

SRCS = main.c shader.c texture.c maze.c walker.c camera.c wall.c ico.c rat.c
OBJS = $(SRCS:.c=.o)
TARGET = maze

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Header dependencies (keep it simple for embedded use)
main.o:    main.c math3d.h shader.h texture.h maze.h walker.h camera.h wall.h ico.h rat.h
shader.o:  shader.c shader.h math3d.h
texture.o: texture.c texture.h
maze.o:    maze.c maze.h
walker.o:  walker.c walker.h maze.h math3d.h
camera.o:  camera.c camera.h math3d.h
wall.o:    wall.c wall.h shader.h texture.h math3d.h
ico.o:     ico.c ico.h shader.h math3d.h
rat.o:     rat.c rat.h walker.h shader.h texture.h math3d.h

clean:
	rm -f $(OBJS) $(TARGET)
