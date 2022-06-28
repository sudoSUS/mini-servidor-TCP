#define ser "SERVER"
#define cli "CLIENT"

typedef enum {ERROR, OK} t_estado;

int gethostip(char*);
t_estado f_socket(int *, char*);
void initservaddr(struct sockaddr_in *, char*, unsigned int);
t_estado f_bind(int, struct sockaddr_in *, char*);
t_estado f_listen(int, struct sockaddr_in, int, char*);
int f_accept(int, struct sockaddr_in *);