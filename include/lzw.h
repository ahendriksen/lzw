#pragma once


#define K            1024
#define M            (K*K)


// MAX_LOOKBACK is the maximum number of chars that the lzw algorithm
// is allowed to look back. 
// WINDOWSZ is the buffer window that is loaded into memory at once. 
// BUFSZ takes into account the margins that have to be loaded as well. 

#define MAX_LOOKBACK (512)
#define WINDOWSZ     (4*K)
#define BUFSZ	     (WINDOWSZ + 2 * MAX_LOOKBACK)

struct triple{
  int offset;
  int length;
  char c;
};


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
		    struct triple *output, FILE *file_out);


void compress_mathe(int s, char *all_characters, int BLOCK_SIZE, int m, int n);
