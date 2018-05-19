#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define LINE_MAX 5000
#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

struct arg_struct {
    float **filter;
    int **picture_array;
    int c;
    int *helper;
    int starting_row;
    int ending_row;
    int row_width;
    int column_height;
};

void calctimediff (struct timeval tv1,struct timeval tv2)
{
    long secs = tv2.tv_sec - tv1.tv_sec;
    long msecs = tv2.tv_usec - tv1.tv_usec;
    if (msecs < 0) { msecs += 1000000; secs -= 1; }
    printf("time spent: %ld.%06lds\n", secs, msecs);
}

int calculatepixel(float **filter, int **picture_array, int c, int x, int y, int width, int height)
{
    float pixel =0;
    int filtercordx;
    int filtercordy =0;
    for (int i = y-c/2; i <= y+c/2; ++i, filtercordy++) {
        filtercordx =0;
        for (int j = x-c/2; j <= x+c/2; ++j, filtercordx++) {
            if (i < 0 || j < 0 || i>= height || j>= width) pixel += 1*filter[filtercordy][filtercordx];
            else pixel += picture_array[i][j] * filter[filtercordy][filtercordx];

        }
    }
    int result = (int) roundf(pixel);
    return result;
}

void *fillarray (void *args)
{
    struct arg_struct *arguments = (struct arg_struct *)args;
    printf("%d - start, %d - end\n", arguments->starting_row, arguments-> ending_row);
    for (int i = arguments->starting_row; i < arguments->ending_row; ++i) {
        for (int j = 0; j <arguments->row_width; ++j) {
            arguments->helper[i*arguments->row_width + j] = calculatepixel(arguments->filter, arguments->picture_array,
            arguments->c, j, i, arguments->row_width, arguments->column_height);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) FAILURE_EXIT(1, "Pass 4 arguments.\n");
    int threads = (int) strtol(argv[1], NULL, 10);
    FILE *input = fopen(argv[2], "r");
    FILE *filter = fopen(argv[3], "r");
    FILE *output = fopen(argv[4], "w");

    if (input == NULL || output == NULL || filter == NULL)
    {
        FAILURE_EXIT(1, "No input or output or filter detected.\n");
    }
    char line[LINE_MAX];
    int len = LINE_MAX;
    fgets(line, len, filter); // pierwsza linijka powinna miec c
    int c = (int) strtol(line, NULL, 10);
    float  **filterarray = malloc(c* sizeof(float *));
    int l;
    for (l = 0; l <c; ++l) {
        filterarray[l] = malloc(c *sizeof(float));
    }
    int k=0;
    int a;
    char *word;
    char *checker;
    while(fgets(line, len, filter)) //przyjmuje ze dane sa w postaci macierzy linijki/kolumny
    {
        a =0;
        word = line;
        while ((checker = strtok_r(NULL, " \n", &word))) // pobieranie floatow po kolei
        {
            if (checker !=NULL) filterarray[k][a++] = strtof(checker, NULL);
        }
        k++;
    }
    fgets(line, len, input); // tu jest jakas P2 nie weim po co
    fgets(line, len, input); // tu jest W i H
    word = line;
    int W = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    int H = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    fgets(line, len, input); // kolejna linijka powinna miec M
    word = line;
    int M = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    if (M > 255) FAILURE_EXIT(1, "M was bigger than 255. Exiting.\n");
    int **picturearray = malloc(H* sizeof(int *));
    for ( l = 0;  l< H; ++l) {
        picturearray[l] = malloc(W * sizeof(int));
    }
    int w =0;
    int h = 0;
    fgets(line, len, input); // piksele sa w jednej dlugiej linii
    word = line;
    while ((checker = strtok_r(NULL, " \n", &word))) // pobieranie intow
    {
        if (checker != NULL) picturearray[h][w++] = (int) strtol(checker, NULL, 10);
        if (w == W)
        {
            w = 0;
            h++;
        }
    }
    int *helper = malloc((size_t)W*H);
    pthread_t thread_array[threads];
    int helpthreads = threads;
    int rows_remaining = H+1;
    int start = 0;
    int end = start + rows_remaining/helpthreads;
    int quantity; // how many rows are you taking at once
    int i =0;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, 0);
    while (helpthreads > 0)
    {
        struct arg_struct arguments;
        arguments.filter = filterarray;
        arguments.picture_array = picturearray;
        arguments.c = c;
        arguments.helper = helper;
        arguments.row_width = W;
        arguments.column_height = H;
        arguments.starting_row = start;
        if (threads != 1)arguments.ending_row = end;
        else arguments.ending_row = H;
        pthread_t thread;
        if (pthread_create(&thread, NULL, &fillarray, (void *)&arguments)) FAILURE_EXIT(1, "Couldn't make a thread.\n");
        thread_array[i++] = thread;
        quantity = rows_remaining/helpthreads;
        rows_remaining -= quantity;
        helpthreads--;
        start = end;
        if (helpthreads == 1) end = H;
        else end = start + quantity;
    }
    for ( i = 0; i <threads; ++i) {
        pthread_join(thread_array[i], NULL);
    }
    gettimeofday(&tv2, 0);
    calctimediff(tv1, tv2);
    fprintf(output, "P2\n");
    fprintf(output, "%d %d\n", W, H);
    fprintf(output, "%d\n", M);
    fprintf(output, "%d", helper[0]);
    for (int j = 1; j <W*H ; ++j) {
        fprintf(output, " %d", helper[j]);
    }
    fclose(input);
    fclose(filter);
    fclose(output);
    return 0;
}