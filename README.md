# Quick X Graphics

What's the quickest (aka simplest) way to put some graphics on the screen
under X?
I wasn't sure either, so I decided to try a few different approaches, all
from C of course.
Since it doesn't really matter what exactly we draw, I cloned the classic
"Boxes" demo from the early Amiga days.

Enjoy!

## xcb

- complexity from okay (dis/connect) to insane (handling window close)
- documentation *extremely* spotty (and *horribly* out-of-date in places)
- requires familiarity with nearly all of the underlying protocols
- masks/values APIs very brittle (order-dependent!)
- cookie/reply and cookie/check APIs somewhat painful (async hell?)
