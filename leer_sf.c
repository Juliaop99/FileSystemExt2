//Hay que montar y desmontar el dispostivo virtual y hacer un include de ficheros basico.h
#include "ficheros_basico.h"

void sizeOfInodo();
int mostrarPunterosDirectos0();
int mostrarSuperbloque();
int reservarYLiberarBloque();
int mostrarBloquesMetadatosMB();
int mostrarDatosDirectorioRaiz();

int main(int argc, char **argv) {
	//argv[0] = "leer_sf"
	//argv[1] = nombre disp

	//Montar disp
	bmount(argv[1]);

	//sizeOfInodo();
	mostrarSuperbloque();
	//mostrarPunterosDirectos0();
	reservarYLiberarBloque();
	mostrarBloquesMetadatosMB();
	mostrarDatosDirectorioRaiz();

	//Desmontar disp
	bumount();
}

void sizeOfInodo() {
	printf("Sizeof struct inodo is: %lu \n", sizeof(struct inodo));
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

int reservarYLiberarBloque() {
	printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
	unsigned int nbloqueReservado = reservar_bloque();
	printf("El bloque fisico reservado ha sido el %d \n", nbloqueReservado);
	
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) {return -1;}
	printf("Número de bloques libres: %u \n", SB.cantBloquesLibres);
	
	liberar_bloque(nbloqueReservado);
	if (bread(posSB, &SB) == -1) {return -1;}
	printf("Tras liberar el bloque reservado anteriormente, el numero de bloques libres es: %u \n", SB.cantBloquesLibres);
	return 0;
}

int mostrarBloquesMetadatosMB() {
	struct superbloque SB;
	bread(posSB, &SB);

	printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

	// SB
	printf("posSB: %d → leer_bit(%d) = %d\n",
		posSB, posSB, leer_bit(posSB));

	// MB
	printf("SB.posPrimerBloqueMB: %d → leer_bit(%d) = %d\n",
		SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));

	printf("SB.posUltimoBloqueMB: %d → leer_bit(%d) = %d\n",
		SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));

	// AI
	printf("SB.posPrimerBloqueAI: %d → leer_bit(%d) = %d\n",
		SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));

	printf("SB.posUltimoBloqueAI: %d → leer_bit(%d) = %d\n",
		SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));

	// DATOS
	printf("SB.posPrimerBloqueDatos: %d → leer_bit(%d) = %d\n",
		SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));

	printf("SB.posUltimoBloqueDatos: %d → leer_bit(%d) = %d\n",
		SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));
	return 0;
}

int mostrarDatosDirectorioRaiz() {
	printf("\nDATOS DEL DIRECTORIO RAIZ\n");

	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	char btime[80];

	struct inodo inodo;
	int ninodo=0;

	leer_inodo(ninodo, &inodo);
	printf("tipo: %c\n",inodo.tipo);
	printf("permisos: %d\n",inodo.permisos);

	ts = localtime(&inodo.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S\n", ts);
	ts = localtime(&inodo.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S\n", ts);
	ts = localtime(&inodo.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S\n", ts);
	ts = localtime(&inodo.btime);
	strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S\n", ts);
	printf("ID: %d\nATIME: %sMTIME: %sCTIME: %sBTIME: %s",ninodo,atime,mtime,ctime,btime);

	printf("nlinks: %d\n",inodo.nlinks);
	printf("tamEnBytesLog: %d\n",inodo.tamEnBytesLog);
	printf("numBloquesOcupados: %d\n",inodo.numBloquesOcupados);
	return 0;
}