CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Og -g -D_DEFAULT_SOURCE
ALL=xcb

all: $(ALL)

xcb: CFLAGS+=`pkg-config --cflags xcb xcb-icccm`
xcb: LDLIBS+=`pkg-config --libs xcb xcb-icccm`

.PHONY: clean
clean:
	rm -rf $(ALL)
