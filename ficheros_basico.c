#include "ficheros_basico.h"


//tamMB calcula el tamaño en bloques necesario para el mapa de bits
unsigned int tamMB (const unsigned int nbloques) {
	const unsigned int tamMB = nbloques / 8 / BLOCKSIZE;
	if (nbloques / 8 % BLOCKSIZE != 0) {
		return tamMB + 1;
	}
	return tamMB;
}

//tamAI calcula el tamaño en bloques del array de inodos
unsigned int tamAI (const unsigned int ninodos) {
	//sabiendo que ninodos es nbloques/4
	const unsigned int tamAI = (ninodos*INODOSIZE)/BLOCKSIZE;
	if (ninodos * INODOSIZE % BLOCKSIZE != 0) {
		return tamAI + 1;
	} return tamAI;
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
	SB.posPrimerInodoLibre = 0; 
	
	SB.cantBloquesLibres = nbloques; 
	SB.cantInodosLibres = ninodos; 
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;

	if (bwrite(posSB, &SB) == -1) {return -1;}
	return 0;
}

//initMB inicializa todos los bloques que contienen metadatos a 1, es decir, ocupados
int initMB() {
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) {return -1;}

	//Es decir tenemos que poner tantos bits a 1 como bloquesMetadatos haya
	unsigned int bits_a_escribir = tamAI(SB.totBloques/4) + tamMB(SB.totBloques) + tamSB;
	unsigned int bloques_enteros =  bits_a_escribir / (8 * BLOCKSIZE);
	unsigned int bits_restantes = bits_a_escribir % (8 * BLOCKSIZE);
	unsigned char bufferAuxiliar[BLOCKSIZE];
	int contador = 0;

	//Escribimos los bloques de unos completos (Si los hay)
	if (bloques_enteros != 0) {
		memset(bufferAuxiliar,255,BLOCKSIZE);
		while (bloques_enteros != 0) {
			if (bwrite(SB.posPrimerBloqueMB + contador, bufferAuxiliar) == -1) {return -1;}
			contador++;
			bloques_enteros--;
		}
	}

	//Escribimos el bloque final
	unsigned int bytes_completos = bits_restantes / 8;
	unsigned int bits_sobrantes = bits_restantes % 8;
	memset(bufferAuxiliar,0,BLOCKSIZE);
	for (int i = 0; i < bytes_completos; i++) {
		bufferAuxiliar[i] = 255;
	}
	bufferAuxiliar[bytes_completos] = 255 << (8 - bits_sobrantes);
	if (bwrite(SB.posPrimerBloqueMB + contador, bufferAuxiliar) == -1) {return -1;}

	SB.cantBloquesLibres = SB.cantBloquesLibres - bits_a_escribir;
	if (bwrite(posSB, &SB) == -1) {return -1;}
	return 0;
}

int resto (unsigned int num_bits) {
	if (num_bits == 0) return 255;  //Caso especial

	//Caso general
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
	//leer superbloque
	struct superbloque SB;
	if (bread(posSB,&SB) == -1) {return -1;}

	//crear buffer de inodos
	struct inodo inodos[BLOCKSIZE / INODOSIZE];

	unsigned int contadorInodos = 0;
	//recorremos todos los bloques de AI
	for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
		//leemos el bloque actual de inodos
		if (bread(i, inodos) == -1) {return -1;}

		//para cada nodo tipo = l y punterosDirectos[0] = UINT_MAX;
		for (unsigned int j = 0; j < BLOCKSIZE/INODOSIZE; j++) {
			inodos[j].tipo = 'l';

			if (contadorInodos == SB.totInodos-1) {
				inodos[j].punterosDirectos[0] = UINT_MAX;
			} else {
				inodos[j].punterosDirectos[0] = contadorInodos + 1;
			}
			contadorInodos++;
		}

		//escribir bloque actualizado
		if (bwrite(i, inodos) == -1) {return -1;}
	}
	return 0;
}

//escribir_bit escribe el parametro bit en el bit del MB que representa nbloque
//lo usaremos para reservar y liberar bloques
int escribir_bit(unsigned int nbloque, unsigned int bit) {
	//leer el superbloque para obtener la localizacion del MB
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) { return -1;}
	
	//calculamos que byte del MB (posbyteMB) que contiene el bit
	//y la posicion del bit dentro de ese byte (posbit)
	unsigned int posbyteMB = nbloque / 8;
	unsigned int posbit = nbloque % 8;
	//determinamos en que bloque del MB (nbloqueMB) se halla este byte
	unsigned int nbloqueMB = posbyteMB / BLOCKSIZE;
	//finalmente hemos de obtener en que posicion absoluta del dispositivo virtual se
	//encuentra ese bloque
	unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

	//leemos el bloque del MB que contiene el bit que representa el bloque fisico
	unsigned char bufferMB[BLOCKSIZE];
	if(bread(nbloqueabs, bufferMB) == -1) { return -1;}
	unsigned int posbyte = posbyteMB % BLOCKSIZE;

	//usando una máscara y el operador binario or o and podemos escribir un 0 o un 1
	//en un bit concretoconcreto
	unsigned char mascara = 128;
	mascara>>=posbit;
	if (bit == 1) {
		bufferMB[posbyte] |= mascara; //OR
	} else {
		bufferMB[posbyte] &= ~mascara; //AND y NOT
	}

	//finalmente escribimos el buffer en el dispositivo
	if (bwrite(nbloqueabs, bufferMB) == -1) { return -1;}
	return 0;
}

//leer_bit lee un determinado bit del MB y devuelve su valor
char leer_bit(unsigned int nbloque) {
	//repetimos el mismo procedimiento que en escribir_bit para obtener la posicion del bit
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) { return (char)-1;}
	
	unsigned int posbyteMB = nbloque / 8;
	unsigned int posbit = nbloque % 8;
	unsigned int nbloqueMB = posbyteMB / BLOCKSIZE;
	unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

	unsigned char bufferMB[BLOCKSIZE];
	if(bread(nbloqueabs, bufferMB) == -1) { return (char)-1;}
	unsigned posbyte = posbyteMB % BLOCKSIZE;
	
	//volvemos a crear una mascara, la desplazamos posbit y hacemos una AND
	//binaria con el contenido del buffer y desplazamos el bit a la derecha para leerlo.
	unsigned char mascara = 128;
	mascara >>= posbit;
	mascara &= bufferMB[posbyte];
	mascara >>= (7-posbit);
	return (char)mascara;
}

//reservar_bloque encuentra el primer bloque libre consultando el MB, lo ocupa
//con escribir_bit() y devuelve su posicion
int reservar_bloque() {
	//leemos el superbloque
	struct superbloque SB;
	if (bread(posSB,&SB) == -1) { return -1;}

	//comprovamos si quedan bloques libres
	if (SB.cantBloquesLibres == 0) { 
		printf("No quedan bloques libres\n");
		return -1;
	}
	
	//iteramos nbloqueMB, hasta encontrar uno que tenga algun 0
	int nbloqueMB = 0;
	unsigned char bufferMB[BLOCKSIZE];
	unsigned char bufferAux[BLOCKSIZE];
	memset(bufferAux, 255,BLOCKSIZE); //Lo llenamos de unos
	int primerCero = 0;

	while(primerCero == 0) {
		//actualizamos el buffer
		if(bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB) == -1) {return -1;}
		//comparamos
		if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {primerCero = 1;}
		else {nbloqueMB++;}
	}

	//al salir del bucle en bufferMB tenemos el bloque que contiene el primer cero
	//buscamos en ese bloque la posicion del primer byte que tenga algun 0
	int nbyteMB = 0;
	primerCero = 0;
	while (!primerCero) {
		if (bufferMB[nbyteMB] != 255) {
			primerCero = 1;
		} else { nbyteMB++;}
	}

	//ahora localizamos el bit concreto dentro del byte
	unsigned char mascara = 128;
	int posbit = 0;

	while (bufferMB[nbyteMB] & mascara) {
		bufferMB[nbyteMB] <<= 1;
		posbit++;
	}

	//finalmente determinamos el numero de bloque fisico, usamos escribir_bit,
	//actualizamos el superbloque, limpiamos el bloque en la zona de datos por
	//si habia basura y devolvemos el nº de bloque que hemos reservado, nbloque
	int nbloque = (nbloqueMB * BLOCKSIZE + nbyteMB) * 8 + posbit;
	escribir_bit(nbloque, 1);

	SB.cantBloquesLibres--;
	if (bwrite(posSB, &SB) == -1) { return -1;}

	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	if (bwrite(nbloque, buffer) == -1) { return -1;}

	return nbloque;
}

//liberar_bloque sirve para liberar un bloque dado
unsigned int liberar_bloque(unsigned int nbloque) {
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) {return -1;}
	escribir_bit(nbloque, 0);
	SB.cantBloquesLibres++;
	if (bwrite(posSB, &SB) == -1) {return -1;}
	return nbloque;
}

//escribir_inodo escribe el contenido de una variable struct inodo pasada por referencia
//en un inodo del array de inodos (como la escritura se hace por bloques hay que preservar
//el valor de los demas inodos del bloque)
int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
	//leemos el superbloque
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) { return -1;}

	//calculamos la poicion absoluta del inodo y lo escribimos
	unsigned int nbloqueAI = (ninodo * sizeof(struct inodo)) / BLOCKSIZE;
	unsigned int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE / INODOSIZE];
	if(bread(nbloqueabs, inodos) == -1) { return -1;}
	const unsigned int posinodo = ninodo % (BLOCKSIZE / sizeof(struct inodo));
	inodos[posinodo] = *inodo;
	if(bwrite(nbloqueabs, inodos) == -1) { return -1;}
	return 0;
}


//leer_inodo lee un determinado inodo del array de inodos para volcarlo en
//una variable struct inodo pasada por referencia
int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
	//leemos el superbloque
	struct superbloque SB;
	if (bread(posSB, &SB) == -1) {return -1;}

	//calculamos la poicion absoluta del inodo y lo escribimos
	unsigned int nbloqueAI = (ninodo * sizeof(struct inodo)) / BLOCKSIZE;
	unsigned int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE / INODOSIZE];
	if(bread(nbloqueabs, inodos) == -1) { return -1;}
	unsigned int posinodo = ninodo % (BLOCKSIZE / sizeof(struct inodo));
	*inodo = inodos[posinodo];
	return 0;
}

//reservar_inodo encuentra el primer inodo libre, lo reserva, devuelve su numero y actualiza la 
unsigned int reservar_inodo(unsigned char tipo, unsigned char permisos) {
	//leer superbloque
	struct superbloque SB;
	if(bread(posSB, &SB) == -1) { return -1;}
	
	if(SB.cantInodosLibres == 0) { return -1;}
	unsigned int posInodoReservado = SB.posPrimerInodoLibre;

	//Leemos el inodo que vamos a modificar
	struct inodo inodoLibre;
	if (leer_inodo(posInodoReservado, &inodoLibre) == -1) {return -1;}
	//Actualizamos la lista enlazada
	SB.posPrimerInodoLibre = inodoLibre.punterosDirectos[0];

	struct inodo inodoAreservar;
	inodoAreservar.tipo = tipo;
	inodoAreservar.permisos = permisos;
	inodoAreservar.nlinks = 1;
	inodoAreservar.tamEnBytesLog = 0;
	inodoAreservar.atime = time(NULL);
	inodoAreservar.mtime = time(NULL);
	inodoAreservar.ctime = time(NULL);
	inodoAreservar.btime = time(NULL);
	inodoAreservar.numBloquesOcupados = 0;
	memset(inodoAreservar.punterosDirectos, 0, sizeof(inodoAreservar.punterosDirectos));
	memset(inodoAreservar.punterosIndirectos, 0, sizeof(inodoAreservar.punterosIndirectos));
	
	if(escribir_inodo(posInodoReservado, &inodoAreservar) == -1) { return -1;}

	SB.cantInodosLibres--;
	if(bwrite(posSB, &SB) == -1) { return -1;}
	return posInodoReservado;
}
