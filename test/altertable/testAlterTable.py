#!/usr/bin/env python

from __future__ import print_function

import os
import sys
import argparse
import time
import numpy as np
import matplotlib.pyplot as plt
import string

from genTable import *


peloton_command = "psql \"sslmode=disable\" -U postgres -h localhost"
postgres_command = "sudo -u postgres psql"


def testAdd(options, psql_command, add_columns=1):
    start = time.time()
    added_column_names = [randName(options.def_name_len)
                          for _ in range(add_columns)]
    commands = ", ".join(['add column {} int'.format(col)
                          for col in added_column_names])
    os.system(psql_command + " -c \"alter table {} {};\" > /dev/null".format(
        options.table_name, commands))
    end = time.time()
    os.system(psql_command +
              " -c \"drop table {};\" > /dev/null".format(options.table_name))
    return (end - start)


def testDrop(options, psql_command, drop_columns=1):
    start = time.time()
    dropped_column_names = random.sample(
        options.column_names[:options.columns], drop_columns)
    commands = ", ".join(['drop column {}'.format(col)
                          for col in dropped_column_names])
    os.system(psql_command + " -c \"alter table {} {};\" > /dev/null".format(
        options.table_name, commands))
    end = time.time()
    os.system(psql_command +
              " -c \"drop table {};\" > /dev/null".format(options.table_name))
    return (end - start)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--def_name_len', default=6, type=int)
    parser.add_argument('--table_name', default=None, type=str)
    parser.add_argument('--columns', default=5, type=int)
    parser.add_argument('--repeats', default=5, type=int)
    # parser.add_argument('--alter_type', default=1,
    #                     type=int, help='add=1, drop=2')
    # parser.add_argument('--changed_columns', default=1, type=int)
    parser.add_argument('--tuples', default=1000, type=int)
    parser.add_argument('--batch_size', default=100, type=int)
    parser.add_argument('--filename', default="largeTable.sql", type=str)
    parser.add_argument('--port', default=15721, type=int)
    options = parser.parse_args()

    if options.table_name is None:
        options.table_name = randName(options.def_name_len)
    options.column_names = string.ascii_lowercase

    global peloton_command
    global postgres_command
    peloton_command += " -p {}".format(options.port)
    genTable(options)

    # Test add postgres
    add_ts = []
    for i in range(1, options.columns + 1):
        ts = []
        for _ in range(options.repeats):
            os.system(postgres_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, postgres_command, i))
        add_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(add_ts)

    f, ax = plt.subplots(1)
    add_ts = np.array(add_ts)
    ax.plot(np.arange(options.columns) + 1, add_ts.T[0], label='postgres')
    ax.fill_between(np.arange(options.columns) + 1,
                    add_ts.T[1], add_ts.T[2], alpha=.2)

    # Test add peloton
    add_ts = []
    for i in range(1, options.columns + 1):
        ts = []
        for _ in range(options.repeats):
            os.system(peloton_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, peloton_command, i))
        add_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(add_ts)

    add_ts = np.array(add_ts)
    ax.plot(np.arange(options.columns) + 1, add_ts.T[0], label='peloton')
    ax.fill_between(np.arange(options.columns) + 1,
                    add_ts.T[1], add_ts.T[2], alpha=.2)

    ax.set_xlabel('Added Columns')
    ax.set_ylabel('Time (seconds)')
    ax.legend(framealpha=0)
    f.savefig('add.png')

    # Test drop postgres
    drop_ts = []
    for i in range(1, options.columns + 1):
        ts = []
        for _ in range(options.repeats):
            os.system(postgres_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testDrop(options, postgres_command, i))
        drop_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(drop_ts)

    f, ax = plt.subplots(1)
    drop_ts = np.array(drop_ts)
    ax.plot(np.arange(options.columns) + 1, drop_ts.T[0], label='postgres')
    ax.fill_between(np.arange(options.columns) + 1,
                    drop_ts.T[1], drop_ts.T[2], alpha=.2)

    # Test drop peloton
    drop_ts = []
    for i in range(1, options.columns + 1):
        ts = []
        for _ in range(options.repeats):
            os.system(peloton_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testDrop(options, peloton_command, i))
        drop_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(drop_ts)

    f, ax = plt.subplots(1)
    drop_ts = np.array(drop_ts)
    ax.plot(np.arange(options.columns) + 1, drop_ts.T[0], label='peloton')
    ax.fill_between(np.arange(options.columns) + 1,
                    drop_ts.T[1], drop_ts.T[2], alpha=.2)

    ax.set_xlabel('Dropped Columns')
    ax.set_ylabel('Time (seconds)')
    ax.legend(framealpha=0)
    f.savefig('drop.png')

    # Test tuples postgres
    default_tuples = options.tuples
    tuple_ts = []
    for i in range(5):
        options.tuples = default_tuples / 5 * (i + 1)
        genTable(options)
        ts = []
        for _ in range(options.repeats):
            os.system(postgres_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, postgres_command, 1))
        tuple_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(tuple_ts)
    options.tuples = default_tuples
    genTable(options)

    f, ax = plt.subplots(1)
    tuple_ts = np.array(tuple_ts)
    ax.plot(np.arange(5) * default_tuples / 1000,
            tuple_ts.T[0], label='postgres')
    ax.fill_between(np.arange(5) * default_tuples / 1000,
                    tuple_ts.T[1], tuple_ts.T[2], alpha=.2)

    # Test tuples peloton
    default_tuples = options.tuples
    tuple_ts = []
    for i in range(5):
        options.tuples = default_tuples / 5 * (i + 1)
        genTable(options)
        ts = []
        for _ in range(options.repeats):
            os.system(peloton_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, peloton_command, 1))
        tuple_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(tuple_ts)
    options.tuples = default_tuples
    genTable(options)

    f, ax = plt.subplots(1)
    tuple_ts = np.array(tuple_ts)
    ax.plot(np.arange(5) * default_tuples /
            1000, tuple_ts.T[0], label='peloton')
    ax.fill_between(np.arange(5) * default_tuples / 1000,
                    tuple_ts.T[1], tuple_ts.T[2], alpha=.2)

    ax.set_xlabel('Tuples (Thousands)')
    ax.set_ylabel('Time (seconds)')
    ax.legend(framealpha=0)
    f.savefig('tuples.png')

    # Test columns postgres
    default_columns = options.columns
    column_ts = []
    for i in range(5):
        genTable(options)
        ts = []
        for _ in range(options.repeats):
            os.system(postgres_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, postgres_command, 1))
        column_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(column_ts)
        options.columns += 1
    options.columns = default_columns

    f, ax = plt.subplots(1)
    column_ts = np.array(column_ts)
    ax.plot(np.arange(5) + default_columns, column_ts.T[0], label='postgres')
    ax.fill_between(np.arange(5) + default_columns,
                    column_ts.T[1], column_ts.T[2], alpha=.2)

    # Test columns peloton
    default_columns = options.columns
    column_ts = []
    for i in range(5):
        genTable(options)
        ts = []
        for _ in range(options.repeats):
            os.system(peloton_command +
                      " < {} > /dev/null".format(options.filename))
            ts.append(testAdd(options, peloton_command, 1))
        column_ts.append([np.mean(ts), np.min(ts), np.max(ts)])
        # print(column_ts)
        options.columns += 1
    options.columns = default_columns

    f, ax = plt.subplots(1)
    column_ts = np.array(column_ts)
    ax.plot(np.arange(5) + default_columns, column_ts.T[0], label='peloton')
    ax.fill_between(np.arange(5) + default_columns,
                    column_ts.T[1], column_ts.T[2], alpha=.2)

    ax.set_xlabel('Columns')
    ax.set_ylabel('Time (seconds)')
    ax.legend(framealpha=0)
    f.savefig('columns.png')


if __name__ == "__main__":
    main()
