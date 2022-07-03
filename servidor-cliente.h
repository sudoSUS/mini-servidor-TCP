#ifndef SERVIDOR_CLIENTE_H
#define SERVIDOR_CLIENTE_H

#define ser "SERVER"
#define cli "CLIENT"

#include<netinet/in.h>

typedef enum {ERROR, OK} t_estado;

int gethostip(char*);
t_estado f_socket(int *, char*);
void initservaddr(struct sockaddr_in *, char*, unsigned int);
t_estado f_bind(int, struct sockaddr_in *, char*);
t_estado f_listen(int, struct sockaddr_in, int, char*);
int f_accept(int, struct sockaddr_in *);
t_estado menu_principal(float alto_porc, float ancho_porc, short centrado);

#endif