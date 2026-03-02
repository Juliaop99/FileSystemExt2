//Hay que montar y desmontar el dispostivo virtual y hacer un include de ficheros basico.h
#include "ficheros_basico.h"

void sizeOfInodo();
int mostrarPunterosDirectos0();
int mostrarSuperbloque();

int main(int argc, char **argv) {
	//argv[0] = "leer_sf"
	//argv[1] = nombre disp

	//Montar disp
	bmount(argv[1]);

	sizeOfInodo();
	mostrarPunterosDirectos0();
    mostrarSuperbloque();

	//Desmontar disp
	bumount();
}

void sizeOfInodo() {
	printf("sizeof struct inodo is: %lu \n", sizeof(struct inodo));
}

int mostrarPunterosDirectos0() {
	//Leer superbloque
	struct superbloque SB;
	if (bread(posSB,&SB) == -1) {return -1;}

	//Crear buffer de inodos
	struct inodo inodos[BLOCKSIZE / INODOSIZE];

	unsigned int contadorInodos = 0;
	//Recorremos todos los bloques de AI
	for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
		//Leemos el bloque actual de inodos
		if (bread(i, inodos) == -1) {return -1;}

		//Para cada nodo tipo = l y punterosDirectos[0] = UINT_MAX;
		for (unsigned int j = 0; j < BLOCKSIZE/INODOSIZE; j++) {
			printf("Inodo %u -> punteroDirecto[0]: %u\n",
				   contadorInodos + j,
				   inodos[j].punterosDirectos[0]);		}
	}
	return 0;
}

int mostrarSuperbloque() {

	struct superbloque SB;

	if (bread(posSB, &SB) == -1) {
		perror("Error al leer el superbloque");
		return -1;
	}

	printf("\n========= SUPERBLOQUE =========\n");

	printf("posPrimerBloqueMB:      %u\n", SB.posPrimerBloqueMB);
	printf("posUltimoBloqueMB:      %u\n", SB.posUltimoBloqueMB);

	printf("posPrimerBloqueAI:      %u\n", SB.posPrimerBloqueAI);
	printf("posUltimoBloqueAI:      %u\n", SB.posUltimoBloqueAI);

	printf("posPrimerBloqueDatos:   %u\n", SB.posPrimerBloqueDatos);
	printf("posUltimoBloqueDatos:   %u\n", SB.posUltimoBloqueDatos);

	printf("posInodoRaiz:           %u\n", SB.posInodoRaiz);
	printf("posPrimerInodoLibre:    %u\n", SB.posPrimerInodoLibre);

	printf("cantBloquesLibres:      %u\n", SB.cantBloquesLibres);
	printf("cantInodosLibres:       %u\n", SB.cantInodosLibres);

	printf("totBloques:             %u\n", SB.totBloques);
	printf("totInodos:              %u\n", SB.totInodos);

	printf("================================\n");

	return 0;
}