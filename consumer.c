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
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }

    ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }

    for (int i = 0; i < MAX_RECORDS; i++) {
        printf("Consumed: %s | %s\n", ptr[i].usim_id, ptr[i].key);

        // 🚨 검출 기능 추가
        if (strcmp(ptr[i].usim_id, "usim_3") == 0) {
            printf("🚨 ALERT: Suspicious USIM ID detected: %s\n", ptr[i].usim_id);
        }

        if (strstr(ptr[i].key, "value_3") != NULL) {
            printf("🚨 ALERT: Suspicious Key detected: %s\n", ptr[i].key);
        }

        sleep(2); // 2초마다 읽기
    }

    munmap(ptr, SHM_SIZE);
    close(shm_fd);

    shm_unlink(SHM_NAME);

    return 0;
}
