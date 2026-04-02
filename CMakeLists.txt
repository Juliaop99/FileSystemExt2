cmake_minimum_required(VERSION 4.1)
project(FileSystem C)

set(CMAKE_C_STANDARD 17)

add_executable(FileSystem bloques.c
        bloques.h
        mi_mkfs.c
        ficheros_basico.c
        ficheros_basico.h
        leer_sf.c
)
