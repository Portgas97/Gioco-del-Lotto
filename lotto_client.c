#include "/home/studenti/NetBeansProjects/funzioni.h"

// IL CLIENT DEVE USARE LA PRIMITIVA SELECT PER GESTIRE PIÙ CANALI DI I/O

// MEGLIO RENDERE USERNAME E PASSWORD ARRAY DINAMICI

/* https://stackoverflow.com/questions/39402886/select-on-stdin-and-incoming-socket */


int main(int argc, char* argv[]){
   
    // variabili per il socket
    int ret, socket_locale;
    struct sockaddr_in server_address;
    socklen_t address_len;
    char *connection_result;
    
    // ID di sessione per l'utente
    char sessionID[SESSION_ID_LEN];
    memset(sessionID, 0, CREDENTIAL_LEN);


    
    // validazione dell'input
    // si può fare un controllo SINTATTICO sull'IP??
    if(argc == 1 || argc == 2){
        printf("È necessario inserire due parametri: indirizzo IP del server e porta\n");
        exit(1);
    } 
    else if(argc == 3){
        
        if(strcmp(argv[2],"4242") == 0 && strcmp(argv[1], "127.0.0.1") == 0){ // il secondo controllo non è del tutto corretto
            printf("Porta e indirizzo inseriti correttamente\n");
        } else{
            printf("La porta %s o l'indirizzo %s non sono corretti:\n", argv[2], argv[1]);
            exit(1);
        }
    }
    else{ // default safe
        printf("Numero di argomenti passati non corretto. Inserire indirizzo IP del server e porta\n");
        exit(1);
    }
    
    // creazione di una socket 
    socket_locale = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_locale < 0){
        printf("La primitiva socket() ha fallito, termino con un errore\n");
        perror("Error in socket(): \n");
        exit(1);
    }
    // creazione indirizzo del server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);
    
    // connessione 
    address_len = (socklen_t) sizeof(server_address);
    ret = connect(socket_locale, (struct sockaddr*)&server_address, address_len);

    if(ret < 0){
        printf("La primitiva connect() ha fallito, termino con un errore\n");
        perror("Error in connect(): \n");
        exit(1);
    }
   
    
    
    while(1){ 
        
        // variabili per i comandi
        int i;
        int tmp;
        int index;
        char bool;
        char terminatore;
        char *comando;
        char *buffer;
        char *risposta;
        
        // per la chiusura della connessione 
        int termina_client;
        
        // variabili per gli utenti
        char *username;
        char *password;        
       
        termina_client = 0;
        
        // fflush(stdin);
        connection_result = calloc(RESPONSE_LEN, sizeof(char));
        ricevi(connection_result, socket_locale);
        if(strcmp(connection_result, "Sei stato bannato, riprova più tardi") == 0){
            printf("Sei stato bannato per eccesso di tentativi di login, riprova più tardi\n");
            exit(0);
        }
        free(connection_result);
        
        printf("LOG_DEBUG: connessione effettuata con successo\n");
        stampa_menu(0);
        
        
        // prelievo del comando, contiene l'intera stringa con comando e parametri
        printf("Inserisci un comando: \n");
        comando = calloc(BUFFER_LENGHT, sizeof(char));
        // per permettere la lettura degli spazi e per consumare \n rimasti nel buffer di input
        scanf(" %[^\n]s", comando);
        /* 
        if(!fgets(comando, MAX_SIZE_INPUT, stdin)){
            printf("Errore nella lettura del comando\n");
            free(comando);
            exit(1);
        }      
        */ 
        printf("LOG_DEBUG: comando inserito: ");
        printf("%s\n", comando);
        printf("\n");
        comando[strlen(comando)] = '\0';

        if(comando[0] != '!'){
            printf("Il comando deve iniziare per !\n");
            free(comando);
            exit(1);
        }

        // elimino il carattere !
        for(i = 0; i < strlen(comando) + 1; i++)
            comando[i] = comando[i+1];


        printf("\n LOG_DEBUG: ^^^^^^^^^^^^^ ANALISI DEL COMANDO ^^^^^^^^^^^^^ \n");
        printf("\n");

        // estraggo il comando e lo inserisco nel buffer
        index = 0;
        buffer = calloc(MAX_SIZE_PARAMETER, sizeof(char));
        terminatore = sotto_stringa(comando, buffer, &index);
        buffer[strlen(buffer)] = '\0';
        // associo un numero al comando
        tmp = rileva_comando(buffer);
        // printf("LOG_DEBUG: Valore di tmp: %d\n", tmp);
        // printf("LOG_DEBUG: È stato inserito il comando numero %d\n", tmp);
        
        // controllare exit(1) e semmai sostituire con termina_client = 1.
        printf("\nLOG_DEBUG: esecuzione dello switch nel client\n");
        
        // SELEZIONE DEL COMANDO
        switch(tmp){

            case 0: // ######################################################## errore
                
                free(comando); // DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server

                printf("Nessun comando rilevato\n");
                
                termina_client = 1;
                break;

            case 1: // ######################################################## help 
                
                free(buffer); 
                printf("\n ----> Elaborazione del comando help <---- \n");
                
                bool = rileva_parametro(comando, &index, terminatore);

                if(bool == 'n') // non è presente un parametro
                    stampa_menu(0); 
                else if(bool == 's'){ // può esserci solo un parametro
                    buffer = calloc(MAX_SIZE_PARAMETER, sizeof(char));
                    terminatore = sotto_stringa(comando, buffer, &index);
                    
                    tmp = rileva_comando(buffer);
                    if(tmp == 0){
                        printf("Il parametro inserito non è ammesso \n");
                        termina_client = 1;
                        free(comando);
                        free(buffer);
                        break;
                    }
                        
                    free(comando);
                    free(buffer);
                    stampa_menu(tmp-1);
                    
                } else{
                    free(comando);
                    printf("Errore inaspettato, termino\n");
                    termina_client = 1;
                }
                
                break;

            case 2: // ######################################################## signup
                
                printf("\n ----> Elaborazione del comando signup <---- \n");
                
                // 1) invio il comando ricevuto
                trasmetti(buffer, socket_locale);
                free(buffer);
                
                printf("Ho inviato il comando al server! \n");
                
                printf("LOG_DEBUG: comando: %s\n", comando);
                username = calloc(CREDENTIAL_LEN, sizeof(char));
                // 2) invio username (INVIARE UN MESSAGGIO PER QUANDO USERNAME È VUOTO)
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, username, &index);
                
                trasmetti(username, socket_locale);
                
                password = calloc(CREDENTIAL_LEN, sizeof(char));
                // 3) invio password (INVIARE UN MESSAGGIO PER QUANDO PASSWORD È VUOTO)
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, password, &index);

                trasmetti(password, socket_locale);                
                
                free(comando);
                
                // 4) leggo la risposta
                printf("Risposta del server\n");
                
                risposta = calloc(RESPONSE_LEN, sizeof(char));
                ricevi(risposta, socket_locale);
                 
                if(strcmp(risposta, "username esistente") == 0){
                    printf("È necessario rieseguire il comando !signup, username già esistente\n");
                    
                } else if(strcmp(risposta, "password non adatta") == 0){
                    printf("È necessario rieseguire il comando !signup, minima lunghezza della password consentita: 4 caratteri\n");
                    
                } else if(strcmp(risposta, "signup effettuato") == 0){
                    printf("signup effettuato correttamente!\n");
                    
                } else {
                    printf("Errore inaspettato, termino\n");
                    free(username);
                    free(password);
                    free(risposta);
                    exit(1);
                    
                }
                
                free(username);
                free(password);
                free(risposta);
                break;

            case 3: // ######################################################## login
                
                printf("\n ----> Elaborazione del comando login <---- \n");
                
                // invio il comando
                trasmetti(buffer, socket_locale);
                printf("Ho inviato il comando %s al server! \n", buffer);
                free(buffer);
                
                risposta = calloc(RESPONSE_LEN, sizeof(char));
                ricevi(risposta, socket_locale);
                if(strcmp("login effettuato", risposta) == 0){
                    printf("La fase di login è già stata effettuata, eseguire altri comandi\n");
                    free(risposta);
                    break;
                }
                free(risposta);
                
                username = calloc(CREDENTIAL_LEN, sizeof(char));
                // invio lo username 
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, username, &index);
                 
                trasmetti(username, socket_locale);
                
                password = calloc(CREDENTIAL_LEN, sizeof(char));
                // invio la password
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, password, &index);
                
                trasmetti(password, socket_locale);  
                
                free(username);
                free(password);
                free(comando);
                
                while(1){
                    
                    risposta = calloc(RESPONSE_LEN, sizeof(char));
                    ricevi(risposta, socket_locale);
                    
                    if(strcmp(risposta, "username inesistente, reinserire credenziali") == 0){
                        
                        free(risposta);
                        username = calloc(CREDENTIAL_LEN, sizeof(char));
                        printf("Username inesistente, è necessario reinserire le credenziali \n");
                        printf("Username: ");
                        scanf("%s", username);
                        trasmetti(username, socket_locale);
                        free(username);
                        
                        password = calloc(CREDENTIAL_LEN, sizeof(char));
                        printf("Password: ");
                        scanf("%s", password);
                        trasmetti(password, socket_locale);
                        free(password);
                        
                        
                    } else if(strcmp(risposta, "troppi tentativi, BAN di 30 minuti") == 0) { // BAN
                        free(risposta);
                        printf("Utente bannato per 30 minuti, troppi tentativi\n");
                        termina_client = 1;
                        break;
                    } else if(strcmp(risposta, "Login effettuato con successo") == 0){ // success
                        free(risposta);
                        ricevi(sessionID, socket_locale);
                        printf("Login effettuato con successo, il tuo session ID è: %s\n", sessionID);
                        break;
                    } else {
                        free(risposta);
                        printf("Errore inaspettato, termino con un errore\n");
                        exit(1);
                    }
                }
                
                break; 

            case 4:// ######################################################## invia_giocata
            {    
                char *opzione;
                char *parametro;
                
                printf("\n ----> Elaborazione del comando invia_giocata <---- \n");
                
                trasmetti(buffer, socket_locale);
                printf("Ho inviato il comando %s al server! \n", buffer);
                free(buffer);
                // trasmetto l'id dell'utente
                printf("LOG_DEBUG: sessionID: %s\n", sessionID);
                trasmetti(sessionID, socket_locale);
                
                risposta = calloc(RESPONSE_LEN, sizeof(char));
                ricevi(risposta, socket_locale);
                    
                if(strcmp(risposta, "Login non effettuato") == 0){
                    printf("Eseguire prima la fase di login\n");
                    break;
                }
                
                if(strcmp(risposta, "sessionID non rilevato") == 0){
                    printf("Errore in ricezione sul corretto scambio di sessionID\n");
                    free(risposta);
                    exit(1);
                }
                
                free(risposta);
                
                // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Lettura delle ruote ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
                opzione = calloc(2, sizeof(char));
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, opzione, &index);
                
                if(strcmp(opzione, "-r") != 0){
                    free(comando);
                    free(opzione);
                    printf("Errore, parametro -r non rilevato\n");
                    exit(1);
                }
                printf("lOG_DEBUG: Lettura dei parametri per l'opzione -r\n");
                free(opzione);
                
                parametro = calloc(WHEEL_LENGHT, sizeof(char));
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, parametro, &index); 
                
                // DA AGGIUNGERE IL CONTROLLO SULL'ESISTENZA DELLA RUOTA
                while(strcmp(parametro, "-n") != 0){
                  
                    if(strcmp(parametro, "tutte") == 0 && i != 0){ // non si può scrivere "tutte" dopo aver scritto altre ruote
                        free(buffer);
                        free(comando);
                        free(parametro);
                        printf("Non si può scrivere \'tutte\' dopo aver inserito altre ruote\n");
                        exit(1);
                    }
                    else if(strcmp(parametro, "tutte") == 0 && i == 0){
                        trasmetti("tutte", socket_locale);
                        break;
                    }

                    trasmetti(parametro, socket_locale);
                    free(parametro);
                    parametro = calloc(WHEEL_LENGHT, sizeof(char));
                    bool = rileva_parametro(comando, &index, terminatore);
                    acquisisci_parametro(bool, comando, parametro, &index); 
                } 
                
                trasmetti("fine -r", socket_locale);
                
                // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Lettura dei numeri giocati ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
                opzione = calloc(2, sizeof(char));
                strncpy(opzione, parametro, 3); // per leggibilità le opzioni le mettiamo nella variabile opzione
                free(parametro);
                
                if(strcmp(opzione, "-n") != 0){
                    free(comando);
                    free(opzione);
                    printf("Errore, parametro -n non rilevato\n");
                    exit(1);
                }
                printf("lOG_DEBUG: Lettura dei parametri per l'opzione -n\n");
                free(opzione);
                
                parametro = calloc(2, sizeof(char));
                bool = rileva_parametro(comando, &index, terminatore);
                acquisisci_parametro(bool, comando, parametro, &index); 
                i = 1; // conto i numeri giocati

                while(strcmp(parametro, "-i") != 0){
                    trasmetti(parametro, socket_locale);
                    free(parametro);
                    
                    parametro = calloc(2, sizeof(char));
                    bool = rileva_parametro(comando, &index, terminatore);
                    acquisisci_parametro(bool, comando, parametro, &index);
                    
                    if(bool == 'n'){
                        printf("Parametro mancante\n");
                        trasmetti("Parametro mancante", socket_locale);
                        free(parametro);
                        free(comando);
                        exit(1);
                    }
                    
                    i++;
                    if(i > 10){
                        trasmetti("Troppi numeri", socket_locale);
                        printf("Hai inserito più di 10 numeri, termino\n");
                        free(parametro);
                        free(comando);
                        exit(1);
                    }
                } 
                
                trasmetti("fine -n", socket_locale);
                
                // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Lettura degli importi ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
                opzione = calloc(2, sizeof(char));
                strncpy(opzione, parametro, 3); // per leggibilità le opzioni le mettiamo nella variabile opzione
                free(parametro);
                
                if(strcmp(opzione, "-i") != 0){
                    free(comando);
                    free(opzione);
                    printf("Errore, parametro -n non rilevato\n");
                    exit(1);
                }
                printf("lOG_DEBUG: Lettura dei parametri per l'opzione -i\n");
                free(opzione);
                
                parametro = calloc(3, sizeof(char)); // importo massimo pari a 200€
                bool = rileva_parametro(comando, &index, terminatore);
                
                while(bool != 'n'){
                    
                    if(index > strlen(comando)){
                        break;
                    }
                    
                    acquisisci_parametro(bool, comando, parametro, &index);
                    if(strlen(parametro) > 3){
                        trasmetti("Errore nell'importo", socket_locale);
                        printf("Importo inserito non corretto\n");
                        free(parametro);
                        free(comando);
                        exit(1);
                    }
                    trasmetti(parametro, socket_locale);
                    free(parametro);
                    parametro = calloc(3, sizeof(char));
                    bool = rileva_parametro(comando, &index, terminatore); 
                }
                
                free(parametro);
                free(comando);
                trasmetti("fine comando invia_giocata", socket_locale);
                
                // ricevo una risposta ....
                break;
            }
            case 5: // ######################################################## vedi_giocata
                
                // free(comando); // DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server
                printf("\n ----> Elaborazione del comando vedi_giocata <---- \n");
                
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server
                //operazioni
                break;

            case 6: // ######################################################## vedi_estrazioni
                
                // free(comando); // DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server
                printf("\n ----> Elaborazione del comando vedi_estrazioni <---- \n");
                //operazioni
                break;

            case 7: // ######################################################## esci
                
                // free(comando); // DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server
                printf("\n ----> Elaborazione del comando esci <---- \n"); // DA CAMBIARE 
                printf("Terminazione del client lotto_client! \n");
                termina_client = 1;
                break;

            default: // ####################################################### default
                
                // free(comando); // DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE
                // free(buffer); DA VERIFICARE SE QUI SERVE E SEMMAI NON CHIAMARE FREE, spesso serve perchè si invia il comando al server
                printf("Errore nello switch di selezione del comando\n");
                exit(1);

        }
       
        if(termina_client != 0)
            break;

        //DENTRO AL CICLO INFINITO
    }
        // FUORI DAL CICLO INFINITO
    
    // chiusura connessioni???
    printf("Terminazione del client\n");
    close(socket_locale);
    exit(0);
}