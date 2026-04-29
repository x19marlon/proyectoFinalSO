#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){


      printf("Iniciando...\n");
    sleep(1);

    printf("Cargando datos...\n");
    sleep(2);

    printf("Presiona Enter para continuar...");
    getchar();

    printf("Bienvenido!\n");

    printf("\033[1;34m"); 
printf(
"░█▀█░█▀█░█▀▀░▀█▀░█░█\n"
"░█▀▀░█░█░▀▀█░░█░░▄▀▄\n"
"░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░▀\n"
);
printf("\033[0m"); 

printf("\033[1;31mMarlon Garcia\n");     
printf("\033[1;34mJuan Pene\n");          
printf("\033[1;35mMariaPeneCucadura\n");  
printf("\033[0m"); 

    return 0;
}