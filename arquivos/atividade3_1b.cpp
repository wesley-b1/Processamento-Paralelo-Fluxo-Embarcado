#include <iostream>
#include <pthread.h>
#include <cmath> // Para M_PI

#define T 4 // Número de threads
/*
threads por núcleo: 2
Núcleos por socket: 2
Socket: 1
Intel(R) Core(TM) i5-7200U CPU @ 2.50GHz
*/
int n = 10000000; // Altere este valor para mudar a quantidade de termos
double sum = 0.0; // Acumulador global
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Função executada por cada thread
void *calc_pi_thread(void *arg)
{
    long my_id = (long)arg;
    std::cout << "thread " << my_id;
    pthread_t thId = pthread_self(); // obtém o identificador da thread atual
    printf(" with ID: %ld\n", thId); // mostra qual está sendo processada

    int terms_per_thread = n / T;
    int start = my_id * terms_per_thread;

    // Isso garante que a última thread (my_id == T - 1) processe até o final (n),
    // absorvendo qualquer "resto" da divisão, caso n não seja divisível exatamente por T.
    // As outras threads (0 a T−2) processam apenas seu "bloco" de terms_per_thread.
    int end;
    if (my_id == T - 1)
    {
        end = n;
    }
    else
    {
        end = start + terms_per_thread;
    }

    double factor;
    // Como cada thread começa a calcular em uma parte diferente da série (usando start)
    // ela precisa saber qual sinal deve usar no primeiro termo. Assim:
    if (start % 2 == 0) // Se start é par, o termo começa positivo (+).
    {
        factor = 1.0;
    }
    else // Se start é ímpar, o termo começa negativo (-).
    {
        factor = -1.0;
    }
    // Somatório local fora da região crítica
    // para evitar muitas chamadas a pthread_mutex_lock() por thread (chamada privada da thread)
    double my_sum = 0.0;
    for (int i = start; i < end; i++)
    {
        double term = factor / (2 * i + 1);
        my_sum += term;
        factor = -factor;
    }
    // Atualiza a soma global em região crítica (uma única vez por thread)
    pthread_mutex_lock(&lock); // bloqueia o mutex
    sum += my_sum;
    pthread_mutex_unlock(&lock); // desbloqueia o mutex

    return NULL;
}

int main()
{
    pthread_t threads[T];
    // Criação das T = 4 pthreads
    for (long i = 0; i < T; i++)
    {
        pthread_create(&threads[i], NULL, calc_pi_thread, (void *)i);
    }
    // Espera todas T=4 pthreads terminarem
    for (int i = 0; i < T; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // somatório total das somas multiplicado por 4 referente as 4 pthreads
    double pi = 4.0 * sum;
    // Imprime o resultado com 15 casas decimais de precisão.
    std::cout.precision(15);
    std::cout << "PI com " << n << " termos: " << pi << std::endl;

    return 0;
}