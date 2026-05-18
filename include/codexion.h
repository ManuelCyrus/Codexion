#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct s_request {
    int         coder_id;
    long long   priority_key; // Timestamp de chegada (FIFO) ou Deadline de Burnout (EDF)
    int         sequence_num; // Desempate determinístico para o EDF
} t_request;

typedef struct s_heap {
    t_request   *data;
    int         size;
    int         capacity;
} t_heap;

typedef struct s_dongle {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             id;
    int             is_taken;
    int             current_holder_id;
    long long       available_at; // Fim do dongle_cooldown
    t_heap          queue;        // Fila de prioridade específica deste dongle
} t_dongle;

typedef struct s_coder {
    pthread_t       thread;
    int             id;           // 1 até number_of_coders
    int             compiles_count;
    long long       last_compile_start;
    t_dongle        *left_dongle;
    t_dongle        *right_dongle;
    struct s_env    *env;
} t_coder;

typedef struct s_env {
    int             num_coders;
    int             time_to_burnout;
    int             time_to_compile;
    int             time_to_debug;
    int             time_to_refactor;
    int             compiles_required;
    int             dongle_cooldown;
    int             is_edf;       // 1 se for edf, 0 se for fifo
    
    long long       start_time;
    int             simulation_running;
    int             global_sequence; 
    pthread_mutex_t print_mutex;  
    pthread_mutex_t state_mutex;  
    
    t_dongle        *dongles;
    t_coder         *coders;
} t_env;

long long   get_time_ms(void);
void        print_status(t_env *env, int coder_id, const char *status);
int         validate_args(int ac, char **av, t_env *env);

void        heap_init(t_heap *heap, int capacity);
void        heap_insert(t_heap *heap, t_request req);
t_request   heap_peek(t_heap *heap);
void        heap_extract_min(t_heap *heap);
void        heap_remove_coder(t_heap *heap, int coder_id);




void        *coder_routine(void *arg);
void        *monitor_routine(void *arg);

#endif
