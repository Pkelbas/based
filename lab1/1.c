#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int main(int argc, char *argv[]) {
    int count_of_bytes = 0;
    int words = 0;
    int strings = 0;

    if (argc < 3) {
        printf("Sorry, I can't do anything. Try again");
        exit(1);
    }

    // Подключение файлa
    FILE *file_lab1;
    file_lab1 = fopen(argv[argc - 1], "rb");
    if (file_lab1 == NULL){
        printf("Sorry, give me another file.");
        exit(1);
    }

    char prev_symbol = fgetc(file_lab1);
    while (prev_symbol != EOF) {
        char cur_symbol = fgetc(file_lab1);
        if (!isspace(prev_symbol) && (isspace(cur_symbol) || cur_symbol == EOF)) {
            words++;
        }
        if (prev_symbol == '\n' || cur_symbol == EOF) {
            strings++;
        }

        count_of_bytes++;
        prev_symbol = cur_symbol;
    }

    fclose(file_lab1);

    // обработка аргументов командной строки
    for (int i = 1; i < argc - 1; ++i) {
        if ((strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "--words") == 0)) {
            printf("%d\n", words);
        } else if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--bytes") == 0)) {
            printf("%d\n", count_of_bytes);
        } else if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--lines") == 0)) {
            printf("%d\n", strings);
        } else {
            printf("Sorry, I don't know this option\n");
        }
    }

    return 0;
}

