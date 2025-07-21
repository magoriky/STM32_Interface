#include "shell_extensions.h"
#include "shell.h"
#include <string.h>
#include <stdio.h>
#include "shell_commands.h"
extern LineEditorState editor;

//#define SHELL_PROMPT ANSI_BOLD ANSI_MAGENTA "root" ANSI_RESET ANSI_BOLD ANSI_GREEN "@" ANSI_RESET ANSI_BOLD ANSI_MAGENTA "root" ANSI_RESET ANSI_BOLD ANSI_YELLOW "> " ANSI_RESET

#define SHELL_PROMPT "root@root> "

void Shell_ClearScreen(void) {
    Shell_Print("\033[2J\033[H");
}

void Shell_PrintBanner(void) {
    Shell_Print(ANSI_BOLD ANSI_RED "\r\n"
        "   ██████╗ ████╗  ██╗██╗  ██╗\r\n"
        "  ██╔════╝ ██╔██╗ ██║██║ ██╔╝\r\n"
        "  ██║  ███╗██║╚██╗██║█████╔╝ \r\n"
        "  ██║   ██║██║ ╚████║██╔═██╗ \r\n"
        "  ╚██████╔╝██║  ╚███║██║  ██╗\r\n"
        "   ╚═════╝ ╚═╝   ╚══╝╚═╝  ╚═╝\r\n"
        ANSI_RESET);

    Shell_Print(ANSI_BOLD ANSI_RED "\r\n==============================================\r\n" ANSI_RESET);
    Shell_Print(ANSI_BOLD ANSI_YELLOW "   GNK FOOD TECH: EMBEDDED INNOVATION HUB\r\n" ANSI_RESET);
    Shell_Print(ANSI_BOLD ANSI_RED   "==============================================\r\n" ANSI_RESET);
}

void Shell_PrintWelcome(void) {
    Shell_Print(ANSI_BOLD ANSI_GREEN
        "  🚀 Powered by STM32 - Engineered for Excellence\r\n"
        "  🍽️  Automating Food Tech with Precision Control\r\n"
        "  🔐 Secure. \r\n  🔧 Scalable. \r\n  ⚡ Real-Time. \r\n"
        ANSI_RESET);
}

void Shell_PrintCursor(void) {
    Shell_Print(ANSI_BOLD ANSI_MAGENTA "root" ANSI_RESET
              ANSI_BOLD ANSI_GREEN "@" ANSI_RESET
              ANSI_BOLD ANSI_MAGENTA "root" ANSI_RESET
              ANSI_BOLD ANSI_YELLOW "> " ANSI_RESET);
}

//void Shell_PrintCursor(void) {
//    Shell_Print(ANSI_CYAN "> " ANSI_RESET);
//}
void Shell_HandleTabCompletion(void) {
    // Ensure null-termination
    if (editor.line_length >= SHELL_INPUT_BUFFER_SIZE)
        return;  // safety check
    editor.line[editor.line_length] = '\0';  // ✅ Fix

    int matches = 0;
    const char* last_match = NULL;
    size_t len = editor.line_length;  // ✅ use exact length

    for (int i = 0; i < shell_command_count; ++i) {
        if (strncmp(shell_commands[i].cmd, editor.line, len) == 0) {
            if (matches == 0) {
                last_match = shell_commands[i].cmd;
            } else {
                last_match = NULL;
            }
            matches++;
        }
    }

    if (matches == 1 && last_match != NULL) {
        const char* suffix = last_match + len;  // complete only remainder
        strncat(editor.line, suffix, SHELL_INPUT_BUFFER_SIZE - len - 1);
        editor.line_length += strlen(suffix);
        editor.cursor_pos = editor.line_length;

        Shell_Print(suffix);
    } else if (matches > 1) {
        Shell_Print("\r\n");
        for (int i = 0; i < shell_command_count; ++i) {
            if (strncmp(shell_commands[i].cmd, editor.line, len) == 0) {
            	Shell_Print("  ");
            	Shell_Print(ANSI_BOLD ANSI_WHITE);  // Prefix in bold white
            	Shell_PrintN(shell_commands[i].cmd, len);  // Custom function: print only the prefix
            	Shell_Print(ANSI_CYAN);  // Suffix in cyan
            	Shell_Print(shell_commands[i].cmd + len);  // Remaining part
            	Shell_Print(ANSI_RESET "\r\n");

            }
        }
        Shell_PrintCursor();
        Shell_Print(editor.line);
    } else {
        Shell_Print("\a");  // No match
    }
}
void Shell_RedrawLineSmooth(void) {
    Shell_Print("\r");             // Return to start of line
    Shell_PrintCursor();          // Print prompt

    // Extract the first word (command)
    char command[32] = {0};
    int space_idx = -1;
    for (uint16_t i = 0; i < editor.line_length && i < sizeof(command) - 1; i++) {
        if (editor.line[i] == ' ') {
            space_idx = i;
            break;
        }
        command[i] = editor.line[i];
    }
    if (space_idx == -1) space_idx = strlen(command);

    // Check if command is valid
    bool is_cmd_valid = false;
    for (int i = 0; i < shell_command_count; ++i) {
        if (strncmp(shell_commands[i].cmd, command, space_idx) == 0 &&
            strlen(shell_commands[i].cmd) == space_idx) {
            is_cmd_valid = true;
            break;
        }
    }

    // Print colored command if valid
    if (is_cmd_valid) {
        Shell_Print(ANSI_GREEN);
        Shell_PrintN(editor.line, space_idx);
        Shell_Print(ANSI_RESET);
    } else {
        Shell_PrintN(editor.line, space_idx);
    }

    // Print the rest of the line normally
    Shell_Print(&editor.line[space_idx]);

    // Clear remaining chars from previous line render
    Shell_Print("\033[K");

    // Move cursor to correct spot
    // Move cursor to correct spot
    int prompt_len = strlen("root@root> ");  // or define a macro
    char buf[32];
    snprintf(buf, sizeof(buf), "\033[%dG", prompt_len + editor.cursor_pos + 1); // 1-based
    Shell_Print(buf);
}
