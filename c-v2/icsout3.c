#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"

#define MAX_OUTPUT_LEN 132


/*
    Read in the provided .ics file and populate the eventstruct
    array E with the necessary data for each event.
*/
node_t* read_events(char* filename, node_t* list) {
    FILE* fp;
    fp = fopen(filename, "r");
    char* line = NULL;
    size_t len = 0;

    event_t* tmp_event = NULL;
    node_t* tmp_node = NULL;
    int in_event = 0;
    while (getline(&line, &len, fp) != -1) { // Iterate through each line in input
        // Event begins
        if (strncmp(line, "BEGIN:VEVENT", strlen("BEGIN:VEVENT")) == 0) {
            in_event = 1;
            tmp_event = emalloc(sizeof(event_t));
        }

        // Set the event's start date
        if (in_event && strncmp(line, "DTSTART:", strlen("DTSTART:")) == 0) {
            strcpy(tmp_event->DTSTART, line + strlen("DTSTART:"));
        }
        // Set the event's end date
        if (in_event && strncmp(line, "DTEND:", strlen("DTEND:")) == 0) {
            strcpy(tmp_event->DTEND, line + strlen("DTEND:"));
        }

        // Check if event repeats
        char* pos = strstr(line, "UNTIL=");
        if (in_event && pos != NULL) {
            strncpy(tmp_event->UNTIL, pos + strlen("UNTIL="), 15);
            tmp_event->UNTIL[16] = '\0';
            tmp_event->REPEAT = 1;
        }

        // Set the event's location
        if (in_event && strncmp(line, "LOCATION:", strlen("LOCATION:")) == 0) {
            strcpy(tmp_event->LOCATION, line + strlen("LOCATION:"));
        }
        // Set the event's summary
        if (in_event && strncmp(line, "SUMMARY:", strlen("SUMMARY:")) == 0) {
            strcpy(tmp_event->SUMMARY, line + strlen("SUMMARY:"));
        }
        
        // End event, add to list in order
        if (strncmp(line, "END:VEVENT", strlen("END:VEVENT")) == 0) {
            in_event = 0;
            tmp_node = new_node(tmp_event);
            list = add_inorder(list, tmp_node);
        }
    }

    free(line);
    fclose(fp);
    return list;
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
    corresponding dates incremented by N weeks.
*/
void expand_events(node_t* list) {
    node_t* curr = list;
    event_t* new = NULL;
    while (curr != NULL) {
        if (curr->val->REPEAT) {
            new = emalloc(sizeof(event_t));
            add_week(new->DTSTART, curr->val->DTSTART, 1);
            add_week(new->DTEND, curr->val->DTEND, 1);
            strcpy(new->LOCATION, curr->val->LOCATION);
            strcpy(new->SUMMARY, curr->val->SUMMARY);
            strcpy(new->UNTIL, curr->val->UNTIL);

            // check if new event has a repeat
            // i.e. start date + 7 <= until date
            char tmp_dt[20];
            add_week(tmp_dt, new->DTSTART, 1);
            if (strcmp(tmp_dt, new->UNTIL) <= 0) {
                new->REPEAT = 1;
            }

            list = add_inorder(list, new_node(new));
        }
        curr = curr->next;
    }
}


/*
    Takes a raw datetime string and parses it to proper format.
*/
void parse_date(char *formatted_date, const char *dt_time, const int len) {
    struct tm temp_time;
    time_t full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_date, len, "%B %d, %Y (%a)", localtime(&full_time));
}

/*
    Adds dashes under the formatted date to be ready for output.
*/
void format_date(char* dest, char* src) {
    char formatted_date[MAX_OUTPUT_LEN];
    parse_date(formatted_date, src, MAX_OUTPUT_LEN);
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
    } else if (hour == 0)
        hour += 12;

    char min_str[3];
    strncpy(min_str, src + 11, 2);
    min_str[2] = '\0';
    int min = atoi(min_str);

    sprintf(dest, "%2d:%02d %s", hour, min, am_or_pm);
}

/*
    Main output function
*/
void print_events(node_t* list, int from_yy, int from_mm, int from_dd, int to_yy, int to_mm, int to_dd) {
    // Convert integer date representations to datetime strings
    char from_dt_str[17];
    char to_dt_str[17];
    sprintf(from_dt_str, "%d%02d%02d%s", from_yy, from_mm, from_dd, "T000000");
    sprintf(to_dt_str, "%d%02d%02d%s", to_yy, to_mm, to_dd, "T235959");
    
    char curr_dt_str[MAX_OUTPUT_LEN];
    curr_dt_str[0] = '\0';

    node_t* curr_node = list->next; // skip dummy node
    for ( ; curr_node != NULL; curr_node = curr_node->next) {
        
        // Check if event falls within specified date range
        if (strcmp(curr_node->val->DTSTART, from_dt_str) >= 0 && strcmp(curr_node->val->DTSTART, to_dt_str) <= 0) {
            
            // Get the formatted date (including dashes)
            char formatted_date[MAX_OUTPUT_LEN];
            format_date(formatted_date, curr_node->val->DTSTART);

            // Check if the event date is different from previous event (i.e. print new date line)
            if (strcmp(curr_dt_str, formatted_date) != 0) {
                if (curr_dt_str[0] != '\0')
                    printf("\n");
                printf("%s", formatted_date);
                strcpy(curr_dt_str, formatted_date);
            }

            // Get the formatted start and end times
            char start_time[MAX_OUTPUT_LEN], end_time[MAX_OUTPUT_LEN];
            format_time(start_time, curr_node->val->DTSTART);
            format_time(end_time, curr_node->val->DTEND);

            // Remove trailing newline character
            curr_node->val->SUMMARY[strlen(curr_node->val->SUMMARY)-1] = '\0';
            curr_node->val->LOCATION[strlen(curr_node->val->LOCATION)-1] = '\0';
            printf("%s to %s: %s {{%s}}\n", start_time, end_time, curr_node->val->SUMMARY, curr_node->val->LOCATION);
        }
    }
}


int main(int argc, char* argv[]) {
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char* filename = NULL;

    for (int i=0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 7) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 5) == 0) {
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

    node_t* event_list = NULL;
    event_list = read_events(filename, event_list);
    expand_events(event_list);
    print_events(event_list, from_y, from_m, from_d, to_y, to_m, to_d);


    /* Free dynamically allocated event list */
    node_t* tmp;
    while (event_list != NULL) {
        tmp = event_list;
        event_list = event_list->next;
        free(tmp->val);
        free(tmp);
    }

    exit(0);
}
