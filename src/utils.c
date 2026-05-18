#include "../include/codexion.h"

long long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void print_status(t_env *env, int coder_id, const char *status)
{
    pthread_mutex_lock(&env->print_mutex);
    pthread_mutex_lock(&env->state_mutex);
    if (env->simulation_running)
    {
        long long now = get_time_ms() - env->start_time;
        printf("%lld %d %s\n", now, coder_id, status);
    }
    pthread_mutex_unlock(&env->state_mutex);
    pthread_mutex_unlock(&env->print_mutex);
}

static int is_numeric(const char *str)
{
    int i = 0;
    if (!str || str[i] == '\0') return (0);
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9') return (0);
        i++;
    }
    return (1);
}

int validate_args(int ac, char **av, t_env *env)
{
    if (ac != 9)
    {
        fprintf(stderr, "Error: Invalid number of arguments.\n");
        return (0);
    }
    for (int i = 1; i <= 7; i++)
    {
        if (!is_numeric(av[i]))
        {
            fprintf(stderr, "Error: Argument %d must be a positive integer.\n", i);
            return (0);
        }
    }
    env->num_coders = atoi(av[1]);
    env->time_to_burnout = atoi(av[2]);
    env->time_to_compile = atoi(av[3]);
    env->time_to_debug = atoi(av[4]);
    env->time_to_refactor = atoi(av[5]);
    env->compiles_required = atoi(av[6]);
    env->dongle_cooldown = atoi(av[7]);

    if (env->num_coders < 1) return (0);

    if (strcmp(av[8], "fifo") == 0)
        env->is_edf = 0;
    else if (strcmp(av[8], "edf") == 0)
        env->is_edf = 1;
    else
    {
        fprintf(stderr, "Error: Scheduler must be 'fifo' or 'edf'.\n");
        return (0);
    }
    return (1);
}
