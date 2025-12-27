#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80

void execSeq(char** args) 
{
    pid_t pid;

    pid = fork();

    if (pid == 0) { 
        
        execvp(args[0], args);

        if(execvp(args[0], args) == -1) {   // Valido se o comando é valido
                                            // Valido se ele está com a linha está sem comandos ou "vazia" 
            if(strcmp(args[0], "\0") == 0 || strcmp(args[0], "\n") == 0 || strcmp(args[0], " ") == 0) {
                printf("No commands\n");
                exit(EXIT_SUCCESS);
            }
            else if(strcmp(args[0], "!!") == 0 || strcmp(args[0], "style") == 0) 
            {
                exit(EXIT_SUCCESS);
            }
            else {
                printf("%s: invalid command\n", args[0]); 
            }
              
        } 
        exit(EXIT_SUCCESS); // Encerro o processo
    }
    else if(pid > 0) {
        wait(NULL);
        //printf("\n\n\n\n\n\n%s\n", args[1]);
    }   
}

void execPar(char** args, int tamanhoSub)
{
    pid_t pid;

    pid = fork();

    if (pid == 0)
    { 
        if(execvp(args[0], args) == -1)
        {
            if(strcmp(args[0], "\0") == 0 || strcmp(args[0], "\n") == 0 || strcmp(args[0], " ") == 0)
            {
                printf("No commands\n");
                exit(EXIT_SUCCESS);
            }
            else if(strcmp(args[0], "!!") == 0 || strcmp(args[0], "style") == 0)
            {
                exit(EXIT_SUCCESS);
            }
            else
            {
                printf("%s: invalid command\n", args[0]);
            }  
        }
        //waitpid(pid, NULL, 0);   
        exit(EXIT_SUCCESS);
    }
    //printf("ESSE É O TAMANHO DAS SUBSTRINGS -> %d\n", tamanhoSub);
}

void execPipe(char** args, char** arg1, char** arg2)
{
    int fd[2];
    pid_t pid1;
    pid_t pid2;
    //char *arg1[MAX_LINE/2+1];
    //char *arg2[MAX_LINE/2+1];

        printf("Chegou pra executar!\n");
    if(pipe(fd) == -1)
    {
        exit(EXIT_FAILURE);
    }

    pid1 = fork();

    if(pid1 < 0)
    {
        printf("Child ERRO!\n");
        exit(EXIT_FAILURE);
    }

    if(pid1 == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(arg1[0], arg1);
    }

    pid2 = fork();

    if(pid2 < 0)
    {
        printf("Child ERRO!\n");
        exit(EXIT_FAILURE);
    }

    if(pid2 == 0)
    {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(arg2[0], arg2);
    }

    close(fd[0]);
    close(fd[1]);
    
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    for(int i = 0; i < 80; i++){
        wait(NULL);
    }
}

int validadorPIPE(char** args, int tamanhoSubString, char* acao, int ponteiroPIPE)
{   
    for(int i = 0; i < tamanhoSubString; i++)
    {
        //printf("Esse foi o comando -> %s\n", args[i]);
        if (strcmp(args[i], acao) == 0)
        {
            printf("TEM PIPE!\n");
            ponteiroPIPE = i;
            return i;
        }
        
    }
    //printf("AQUI FOI O ULTIMO CODIGO -> %s\n", args[0]);
    //printf("AQUI FOI O ULTIMO CODIGO -> %s\n", args[1]);
    return 0;
}

void separadorDeComandosPipe(char* arrayComPipe, int tamanhoSubString, int localizacaoPIPE)
{
    char *argsSemPIPE[MAX_LINE/2 + 1];
    char **prox = argsSemPIPE;
    char *argsTemp = strtok(arrayComPipe, "|");
    int tamanhoSUB2 =0;
    
    char *arg1[MAX_LINE/2 + 1];
    char *arg2[MAX_LINE/2 + 1];

    while (argsTemp != NULL)
    {
        *prox++ = argsTemp;
        argsTemp = strtok(NULL, "|");
        tamanhoSUB2++;
    }
    *prox = NULL;
    printf("\n\n%d\n\n", tamanhoSUB2);
    for(int ii = 0; ii < tamanhoSubString; ii++)
    {

    }
    //printf("\n\n%s\n\n", argsSemPIPE[2]);
    printf("Passou daqui!\n");
    execPipe(argsSemPIPE, arg1, arg2);
}

void separadorDeComandoSpace(char *listComandos, int modoDeOperacao, int tamanhoSubstring)
{
    char *args[MAX_LINE/2 + 1];
    char **prox = args;
    char *argsTemp = strtok(listComandos, " ");

    int localizacaoRedirect = 0;
    int localizacaoPIPE = 0;

    char *acaoPipe = "|";
    char *arg1[MAX_LINE/2 + 1];
    char *arg2[MAX_LINE/2 + 1];
    int auxArg2 = 0;

    //printf("\n\n\nTamanho1 da SUBSTRING -> %d\n", tamanhoSubstring);
    tamanhoSubstring = 0;

    while (argsTemp != NULL)
    {
        *prox++ = argsTemp;
        argsTemp = strtok(NULL, " ");
        tamanhoSubstring++;
    }
    
    *prox = NULL;
    
    //printf("\n\n\nTamanho2 da SUBSTRING -> %d\n", tamanhoSubstring);
    localizacaoPIPE = validadorPIPE(args, tamanhoSubstring, acaoPipe, localizacaoPIPE);
    //DEBUG localização pipe
    //printf("\n\n%d\n\n", localizacaoPIPE);
    if(localizacaoPIPE >= 1)
    {   
        printf("Entrou certo\n");
        printf("Tamanho da SUBSTRING ANTES DO PIPE = %d\n", tamanhoSubstring);
        for(int ii = 0; ii < tamanhoSubstring; ii++)
        {   
            if(ii < localizacaoPIPE)
            {
                arg1[ii] = args[ii];
            }
            if(ii == localizacaoPIPE)
            {
                //arg1[ii] = NULL;
            }
            if(ii > localizacaoPIPE)
            {
                arg2[auxArg2] = args[ii];
                auxArg2++;
                //printf("11 ARG2 = %s\n", arg2[auxArg2]);
            }
            if(ii == tamanhoSubstring - 1)
            {
                //arg2[auxArg2 + 1] = NULL;
            }
            
        }
        //DEBUG
        printf("ARG1 = %s\n", arg1[0]);
        printf("ARG2 = %s\n", arg2[0]);
        execPipe(args, arg1, arg2);
        //separadorDeComandosPipe(listComandos, tamanhoSubstring, localizacaoPIPE);
    }

    if(modoDeOperacao == 0)
    { 
        execSeq(args);
    }
    else if(modoDeOperacao == 1)
    {   
        execPar(args, tamanhoSubstring);
    }
}

void separadorDeComandoPontoVirgula(char* linhaDeComando, int modoDeOperacao)
{   

    char *args[MAX_LINE/2 + 1];             // Cria a array auxiliar, com o tamanho max da array
    memset(args, '\0', sizeof(args));       // Limpa a array
    char **prox = args;                     // Cria a array auxiliar, para indicar o prox comando
    memset(prox, '\0', sizeof(prox));       // Limpa a array
    char *argsTemp = strtok(linhaDeComando, "\n"); // valida que a array auxiliar não vai ter quebra de linha
    char *listComandos[MAX_LINE/2 + 1];     // Cria a array auxiliar, onde vai ter os comandos

    int i = 0;

    argsTemp = strtok(linhaDeComando, ";"); // Retira o ;
    
    while (argsTemp != NULL)                // Aponta para o proximo valor, para limpalo, e apos isso acrescentar
    {                                       // novamente na array auxiliar 
        *prox++ = argsTemp;
        listComandos[i] = strdup(argsTemp);
        i++;
        argsTemp = strtok(NULL, ";");
    }
    //printf("ESSE É O TAMANHO DAS SUBSTRINGS -> %d\n", i);

    *prox = NULL;                           // Aponta que o ultimo falor da array é NULL

    for(int j = 0; j < i; j++)              // Passa a array para o proximo parsing
    {
        separadorDeComandoSpace(listComandos[j], modoDeOperacao, i);
    }
}

int main(int argc, char *argv[])
{
    
    int codigoSaida = 1;              /* flag para sair do programa */
    int modoDeAtuacao = 0;            /* 0 = seq , 1 = prll */

    char linhaDeComando[MAX_LINE/2 + 1];

    char seq[] = "style sequential";
    char prll[] = "style parallel";
    char history[MAX_LINE/2 + 1];
    char linhaDeComandoARMAZENADA[MAX_LINE/2 + 1];

    // Faço a verificação do history no começo de cada operação, assim que ele
    // pega a linha no fgets, ele ja adiciona no history.
    if(!linhaDeComando) {       
        printf("fvm seq> Memory Allocation Error!\n");
        exit(EXIT_FAILURE);
    }

    if(argc < 2) // Valida se for passado um arquivo
    { 
    
        while (codigoSaida) {
            if(modoDeAtuacao == 0) {
                printf("fvm seq> ");
                //memset(linhaDeComando, '\0', sizeof(linhaDeComando));
                //fgets(linhaDeComando, sizeof(linhaDeComando), stdin);
                
                if (fgets(linhaDeComando, MAX_LINE/2 + 1, stdin) == NULL)
                {
                    printf("\n");
                    exit(EXIT_SUCCESS);  
                }    

                if (strcmp(linhaDeComando,"exit\n") == 0) // função exit
                {
                    codigoSaida = 0;
                    exit(EXIT_SUCCESS);
                }

                if(strcmp(linhaDeComando, "!!\n") == 0) {
                    if(strcmp(history, "\0") == 0) {
                        printf("No commands\n");
                    }
                    else {   
                        printf("%s", history);
                        separadorDeComandoPontoVirgula(history, modoDeAtuacao); 
                    }
                }

                strcpy(linhaDeComandoARMAZENADA, linhaDeComando);
                strcpy(history, linhaDeComandoARMAZENADA);
                separadorDeComandoPontoVirgula(linhaDeComandoARMAZENADA, modoDeAtuacao);
                
            }
            if(modoDeAtuacao == 1)
            {   
                printf("fvm par> ");
            
                //fgets(linhaDeComando, sizeof(linhaDeComando), stdin);
                
                if (fgets(linhaDeComando, MAX_LINE/2 + 1, stdin) == NULL)
                {
                    printf("\n");
                    exit(EXIT_SUCCESS);  
                }  

                if (strcmp(linhaDeComando,"exit\n") == 0) { // função exit 
                    codigoSaida = 0;
                    exit(EXIT_SUCCESS);
                }

                if(strcmp(linhaDeComando, "!!\n") == 0) {
                    if(strcmp(history, "\0") == 0) {
                        printf("No commands\n");
                    }
                    else {   
                        printf("%s", history);
                        separadorDeComandoPontoVirgula(history, modoDeAtuacao); 
                    }
                }

                strcpy(linhaDeComandoARMAZENADA, linhaDeComando);
                strcpy(history, linhaDeComandoARMAZENADA);
                separadorDeComandoPontoVirgula(linhaDeComandoARMAZENADA, modoDeAtuacao);

                for(int i = 0; i < 80; i++){
                    wait(NULL);
                }

            }
            
            if(strncmp(linhaDeComando, seq, strlen(seq)) == 0) {
                //printf("Modo de leitura alterado para SEQUENCIAL!\n");
                //fflush(stdout);
                modoDeAtuacao = 0;
            }
            else if(strncmp(linhaDeComando, prll, strlen(prll)) == 0) {
                //printf("Modo de leitura alterado para PARALELO!\n");
                //fflush(stdout);
                modoDeAtuacao = 1;
            }
            else {
                codigoSaida = 1;
            }
        }
        return 0;
    }
    else if(argc == 2) // ler linha de comando linha a linha
    {   
        char stringPrintavel[MAX_LINE/2+1];
        FILE *arquivo = fopen(argv[1], "r");
        int tamanhoLinhasArquivo;

        tamanhoLinhasArquivo = 0;

        if(arquivo == NULL) 
        {
            printf("Couldn't open this file. Try check the file name.\n");
            exit(EXIT_FAILURE);
        }
        printf("Command list: \n");

        while(!feof(arquivo)) // Lê o arquivo printando cada uma das linhas
        {   
            fgets(stringPrintavel, sizeof(stringPrintavel), arquivo);
            //fscanf(arquivo,"%s", stringPrintavel);
            printf("%s",stringPrintavel);
            memset(stringPrintavel, 0, sizeof(stringPrintavel));
            tamanhoLinhasArquivo++;
        }
        fclose(arquivo);
        //printf("%d", tamanhoLinhasArquivo); USADO PARA SABER O TAMANHO DE LINHAS DO ARQUIVO
        printf("\nCommand execution: \n");

        arquivo = fopen(argv[1], "r");
        int tamanhoTotal = 0;
        while(!feof(arquivo) || codigoSaida) // Lê o arquivo printando cada uma das linhas
        {   
            tamanhoTotal++;
            //if(tamanhoTotal == tamanhoLinhasArquivo)
            //{
                //exit(EXIT_SUCCESS);
            //}
            if(modoDeAtuacao == 0) {
            
                fgets(linhaDeComando, sizeof(linhaDeComando), arquivo);
                   
                if (strcmp(linhaDeComando,"exit\n") == 0 || strncmp(linhaDeComando,"exit", strlen("exit")) == 0) // função exit
                {
                    codigoSaida = 0;
                    fclose(arquivo);
                    exit(EXIT_SUCCESS);
                }

                if(strcmp(linhaDeComando, "!!\n") == 0) {
                    if(strcmp(history, "\0") == 0) {
                        printf("No commands\n");
                    }
                    else {   
                        printf("%s", history);
                        separadorDeComandoPontoVirgula(history, modoDeAtuacao); 
                    }
                }

                strcpy(linhaDeComandoARMAZENADA, linhaDeComando);
                //memset(linhaDeComando, 0, sizeof(linhaDeComando));
                strcpy(history, linhaDeComandoARMAZENADA);
                separadorDeComandoPontoVirgula(linhaDeComandoARMAZENADA, modoDeAtuacao);
                
            }
            if(modoDeAtuacao == 1)
            {   
            
                fgets(linhaDeComando, sizeof(linhaDeComando), arquivo);
                   
                if (strcmp(linhaDeComando,"exit\n") == 0 || strncmp(linhaDeComando,"exit", strlen("exit")) == 0) { // função exit 
                    codigoSaida = 0;
                    fclose(arquivo);
                    exit(EXIT_SUCCESS);
                }

                if(strcmp(linhaDeComando, "!!\n") == 0) {
                    if(strcmp(history, "\0") == 0) {
                        printf("No commands\n");
                    }
                    else {   
                        printf("%s", history);
                        separadorDeComandoPontoVirgula(history, modoDeAtuacao); 
                    }
                }

                strcpy(linhaDeComandoARMAZENADA, linhaDeComando);
                strcpy(history, linhaDeComandoARMAZENADA);
                separadorDeComandoPontoVirgula(linhaDeComandoARMAZENADA, modoDeAtuacao);

                for(int i = 0; i < 80; i++){
                    wait(NULL);
                }

            }
            
            if(strncmp(linhaDeComando, seq, strlen(seq)) == 0) {
                modoDeAtuacao = 0;
            }
            else if(strncmp(linhaDeComando, prll, strlen(prll)) == 0) {
                modoDeAtuacao = 1;
            }
            else {
                codigoSaida = 1;
            }
            if(tamanhoTotal == tamanhoLinhasArquivo)
            {
                exit(EXIT_SUCCESS);
            }
        }

        fclose(arquivo);
        return 0;
    }
    else 
    {
        printf("Uncommon error ocorred, please try to to pass less arguments, or verify if the file has the right name.\n");
    }
}