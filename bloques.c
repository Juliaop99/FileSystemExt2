#include "bloques.h"
int fd;

//Esta funcion es para crear/montar el dispositivo virtual y
//dado que se trata de un fichero, esa acción consistira en abrirlo.
//RETURN: descriptor del fichero o -1 if error.
int bmount(const char *camino) {
    //Llamamos a la llamada del sistema open, para obtener el descriptor del fichero que
    //usaremos como dispositivo virtual.
    fd = open (camino, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
    	perror(RED "Error"); printf(RESET);
    } return fd;
}

//Esta función desmonta el dispositivo virtual. Bàsicamente llama a la función close() para 
//liberar el descriptor de fichero.
int bumount(const char *camino) {
    if (close(fd) == -1) {
	perror(RED "Error"); printf(RESET);
	return -1;
    } return 0;
}

int bwrite(unsigned int nbloque, const void *buf) {
    //Escribe 1 bloque en e1 dispositivo virtual, primero  se posiciona con lseek y luego escribe
    int offset = nbloque * BLOCKSIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
    	return -1;
    }

    if (write(fd, buf, BLOCKSIZE) == -1) {
	return -1;
    }

    return BLOCKSIZE;
}

int bread(unsigned int nbloque, void *buf) {
   int offset = nbloque * BLOCKSIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
    	return -1;
    }

    if (read(fd, buf, BLOCKSIZE) == -1) {
	return -1;
    }

    return BLOCKSIZE;
}
