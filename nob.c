#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

void common_flags(Cmd* cmd) {
    cmd_append(cmd, "-Wall", "-Wextra", "-Werror", "-g");
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    common_flags(&cmd);
    cmd_append(&cmd, "src/main.c", "-o", "nslc", "src/lexer.c", "src/parser.c", "src/arena.c", "src/qbe.c", "src/codegen.c");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    if (argc >= 2 && strcmp(argv[1], "run") == 0) {
        cmd_append(&cmd, "./nslc");
        for (int i = 2; i < argc; i++) {
            cmd_append(&cmd, argv[i]);
        }
        if (!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}
