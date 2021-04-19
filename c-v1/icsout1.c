#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500
#define DATE_LEN 16

struct eventstruct {
    char DTSTART[DATE_LEN+1];
    char DTEND[DATE_LEN+1];
    char LOCATION[80];
    char SUMMARY[80];
    int REPEAT;
    char UNTIL[DATE_LEN+1];
};

struct eventstruct E[MAX_EVENTS];
int num_events = 0;


/*
    Read in the provided .ics file and populate the eventstruct
    array E with the necessary data for each event.
*/
void read_events(char* filename) {
    FILE* fp;
    fp = fopen(filename, "r");
    char* line = NULL;
    size_t len = 0;

    int in_event = 0;
    int i=0;
    while (getline(&line, &len, fp) != -1) { // Iterate through each line in input file
        // Event begins
        if (strncmp(line, "BEGIN:VEVENT", strlen("BEGIN:VEVENT")) == 0) {
            in_event = 1;
        }

        // Set E[i].DTSTART to the event's start date in input
        if (in_event && strncmp(line, "DTSTART:", strlen("DTSTART:")) == 0) {
            strcpy(E[i].DTSTART, line + strlen("DTSTART:"));
        }
        // Set E[i].DTEND to the event's end date in input
        if (in_event && strncmp(line, "DTEND:", strlen("DTEND:")) == 0) {
            strcpy(E[i].DTEND, line + strlen("DTEND:"));
        }

        // Check if event repeats, set E[i].UNTIL and E[i].REPEAT if so
        char* pos = strstr(line, "UNTIL=");
        if (in_event && pos != NULL) {
            strncpy(E[i].UNTIL, pos + strlen("UNTIL="), DATE_LEN-1);
            E[i].UNTIL[DATE_LEN] = '\0';
            E[i].REPEAT = 1;
        }

        // Set E[i].LOCATION to the event's location in input
        if (in_event && strncmp(line, "LOCATION:", strlen("LOCATION:")) == 0) {
            strcpy(E[i].LOCATION, line + strlen("LOCATION:"));
        }
        // Set E[i].SUMMARY to the event's summary in input
        if (in_event && strncmp(line, "SUMMARY:", strlen("SUMMARY:")) == 0) {
            strcpy(E[i].SUMMARY, line + strlen("SUMMARY:"));
        }
        
        // End event, increment event counter
        if (strncmp(line, "END:VEVENT", strlen("END:VEVENT")) == 0) {
            in_event = 0;
            i++;
        }
    }
    num_events = i;

    fclose(fp);
}

/*
    Given a datetime string, add num_weeks weeks and store
    the resulting date in new_date.
*/
void add_week(char* new_date, char* date, int num_weeks) {
    char year_str[5];
    strncpy(year_str, date, 4);
    year_str[4] = '\0';

    char month_str[3];
    strncpy(month_str, date+4, 2);
    month_str[2] = '\0';

    char day_str[3];
    strncpy(day_str, date+6, 2);
    day_str[2] = '\0';

    int y=atoi(year_str), m=atoi(month_str), d=atoi(day_str);
    struct tm t = { .tm_year=y-1900, .tm_mon=m-1, .tm_mday=d };
    t.tm_mday += 7 * num_weeks;
    mktime(&t);

    sprintf(new_date, "%d%02d%02d%s", t.tm_year+1900, t.tm_mon+1, t.tm_mday, date+8);
    return;
}

/*
    Expand repeating events by generating new events with
    corresponding dates incremented by N weeks and add to E.
*/
void expand_E() {
    int num_events_curr = num_events;
    for (int i=0; i < num_events_curr; i++) {

        // Check if the event has repeats
        if (E[i].REPEAT) {
            struct eventstruct tmp = E[i];
            
            // Generate repeat events while the tmp event start date is less than E[i].UNTIL
            while (strcmp(tmp.DTSTART, E[i].UNTIL) <= 0) {
                add_week(E[num_events].DTSTART, tmp.DTSTART, 1);
                add_week(E[num_events].DTEND, tmp.DTEND, 1);
                E[num_events].REPEAT = 0;
                strcpy(E[num_events].LOCATION, E[i].LOCATION);
                strcpy(E[num_events].SUMMARY, E[i].SUMMARY);
                tmp = E[num_events];
                num_events++;
            }
            // Remove last event if it has exceeded range limit
            if (strcmp(E[num_events-1].DTSTART, E[i].UNTIL) > 0)
                num_events--;
        }
    }
}


/*
    Helper function for sort_E. Event A > Event B if DTSTART A > DTSTART B.
*/
int event_comparator(const void *v1, const void *v2) {
    const struct eventstruct* p1 = (struct eventstruct*)v1;
    const struct eventstruct* p2 = (struct eventstruct*)v2;
    return strcmp(p1->DTSTART, p2->DTSTART);
}

/*
    Sorts the events in E in ascending order based on DTSTART
    using event_comparator.
*/
void sort_E() {
    qsort(E, num_events, sizeof(struct eventstruct), event_comparator);
}


/*
    Takes a raw datetime string and formats the date.
*/
void dt_format(char *formatted_date, const char *dt_time, const int len) {
    struct tm temp_time;
    time_t full_time;
    char temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_date, len, "%B %d, %Y (%a)", localtime(&full_time));
}

/*
    Adds dashes to the formatted date to be ready for output.
*/
void format_date(char* dest, char* src) {
    char formatted_date[MAX_LINE_LEN];
    dt_format(formatted_date, src, MAX_LINE_LEN);
    sprintf(dest, "%s\n", formatted_date);
    
    // Add dashes underneath the date matching the length
    for (int i=0; i < strlen(formatted_date); i++) {
        sprintf(dest + strlen(dest), "-");
    }
    sprintf(dest + strlen(dest), "\n");
}

/*
    Takes a raw datetime string and formats the time for output.
*/
void format_time(char* dest, char* src) {
    char hour_str[3];
    strncpy(hour_str, src + 9, 2);
    hour_str[2] = '\0';
    int hour = atoi(hour_str);

    char* am_or_pm = "AM";
    if (hour >= 12) {
        if (hour > 12)
            hour -= 12;
        am_or_pm = "PM";
    }

    char min_str[3];
    strncpy(min_str, src + 11, 2);
    min_str[2] = '\0';
    int min = atoi(min_str);

    sprintf(dest, "%2d:%02d %s", hour, min, am_or_pm);
}

/*
    Main output function
*/
void print_events(int from_yy, int from_mm, int from_dd, int to_yy, int to_mm, int to_dd) {
    // Convert integer date representations to datetime strings
    char from_date_str[DATE_LEN+1];
    sprintf(from_date_str, "%d%02d%02d%s", from_yy, from_mm, from_dd, "T000000");
    char to_date_str[DATE_LEN+1];
    sprintf(to_date_str, "%d%02d%02d%s", to_yy, to_mm, to_dd, "T235959");
    
    char current_date[MAX_LINE_LEN];
    current_date[0] = '\0';

    for (int i=0; i < num_events; i++) {
        
        // Check if event falls within specified date range
        if (strcmp(E[i].DTSTART, from_date_str) >= 0 && strcmp(E[i].DTSTART, to_date_str) <= 0) {
            
            // Get the formatted date (including dashes)
            char formatted_date[MAX_LINE_LEN];
            format_date(formatted_date, E[i].DTSTART);

            // Check if the event date is different from previous event (i.e. print new date line)
            if (strcmp(current_date, formatted_date) != 0) {
                if (current_date[0] != '\0')
                    printf("\n");
                printf("%s", formatted_date);
                strcpy(current_date, formatted_date);
            }

            // Get the formatted start and end times
            char start_time[MAX_LINE_LEN], end_time[MAX_LINE_LEN];
            format_time(start_time, E[i].DTSTART);
            format_time(end_time, E[i].DTEND);

            // Remove trailing newline character
            E[i].SUMMARY[strlen(E[i].SUMMARY)-1] = '\0';
            E[i].LOCATION[strlen(E[i].LOCATION)-1] = '\0';
            printf("%s to %s: %s {{%s}}\n", start_time, end_time, E[i].SUMMARY, E[i].LOCATION);
        }
    }
}


int main(int argc, char *argv[]) {
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char* filename = NULL;
    int i; 

    for (i=0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }

    read_events(filename);
    expand_E();
    sort_E();
    print_events(from_y, from_m, from_d, to_y, to_m, to_d);

    exit(0);
}