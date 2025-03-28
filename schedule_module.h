#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "node.h"

#define MAX_MEMBERS 5
#define MAX_PARKING_SPACES 10
#define MAX_BATTERIES 3
#define MAX_CABLES 3
#define MAX_LOCKERS 3
#define MAX_UMBRELLAS 3
#define MAX_VALETS 3
#define MAX_INFLATIONS 3
#define TESTING_DAY 7
#define TIME_SLOT_PER_DAY 24
#define RESOURCE_NUM 7

enum ResourceType {
    SPACE = 0,
    BATTERY,
    CABLE,
    LOCKER,
    UMBRELLA,
    VALET,
    INFLATION
};

const char* TEST_START_DATE = "2025-05-10";

int resource_pipes_ptc[RESOURCE_NUM][2];
int resource_pipes_ctp[RESOURCE_NUM][2];
pid_t child_pids[RESOURCE_NUM];

Node* create_node(Booking booking);
void append_node(Node **head, Booking booking);
void free_list(Node *head);
void create_resource_managers();
void resource_manager(int resource_type);
void cleanup_child_processes();
int date_to_day_index(const char* date);
void print_bookings_fcfs(Node* head, Node* accepted, Node* rejected);
void print_bookings_priority(Node* head, Node* accepted, Node* rejected);

Node* create_node(Booking booking) {
    //Create new node for linked list
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->booking = booking;
    new_node->next = NULL;
    return new_node;
}

void append_node(Node **head, Booking booking) {
    //Append node to linked list
    Node *new_node = create_node(booking);
    
    if (*head == NULL) {
        *head = new_node;
    } else {
        Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void free_list(Node *head) {
    //Free linked list
    while (head != NULL) {
        Node *temp = head;
        head = head->next;
        free(temp);
    }
}

void create_resource_managers() {
    //Create pipes and fork child processes for each resource type
    for (int i = 0; i < RESOURCE_NUM; i++) {
        if (pipe(resource_pipes_ptc[i]) == -1) {
            perror("pipe");
        }
        if (pipe(resource_pipes_ctp[i]) == -1) {
            perror("pipe");
        }

        child_pids[i] = fork();
        if (child_pids[i] == -1) {
            perror("fork");
        }

        if (child_pids[i] == 0) {
            //Child process
            close(resource_pipes_ptc[i][1]);    //Close parent to child write end
            close(resource_pipes_ctp[i][0]);    //Close child to parent read end
            
            //Each child handles one resource type
            resource_manager(i);
            exit(0);
        } else {
            //Parent process
            close(resource_pipes_ptc[i][0]);    //Close parent to child read end
            close(resource_pipes_ctp[i][1]);    //Close child to parent write end
        }
    }
}

void resource_manager(int resource_type) {
    int max_resource_num[RESOURCE_NUM] = {MAX_PARKING_SPACES, MAX_BATTERIES, MAX_CABLES, MAX_LOCKERS, MAX_UMBRELLAS, MAX_VALETS, MAX_INFLATIONS};
    int resource_time_slot[max_resource_num[resource_type]][TESTING_DAY][TIME_SLOT_PER_DAY];

    //Initialize_time_slot
    for (int resource_id = 0; resource_id < max_resource_num[resource_type]; resource_id++) {
        for (int day = 0; day < TESTING_DAY; day++) {
            for (int time_slot = 0; time_slot < TIME_SLOT_PER_DAY; time_slot++) {
                resource_time_slot[resource_id][day][time_slot] = -1;
            }
        }
    }

    while (1) {
        int start_day, end_day, start_hour, end_hour;
        int resource_id;
        int response = 0;
        int schedule = 0;

        //Read request from parent
        if (read(resource_pipes_ptc[resource_type][0], &start_day, sizeof(int)) <= 0) break;
        if (read(resource_pipes_ptc[resource_type][0], &end_day, sizeof(int)) <= 0) break;
        if (read(resource_pipes_ptc[resource_type][0], &start_hour, sizeof(int)) <= 0) break;
        if (read(resource_pipes_ptc[resource_type][0], &end_hour, sizeof(int)) <= 0) break;

        // Check availability based on resource type
        for (resource_id = 0; resource_id < max_resource_num[resource_type]; resource_id++) {
            int available = 1;
            for (int day = start_day; day <= end_day; day++) {
                int start_time_slot = start_hour;
                int end_time_slot = end_hour;
    
                //If not start day, the time slot is start from 0
                if (day != start_day) {
                    start_time_slot = 0;
                }
    
                //If not end day, the time slot is end by last time slot
                if (day != end_day) {
                    end_time_slot = TIME_SLOT_PER_DAY - 1;
                }
    
                for (int time_slot = start_time_slot; time_slot <= end_time_slot; time_slot++) {
                    if (resource_time_slot[resource_id][day][time_slot] != -1) {
                        //The time slot is not available
                        available = 0;
                        break;
                    }
                }
                if (!available) {
                    //The time slot is not available
                    break;
                }
            }
            if (available) {
                //The time slot is available
                response = 1;
            }
        }
        // Send response back to parent
        write(resource_pipes_ctp[resource_type][1], &response, sizeof(int));

        //Read schedule request
        if (read(resource_pipes_ptc[resource_type][0], &schedule, sizeof(int)) <= 0) break;

        if (schedule) {
            for (int day = start_day; day <= end_day; day++) {
                int start_time_slot = start_hour;
                int end_time_slot = end_hour;

                //If not start day, the time slot is start from 0
                if (day != start_day) {
                    start_time_slot = 0;
                }

                //If not end day, the time slot is end by last time slot
                if (day != end_day) {
                    end_time_slot = TIME_SLOT_PER_DAY - 1;
                }

                for (int time_slot = start_time_slot; time_slot <= end_time_slot; time_slot++) {
                    resource_time_slot[resource_id][day][time_slot] = 1;
                }
            }
        }
    }
    close(resource_pipes_ptc[resource_type][0]);    //Close parent to child read end
    close(resource_pipes_ctp[resource_type][1]);    //Close child to parent write end
    exit(0);
}

void cleanup_child_processes() {
    for (int i = 0; i < RESOURCE_NUM; i++) {
        close(resource_pipes_ptc[i][1]);    //Close parent's write end
        if (kill(child_pids[i], SIGTERM) == -1) {   //Send termination signal
            perror("kill");
        }
        waitpid(child_pids[i], NULL, 0);    //Wait for child to exit
    }
}

int date_to_day_index(const char* date) {
    //Parse the date to day index of time slot
    struct tm start_tm = {0};
    struct tm current_tm = {0};
    
    //Parse TEST_START_DATE
    if (strptime(TEST_START_DATE, "%Y-%m-%d", &start_tm) == NULL) {
        return -1; // Invalid start date format
    }
    time_t start_time = mktime(&start_tm);
    
    //Parse input date
    if (strptime(date, "%Y-%m-%d", &current_tm) == NULL) {
        return -1; //Invalid date format
    }
    time_t current_time = mktime(&current_tm);
    
    //Calculate difference in seconds
    double seconds = difftime(current_time, start_time);
    
    //Convert to days (86400 seconds per day)
    int days = (int)(seconds / 86400);
    
    return (days >= 0) ? days : -1;
}

void print_bookings_fcfs(Node* head, Node* accepted, Node* rejected) {
    create_resource_managers();

    Node* current = head;
    while (current != NULL) {
        Booking booking = current->booking;

        //Convert the date to day index
        int start_day = date_to_day_index(booking.date);
        if (start_day < 0 || start_day >= TESTING_DAY) {    //Not in testing period
            current = current->next;
            continue;
        }
        int end_day = start_day;

        //Get start and end time slots
        int start_hour = atoi(booking.time);
        int end_hour = start_hour + (int)booking.duration;
        while (end_hour > TIME_SLOT_PER_DAY) {
            end_day++;
            end_hour -= 24;
        }

        if (end_day < 0 || end_day >= TESTING_DAY) {    //Not in testing period
            current = current->next;
            continue;
        }


        //Check all requested space or items are available or not
        int all_request_available = 1;  //Default true

        //Check parking space availability
        if (booking.parking_space) {
            int available;
            write(resource_pipes_ptc[SPACE][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[SPACE][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[SPACE][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[SPACE][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[SPACE][0], &available, sizeof(int));
            if (!available) {
                //If parking space is not available
                all_request_available = 0;
            }
        }

        //Check battery availability
        if (booking.battery && all_request_available) {
            int available;
            write(resource_pipes_ptc[BATTERY][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[BATTERY][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[BATTERY][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[BATTERY][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[BATTERY][0], &available, sizeof(int));
            if (!available) {
                //If battery is not available
                all_request_available = 0;
            }
        }

        //Check cable availability
        if (booking.cable && all_request_available) {
            int available;
            write(resource_pipes_ptc[CABLE][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[CABLE][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[CABLE][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[CABLE][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[CABLE][0], &available, sizeof(int));
            if (!available) {
                //If cable is not available
                all_request_available = 0;
            }
        }

        //Check locker availability
        if (booking.locker && all_request_available) {
            int available;
            write(resource_pipes_ptc[LOCKER][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[LOCKER][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[LOCKER][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[LOCKER][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[LOCKER][0], &available, sizeof(int));
            if (!available) {
                //If locker is not available
                all_request_available = 0;
            }
        }

        //Check umbrella availability
        if (booking.umbrella && all_request_available) {
            int available;
            write(resource_pipes_ptc[UMBRELLA][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[UMBRELLA][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[UMBRELLA][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[UMBRELLA][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[UMBRELLA][0], &available, sizeof(int));
            if (!available) {
                //If umbrella is not available
                all_request_available = 0;
            }
        }

        //Check valet availability
        if (booking.valet && all_request_available) {
            int available;
            write(resource_pipes_ptc[VALET][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[VALET][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[VALET][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[VALET][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[VALET][0], &available, sizeof(int));
            if (!available) {
                //If valet is not available
                all_request_available = 0;
            }
        }

        //Check inflation availability
        if (booking.inflation && all_request_available) {
            int available;
            write(resource_pipes_ptc[INFLATION][1], &start_day, sizeof(int));
            write(resource_pipes_ptc[INFLATION][1], &end_day, sizeof(int));
            write(resource_pipes_ptc[INFLATION][1], &start_hour, sizeof(int));
            write(resource_pipes_ptc[INFLATION][1], &end_hour, sizeof(int));
            read(resource_pipes_ctp[INFLATION][0], &available, sizeof(int));
            if (!available) {
                //If inflation is not available
                all_request_available = 0;
            }
        }

        if (all_request_available) {    //All space and items are available
            //Add to accepted list
            if (accepted == NULL) {  //Accepted list is null
                accepted = create_node(booking);
            } else {    //Accepted list is not null
                append_node(&accepted, booking);
            }
        } else {    //All space and items are not available
            //Add to rejected list
            if (rejected == NULL) {  //Rejected list is null
                rejected = create_node(booking);
            } else {    //Rejected list is not null
                append_node(&rejected, booking);
            }
        }

        //Send schedule time slot signal
        if (booking.parking_space) {
            write(resource_pipes_ptc[SPACE][1], &all_request_available, sizeof(int));
        }
        if (booking.battery) {
            write(resource_pipes_ptc[BATTERY][1], &all_request_available, sizeof(int));
        }
        if (booking.cable) {
            write(resource_pipes_ptc[CABLE][1], &all_request_available, sizeof(int));
        }
        if (booking.locker) {
            write(resource_pipes_ptc[LOCKER][1], &all_request_available, sizeof(int));
        }
        if (booking.umbrella) {
            write(resource_pipes_ptc[UMBRELLA][1], &all_request_available, sizeof(int));
        }
        if (booking.valet) {
            write(resource_pipes_ptc[VALET][1], &all_request_available, sizeof(int));
        }
        if (booking.inflation) {
            write(resource_pipes_ptc[INFLATION][1], &all_request_available, sizeof(int));
        }

        current = current->next;
    }
    cleanup_child_processes();
}

void print_bookings_priority(Node* head, Node* accepted, Node* rejected) {
    create_resource_managers();

    //Create separate lists for each priority level
    Node* priority_lists[4] = {NULL}; // Index 0 unused, priorities are 1-4

    //Sort bookings into priority lists
    Node* current = head;
    while (current != NULL) {
        Booking booking = current->booking;
        
        //Insert into appropriate priority list
        if (booking.priority == 4) {
            if (priority_lists[3] == NULL) {
                priority_lists[3] = create_node(booking);
            } else {
                append_node(&priority_lists[3], booking);
            }
        } else if (booking.priority == 3) {
            if (priority_lists[2] == NULL) {
                priority_lists[2] = create_node(booking);
            } else {
                append_node(&priority_lists[2], booking);
            }
        } else if (booking.priority == 2) {
            if (priority_lists[1] == NULL) {
                priority_lists[1] = create_node(booking);
            } else {
                append_node(&priority_lists[1], booking);
            }
        } else {
            if (priority_lists[0] == NULL) {
                priority_lists[0] = create_node(booking);
            } else {
                append_node(&priority_lists[0], booking);
            }
        }
        current = current->next;
    }
    
    //Process bookings in priority order (4 first, then 3, then 2, then 1)
    for (int prio = 3; prio >= 0; prio--) {
        current = priority_lists[prio];
        while (current != NULL) {
            Booking booking = current->booking;

            //Convert the date to day index
            int start_day = date_to_day_index(booking.date);
            if (start_day < 0 || start_day >= TESTING_DAY) {    //Not in testing period
                current = current->next;
                continue;
            }
            int end_day = start_day;

            //Get start and end time slots
            int start_hour = atoi(booking.time);
            int end_hour = start_hour + (int)booking.duration;
            while (end_hour > TIME_SLOT_PER_DAY) {
                end_day++;
                end_hour -= 24;
            }

            if (end_day < 0 || end_day >= TESTING_DAY) {    //Not in testing period
                current = current->next;
                continue;
            }

            //Check all requested space or items are available or not
            int all_request_available = 1;  //Default true
            
            //Check parking space availability
            if (booking.parking_space) {
                int available;
                write(resource_pipes_ptc[SPACE][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[SPACE][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[SPACE][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[SPACE][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[SPACE][0], &available, sizeof(int));
                if (!available) {
                    //If parking space is not available
                    all_request_available = 0;
                }
            }
            
            //Check battery availability
            if (booking.battery && all_request_available) {
                int available;
                write(resource_pipes_ptc[BATTERY][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[BATTERY][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[BATTERY][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[BATTERY][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[BATTERY][0], &available, sizeof(int));
                if (!available) {
                    //If battery is not available
                    all_request_available = 0;
                }
            }
            
            //Check cable availability
            if (booking.cable && all_request_available) {
                int available;
                write(resource_pipes_ptc[CABLE][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[CABLE][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[CABLE][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[CABLE][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[CABLE][0], &available, sizeof(int));
                if (!available) {
                    //If cable is not available
                    all_request_available = 0;
                }
            }
            
            //Check locker availability
            if (booking.locker && all_request_available) {
                int available;
                write(resource_pipes_ptc[LOCKER][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[LOCKER][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[LOCKER][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[LOCKER][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[LOCKER][0], &available, sizeof(int));
                if (!available) {
                    //If locker is not available
                    all_request_available = 0;
                }
            }

            //Check umbrella availability
            if (booking.umbrella && all_request_available) {
                int available;
                write(resource_pipes_ptc[UMBRELLA][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[UMBRELLA][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[UMBRELLA][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[UMBRELLA][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[UMBRELLA][0], &available, sizeof(int));
                if (!available) {
                    //If umbrella is not available
                    all_request_available = 0;
                }
            }

            //Check valet availability
            if (booking.valet && all_request_available) {
                int available;
                write(resource_pipes_ptc[VALET][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[VALET][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[VALET][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[VALET][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[VALET][0], &available, sizeof(int));
                if (!available) {
                    //If valet is not available
                    all_request_available = 0;
                }
            }

            //Check inflation availability
            if (booking.inflation && all_request_available) {
                int available;
                write(resource_pipes_ptc[INFLATION][1], &start_day, sizeof(int));
                write(resource_pipes_ptc[INFLATION][1], &end_day, sizeof(int));
                write(resource_pipes_ptc[INFLATION][1], &start_hour, sizeof(int));
                write(resource_pipes_ptc[INFLATION][1], &end_hour, sizeof(int));
                read(resource_pipes_ctp[INFLATION][0], &available, sizeof(int));
                if (!available) {
                    //If inflation is not available
                    all_request_available = 0;
                }
            }

            if (all_request_available) {    //All space and items are available
                //Add to accepted list
                if (accepted == NULL) { //Accepted list is null
                    accepted = create_node(booking);
                } else {    //Accepted list is not null
                    append_node(&accepted, booking);
                }
            } else {    // Not all resources available
                // Add to rejected list
                if (rejected == NULL) {
                    rejected = create_node(booking);
                } else {
                    append_node(&rejected, booking);
                }
            }

            //Send schedule time slot signal
            if (booking.parking_space) {
                write(resource_pipes_ptc[SPACE][1], &all_request_available, sizeof(int));
            }
            if (booking.battery) {
                write(resource_pipes_ptc[BATTERY][1], &all_request_available, sizeof(int));
            }
            if (booking.cable) {
                write(resource_pipes_ptc[CABLE][1], &all_request_available, sizeof(int));
            }
            if (booking.locker) {
                write(resource_pipes_ptc[LOCKER][1], &all_request_available, sizeof(int));
            }
            if (booking.umbrella) {
                write(resource_pipes_ptc[UMBRELLA][1], &all_request_available, sizeof(int));
            }
            if (booking.valet) {
                write(resource_pipes_ptc[VALET][1], &all_request_available, sizeof(int));
            }
            if (booking.inflation) {
                write(resource_pipes_ptc[INFLATION][1], &all_request_available, sizeof(int));
            }
            
            current = current->next;
        }
    }

    //Free priority lists
    for (int i = 0; i < 4; i++) {
        free_list(priority_lists[i]);
    }

    cleanup_child_processes();
}