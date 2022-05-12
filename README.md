# CHIP-8
A CHIP-8 interpreter written in C with the SDL 2 media library.

A WIP interpreter for the CHIP-8 programming language and virtual machine. Mostly developed so I could learn how emulation software is typically structured. This project gave me a great head start when developing my MOS 6502 CPU emulator (and WIP Nintendo Entertainment System emulator) later.

As this project was (originally) just a quick experiment, it does have a few flaws:

- No sound (though a bunch of Z's get printed to the console as a substitute ;) )
- Update rate is limited to 700Hz as this is supposedly the ideal speed for most C8 games (ideally it should be changeable, as this would help with support for other implementations of the CHIP-8 VM which ran at different speeds).
- Font rendering is totally bugged. In Pong for example, you have to score 5 times for the number to change over completely. That being said, there's a chance this is actually a problem with another non-font-related opcode and I just glossed over it.
- Both the main 60Hz timer (used to update the CHIP-8's sound and delay timers) and the system update timer (the one running at 700Hz) are clumped in together and update all in the same block of code. Ideally, I should have made a timer type to store their deltas, accumulators, etc and updated them seperately.
- No support for later updates/expansions/changes made to the CHIP-8 specification.


Initial uploaded on March 7th 2018
