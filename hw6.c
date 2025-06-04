#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NORTH 0
#define SOUTH 1

pthread_mutex_t bridge_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_north = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_south = PTHREAD_COND_INITIALIZER;

int current_direction = -1;  // -1 یعنی خالی
int on_bridge = 0;
int waiting_north = 0;
int waiting_south = 0;

void enter_bridge(int direction) {
    pthread_mutex_lock(&bridge_mutex);

    if (direction == NORTH) waiting_north++;
    else waiting_south++;

    while ((on_bridge > 0 && current_direction != direction)) {
        if (direction == NORTH)
            pthread_cond_wait(&cond_north, &bridge_mutex);
        else
            pthread_cond_wait(&cond_south, &bridge_mutex);
    }

    if (direction == NORTH) waiting_north--;
    else waiting_south--;

    current_direction = direction;
    on_bridge++;

    printf(" Farmer from %s ENTERED the bridge. [On bridge: %d]\n",
           direction == NORTH ? "NORTH" : "SOUTH", on_bridge);

    pthread_mutex_unlock(&bridge_mutex);
}

void exit_bridge(int direction) {
    pthread_mutex_lock(&bridge_mutex);

    on_bridge--;

    printf(" Farmer from %s EXITED the bridge. [Remaining: %d]\n",
           direction == NORTH ? "NORTH" : "SOUTH", on_bridge);

    if (on_bridge == 0) {
        // پل خالی شده؛ می‌تونیم جهت رو عوض کنیم
        if (direction == NORTH && waiting_south > 0)
            current_direction = SOUTH;
        else if (direction == SOUTH && waiting_north > 0)
            current_direction = NORTH;
        else
            current_direction = -1;
        
        // بیدار کردن کشاورزان جهت جدید
        if (current_direction == NORTH)
            pthread_cond_broadcast(&cond_north);
        else if (current_direction == SOUTH)
            pthread_cond_broadcast(&cond_south);
    }

    pthread_mutex_unlock(&bridge_mutex);
}

void* farmer(void* arg) {
    int direction = *(int*)arg;

    enter_bridge(direction);

    // عبور از پل: شبیه‌سازی با sleep تصادفی
    int crossing_time = rand() % 3 + 1;
    printf(" Farmer from %s is CROSSING the bridge (sleep %d sec)...\n",
           direction == NORTH ? "NORTH" : "SOUTH", crossing_time);
    sleep(crossing_time);

    exit_bridge(direction);

    return NULL;
}

int main() {
    srand(time(NULL));

    int total_farmers = 10;
    pthread_t threads[total_farmers];
    int directions[total_farmers];

    for (int i = 0; i < total_farmers; i++) {
        directions[i] = rand() % 2;  
        pthread_create(&threads[i], NULL, farmer, &directions[i]);
        sleep(1); // ایجاد فاصله بین کشاورزان
    }

    for (int i = 0; i < total_farmers; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
