#include "/home/studenti/NetBeansProjects/funzioni.h"

// riprendere da fase di login, riguardare controllo sul ban e scorrere del tempo


// argc è il numero degli argomenti passati + 1
// argv è un array di stringhe
int main(int argc, char* argv[]){
    
    int periodo; // minuti che intercorrono fra le estrazioni
    
    // per l'approccio multiprocesso
    pid_t pid;
    
    // varie
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int socket_listener;
    int error_saved; 
    socklen_t address_len;
    int i, ret, tmp;
    char *comando; 
    
    // file
    FILE * fd;
    int size;
    struct stat info;
    char *stringa;
    char *buffer; 
    
    
    printf("LOG_DEBUG: ci sono %d argomenti:\n", argc);
    printf("LOG_DEBUG:\n");
    for(i = 0; i < argc; ++i){
        printf("%s\n", argv[i]);
    }
    
    // # # # # # # # # # # VALIDAZIONE DEI PARAMETRI IN INGRESSO # # # # # # # # # # 
    
    if(argc == 2){ // solo porta 
        
        if(strcmp(argv[1], "4242") == 0){ 
            printf("Porta inserita correttamente\n");
            periodo = 5; // default
        } 
        else{
            printf("La porta: %s inserita non è valida\n", argv[1]);
            exit(1);
        }      
        
    } 
    else if(argc == 3){ // porta e periodo    
        
        if(strcmp(argv[1],"4242") == 0){
            
            if(strcmp(argv[2], "0") > 0 && strcmp(argv[2], "60") < 0){  // 33ù è passato al test, meglio convertire con ATOIIIIIIIIIIIIIIIIIIII
                
                printf("LOG_DEBUG: porta %s e periodo %s inseriti correttamente\n", argv[1], argv[2]);
                printf("porta e periodo corretti\n");
                periodo = *(int*)argv[2];
            } 
            else{
                printf("Valore del periodo %s non permesso\n", argv[2]);
                exit(1);
            }
        }
        else {
            printf("Porta %s non consentita\n", argv[1]);
            exit(1);
        }
    }
    else{
        printf("I parametri inseriti non sono permessi. Inserire numero di porta e periodo\n");
        exit(1);
    }
         

    
    printf("LOG_DEBUG: creazione della socket in lotto_server.c\n");
    
    // protocollo IPv4, socket TCP di tipo non bloccante
    // resituisce un descrittore di file
    socket_listener = socket(AF_INET, SOCK_STREAM, 0); // DA AGGIUNGERE non bloccante
    
    if(socket_listener < 0){
        printf("La primitiva socket() ha fallito, termino con un errore\n");
        perror("Error in socket(): \n");
        exit(1);
    }
    
    
    /* Ripasso: 
     * 
     *   struct sockaddr_in {
     *      sa_family_t     sin_family;
     *      in_port_t       sin_port;
     *      struct in_addr  sin_addr;
     *  };
     * 
     *   struct in_addr {
     *      uint32_t    s_addr;
     *  };
     * 
     */
    
    printf("LOG_DEBUG: inizializzazione dei campi per server_address\n");
    
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // come richiesto dalle specifiche ascolto su tutte le interfacce
    server_address.sin_port = htons(atoi(argv[1])); // espresso in network order
    
    printf("LOG_DEBUG: esecuzione della primitiva bind()\n");
    
    // Assegno l'indirizzo appena costruito al socket
    ret = bind(socket_listener, (struct sockaddr*)&server_address, sizeof(server_address));
    
    if(ret < 0){
        perror("Error in bind(): \n");
        exit(1);
    }
    
    printf("LOG_DEBUG: chiamata a listen() \n");
    
    // specifico che è un socket passivo, usato per ricevere richieste
    // riguarda cosa è 10/50 e setta come costante 
    ret = listen(socket_listener, 50);
    
    if(ret < 0){
        perror("Error in listen(): \n");
        exit(1);
    }
    
    // creo il file
    fd = fopen(IP_BANNATI, "w");
    if(fd == NULL){
        perror("Errore nella creazione del file IP_bannati.txt :\n");
        exit(1);
    }

    printf("LOG_DEBUG: creato il file IP_bannati.txt\n");
    fclose(fd);
    
    
    // Sono sempre in ascolto, se arriva una richiesta di connessione creo un 
    // processo figlio per gestirla
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CICLO INFINITO processo PADRE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    while(1){
        
        int new_socket;
        
        fflush(stdin); // non so se è necessario 
        
        address_len = (socklen_t)sizeof(client_address);
        
        printf("Server esegue accept()\n");
        
        new_socket = accept(socket_listener, (struct sockaddr*)&client_address, &address_len);
        
        // per primitiva non bloccante 
        /*
        // salvo il valore perchè altre chiamate (come ad esempio le printf di debug possono cambiarne il valore)
        error_saved = errno;
        
        //printf("Valore di new_socket: %d\n", new_socket);
     
        if(error_saved == EWOULDBLOCK){
            errno = 0;
            //printf("LOG_DEBUG: errno settato a EWOULDBLOCK da accept(), non sono presenti dati disponibili\n");
            sleep(5);
            
            // polling attivo, non sto sfruttando il socket non bloccante
            continue;
        }
        */
        
        if(new_socket < 0){
            perror("Error in accept : \n");
            exit(1);
            // forse qui devo mettere un continue ??
        } 
        
        printf("LOG_DEBUG: Creato socket %d per la connessione con il client\n", new_socket);
        
        pid = fork();
        
        if(pid == 0){ // caso figlio
            
            // utenti
            // char username[CREDENTIAL_LEN];
            // char password[CREDENTIAL_LEN]; 
            char *username;
            char *password;
            int trovato; // per verificare che non esista un utente con lo stesso username
            char sessionID[SESSION_ID_LEN];
            char receivedID[SESSION_ID_LEN];
            int contatore; // tentativi di login

            // per la memorizzazione dell'indirizzo 
            struct sockaddr_in *tmp_sockaddr_in;
            struct in_addr IPaddress;
            char savedIP[INET_ADDRSTRLEN];
            int errore;
            
            int termina_server;

            termina_server = 0;
            sessionID[0] = '\0';
            errore = 0;
            
            printf("~~~~~~~~~~~~~~~~ Sono il processo FIGLIO creato dalla accept() nel server! ~~~~~~~~~~~~~~~~ \n");
            
            // memorizzo l'indirizzo del client
            tmp_sockaddr_in = (struct sockaddr_in*)&client_address;
            IPaddress = tmp_sockaddr_in -> sin_addr;
            inet_ntop(AF_INET, &IPaddress, savedIP, INET_ADDRSTRLEN);
            
            printf("LOG_DEBUG: indirizzo IP del client: %s\n", savedIP);
            
            // il processo figlio non ascolta per nuova connessioni
            ret = close(socket_listener);
            if(ret < 0){
                perror("Error in close : \n");
                exit(1);
            }
            
            // devo rendere il socket non bloccante??
            
            // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CICLO INFINITO processo FIGLIO %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            while(1){ 
                
                printf("Processo figlio entra nel while infinito\n");
                

                // si può aggiungere il tempo mancanteeeeeeeeee
                errore = check_IP(savedIP);
                if(errore){
                    printf("IP bannato tenta di riconnettersi\n");
                    trasmetti("Sei stato bannato, riprova più tardi", new_socket);
                    close(new_socket);
                    exit(0);
                } else
                    trasmetti("Connessione effettuata", new_socket);
                
                comando = calloc(LEN_COMANDI, sizeof(char));
                ricevi(comando, new_socket);
                tmp = rileva_comando(comando);
                free(comando);
                
                // SELEZIONE DEL COMANDO 
                switch(tmp){

                    case 0: // #################################################### errore

                        printf("Nessun comando rilevato, termino\n");
                        termina_server = 1;
                        break;

                    case 1: // #################################################### help

                        printf("Non è possibile ricevere un comando di help\n");
                        termina_server = 1;
                        break;

                    case 2: // #################################################### signup

                        printf(" > > > > > > > >  Ricevuto un comando di signup < < < < < < < < \n");

                        // ricevo username e passowrd e li salvo nelle relative variabili
                        username = calloc(CREDENTIAL_LEN, sizeof(char));
                        password = calloc(CREDENTIAL_LEN, sizeof(char));
                        ricevi(username, new_socket);
                        ricevi(password, new_socket);
                        printf("Username: %s\n", username);
                        printf("Password: %s\n", password);

                        // se è il primo creo i file
                        fd = fopen("/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt", "a");
                        if(fd == NULL){
                            perror("Errore nella apertura del file lista_utenti.txt :\n");
                            exit(1);
                        }

                        printf("LOG_DEBUG: creato il file lista_utenti.txt\n");
                        fclose(fd);
                        
                        ret = stat("/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt", &info);
                        size = info.st_size;
                        printf("LOG_DEBUG: dimensione del file lista_utenti.txt: %d\n", size);
                        
                        ret = 0; // per controllare i valori di ritorno
                        
                        if(size == 0)
                            printf("Primo utente collegato\n");
                        else
                            printf("Nuovo utente collegato\n");
                        
                        
                        if(strlen(password) < 4){
                            printf("LOG_DEBUG: vincolo sull password non rispettato\n");
                            trasmetti("password non adatta", new_socket);
                            break; // mi metto in ascolto di un nuovo comando
                        }
                        
                        
                        if(size != 0){ 
                            fd = fopen("/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt", "r");
                            if(fd == NULL){
                                perror("Errore nell'apertura del file lista_utenti.txt\n");
                                exit(1);
                            }
                            
                            // controllo se esiste già un utente con questo username
                            trovato = 0;
                            while(!feof(fd)){
                                stringa = calloc(BUFFER_LENGHT, sizeof(char));
                                ret = fscanf(fd, "%s", stringa);
                                if(strcmp(stringa, username) == 0){
                                    trovato = 1;
                                    free(stringa);
                                    break;
                                }
                                free(stringa);
                            }

                            fclose(fd);
                
                            if(trovato){
                                printf("LOG_DEBUG: username esistente\n");
                               trasmetti("username esistente", new_socket);
                               free(username);
                               free(password);
                               break; // esco dallo switch, poi tornerò nel ciclo in attesa di un nuovo username 
                            }
                        }
                        // rewind(fd);

                        fd = scrivi_file(username, "/home/studenti/NetBeansProjects/lotto_server/lista_utenti.txt", "a");
                        
                        stringa = calloc(BUFFER_LENGHT, sizeof(char));
                        fd = crea_file_registro(username, stringa); 
                        
                        // come prima informazione mantengo la password
                        // utilizza il valore stringa modificato da crea_file_registro
                        fd = scrivi_file(password, stringa, "a"); 
                        printf("LOG_DEBUG: percorso file registro: %s\n", stringa);
                        free(stringa);
                        trasmetti("signup effettuato", new_socket);                            
                        
                        printf("signup effettuato correttamente\n");
                        free(username);
                        free(password);
                        break;

                    case 3: // #################################################### login

                        printf(" > > > > > > > >  Ricevuto un comando di login < < < < < < < < \n");
                        
                        if(sessionID[0] != '\0'){
                            trasmetti("login effettuato", new_socket);
                            break;
                        }
                        else 
                            trasmetti("fase di login", new_socket);
                        
                        contatore = 0; 
                        while(contatore < 3){
                            
                            // ricevo username e passowrd e li salvo nelle relative variabili
                            username = calloc(CREDENTIAL_LEN, sizeof(char));
                            password = calloc(CREDENTIAL_LEN, sizeof(char));
                            ricevi(username, new_socket);
                            ricevi(password, new_socket);
                            printf("Username: %s\n", username);
                            printf("Password: %s\n", password);

                            trovato = trova_username(username);
                            if(trovato){
                                // utente presente, convalido la password
                                ret = verifica_password(username, password);
                                if(ret)
                                    break;
                            }    
                            
                            if(contatore != 2) // escludo l'ultimo tentativo
                                trasmetti("username inesistente, reinserire credenziali", new_socket);
                            
                            // SEMMAI DA AGGIUNGERE UNA FUNZIONE TRASMETTI_INT CON L'INVIO DEI TENTATIVI RIMANENTI
                            contatore++;
                            
                        }
                        
                        
                        if(contatore == 3){ // login fallito
                            // per lavorare con il tempo
                            time_t ban_time;
                            time_t current_time;
                            struct tm *ts;
                            char buf[TIME_LEN];
                            
                            time(&ban_time);
                            printf("LOG_DEBUG: ban_time: %s\n", ctime(&ban_time));
                            
                            ts = localtime(&ban_time);
                            strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
                            printf("LOG_DEBUG: risultato strftime: %s\n", buf);
                            
                            printf("Scrivo IP e timestamp nel file IP_bannati.txt\n");
                            
                            buffer = calloc(BUFFER_LENGHT, sizeof(char));
                            strncpy(buffer, savedIP, INET_ADDRSTRLEN);
                            strncat(buffer, " ", 2);
                            strncat(buffer, buf, TIME_LEN);
                            fd = scrivi_file(buffer, IP_BANNATI, "a");
                            free(buffer);
                            
                            trasmetti("troppi tentativi, BAN di 30 minuti", new_socket);
                            
                            ret = close(new_socket);
                            if(ret < 0){
                                perror("Error in close : \n");
                                free(username);
                                free(password);
                                exit(1);
                            }
                            
                            do {
                              time(&current_time);
                              //printf("LOG_DEBUG: current_time: %s\n", ctime(&current_time));
                              //printf("LOG_DEBUG: ban_time: %s\n", ctime(&ban_time));
                              //printf("LOG_DEBUG: current_time (intero): %d\n", (int)current_time);
                              //printf("LOG_DEBUG: ban_time (intero): %d\n", (int)ban_time);
                              printf("Controllo sullo scorrere del tempo\n");
                              sleep(5); // Controllo ogni minuto, per efficienza
                            } while((int)current_time < (int)ban_time + 60); // DEBUG
                            
                            printf("LOG_DEBUG: uscito dal ciclo while sullo scorrere del tempo\n");
                            // se sono qui è passato il tempo necessario
                            aggiorna_bannati(savedIP);
                            termina_server = 1;
                            
                            
                        } else if(contatore < 3){ // login riuscito
                            static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                            int i;
                            
                            srand(time(0)); 
                            for(i = 0; i < SESSION_ID_LEN; i++){
                                int index;
                                index = (int)((double)rand() / ((double)RAND_MAX + 1) * strlen(alphanum));
                                printf("LOG_DEBUG: iterazione numero: %d --> ", i);
                                sessionID[i] = alphanum[index]; 
                                printf("%c\n", sessionID[i]);
                                printf("Indice: %d\n", index);
                            }	
                            sessionID[SESSION_ID_LEN] = '\0';
                            printf("LOG_DEBUG: sessionID <---> %s\n", sessionID);
                            
                            trasmetti("Login effettuato con successo", new_socket);

                            trasmetti(sessionID, new_socket);

                            stringa = calloc(BUFFER_LENGHT, sizeof(char));
                            percorso_file_registro(username, stringa);
                            scrivi_file(sessionID, stringa, "a");
                            free(stringa);
                                                        
                        } else { // errore inaspettato
                            printf("Errore inaspettato, termino con un errore\n");
                            free(username);
                            free(password);
                            exit(1);
                        }
                        free(username);
                        free(password);
                        break;

                    case 4: // #################################################### invia_giocata
                    { 
                        struct schedina *punt_schedina;
                        
                        printf(" > > > > > > > >  Ricevuto un comando di invia_giocata < < < < < < < < \n");

                        // ricevo il session ID e controllo 
                        ricevi(receivedID, new_socket);
                        
                        if(sessionID[0] == '\0'){
                            trasmetti("Login non effettuato", new_socket);
                        }
                        
                        // printf("LOG_DEBUG: sessionID: %s\n", sessionID);
                        // printf("LOG_DEBUG: receivedID: %s\n", receivedID);
                        if(strncmp(receivedID, sessionID, 10) != 0){
                            trasmetti("sessionID non rilevato", new_socket);
                            printf("receivedID %s non corretto\n", receivedID);
                            printf("sessiond ID: %s\n", sessionID);
                            printf("Termino con un errore\n");
                            exit(1);
                        }
                        
                        trasmetti("sessionID corretto", new_socket);
                        
                        punt_schedina = (struct schedina*)calloc(1, sizeof(struct schedina));
                        
                        // lettura delle ruote
                        printf("LOG_DEBUG: Lettura delle ruote\n");
                        i = 0;
                        punt_schedina->quante_ruote = 0;
                        while(1){
                            char *parametro;
                            
                            parametro = calloc(WHEEL_LENGHT, sizeof(char));
                            ricevi(parametro, new_socket);
                            
                            if(strcmp(parametro, "tutte") == 0){ // tutte le ruote
                                punt_schedina->quante_ruote = 11;
                                punt_schedina->flag_tutte = 's';
                                free(parametro);
                                break;
                                
                            } else if(strcmp(parametro, "fine -r") == 0){ // terminano le ruote
                                free(parametro);
                                break;
                                
                            } else { // ricevo una ruota
                                punt_schedina->quante_ruote++;
                                parametro[strlen(parametro)] = '\0';
                                strncpy(punt_schedina->ruote[i], parametro, strlen(parametro) + 1);
                                free(parametro);
                                i++;
                            }  
                        }
                        
                        // lettura dei numeri 
                        printf("LOG_DEBUG: Lettura dei numeri giocati\n");
                        punt_schedina->quanti_numeri = 0;
                        i = 0;
                        while(1){
                            char *parametro;
                            
                            parametro = calloc(RESPONSE_LEN, sizeof(char));
                            ricevi(parametro, new_socket);
                            
                            if(strcmp(parametro, "Troppi numeri") == 0){ // errore
                                free(parametro);
                                printf("Ricevuto un messaggio di troppi numeri inseriti\n");
                                printf("Terminazione del server\n");
                                close(new_socket);
                                exit(1);
                                
                            } else if(strcmp(parametro, "fine -n") == 0){ // proseguo con la prossima opzione 
                                free(parametro);
                                break;
                                
                            } else if(strcmp(parametro, "Parametro mancante") == 0) {
                                free(parametro);
                                printf("Ricevuto un messaggio di troppi numeri inseriti\n");
                                printf("Terminazione del server\n");
                                close(new_socket);
                                exit(1);
                            } else { // ricevo una ruota
                                punt_schedina->quanti_numeri++;
                                punt_schedina->numeri_giocati[i] = atoi(parametro);
                                free(parametro);
                                i++;
                            }  
                        }
                        
                        // leggo gli importi
                        printf("LOG_DEBUG: Lettura degli importi\n");
                        punt_schedina->quanti_importi = 0;
                        i = 0;
                        while(1){
                            char *parametro;
                            
                            parametro = calloc(RESPONSE_LEN, sizeof(char));
                            ricevi(parametro, new_socket);
                            
                            if(strcmp(parametro, "Errore nell'importo") == 0){ // errore
                                free(parametro);
                                printf("Ricevuto un messaggio di errore nell'importo\n");
                                printf("Terminazione del server\n");
                                close(new_socket);
                                exit(1);
                                
                            } else if(strcmp(parametro, "fine comando invia_giocata") == 0){ // proseguo
                                free(parametro);
                                break;
                                
                            } else { // ricevo una ruota
                                punt_schedina->quanti_importi++;
                                punt_schedina->importi[i] = atoi(parametro);
                                free(parametro);
                                i++;
                            }  
                        }
                        
                        printf("---  ---  ---  STAMPA DELLA SCHEDINA  ---  ---  ---\n");
                        stampa_schedina(punt_schedina);
                        // inserisco la schedina in una lista di giocate attive
                        
                        // scrivo la schedina nel file di registro
                        
                        // invio risposta
                        
                        // serve una funzione che stampa la schedina anche solo per debug
                        break;
                    }
                    case 5: // #################################################### vedi_giocata

                        break;

                    case 6: // #################################################### vedi_estrazioni

                        break;

                    case 7: // #################################################### esci

                        termina_server = 1;
                        break;

                    default: // #################################################### default

                        break;

                }

                // TERMINO
                if(termina_server != 0){
                    printf("Terminazione del server\n");
                    close(new_socket);
                    exit(0);
                }
                
                // printf("LOG_DEBUG: inizia un nuovo ciclo while del figlio\n");
            } // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FINE CICLO INFINITO processo FIGLIO %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            
            // forse questa parte è svolta da termina_server e va tolta
            printf("Chiusura del socket di comunicazione nel processo figlio\n");
            close(new_socket);
            exit(0);
            
        } else if(pid < 0){ // errore
            printf("LOG_DEBUG: errore nella chiamata a fork()\n");
            exit(1);
            
        } else if(pid > 0) { // caso padre
            
        printf(" ~~~~~~~~~~~~~~~~ Sono il processo PADRE in lotto_server! ~~~~~~~~~~~~~~~~ \n");
        close(new_socket);

        // TODO 
        
        } 
        
    } // // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FINE CICLO INFINITO processo PADRE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
   
    
    exit(0);
}