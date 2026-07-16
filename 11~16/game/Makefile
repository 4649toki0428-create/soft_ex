TARGET = soda_dash
VPATH = src
SRCS = main.c window.c player.c gimmick.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Iheader
LIBS = -lSDL2 -ljoyconlib -lhidapi-hidraw -lm -lSDL2_ttf -lSDL2_image

$(TARGET): $(OBJS)
		gcc $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
		gcc $(CFLAGS) -c $< -o $@