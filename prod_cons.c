#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "shared_memory.h"

usim_info buffer[MAX_RECORDS];
int in = 0, out = 0, count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void* process_watcher(void* arg) {
    while (1) {
        FILE *fp = popen("ps -eo pid,comm", "r");
        if (!fp) {
            perror("popen failed");
            return NULL;
        }

        char line[512];
        int skip = 1;

        while (fgets(line, sizeof(line), fp)) {
            if (skip) { skip = 0; continue; }

            int pid;
            char pname[256];
            sscanf(line, "%d %255s", &pid, pname);

            // 예외 처리: tracker-miner-f는 제외
            if ((strstr(pname, "hack") || strstr(pname, "miner") || strstr(pname, "malware")) &&
                strcmp(pname, "tracker-miner-f") != 0) {
                printf("🚨 [Watcher] Suspicious Process Detected: PID=%d, NAME=%s\n", pid, pname);
            }
        }

        pclose(fp);
        sleep(5);  // 5초마다 감시
    }
    return NULL;
}

void* producer(void* arg) {
    for (int i = 0; i < MAX_RECORDS; i++) {
        usim_info item;
        snprintf(item.usim_id, sizeof(item.usim_id), "usim_%d", i);
        snprintf(item.key, sizeof(item.key), "key_value_%d", i);

        pthread_mutex_lock(&mutex);

        while (count == MAX_RECORDS)
            pthread_cond_wait(&not_full, &mutex);

        buffer[in] = item;
        in = (in + 1) % MAX_RECORDS;
        count++;

        printf("Produced: %s | %s\n", item.usim_id, item.key);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(2);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < MAX_RECORDS; i++) {
        pthread_mutex_lock(&mutex);

        while (count == 0)
            pthread_cond_wait(&not_empty, &mutex);

        usim_info item = buffer[out];
        out = (out + 1) % MAX_RECORDS;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        printf("Consumed: %s | %s\n", item.usim_id, item.key);

        // 🚨 감시 기능
        if (strcmp(item.usim_id, "usim_3") == 0)
            printf("🚨 ALERT: Suspicious USIM ID detected: %s\n", item.usim_id);

        if (strstr(item.key, "value_3") != NULL)
            printf("🚨 ALERT: Suspicious Key detected: %s\n", item.key);

        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t prod_tid, cons_tid, watcher_tid;

    pthread_create(&prod_tid, NULL, producer, NULL);
    pthread_create(&cons_tid, NULL, consumer, NULL);
    pthread_create(&watcher_tid, NULL, process_watcher, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    // watcher는 무한 루프이므로 종료 로직 필요
    pthread_cancel(watcher_tid);
    pthread_join(watcher_tid, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}
