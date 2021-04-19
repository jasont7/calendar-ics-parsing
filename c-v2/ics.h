#ifndef _ICS_H_
#define _ICS_H_

#define DT_LEN 17
#define SUMMARY_LEN 80
#define LOCATION_LEN 80

typedef struct event_t {
    char DTSTART[DT_LEN];
    char DTEND[DT_LEN];
    char LOCATION[LOCATION_LEN];
    char SUMMARY[SUMMARY_LEN];
    int REPEAT;
    char UNTIL[DT_LEN];
} event_t;

#endif
