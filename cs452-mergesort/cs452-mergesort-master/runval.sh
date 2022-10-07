#!/bin/bash
valgrind --leak-check=full ./test-mergesort 10000 1 1234
