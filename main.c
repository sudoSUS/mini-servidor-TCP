// Estructurar el programa en archivos diferentes (archivos de cabecera y main).

#include <unistd.h> // Para el uso de file decriptors, aparentemente.

#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>

#include<ifaddrs.h>

#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define BUF_SIZE 100 // Tamaño del búfer de entrada y salida. Implementar dinámicamente proximamente.
#define BACKLOG 5 // Número máximo de clientes.


int main(int argc, char*argv[]){
    if (argc!=3){
        fprintf(stderr, "Error.\nModo de uso:\n%s <puerto> <direccion_host>\n");
        exit(EXIT_FAILURE);
    }
    int puerto = atoi(argv[1]);
    char* serv_host = argv[2];

    int sockfd, connfd; // File descriptors.
    unsigned int len;  // Longitud de la dirección del cliente.
    struct sockaddr_in servaddr, client; //Estructura de <netinet/in.h> para describir direcciones de sockets de Internet.

    int len_rx, len_sx = 0; // Longitud recibida y enviada en bytes.
    char buff_rx[BUF_SIZE], buff_sx[BUF_SIZE]; // Búferes de entrada y salida.

    return 0;
}

