import datetime as dt
import re


class ICSout:

    def __init__(self, filename):
        self.filename = filename
        self.E = []
        self.__read_events()
        self.__expand_events()
        self.E.sort(key= lambda event: event['DTSTART'])


    def __read_events(self):
        '''
        Read in the provided .ics file and populate the list of
        dictionaries containing the necessary data for each event.
        '''

        file = open(self.filename, mode='r')

        in_event = False
        for line in file:
            # Event begins
            if re.search(r"BEGIN:VEVENT", line):
                in_event = True
                self.E.append({'REPEAT': False})
            
            # Get the event's start date
            if in_event and re.search(r"DTSTART:", line):
                start_dt = re.search(r"\d{8}T\d{6}", line)
                self.E[-1]['DTSTART'] = start_dt.group()

            # Get the event's end date
            if in_event and re.search(r"DTEND:", line):
                end_dt = re.search(r"\d{8}T\d{6}", line)
                self.E[-1]['DTEND'] = end_dt.group()

            # Get the event's location
            if in_event and re.search(r"LOCATION:", line):
                location = re.search(r"(?<=LOCATION:).*", line) # get text after location prompt
                self.E[-1]['LOCATION'] = location.group()

            # Get the event's summary
            if in_event and re.search(r"SUMMARY:", line):
                summary = re.search(r"(?<=SUMMARY:).*", line) # get text after summary prompt
                self.E[-1]['SUMMARY'] = summary.group()

            # Check if event repeats
            if in_event and re.search(r"UNTIL", line):
                until_dt = re.search(r"\d{8}T\d{6}", line)
                self.E[-1]['UNTIL'] = until_dt.group()
                self.E[-1]['REPEAT'] = True

            # End of event
            if re.search(r"END:VEVENT", line):
                in_event = False
        
        file.close()


    def __expand_events(self):
        '''
        Expand repeating events by generating new events with
        corresponding dates incremented by N weeks.
        Extends the E array to include newly generated events.
        '''

        new_events = []
        for event in self.E:
            if event['REPEAT'] == True:
                # Get the dates on which the event occurs
                repeat_dates = []
                dt_until_str = re.search(r"\d{8}", event['UNTIL']).group() # get date without time
                dt_until = dt.datetime.strptime(dt_until_str, '%Y%m%d')
                dt_curr_str = re.search(r"\d{8}", event['DTSTART']).group()
                dt_curr = dt.datetime.strptime(dt_curr_str, '%Y%m%d') + dt.timedelta(weeks=1)
                while dt_curr <= dt_until:
                    repeat_dates.append(dt_curr)
                    dt_curr += dt.timedelta(weeks=1)

                # Create a new event for each repeating date
                for date in repeat_dates:
                    new_events.append({
                        'DTSTART': date.strftime('%Y%m%d') + re.search(r"T\d{6}", event['DTSTART']).group(), # add time to dt string
                        'DTEND': date.strftime('%Y%m%d') + re.search(r"T\d{6}", event['DTEND']).group(),
                        'REPEAT': event['REPEAT'],
                        'UNTIL': event['UNTIL'],
                        'LOCATION': event['LOCATION'],
                        'SUMMARY': event['SUMMARY']
                    })
        self.E.extend(new_events)
    

    def __format_date(self, date):
        '''
        Takes a datetime object and formats it properly for output,
        adding dashes underneath the formatted date.
        '''

        formatted_date = date.strftime('%B %d, %Y (%a)') + '\n'
        for _ in range(len(formatted_date)-1):
            formatted_date += '-'  # add dashes
        return formatted_date


    def get_events_for_day(self, date):
        '''
            If the day corresponding to the date parameter has events, then
            the	method returns a string with that day’s events formatted.
            If the day does not contain any events, None will be returned.
        '''

        output_str = ''

        no_events = 1
        for event in self.E:
            event_date_str = re.search(r"\d{8}", event['DTSTART']).group() # get date without time
            event_date = dt.datetime.strptime(event_date_str, '%Y%m%d')
            
            # Check if event falls within specified date range
            if event_date == date:

                # Print formatted date line with dashes underneath
                if no_events:
                    output_str += self.__format_date(event_date)
                    no_events = 0
                
                start_time_str = re.search(r"T\d{6}", event['DTSTART']).group()
                start_time = dt.datetime.strptime(start_time_str[1:], '%H%M%S')
                end_time_str = re.search(r"T\d{6}", event['DTEND']).group()
                end_time = dt.datetime.strptime(end_time_str[1:], '%H%M%S')
                start_time_str = start_time.strftime('%-I:%M %p')
                end_time_str = end_time.strftime('%-I:%M %p')

                # Add initial space if single digit hour
                if re.search(r"\b\d:\d\d", start_time_str):
                    start_time_str = ' ' + start_time_str
                if re.search(r"\b\d:\d\d", end_time_str):
                    end_time_str = ' ' + end_time_str
                
                event_str = f"{start_time_str} to {end_time_str}: {event['SUMMARY']} "
                event_str += "{{" + event['LOCATION'] + "}}"
                output_str += '\n' + event_str
        
        return output_str if output_str != '' else None