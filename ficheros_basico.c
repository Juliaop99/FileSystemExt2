#include "ficheros_basico.h"
//En este segundo nivel inicializaremos el superbloque (initSB),
//el mapa de bits (initMB) y el aray de inodos (initAI).
//
//Para delimitar cada una de las áreas del disco virtual necesitaremos
//averiguar cuantos bloques ocupa el mapa de bits (tamMB) y los bloques
//que ocupa el array de inodos (tamAI). El superbloque sabemos que ocupa 1
//bloque, tamSB y lo ubicaremos en el bloque 0, posSB.
//
//Finalmente, actualizaremos la función de formateo del disco virtual, mi_mkfs.c,
//para llamar las funciones de inicialización.

//TamMB calcula el tamaño en bloques necesario para el mapa de bits
int tamMB (unsigned int nbloques) {
	int tamMB = ((nbloques/8)/BLOCKSIZE);
	if (((nbloques/8)%BLOCKSIZE) != 0) {
		return (tamMB+1);
	} return (tamMB);
}

//tamAI calcula el tamaño en bloques del array de inodos
int tamAI (unsigned int ninodos) {
	//Sabiendo que ninodos es nbloques/4
	int tamAI = (ninodos*INODOSIZE)/BLOCKSIZE;
	if (((ninodos*INODOSIZE)%BLOCKSIZE) != 0) {
		return (tamAI+1);
	} return (tamAI);
}

//initSB inicializa los datos del superbloque
int initSB(unsigned int nbloques, unsigned int ninodos) {
	struct superbloque SB;
	SB.posPrimerBloqueMB = posSB + tamSB;
	SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
	SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1; 
	SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
	SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
	SB.posUltimoBloqueDatos = nbloques - 1;

	SB.posInodoRaiz = 0;
	SB.posPrimerInodoLibre = 0; //Inicialmente, 0, tras inicializar el inodo raiz 1 y después va cambiando
	
	SB.cantBloquesLibres = nbloques; //Inicialmente
	SB.cantInodosLibres = ninodos; //Inicialmente
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;

	if (bwrite(posSB, &SB) == -1) {return -1;}
	return 0;
}

//InitMB inicializa todos los bloques que contienen metadatos a 1, es decir, ocupados
int initMB() {
	//Leer superbloque
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) {return -1;}

	int contadorBloqueMB = 0;
	unsigned int bytes_a_escribir;
	unsigned int bloques_ocupados = SB.posUltimoBloqueAI + 1; //tamaño metadatos bits que poner a 1
	unsigned int bytes_ocupados = bloques_ocupados / 8; //bytes que poner a 1
	if (bytes_ocupados % 8 != 0) {bytes_ocupados++;}

	if (bytes_ocupados > BLOCKSIZE) {
		//Obtener el número de bloques completos que hay que poner a 1
		unsigned int bloques_completos = bytes_ocupados / BLOCKSIZE;
		for (int j = 0; j < bloques_completos; j++) {
			unsigned char bufferUnos[BLOCKSIZE];
			memset(bufferUnos, 255, BLOCKSIZE);
			if (bwrite(SB.posPrimerBloqueMB + contadorBloqueMB++, bufferUnos) == -1) {return -1;}
		}
		bytes_a_escribir = bytes_ocupados % BLOCKSIZE;
	} else {bytes_a_escribir = bytes_ocupados;}

	unsigned char bufferMB[BLOCKSIZE];
	//llenamos de unos el buffer
	for (int i = 0; i<bytes_a_escribir; i++) {bufferMB[i] = 255;}
	bufferMB[bytes_a_escribir] = resto(bloques_ocupados % 8);
	//los bits restantes los ponemos a cero
	for (unsigned int i = bytes_a_escribir; i<BLOCKSIZE; i++) {
		bufferMB[i] = 0;
	}
	SB.cantBloquesLibres -= bloques_ocupados;
	if (bwrite(posSB, &SB) == -1) {return -1;}
	if (bwrite(SB.posPrimerBloqueMB + contadorBloqueMB, bufferMB) == -1) {return -1;}
	return 0;
}

int resto (unsigned int num_bits) {
	int exponente = 7;
	int resultado = 0;

	for (int i = 0; i<num_bits; i++) {
		resultado += exponencial(2, exponente);
		exponente--;
	} return resultado;
}

int exponencial (int base, int exponente) {
	int resultado = 1;
	for (int i = 0; i<exponente; i++) {
		resultado *= base;
	} return resultado;
}

//initAI inicializa la lista de inodos libres
int initAI() {
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
			inodos[j].tipo = 'l';

			if (contadorInodos == SB.totInodos-1) {
				inodos[j].punterosDirectos[0] = UINT_MAX;
			} else {
				inodos[j].punterosDirectos[0] = contadorInodos + 1;
			}
			contadorInodos++;
		}

		//Escribir bloque actualizado
		if (bwrite(i, inodos) == -1) {return -1;}
	}
	return 0;
}



