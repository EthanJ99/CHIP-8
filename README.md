# CHIP-8
A CHIP-8 interpreter written in C with the SDL 2 media library.

As this was just a quick experimental project, a lot of things aren't implemented or aren't up to the standards of other intepreters. I'm also aware of some areas where my code is messy and some features are outright broken. I'll list these flaws here for quick reference:

- No sound (though a bunch of Z's get printed to the console as a substitute ;) )
- Update rate is limited to 700Hz as this is supposedly the ideal speed for most C8 games (ideally it should be changeable, as this would help with support for other implementations of the CHIP-8 VM which ran at different speeds).
- Font rendering is totally bugged. In Pong for example, you have to score 5 times for the number to change over completely. That being said, there's a chance this is actually a problem with another non-font-related opcode and I just glossed over it.
- Both the main 60Hz timer (used to update the CHIP-8's sound and delay timers) and the system update timer (the one running at 700Hz) are clumped in together and update all in the same block of code. Ideally, I should have made a timer type to store their deltas, accumulators, etc and updated them seperately.
- There's a bunch of unnecessary SDL headers bundled with the source. Only the regular old SDL headers are needed but I couldn't remember which ones they were!

The idea behind programming this was to understand the basic development process of something akin to an emulator. As I have no real interest in the Chip-8 system itself, and feel I have learnt everything I expected to learn from this project, the various bugs and unimplemented features in this program are unlikely to be fixed. Was fun to get it (mostly) working though!

Uploaded on March 7th 2018
