#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define SHM_NAME "/usim_shm"
#define SHM_SIZE 4096
#define MAX_RECORDS 5 // 데이터 5개 저장

typedef struct {
    char usim_id[32];
    char key[64];
} usim_info;

#endif
