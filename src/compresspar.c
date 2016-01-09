#include <stdlib.h>
#include <sys/stat.h>

#include "bspedupack.h"
#include "bsp-aux.h"

int P;
char *name;
int m;
int n;

static int get_file_size(char *path);

static int get_file_size(char *path) {
    struct stat st;
    int ret = stat(path, &st);

    if (ret < 0) {
        return -1;
    } else {
        return st.st_size;
    }
}

void bsp_compresssimple() {
    // Initialize parallel part
    int p, s;
    bsp_begin(P);
    p = bsp_nprocs(); /* p = number of processors obtained */
    s = bsp_pid();    /* s = processor number */
    // Get file size
    int file_size = get_file_size(name);

    // Print input
    if (s == 0) {
        printf("\nName: %s\nWindow length: %d\nLook ahead length: %d\n", name,
               m, n);
        printf("Number of processors: %d\n", P);
        printf("File size: %d bytes\n\n", file_size);
    }

    if (file_size < 0) {
        bsp_abort("Error: file size could not be determined.\n");
    }

    int n_0 = block_distr_start(p, file_size, s); // index of start of interval
    int l   = block_distr_len(p, file_size, s);     // length of own interval

    // Put all characters from the file in array all_characters
    //(initial m are reserved for window of previous processor)
    int BLOCK_SIZE = ceil((double)file_size / p);
    char all_characters[m + BLOCK_SIZE];
    FILE *fp;
    fp = fopen(name, "r");
    char kar;
    fseek(fp, s * BLOCK_SIZE, SEEK_SET);
    fscanf(fp, "%c", &kar);
    int counter = 0;
    while (counter < BLOCK_SIZE) {
        if (file_size < 1000)
            printf("%d: %c ", s, kar);
        all_characters[m + counter] = kar;
        counter++;
        fscanf(fp, "%c", &kar);
    } // while
    if (s == p - 1)
        all_characters[m + BLOCK_SIZE - 1] = -1;
    printf("\n");

    // Register character array for exchange between processor
    //(currently the entire window is registered...)
    bsp_push_reg(all_characters, BLOCK_SIZE);
    bsp_sync();

    // Exchange the characters
    if (s != p - 1)
        bsp_put(s + 1, &all_characters[BLOCK_SIZE], &all_characters[0], 0, m);
    bsp_sync();

    // Window values
    int window = m;     // m
    int look_ahead = n; // n

    // Start encoding
    int windowi = m;
    int i = 0;
    int j = 0;
    for (i = m; i < BLOCK_SIZE + m; i++) { // iterate over all characters
        if ((s == 0 && i - window >= m) || (s != 0 && i - window >= 0))
            windowi = i - window; // Set starting index of window
        int longestmatch = 0;     // length of the matching
        int goback = 0;

        for (j = windowi; j < i - longestmatch; j++) { // iterate over window
            int k = 0;
            if (all_characters[j] ==
                all_characters[i]) { // start of possible matching

                while (i + k < BLOCK_SIZE + m &&
                       k < look_ahead // determine length of matching
                       && all_characters[j + k] == all_characters[i + k]) {
                    k++;
                } // while

                if (k > longestmatch) { // update information on longest
                                        // matching
                    goback = i - j;
                    longestmatch = k;
                } // if
                j = j + k;
            } // if
        }     // for
        i = i + longestmatch;
        printf("%d: (%d,%d,%d)\n", s, goback, longestmatch,
               (int)all_characters[i]);
    } // for

    bsp_pop_reg(all_characters);
    bsp_end();

} // compress

static const char *usage =
    "Usage: parallel_lzw [path] [window size] [lookahead] [n_cores (P)]";

int main(int argc, char *argv[]) {
    bsp_init(bsp_compresssimple, argc, argv);

    name = "";

    if (argc != 5) {
        printf("Error in input: not enough arguments supplied.\n");
        printf("%s\n", usage);
        exit(1);
    }
    name = argv[1];
    m = atoi(argv[2]);
    n = atoi(argv[3]);
    P = atoi(argv[4]);

    if (P > bsp_nprocs()) {
        printf("Sorry, not enough processors available.\n");
        exit(1);
    }

    char choice;
    printf("(C)oderen of (D)ecoderen\n");
    scanf("%c", &choice);
    if (choice == 'C' || choice == 'c') {
        bsp_compresssimple();
    }

    exit(0);

} /* end main */
