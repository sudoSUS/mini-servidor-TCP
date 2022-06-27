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

// Prototipos.
int gethostip(char*); // Una función que encontré en Internet.

int main(int argc, char*argv[]){
    /*if (argc!=3){
        fprintf(stderr, "Error.\nModo de uso:\n%s <puerto> <direccion_host>\n");
        exit(EXIT_FAILURE);
    }*/
    int puerto = atoi(argv[1]);
    char* serv_host = argv[2], *PRUEBA;

    int sockfd, connfd; // File descriptors.
    unsigned int len;  // Longitud de la dirección del cliente.
    struct sockaddr_in servaddr, client; //Estructura de <netinet/in.h> para describir direcciones de sockets de Internet.

    int len_rx, len_sx = 0; // Longitud recibida y enviada en bytes.
    char buff_rx[BUF_SIZE], buff_sx[BUF_SIZE]; // Búferes de entrada y salida.

    gethostip(PRUEBA);
    puts(PRUEBA);
    return 0;
}


int gethostip(char *hostip)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char *host = NULL;
 
	if (getifaddrs(&ifaddr) == -1) 
	{
		return ( -1 );
	}
 
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
 
		family = ifa->ifa_addr->sa_family;
 
		if (!strcmp(ifa->ifa_name, "lo"))
			continue;
		if (family == AF_INET) 
		{
			s = getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in) :
					sizeof(struct sockaddr_in6),
					hostip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0)
			{
				return ( -1 );
			}
			freeifaddrs(ifaddr);
			return ( 0 );
		}
	}
	return ( -1 );
}