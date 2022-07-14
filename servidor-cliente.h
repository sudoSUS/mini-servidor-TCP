#ifndef SERVIDOR_CLIENTE_H
#define SERVIDOR_CLIENTE_H

#include<netinet/in.h>
#include<ncurses.h>

#define ser "SERVER"
#define cli "CLIENT"
#define MAXNOM 50

// Estructura describiendo un usuario y los colores a usar.
typedef struct{
	char nombre[MAXNOM+1];
	short c_pair[2]; // Para el para de colores.
	
} t_usuario;

int gethostip(char*);
short f_socket(int *, char*);
void initservaddr(struct sockaddr_in *, char*, unsigned int);
short f_bind(int, struct sockaddr_in *, char*);
short f_listen(int, struct sockaddr_in, int, char*);
int f_accept(int, struct sockaddr_in *);
short _gen_menu_principal(float alto_porc, float ancho_porc, short centrar_titulo, short centrar_opcion, short sep_titulo, short sep_opcion);
void _gen_print(WINDOW* win, short* linea, short ncols, char* string, short sep, short centrado);
void entrada_larga(WINDOW* win, short y, short x, int cols, char* buffer, unsigned int size, int (*verfunc)(int));
void personalizar_usuario(WINDOW *win, t_usuario* usuario, int ncols);

#define menu_principal(alto_porc, ancho_porc) (_gen_menu_principal(alto_porc, ancho_porc, 1, 1, 4, 2))
#define printc(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 1))
#define print(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 0))

#endif