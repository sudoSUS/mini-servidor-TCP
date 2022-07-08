#include"servidor-cliente.h"

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>

#include<ncurses.h>

#include<ifaddrs.h>
#include<netdb.h>

#include<arpa/inet.h>


#define MAXNOM 20


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
short f_socket(int *sockfd, char* est){ 
	if((*sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
		fprintf(stderr, "[%s-error]: creación del socket fallida. %d: %s\n",est,errno, strerror(errno));
		return -1;
	}
	printf("[%s]: socket creado satisfactoriamente.\n",est);
	return OK;
}

void initservaddr(struct sockaddr_in *servaddr, char* IPv4, unsigned int puerto){
	memset(servaddr, 0, sizeof(*servaddr));

	servaddr->sin_family = AF_INET; // Familia IPv4.
	servaddr->sin_addr.s_addr = inet_addr(IPv4); // Función de <arpa/inet.h>.
	servaddr->sin_port = htons(puerto); // Función de <arpa/inet.h>. Cambia el orden de bytes al formato de la red.
}

short f_bind(int sockfd, struct sockaddr_in *servaddr, char* est){
	if((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
		fprintf(stderr, "[%s-error]: llamada bind fallida. %d: %s\n", est, errno, strerror(errno));
		return -1;
	}
	printf("[%s]: llamada bind realizada satisfactoriamente.\n",est);
	return OK;
}

short f_listen(int sockfd, struct sockaddr_in servaddr, int maxClients, char* est){
	if ((listen(sockfd, maxClients)) != 0) {
		fprintf(stderr,"[%s-error]: estado de escucha fallido. %d: %s\n", est, errno, strerror(errno));
		return -1;
	}
	printf("[%s]: escuchando en el SERV_PORT %d\n\n", est, ntohs(servaddr.sin_port));
	return OK;
}


/*
Se mantiene a la espera de una conección de algún cliente.
Se retorna un file descriptor de un socket que permite la conección al cliente.
Se guardan los datos del cliente en la estructura del parámetro CLIENT.
*/
int f_accept(int sockfd, struct sockaddr_in *client){
	// Antes era: int len = sizeof(*client),
	unsigned int len, \
	conectionfd = accept(sockfd, (struct sockaddr*) client, &len);

	return conectionfd;

}

// Estructura describiendo un usuario y los colores a usar.
typedef struct{
	char nombre[MAXNOM+1];
	short c_pair[2]; // Para el para de colores.
	
} t_usuario;

/*
Imprimir adaptándose al número de columnas NCOLS.
La variable SEP indica qué tan separado estará de la orilla (se recomienda que sea al menos 1).
Variable centrado: 0: no centrar (por palabras); 1: centrar (por palabras); 2: no centrar; 3: centrar (solo la última línea).
*/
void _gen_print(WINDOW* win, short* linea, short ncols, char* string, short sep, short centrado){
	if (sep<0) return; // Si la entrada es inválida, no imprime nada.

		/*
		Error en el cálculo del centro.
			Antes: ((ncols+sep-len%(ncols+1))/2)
		*/
	//short c=(ncols-strlen(string)%(ncols+1))/2+sep
	short len=strlen(string), capacidad=ncols-sep*2;

/* 
Si hubiera problemas de optimización trabajando con grandes cantidades de texto, puede dividirse el código para cada 
elección individualmente. Así se evita la evaluación de las sentencias if en cada iteración.
*/
	switch (centrado){ 
		case 0:
		case 1:
			for (int fin=0, inicio=0; fin<len-1;){
				if (len-fin < capacidad) fin = len-1;
				else {
					fin+=capacidad;
					while (isalpha(string[fin]) && fin>inicio) fin--;
					if (fin==inicio) fin+=capacidad;
				}

				while (isspace(string[fin]) && fin>inicio) fin--;
				while (isspace(string[inicio]) && fin>inicio) inicio++;

				wmove(win, (*linea)++, (centrado)? (ncols-fin+inicio)/2 : sep);
				for (; inicio<=fin; inicio++) wprintw(win, "%c", string[inicio]);
			}
			break;
		case 2:
		case 3:
			for (int i=0; i<len; i++){
				if ((len%(capacidad)==len-i)&&centrado==3) wmove(win, *linea, (ncols-len%(ncols-2*sep))/2);
				else if (! ( i%(capacidad))) wmove(win, (*linea)++, sep);

				wprintw(win, "%c", string[i]);
			}
			(*linea)++;
	}
	
}


/*
Los argumentos que se reciben son el porcentaje de la pantalla que usará la ventana del menú.
El argumento CENTRADO define si las opciones estarán centradas o hacia la izquierda:
	0: izquierda.
	1: centro.
*/
short _gen_menu_principal(float alto_porc, float ancho_porc, short centrar_titulo, short centrar_opcion, short sep_titulo, short sep_opcion){
	/*
	Notas:
	Par de colores 1 para el nombre de usuario.
	*/

	if (alto_porc>100 || ancho_porc>100 || alto_porc<0 || ancho_porc<0) return -1; 
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
	nlines--, ncols--;
	WINDOW* win = newwin(nlines, ncols, begin_y, begin_x);
	keypad(win,TRUE);
	box(win,0,0);

	// Comprobar si hay un usuario guardado.
	FILE* fuser=fopen(".user.data", "rb");
	t_usuario usuario;
	if (fuser!=NULL){
		fread(&usuario, sizeof(usuario), 1, fuser);
		fclose(fuser);
	}
	else {
		strncpy(usuario.nombre, "Anónimo", MAXNOM);
		usuario.c_pair[0]=COLOR_WHITE;
		usuario.c_pair[1]=COLOR_BLACK;
	}
	
	char color_has; // Almacenará si hay soporte para colores o no.
	if ((color_has=has_colors())){
		start_color();
		init_pair(1, usuario.c_pair[0], usuario.c_pair[1]);
	}

	char* opciones[]={ // El primer elemento será el título del menú.
		//"Bienvenido: ",
		"Ejecutar servidor",
		"Conectarse a servidor",
		"Elemento muy largo de prueba. Largo, pero pero pero muy largo."
	};
	int len_menu=sizeof(opciones)/sizeof(opciones[0]); // Revisar si hay un error porque tomó el puntero como un array.

	char loop=1; // Usé char en lugar de int porque char ocupa un solo byte de memoria, en cambio, un int normalmente 4.
	short opcion=0;

	while(loop){
		short linea=1;
		// Las operaciones son para centrar el título.
		//mvwprintw(win, linea++, 1, "%s", opciones[0]);
		if (color_has) {
			wattron(win,COLOR_PAIR(1));
			_gen_print(win, &linea, ncols, usuario.nombre, sep_titulo, centrar_titulo);
			wattroff(win,COLOR_PAIR(1));
		}
		else _gen_print(win, &linea, ncols, usuario.nombre, sep_titulo, centrar_titulo);

		wmove(win, linea, 1);
		linea+=2;
		for (short i=0; i<ncols-2; i++)wprintw(win, "_"); // El separador.

		for (int i=0; i<len_menu; i++){
			if (opcion==i) {
				wattron(win, A_STANDOUT);
				_gen_print(win, &linea, ncols, opciones[i], sep_opcion, centrar_opcion);
				wattroff(win, A_STANDOUT);
			}
			else 
				_gen_print(win, &linea, ncols, opciones[i], sep_opcion, centrar_opcion);


			linea+=2;
}
		int key=wgetch(win);

		switch(key){
			case KEY_UP:
				opcion=(opcion)?opcion-1:len_menu-1;
				break;

			case KEY_DOWN:
				opcion=(opcion==len_menu-1)?0:opcion+1;
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