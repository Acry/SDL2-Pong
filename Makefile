CFLAGS   = -Wall -Wextra -mtune=native `sdl2-config --cflags`
LDFLAGS  = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm 

.SUFFIXES:
.SUFFIXES: .c .o

srcdir	 =src/
TARGETS	 = pong

.PHONY: all
all: $(TARGETS)

pong: $(srcdir)helper.c $(srcdir)main.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

.PHONY: clean
clean:
	@rm $(TARGETS) 2>/dev/null || true

