#include "myshell.h"

int main(int argc, char *argv[])
{
    /* Получаем полный путь к исполняемому файлу myshell */
    char *shell_path = get_executable_path(argv[0]);
    set_shell_environment(shell_path);

    if (argc > 1) {
        /* Режим batch-файла */
        FILE *batch = fopen(argv[1], "r");
        if (batch == NULL) {
            perror("Не удалось открыть batch-файл");
            exit(1);
        }
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), batch) != NULL) {
            char *args[MAX_ARGS];
            int num_args = parse_line(line, args);
            if (num_args > 0) {
                if (!handle_builtins(args, num_args, shell_path))
                    execute_external(args, shell_path);
            }
        }
        fclose(batch);
    } else {
        /* Интерактивный режим */
        char line[MAX_LINE];
        while (1) {
            print_prompt();
            if (fgets(line, sizeof(line), stdin) == NULL)
                break;
            char *args[MAX_ARGS];
            int num_args = parse_line(line, args);
            if (num_args > 0) {
                if (!handle_builtins(args, num_args, shell_path))
                    execute_external(args, shell_path);
            }
        }
    }
    return 0;
}