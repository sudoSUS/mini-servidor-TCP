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
short menu_principal(float alto_porc, float ancho_porc, short centrado);

#endif