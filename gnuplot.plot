set datafile separator ","
set lmargin 5
set rmargin 5

# You can comment some of these out with hash

set term wxt 0
plot "data/d3nd3-o0.csv.out.csv" using 5 title 'accel' with lines, \
     "" using 7 title 'orig' with lines, \
     "" using 6 title 'filtered' with lines, \
     "" using 9 title 'thresh' with lines, \
     "" using 8 title 'current' with lines

set term wxt 1
plot "data/HughB-nosteps1.csv.out.csv" using 5 title 'accel' with lines, \
     "" using 6 title 'filtered' with lines, \
     "" using 7 title 'orig' with lines, \
     "" using 9 title 'thresh' with lines, \
     "" using 8 title 'current' with lines

set term wxt 2
plot "data/HughB-nosteps2.csv.out.csv" using 5 title 'accel' with lines, \
     "" using 6 title 'filtered' with lines, \
     "" using 7 title 'orig' with lines, \
     "" using 9 title 'thresh' with lines, \
     "" using 8 title 'current' with lines

set term wxt 3
plot "data/100_3.csv.out.csv" using 5 title 'accel' with lines, \
     "" using 7 title 'orig' with lines, \
     "" using 6 title 'filtered' with lines, \
     "" using 9 title 'thresh' with lines, \
     "" using 8 title 'current' with lines
