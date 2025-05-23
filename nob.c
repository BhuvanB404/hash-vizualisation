#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"


int main(int argc, char **argv)
{

    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0}; // memort holder for the arena.

    cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-Wswitch-enum", "-ggdb", "-o", "randomart", "randomart.c", "-lm");
    if(!cmd_run_sync(cmd)) return 1;

    cmd.count = 0;

    cmd_append(&cmd, "./randomart");
    if(!cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;

}
