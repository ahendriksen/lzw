#include <stdio.h>
#include <stdlib.h>
#include "lzw.h"

static inline int min(int a, int b)
{
    return (a < b) ? a : b;
}

/* compress_buffer

   arguments:
   - buffer: input buffer;
   - length: length of the input buffer;
   - start: start compression at index start;
   - end: stop compressing before end
   (strictly speaking: the last triple should not start with the char at end)
   - output: buffer for output triples. Should have length equal to buffer.

   returns:
   an int with the last index used for compression.
*/
int compress_buffer(char *buffer, int length, int start, int end,
                    struct triple *output, FILE *file_out)
{
    int i, j;             // i is index into buffer, j into output
    struct triple best_t; // contains the best triple for each i
    int l, o;             // contains length, offset running variables

    int max_length, max_offset;
    j = 0;
    for (i = start; i < end; i += best_t.length + 1) {
        best_t.offset = 0;
        best_t.length = 0;
        best_t.c = buffer[i];

        // for max_offset we must not reach in front of the buffer
        // outside of MAX_LOOKBACK
        max_offset = min(i, MAX_LOOKBACK);
        for (o = max_offset; best_t.length < o; o--) {
            // length cannot exceed offset.
            // Also, the last triple must contain buffer[length - 1] as
            // character.
            max_length = min(o, length - i - 1);
            for (l = 0; l < max_length; l++) {
                if (buffer[i + l] != buffer[i - o + l]) {
                    break;
                }
            }
            if (best_t.length < l) {
                best_t.offset = o;
                best_t.length = l;
                best_t.c = buffer[i + l];
            }
        }

        // best_t now contains the best triple for this index i
        printf("(%d,%d,%c)\n", best_t.offset, best_t.length, best_t.c);
        // store it
        output[j++] = best_t;
    }

    // write the triples
    if (file_out != NULL) {
        if (fwrite(output, sizeof(output[0]), j, file_out) != j) {
            fprintf(stderr, "Could not write to output file\n");
            exit(3);
        }
    }
    return i;
}

void compress_mathe(int s, char *all_characters, int BLOCK_SIZE, int m, int n)
{
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
        printf("%d: (%d,%d,'%c')\n", s, goback, longestmatch,
               all_characters[i]);
    } // for
}
