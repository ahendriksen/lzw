#pragma once

void slow_sync  (int origin_proc, void *var, size_t size);
void slow_sync_0(void *var, size_t size);

int block_distr_start(int p, int n, int s);
int block_distr_len(int p, int n, int s);

char *vecallocc(int n);
