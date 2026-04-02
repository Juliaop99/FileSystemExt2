#include "ficheros_basico.h"

int main (int argc, char **argv) {
    /* argc = 3
     *
     * argv[0] = "mi_mkfs"
     * argv[1] = nombre_dispositivo
     * argv[2] = nbloques (atoi() para obtener el valor numerico a partir del string)
     */

    //Primer paso: Montar el dispositivo virtual con bmount
    bmount(argv[1]);

    // Llamar a bwrite el numero de veces necesario e inicializar el contenido
    int nbloque = atoi(argv[2]);
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    for (int i = 0; i < nbloque; i++) {
        bwrite(i, buffer);
    }

    //Inicializamos los diferentes canpos del disco
    initSB(nbloque,(nbloque / 4));
    initMB();
    initAI();

    //Creamos el directorio raiz
    if (reservar_inodo('d', 7) == -1) {
        printf("Error al reservar inodo raiz");
    }

    //Finalmente: Desmontar el dispositivo virtual con bumount()
    bumount();
}
