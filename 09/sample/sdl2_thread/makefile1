OBJS = sdl2_thread.c
OBJ_NAME = sdl2_thread

all: $(OBJS)
	gcc $(OBJS) -ljoyconlib -lhidapi-hidraw -lSDL2 -lSDL2_gfx -lm -L/usr/lib -g -o $(OBJ_NAME)

.PHONY: clean

clean:
	rm -f *.o core $(OBJ_NAME)
