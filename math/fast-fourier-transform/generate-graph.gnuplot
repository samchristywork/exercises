#!/usr/bin/gnuplot

set terminal png size 800, 600
set output 'output/graph.png'
set title 'FFT Comparison'
set xlabel 'Index'
set ylabel 'Magnitude'
set grid

magnitude(x, y) = sqrt(x**2 + y**2)

plot 'output/a.txt' using ($0):(magnitude($1, $2)) with lines title 'File A', \
     'output/b.txt' using ($0):(magnitude($1, $2)) with lines title 'File B'
