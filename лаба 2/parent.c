#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    char filename[256];
    printf("Введите имя файла: ");
    fflush(stdout);

    // Считываем имя файла
    if (fgets(filename, sizeof(filename), stdin) != NULL) {
        // Удаляем символ новой строки '\n', если он есть
        filename[strcspn(filename, "\n")] = 0;
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        perror("Не удалось открыть файл");
        return 1;
    }

    // Создаём канал pipe1: pipefd[0] - чтение, pipefd[1] - запись
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Ошибка pipe");
        close(file_fd);
        return 1;
    }

    // Создаём дочерний процесс
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        close(file_fd);
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid == 0) {
        // Его stdin теперь - наш файл
        if (dup2(file_fd, STDIN_FILENO) == -1) {
            perror("dup2 (stdin)");
            exit(1);
        }
        // Его stdout теперь - запись в канал
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2 (stdout)");
            exit(1);
        }
        // Закрываем лишние дескрипторы
        close(file_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        // Запускаем другую программу - child
        execl("./child", "child", (char *)NULL);
        perror("Ошибка execl"); // Сюда попадём только если запуск не удался
        exit(1);
    }

    close(file_fd);     // Файл родителю не нужен
    close(pipefd[1]);   // Родитель только читает из канала

    // Читаем всё, что пишет ребёнок, и выводим на экран
    char buf[256];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, n, stdout);
        fflush(stdout);
    }
    if (n == -1) {
        perror("Ошибка чтения из pipe");
    }
    close(pipefd[0]);

    // Ждём завершения ребёнка и смотрим статус
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Ошибка waitpid");
        return 1;
    }
    
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code == 2) {
            printf("Дочерний процесс встретил простое/отрицательное число.\n");
        } else if (code == 1) {
            printf("Дочерний процесс завершился с ошибкой.\n");
            return 1;
        } else {
            printf("Файл обработан полностью.\n");
        }
    }
    return 0;
}