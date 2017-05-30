#!/usr/bin/env python

from __future__ import print_function

import sys
import argparse
import random
import string


def randName(nameLength):
    return ''.join(random.choice(string.ascii_uppercase) for _ in range(nameLength))


def randIntTuple(columns, N=1):
    return [tuple([random.randint(0, 100) for _ in range(columns)]) for _ in range(N)]


def createTable(options, f):
    schema = ", ".join([options.column_names[i] +
                        " int" for i in range(options.columns)])
    print("drop table {};".format(options.table_name), file=f)
    print("create table {} ({});".format(options.table_name, schema), file=f)


def insertTuple(options, f):
    for _ in range(options.tuples / options.batch_size):
        newTuple = ", ".join(map(str, randIntTuple(
            options.columns, options.batch_size)))
        print("insert into {} values {};".format(
            options.table_name, newTuple), file=f)


def genTable(options):
    f = open(options.filename, 'wb')

    createTable(options, f)
    insertTuple(options, f)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--def_name_len', default=6, type=int)
    parser.add_argument('--table_name', default=None, type=str)
    parser.add_argument('--columns', default=2, type=int)
    parser.add_argument('--tuples', default=1000, type=int)
    parser.add_argument('--batch_size', default=10, type=int)
    parser.add_argument('--filename', default="largeTable.sql", type=str)
    options = parser.parse_args()

    if options.table_name is None:
        options.table_name = randName(options.def_name_len)
    options.column_names = [randName(options.def_name_len).lower()
                            for _ in range(options.columns)]

    genTable(options)


if __name__ == "__main__":
    main()
