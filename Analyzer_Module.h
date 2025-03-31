#include <stdio.h>
#include "node.h"

#define MAX_BATTERIES 3
#define MAX_CABLES 3
#define MAX_LOCKERS 3
#define MAX_UMBRELLAS 3
#define MAX_VALETS 3
#define MAX_INFLATIONS 3
#define TESTING_DAY 7
#define TIME_SLOT_PER_DAY 24

void gen_report(FILE* report, Node* booking, Node* accepted, Node* rejected, int invalid_requests);
void count_resources(Node* head, int* battery, int* cable, int* locker, int* umbrella, int* valet, int* inflation);
void count_max_resources(int* battery, int* cable, int* locker, int* umbrella, int* valet, int* inflation);
int list_length(Node* list);

void gen_report(FILE* report, Node* booking, Node* accepted, Node* rejected, int invalid_requests) {
    int booking_num = list_length(booking);
    int request_num = booking_num + invalid_requests;
    int battery, cable, locker, umbrella, valet, inflation, max_battery, max_cable, max_locker, max_umbrella, max_valet, max_inflation;
    count_resources(accepted, &battery, &cable, &locker, &umbrella, &valet, &inflation);
    count_max_resources(&max_battery, &max_cable, &max_locker, &max_umbrella, &max_valet, &max_inflation);

    fprintf(report, " \t\tTotal Number of Bookings Received: %d (%.1f%%)\n", booking_num, (float)booking_num/request_num*100);
    fprintf(report, " \t\t\t  Number of Bookings Assigned: %d (%.1f%%)\n", list_length(accepted),(float)list_length(accepted)/booking_num*100);
    fprintf(report, " \t\t\t  Number of Bookings Rejected: %d (%.1f%%)\n",list_length(rejected) ,(float)list_length(rejected)/booking_num*100);
    fprintf(report, " \t\tUtilization of Time Slot:\n");
    fprintf(report, " \t\t\t Battery   - %.1f%%\n", (float)battery/max_battery*100);
    fprintf(report, " \t\t\t Cable     - %.1f%%\n", (float)cable/max_cable*100);
    fprintf(report, " \t\t\t Locker    - %.1f%%\n", (float)locker/max_locker*100);
    fprintf(report, " \t\t\t Umbrella  - %.1f%%\n", (float)umbrella/max_umbrella*100);
    fprintf(report, " \t\t\t Valet     - %.1f%%\n", (float)valet/max_valet*100);
    fprintf(report, " \t\t\t Inflation - %.1f%%\n", (float)inflation/max_inflation*100);
    fprintf(report, "\n \t\tInvalid request(s) made: %d\n", invalid_requests);
}

void count_resources(Node* head, int* battery, int* cable, int* locker, int* umbrella, int* valet, int* inflation) {
    //Count resources total duration in accepted
    *battery = *cable = *locker = *umbrella = *valet = *inflation = 0;
    Node* current = head;
    while (current != NULL) {
        if (current->booking.battery) (*battery) += current->booking.duration;
        if (current->booking.cable) (*cable) += current->booking.duration;
        if (current->booking.locker) (*locker) += current->booking.duration;
        if (current->booking.umbrella) (*umbrella) += current->booking.duration;
        if (current->booking.valet) (*valet) += current->booking.duration;
        if (current->booking.inflation) (*inflation) += current->booking.duration;
        current = current->next;
    }
}

void count_max_resources(int* battery, int* cable, int* locker, int* umbrella, int* valet, int* inflation) {
    //Count resources total time slot
    *battery = MAX_BATTERIES*TESTING_DAY*TIME_SLOT_PER_DAY;
    *cable = MAX_CABLES*TESTING_DAY*TIME_SLOT_PER_DAY;
    *locker = MAX_LOCKERS*TESTING_DAY*TIME_SLOT_PER_DAY;
    *umbrella = MAX_UMBRELLAS*TESTING_DAY*TIME_SLOT_PER_DAY;
    *valet = MAX_VALETS*TESTING_DAY*TIME_SLOT_PER_DAY;
    *inflation = MAX_INFLATIONS*TESTING_DAY*TIME_SLOT_PER_DAY;
}

int list_length(Node* list) {
    //Count the length of linkedlist
    int count = 0;
    Node* current = list;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}