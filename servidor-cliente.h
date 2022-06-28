#define ser "SERVER"
#define cli "CLIENT"

//typedef enum {false, true} bool;
typedef enum {ERROR, OK} t_estado;

int gethostip(char*);
t_estado f_socket(int &, char*);
//bool allint(char*);