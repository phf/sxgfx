# Simple X Graphics: An Incomplete Survey

Quick, what's the *simplest* way to put some graphics on the screen under
[X](https://en.wikipedia.org/wiki/X_Window_System)?
I wasn't sure either, so I decided to try a few different approaches, all
from C of course.
Since it doesn't really matter what we draw, I cloned the classic "Boxes" demo
from the early [Amiga](https://en.wikipedia.org/wiki/Amiga) days.

Enjoy!

## [xcb](https://en.wikipedia.org/wiki/XCB)

- complexity from okay (dis/connect) to insane (handling window close)
- documentation *extremely* spotty (and *horribly* out-of-date in places)
- requires familiarity with nearly *all* of the underlying protocols
- mask/values APIs are *very* brittle (order-dependent!)
- cookie/reply and cookie/check APIs somewhat painful (async hell?)

## [sdl](https://en.wikipedia.org/wiki/Simple_DirectMedia_Layer)

- complexity quite reasonable, just a lot of flags to look through
- documentation quite reasonable, includes (correct!) examples as well
- *lots* of [valgrind](https://en.wikipedia.org/wiki/Valgrind) issues (not just
  leaks!) in the library, pretty sad

## [gtk](https://en.wikipedia.org/wiki/GTK%2B)

- complexity moderately convoluted throughout, often 12 ways of doing something
  and little to help decide; lots of layers (glib, gdk, gtk, cairo)
- documentation reasonable, lots of it however (maybe too much?) and lots of
  (slightly) different versions; return values sometimes strangely absent;
  often hard to find the correct callback prototype
- the idle/tick combination is actually somewhat cute for real-time graphics
- threading implications are not clear at all but I didn't run afoul of that
- entire design seems leaky as [valgrind](https://en.wikipedia.org/wiki/Valgrind)
  is quick to point out

## [x11](https://en.wikipedia.org/wiki/X_Window_System)

- in progress
