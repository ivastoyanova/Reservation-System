#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MSG_KEY 1234
#define MSG_TYPE_REQUEST 1
#define MSG_TYPE_RESPONSE 2
#define MSG_TYPE_TOTAL_SUM 3 

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

int main() {
    int msgid;
    struct ReservationMsg reservation;
    struct TotalSumMsg total_msg;
    int choice;

    msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }
    printf("Изберете опция:\n1. Направи резервация\n2. Изискай крайна сметка\n");
    scanf("%d", &choice);
    getchar();  
    
  if (choice==1){
   
    printf("Въведете името на клиента: ");
    fgets(reservation.name, sizeof(reservation.name), stdin);
    reservation.name[strcspn(reservation.name, "\n")] = 0; 
    
    printf("Въведете номер на маса: ");
    scanf("%d", &reservation.table_number);
    getchar();  

    printf("Въведете дата на резервацията (например 2025-04-01): ");
    fgets(reservation.date, sizeof(reservation.date), stdin);
    reservation.date[strcspn(reservation.date, "\n")] = 0; 

    printf("Въведете брой хора: ");
    scanf("%d", &reservation.num_people);

    printf("Въведете платен куверт: ");
    scanf("%f", &reservation.paid_amount);

    reservation.msg_type = MSG_TYPE_REQUEST;
    if (msgsnd(msgid, &reservation, sizeof(struct ReservationMsg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Резервацията е изпратена към сървъра.\n");

    if (msgrcv(msgid, &reservation, sizeof(struct ReservationMsg) - sizeof(long), MSG_TYPE_RESPONSE, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("Получен отговор от сървъра: Резервацията е приета.\n");
    } else if (choice == 2) {
  
        total_msg.msg_type = MSG_TYPE_TOTAL_SUM;

        if (msgsnd(msgid, &total_msg, sizeof(struct TotalSumMsg) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        if (msgrcv(msgid, &total_msg, sizeof(struct TotalSumMsg) - sizeof(long), MSG_TYPE_RESPONSE, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("Обща сума от всички резервации: %.2f\n", total_msg.total_sum);
    }

    return 0;

}
