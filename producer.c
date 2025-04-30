#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "shared_memory.h"

int main() {
    int shm_fd;
    usim_info *ptr;

    // 공유 메모리 열기
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }
    if (ftruncate(shm_fd, SHM_SIZE) == -1) { perror("ftruncate"); exit(1); }

    // 메모리 매핑
    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }

    // 5개 데이터 차례로 저장
    for (int i = 0; i < MAX_RECORDS; i++) {
        snprintf(ptr[i].usim_id, sizeof(ptr[i].usim_id), "usim_%d", i);
        snprintf(ptr[i].key, sizeof(ptr[i].key), "key_value_%d", i);
        printf("Produced: %s | %s\n", ptr[i].usim_id, ptr[i].key);
        sleep(2); // 2초마다 쓰기
    }

    munmap(ptr, SHM_SIZE);
    close(shm_fd);

    return 0;
}
