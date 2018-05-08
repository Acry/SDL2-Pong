CFLAGS   = -Wall -Wextra -mtune=native `sdl2-config --cflags`
LDFLAGS  = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm 

.SUFFIXES:
.SUFFIXES: .c .o

srcdir	 =src/
TARGETS	 = pong pong_s

.PHONY: all
all: $(TARGETS)

pong: $(srcdir)helper.c $(srcdir)main.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

pong_s: $(srcdir)helper2.c $(srcdir)2.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

.PHONY: clean
clean:
	@rm $(TARGETS) 2>/dev/null || true

