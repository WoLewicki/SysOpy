#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define LINE_MAX 5000
#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}


FILE *input;
FILE *filter;
FILE *output;

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

int find_proper (int min, int b, int max)
{
	if (b<=min) return min;
	else if (b > min && b <= max) return b;
	else return max;
}

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
    int proper_i;
    int proper_j;
    for (int i = y-c/2; i <= y+c/2; ++i, filtercordy++) {
        filtercordx =0;
        for (int j = x-c/2; j <= x+c/2; ++j, filtercordx++) {
        	proper_i = find_proper(0, i, height-1);
        	proper_j = find_proper(0, j, width-1);
            pixel += picture_array[proper_i][proper_j] * filter[filtercordy][filtercordx];
        }
    }
    int result = (int) roundf(pixel);
    return result;
}

void *fillarray (void *args)
{
    struct arg_struct *arguments = (struct arg_struct *)args;
    for (int i = arguments->starting_row; i < arguments->ending_row; ++i) {
        for (int j = 0; j <arguments->row_width; ++j) {
            arguments->helper[i*arguments->row_width + j] = calculatepixel(arguments->filter, arguments->picture_array,
            arguments->c, j, i, arguments->row_width, arguments->column_height);
        }
    }
    pthread_exit(NULL);
}

void closer()
{
    fclose(input);
    fclose(filter);
    fclose(output);
}

int main(int argc, char *argv[]) {
    if (argc != 5) FAILURE_EXIT(1, "Pass 4 arguments.\n");
    int threads = (int) strtol(argv[1], NULL, 10);
    input = fopen(argv[2], "r");
    filter = fopen(argv[3], "r");
    output = fopen(argv[4], "w");

    if (input == NULL || output == NULL || filter == NULL) {
        FAILURE_EXIT(1, "No input or output or filter detected.\n");
    }
    if (atexit(closer)) FAILURE_EXIT(1, "Cannot register atexit function.\n");
    char line[LINE_MAX];
    int len = LINE_MAX;
    fgets(line, len, filter); // pierwsza linijka powinna miec c
    int c = (int) strtol(line, NULL, 10);
    if (c%2 != 1) FAILURE_EXIT(1, "c should be odd.\n");
    float **filterarray = malloc(c * sizeof(float *));
    int l;
    for (l = 0; l < c; ++l) {
        filterarray[l] = malloc(c * sizeof(float));
    }
    int k = 0;
    int a;
    char *word;
    char *checker;
    while (fgets(line, len, filter)) //przyjmuje ze dane sa w postaci macierzy linijki/kolumny
    {
        a = 0;
        word = line;
        while ((checker = strtok_r(NULL, " \n", &word))) // pobieranie floatow po kolei
        {
            if (checker != NULL) filterarray[k][a++] = strtof(checker, NULL);
        }
        k++;
    }
    fgets(line, len, input); // tu jest P2
    fgets(line, len, input); // tu jest W i H
    word = line;
    int W = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    int H = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    if (threads >= H) FAILURE_EXIT(1, "There is no point in having more threads than rows to produce. Exiting.\n");
    fgets(line, len, input); // kolejna linijka powinna miec M
    word = line;
    int M = (int) strtol(strtok_r(NULL, " \n", &word), NULL, 10);
    if (M > 255 || W < 0 || H < 0) FAILURE_EXIT(1, "M was bigger than 255 or W/H <0. Exiting.\n");
    int *inputhelper = calloc(sizeof(int), (size_t) W * H);
    a = 0;
    while (fgets(line, len, input)) //przyjmuje ze dane sa w postaci macierzy linijki/kolumny
    {
        word = line;
        while ((checker = strtok_r(NULL, " \n", &word))) // pobieranie floatow po kolei
        {
            if (checker != NULL) inputhelper[a++] = (int) strtol(checker, NULL, 10);
        }
    }
    int **picturearray = malloc(H * sizeof(int *));
    for (l = 0; l < H; ++l) {
        picturearray[l] = malloc(W * sizeof(int));
    }
    for (int n = 0; n < H; ++n) {
        for (int i = 0; i < W; ++i) {
            picturearray[n][i] = inputhelper[n * W + i];
        }
    }
    int *helper = malloc((size_t) W * H);
    pthread_t thread_array[threads];
    int helpthreads = threads;
    int rows_remaining = H + 1;
    int start = 0;
    int end = start + rows_remaining / helpthreads;
    int quantity;
    int i = 0;
    struct arg_struct structarray[threads];
    while (helpthreads > 0) {
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
        structarray[i] = arguments;
        pthread_t thread;
        thread_array[i++] = thread;
        quantity = rows_remaining / helpthreads;
        rows_remaining -= quantity;
        helpthreads--;
        start = end;
        if (helpthreads == 1) end = H;
        else end = start + quantity;
    }
    struct timeval tv1, tv2;
    gettimeofday(&tv1, 0);
    for (i = 0; i < threads; ++i) {
        if (pthread_create(&thread_array[i], NULL, &fillarray, (void *) &structarray[i])) FAILURE_EXIT(1,
                                                                                                       "Couldn't make a thread.\n");
    }
    for (i = 0; i < threads; ++i) {
        pthread_join(thread_array[i], NULL);
    }
    gettimeofday(&tv2, 0);
    printf("Picture resolution: %dx%d, filter c: %d, threads: %d, ", W, H, c, threads);
    calctimediff(tv1, tv2);
    fprintf(output, "P2\n");
    fprintf(output, "%d %d\n", W, H);
    fprintf(output, "%d\n", M);
    for (int m = 0; m < H; ++m) {
        for (int j = 0; j < W; ++j) {
            fprintf(output, "%d ", helper[m * W + j]);
        }
        fprintf(output, "\n");
    }
    return 0;
}
