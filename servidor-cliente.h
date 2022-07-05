#ifndef SERVIDOR_CLIENTE_H
#define SERVIDOR_CLIENTE_H

#define ser "SERVER"
#define cli "CLIENT"


#include<netinet/in.h>

int gethostip(char*);
short f_socket(int *, char*);
void initservaddr(struct sockaddr_in *, char*, unsigned int);
short f_bind(int, struct sockaddr_in *, char*);
short f_listen(int, struct sockaddr_in, int, char*);
int f_accept(int, struct sockaddr_in *);
short _gen_menu_principal(float alto_porc, float ancho_porc, short centrar_titulo, short centrar_opcion, short sep_titulo, short sep_opcion);

#define menu_principal(alto_porc, ancho_porc) (_gen_menu_principal(alto_porc, ancho_porc, 1, 1, 4, 2))
#define printc(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 1))
#define print(win, linea, ncols, string, sep) (_gen_print(win, linea, ncols, string, sep, 0))

#endif