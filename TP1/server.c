#include "common.h"
#include "constants.h"
#include "location.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>


Location location;
Location locations[MAX_NUMBER_OF_LOCATIONS];


void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

// RECEBER O TIPO DE OPERAÇAO E
//E EXECUTA A FUNCAO RESPECTIVA
void selectOperation(int clientSocket, char *operation)
{
    

    if ( strcmp(operation,"add") == 0 ) 
    {
        
        addLocation(clientSocket);
    }
    else if ( strcmp(operation ,"rm") == 0 )
    {
        
        removeLocation(clientSocket);
    }
    else if ( strcmp(operation ,"query") == 0)
    {
        //printf("e aq entrou em query ?");
        query(clientSocket);
    }
    else
    {
        sendMessageToClient(clientSocket, " invalid arguments");
    }
    
}

// estou colocando aqui as funções q diz respeito ao server

int checkIsValidAndIsSaved(int clientSocket)
{

    char buffer[BUFFER_SIZE];

    int isValid = checkIfLocationIsValid(location);

    if (!isValid)
    {
        //muydar dps pra sprintf
        //envio da mensagem em buffer
        sprintf(buffer, "invalid location");

        printf("%s\n", buffer);
        //send message function em common ...
        sendMessageToClient(clientSocket, buffer);

        return 0;
    }

    int isSaved = checkIfLocationIsSaved(location, locations);

    if (isSaved)
    {
        //mudar dps pra sprintf
        //envio da mensagem em buffer
        sprintf(buffer, "%d %d already exists", location.x, location.y);

        //send message function em common ...
        sendMessageToClient(clientSocket,buffer);
        return 0;
    }

    return 1;
}

//add location   NAO TESTADA
void addLocation(int clientSocket)
{

    char buffer[BUFFER_SIZE];
    memset(buffer,0,BUFFER_SIZE);

    //LOCAL PROBLEMÁTICO INICIE AQUI !

    if (!checkIsValidAndIsSaved(clientSocket))
    {
        sprintf(buffer,"invalid location");
        return;
        //sendMessageToClient(clientSocket,buffer);
    }

    int indexOfLocationToAdd = getFirstEmptyPlace(location ,locations);

    //printf("valor do index : %d",indexOfLocationToAdd);

    locations[indexOfLocationToAdd].x = location.x;
    locations[indexOfLocationToAdd].y = location.y;

    //send message function localizada em common ...
    sprintf(buffer, "%d %d added", location.x, location.y);
    

    sendMessageToClient(clientSocket,buffer);
}

// remove location 
void removeLocation(int clientSocket)
{

    char buffer[BUFFER_SIZE];
   
    if(!checkIfLocationIsValid(location)){
        printf("tragedia\n");
        return;
    }
   
    int indexOfLocationToRemove = getLocationIndex(location, locations);

    printf("valor de index para remoção : %d ",indexOfLocationToRemove);
    if (indexOfLocationToRemove < 0)
    {
        //send message function em common ...
        //nao existe
        sprintf(buffer, "%d %d does not exist", location.x, location.y);
        
        sendMessageToClient(clientSocket,buffer);
        return;
    }

    locations[indexOfLocationToRemove].x = -1;
    locations[indexOfLocationToRemove].y = -1;

    //send message function em common ...
    sprintf(buffer, "%d %d removed", location.x, location.y);
    //printf("%p\n", &buffer);

    sendMessageToClient(clientSocket,buffer);
}

void listLocations(int clientSocket)
{

    //printf("entrou em listLocations ? \n");
    char buffer[BUFFER_SIZE] = "";

    if (isEmpty(locations))
    {
        sprintf(buffer, "none\n");
    }
    else
    {

        for (size_t i = 0; i < MAX_NUMBER_OF_LOCATIONS; i++)
        {
            if(locations[i].x >= 0 && locations[i].y >= 0){
                if (checkIfLocationIsValid(locations[i])){
                char location[MAX_LOCATION_SIZE] = "";

                sprintf(location, "%d %d ", locations[i].x, locations[i].y);
                strcat(buffer, location);
                }


            }

            
        }
    }
    sendMessageToClient(clientSocket, buffer);
}

void query(int clientSocket)
{

    char buffer[BUFFER_SIZE] = "";

    double smallestDistance = 10000 * sqrt(2);
    Location smallestLocation;

    if (isEmpty(locations))
    {
        sprintf(buffer, "none\n");
    }
    else
    {
        for (size_t i = 0; i < MAX_NUMBER_OF_LOCATIONS; i++)
        {

            if (checkIfLocationIsValid(locations[i]))
            {
                printf("query x: %d\n", location.x);
                double distance = getDistanceBetweenLocations(location, locations[i]);
                printf("loc x: %d dist: %f\n", locations[i].x, distance);

                if (distance < smallestDistance && distance != 0)
                {
                    smallestDistance = distance;
                    smallestLocation = locations[i];
                }
            }
        }
        printf("essa eh a smallest x : %d\n",smallestLocation.x);
        printf("essa eh a smallest y : %d\n",smallestLocation.y);

        sprintf(buffer, "%d %d", smallestLocation.x, smallestLocation.y);
    }

    printf("valor de buffer : %s\n",buffer);

    sendMessageToClient(clientSocket, buffer);
    return;
}

int checkIfOperationIsValid(char *buf) {
    char * token = strtok(buf, " ");

    char operation[MAX_OP_SIZE];
    Location location;

    int count = 0;
    printf("Esse eh o buf: %s|\n", buf);

    if ((strcmp(buf, ADD) == 0)){
        return 1;
    }
    if ((strcmp(buf, REMOVE) == 0)){
        return 1;
    }
    if ((strcmp(buf, QUERY) == 0)){ 
        return 1;
    }
    if ((strcmp(buf, LIST) == 0)){ 
        return 1;
    }

    return 0;
}

void serverExec(char *buf,int clientSocket){


    int contLines=0;
    char *token = strtok(buf,"\n");
    int i = 0;

    char lines[BUFFER_SIZE][BUFFER_SIZE];
    memset(lines,0,BUFFER_SIZE*BUFFER_SIZE);

     while (token != NULL)
    {

  
        strcpy(lines[contLines], token);
        printf("every token : %s\n",token);
        contLines ++;
        token = strtok(NULL, "\n");

    }



    // loop para iterar entre comandos
    printf("contagem de linhas : %d ",contLines);
    for(i=0;i<contLines;i++){

        
        char line[BUFFER_SIZE][BUFFER_SIZE];
        memset(line,0,BUFFER_SIZE*BUFFER_SIZE);
        
        char *tok = strtok(lines[i]," ");
        int numOfTokens =0;
        //int cont=0;
       // printf("Lines[0] depois do strtok? %s\n", lines[0]);
        
        
        
        while(tok != NULL){
            strcpy(line[numOfTokens],tok);
            numOfTokens++;
            tok = strtok(NULL," ");

        }


        // AQUI VAI O TRATAMENTO DE COMANDOS INVALIDOS PELO NUMERO DE TOKENS


        //TRATAMENTO DO PRIMEIRO TOKEN SE EH UM DOS 4 COMANDOS PERMITIDOS
        
        if(checkIfOperationIsValid(lines[i])) {
            if (numOfTokens > 1 && numOfTokens <= 3){

                if (isNumber(line[1]) && isNumber(line[2])) {

                    location.x=atoi(line[1]);
                    location.y=atoi(line[2]);
                    
                    printf(" valor de operation : %s\n",line[0]);
                    printf(" valor de location.x : %d\n",location.x);
                    printf(" valor de location.y : %d\n",location.y);

                    selectOperation(clientSocket,line[i]);
                } else {
                    sendMessageToClient(clientSocket,"Invalid command hereheh");  
                }
                
            }else if(numOfTokens == 1){
                //se o comando so tem 1 palavra eh pq eh list
                listLocations(clientSocket);
            } else {
                sendMessageToClient(clientSocket,"Invalid command hereh");           
            }

            
        } else {
            sendMessageToClient(clientSocket,"Invalid command here");           
        }
    }   
}





int main(int argc, char **argv)
{

    // argumentos invalidos
    if (argc < 3)
    {
        usage(argc, argv);
    }

    // criacao storage
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage(argc, argv);
    }

    //socket server
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logexit("socket");
    }

    // socket options
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    //bind de porta para o server
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage)))
    {
        logexit("bind");
    }

    // w8 connections
    if (0 != listen(s, 10))
    {
        logexit("listen");
    }

    // log espera de conexões
    char addrstr[BUFFER_SIZE];
    addrtostr(addr, addrstr, BUFFER_SIZE);
    printf("bound to %s, waiting connections\n", addrstr);

    //INICIALIZAÇÃO DO AMBIENTE
    initializeLocations(locations);

    //loop server
    while (1)
    {

        // storage e socket client
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        //acc connections
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
        {
            logexit("accept");
        }

        //log conexao cliente
        char caddrstr[BUFFER_SIZE];
        addrtostr(caddr, caddrstr, BUFFER_SIZE);
        printf("[log] connection from %s\n", caddrstr);

        while (1)
        {

            char buf[BUFFER_SIZE] = "";
            memset(buf, 0, BUFFER_SIZE);
            size_t count = recv(csock, buf, BUFFER_SIZE - 1, 0);
            
            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

            //recuperar mensagem total
            char input[BUFFER_SIZE];
            memset(input, 0, BUFFER_SIZE);
            strcpy(input, buf);


            for (size_t i = 0; i <= strlen(input); i++)
            {
                if (input[i] == '\n')
                {
                    input[i] = 0;
                }
            }


            //verificar kill    
            if (strcmp(input, "kill") == 0)
            {
                logexit("killed server");
                close(csock);
                return -1;
            }
        
          

            //execucao servidor "init"
            if ((int)count > BUFFER_SIZE) {
                close(csock);
            }

            serverExec(input,csock);

        }

        exit(EXIT_SUCCESS);
    }
}