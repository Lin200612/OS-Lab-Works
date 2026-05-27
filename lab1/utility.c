#include "myshell.h"

/* Вывод приглашения (текущий каталог> ) */
void print_prompt(void)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s> ", cwd);
    else
        printf("myshell> ");
}

/* Разбор строки на аргументы (strtok автоматически сжимает пробелы/табы) */
int parse_line(char *line, char **args)
{
    int count = 0;
    char *token = strtok(line, " \t\r\n");
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[count] = NULL;
    return count;
}

/* Простой вывод содержимого каталога */
void list_directory(char *dir_name)
{
    DIR *dir = opendir(dir_name ? dir_name : ".");
    if (dir == NULL) {
        perror("dir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        printf("%s\n", entry->d_name);
    closedir(dir);
}

/* Обработка встроенных команд */
int handle_builtins(char **args, int num_args, char *shell_path)
{
    if (strcmp(args[0], "cd") == 0) {
        if (num_args == 1) {                          /* cd без аргумента */
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);
        } else {
            if (chdir(args[1]) == 0) {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                    setenv("PWD", cwd, 1);
            } else
                perror("cd");
        }
        return 1;
    }
    if (strcmp(args[0], "clr") == 0) {
        printf("\033[2J\033[H");   /* ANSI-очистка экрана */
        return 1;
    }
    if (strcmp(args[0], "dir") == 0) {
        list_directory(num_args > 1 ? args[1] : NULL);
        return 1;
    }
    if (strcmp(args[0], "environ") == 0) {
        char **env = environ;
        while (*env) {
            printf("%s\n", *env);
            env++;
        }
        return 1;
    }
    if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; i < num_args; i++) {
            printf("%s", args[i]);
            if (i < num_args - 1) printf(" ");
        }
        printf("\n");
        return 1;
    }
    if (strcmp(args[0], "help") == 0) {
        printf("=== Руководство пользователя myshell ===\n"
               "cd [directory]     - смена каталога (без аргумента — показать текущий)\n"
               "clr                - очистка экрана\n"
               "dir [directory]    - содержимое каталога\n"
               "environ            - все переменные окружения\n"
               "echo <comment>     - вывод строки\n"
               "help               - это руководство\n"
               "pause              - пауза до нажатия Enter\n"
               "quit               - выход из оболочки\n"
               "Все остальные команды запускаются как внешние программы.\n"
               "Подробное описание концепций — в файле readme.\n");
        return 1;
    }
    if (strcmp(args[0], "pause") == 0) {
        printf("Нажмите Enter для продолжения...\n");
        while (getchar() != '\n');
        return 1;
    }
    if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }
    return 0;   /* не встроенная команда */
}

/* Запуск внешней программы через fork + exec */
void execute_external(char **args, char *shell_path)
{
    pid_t pid = fork();
    if (pid == 0) {                     /* дочерний процесс */
        setenv("parent", shell_path, 1);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {               /* родитель */
        wait(NULL);
    } else {
        perror("fork");
    }
}

/* Полный путь к myshell (realpath) */
char *get_executable_path(char *argv0)
{
    static char path[PATH_MAX];
    if (realpath(argv0, path) == NULL)
        strncpy(path, argv0, PATH_MAX - 1);
    return path;
}

/* Установка переменной shell=... */
void set_shell_environment(char *path)
{
    setenv("shell", path, 1);
}