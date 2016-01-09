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

/*
   get_file_block reads a block from disk and stores it at the
   location given by *dest.
*/
void get_file_block(char *path, int BLOCK_START, int BLOCK_SIZE, void *dest) {
    FILE *fp;
    if ((fp = fopen(name, "r")) == NULL) {
        bsp_abort("Error: could not open file");
    }
    if (fseek(fp, BLOCK_START, SEEK_SET) == -1) {
        bsp_abort("Error: could not seek in file");
    }
    if (fread(dest, sizeof(char), BLOCK_SIZE, fp) != BLOCK_SIZE) {
        bsp_abort("Error: could not read file");
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

    // Put all characters from the file in array all_characters
    //(initial m are reserved for window of previous processor)
    int BLOCK_START = block_distr_start(p, file_size, s);
    int BLOCK_SIZE = block_distr_len(p, file_size, s);
    char *all_characters = vecallocc(m + BLOCK_SIZE);

    get_file_block(name, BLOCK_START, BLOCK_SIZE, all_characters + m);

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

    if (scanf("%c", &choice) == 1 && (choice == 'C' || choice == 'c')) {
        bsp_compresssimple();
    }

    exit(0);
} /* end main */
