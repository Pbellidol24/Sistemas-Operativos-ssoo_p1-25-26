#include <fcntl.h>   
#include <unistd.h>   /* Para read, write, close, lseek */
#include <stdlib.h>   /* Para atoi */
#include <string.h>   /* Para strlen y strcmp */
#include <limits.h>   /* Para INT_MIN */

// Función auxiliar para imprimir cadenas
void print_string(int fd, const char *str) {
    write(fd, str, strlen(str)); // Se usa la llamada write en lugar de printf 
}

// Función para convertir un entero a cadena (porque no tenemos sprintf)
void int_to_str(int n, char *buf) {
    int i = 0, j;
    int is_negative = 0;
    unsigned int un;

    if (n == 0) { buf[i++] = '0'; buf[i] = '\0'; return; }
    
    // Manejar negativos incluyendo INT_MIN sin overflow
    if (n < 0) {
        is_negative = 1;
        un = (unsigned int)(-(n + 1)) + 1u;  // Evita overflow con INT_MIN
    } else {
        un = (unsigned int)n;
    }
    
    while (un != 0) {
        buf[i++] = (char)((un % 10) + '0');
        un /= 10;
    }
    if (is_negative) buf[i++] = '-';
    buf[i] = '\0';
    // Invertir la cadena
    for (j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}

int main(int argc, char *argv[]) {
    // 1. Verificación de argumentos
    if (argc < 3) {
        print_string(2, "Uso (1): ./mycalc <num1> <operacion> <num2>\n");
        print_string(2, "Uso (2): ./mycalc -b <num_operacion>\n");
        return -1; // Error si no hay argumentos suficientes
    }

    // Modo historial (-b)
    if (strcmp(argv[1], "-b") == 0) {
        if (argc != 3) return -1;
        int target_line = atoi(argv[2]);
        if (target_line <= 0) {
            print_string(2, "Error: El número de línea no es válido\n");
            return -1;
        }

        int fd = open("mycalc.log", O_RDONLY);
        if (fd < 0) return -1;

        char c;
        int current_line = 1;
        // Leer el archivo carácter por carácter para contar líneas
        while (read(fd, &c, 1) > 0) {
            if (current_line == target_line) {
                // Si estamos en la línea buscada, imprimimos el contenido
                if (c == '\n') break;
                write(1, &c, 1);
            }
            if (c == '\n') current_line++;
        }
        close(fd);
        write(1, "\n", 1);
        if (current_line < target_line) {
             print_string(2, "Error: El número de línea no es válido\n");
             return -1;
        }
        return 0;
    }

    // Modo interactivo
    if (argc == 4) {
        // Validar que la operación tenga exactamente 1 carácter
        if (strlen(argv[2]) != 1) {
            print_string(2, "Error: Operación no válida\n");
            return -1;
        }
        int num1 = atoi(argv[1]);
        char op = argv[2][0];
        int num2 = atoi(argv[3]);
        int res = 0;

        // Procesar operación
        if (op == '+') res = num1 + num2;
        else if (op == '-') res = num1 - num2;
        else if (op == 'x') res = num1 * num2;
        else if (op == '/') {
            if (num2 == 0) {
                print_string(2, "Error: División por cero\n");
                return -1;
            }
            res = num1 / num2;
        } else {
            print_string(2, "Error: Operación no válida\n");
            return -1;
        }

        // Cadena de salida:
        char s_res[20], s_n1[20], s_n2[20];
        int_to_str(num1, s_n1);
        int_to_str(num2, s_n2);
        int_to_str(res, s_res);

        // Mostrar por pantalla
        print_string(1, "Operación: ");
        print_string(1, s_n1); write(1, &op, 1); print_string(1, s_n2);
        print_string(1, "="); print_string(1, s_res); write(1, "\n", 1);

        // Guardar en el log
        int fd_log = open("mycalc.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd_log >= 0) {
            write(fd_log, "Operación: ", 12);
            write(fd_log, s_n1, strlen(s_n1));
            write(fd_log, &op, 1);
            write(fd_log, s_n2, strlen(s_n2));
            write(fd_log, "=", 1);
            write(fd_log, s_res, strlen(s_res));
            write(fd_log, "\n", 1);
            close(fd_log);
        }
        return 0;
    }

    return -1;
}
