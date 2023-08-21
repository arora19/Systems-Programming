#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;


//to see if any cache is created or destroyed
int storage = 0; 

int cache_create(int num_entries) {
  if (num_entries < 2 || num_entries > 4096){
    return -1;
  }
  if (storage == 0){
    cache = calloc(num_entries, sizeof(cache_entry_t));  // dynamically allocate space for cache
    cache_size = num_entries;
    storage = 1;
    return 1;
  }
  return -1;
}
int inserted = 0;
int cache_destroy(void) {
  if (storage == 1){
    free(cache);
    cache = NULL;
    cache_size = 0;
    storage = 0;
    inserted = 0;
    clock = 0;
    return 1;
  }
  return -1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if (inserted == 0){ // checks if anything has been inserted
    return -1;
  }
  num_queries++;
  for (int i = 0; i < cache_size; i++){
    if (cache[i].disk_num == disk_num && cache[i].block_num == block_num && buf != NULL){
      num_hits++;
      clock++;
      cache[i].access_time = clock;
      memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
      return 1;     
    }
  }
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  for (int i = 0; i < cache_size; i++){
    if (cache[i].disk_num == disk_num && cache[i].block_num == block_num ){
      memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE); // updates data in cache
      clock++;
      cache[i].access_time = clock;
    }
  }
}
int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  int i = -1;
  int temp;
  // check to see if there are invalid parameters
  if (storage == 0 || buf == NULL || cache_size == 0){
    return -1;
  }
  if (disk_num > 16 || disk_num < 0 || block_num > 256 || block_num < 0){
    return -1;
  }
  inserted = 1;
  
  for (int i = 0; i < cache_size; i++){
    if (cache[i].disk_num == disk_num && cache[i].block_num == block_num && block_num != 0 && disk_num != 0){ 
    return -1;
    }
    if (cache[i].valid == 0){
      i = i;
      break;
    }
  }
  // no room in cache
  if (i == -1){
    temp = cache[0].access_time;
    i = 0;
    for (int i = 1; i < cache_size; i++){
      if (cache[i].access_time < temp){
        temp = cache[i].access_time;
        i = i;
      }
    }
  }
  
  
  // copy buf into block 
  memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE); 
  
  // to update disk_num and block_num
  cache[i].disk_num = disk_num; 
  cache[i].block_num = block_num;
  
  cache[i].valid = 1; 
  clock++;
  cache[i].access_time = clock;
  return 1;
}

bool cache_enabled(void) {
  return false;
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
