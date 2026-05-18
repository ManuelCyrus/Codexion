#include "../include/codexion.h"

// Função auxiliar para calcular a chave de prioridade dependendo do scheduler
static long long get_priority_key(t_coder *coder)
{
    if (coder->env->is_edf)
        return (coder->last_compile_start + coder->env->time_to_burnout);
    return (get_time_ms()); // FIFO usa o tempo atual de chegada
}

// Tenta adquirir um dongle específico seguindo as regras do Heap e Cooldown
static void acquire_dongle(t_coder *coder, t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);

    pthread_mutex_lock(&coder->env->state_mutex);
    int seq = coder->env->global_sequence++;
    pthread_mutex_unlock(&coder->env->state_mutex);

    t_request req = {coder->id, get_priority_key(coder), seq};
    heap_insert(&dongle->queue, req);

    while (1)
    {
        pthread_mutex_lock(&coder->env->state_mutex);
        int am_i_first = (dongle->queue.size > 0 && heap_peek(&dongle->queue).coder_id == coder->id);
        int running = coder->env->simulation_running;
        pthread_mutex_unlock(&coder->env->state_mutex);

        if (!running)
        {
            heap_remove_coder(&dongle->queue, coder->id);
            pthread_mutex_unlock(&dongle->mutex);
            pthread_exit(NULL);
        }

        long long now = get_time_ms();
        if (am_i_first && !dongle->is_taken && now >= dongle->available_at)
        {
            // Conseguiu o dongle!
            dongle->is_taken = 1;
            dongle->current_holder_id = coder->id;
            heap_extract_min(&dongle->queue);
            pthread_mutex_unlock(&dongle->mutex);
            
            print_status(coder->env, coder->id, "has taken a dongle");
            return;
        }

        // Se o dongle está ocupado ou em cooldown, espera o sinal
        if (!dongle->is_taken && now < dongle->available_at && am_i_first)
        {
            pthread_mutex_unlock(&dongle->mutex);
            usleep((dongle->available_at - now) * 1000);
            pthread_mutex_lock(&dongle->mutex);
        }
        else
        {
            pthread_cond_wait(&dongle->cond, &dongle->mutex);
        }
    }
}

static void release_dongle(t_coder *coder, t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->is_taken = 0;
    dongle->current_holder_id = 0;
    dongle->available_at = get_time_ms() + coder->env->dongle_cooldown;
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

void *coder_routine(void *arg)
{
    t_coder *coder = (t_coder *)arg;
    t_env   *env = coder->env;

    // Identifica qual dongle tem o ID menor para evitar Deadlock estrito
    t_dongle *first = (coder->left_dongle->id < coder->right_dongle->id) ? coder->left_dongle : coder->right_dongle;
    t_dongle *second = (first == coder->left_dongle) ? coder->right_dongle : coder->left_dongle;

    // Caso bizarro de apenas 1 coder na mesa (1 dongle total)
    if (coder->left_dongle == coder->right_dongle)
    {
        acquire_dongle(coder, coder->left_dongle);
        // Ele vai travar aqui tentando o segundo e morrer por burnout (comportamento correto)
        acquire_dongle(coder, coder->left_dongle); 
    }

    while (1)
    {
        // 1. Tenta pegar os dois dongles de forma ordenada
        acquire_dongle(coder, first);
        acquire_dongle(coder, second);

        // 2. Compilar
        pthread_mutex_lock(&env->state_mutex);
        coder->last_compile_start = get_time_ms();
        pthread_mutex_unlock(&env->state_mutex);

        print_status(env, coder->id, "is compiling");
        usleep(env->time_to_compile * 1000);

        pthread_mutex_lock(&env->state_mutex);
        coder->compiles_count++;
        pthread_mutex_unlock(&env->state_mutex);

        // 3. Largar os dongles (Liberar em ordem inversa)
        release_dongle(coder, second);
        release_dongle(coder, first);

        // 4. Debuggar
        print_status(env, coder->id, "is debugging");
        usleep(env->time_to_debug * 1000);

        // 5. Refatorar
        print_status(env, coder->id, "is refactoring");
        usleep(env->time_to_refactor * 1000);

        // Checagem rápida de término voluntário
        pthread_mutex_lock(&env->state_mutex);
        if (!env->simulation_running)
        {
            pthread_mutex_unlock(&env->state_mutex);
            break;
        }
        pthread_mutex_unlock(&env->state_mutex);
    }
    return (NULL);
}

void *monitor_routine(void *arg)
{
    t_env *env = (t_env *)arg;

    while (1)
    {
        pthread_mutex_lock(&env->state_mutex);
        if (!env->simulation_running)
        {
            pthread_mutex_unlock(&env->state_mutex);
            break;
        }

        long long now = get_time_ms();
        int all_done = 1;

        for (int i = 0; i < env->num_coders; i++)
        {
            // Checar Burnout
            long long deadline = env->coders[i].last_compile_start + env->time_to_burnout;
            if (now >= deadline)
            {
                env->simulation_running = 0;
                pthread_mutex_unlock(&env->state_mutex);
                
                // Print imediato (requisito de precisão < 10ms)
                pthread_mutex_lock(&env->print_mutex);
                printf("%lld %d burned out\n", now - env->start_time, env->coders[i].id);
                pthread_mutex_unlock(&env->print_mutex);
                
                // Acorda todo mundo que sobrou esperando em variáveis de condição para encerrarem
                for (int d = 0; d < env->num_coders; d++)
                {
                    pthread_mutex_lock(&env->dongles[d].mutex);
                    pthread_cond_broadcast(&env->dongles[d].cond);
                    pthread_mutex_unlock(&env->dongles[d].mutex);
                }
                return (NULL);
            }

            // Checar se ainda falta alguém bater a meta de compilações
            if (env->coders[i].compiles_count < env->compiles_required)
                all_done = 0;
        }

        if (all_done)
        {
            env->simulation_running = 0;
            pthread_mutex_unlock(&env->state_mutex);
            
            // Acorda threads pendentes para encerrarem de forma limpa
            for (int d = 0; d < env->num_coders; d++)
            {
                pthread_mutex_lock(&env->dongles[d].mutex);
                pthread_cond_broadcast(&env->dongles[d].cond);
                pthread_mutex_unlock(&env->dongles[d].mutex);
            }
            return (NULL);
        }

        pthread_mutex_unlock(&env->state_mutex);
        usleep(1000); // Granularidade fina de 1ms para o monitor
    }
    return (NULL);
}
