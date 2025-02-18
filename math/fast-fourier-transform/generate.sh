#!/bin/bash

mkdir -p output/

signals=("dc" "noise" "sine" "square")
for signal in "${signals[@]}"; do
  ./fft < samples/$signal.txt > output/$signal.fft
  ./reference-implementation < samples/$signal.txt > output/$signal.reference

  a_file="output/$signal.fft"
  b_file="output/$signal.reference"

  gnuplot << EOF
  set terminal png size 800, 600
  set output 'output/$signal.png'
  set title 'FFT Comparison for "$signal"'
  set xlabel 'Index'
  set ylabel 'Magnitude'
  set grid

  magnitude(x, y) = sqrt(x**2 + y**2)

  plot '$a_file' using (\$0):(magnitude(\$1, \$2)) with lines title '$a_file', \
       '$b_file' using (\$0):(magnitude(\$1, \$2)) with lines title '$b_file'
EOF
done
