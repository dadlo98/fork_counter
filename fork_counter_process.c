#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <semaphore.h>

sem_t * process_semaphore;

int * fork_counter;

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }
#define CHECK_ERR_MMAP(a,msg) {if ((a) == MAP_FAILED) { perror((msg)); exit(EXIT_FAILURE); } }

int main(void) {
    
    // ESERCIZIO: rendere fork_counter una variabile condivisa tra tutti i processi
    // gestendo opportunamente la concorrenza
    
    int res;
    pid_t pid;
    
    process_semaphore = mmap(NULL, // NULL: è il kernel a scegliere l'indirizzo
                    sizeof(sem_t) + sizeof(int), // dimensione della memory map
                    PROT_READ | PROT_WRITE, // memory map leggibile e scrivibile
                    MAP_SHARED | MAP_ANONYMOUS, // memory map condivisibile con altri processi e senza file di appoggio
                    -1,
                    0); // offset nel file
    CHECK_ERR_MMAP(process_semaphore,"mmap")

    fork_counter = (int *) (process_semaphore + 1);
    printf("\nfork_counter before any fork: %d \n", *fork_counter);

    res = sem_init(process_semaphore, 1, 1);
    CHECK_ERR(res, "sem_init");

    pid = getpid();

    for(int i = 0; i < 4; i++) {
        if(fork() == -1)
            exit(EXIT_FAILURE);
        if(wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
    }

    if(sem_wait(process_semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }

    (*fork_counter)++;

    if(sem_post(process_semaphore) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

    printf("\npartial fork_counter result: %d\n", *fork_counter);

    if(pid == getpid()){
        printf("\n\n***final fork_counter result: %d***\n\n", *fork_counter);
    }
    
    res = sem_destroy(process_semaphore);
    CHECK_ERR(res,"sem_destroy")

    return 0;
}
