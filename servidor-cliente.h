#ifndef SERVIDOR_CLIENTE_H
#define SERVIDOR_CLIENTE_H

#include<netinet/in.h>
#include<ncurses.h>

#define MAXNOM 50
#define MMENSAJE 500

// Estructura describiendo un usuario y los colores a usar.
typedef struct{
	char nombre[MAXNOM+1];
	short c_pair[2]; // Para el para de colores.
	
} t_usuario;

typedef struct usuarios {
	struct usuarios* sig;
	short pair; // Para el par de colores.
	int connectionfd;
	t_usuario user;
} t_usuarios;

typedef struct mensajes {
	struct mensajes* sig;
	char* mensaje;
	t_usuarios* usuario;
} t_mensajes;

int gethostip(char*);
short f_socket(int *);
void initservaddr(struct sockaddr_in *, char*, unsigned int);
short f_bind(int, struct sockaddr_in *);
short f_listen(int, struct sockaddr_in, int);
int f_accept(int, struct sockaddr_in *);
short _gen_menu_principal(float alto_porc, float ancho_porc, short centrar_titulo, short centrar_opcion, short sep_titulo, short sep_opcion);
void _gen_print(WINDOW* win, short* linea, short ncols, char* string, short sep, short centrado);
void entrada_larga(WINDOW* win, short y, short x, int cols, char* buffer, unsigned int size, int (*verfunc)(int));
void personalizar_usuario(WINDOW *win, t_usuario* usuario, int ncols);

#define menu_principal(alto_porc, ancho_porc) (_gen_menu_principal(alto_porc, ancho_porc, 1, 1, 4, 2))
#define printc(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 1))
#define print(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 0))

#endif