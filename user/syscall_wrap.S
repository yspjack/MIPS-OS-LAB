#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>


LEAF(msyscall)
    // TODO: execute a `syscall` instruction and return from msyscall
    move v0, a0     // syscall number
    syscall
    jr ra
END(msyscall)

