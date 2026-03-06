#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>  /* Para ssize_t */
#include <limits.h>

// Estructura para el historial binario
struct Resumen {
    unsigned int tamano;
    char ruta[PATH_MAX];
};

// Función para guardar en el archivo binario usando llamadas al sistema
void guardar_en_historial(unsigned int tamano, const char *path) {
    struct Resumen registro;
    registro.tamano = tamano;
    strncpy(registro.ruta, path, PATH_MAX - 1);
    registro.ruta[PATH_MAX - 1] = '\0';  // Asegurar terminación nula

    int fd = open("mydu.bin", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        write(fd, &registro, sizeof(struct Resumen));
        close(fd);
    }
}

// Calcular el tamaño
unsigned int calcular_tamano_recursivo(const char *nombre_dir) {
    struct stat st;
    if (lstat(nombre_dir, &st) < 0) return 0;

    // Si no es un directorio, devolvemos su tamaño en KB
    if (!S_ISDIR(st.st_mode)) {
        return (unsigned int)((st.st_size + 1023) / 1024);
    }

    DIR *dir = opendir(nombre_dir);
    if (!dir) return 0;

    struct dirent *entrada;
    unsigned int tamano_total = (unsigned int)((st.st_size + 1023) / 1024);

    while ((entrada = readdir(dir)) != NULL) {
        // Ignorar "." y ".." para evitar bucles infinitos
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
            continue;

        char path_completo[PATH_MAX];
        snprintf(path_completo, sizeof(path_completo), "%s/%s", nombre_dir, entrada->d_name);

        tamano_total += calcular_tamano_recursivo(path_completo);
    }

    closedir(dir);
    printf("%u\t%s\n", tamano_total, nombre_dir); // Mostrar por pantalla
    guardar_en_historial(tamano_total, nombre_dir); // Guardar cada carpeta analizada 
    
    return tamano_total;
}

int main(int argc, char *argv[]) {
    // Modo lectura binario (-b)
    if (argc == 2 && strcmp(argv[1], "-b") == 0) {
        int fd = open("mydu.bin", O_RDONLY);
        if (fd < 0) return -1;

        struct Resumen registro;
        ssize_t bytes_read;
        printf("Contenido del archivo binario\n");
        while ((bytes_read = read(fd, &registro, sizeof(struct Resumen))) > 0) {
            // Validar lectura completa del registro
            if (bytes_read != (ssize_t)sizeof(struct Resumen)) break;
            printf("%u\t%s\n", registro.tamano, registro.ruta);
        }
        close(fd);
        return 0;
    }

    // Modo cálculo
    char *dir_a_analizar = (argc > 1) ? argv[1] : ".";

    // Validar que no sea un fichero regular
    struct stat st_check;
    if (stat(dir_a_analizar, &st_check) == 0 && S_ISREG(st_check.st_mode)) {
        fprintf(stderr, "%s: No es un directorio\n", dir_a_analizar);
        return -1;
    }

    if (calcular_tamano_recursivo(dir_a_analizar) == 0 && argc > 1) {
        return -1;
    }

    return 0;
}
