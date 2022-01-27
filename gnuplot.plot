set datafile separator ","
set lmargin 5
set rmargin 5

# You can comment some of these out with hash

set term wxt 0
set title "HughB-walk-2350"
show title
plot "data/HughB-walk-2350.csv.out.csv" using 5 title 'accel' with lines, \
     "" using 7 title 'orig' with lines, \
     "" using 6 title 'filtered' with lines, \
     "" using 9 title 'thresh' with lines, \
     "" using 8 title 'current' with lines

#set term wxt 1
#set title "HughB-nosteps1"
#show title
#plot "data/HughB-nosteps1.csv.out.csv" using 5 title 'accel' with lines, \
#     "" using 6 title 'filtered' with lines, \
#     "" using 7 title 'orig' with lines, \
#     "" using 9 title 'thresh' with lines, \
#     "" using 8 title 'current' with lines

#set term wxt 2
#set title "HughB-nosteps2"
#show title
#plot "data/HughB-nosteps2.csv.out.csv" using 5 title 'accel' with lines, \
#     "" using 6 title 'filtered' with lines, \
#     "" using 7 title 'orig' with lines, \
#     "" using 9 title 'thresh' with lines, \
#     "" using 8 title 'current' with lines

#set term wxt 3
#set title "HughB-driving-36min"
#show title
#plot "data/HughB-driving-36min.csv.out.csv" using 5 title 'accel' with lines, \
#     "" using 7 title 'orig' with lines, \
#     "" using 6 title 'filtered' with lines, \
#     "" using 9 title 'thresh' with lines, \
#     "" using 8 title 'current' with lines

#set term wxt 4
#set title "100_5"
#show title
#plot "data/100_5.csv.out.csv" using 5 title 'accel' with lines, \
#     "" using 7 title 'orig' with lines, \
#     "" using 6 title 'filtered' with lines, \
#     "" using 9 title 'thresh' with lines, \
#     "" using 8 title 'current' with lines
