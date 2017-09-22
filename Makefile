CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Og -g -D_DEFAULT_SOURCE
ALL=xcb sdl gtk

all: $(ALL)

xcb: CFLAGS+=`pkg-config --cflags xcb xcb-icccm`
xcb: LDLIBS+=`pkg-config --libs xcb xcb-icccm`

sdl: CFLAGS+=`pkg-config --cflags sdl2`
sdl: LDLIBS+=`pkg-config --libs sdl2` -lm

gtk: CFLAGS+=`pkg-config --cflags gtk+-3.0` -Wno-unused-parameter
gtk: LDLIBS+=`pkg-config --libs gtk+-3.0`

.PHONY: clean
clean:
	rm -rf $(ALL)
