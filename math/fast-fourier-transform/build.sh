#!/bin/bash

gcc main.c -o fft -lm
gcc reference-implementation.c -o reference-implementation -lfftw3
