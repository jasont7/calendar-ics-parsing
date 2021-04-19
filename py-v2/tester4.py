#!/usr/bin/env python3

#
# UVic SENG 265, Spring 2021, Assignment #4
#
# This test-driver program invokes methods on the class to be
# completed for the assignment.
# 
# The content of tester4.py must not be modified. It will be used
# when evaluating your solution to icsout4.py.
#

import sys
import argparse
import datetime
import icsout4


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='file to be processed')
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')

    args = parser.parse_args()

    if not args.file:
        print("Need --file=<ics filename>")

    if not args.start:
        print("Need --start=yyyy/mm/dd")

    if not args.end:
        print("Need --end=yyyy/mm/dd")

    (start_yy, start_mm, start_dd) = \
        (int(field) for field in args.start.split("/"))

    (end_yy, end_mm, end_dd) = \
        (int(field) for field in args.end.split("/"))

    start_dt = datetime.datetime(start_yy, start_mm, start_dd)
    end_dt   = datetime.datetime(end_yy, end_mm, end_dd)

    orig_stdout = sys.stdout
    sys.stdout = None

    c = icsout4.ICSout(args.file)
  
    newline = ""
 
    curr_dt = start_dt
    days_events = c.get_events_for_day(curr_dt)
    if (days_events):
        sys.stdout = orig_stdout
        print(days_events)
        newline = '\n'
        sys.stdout = None
    curr_dt = curr_dt + datetime.timedelta(days=1)

    while (curr_dt <= end_dt):
        days_events = c.get_events_for_day(curr_dt)
        if (days_events):
            sys.stdout = orig_stdout
            print(end=newline)
            print(days_events)
            newline = '\n'
            sys.stdout = None
        curr_dt = curr_dt + datetime.timedelta(days=1)

    sys.stdout = orig_stdout

if __name__ == "__main__":
    main()
