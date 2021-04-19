#!/usr/bin/env python3

import argparse
import datetime as dt


def read_events(filename):
    '''
    Read in the provided .ics file and populate the list of
    dictionaries containing the necessary data for each event.
    '''

    file = open(filename, mode='r')

    E = []
    in_event = False
    for line in file:
        # Event begins
        if line == "BEGIN:VEVENT\n":
            in_event = True
            E.append({'REPEAT': False})
        
        # Get the event's start date
        if in_event and line[:8] == "DTSTART:":
            E[-1]['DTSTART'] = line[8:-1]

        # Get the event's end date
        if in_event and line[:6] == "DTEND:":
            E[-1]['DTEND'] = line[6:-1]

        # Get the event's location
        if in_event and line[:9] == "LOCATION:":
            E[-1]['LOCATION'] = line[9:-1]

        # Get the event's summary
        if in_event and line[:8] == "SUMMARY:":
            E[-1]['SUMMARY'] = line[8:-1]

        # Check if event repeats
        if in_event and "UNTIL" in line:
            until_idx = line.find("UNTIL")
            E[-1]['UNTIL'] = line[until_idx+6 : until_idx+21]
            E[-1]['REPEAT'] = True

        # End of event
        if line[:-1] == "END:VEVENT":
            in_event = False
    
    file.close()
    return E


def expand_events(E):
    '''
    Expand repeating events by generating new events with
    corresponding dates incremented by N weeks.
    Extends the input array to include newly generated events.
    '''

    new_events = []
    for event in E:
        if event['REPEAT'] == True:
            # Get the dates on which the event occurs
            repeat_dates = []
            dt_until = dt.datetime.strptime(event['UNTIL'][:8], '%Y%m%d')
            tmp = dt.datetime.strptime(event['DTSTART'][:8], '%Y%m%d') + dt.timedelta(weeks=1)
            while tmp <= dt_until:
                repeat_dates.append(tmp)
                tmp += dt.timedelta(weeks=1)

            # Create a new event for each repeating date
            for date in repeat_dates:
                new_events.append({
                    'DTSTART': date.strftime('%Y%m%d') + event['DTSTART'][8:],
                    'DTEND': date.strftime('%Y%m%d') + event['DTEND'][8:],
                    'REPEAT': event['REPEAT'],
                    'UNTIL': event['UNTIL'],
                    'LOCATION': event['LOCATION'],
                    'SUMMARY': event['SUMMARY']
                })
    E.extend(new_events)
    return E


def format_date(date):
    '''
    Takes a datetime object and formats it properly for output,
    adding dashes underneath the formatted date.
    '''

    formatted_date = date.strftime('%B %d, %Y (%a)') + '\n'
    for _ in range(len(formatted_date)-1):
        formatted_date += '-'  # add dashes
    return formatted_date


def print_events(E, from_dt, to_dt):
    '''
    Main output function
    '''

    current_date = None
    for event in E:
        event_date = dt.datetime.strptime(event['DTSTART'][:8], '%Y%m%d')
        
        # Check if event falls within specified date range
        if event_date >= from_dt and event_date <= to_dt:

            # Check if the event date is different from previous event (i.e. print new date line)
            if event_date != current_date:
                if current_date is not None:
                    print()
                print(format_date(event_date))
                current_date = event_date
            
            start_time = dt.datetime.strptime(event['DTSTART'][9:], '%H%M%S')
            end_time = dt.datetime.strptime(event['DTEND'][9:], '%H%M%S')
            start_time_str = start_time.strftime('%-I:%M %p')
            end_time_str = end_time.strftime('%-I:%M %p')

            # Add initial space if single digit hour
            if start_time_str[1] == ':':
                start_time_str = ' ' + start_time_str
            if end_time_str[1] == ':':
                end_time_str = ' ' + end_time_str
            
            event_str = f"{start_time_str} to {end_time_str}: {event['SUMMARY']} "
            event_str += "{{" + event['LOCATION'] + "}}"
            print(event_str)
            

def main():
    '''
    Parses command line arguments for file name, start date, and end date.
    Gets all events in the date range using helper functions and prints 
    out in proper format.
    '''

    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='file to be processed')
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')
    args = parser.parse_args()

    need_arg = False
    if not args.file:
        print("Need --file=<ics filename>")
        need_arg = True
    if not args.start:
        print("Need --start=dd/mm/yyyy")
        need_arg = True
    if not args.end:
        print("Need --end=dd/mm/yyyy")
        need_arg = True
    if need_arg:
        return
    
    start_date = dt.datetime.strptime(args.start, '%Y/%m/%d')
    end_date = dt.datetime.strptime(args.end, '%Y/%m/%d')

    all_events = read_events(args.file)
    all_events = expand_events(all_events)
    all_events.sort(key= lambda event: event['DTSTART'])

    print_events(all_events, start_date, end_date)


if __name__ == "__main__":
    main()