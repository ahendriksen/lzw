/*
   Auxilliary functions for BSP

*/
#include <stdint.h>
#include "bspedupack.h"
#include "bsp-aux.h"

void slow_sync(int origin_proc, void *var, size_t size) {
    bsp_push_reg(var, size);
    bsp_sync();
    bsp_get(origin_proc, var, 0, var, size);
    bsp_sync();
    bsp_pop_reg(var);
}

void slow_sync_0(void *var, size_t size) {
    slow_sync(0, var, size);
}

int block_distr_start(int p, int n, int s) {
    // use long ints to prevent overflow in the multiplication below.
    uint64_t pl = p;
    uint64_t nl = n;
    uint64_t sl = s;

    return (int)((sl * nl) / pl);
}

int block_distr_len(int p, int n, int s) {
    // return the difference between the start of the current block and
    // the start of the next block
    int start = block_distr_start(p, n, s);
    int end = block_distr_start(p, n, s + 1);

    return end - start;
}
