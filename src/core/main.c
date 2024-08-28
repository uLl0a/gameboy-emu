#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rom.h"
#include "cpu.h"
#include "useful_utils.h"

int main(int argc, char **argv)
{
    /* checks if have any arguments */
    if (argc <= 1)
        goto usage;

    /* step foward in the array */
    --argc;
    ++argv;

    /* parssing the emulator arguments */
    while (argc > 0) {
        if (!strcmp(argv[0], "-rom")) {
            --argc;
            ++argv;
            if (!argc || argv[0][0] == '-') {
                goto usage;
            }
            load_rom(argv[0]);
            --argc;
            ++argv;
        }
    }

    cpu_reset();
    for(;;){
        cpu_step();
    }

    unload_rom();
    goto end;

usage:
    printf("Usage: gbemu [option] ...\n");
    printf("-rom <rom.gb>\n");
end:
    exit(0);
}
