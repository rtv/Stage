#!/usr/bin/python

# Desc: Test program for the Stage worldfile
# Author: Andrew Howard
# Date; 6 Jun 2002
# CVS: $Id: test.py,v 1.1 2002-06-07 16:28:40 inspectorg Exp $

import os
import string
import sys


def test_file(filename):
    """Runs stage with the given filename and validates the output."""

    file = open(filename)
    a = read_lists(file)
    del file

    file = os.popen('../../src/stage %s' % filename, 'r')
    b = read_lists(file)
    del file

    print '%s : errors  ' % filename,
    if compare_lists(a[2], b[2]) != 0:
        print ': \033[41mfail\033[0m'
        print_lists(a[2], b[2])
        return 1
    else:
        print ': pass'        

    print '%s : sections' % filename,
    if compare_lists(a[0], b[0]) != 0:
        print ': \033[41mfail\033[0m'
        print_lists(a[0], b[0])
    else:
        print ': pass'        

    print '%s : items   ' % filename,
    if compare_lists(a[1], b[1]) != 0:
        print ': \033[41mfail\033[0m'
        print_lists(a[1], b[1])
    else:
        print ': pass'        

    return 0


def read_lists(file):
    """Read and return the various lists from a file object."""
    
    list = None
    sections = []
    items = []
    errors = []

    while 1:

        line = file.readline()
        if not line:
            break
        bits = string.split(line)
        if not bits:
            continue
        if bits[0] == '##':
            if bits[1] == 'begin':
                if bits[2] == 'sections':
                    list = sections
                elif bits[2] == 'items':
                    list = items
            elif bits[1] == 'end':
                list = None
            elif bits[1] == 'stage' and bits[2] == 'error':
                errors.append(string.join(bits[4:]))
            elif list != None:
                list.append(string.join(bits[1:]))
        elif bits[0] == 'stage' and bits[1] == 'error':
            errors.append(string.join(bits[3:]))
        
    return (sections, items, errors)


def compare_lists(la, lb):
    """Compare two lists."""

    if len(la) != len(lb):
        return 1
    for i in range(0, len(la)):
        sa = la[i]
        sb = lb[i]
        if sa != sb:
            return 1
    return 0


def print_lists(la, lb):
    """Print two lists."""

    for i in range(0, max(len(la), len(lb))):
        if i < len(la):
            sa = la[i]
        else:
            sa = ''
        if i < len(lb):
            sb = lb[i]
        else:
            sb = ''
        if sa == sb:
            print sa
            print sb
        else:
            print '\033[42m' + sa + '\33[K\033[0m'
            print '\033[41m' + sb + '\33[K\033[0m'
    return




def main(argv):
    """Run the tests."""

    for test in argv[1:]:
        test_file(test)

    return




if __name__ == '__main__':

    main(sys.argv)
