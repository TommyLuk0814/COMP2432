#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"


void readFromUserInput();
void executeCommand(char *keyword[],int keywordLength );

int checkForMemberName(char *name);
int checkForaddParking(char *keyword[],int keywordLength);
int checkForAddReservation(char *keyword[],int keywordLength) ;
int checkForAddEvent(char *keyword[],int keywordLength);
int checkForBookEssentials(char *keyword[],int keywordLength);

void insertEssentials(Booking *booking, int numOfEssentials, char *keyword[],int isPair) ;
void insertToLinklist(Booking *booking);
void readBatchFile(char *filename);

int commonInspectItem(char *keyword[],int keywordLength) ;
int checkForDate(const char *date);
int checkForTime(const char *time);
int checkForHours(char *hours) ;
int checkForEssentials(char* keyword[],int keyLength);

void printLinklist();

Node *head = NULL;

int main() {
    printf("~~ WELCOME TO PolyU ~~\n");

    while (1) {
        char line[100];
        printf("Please enter booking:\n");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = 0;

        char *keyword[10];  // at most store 10 words
        int keywordLength = 0;

        //divide commands by sapce
        char *word = strtok(line, " ");
        while (word != NULL) {
            keyword[keywordLength] = word;
            keywordLength++;
            word = strtok(NULL, " ");
        }

        if ((strcmp(keyword[0], "addParking") == 0) ||
            (strcmp(keyword[0], "addReservation") == 0) ||
            (strcmp(keyword[0], "bookEssentials") == 0) ||
            (strcmp(keyword[0], "addEvent") == 0))
            { executeCommand(keyword,keywordLength);  }

        else if (strcmp(keyword[0], "addBatch") == 0) {
            readBatchFile(keyword[1]);

        } else if (strcmp(keyword[0], "printBookings") == 0) {
        } else if (strcmp(keyword[0], "endProgram") == 0) {
            printf("-> Bye!\n");
            break;
        }else if (strcmp(keyword[0], "print") == 0) {
            printLinklist();
        }
        else {
            printf("-> Please check your command again.\n");
        }
    }

    return 0;
}

void printLinklist() {
    if (head == NULL) {
        printf("-> No bookings to display.\n");
        return;
    }
    Node *current = head;
    int count = 1;
    while (current != NULL) {
        Booking *b = &current->booking;
        printf("Booking %d:", count++);
        printf("  Member: %s", b->member);
        printf("  Date: %s", b->date);
        printf("  Time: %s", b->time);
        printf("  Duration: %.1f", b->duration);
        printf("  Priority: %d", b->priority);
        printf("  Essentials:");
        if (b->battery) printf("    - Battery");
        if (b->cable) printf("    - Cable");
        if (b->locker) printf("    - Locker");
        if (b->umbrella) printf("    - Umbrella");
        if (b->valet) printf("    - Valet Park");
        if (b->inflation) printf("    - Inflation Service");
        current = current->next;
        printf("\n");
    }
}


void insertToLinklist(Booking *booking) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        printf("-> Memory allocation failed while inserting booking.\n");
        return;
    }

    newNode->booking = *booking;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        Node *temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}


void executeCommand(char *keyword[],int keywordLength ) {
    Booking booking;
    if (strcmp(keyword[0], "addParking") == 0) {
        if (checkForaddParking(keyword, keywordLength)) {
            booking.priority = 2;

            strncpy(booking.member, keyword[1], sizeof(booking.member) - 1);
            booking.member[sizeof(booking.member) - 1] = '\0';

            strcpy(booking.date, keyword[2]);
            strcpy(booking.time, keyword[3]);
            booking.duration = atoi(keyword[4]);

            insertEssentials(&booking,keywordLength-5,keyword,1);

            insertToLinklist(&booking);


        }
    }else if (strcmp(keyword[0], "addReservation") == 0) {

        if (checkForAddReservation(keyword, keywordLength)) {
            booking.priority = 3;

            strncpy(booking.member, keyword[1], sizeof(booking.member) - 1);
            booking.member[sizeof(booking.member) - 1] = '\0';

            strcpy(booking.date, keyword[2]);
            strcpy(booking.time, keyword[3]);
            booking.duration = atoi(keyword[4]);

            insertEssentials(&booking,keywordLength-5,keyword,1);

            insertToLinklist(&booking);
        }


    } else if (strcmp(keyword[0], "bookEssentials") == 0) {

        if (checkForBookEssentials(keyword, keywordLength)) {
            booking.priority = 1;

            strncpy(booking.member, keyword[1], sizeof(booking.member) - 1);
            booking.member[sizeof(booking.member) - 1] = '\0';

            strcpy(booking.date, keyword[2]);
            strcpy(booking.time, keyword[3]);
            booking.duration = atoi(keyword[4]);

            insertEssentials(&booking,keywordLength-5,keyword,0);

            insertToLinklist(&booking);

        }

    } else if (strcmp(keyword[0], "addEvent") == 0) {

        if (checkForAddEvent(keyword, keywordLength)) {
            booking.priority = 4;

            strncpy(booking.member, keyword[1], sizeof(booking.member) - 1);
            booking.member[sizeof(booking.member) - 1] = '\0';

            strcpy(booking.date, keyword[2]);
            strcpy(booking.time, keyword[3]);
            booking.duration = atoi(keyword[4]);

            insertEssentials(&booking,keywordLength-5,keyword,1);

            insertToLinklist(&booking);
        }

    }

}
void readBatchFile(char *filename) {
    // 處理開頭 '-' 和結尾 ';'
    char cleaned[100];
    strncpy(cleaned, filename, sizeof(cleaned));
    cleaned[sizeof(cleaned) - 1] = '\0';

    int len = strlen(cleaned);
    if (cleaned[len - 1] == ';') cleaned[len - 1] = '\0';
    if (cleaned[0] == '-') memmove(cleaned, cleaned + 1, strlen(cleaned));

    FILE *file = fopen(cleaned, "r");
    if (!file) {
        printf("-> Could not open file: %s\n", cleaned);
        return;
    }

    char *buffer = malloc(100000);
    if (!buffer) {
        printf("-> Memory allocation failed.\n");
        fclose(file);
        return;
    }

    size_t length = fread(buffer, 1, 99999, file);
    buffer[length] = '\0';
    fclose(file);

    // 分句（去除空格但保留 ; 在最後一個 word）
    char *saveptr1;
    char *sentence = strtok_r(buffer, ";", &saveptr1);
    while (sentence != NULL) {
        while (*sentence == ' ' || *sentence == '\n' || *sentence == '\r') sentence++;  // 去除開頭空白及換行符

        // 加回句尾的 ;
        char fullSentence[256];
        snprintf(fullSentence, sizeof(fullSentence), "%s;", sentence);

        // 拆字
        char *keyword[10];
        int keywordLength = 0;

        char *saveptr2;
        char *word = strtok_r(fullSentence, " ", &saveptr2);
        while (word != NULL && keywordLength < 10) {
            keyword[keywordLength++] = word;
            word = strtok_r(NULL, " ", &saveptr2);
        }

        if (keywordLength > 0) {
            executeCommand(keyword, keywordLength);
        }

        sentence = strtok_r(NULL, ";", &saveptr1);  // 下一句
    }

    free(buffer);
}

//addParking -aaa YYYY-MM-DD hh:mm n.n bbb ccc;
int checkForaddParking(char *keyword[],int keywordLength) {
    if (keywordLength < 5) {printf("-> Invalid request: please check whether the complete command is entered.\n"); return 0;}

    if (!commonInspectItem(keyword,keywordLength) ) return 0;


    if (keywordLength < 6) return 1;
    if (keywordLength >=8) { printf("-> Booking quantity is incorrect.\n"); return 0; }

    if ( !checkForEssentials(keyword,keywordLength)){printf("-> Invalid request: essentials not recognized.\n"); return 0;}

    return 1;
}

//addReservation -aaa YYYY-MM-DD hh:mm n.n bbb ccc;
int checkForAddReservation(char *keyword[],int keywordLength) {
    if (keywordLength != 7) {printf("-> Invalid request: please check whether the complete command is entered.\n"); return 0;}

    if (!commonInspectItem(keyword,keywordLength) ) return 0;

    if ( !checkForEssentials(keyword,keywordLength)){printf("-> Invalid request: essentials not recognized.\n"); return 0;}

    return 1;
}

//addEvent -aaa YYYY-MM-DD hh:mm n.n bbb ccc ddd;
int checkForAddEvent(char *keyword[],int keywordLength) {

    if (keywordLength <5 || keywordLength >8) {printf("-> Invalid request: please check whether the complete command is entered.\n"); return 0;}

    if (!commonInspectItem(keyword,keywordLength) ) return 0;

    if (keywordLength == 5) return 1;

    if ( !checkForEssentials(keyword,keywordLength)){printf("-> Invalid request: essentials not recognized.\n"); return 0;}

    return 1;
}

//bookEssentials –member_C 2025-05-011 13:00 4.0 battery;
int checkForBookEssentials(char *keyword[],int keywordLength) {

    if (keywordLength != 6) {printf("-> Invalid request: please check whether the complete command is entered.\n"); return 0;}

    if (!commonInspectItem(keyword,keywordLength) ) return 0;

    if ( !checkForEssentials(keyword,keywordLength)){printf("-> Invalid request: essentials not recognized.\n"); return 0;}


    return 1;
}


void insertEssentials(Booking *booking, int numOfEssentials, char *keyword[],int isPair) {
    booking->battery = 0;
    booking->cable = 0;
    booking->locker = 0;
    booking->umbrella = 0;
    booking->valet = 0;
    booking->inflation = 0;

    if (!isPair) {
        char *essential = keyword[5];
        if (strcmp(essential, "battery") == 0 ) {
            booking->battery = 1;
        }else if (strcmp(essential, "cable")  == 0) {
            booking->cable = 1;
        }else if (strcmp(essential, "umbrella") == 0) {
            booking->umbrella = 1;
        }else if (strcmp(essential, "locker") == 0 ) {
            booking->locker = 1;
        } else if ( strcmp(essential, "InflationService") == 0) {
            booking->inflation = 1;
        }else if (strcmp(essential, "valetPark") == 0 ) {
            booking->valet = 1;
        }

        return;
    }

    for (int i = 0; i < numOfEssentials; i++) {
        char *essential = keyword[5 + i]; // essentials start from keyword[5]

        if (strcmp(essential, "battery") == 0 || strcmp(essential, "cable")  == 0) {
            booking->battery = 1;
            booking->cable = 1;
        } else if (strcmp(essential, "locker") == 0 || strcmp(essential, "umbrella") == 0) {
            booking->locker = 1;
            booking->umbrella = 1;
        } else if (strcmp(essential, "valetPark") == 0 || strcmp(essential, "InflationService") == 0) {
            booking->valet = 1;
            booking->inflation = 1;
        }
    }
}


int commonInspectItem(char *keyword[],int keywordLength) {
    if ( !checkForMemberName(keyword[1])){ printf("-> Invalid request: member name not recognized.\n");return 0; }
    if ( !checkForDate(keyword[2])){  printf("-> Invalid request: date format not recognized (YYYY-MM-DD expected).\n");return 0; }
    if ( !checkForTime(keyword[3])){ printf("-> Invalid request: time format not recognized (HH:MM expected).\n"); return 0; }
    if ( !checkForHours(keyword[4])) { printf("-> Invalid request: booking hours format not recognized (n.n).\n"); return 0; }

    return 1;
}

// check the name is match the rules
int checkForMemberName(char *name) {
    const char *validMembers[] = {
        "–member_A", "–member_B", "–member_C", "–member_D", "–member_E"
    };


    for (int i = 0; i < 5; i++) {
        if (strcmp(name, validMembers[i]) == 0) {
            // Try to skip UTF-8 encoded dash (e.g., EN DASH or EM DASH, often 3 bytes)
            unsigned char *p = (unsigned char *)name;
            while (*p && *p >= 0x80) p++;  // skip over UTF-8 multibyte prefix
            memmove(name, p, strlen((char *)p) + 1); // shift cleaned name to front
            return 1;
        }
    }

    return 0;
}

int checkForDate(const char *date) {
    // verify the format YYYY-MM-DD
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') return 0;


    // extract and validate the month range
    int month = (date[5] - '0') * 10 + (date[6] - '0');
    if (month < 1 || month > 12) return 0;

    return 1; // valid date
}

int checkForTime(const char *time) {
    // verify the format HH:MM
    if (strlen(time) != 5 || time[2] != ':') return 0;


    // extract and validate hour and minute
    int hour = (time[0] - '0') * 10 + (time[1] - '0');
    int minute = (time[3] - '0') * 10 + (time[4] - '0');

    if (hour < 0 || hour > 23) return 0;
    if (minute < 0 || minute > 59) return 0;

    return 1; // valid time
}

int checkForHours(char *hours) {

    if (atof(hours) - atoi(hours) == 0) return 1;
    else return 0;

}

int checkForEssentials(char* keyword[],int keyLength) {

    //check whether last char of last element is ';' symbol.
    //then remove.
    int len = strlen(keyword[keyLength - 1]);
    if (len == 0) return 0;

    if (keyword[keyLength - 1][len - 1] != ';') return 0;

    keyword[keyLength - 1][len - 1] = '\0';  // remove ';'


    const char *validEssentials[] = {
        "battery", "cable", "locker","umbrella","InflationService","valetpark"
    };

    int isValid = 0;
    int numValidEssentials = sizeof(validEssentials) / sizeof(validEssentials[0]);

    for (int i = 5; i < keyLength; i++) {
        int found = 0;
        for (int j = 0; j < numValidEssentials; j++) {
            if (strcmp(validEssentials[j], keyword[i]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
        else isValid = 1;
    }

    return isValid;
}


