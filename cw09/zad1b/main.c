#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINE_MAX 1024
#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define SEARCH_LESSER 0
#define SEARCH_EQUAL 1
#define SEARCH_GREATER 2

sem_t array_sem;

char **global_array;
FILE *read_input;
int should_print;
int search_type;
int L;
int P;
int N;
int resources =0;
int p_index=0;
int c_index=0;

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



int get_P_index(){
    while(resources == N)
    {
        sem_post(&array_sem);
        sleep(1);
        sem_wait(&array_sem);
    }
    int index =p_index;
    while(global_array[index] != NULL) {
        if (index == N-1) index = 0;
        else index++;
    }
    return index;
}

void* producent_function(void* arg){
    int index;
    unsigned int name = *((unsigned int *) arg);
    char lineBuffer[LINE_MAX];
    while(1){
        sem_wait(&array_sem);
        index = get_P_index();
        p_index = index;
        if(fgets(lineBuffer, LINE_MAX, read_input) == NULL) {
            printf("End of Pan Tadeusz.\n");
            exit(0);
        }
        if (should_print) printf("Found place (%d) to put piece of text in producent %d\n", index, name);
        global_array[index] = malloc(sizeof(char) * (strlen(lineBuffer) + 1));
        strcpy(global_array[index], lineBuffer);
        resources++;
        sem_post(&array_sem);
    }
}



int get_C_index(){
    while(resources == 0)
    {
        sem_post(&array_sem);
        sleep(1);
        sem_wait(&array_sem);
    }
    int index =c_index;
    while(global_array[index] == NULL) {
        if (index == N-1) index = 0;
        else index++;
    }
    return index;
}

void* consumer_function(void* arg) {
    int index;
    while(1){
        sem_wait(&array_sem);
        index = get_C_index();
        c_index = index;
        if ((strlen(global_array[index]) < L && search_type == SEARCH_LESSER) ||
            (strlen(global_array[index]) == L && search_type == SEARCH_EQUAL) ||
            (strlen(global_array[index]) > L && search_type == SEARCH_GREATER)) {

            printf("%s",global_array[index]);
        }
        free(global_array[index]);
        global_array[index] = NULL;
        resources--;
        sem_post(&array_sem);
    }
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

    
    if (sem_init(&array_sem, 0, 1) < 0) FAILURE_EXIT(1, "Couldnt make semaphore.\n");

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
