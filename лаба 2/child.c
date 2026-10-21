#include <stdio.h>
#include <stdlib.h>

// Функция проверки числа на составное
int is_composite(int n) {
    if (n < 4) return 0; // false (не составное)
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return 1; // true (составное)
    }
    return 0; // false
}

int main() {
    int n;
    // Считываем числа из стандартного ввода (который перенаправлен на файл)
    while (scanf("%d", &n) == 1) {
        if (is_composite(n)) {
            printf("%d\n", n);
            fflush(stdout); // Обязательно сбрасываем буфер вывода в pipe
        } else {
            exit(2); // Если число простое или отрицательное, выходим с кодом 2
        }
    }
    return 0;
}