#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_KEY 1234
#define MSG_TYPE_REQUEST 1
#define MSG_TYPE_RESPONSE 2
#define MSG_TYPE_TOTAL_SUM 3   
#define MAX_RESERVATIONS 10

struct ReservationMsg {
    long msg_type;
    char name[50];
    int table_number;
    char date[20];
    int num_people;
    float paid_amount;
};

struct TotalSumMsg {
    long msg_type;
    float total_sum;
};

struct ReservationMsg reservations[MAX_RESERVATIONS];
int current_reservation = 0;

void show_statistics() {
    printf("\n--- Статистика на резервациите ---\n");
    float total_sum = 0;
    for (int i = 0; i < current_reservation; i++) {
        printf("Резервация %d: %s, Маса: %d, Дата: %s, Брой хора: %d, Платено: %.2f\n",
               i + 1, reservations[i].name, reservations[i].table_number,
               reservations[i].date, reservations[i].num_people, reservations[i].paid_amount);
        total_sum += reservations[i].paid_amount;
    }
    printf("Обща сума от всички резервации: %.2f\n", total_sum);
}

float calculate_total_sum() {
    float total = 0;
    for (int i = 0; i < current_reservation; i++) {
        total += reservations[i].paid_amount;
    }
    return total;
}

int main() {
    int msgid;
    struct ReservationMsg reservation;
    struct TotalSumMsg total_msg;

    msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Сървърът е стартиран и чака резервации...\n");

    while (1) {
        if (msgrcv(msgid, &reservation, sizeof(struct ReservationMsg) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        if (reservation.msg_type == MSG_TYPE_REQUEST) {
            if (current_reservation < MAX_RESERVATIONS) {
                reservations[current_reservation] = reservation;
                current_reservation++;

                printf("Получена резервация: %s, Масата: %d, Дата: %s, Брой хора: %d, Платено: %.2f\n",
                       reservation.name, reservation.table_number, reservation.date,
                       reservation.num_people, reservation.paid_amount);
                show_statistics();
            } else {
                printf("Няма свободни места за резервации!\n");
            }

            reservation.msg_type = MSG_TYPE_RESPONSE;
            if (msgsnd(msgid, &reservation, sizeof(struct ReservationMsg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
        } 
        else if (reservation.msg_type == MSG_TYPE_TOTAL_SUM) {
            total_msg.msg_type = MSG_TYPE_RESPONSE;
            total_msg.total_sum = calculate_total_sum();

            if (msgsnd(msgid, &total_msg, sizeof(struct TotalSumMsg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
        }
    }

    return 0;
}
