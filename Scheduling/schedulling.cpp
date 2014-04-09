/*********************************************************** 
*
*       PONTIFÍCIA UNIVERSIDADE CATÓLICA DE MINAS GERAIS
*                   Engenharia de Computação
*
*  NOME: William Azevedo de Paula
*  
*             Trabalho de conclusão de curso
*-----------------------------------------------------------
*     SCHEDULING DE TAREFAS EM AMBIENTE JOB SHOP 
*-----------------------------------------------------------
*
* Compilado utilizando a IDE Dev-C++ 4.9.9.2
* Sistema Operacional Windows
*-----------------------------------------------------------
*
* Para o correto funcionamento, dentro do diretório onde
* se encontra este código, deve haver o diretório "ga" e,
* dentro deste, os arquivos referentes à biblioteca 
* GALib v. 2.4.7
*
*************************************************************/

#include<stdio.h>
#include<iostream>
#include<string>
#include<fstream>
#include<math.h>
#include<conio.h>
#include <sstream>
#include<time.h>

//Bibliotecas pertencentes à GALib, para usar GARandomInt
#include<ga\garandom.h>
#include<ga\garandom.c>


//Valores Default para variáveis
#define DEFAULT_MUTATION_TAX 3
#define DEFAULT_GENERATIONS_NUMBER 1000
#define DEFAULT_CROSSOVER_TAX 1
#define DEFAULT_POPULATION_SIZE 50
#define DEFAULT_RESOURCE_QUANTITY 10
#define DEFAULT_PROCESS_QUANTITY 10
#define RETRY_TIMES 0
#define DEFAULT_PENALTY  999

//Nome do arquivo de inicialização
#define DEFAULT_INPUT_FILE_NAME "input.txt"

//Textos padrão
#define TEXT_MUTATION_TAX "taxa de mutacao:"
#define TEXT_GENERATIONS_NUMBER "numero de geracoes:"
#define TEXT_CROSSOVER_TAX "quantidade de cruzamentos:"
#define TEXT_POPULATION_SIZE "tamanho da populacao:"
#define TEXT_RESOURCE_QUANTITY "quantidade de recursos:"
#define TEXT_RESOURCE_VECTOR "vetor recursos:"
#define TEXT_PROCESS_VECTOR "matriz processos:"
#define TEXT_PROCESS_QUANTITY "quantidade de processos:"
#define TEXT_RESOURCE_TIME_BY_PROCESS "Tempo de Recurso por Processo:"
#define TEXT_RESOURCE "Recurso"
#define TEXT_PROCESS  "Processo"
#define TEXT_NAME     "Nome";
#define TEXT_UTILIZATION_ORDER "ordem de utilizacao:"
#define TEXT_UTILIZATION_ORDER_BY_PROCESS "ordem de utilizacao por processo"
#define TEXT_DEADLINE "prazo de entrega:"
#define TEXT_TIME "Tempo"

//Não alterar o define a seguir, a não ser que aumente a quantidade de validações em Validate()
#define VALIDATION_QUANTITY 7

using namespace std;

//Variáveis do problema. São inicializadas com os valores padrão, mas podem ser alteradas
int mutation_tax = DEFAULT_MUTATION_TAX; 
int generations_number = DEFAULT_GENERATIONS_NUMBER;
int crossover_tax = DEFAULT_CROSSOVER_TAX;
int population_size = DEFAULT_POPULATION_SIZE;
int resource_quantity = DEFAULT_RESOURCE_QUANTITY;
int process_quantity = DEFAULT_PROCESS_QUANTITY;
int retry_times = RETRY_TIMES;
int penalty = DEFAULT_PENALTY;


/*Variáveis que controlam os textos exibidos. São inicializadas com os valores padrão
  Esta forma de inicialização foi escolhida para que, se necessário, possam ser feitas 
  alterações futuras no programa, permitindo alterar o valor dos textos. Um possível
  motivo é a internacionalização
  */
string  MUTATION_TAX                 = TEXT_MUTATION_TAX;
string  GENERATIONS_NUMBER           = TEXT_GENERATIONS_NUMBER;
string  CROSSOVER_TAX                = TEXT_CROSSOVER_TAX;
string  POPULATION_SIZE              = TEXT_POPULATION_SIZE;
string  RESOURCE_QUANTITY            = TEXT_RESOURCE_QUANTITY;
string  RESOURCE_VECTOR              = TEXT_RESOURCE_VECTOR;
string  PROCESS_VECTOR               = TEXT_PROCESS_VECTOR;
string  PROCESS_QUANTITY             = TEXT_PROCESS_QUANTITY;
string  RESOURCE                     = TEXT_RESOURCE;
string  PROCESS                      = TEXT_PROCESS;
string  NAME                         = TEXT_NAME; 
string  RESOURCE_TIME_BY_PROCESS     = TEXT_RESOURCE_TIME_BY_PROCESS;
string  UTILIZATION_ORDER            = TEXT_UTILIZATION_ORDER; 
string  UTILIZATION_ORDER_BY_PROCESS = TEXT_UTILIZATION_ORDER_BY_PROCESS;
string  DEADLINE                     = TEXT_DEADLINE;
string  TIME                         = TEXT_TIME;

double *resources;
char   *resource_names[50];
double **process_time;
char   *process_names[50];
int    **utilization_order;
int    **ordered_resources;
double *evaluation;
double *deadline;
int *parents[2];
int *piores;
int results[VALIDATION_QUANTITY];

int ***populacao;
int ***populacaoNova;
int selected;

ifstream input(DEFAULT_INPUT_FILE_NAME);

bool readInputFile();
void paramSet(string line);


//Método de inicialização da população
int*** PopulacaoInicial();  
//Método para avaliar os indivíduos da população
double*   Evaluate();
//Varifica a validade do individuo da população
bool Validate(int** individuo, int results[VALIDATION_QUANTITY]);
//Verifica o tempo da etapa (ou estágio)
double tempo_etapa(int etapa,int*** binary);
//Realiza a seleção dos indivíduos para crossover
int Selection(int** selecionados,int lin, int col);
//Método da roleta. Chamado dentro da seleção. Desta forma, facilita a inclusão de outras modalidades
int Roleta(int** selecionados,int lin, int col);
//Retorna os piores indivídos (pontuação mais alta) de acordo com a quantidade de cruzamentos
int * Piores();
//Insere um valor na posição "index" de um vetor
void insere(int *vector,int index, int value);
//Realiza o crossover entre dois individuos
int*** CrossOver(int*** populacao,int **parents, int* piores);
//Executa o operador de mutação
int ** Mutacao(int ** individuo);
//Retorna o melhor indivíduo da população
int  Melhor();
//Compara duas matrizes. Retorna True se forem iguais, False caso contrário
bool Equals(int** matriz1, int** matriz2,int lin, int col);
//Retorna uma cópia de uma matriz de tamanho x por y
int** MatrixCopy(int ** matriz, int x, int y);

//Classe que representa um indivíduo. É utilizada para conter o melhor indiícuo (Solução parcial)
class Individuo{
       private:
        int **individuo;              //Matriz que representa o indivíduo
        double pontuacao;             //Pontuação do indivíduo
        int number;                   //Número do indivíduo
       public: 
         Individuo();
         void setNumber(int numer);
         int getNumber();
         void change(int **_individuo,int _number,double _pontuacao);         
         double getPontuacao();
};
//Construtor da classe
Individuo::Individuo(){
   individuo   = NULL;
   pontuacao   = -1;   
}
//Troca o indivíduo
void Individuo::change(int **_individuo,int _number,double _pontuacao){
   individuo = MatrixCopy(_individuo,process_quantity /*10*/,(process_quantity+resource_quantity-1));
   pontuacao = _pontuacao;
   number = _number;
}
//Retorna a pontuação do indivíduo armazenado
double Individuo::getPontuacao(){
   return pontuacao;
}
//Altera o número do indivíduo (posição na população)
void Individuo::setNumber(int newNumber){
     number = newNumber;
}
//Retorna o número do indivíduo (posição na população)
int Individuo::getNumber(){
    return number;
}

//Guarda a posição do melhor indivíduo da geração anterior
int melhor_antigo = -1;    




int main(void){

    GARandomSeed(time(NULL));
    
    //Lê o arquivo de inicialização, iniciando as variáveis    
    if (!readInputFile()){
       cout<<"Erro ao abir arquivo";
    }    
    
    Individuo melhor;
    
    /*-----------EXIBE OS DADOS INICIAIS DA EXECUÇÃO DO ALGORITMO-------------*/
    cout<<TEXT_MUTATION_TAX<<mutation_tax<<"\n";
    cout<<TEXT_GENERATIONS_NUMBER<<generations_number<<"\n";
    cout<<TEXT_CROSSOVER_TAX<<crossover_tax<<"\n";
    cout<<TEXT_POPULATION_SIZE<<population_size<<"\n";        
    cout<<TEXT_RESOURCE_QUANTITY<<resource_quantity<<"\n";            
    cout<<TEXT_PROCESS_QUANTITY<<process_quantity<<"\n";
    cout<<"\n"<<RESOURCE_VECTOR<<"\n";            
    cout<<"-------------------------------------------------------------\n";
    cout<<"ID \tQtde.\t"<<NAME<<"\n\n";
    for(int i=0;i<resource_quantity;i++){
            cout<<"r"<<i<<"\t";
            cout<<resources[i]<<"\t";
            for(int y=0;(y<50)&&(resource_names[y][i]!='\0');y++){
                    cout<<resource_names[y][i];
            }
            cout<<"\n";
    }
    cout<<"\n";
    cout<<"\n"<<PROCESS_VECTOR<<"\n";
    cout<<"-------------------------------------------------------------\n";
    cout<<"ID \t"<<NAME<<"\n\n";
    for(int i=0;i<process_quantity;i++){
        cout<<"p"<<i<<"\t";
        for(int y=0;(y<50)&&(process_names[y][i]!='\0');y++){
              cout<<process_names[y][i];
        } 
        cout<<"\n";
    }
    cout<<"\n\n"<<DEADLINE<<"\n";
    cout<<"-------------------------------------------------------------\n";
    cout<<"ID\t"<<TIME<<"\n\n";
    for(int i=0;i<process_quantity;i++){
        cout<<"p"<<i<<"\t";
        cout<<deadline[i]<<"\t";

        cout<<"\n";
    }
    cout<<"\n\n"<<RESOURCE_TIME_BY_PROCESS<<"\n";
    cout<<"-------------------------------------------------------------\n\t";
    for(int y=0;y<resource_quantity;y++){
            cout<<"r"<<y<<"\t";
    }cout<<"\n\n";    
    for(int i=0;i<process_quantity;i++){
        cout<<"p"<<i<<"\t";
        for(int y=0;y<resource_quantity;y++){
                cout<<process_time[i][y]<<"\t";
        }             
        cout<<"\n";
    }            
    cout<<"\n";
    cout<<"\n"<<UTILIZATION_ORDER_BY_PROCESS<<"\n";
    cout<<"-------------------------------------------------------------\n\t";
    for(int y=0;y<resource_quantity;y++){
            cout<<"r"<<y<<"\t";
    }cout<<"\n\n";
    
    for(int i=0;i<process_quantity;i++){
        cout<<"p"<<i<<"\t";
        for(int y=0;y<resource_quantity;y++){
                cout<<utilization_order[i][y]<<"\t";
        }             
        cout<<"\n";
    }
    cout<<"\n\n";

    for(int x=0;x<process_quantity;x++){     
       for(int c=0;(c<50)&&(process_names[c][x]!='\0');c++){
                         cout<<process_names[c][x];
                }cout<<":\t";         
        for(int y=0;y<resource_quantity;y++){                
                if(ordered_resources[x][y]!=-1){
                      if(ordered_resources[x][y]>0){
                          cout<<" -> ";                          
                      }                          
                      for(int c=0;(c<50)&&(resource_names[c][ordered_resources[x][y]]!='\0');c++){
                         cout<<resource_names[c][ordered_resources[x][y]];
                      }
                }
        }             
        cout<<"\n";
    }
    //Pausa a execução, aguardando a confirmação para continuar
    system("pause");
    
    int m;
    int resource;
    int randon;


    /****************************************************************************
          A PARTIR DESTE PONTO, SE INICIA A EXECUÇÃO DO ALGORITMO GENÉTICO                                                                   
     ***************************************************************************/
            
    //Inicializa a população
    populacao  = PopulacaoInicial();
    //Avalia os indivíduos da população (calcula o fitness)
    evaluation = Evaluate();
    
    //Exibe a solução inicial (Melhor individuo da população inicial)
    cout<<"-------------------------------------------------------------\n";
    cout<<"SOLUCAO INICIAL: \n\n";
    for(int x=0;x<process_quantity;x++){
            for(int y=0;y<process_quantity+resource_quantity-1;y++){
                    cout<<populacao[Melhor()][x][y]+1<<"\t";
            }
            cout<<"\n";
    }
    cout<<"\n\n";
    
    //Exibe o número do indivíduo (Posição na população)
    cout<<"individuo "<<melhor.getNumber()<<"\tvalor: "<<evaluation[Melhor()]<<"\n";
    //Indica se o individuo representa ou não uma solução válida
    if(Validate(populacao[Melhor()],results)){
       cout<<"Valido: Sim\n";
    }else{
       cout<<"Valido: Nao\n";          
    }
    //Exibe a geração em que a solução foi encontrada (no caso, 0)
    cout<<"Geracao: 0\n\n";    
   
    int ** aux = populacao[Melhor()];  
    double pontuacao = evaluation[Melhor()];
    
    //Guarda o melhor individuo
    melhor.change(aux,Melhor(),pontuacao);
    
    
    /*-------Inicia as gerações--------*/    
    int geracoes = 0;    
    do{
    //Avalia os individuos da população (calcula o fitness)
    evaluation = Evaluate();
    
    //Identifica os piores individuos
    piores = Piores();
    
    //Aloca memoria para os pais, usados no cruzamento
    parents[0] = (int*)malloc(sizeof(int)*(crossover_tax));
    parents[1] = (int*)malloc(sizeof(int)*(crossover_tax));
  
    int repete=false;
    int c = 0;
    
    //Realiza a seleção, para selecionar os pais usados no crossover
    for(int i=0;i<crossover_tax;i++){
       
       //Seleciona o primeiro pai (para cada crossover a ser realizado)
       c=0;
       do{           
          repete = false;   
          parents[0][i]   = Selection(parents,0,i);
        
           c++;
           if(c>1000)

          for(int j=0;j<i;j++){
               if((parents[0][j]==parents[0][i])||(parents[1][j]==parents[0][i])){
                  repete = true;
               }
          }
       }while(repete);
       c=0;
       //Seleciona o segundo pai (para cada crossover a ser realizado)
       do{

          repete=false;
          parents[1][i]   = Selection(parents,1,i);
          if(parents[1][i]==parents[0][i]){
             parents[1][i]   = Selection(parents,1,i);                                           
          }
         
           c++;
           if(c>100)

          for(int j=0;j<i;j++){
             if((parents[0][j]==parents[1][i])||(parents[1][j]==parents[1][i])||(parents[1][i]==parents[0][i])){
                  repete = true;           
             }
          }
       }while(repete);
       
    }
    
//Exibe os resltados das gerações. Para a geração 0 (inicialização), não realiza crossover    
if((geracoes == generations_number-1)||((melhor.getPontuacao()==-1)||(evaluation[Melhor()]<melhor.getPontuacao()))){    
    
    if((melhor.getPontuacao()==-1)||(evaluation[Melhor()]<melhor.getPontuacao())){
           int ** aux = populacao[Melhor()];  
           double pontuacao = evaluation[Melhor()];
           melhor.change(aux,Melhor(),pontuacao);
    }
    
    cout<<"\n\n";
    cout<<"-------------------------------------------------------------\n";
    
    if(geracoes == generations_number-1)
       cout<<"SOLUCAO FINAL: \n\n";    
    if(geracoes < generations_number-1)
          cout<<"SOLUCAO PARCIAL: \n\n";     
    
    cout<<"\n\n";
    for(int x=0;x<process_quantity;x++){
            for(int y=0;y<process_quantity+resource_quantity-1;y++){
                    cout<<populacao[Melhor()][x][y]+1<<"\t";
            }
            cout<<"\n";
    }
    cout<<"\n\n";
        
    cout<<"individuo "<<melhor.getNumber()<<"\tvalor: "<<evaluation[Melhor()]<<"\n";
    if(Validate(populacao[Melhor()],results)){
       cout<<"Valido: Sim\n";
    }else{
       cout<<"Valido: Nao\n";          
    }
    cout<<"Geracao: "<<geracoes+1<<"\n\n";
    if(melhor.getPontuacao() != evaluation[Melhor()]){
                     melhor.change(populacao[Melhor()],Melhor(),evaluation[Melhor()]);
    }
}

    //Realiza o crossover entre os pais selecionados
    populacao = CrossOver(populacao,parents,piores);
    

    
    //incrementa o contador de gerações
    geracoes++;
    }while(geracoes < (generations_number));
    system("pause");
    system("cls");
}


/* MÉTODO DE LEITURA DO ARQUIVO DE INICIALIZAÇÃO
*/
bool readInputFile(){
   char line[50];
   
   if(input.eof()){
                 return false;
   }else{
      while(! input.eof()){
           input.getline(line,50);
            paramSet(line);
           
        } 
        
   }           
   input.close();    
   return true; 
  
}

/* SETA OS VALORES DAS VARIÁVEIS
*/
void paramSet(string line){    
    string text,textAux;
    char *aux, aux2[5];
    int i;

     //Taxa de mutação
     if(line.size() > MUTATION_TAX.size()){
        if(line.compare(0,MUTATION_TAX.size(),MUTATION_TAX)==0){
           mutation_tax = atoi(line.substr(MUTATION_TAX.size(),line.size()).data());
        }
     }
     //Numero de gerações
     if(line.size() > GENERATIONS_NUMBER.size()){
        if(line.compare(0,GENERATIONS_NUMBER.size(),GENERATIONS_NUMBER)==0){
           generations_number = atoi(line.substr(GENERATIONS_NUMBER.size(),line.size()).data());
        }
     }
     //Taxa de crossover
     if(line.size() > CROSSOVER_TAX.size()){
        if(line.compare(0,CROSSOVER_TAX.size(),CROSSOVER_TAX)==0){
           crossover_tax = atoi(line.substr(CROSSOVER_TAX.size(),line.size()).data());
        }
     }
     //Tamanho da população
     if(line.size() > POPULATION_SIZE.size()){
        if(line.compare(0,POPULATION_SIZE.size(),POPULATION_SIZE)==0){
           population_size = atoi(line.substr(POPULATION_SIZE.size(),line.size()).data());
        }
     }
     //Quantidade de recursos
     if(line.size() > RESOURCE_QUANTITY.size()){
        if(line.compare(0,RESOURCE_QUANTITY.size(),RESOURCE_QUANTITY)==0){
           resource_quantity = atoi(line.substr(RESOURCE_QUANTITY.size(),line.size()).data());                      
        }
     }
     //Quantidade de processos
     if(line.size() > PROCESS_QUANTITY.size()){
        if(line.compare(0,PROCESS_QUANTITY.size(),PROCESS_QUANTITY)==0){
           process_quantity = atoi(line.substr(PROCESS_QUANTITY.size(),line.size()).data());                      
        }
     }
     //Vetor de recursos
     if(line.size() == RESOURCE_VECTOR.size()){
        if(line.compare(0,RESOURCE_VECTOR.size(),RESOURCE_VECTOR)==0){
          resources = (double*)malloc(sizeof(double)*resource_quantity);
          
          for(int x=0;x<50;x++){
             resource_names[x] = (char*)malloc(sizeof(char*)*resource_quantity);             
             for(int y=0;y<resource_quantity;y++){
                resource_names[x][y] = ' ';
             }
          }                   
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
          do{
             input >> text;
             resources[i] = atof(text.c_str()); 
             i++;  
          }while(((text!="}"))&&(i<resource_quantity)); 
                    
          for(int j=0; j<resource_quantity;j++){             
                  
             aux= (char*)malloc(sizeof(char)*10);                 
             itoa(j,aux,10);

             textAux = RESOURCE+aux;
             
             for(int y=0;y<textAux.size();y++){     
                  resource_names[y][j] = textAux[y];
             }                                        
          }
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
          while(((text!="}"))&&(i<resource_quantity)){
            do{
                input.getline(aux2,50);
             }while((aux2 == "\0")||(aux2 == "\n")||(aux2 == "")||(aux2[0] == '\0'));
             text = aux2;
             for(int y=0;y<text.size();y++){     
                  resource_names[y][i] = text[y];
                  resource_names[y+1][i] = '\0';
             }
             i++;  
          } 
          
          
           
        }
     }
     //Matriz de processos
     if(line.size() == PROCESS_VECTOR.size()){
        if(line.compare(0,PROCESS_VECTOR.size(),PROCESS_VECTOR)==0){
          
          process_time = (double**)malloc(sizeof(double)*process_quantity);
          for(int p=0;p<process_quantity;p++){
             process_time[p] = (double*)malloc(sizeof(double)*resource_quantity);        
          }
          for(int x=0;x<50;x++){
             process_names[x] = (char*)malloc(sizeof(char*)*process_quantity);             
             for(int y=0;y<process_quantity;y++){
                process_names[x][y] = ' ';
             }
          }                           
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
          do{
             for(int j=0;j<resource_quantity;j++){ 
                input >> text;
                process_time[i][j] = atof(text.c_str()); 
             }
             i++;  
          }while(((text!="}"))&&(i<process_quantity)); 
          
          
          for(int j=0; j<process_quantity;j++){             
                  
             aux= (char*)malloc(sizeof(char)*10);                 
             itoa(j,aux,10);

             textAux = PROCESS+aux;
             
             for(int y=0;y<textAux.size();y++){     
                  process_names[y][j] = textAux[y];
             } 
          }    
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
               
          while(((text!="}"))&&(i<process_quantity)){
             do{
                input.getline(aux2,50);
             }while((aux2 == "\0")||(aux2 == "\n")||(aux2 == "")||(aux2[0] == '\0'));
             text = aux2;
             for(int y=0;y<text.size();y++){     
                  process_names[y][i] = aux2[y];
                  process_names[y+1][i] = '\0';
             }
             i++;  
          }                                
        }
     }
     
     
     
     //Vetor de deadline (prazo final dos processos)
     if(line.size() == DEADLINE.size()){
        if(line.compare(0,DEADLINE.size(),DEADLINE)==0){

          deadline = (double*)malloc(sizeof(double)*process_quantity);
          
          for(int x=0;x<process_quantity;x++){
             deadline[x] = -1;
          }
                        
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
          do{            
                input >> text;
                deadline[i] = atof(text.c_str());              
             i++;  
          }while(((text!="}"))&&(i<process_quantity));                              
        }
     }
     
     
     //Ordem de utilização dos recursos
     if(line.size() == UTILIZATION_ORDER.size()){
        if(line.compare(0,UTILIZATION_ORDER.size(),UTILIZATION_ORDER)==0){

          utilization_order = (int**)malloc(sizeof(int)*process_quantity);
          ordered_resources = (int**)malloc(sizeof(int)*process_quantity);
          for(int p=0;p<process_quantity;p++){
             utilization_order[p] = (int*)malloc(sizeof(int)*resource_quantity);        
             ordered_resources[p] = (int*)malloc(sizeof(int)*resource_quantity);        
             for(int r=0;r<resource_quantity;r++){
                  ordered_resources[p][r] = -1;   
             }
          }         
          
          
          do{
            input >> text; 
          }while((text!="{")); 
          
          i = 0; 
          do{
             for(int j=0;j<resource_quantity;j++){ 
                input >> text;
                utilization_order[i][j] = atoi(text.c_str()); 
                if(atoi(text.c_str())>0){
                   ordered_resources[i][atoi(text.c_str())-1]=j;
                }
             }
             i++;  
          }while(((text!="}"))&&(i<process_quantity));                                                           
        }
     }
     free(aux);
      
}


/*INICIALIZA A POPULAÇÃO, EXECUTANDO UMA HEURISTICA GULOSA
* retorna um vetor tridimensional contendo a população (um vetor de individuos,
* onde cada individuo é uma matriz)
*/
int*** PopulacaoInicial(){
    int m;
    int resource;
    int randon;

    int lin = process_quantity; 
    int col = (process_quantity + resource_quantity - 1);
    
            
    /* Um arranjo tridimentional representa a população:
       A primeira dimensão indica o indivíduo da população
       As outras duas correspondem a uma possível solução para o problema
       
       na prática, "populacao" é uma lista de 'n' soluções, onde 'n'
       é o tamanho parametrizado da pululação, e cada solução é uma
       matriz de tamanho igual a:
              
              Qtde. Processos X (Qt. Processos + Qt. Recursos) - 1
              
       que é a quantidade de estados utilizados no pior caso, correspondendo
       a um 'pipeline' em que não há simultaneidade de utilização de um mesmo
       recurso.
    */
    int ***populacao = (int***)malloc(sizeof(int)*population_size);    
    for(int i=0;i<population_size;i++){
            populacao[i]=(int**)malloc(sizeof(int)*lin);
            for(int j=0;j<lin;j++){
                    populacao[i][j]=(int*)malloc(sizeof(int)*col);
            }
    } 
    //Para cada Indivíduo da população
    for(int i = 0;i<population_size;i++){
       
       //Para cada processo
       int count=0;
       do{
       for(int j=0;j<lin;j++){
           
           m=0;
           resource = (ordered_resources[j][0]+1);
           //randon = (rand()%2);                    
           randon = GARandomInt(0,1);
           populacao[i][j][0] = (resource*randon)-1; 
           if(populacao[i][j][0]!= -1){
                      m=1;
           }
           
           for(int k=1;k<col;k++){

                   if(m>(resource_quantity-1))
                      resource = 0;
                  else
                      resource = (ordered_resources[j][m]+1);
                   
                   //randon = (rand()%2);         
                   randon = GARandomInt(0,1);
                   populacao[i][j][k] = (resource*randon)-1;                
                   if(populacao[i][j][k]!= -1){
                      m++;
                   }
           }               
       }     
       count++;
       }while(!Validate(populacao[i],results)&&(count<retry_times));  
    }

    return populacao;
    

}

/* AVALIA A POPULAÇÃO (CALCULA O FITNESS DE TODOS OS INDIVIDUOS)
*  Retorna um vetor contendo o valor de todos os individuos
*/
double*   Evaluate(){
    double* Result = (double*)malloc(sizeof(double)*population_size);
    double maior = 0;    
    double Parcial_Result;

    
    for(int i=0;i<population_size;i++){  
            Parcial_Result = 0;           
            for(int j=0;j<process_quantity+resource_quantity-1;j++){
                    maior = 0;
                    for(int k=0;k<process_quantity;k++){
                          if (populacao[i][k][j] != -1){  
                             if((process_time[k][(populacao[i][k][j])])>maior){
                                maior = (process_time[k][(populacao[i][k][j])]);
                             }
                          }
                    }
                    Parcial_Result += maior;
            }
            
            //Penalisa individuos invalidos
            if(!Validate(populacao[i],results)){
                for(int r=0;r<VALIDATION_QUANTITY;r++){
                        if(results[r]>0){
                           Parcial_Result += (results[r]*penalty);
                        }
                }                
            }
            //Se a pontuação for zero, significa que não alocou nenhum recurso
            if (Parcial_Result==0){
               Parcial_Result = penalty*resource_quantity;                    
            }
            //Penaliza indivíduos identicos para evitar que cruzem entre si
            for(int r=0;r<i;r++){
               if(Equals(populacao[r],populacao[i],process_quantity,(process_quantity+resource_quantity-1))){
                   Parcial_Result = Parcial_Result * 2;                                  
               }        
            }  
              
            
            //Desempate: Soma 1 para desempatar
            bool repete = false;
            do{
            for(int m=0;m<i;m++){
                    if(Result[m]==Parcial_Result){
                       Parcial_Result++;
                       repete = true;
                       break;
                    }else{
                       repete = false;
                    }
            }            
            }while(repete);  
            Result[i] = Parcial_Result;
                        
                
    }  
    
    return Result;
}

/* VERIFICA SE UM INDIVÍDUO É VALID
*  retorna true se o indivíduo for válido, e false caso contrário
*/
bool Validate(int** individuo, int results[VALIDATION_QUANTITY]){
    int *** binary   =  (int***)malloc(sizeof(int)*process_quantity);    
    int soma=0;
    double Dsoma = 0;
    int aux;
    int m;
    int** S;
    bool valid = false;
    bool validated = false;       
         
    
    //Converte em matriz de binários
    for(int i=0;i<process_quantity;i++){
            binary[i]=(int**)malloc(sizeof(int)*resource_quantity);
            for(int j=0;j<resource_quantity;j++){
                    binary[i][j]=(int*)malloc(sizeof(int)*(process_quantity+resource_quantity-1));
                    for(int k=0;k<(process_quantity+resource_quantity-1);k++){
                            binary[i][j][k] = 0;
                    }
            }
    }

    for(int i=0;i<process_quantity;i++){
       for(int j=0;j<(process_quantity+resource_quantity-1);j++){
               if(individuo[i][j]!= -1){
                                    int r=0;
                     aux=individuo[i][j];  
                     if((individuo[i][j] <0)||(individuo[i][j]>5)){
                     r=0;          
                     }           
                     binary[i][aux][j]=1;
               }
       }
    }
    
    /*Executa as validações
     *---------------------
     */
    
    //i) Em uma mesma etapa, um mesmo recurso não pode ser alocado para mais de um processo, 
    //   a menos que haja quantidade suficiente do recurso (Qj).
    results[0]=0;
    for(int k=0;(k<process_quantity+resource_quantity-1)&&(!validated);k++){
                        
          for(int j=0;(j<resource_quantity)&&(!validated);j++){
                    soma=0;
                    for(int i=0;(i<process_quantity)&&(!validated);i++){
                        soma += binary[i][j][k]; 
                    }     
                    if(soma>resources[j]){
                       //validated = true;
                       valid = false;
                       results[0]++;
                    }else{
                       soma=0;   
                       valid = true;                       
                    }
          }
    }

    //ii) Em uma mesma etapa, o mesmo processo não pode alocar mais de um recurso simultaneamente.
    validated = false;
    results[1]=0;
    for(int k=0;(k<(resource_quantity+process_quantity-1))&&(!validated);k++){
        for(int i=0;(i<process_quantity)&&(!validated);i++){
           soma=0;
           for(int j=0;(j<resource_quantity)&&(!validated);j++){
             soma += binary[i][j][k];       
             if(soma>1){
                 valid = false;
                 //validated = true;
                 results[1]++;
             }else{
                 valid = true;                   
             }
           }        
        }    
    }
    
    //iii) Um mesmo processo não pode ficar com o mesmo recurso alocado em mais de uma etapa.
    validated = false;
    results[2] = 0;
    for(int i=0;(i<process_quantity)&&(!validated);i++){
       for(int j=0;(j<resource_quantity)&&(!validated);j++){
             soma=0;
             for(int k=0;(k<(resource_quantity+process_quantity-1))&&(!validated);k++){
                soma += binary[i][j][k];       
                if(soma>1){
                   valid = false;
                   //validated = true;
                   results[2]++;
                }else{
                   valid = true;                   
                }
             }
       }        
    }
    
    //iv) Um recurso alocado na primeira etapa somente pode ter ordem 1 ou 0 (quando o recurso não foi alocado).
    validated = false;
    results[3] = 0;
    for(int i=0;(i<process_quantity)&&(!validated);i++){
       for(int j=0;(j<resource_quantity)&&(!validated);j++){
                soma = binary[i][j][0]*(utilization_order[i][j]-1);       
                if(soma!=0){
                   valid = false;
                   //validated = true;
                   results[3]++;
                }else{
                   valid = true;                   
                }    
       }
    }
    
    //v) Nas etapas seguintes à primeira, um recurso somente pode ser alocado se todos os 
    //   recursos de ordem inferior tiverem sido alocados nas etapas anteriores.
    validated = false;
    results[4] = 0;
    for(int i=0;(i<process_quantity)&&(!validated);i++){
        for(int k=0;(k<(resource_quantity+process_quantity-1))&&(!validated);k++){
             soma=0; 
             aux = 0;   
             for(int j=0;(j<resource_quantity)&&(!validated);j++){
                     soma += binary[i][j][k];
             }
             aux = soma;
             soma = 0;
             for(int n=0;(n<(k));n++){
                     for(int j=0;(j<resource_quantity)&&(!validated);j++){
                             soma += (binary[i][j][n]*utilization_order[i][j]);
                     }
             }
             soma *= aux;
             
             m=0;
             aux=0;
             for(int j=0;(j<resource_quantity)&&(!validated);j++){
                 aux += (binary[i][j][k]*utilization_order[i][j]);
             }
             m=0;
             for(int x=0;x < aux;x++){
                     m +=x;
             }
             
             if(soma != m){
                  valid = false;
                  //validated = true;
                  results[4]++;
             }
             else{
                  valid = true;
             }
        }    
    }
            
    //vi) A soma de todas as variáveis de decisão de um processo, multiplicadas pela ordem, 
    //    deve ser igual à soma de todas as ordens. Esta restrição garante que todos os 
    //    recursos sejam utilizados por um processo durante o planejamento.
    validated = false;
    results[5] = 0;
    for(int i=0;(i<process_quantity)&&(!validated);i++){
       soma=0;
       for(int k=0;k<(process_quantity+resource_quantity-1)&&(!validated);k++){
               for(int j=0;(j<resource_quantity)&&(!validated);j++){
                       soma += (binary[i][j][k]*utilization_order[i][j]);
               }
       }
       aux=0;
       for(int j=0;(j<resource_quantity)&&(!validated);j++){
               aux+=utilization_order[i][j];
       }
       if(soma != aux){
               valid = false;
               //validated = true;
               results[5] += abs(aux-soma);
       }else{
             valid = true;
       }
       
    }
    
   // vii) A variável auxiliar Sk,i indica, quando zero, que nenhum recurso mais é alocado 
   //      na etapa n para o processo i, e nem nas etapas seguintes (quando Sn,i = 0, indica 
   //      que todo o processo i foi finalizado na etapa n-1). Esta restrição informa que o 
   //      valor de Sk,i quando k = p x r (última etapa) deve a soma de todos os dk,i,j 
   //      (como apenas um recurso é alocado para um processo a cada etapa, Sk,n será igual a 0 ou 1).
   validated = false;
   
   S = (int**)malloc(sizeof(int)*process_quantity);
   for(int i=0;i<process_quantity;i++){
         S[i] = (int*)malloc(sizeof(int)*(resource_quantity+process_quantity-1));
   }
   
   int pos=(process_quantity+resource_quantity-2);
   for(int i=0;(i<process_quantity)&&(!validated);i++){
       soma = 0;
       for(int j=0;(j<resource_quantity)&&(!validated);j++){
               soma += binary[i][j][pos];
       }       
       S[i][pos] = soma;
   }
   
   //viii) Esta restrição informa que para todas as etapas antes da última, Sk,i deve ser o maior 
   //      valor entre a soma dos dk,i,j e o Sk+1,i (Sk,i da etapa seguinte). Dessa forma, se uma 
   //      etapa possui Sk,i = 1, garante-se que todas as anteriores (mesmo que não haja nenhum 
   //      recurso alocado para o processo i na etapa) também será 1.
   validated = false;
   for(int i=0;(i<process_quantity)&&(!validated);i++){
       for(int k=(pos-1);k>-1&&(!validated);k--){    
          soma = 0;
          for(int j=0;(j<resource_quantity)&&(!validated);j++){
                  soma += binary[i][j][k];
          }       
          S[i][k] = max(soma,S[i][k+1]);
       }
   } 
   
   //ix) Como a variável Sk,i possui valor 0 apenas depois da finalização do processo i, pode-se 
   //    restringir o tempo máximo de cada processo através desta restrição, pois Sk,i multiplicada 
   //    pelo tempo total de cada etapa será zero em todas as etapas posteriores à finalização. A soma
   //    de todos os tempos de etapa multiplicados pelo Sk,i da etapa fornecerá o tempo total de produção 
   //    do processo i, o qual deve ser menor ou igual ao tempo limite Li.
   validated = false;
   results[6] = 0;
   for(int i=0;(i<process_quantity)&&(!validated);i++){
           Dsoma = 0;
           for(int k=0;(k<(resource_quantity+process_quantity-1))&&(!validated);k++){
                   Dsoma += S[i][k]*tempo_etapa(k,binary);
           }
           if(Dsoma > deadline[i]){
                   valid = false;
                   //validated = true;
                   results[6] = (int)(Dsoma-deadline[i]);
           }else{
                 valid = true;
           }
           
   }
       
   for(int i=0;i<VALIDATION_QUANTITY;i++){
            if(results[i]>0){
               return false;
            }
    }
    free(binary);
    free(S);
    return true;
        
}

//Retorna o tempo de uma etapa
double tempo_etapa(int etapa,int*** binary){
   double maior = 0;    
       
   for(int i = 0;i<process_quantity;i++){
           for(int j=0;j<resource_quantity;j++){
                maior = max(maior,(binary[i][j][etapa]*process_time[i][j]));   
           }
   }       
   return maior;
}

/* REALIZA A ETAPA DE SELEÇÃO
*  Retorna um individuo da população. O parâmetro "selecionados"
*  contém todos os individuos já selecionados na geração, para
*  impedir que um individuo repetido seja selecionado na mesma geração
*/
int Selection(int** selecionados,int lin, int col){

   //Chama a roleta, como método de seleção
   return Roleta(selecionados, lin,  col);           
}

/* EXECUTA A ROLETA PARA SELECIONAR UM INDIVÍDUOS
*  Retorna um individuo da população através do método da roleta. O parâmetro 
*  "selecionados" contém todos os individuos já selecionados na geração, para
*  impedir que um individuo repetido seja selecionado na mesma geração
*/
int Roleta(int** selecionados,int lin, int col){
    double soma = 0;
    int S,s1;
    bool boSoma;
    int count = 0;
    
    for(int i=0;i<population_size;i++){
           boSoma = true;
            for(int x=0;(x<(col+1))&&(boSoma);x++){
               for(int y=0;(y<(2))&&(boSoma);y++){
                       if(selecionados[y][x]==i){
                          boSoma = false;
                       }
               }
            }
            if(boSoma){
               soma += (evaluation[piores[0]]-evaluation[i]);    
            }

    }
    s1 = GARandomInt(0,(int)soma);
    
    soma=0;
    for(int i=0;i<population_size;i++){
              
             boSoma = true;
             for(int x=0;(x<(col+1))&&(boSoma);x++){
               for(int y=0;(y<(2))&&(boSoma);y++){
                       if(selecionados[y][x]==i){
                          boSoma = false;
                       }
               }
             }
             if(boSoma) {
              soma += (evaluation[piores[0]]-evaluation[i]);
              }
              if(soma > (S-1)){
                  
                   if(boSoma){
                     return i;
                   }

              }      
    }    
}   


/* RETORNA OS n * 2 PIORES INDIVIDUOS DA POPULAÇÃO
*  onde n é a quantiodade de crossover por geração
*/
int * Piores(){
   int menor = 0;   
   

   int * selected = (int*)malloc(sizeof(int)*(crossover_tax*2));

         
   for(int i=0;i<(crossover_tax*2);i++){
               selected[i]  = -1;
   } 
  
   for(int i=0;i<population_size;i++){
       for(int j=0;j<(crossover_tax*2);j++){
               if((evaluation[i]>evaluation[selected[j]])||(evaluation[i]==evaluation[selected[j]])||(selected[j]==-1)){
                       insere(selected,j,i);                     
                       break;
               }
       }
   }  
   return selected;       
}

/* RETORNA O MELHOR INDIVÍDUO DA POPULAÇÃO
*/
int Melhor(){
    int menor = -1;     
    int i;
    for( i=0;i<population_size;i++){
          if(menor != -1){
           if((evaluation[i]<evaluation[menor])and(Validate(populacao[i],results))){
              menor = i;
           }    
          }else{
              if(Validate(populacao[i],results)){
                 menor = i;
              }  
          }
   }
   if(menor<0) menor = 0;
   return menor;
}

/* INSEREN UM VALOR NA POSIÇÃO index DO VETOR vector
*/
void insere(int* vector,int index, int value){
     for(int i=(crossover_tax*2)-1;i>index;i--){
             vector[i] = vector[i-1];
     }
     vector[index]=value;
}

/* REALIZA O CROSSOVER
*  populac: vetor contendo a população
*  parents: vetor contendo os pais a serem utilizados para crossover
*  piores: vetor contendo os piores individuos, substituidos durante o crossover
*/
int*** CrossOver(int*** populac,int **parents, int* piores){
       int S;
       int pos2=0;
       int** individuo,**individuo2, **prog1,**prog2;

       int*** populacao_nova = (int***)malloc(sizeof(int)*population_size);
       for(int k = 0;k<population_size;k++){
               populacao_nova[k] = (int**)malloc(sizeof(int)*process_quantity);
               for(int i=0;i<process_quantity;i++){
                      populacao_nova[k][i] = (int*)malloc(sizeof(int)*(process_quantity+resource_quantity-1));
                      for(int j=0;j<(process_quantity+resource_quantity-1);j++){
                              populacao_nova[k][i][j] = populacao[k][i][j];
                      }
               }
       }
              
       individuo  = (int**)malloc(sizeof(int)*process_quantity);
       individuo2 = (int**)malloc(sizeof(int)*process_quantity);
       for(int i=0;i<process_quantity;i++){
               individuo [i] = (int*)malloc(sizeof(int)*(process_quantity + resource_quantity - 1));
               individuo2[i] = (int*)malloc(sizeof(int)*(process_quantity + resource_quantity - 1));
       }      
       

       
       for(int i=0;i<(crossover_tax);i++){
            
              //Sorteia o tipo de crossover               
              S = GARandomInt(0,2);
               
               switch(S){
               case 0://CrossOver vertical

                    for(int k=0;k<(process_quantity + resource_quantity - 1);k=k+2){
                       for(int l= 0;l<process_quantity;l++){
                            individuo [l][k] = populacao[parents[0][i]][l][k];
                            individuo2[l][k] = populacao[parents[1][i]][l][k];
                       }
                    }
                    for(int k=1;k<(process_quantity + resource_quantity - 1);k=k+2){
                       for(int l= 0;l<process_quantity;l++){
                             individuo[l][k] = populacao[parents[1][i]][l][k];
                            individuo2[l][k] = populacao[parents[0][i]][l][k];                            
                       }
                    }  
                    break;    
               case 1://CrossOver horizontal
                    
                    for(int k=0;k<(process_quantity + resource_quantity - 1);k++){
                       for(int l= 0;l<process_quantity;l=l+2){                               
                            individuo[l][k] = populacao[parents[0][i]][l][k];
                           individuo2[l][k] = populacao[parents[1][i]][l][k];
                       }
                    }                                                  
                    for(int k=0;k<(process_quantity + resource_quantity - 1);k++){
                       for(int l= 1;l<process_quantity;l=l+2){                              
                            individuo[l][k] = populacao[parents[1][i]][l][k];
                           individuo2[l][k] = populacao[parents[0][i]][l][k];
                       }
                    }  
                    break;
               case 2://Crossover de tres pontos (A cada linha, novos pontos são selecionados)\n"; 
                    
                   int point1;
                   int point2;
                   int point3;
             
                   int sort;  
                   //Para cada processo
                   for(int j = 0;j< (process_quantity);j++){
                           //sort = (rand()%2);
                           sort = GARandomInt(0,1);
                           if(sort==0){ 
                                prog1 = populacao[parents[0][i]]; 
                                prog2 = populacao[parents[1][i]]; 
                            }
                            else
                            {
                                prog1 = populacao[parents[1][i]]; 
                                prog2 = populacao[parents[0][i]]; 
                            }
                            //Sorteia os tres pontos de crossOver. Eles variam de uma linha para outra
                            point1 = GARandomInt(0,(process_quantity+resource_quantity-1));
                            point2 = GARandomInt(point1+1,(process_quantity+resource_quantity-1));
                            point3 = GARandomInt(point2+1,(process_quantity+resource_quantity-1));
                            
                            for(int k=0;k<(process_quantity+resource_quantity-1);k++){ //Percorre toda a linha
                                    if((k<point1)||((k>(point2-1))&&(k<point3))){
                                       individuo [j][k]  = prog1[j][k];
                                       individuo2[j][k]  = prog2[j][k];
                                    }else{
                                       individuo [j][k]  = prog2[j][k];
                                       individuo2[j][k]  = prog1[j][k];
                                    }
                            }
                   }
                   
               }
              
               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){
                     populacao_nova[piores[pos2]][z][h] = individuo[z][h];
                  }
               }
               
              

               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){

                  }
               }
               individuo = Mutacao(individuo);        
               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){
                     populacao_nova[piores[pos2]][z][h] = individuo[z][h];
                  }
               }
                      
               pos2++;
               
               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){
                     populacao_nova[piores[pos2]][z][h] = individuo2[z][h];
                  }
               }
               
               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){
                  }
               } 
               individuo2 = Mutacao(individuo2);
               for(int z=0;z<process_quantity;z++){
                  for(int h=0;h<(process_quantity+resource_quantity-1);h++){
                     populacao_nova[piores[pos2]][z][h] = individuo2[z][h];
                  }
               }                                                          
               pos2++;               
                           
       }
      
       free(individuo);
       free(individuo2);
       free(populacao);
       return populacao_nova;
}

/* REALIZA A MUTAÇÃO 
*/
int ** Mutacao(int ** individuo){
    int S = mutation_tax;
    int N = 100-mutation_tax;    
    

    int lin1,lin2,col1,col2,aux,val;
    
    //Sorteia se ocorrerá mutação (de acordo com a taxa de mutação)
    int mut = GARandomInt(0,99);
    if(mut < (S+1)){
           int tipo = GARandomInt(1,2);
           
           switch(tipo){
               case 1://Tipo de Mutação 1: Troca dois genes
                        
                       //Sorteia duas posições

                       lin1 = GARandomInt(0,process_quantity-1);
                       lin2 = GARandomInt(0,process_quantity-1);


                       col1 = GARandomInt(0,(process_quantity+resource_quantity-1)-1);
                       col2 = GARandomInt(0,(process_quantity+resource_quantity-1)-1);
                       
                       aux = individuo[lin1][col1];    
                       individuo[lin1][col1] = individuo[lin2][col2];    
                       individuo[lin2][col2] = aux;

                       break; 

                case 2://Tipo de mutação 2: Altera genes aleatoriamente

                       lin1 = GARandomInt(0,process_quantity-1);
                       col1 = GARandomInt(0,(process_quantity+resource_quantity-1)-1);
                       val = GARandomInt(0,resource_quantity-1);
                       
                       individuo[lin1][col1] = val;    
                       
                       break;                                           
           }
           
           return individuo;                      
    }
    else{
       return individuo;     
    }
}

/* COMPARA DUAS MATRIZES
*  Retorna True se forem iguais, e False caso contrário
*/
bool Equals(int** matriz1, int** matriz2,int lin, int col){
     for(int i=0;i<lin;i++){
             for(int j=0;j<col;j++){
                     if(matriz1[i][j] != matriz2[i][j]){
                                      return false;
                     }
             }
     }
     return true;
}

/* RETORNA UMA CÓPIA DE UMA MATRIZ
*/
int** MatrixCopy(int ** matriz, int x, int y){
      int** nova = (int**)malloc(sizeof(int)*x);
      for(int i=0;i<x;i++){
          nova[i] = (int*)malloc(sizeof(int)*y);    
      }
      
      for(int i=0;i<x;i++){
         for(int j=0;j<y;j++){
            nova[i][j] = matriz[i][j];
         }
      }
      return matriz;
}

