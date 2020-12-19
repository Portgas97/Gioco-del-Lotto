#include <stdio.h> // ingresso-uscita, anche per il file
#include <string.h> // funzioni sulle stringhe
#include <unistd.h> // primitiva fork, sleep
#include <stdlib.h> // primitive e utilità
#include <sys/types.h> // per la socket e per la select
#include <sys/socket.h> // per la socket
#include <sys/time.h> // per la select
#include <unistd.h> // per la select
#include <netinet/in.h> // per gli indirizzi 
#include <arpa/inet.h> // funzioni di conversione 
#include <errno.h> // gestione degli errori
#include <sys/stat.h>
#include <time.h> // per time_t

//#include<stdlib.h>
//#include<stdio.h> // per l'ingresso-uscita
//#include<string.h> // funzioni sulle stringhe

// da cambiare, vedi invia_giocata !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define MAX_SIZE_INPUT 31
#define MAX_SIZE_PARAMETER 16
#define NUMERO_COMANDI 7
#define LEN_COMANDI 200
#define BUFFER_LENGHT 1024
#define RESPONSE_LEN 100
#define CREDENTIAL_LEN 8
#define PATH_LEN 100
#define TIME_LEN 80
#define SESSION_ID_LEN 10
#define WHEEL_LENGHT 10

static const char IP_BANNATI[] = "/home/studenti/NetBeansProjects/lotto_server/IP_bannati.txt";
static const char FILE_BUFFER[] = "/home/studenti/NetBeansProjects/lotto_server/buffer.txt";

// struttura per la schedina
    struct schedina {
        int numeri_giocati[10];
        int quanti_numeri;
        char ruote[11][WHEEL_LENGHT];
        int quante_ruote;
        char flag_tutte;
        int importi[5];
        int quanti_importi;
    };
    


int rileva_comando(char *buffer);
char sotto_stringa(char *comando, char *buffer, int *index);
void stampa_menu(int i);
char rileva_parametro(char *comando, int *index, char terminatore);
void acquisisci_parametro(char bool, char *comando, char *buffer, int *index);

void trasmetti(char *buffer, int socket);
void ricevi(char *buffer, int socket);

void percorso_file_registro(char *username, char *stringa);
FILE* crea_file_registro(char *username, char *stringa);
FILE* scrivi_file(char *buffer, const char *path, char* modality);
int trova_username(char *username);
int verifica_password(char *username, char *password);
int check_IP(char *saved_IP);
void aggiorna_bannati(char *saved_IP);

void stampa_schedina(struct schedina* punt_schedina);