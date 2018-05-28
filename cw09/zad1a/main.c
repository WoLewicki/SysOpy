#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>

#define LINE_MAX 1024
#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define SEARCH_LESSER 0
#define SEARCH_EQUAL 1
#define SEARCH_GREATER 2

pthread_mutex_t *array_mutexes;
pthread_mutex_t pointer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c_index_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;


char **global_array;
FILE *read_input;
int should_print;
int c_index =0;
int p_index =0;
int search_type;
int L;
int P;
int main_process_pid;
int N;
int resources =0;
int currentConsumer =-1;
int currentProducer =-1;
int get_C_index();
int get_P_index();

void int_handler (int signo)
{
    if(signo == SIGINT)
    {
        printf("Got CTRL+C. Exiting.\n");
        exit(0);
    }
}

int find_search_type(char *input)
{
    input = strtok_r(NULL, "\n", &input);
    if (strcmp("<", input) == 0) return SEARCH_LESSER;
    else if (strcmp("=", input) == 0) return SEARCH_EQUAL;
    else if (strcmp(">", input) == 0) return SEARCH_GREATER;
    else return -1;
}

void* producent_function(void* arg){
    int index;
    unsigned int id = *((unsigned int *) arg);
    char lineBuffer[LINE_MAX];
    while(1){
        pthread_mutex_lock(&pointer_mutex);
        index = get_P_index();
        pthread_mutex_lock(&array_mutexes[index]);
        pthread_mutex_unlock(&pointer_mutex);
        pthread_mutex_lock(&input_mutex);
        if(fgets(lineBuffer, LINE_MAX, read_input) == NULL) {
            printf("End of Pan Tadeusz.\n");
            exit(0);
        }
        pthread_mutex_unlock(&input_mutex);
        if (should_print) printf("Found place (%d) to put piece of text in producent %d\n", index, id);
        global_array[index] = malloc(sizeof(char) * (strlen(lineBuffer) + 1));
        strcpy(global_array[index], lineBuffer);
        pthread_mutex_unlock(&array_mutexes[index]);
    }
}

void* consumer_function(void* arg) {
    int index;
    unsigned int id = *((unsigned int *) arg);
    while(1){
        pthread_mutex_lock(&pointer_mutex);
        index = get_C_index();
        pthread_mutex_lock(&array_mutexes[index]);
        pthread_mutex_unlock(&pointer_mutex);

        if ((strlen(global_array[index]) < L && search_type == SEARCH_LESSER) ||
            (strlen(global_array[index]) == L && search_type == SEARCH_EQUAL) ||
            (strlen(global_array[index]) > L && search_type == SEARCH_GREATER)) {
			
			if(should_print) printf("Consumer %d:",id);
            printf("%s",global_array[index]);
        }
        free(global_array[index]);
        global_array[index] = NULL;

        pthread_mutex_unlock(&array_mutexes[index]);
    }
}


int get_P_index(){
    while(currentProducer + 1 == currentConsumer || (currentProducer == N- 1 && currentConsumer == 0))
        pthread_cond_wait(&not_full, &pointer_mutex);
    if(currentConsumer == currentProducer)
        pthread_cond_broadcast(&not_empty);

    if(currentProducer == N - 1)
        currentProducer = 0;
    else
        currentProducer++;
    return currentProducer;
}

int get_C_index(){
    while(currentConsumer == currentProducer)
        pthread_cond_wait(&not_empty, &pointer_mutex);
    if(currentProducer + 1 == currentConsumer || (currentProducer == N - 1 && currentConsumer == 0))
        pthread_cond_broadcast(&not_full);

    if(currentConsumer == N - 1)
        currentConsumer = 0;
    else
        currentConsumer++;
    return currentConsumer;

}
int main(int argc, char *argv[]) {
    signal(SIGINT, &int_handler);
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) FAILURE_EXIT(1, "Failed to open configuration file.\n");
    char line[LINE_MAX];
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    P = (int) strtol(line, NULL, 10);
    if (P < 1) FAILURE_EXIT(1, "Producents' number should be bigger than 1.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    int K = (int) strtol(line, NULL, 10);
    if (K < 1) FAILURE_EXIT(1, "Consumers' number should be bigger than 1.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    N = (int) strtol(line, NULL, 10);
    if (N <= 1) FAILURE_EXIT(1, "Array should be bigger than 1.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    char *Pan_Tadeusz = line;
    Pan_Tadeusz = strtok_r(NULL, "\n", &Pan_Tadeusz); // wywalenie koncowki \n
    read_input = fopen(Pan_Tadeusz, "r");
    if (read_input == NULL) FAILURE_EXIT(1, "There is no file to read.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    L = (int) strtol(line, NULL, 10);
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    char *finder = line;
    search_type = find_search_type(finder);
    if (search_type == -1) FAILURE_EXIT(1, "Didn't pass good search type.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    should_print = (int) strtol(line, NULL, 10);
    if (!(should_print == 0 || should_print == 1)) FAILURE_EXIT(1, "should_print should be 1 or 0.\n");
    fgets(line, LINE_MAX, input_file);
    if (line == NULL) FAILURE_EXIT(1, "Couldn't read something.\n");
    int nk = (int) strtol(line, NULL, 10);
    if (!(nk == 0 || nk == 1)) FAILURE_EXIT(1, "nk should be 1 or 0.\n");
    fclose(input_file);

    array_mutexes = malloc(sizeof(pthread_mutex_t) * N);
    for (int k = 0; k <N ; ++k) {
        pthread_mutex_init(&array_mutexes[k], NULL);
    }

    main_process_pid = getpid();
    global_array = calloc((size_t)N, sizeof(char *));
    pthread_t thread_array[P+K];
    unsigned int ints_array[P +K];
    for (unsigned int i = 0; i <P ; ++i) {
        ints_array[i] = i;
        pthread_create(&thread_array[i], NULL, producent_function, (void *)&ints_array[i]);
    }
    for (unsigned int i = (unsigned int) P; i <K+P ; ++i) {
        ints_array[i] = i;
        pthread_create(&thread_array[i], NULL, consumer_function, (void *)&ints_array[i]);
    }
    if (nk > 0)
    {
        sleep((unsigned int)nk);
        printf("End of time.\n");
    }
    else {
        for (int j = 0; j < P + K; ++j) {
            pthread_join(thread_array[j], NULL);
        }
    }
    fclose(read_input);
    free(global_array);
    return 0;
}
