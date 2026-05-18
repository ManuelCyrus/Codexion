#include "../include/codexion.h"

static int init_simulation(t_env *env)
{
    env->dongles = malloc(sizeof(t_dongle) * env->num_coders);
    env->coders = malloc(sizeof(t_coder) * env->num_coders);
    if (!env->dongles || !env->coders)
        return (0);

    pthread_mutex_init(&env->print_mutex, NULL);
    pthread_mutex_init(&env->state_mutex, NULL);
    env->simulation_running = 1;
    env->global_sequence = 0;
    env->start_time = get_time_ms();

    // Inicializa Dongles
    for (int i = 0; i < env->num_coders; i++)
    {
        env->dongles[i].id = i + 1;
        env->dongles[i].is_taken = 0;
        env->dongles[i].current_holder_id = 0;
        env->dongles[i].available_at = 0;
        pthread_mutex_init(&env->dongles[i].mutex, NULL);
        pthread_cond_init(&env->dongles[i].cond, NULL);
        heap_init(&env->dongles[i].queue, env->num_coders);
    }

    // Inicializa Coders
    for (int i = 0; i < env->num_coders; i++)
    {
        env->coders[i].id = i + 1;
        env->coders[i].compiles_count = 0;
        env->coders[i].last_compile_start = env->start_time;
        env->coders[i].env = env;
        
        // Mesa Circular: Dongle Esquerdo (i), Dongle Direito ((i + 1) % num_coders)
        env->coders[i].left_dongle = &env->dongles[i];
        env->coders[i].right_dongle = &env->dongles[(i + 1) % env->num_coders];
    }
    return (1);
}

static void clean_simulation(t_env *env)
{
    if (env->dongles)
    {
        for (int i = 0; i < env->num_coders; i++)
        {
            pthread_mutex_destroy(&env->dongles[i].mutex);
            pthread_cond_destroy(&env->dongles[i].cond);
            free(env->dongles[i].queue.data);
        }
        free(env->dongles);
    }
    if (env->coders)
        free(env->coders);
        
    pthread_mutex_destroy(&env->print_mutex);
    pthread_mutex_destroy(&env->state_mutex);
}

int main(int ac, char **av)
{
    t_env             env;
    pthread_t         monitor;

    memset(&env, 0, sizeof(t_env));
    if (!validate_args(ac, av, &env))
        return (1);

    if (!init_simulation(&env))
    {
        clean_simulation(&env);
        return (1);
    }

    // Cria a thread de monitoramento em tempo real
    if (pthread_create(&monitor, NULL, &monitor_routine, &env) != 0)
    {
        clean_simulation(&env);
        return (1);
    }

    // Cria as threads dos programadores
    for (int i = 0; i < env.num_coders; i++)
    {
        if (pthread_create(&env.coders[i].thread, NULL, &coder_routine, &env.coders[i]) != 0)
        {
            pthread_mutex_lock(&env.state_mutex);
            env.simulation_running = 0;
            pthread_mutex_unlock(&env.state_mutex);
            break;
        }
    }

    // Aguarda o término de todas as threads
    for (int i = 0; i < env.num_coders; i++)
    {
        pthread_join(env.coders[i].thread, NULL);
    }
    pthread_join(monitor, NULL);

    clean_simulation(&env);
    return (0);
}
