#include"servidor-cliente.h"

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>
#include<stdlib.h>

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
Eleva N1 a la potencia N2
*/
float pot(int n1, int n2){
	if (!n1) return 0;
	if (!n2) return 1;
	float res=1;
	if (n2>0)
		for (; n2; n2--) res*=n1;
	else for (; n2; n2++) res/=n1;
	return res;
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
	short len=strlen(string), capacidad=ncols-sep*2, uno=0;
	if (len==1) uno=1;

/* 
Si hubiera problemas de optimización trabajando con grandes cantidades de texto, puede dividirse el código para cada 
elección individualmente. Así se evita la evaluación de las sentencias if en cada iteración.
El uso de la variable "uno" es una solución temporal para imprimir un solo carácter. Buscar a futuro una solución más
eficiente.
*/
	switch (centrado){ 
		case 0:
		case 1:
			for (int fin=0, inicio=0; fin<len-1 || (uno--)==1;){
				if (len-fin < capacidad) fin = len-1;
				else {
					fin+=capacidad;
					while (isalpha(string[fin]) && fin>inicio) fin--;
					if (fin==inicio) fin+=capacidad-1;
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
Para realizar una entrada en la posición Y, X.
Cuando la cantidad de caracteres introducidos rebase las columnas permitidas, se "recorrerá" el texto usando un carácter '<',
como sucede en el editor Nano.
*/
void entrada_larga(WINDOW* win, short y, short x, int cols, char* buffer, unsigned int size, int (*verfunc) (int)){
	if (verfunc==NULL) verfunc=isprint;
	char temp[size--]; temp[0]='\0';
	bool loop=1;
	wmove(win, y, x);
	for (short i=0; i<cols; i++) wprintw(win, " ");
	int caracteres=0, key, i, c, j; // Las tres últimas variables son para los bucles.
	wmove(win, y, x);
	curs_set(1);

	while (loop) {
		key=wgetch(win);

		if (verfunc(key)){
			if (caracteres<size){
				if ((!(caracteres>cols? (caracteres-1)%(cols-1) : caracteres%cols)) && caracteres) {
					wmove(win, y, x+1);
					for (short i=1; i<cols; i++) wprintw(win, " ");
					wmove(win, y, x);
					wattron(win, A_STANDOUT);
					wprintw(win, "<");
					wattroff(win, A_STANDOUT);
				}
				temp[caracteres++]=key;
				temp[caracteres]='\0';
				wprintw(win,"%c",key);
			}
		}
		else switch(key){
			case KEY_BACKSPACE:
				if (caracteres) {
					temp[--caracteres]='\0';
					// Agregar más conficiones para cuando caracteres>cols, aquí y en agregar caracteres.
					if ((!(caracteres>cols? (caracteres-1)%(cols-1) : caracteres%cols)) && caracteres) {
						if(caracteres>cols){
							wmove(win, y, x);
							wattron(win, A_STANDOUT);
							wprintw(win, "<");
							wattroff(win, A_STANDOUT);
							wprintw(win,"%s",(char*)(temp+caracteres-cols+1));
						}
						else
							mvwprintw(win, y, x, "%s", temp);
					}
					else{
						mvwprintw(win, y, x+(caracteres>cols? (caracteres-cols)%(cols-1)+1: caracteres), " ");
						wmove(win, y, x+(caracteres>cols? (caracteres-cols)%(cols-1)+1: caracteres));
					}
				}
				break;
			case '\33':
			case '\n':
				for (i=0; isspace(temp[i]); i++);
				for (c=strlen(temp)-1; isspace(temp[c]); c--);
				for (j=0; i<=c; j++, i++) temp[j]=temp[i];
				temp[j]='\0';
				caracteres=strlen(temp);
				if (caracteres){
					strcpy(buffer, temp);
				} else strcpy(temp, buffer);
				loop=0;
				break;
		}
	}

	curs_set(0);
}

void personalizar_usuario(WINDOW *win, t_usuario* usuario, int ncols){
	short c_pair[2], opcion=0, linea, nopciones=4, posOpciones[nopciones];
	bool colors_has=has_colors(), cambios;
	char nombre[MAXNOM+1], loop=1; strcpy(nombre, usuario->nombre);
	c_pair[0]=usuario->c_pair[0], c_pair[1]=usuario->c_pair[1];

	while (loop){
		werase(win); 
		box(win, 0, 0);
		linea=1;
		cambios=strcmp(usuario->nombre,nombre) || usuario->c_pair[0]!=c_pair[0] || usuario->c_pair[1]!=c_pair[1];
		if (colors_has){
			posOpciones[0]=linea;
			init_pair(2, c_pair[0], c_pair[1]);
			wattron(win, COLOR_PAIR(2));
			_gen_print(win, &linea, ncols, nombre, 4, 1);
			wattroff(win, COLOR_PAIR(2));

			for (short i=0; i<=COLOR_WHITE; i++) init_pair(i+3, 0, i);

			linea+=2;
			for (short i=0, pair; i<2; i++){
				posOpciones[1+i]=linea;
				wattron(win, A_BOLD | A_UNDERLINE | WA_LEFT);
				_gen_print(win, &linea, ncols, (i)? "Background:":"Foreground:", 4, 1);
				wattroff(win, A_BOLD | A_UNDERLINE | WA_LEFT);


				pair= (c_pair[i])? c_pair[i]+2 : 10;
				wattron(win, COLOR_PAIR(pair));
				mvwprintw(win, linea, ncols/2-2, " ");
				wattroff(win, COLOR_PAIR(pair)); 

				pair= c_pair[i]+3;
				wattron(win, COLOR_PAIR(pair));
				wprintw(win,"   ");
				wattroff(win, COLOR_PAIR(pair));

				pair= (c_pair[i]+1)%COLOR_WHITE +3;
				wattron(win, COLOR_PAIR(pair));
				wprintw(win," ");
				wattroff(win, COLOR_PAIR(pair));

				linea+=2;
			}

			posOpciones[3]=linea;
			wattron(win, A_UNDERLINE| ((cambios)?A_BLINK:0) | A_BOLD);
			_gen_print(win,&linea,ncols,"Guardar",4,1);
			wattroff(win, A_UNDERLINE| ((cambios)?A_BLINK:0) | A_BOLD);
			linea++;

			wattron(win, A_STANDOUT | A_BLINK);
			mvwprintw(win, posOpciones[opcion], ncols-3, "*");
			wattroff(win, A_STANDOUT | A_BLINK);

			switch(wgetch(win)) {
				case KEY_UP:
					opcion=(opcion)?opcion-1: nopciones-1;
					break;
				case KEY_DOWN:
					opcion=(opcion+1)%nopciones;
					break;
				case 'q':
				case 'Q':
				case '\33':
					if (cambios){
						wattron(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
						_gen_print(win,&linea,ncols,"¿Desea salir sin guardar los cambios? (S/N)",2,1);
						wattroff(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
						short key;
						do{
							key=tolower(wgetch(win));
						} while (key!='s' && key!='n');

						if (key=='s')
							loop=0;
					}
					else loop=0;
					break;
				
				case KEY_LEFT:
					switch(opcion) {
						case 0:
							entrada_larga(win, 1, 2, ncols-4, nombre, MAXNOM+1, NULL);
							break;
						case 1:
						case 2:
							short* ptcolor=c_pair+opcion-1;
							*ptcolor= (*ptcolor)?(*ptcolor)-1:COLOR_WHITE;
							break;
					}
					break;
				case KEY_RIGHT:
					switch(opcion) {
						case 0:
							entrada_larga(win, 1, 2, ncols-4, nombre, MAXNOM+1, NULL);
							break;
						case 1:
						case 2:
							short* ptcolor=c_pair+opcion-1;
							*ptcolor= ((*ptcolor)+1)%8;
							break;
					}
					break;
				case '\n':
				case ' ':
					switch(opcion){
						case 3:
							if (cambios) {
								wattron(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
								_gen_print(win,&linea,ncols,"¿Desea guardar los cambios? (S/N)",2,1);
								wattroff(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
								short key;

								do{
									key=tolower(wgetch(win));
								} while (key!='s' && key!='n');
								
								if (key=='s') {
									strcpy(usuario->nombre, nombre);
									usuario->c_pair[0]=c_pair[0];
									usuario->c_pair[1]=c_pair[1];
									FILE* fuser;
									if ((fuser=fopen(".user.data","wb"))!=NULL){
										int size_write;
										size_write=fwrite(usuario, sizeof(*usuario), 1, fuser);
										if (size_write!=sizeof(*usuario)){
											werase(win);
											wprintw(win, "Error al guardar en el archivo.");
										}
										loop=0;
									}
								}
							}
							else {
								wattron(win, A_UNDERLINE);
								_gen_print(win,&linea,ncols,"No se han realizado cambios.",2,1);
								_gen_print(win,&linea,ncols,"Presiona una tecla para continuar...",2,1);
								wattroff(win, A_UNDERLINE);
								wgetch(win);
							}
							break;
						case 0:
							entrada_larga(win, 1, 2, ncols-4, nombre, MAXNOM+1, NULL);
							break;
					}
					break;
				default:
					if (!opcion) entrada_larga(win, 1, 2, ncols-4, nombre, MAXNOM+1, NULL);
			}
		
		}
	}
	werase(win); ////// No parece funcionar.
}

// Retorna 1 si el argumento es un número o un punto; 0 de lo contrario.
int puntumero(int caracter){
	if (isdigit(caracter)||caracter=='.') return 1;
	return 0;
}

void servidor(WINDOW* win, t_usuario usuario, int ncols) {
	int sockfd; // File descriptor para el socket del servidor.
	short loop=1, opcion=0, linea, key, nopciones=2, posOpciones[nopciones];
	char ip[16], puerto[6]="8000";
	gethostip(ip);

	struct {
		char* nombre_opcion;
		char* valor_opcion;
	} opciones[] = {
		{"IP", ip,},
		{"Puerto", puerto,}
	};
	short len_opciones=sizeof(opciones)/sizeof(opciones[0]);

	curs_set(1);

	while (loop){
		werase(win);
		linea=1;
		box(win, 0, 0);

		init_pair(2, usuario.c_pair[0], usuario.c_pair[1]);
		wattron(win, COLOR_PAIR(2));
		_gen_print(win, &linea, ncols, usuario.nombre, 4, 1);
		wattroff(win, COLOR_PAIR(2));
		linea++;

		wattron(win, A_BOLD);
		_gen_print(win, &linea, ncols, "Configuración del servidor", 2, 1);
		wattroff(win, A_BOLD);
		linea++;

		for (short i=0; i<len_opciones; i++){
			posOpciones[i]=linea;
			wattron(win, A_UNDERLINE);
			_gen_print(win, &linea, ncols, opciones[i].nombre_opcion, 4, 1);
			wattroff(win, A_UNDERLINE);

			wattron(win, A_ITALIC);
			_gen_print(win, &linea, ncols, opciones[i].valor_opcion, 4, 1);
			wattroff(win, A_ITALIC);

			linea++;
		}

		linea++;
		wattron(win, A_BLINK | A_DIM);
		_gen_print(win, &linea, ncols, "Enter para continuar...", 2, 1);
		wattroff(win, A_BLINK | A_DIM);

		mvwprintw(win, posOpciones[opcion], ncols-3, "*");
		wmove(win, posOpciones[opcion], ncols-3);

		switch((key=wgetch(win))){
			case '\33':
			case 'q':
			case 'Q':
				loop=0;
				break;
			
			case KEY_UP:
				opcion=(opcion)?opcion-1: nopciones-1;
				break;
			case KEY_DOWN:
				opcion=(opcion+1)%nopciones;
				break;
			
			case '\n':
				linea--;
				curs_set(0);
				wmove(win, linea, 1); for (int i=2; i<ncols; i++) wprintw(win, " ");
				wattron(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
				_gen_print(win,&linea,ncols,"¿Iniciar servidor? (S/N)",2,1);
				wattroff(win, A_STANDOUT | A_UNDERLINE | A_BOLD);
				if (tolower(wgetch(win))=='s')
					; ///////// AQUÍ FUNCIÓN PARA EL CHAT (usar strtol para el puerto). Podría hacerse loop=0 también.
				else curs_set(1);
				break;
			
			default:
				switch (opcion)
				{
				case 0:
					entrada_larga(win, posOpciones[0]+1, ncols/2-8, 16, opciones[0].valor_opcion, 16, puntumero);
					break;
				case 1:
					entrada_larga(win, posOpciones[1]+1, ncols/2-3, 6, opciones[1].valor_opcion, 6, isdigit);
					break;
				}
				curs_set(1);
		}

	}

	curs_set(0);
}

void cliente(WINDOW* win, t_usuario usuario, int ncols){

}

/*
Los dos primeros argumentos definen el porcentaje de alto y ancho que el menú ocupará.

El argumento CENTRADO_<PARTE> define si las opciones estarán centradas o no:
	0: no centrado (por palabras);
	1: centrado (por palabras);
	2: no centrado;
	3: centrado.

El argumento SEP_<PARTE> define la separación de la parte con el borde.
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
		"Personalizar nombre de usuario",
	};
	int len_menu=sizeof(opciones)/sizeof(opciones[0]); // Revisar si hay un error porque tomó el puntero como un array.

	char loop=1; // Usé char en lugar de int porque char ocupa un solo byte de memoria, en cambio, un int normalmente 4.
	short opcion=0;

	while(loop){
		box(win,0,0);
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

			case '\n':
			case ' ':
				switch (opcion){
					case 0:
						servidor(win, usuario, ncols);
						break;
					case 1:
						cliente(win, usuario, ncols);
						break;
					case 2:
						personalizar_usuario(win, &usuario, ncols);
						init_pair(1, usuario.c_pair[0], usuario.c_pair[1]);
						break;


				}
				werase(win);
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