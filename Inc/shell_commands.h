#ifndef SHELL_COMMANDS_H_
#define SHELL_COMMANDS_H_

void ShellCmd_led_on(int argc, char** argv);
void ShellCmd_led_off(int argc, char** argv);
void ShellCmd_hello(int argc, char** argv);
void ShellCmd_help(int argc, char** argv);
typedef void (*ShellCommandHandler)(int argc, char **argv);

typedef struct {
    const char *cmd;
    ShellCommandHandler handler;
    const char *desc;
} ShellCommand;

extern const ShellCommand shell_commands[];
extern const int shell_command_count;

#endif
