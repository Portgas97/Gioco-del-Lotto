#include "/home/studenti/NetBeansProjects/funzioni.h"

/* FUNZIONI DI UTILITÀ */

// stampa le funzioni disponibili, se viene passato 0 si stampa l'intero menù
// altrimenti la sola riga associata all'intero passato
void stampa_menu(int i){    
    int j;
    char vettore_comandi[NUMERO_COMANDI][LEN_COMANDI] = { "help <comando> --> mostra i dettagli di un comando",
                                                            "signup <username> <password> --> crea un nuovo utente",
                                                            "login <username> <password> --> autentica un utente", 
                                                            "invia_giocata g --> invia giocata g al server", 
                                                            "vedi_giocata tipo --> visualizza le giocate precedenti dove tipo = {0,1} e permette di visualizzare le giocate passate \'0\' oppure le giocate attive \'1\' (ancora non estratte)", 
                                                            "vedi_estrazione <n> <ruota> --> mostra i numeri delle ultime n estrazioni sulla ruota specificata", 
                                                            "esci --> termina il client"};
    
    if(i != 0){
        printf("\n");
        printf("!");
        for(j = 0; j < LEN_COMANDI; j++)
            if(vettore_comandi[i][j] == '\0'){
                printf("\n");
                return;
            }
            else
                printf("%c", vettore_comandi[i][j]);
        
        printf("\n");
        return;
    }
    
    printf("\n");
    printf("****************************** GIOCO DEL LOTTO ****************************** \n\n");
    for(; i < NUMERO_COMANDI; i++){
        printf("%d) !", i);
        for(j = 0; j < LEN_COMANDI; j++)
            if(vettore_comandi[i][j] == '\0')
                break;
            else
                printf("%c", vettore_comandi[i][j]);
        printf("\n\n");
    }
    printf("***************************************************************************** \n");

    
    /*printf("\n");
    printf("****************************** GIOCO DEL LOTTO ****************************** \n\n");
    printf("1) !help <comando> --> mostra i dettagli di un comando\n\n");
    printf("2) !signup <username> <password> --> crea nu nuovo utente\n\n");
    printf("3) !login <username> <password> --> autentica un utente\n\n");
    printf("4) !invia_giocata g --> invia giocata g al server\n\n");
    printf("5) !vedi_giocata tipo --> visualizza le giocate precedenti dove tipo = {0,1}\n");
    printf("                          e permette di visualizzare le giocate passate '0'\n");
    printf("                          oppure le giocate attive '1' (ancora non estratte)\n\n");
    printf("6) !vedi_estrazione <n> <ruota> --> mostra i numeri delle ultime n estrazioni\n");
    printf("                                    sulla ruota specificata\n\n");
    printf("7) !esci --> termina il client\n\n");
    printf("***************************************************************************** \n");
     * */
}

// data una stringa ricava la seguente sottostringa partendo dall'indice index
// utilizza vari delimitatori (' ', '\n', '\0')
// il valore di index ritornato è quello dello spazio bianco o di ritorno carrello successivo
char sotto_stringa(char* comando, char* buffer, int *index){
    
    int i;
    
    i = 0;
    while(1){
        //printf("Eseguo ciclo numero %d\n", i);
        //printf("Valore di index %d\n", *index);

        if(comando[*index] == '\0'){
            buffer[i] = '\0';
            return '\0';
        }
            
        
        if(comando[*index] == ' '){
            buffer[i] = '\0';
            return ' ';
        }
        
        if(comando[*index] == '\n'){
            buffer[i] = '\0';
            return '\n';
        }
            
        
        //printf("carattere trasferito: %c\n", comando[*index]);
        buffer[i] = comando[*index];
/*
        if(*index == MAX_SIZE_INPUT){
            printf("Il comando inserito è troppo lungo\n");
            exit(1);
        }
*/      //printf("\n");
        (*index)++;
        i++;
    }
    buffer[i] = '\0';
    
}


// associa una certa stringa/comando ad un intero convenzionale
int rileva_comando(char *buffer){
    
    // nella lettura dei parametri potrebbe esserci ancora !
    if(buffer[strlen(buffer) - 1] == ' '){
        buffer[strlen(buffer) - 1] = '\0';
    }
    
    if(strncmp("help", buffer, 4) == 0)
        return 1;
    
    else if(strncmp("signup", buffer, 6) == 0)
        return 2;
    
    else if(strncmp("login", buffer, 5) == 0)
        return 3;
    
    else if(strncmp("invia_giocata", buffer, 13) == 0)
        return 4;
    
    else if(strncmp("vedi_giocata", buffer, 12) == 0)
        return 5;
    
    else if(strncmp("vedi_estrazione", buffer, 15) == 0)
        return 6;
    
    else if(strncmp("esci", buffer, 4) == 0)
        return 7;
    
    else 
        return 0;
}

// controlla se è presente una sottostringa / parametro
char rileva_parametro(char *comando, int *index, char terminatore){
    
    if(terminatore == ' ') // un parametro è presente solo se c'è ' '
        if(comando[*index + 1] != '\n' && comando[*index + 1] != ' ' && comando[*index + 1 != '\0']){ // non accettiamo due ' ' di seguito
            // aggiorno index in modo che si punti all'inizio del parametro
            printf("LOG_DEBUG: è presente un parametro \n");
            (*index)++;
            return 's'; // si, c'è un parametro
        } 
    printf("LOG_DEBUG: non è presente alcun parametro\n");
    return 'n'; //no, non c'è un parametro
    
}

// È possibile che nelle seguenti due funzioni si debbano passare ret1 e ret2 per vedere quanti byte sono effettivamente stati comunicati
void trasmetti(char *buffer, int socket){ 
    
    int len, ret;
    uint16_t msg_len;
    
    printf("\n");

    printf("\nLOG_DEBUG: TRASMETTO %s\n", buffer);
    
    len = strlen(buffer);
    msg_len = htons(len);
    
    // invio il numero di dati
    ret = send(socket, (void*) &msg_len, sizeof(uint16_t), 0);

    if(ret < 0){
        printf("Errore nella send() di invio quantità\n");
        perror("perror --> errno :");
        exit(1);
    }

    if(ret < sizeof(uint16_t)){
        printf("Errore nella send() di invio quantità, inviati %d dati anziché %d\n", ret, (int)sizeof(uint16_t));
        exit(1);
    }
    
    perror("errno nella send di quanti dati :");
    
    // invio i dati
    ret = send(socket, (void*)buffer, len, 0);
    
    if(ret < 0){
        printf("Errore nella send() di invio dati\n");
        perror("perror --> errno :");
        exit(1);
    }
    
    if(ret < len){
        printf("Errore nella send() di invio dati, inviati %d dati anziché %d\n", ret, len);
        exit(1);
    }
    perror("errno nella send del messaggio :");
    
    printf("\n");
}

void ricevi(char *buffer, int socket){ 
    
    int len, ret;
    uint16_t msg_len;
    
    printf("\n");
    
    // ricevo quanti dati
    ret = recv(socket, (void*) &msg_len, sizeof(uint16_t), 0);
    len = ntohs(msg_len);

    
    if(ret == 0){
        printf("Errore nella recv di quanti dati, l'host remoto si è chiuso\n");
        exit(1);
    }
    
    if(ret < 0){
        printf("Errore nella recv di quanti dati\n");
        perror("perror --> errno :");
        exit(1);
    }
    
    if(ret < sizeof(uint16_t)){
        printf("Errore nella recv() della quantità, ricevuti %d dati anziché %d\n", ret, (int)sizeof(uint16_t));
        exit(1);
    }
    
    perror("errno nella recv di quanti dati :");
    
    // ricevo i dati
    ret = recv(socket, (void*)buffer, len,  MSG_WAITALL);
    
    if(ret == 0){
        printf("Errore nella recv dei dati, l'host remoto si è chiuso\n");
        exit(1);
    }
    
    if(ret < 0){
        printf("Errore nella recv di dati\n");
        perror("perror --> errno :");
        exit(1);
    }
    
    if(ret < len){
        printf("Errore nella recv() dei dati, ricevuti %d dati anziché %d\n", ret, len);
        exit(1);
    }
    
    perror("errno nella recv del messaggio :");

    buffer[len] = '\0';
    
    printf("LOG_DEBUG: RICEVUTO %s\n", buffer);
    
    printf("\n");
}      


void percorso_file_registro(char *username, char *stringa){    
    strncpy(stringa, "/home/studenti/NetBeansProjects/lotto_server/file_registro/", PATH_LEN);
    strncat(stringa, username, strlen(username) + 1);
    strncat(stringa, ".txt", 5);
}


FILE* crea_file_registro(char *username, char *stringa){
    
    FILE *fd;

    if(username[strlen(username) - 1] == ' ' || username[strlen(username) - 1] == '\n')
        username[strlen(username) - 1] = '\0';
    
    percorso_file_registro(username, stringa);
    
    printf("%s\n", stringa);
    fd = fopen(stringa, "w");
    
    if(fd == NULL){
        perror("Errore nella creazione del file di registro dell'utente\n");
        exit(1);
    }
    
    printf("Creato il file di registro %s\n", stringa);
    
    fclose(fd);
    
    return fd;
}

// scrive buffer nel file situato in path, seguito da \n 
// richiede anche l'accesso in lettura per poter memorizzare il contenuto
FILE* scrivi_file(char *buffer, const char *path, char* modality){
    
    FILE *fd;
    int ret;
    
   
    ret = 0;
    
    fd = fopen(path, modality);

    if(fd == NULL){
        perror("Errore nell'apertura del file");
        printf("%s\n", path);
        exit(1);
    }

    
    strncat(buffer, "\n", 2);
    ret = fprintf(fd, "%s", buffer);
    if(ret <= 0){ 
        printf("Valore di ret: %d\n", ret); 
        perror("Errore scrittura su file :"); 
        exit(1);
    } 

    fclose(fd);
    return fd;

}
 
int trova_username(char *username){
    
    FILE *fd;
  
    fd = fopen("/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt", "r");

    if(fd == NULL){
        perror("Errore nell'apertura del file");
        printf("/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt\n");
        exit(1);
    }


    while(1){  
        
        int success;
        char *stringa;
        
        stringa = calloc(CREDENTIAL_LEN, sizeof(char));
        if(!stringa){
            printf("Memoria esaurita\n");
            exit(1);
        }
        success = fscanf(fd, "%s", stringa);
        
        if(success == EOF){
            free(stringa);
            break;
        }
        
        if(!success){
            free(stringa);
            break;
        }
        
        if(strcmp(stringa, username) == 0){
            fclose(fd);
            free(stringa);
            return 1;
        }

        free(stringa);
    }
    
    fclose(fd);
    return 0;
}

int verifica_password(char *username, char *password){
    
    FILE *fd;
    char *stringa;
    char *pass;
    
    stringa = calloc(BUFFER_LENGHT, sizeof(char));
    if(!stringa){
        printf("Memoria esaurita\n");
        exit(1);
    }
    if(username[strlen(username) - 1] == ' ' || username[strlen(username) - 1] == '\n')
        username[strlen(username) - 1] = '\0';
    
    
    percorso_file_registro(username, stringa);

    fd = fopen(stringa, "r");

    free(stringa);
    
    if(fd == NULL){
        perror("Errore nell'apertura del file di registro dell'utente\n");
        exit(1);
    }
    
    pass = calloc(CREDENTIAL_LEN, sizeof(char));
    fscanf(fd, "%s", pass); // la prima informazione è la password
    if(pass[strlen(pass) - 1] == '\n')
        pass[strlen(pass) - 1] = '\0';
    
    
    if(password[strlen(password) - 1] == ' ' || password[strlen(password) - 1] == '\n')
        password[strlen(password) - 1] = '\0';
    
    fclose(fd);
    if(strcmp(pass, password) == 0){
        free(pass);
        return 1;
    }
    
    free(pass);
    return 0;
    
}

void acquisisci_parametro(char bool, char *comando, char *buffer, int *index){
    // char terminatore;
    
    if(bool == 'n'){ // non è presente un parametro
        printf("Parametro non inserito\n");
        exit(1);
    }
    else if(bool == 's')
        /*terminatore = */ sotto_stringa(comando, buffer, index);
    
    // per motivi di debug
    // printf("Terminatore: %c", terminatore);
                
}


int check_IP(char *saved_IP){
    
    FILE *fd;
    char *stringa;
    int index;
    char address[INET_ADDRSTRLEN];
    int success;
    
    index = 0;
    success = 0;
   
    stringa = calloc(BUFFER_LENGHT, sizeof(char)); 
    if(!stringa){
        printf("Memoria esaurita\n");
        exit(1);
    }

    fd = fopen(IP_BANNATI, "r");
    if(fd == NULL){
        perror("Errore nell'apertura del file degli IP bannati\n");
        free(stringa);
        exit(1);
    }
   

    while(1){  
        
        success = fscanf(fd, "%s", stringa);
        
        if(success == EOF)
            break;
        
        if(!success)
            break;
        
        sotto_stringa(stringa, address, &index);
                
        if(strcmp(saved_IP, address) == 0){
            fclose(fd);
            free(stringa);
            return 1;
        }
    }
    
    
    fclose(fd);
    free(stringa);
    return 0;
}

void aggiorna_bannati(char *saved_IP){
    FILE *fd_src, *fd_dest;
    int index;
    char *stringa;
    char address[INET_ADDRSTRLEN];
    int res;
    
    fd_src = fopen(IP_BANNATI, "r");
    if(fd_src == NULL){
        perror("Errore nell'apertura del file degli IP bannati\n");
        exit(1);
    }
    
    fd_dest = fopen(FILE_BUFFER, "w");
    if(fd_dest == NULL){
        perror("Errore nell'apertura del file buffer\n");
        exit(1);
    }
    
    while(fscanf(fd_src, "%s", stringa) != EOF ){  //!feof(fd_src)
        stringa = calloc(BUFFER_LENGHT, sizeof(char));
        if(!stringa){
            printf("Memoria esaurita\n");
            exit(1);
        }
        
        index = 0;
        
        sotto_stringa(stringa, address, &index);
                
        if(strcmp(saved_IP, address) == 0){
            free(stringa);
            continue;
        }
        
        fprintf(fd_dest, "%s", stringa);
        free(stringa);
        
    }
    
    fclose(fd_src);
    fclose(fd_dest);
    
   
    res = remove(IP_BANNATI);
    if(res != 0)
      perror("Errore nella rimozione del file");
    
    // aggiorno bannati.txt
    
    fd_dest = fopen(IP_BANNATI, "w");
    if(fd_src == NULL){
        perror("Errore nell'apertura del file degli IP bannati\n");
        exit(1);
    }
    
    fd_src = fopen(FILE_BUFFER, "r");
    if(fd_dest == NULL){
        perror("Errore nell'apertura del file buffer\n");
        exit(1);
    }
    
    while(fscanf(fd_src, "%s", stringa) != EOF){  
        stringa = calloc(CREDENTIAL_LEN, sizeof(char));
        if(!stringa){
            printf("Memoria esaurita\n");
            exit(1);
        }
        
        index = 0;
        fprintf(fd_dest, "%s", stringa);
        free(stringa);
    }
    
    fclose(fd_src);
    fclose(fd_dest);
    
    res = remove(FILE_BUFFER);
    if(res != 0)
      perror("Errore nella rimozione del file");
    
    printf("File IP_bannati.txt aggiornato\n");
    
}

void stampa_schedina(struct schedina* punt_schedina){
    
    int i;
    printf("Contenuto della schedina:\n");
 
    printf("Numeri giocati: ");
    for(i = 0; i < punt_schedina->quanti_numeri; i++){
        printf("%d ", punt_schedina->numeri_giocati[i]);
    }
    printf("\n");
    
    printf("Ruote: ");
    if(punt_schedina->flag_tutte == 's'){
        printf("tutte");
    }
    else { 
        for(i = 0; i < punt_schedina->quante_ruote; i++){
            printf("%s ", punt_schedina->ruote[i]);
        }
    }
    printf("\n");
    
    printf("Importi giocati: \n");
    for(i = 0; i < punt_schedina->quanti_importi; i++){    
        switch(i){
            case 0:
                printf("Estratto semplice: €%d\n", punt_schedina->importi[i]);
                break;
            case 1:
                printf("Ambo: €%d\n", punt_schedina->importi[i]);
                break;  
            case 2:
                printf("Terno: €%d\n", punt_schedina->importi[i]);
                break;
            case 3:
                printf("Quaterna: €%d\n", punt_schedina->importi[i]);
                break;
            case 4:
                printf("Cinquina: €%d\n", punt_schedina->importi[i]);
                break;
            default:
                printf("Errore nello switch degli importi\n");
                break;
        }
    }
    printf("\n");
    
}