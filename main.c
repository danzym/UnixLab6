#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_THREADS 64
#define ITERATIONS 1000000000

typedef struct {
    int *A;
    int *B;
    int *C;
    int n, m, k;
    int row, col;
} thread_data;

pthread_mutex_t mutex;
long shared_variable = 0;

void* multiply_element(void* arg) {
    thread_data* data = (thread_data*) arg;
    int sum = 0;
    for (int i = 0; i < data->m; i++) {
        sum += data->A[data->row * data->m + i] * data->B[i * data->k + data->col];
    }
    data->C[data->row * data->k + data->col] = sum;
    printf("[Thread for C[%d,%d]] Result = %d\n", data->row, data->col, sum);
    pthread_exit(NULL);
}

void* increment_shared_variable(void* arg) {
    int use_mutex = *(int*)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        if (use_mutex) {
            pthread_mutex_lock(&mutex);
            shared_variable++;
            pthread_mutex_unlock(&mutex);
        } else {
            shared_variable++;
        }
    }
    return NULL;
}

void perform_matrix_multiplication(int *A, int *B, int *C, int n, int m, int k) {
    pthread_t threads[n * k];
    thread_data data[n * k];
    int thread_count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            data[thread_count].A = A;
            data[thread_count].B = B;
            data[thread_count].C = C;
            data[thread_count].n = n;
            data[thread_count].m = m;
            data[thread_count].k = k;
            data[thread_count].row = i;
            data[thread_count].col = j;
            pthread_create(&threads[thread_count], NULL, multiply_element, &data[thread_count]);
            thread_count++;
        }
    }

    for (int i = 0; i < n * k; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    int n = 2, m = 3, k = 2;
    int A[6] = {1, 2, 3, 4, 5, 6};
    int B[6] = {6, 5, 4, 3, 2, 1};
    int C[4] = {0};

    printf("Starting matrix multiplication...\n");
    perform_matrix_multiplication(A, B, C, n, m, k);

    pthread_t thread1, thread2;
    int mutex_use = 1;
    pthread_mutex_init(&mutex, NULL);

    printf("Incrementing shared variable with mutex.\n");
    shared_variable = 0;
    pthread_create(&thread1, NULL, increment_shared_variable, &mutex_use);
    pthread_create(&thread2, NULL, increment_shared_variable, &mutex_use);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Final value with mutex: %ld\n", shared_variable);

    printf("Incrementing shared variable without mutex.\n");
    shared_variable = 0;
    mutex_use = 0;
    pthread_create(&thread1, NULL, increment_shared_variable, &mutex_use);
    pthread_create(&thread2, NULL, increment_shared_variable, &mutex_use);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Final value without mutex: %ld\n", shared_variable);

    pthread_mutex_destroy(&mutex);
    return 0;
}
