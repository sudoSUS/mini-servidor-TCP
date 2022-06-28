#include"servidor-cliente.h"

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>

#include<ifaddrs.h>
#include<netdb.h>

#include<arpa/inet.h>




// Guarda la IP local en la dirección del argumento.
int gethostip(char *hostip)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
 
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

/*
bool allint(char* string){
	// Retorna "true" si todos los caracteres son dígitos, de lo contrario, "false".
	for(int i=0;i<strlen(string);i++) if !(isdigit(string[i])) return false;
	return true;
}
*/


/*
Llamada al sistema socket.
Primer argumento: Puntero a variable contenedora del file descriptor del socket.
Segundo argumento: Tipo de socket TCP (servidor o cliente).
*/
t_estado f_socket(int *sockfd, char* est){ 
	if((*sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
		fprintf(stderr, "[%s-error]: creación del socket fallida. %d: %s\n",est,errno, strterror(errno));
		return ERROR;
	}
	printf("[%s]: socket creado satisfactoriamente.\n",est);
	return OK;
}

void initservaddr(struct sockaddr_in *servaddr, char* IPv4, unsigned int puerto){
	memset(servaddr, 0, sizeof(servaddr));

	servaddr->sin_family = AF_INET; // Familia IPv4.
	servaddr->sin_addr.s_addr = inet_addr(IPv4); // Función de <arpa/inet.h>.
	servaddr->sin_port = htons(puerto); // Función de <arpa/inet.h>. Cambia el orden de bytes al formato de la red.
}

t_estado f_bind(int sockfd, struct sockaddr_in *servaddr, char* est){
	if((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
		fprintf(stderr, "[%s-error]: llamada bind fallida. %d: %s\n", est, errno, strerror(errno));
		return ERROR;
	}
	printf("[%s]: llamada bind realizada satisfactoriamente.\n",est);
	return OK;
}

t_estado f_listen(int sockfd, struct sockaddr_in servaddr, int maxClients, char* est){
	if ((listen(sockfd, maxClients)) != 0) {
		fprintf(stderr,"[%s-error]: estado de escucha fallido. %d: %s\n", est, errno, strerror(errno));
		return ERROR;
	}
	printf("[%s]: escuchando en el SERV_PORT %d\n\n", ntohs(servaddr.sin_port));
	return OK;
}


/*
Se mantiene a la espera de una conección de algún cliente.
Se retorna un file descriptor de un socket que permite la conección al cliente.
Se guardan los datos del cliente en la estructura del parámetro CLIENT.
*/
int f_accept(int sockfd, struct sockaddr_in *client){
	int len = sizeof(*client), \
	conectionfd = accept(sockfd, (struct sockaddr*) client, &len);

	return conectionfd;

}

