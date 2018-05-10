CFLAGS   = -Wall -Wextra -mtune=native `sdl2-config --cflags`
LDFLAGS  = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm 

#Win 64 build
CCW64 	  = x86_64-w64-mingw32-gcc
CFLAGS3   = -Wall -Wextra `x86_64-w64-mingw32-sdl2-config --cflags`
LDFLAGS3  = `x86_64-w64-mingw32-sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

.SUFFIXES:
.SUFFIXES: .c .o

srcdir	 =src/
TARGETS	 = pong_L64 pong_s_L64

.PHONY: all
all: $(TARGETS)

pong_L64: $(srcdir)helper.c $(srcdir)main.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

pong_s_L64: $(srcdir)helper2.c $(srcdir)2.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

pong_W64.exe:   $(srcdir)helper2.c $(srcdir)2.c
	$(CCW64) $(CFLAGS3) -DALIEN -o $@ $+ $(LDFLAGS3)

.PHONY: clean
clean:
	@rm $(TARGETS) 2>/dev/null || true

