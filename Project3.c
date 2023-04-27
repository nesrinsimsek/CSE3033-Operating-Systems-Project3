#include <unistd.h>   
#include <sys/types.h>  
#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>    
#include <dirent.h>   

// 100217006 Elif Nur Kemiksiz
// 150119664 Nesrin Şimşek
// 150119809 Hakan Sandıkçı

#define MAX_LEN 300 //This is for line read 

pthread_mutex_t mutex;   // This is for mutex
int numOfElements = 0;   // This is for number of wordsArray's array


void* allocation(); //allocates memory and reallocates if necessary
int search(char *word); //searchs words in the char **arr not to add the same word to the same index
void addElement(char *word); //adds words into the related index in the char **arr  
void* readAndAdd(void* file_name); // reads files line by line. this is the function parameter for pthread creation


char **arr; // Global variables for change array of elements (We cannot solve this problem :'( )
int size;

struct fileProperty{    // This was our next plan but we coundn't use. For this struct we store our file data in linkedlist.
    char *fileName;
    char *path;
    char read_status;   // If it is not read yet, so that thread will start (It was our plan)
    struct fileProperty *next;
};

void* allocation(){
    if( numOfElements == 0 ){   //if words's array isn't allocated yet, it wil allocates 8 times char array size
        arr = (char **) malloc (8 * sizeof(char *));
        size = 8;
        printf("MAIN THREAD: Allocated initial array of 8 pointers.\n");
    }
    else{   //if the memory already allocated, than program should reallocate double size
        arr = (char **) realloc ( arr ,2 * size * sizeof(char *) );
        printf("THREAD %lu: Re-allocated array of %d pointers.\n", pthread_self(), size);
    }
}

int search(char *word){
    int index;
    for(index = 0; index < numOfElements; index++){     //searching all elements in array
        if(strcmp(arr[index], word)  == 0)   
            return index;   // if it finds the word in index sends index number
    }
    return -1;  //if it is not found sends minus one
}

void addElement(char *word){
    int relatedIndex = search(strtok(word,"."));
    pthread_mutex_lock(&mutex);
    
    if(relatedIndex == -1){
        arr[numOfElements] = strtok(word,"."); //discards dot in the word
        printf("THREAD %lu: Added \"%s\" at index %d.\n", pthread_self(), arr[numOfElements], numOfElements );
        numOfElements += 1; //increases number of elements - which is used for storing word count that are added into char **arr -
    }
    else{
        printf("THREAD %lu: The word \"%s\" has already located at index %d.\n",pthread_self(), word, numOfElements);
    }
    if(numOfElements == size){ //checks the allocated memory to determine whether there is a need for reallocation
        allocation();
    } 
    pthread_mutex_unlock(&mutex);
}

/*void threadShift(){       //this was our plan. This will check the unread files and redirect to read file
    
}*/

void* readAndAdd(void* file_name){
    char * path = (char*)malloc(sizeof(char) * 50);
    strcpy(path,(char*)file_name);
    FILE * file = fopen(path, "r");
    char *line;
    line = (char*)malloc(MAX_LEN);  

    while (fgets(line, MAX_LEN, file)!= NULL){  //Getting lines one by one
        int i = 0;
        char *tokenPtr;
        char *rest = strtok(line,"\n");
        while((tokenPtr = strtok_r(rest," ",&rest))){
            addElement(tokenPtr);   //calls addElements for adding tokenPtr to wordsArray's arr
        }
    }
    fclose(file);
    //Thread ends
    //threadShfit(); //This was our plan
    free(path);
}


int main(int argc, char *argv[]){
    int numOfThreads = atoi(argv[4]); //takes the thread count parameter

    int i;
    char *d_name = argv[2];
    strcat(d_name,"/");
    pthread_t t[numOfThreads];
    allocation();

    int index = 0;
    DIR *d; 
    struct dirent *dir; //Its for reading directory
    d = opendir(d_name);    //opens related directory
    char **file_names = (char**)malloc(sizeof(char*) * 10);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if ( !strcmp( dir->d_name, "."  )) continue;
            if ( !strcmp( dir->d_name, ".." )) continue;
            char *path = (char*) malloc(sizeof(char) * 100);
            file_names[index] = (char*)malloc(sizeof(char) * 50);
            strcpy(file_names[index],d_name);   //adds directory name to file names
            strcat(file_names[index],dir->d_name);   //add file names to file name
            index = index + 1;
        }
        closedir(d);
    }
    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < numOfThreads; i++){  //Threads will create
        pthread_create(&t[i], NULL, readAndAdd,(void *)file_names[i]);
        printf("MAIN THREAD: Assigned \"%s\" to worker thread = %lu.\n", file_names[i] , t[i]);
    }

    for ( i = 0; i < numOfThreads; i++)//frees file namse (gets error)
       pthread_join(t[i], NULL);
   
    pthread_mutex_destroy(&mutex);
/*
    for ( i = 0; i<10; i++){
        free(*(file_names+i));      //free file names (gets error)
    }
    free(file_names);
    
    for (i = 0;i<size;i++) 
        free((arr[i]));     //free file names (gets error)
    //free(arr);
*/
    printf("MAIN THREAD: All done (successfully read %d unique words with %d threads from %d files).", numOfElements, numOfThreads, numOfThreads);
}