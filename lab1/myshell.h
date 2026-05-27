#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

extern char **environ;

#define MAX_LINE 1024
#define MAX_ARGS 64

/* Прототипы */
void print_prompt(void);
int parse_line(char *line, char **args);
int handle_builtins(char **args, int num_args, char *shell_path);
void execute_external(char **args, char *shell_path);
char *get_executable_path(char *argv0);
void set_shell_environment(char *path);
void list_directory(char *dir_name);

#endif