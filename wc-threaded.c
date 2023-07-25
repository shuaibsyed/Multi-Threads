/* Counts the number of words, lines, and characters in the files whose
 * names are given as command-line arguments.  If there are no command-line
 * arguments then the line, word, and character counts will just be 0.
 * Mimics the effects of the UNIX "wc" utility, although does not have
 * exactly the same behavior in all cases.
 */

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LEN 256

typedef struct {
    char filename[MAX_FILENAME_LEN];
    int lines;
    int words;
    int chars;
} file_stats_t;

void *process_file(void *arg) {
    file_stats_t *stats = (file_stats_t *)arg;
    char ch, next_ch;
    FILE *fp;

    /* Open the file */
    fp = fopen(stats->filename, "r");
    if (fp == NULL) {
        /* If the file cannot be opened, exit the thread */
        pthread_exit(NULL);
    }

    /* Initialize the statistics to zero */
    stats->lines = stats->words = stats->chars = 0;

    /* Read the file character-by-character */
    ch = fgetc(fp);
    while (!feof(fp)) {
        /* Look ahead to the next character without consuming it */
        next_ch = fgetc(fp);
        ungetc(next_ch, fp);

        /* Update the line count if we've reached the end of a line */
        if (ch == '\n')
            stats->lines++;

        /* Update the word count if we've reached the end of a word */
        if (!isspace(ch) && (isspace(next_ch) || feof(fp)))
            stats->words++;

        /* Increment the character count */
        stats->chars++;

        /* Read the next character */
        ch = fgetc(fp);
    }

    /* Close the file and exit the thread */
    fclose(fp);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i, total_lines = 0, total_words = 0, total_chars = 0;
    file_stats_t *file_stats;
    pthread_t *threads;

    /* Check that at least one filename was provided */
    if (argc < 2) {
        return 1;
    }

    /* Allocate memory for the file statistics and thread IDs */
    file_stats = (file_stats_t *)malloc((argc - 1) * sizeof(file_stats_t));
    if (file_stats == NULL) {
        return 1;
    }
    threads = (pthread_t *)malloc((argc - 1) * sizeof(pthread_t));
    if (threads == NULL) {
        free(file_stats);
        return 1;
    }

    /* Start a thread for each filename provided */
    for (i = 1; i < argc; i++) {
        /* Copy the filename into the file_stats struct */
        strncpy(file_stats[i - 1].filename, argv[i], MAX_FILENAME_LEN);

        /* Start a new thread to process the file */
        pthread_create(&threads[i - 1], NULL, process_file,
                       (void *)&file_stats[i - 1]);
    }

    /* Wait for each thread to finish and accumulate the total statistics
     */
    for (i = 1; i < argc; i++) {
        pthread_join(threads[i - 1], NULL);
        total_lines += file_stats[i - 1].lines;
        total_words += file_stats[i - 1].words;
        total_chars += file_stats[i - 1].chars;
    }

    /* Print the total statistics for all files */
    printf("%4d %4d %4d\n", total_lines, total_words, total_chars);

    /* Free the memory allocated for the file statistics and thread IDs */
    free(file_stats);
    free(threads);

    return 0;
}
