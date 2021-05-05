Simple step counter test
========================

* In `data` there's some step data from http://forum.espruino.com/conversations/359542
* On `main.c` we have some step counter code which can be run on

## Running

Type `make && ./main` to run and execute the file.

Total difference in steps between expected and calculated is shown. 

* `expected` = what we expect
* `orig` = step count based on the most trivial step counter

You can also comment out the `return` line in `void main()` to brute-force over all available coefficients to find the best ones.

## Graphs

With `gnuplot` installed you can run `gnuplot -p gnuplot.plot` to plot graphs of acceleration vs step count/theshold data for debugging.

