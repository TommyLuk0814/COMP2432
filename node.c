
#include <stdio.h>
#include <stdlib.h>



typedef struct {
    char member[9];
    char date[11]; // YYYY-MM-DD
    char time[6];  // hh:mm
    float duration;
    int priority;    //4 = event / 3 = reservation / 2 = parking / 1 = essentials
    int parking_space;
    int battery;
    int cable;
    int locker;
    int umbrella;
    int valet;
    int inflation;
} Booking;



typedef struct Node{
    Booking booking;
    struct Node *next;

} Node;

