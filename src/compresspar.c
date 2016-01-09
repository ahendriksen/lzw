#include <stdlib.h>
#include <sys/stat.h>

#include "bspedupack.h"
#include "bsp-aux.h"
#include "lzw.h"

int P;
char *name;
int m;
int n;

static int get_file_size(char *path);

static int get_file_size(char *path)
{
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
void get_file_block(char *path, int BLOCK_START, int BLOCK_SIZE, void *dest)
{
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

static void print_diagnostics(char *name, int m, int n, int file_size)
{
    // Print input
    printf("\nName: %s\nWindow length: %d\nLook ahead length: %d\n", name, m,
           n);
    printf("Number of processors: %d\n", P);
    printf("File size: %d bytes\n\n", file_size);
}

void bsp_compresssimple()
{
    // Initialize parallel part
    bsp_begin(P);
    int p = bsp_nprocs(); /* p = number of processors obtained */
    int s = bsp_pid();    /* s = processor number */
    // Get file size
    int file_size = get_file_size(name);
    if (s == 0) {
        print_diagnostics(name, m, n, file_size);
    }
    if (file_size < 0) {
        bsp_abort("Error: file size could not be determined.\n");
    }

    // Put all characters from the file in array all_characters
    //(initial m are reserved for window of previous processor)
    int BLOCK_START = block_distr_start(p, file_size, s);
    int BLOCK_SIZE = block_distr_len(p, file_size, s);
    char *all_characters = vecallocc(m + BLOCK_SIZE);

    if (BLOCK_SIZE < m) {
        bsp_abort("Error: block size too small for window size");
    }
    // load block from disk
    get_file_block(name, BLOCK_START, BLOCK_SIZE, all_characters + m);

    // Copy the last m bytes from processor s to the start of
    // all_characters at processor s + 1
    bsp_push_reg(all_characters, BLOCK_SIZE + m);
    bsp_sync();
    if (s != p - 1) {
        bsp_put(s + 1, all_characters + BLOCK_SIZE, all_characters, 0, m);
    }
    bsp_sync();

    struct triple *output =
        (struct triple *)vecallocc(BLOCK_SIZE * sizeof(struct triple));
    compress_buffer(all_characters, // input buffer
                    m + BLOCK_SIZE, // buffer size
                    m,              // start
                    m + BLOCK_SIZE, // end
                    output,         // output buffer
                    NULL);          // output file handle

    // make sure that the output of the two approaches is split
    bsp_sync();
    compress_mathe(s, all_characters, BLOCK_SIZE, m, n);

    bsp_pop_reg(all_characters);
    free(all_characters);
    bsp_end();

} // compress

static const char *usage =
    "Usage: parallel_lzw [path] [window size] [lookahead] [n_cores (P)]";

int main(int argc, char *argv[])
{
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

    bsp_compresssimple();

    exit(0);
} /* end main */
