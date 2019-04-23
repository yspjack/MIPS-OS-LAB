#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *  Search through 'envs' for a runnable environment ,
 *  in circular fashion statrting after the previously running env,
 *  and switch to the first such environment found.
 *
 * Hints:
 *  The variable which is for counting should be defined as 'static'.
 */
void sched_yield(void)
{
    static int i = -1;
    static int p = 0;
    struct Env *e;
    while (1)
    {

#if 1
        if (LIST_EMPTY(&env_sched_list[p]))
        {
            p ^= 1;
        }
        LIST_FOREACH(e, &env_sched_list[p], env_sched_link)
        {
            if (e->env_status == ENV_RUNNABLE)
            {
                break;
            }
        }
        if (e == NULL)
        {
            continue;
        }
        if (i == -1)
        {
            i = e->env_pri;
        }
        else if (i == 0)
        {
            LIST_REMOVE(e, env_sched_link);
            LIST_INSERT_HEAD(&env_sched_list[p ^ 1], e, env_sched_link);
            i = -1;
        }
        else
        {
            // printf("DEBUG:time %d\n", i);
            --i;
            env_run(e);
            panic("unreachable after env_run");
        }
#else
        i = (i + 1) % NENV;
        if (envs[i].env_status == ENV_RUNNABLE)
        {
            env_run(&envs[i]);
            panic("unreachable after env_run");
        }
#endif
    }
    panic("unreachable");
}
