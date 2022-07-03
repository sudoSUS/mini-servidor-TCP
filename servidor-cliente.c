#include"servidor-cliente.h"

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>

#include<ncurses.h>

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
		fprintf(stderr, "[%s-error]: creación del socket fallida. %d: %s\n",est,errno, strerror(errno));
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


/*
Los argumentos que se reciben son el porcentaje de la pantalla que usará la ventana del menú.
El argumento CENTRADO define si las opciones estarán centradas o hacia la izquierda:
	0: izquierda.
	1: centro.
*/
t_estado menu_principal(float alto_porc, float ancho_porc, short centrado){
	if (alto_porc>100 || ancho_porc>100 || alto_porc<0 || ancho_porc<0) return ERROR; 
	// Comprueba si las entradas son válidas.
	
	initscr(); cbreak(); noecho(); curs_set(0);

	int h,w;
	getmaxyx(stdscr, h, w);
	h++,w++; // Comentar la línea si getmaxyx retorna el tamaño y no la posición.

	alto_porc/=100, ancho_porc/=100; // Para no dividir entre 100 cada vez.
	// Los parámetros de la posición de newwin.
	int centro_horiz=(w/2.0-w/2)?w/2+1:w/2, // Posición de la mitad de las columnas.
	nlines=((int)(h*alto_porc)%2)?h*alto_porc:h*alto_porc+1,
	ncols=((int)(w*ancho_porc)%2)?w*ancho_porc:w*ancho_porc+1,
	begin_y = h-h*alto_porc,
	begin_x = ((centro_horiz-ancho_porc*w/2.0)-(centro_horiz-ancho_porc*w/2))? (centro_horiz-ancho_porc*w/2)+1: \
	(centro_horiz-ancho_porc*w/2); 

	//printw("Centro:%d - nlines:%d - ncols:%d - begin_y:%d - begin_x:%d - h:%d - w:%d - alt_por:%f - ancho_por:%f",centro_horiz,nlines,ncols,begin_y,begin_x,h,w,alto_porc,ancho_porc);
	
	WINDOW* win = newwin(nlines, ncols, begin_y, begin_x);
	keypad(win,TRUE);
	box(win,0,0);

	char* opciones[]={ // El primer elemento será el título del menú.
		"Menú",
		"Ejecutar servidor",
		"Conectarse a servidor",
	};
	int len_menu=sizeof(opciones)/sizeof(opciones[0]); // Revisar si hay un error porque tomó el puntero como un array.

	char loop=1; // Usé char en lugar de int porque char ocupa un solo byte de memoria, en cambio, un int normalmente 4.
	short opcion=0;

	while(loop){
		short linea=1;
		mvwprintw(win, linea++, (ncols-strlen(opciones[0]))/2, "%s", opciones[0]); 
		// Las operaciones son para centrar el menú.

		for (int i=1; i<len_menu; i++)
			if (opcion==i-1) {
				wattron(win, A_STANDOUT);
				mvwprintw(win, ++linea, (centrado)?(ncols-strlen(opciones[i]))/2:1, "%s", opciones[i]);
				wattroff(win, A_STANDOUT);
			}
			else 
				mvwprintw(win, ++linea, (centrado)?(ncols-strlen(opciones[i]))/2:1, "%s", opciones[i]);
		
		int key=wgetch(win);

		switch(key){
			case KEY_UP:
				opcion=(opcion)?opcion-1:len_menu-2;
				break;

			case KEY_DOWN:
				opcion=(opcion==len_menu-2)?0:opcion+1;
				break;
			
			case 'q':
			case 'Q':
			case '\33':
				loop=0;
				break;

		}
		/*
		La siguiente función no es necesaria, pero se podrían hacer pruebas para el rendimiento después.
		refresh();
		*/

	}
	endwin();

	return OK;
}