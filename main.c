#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PATH "data/"
#define FILECOUNT  18
const char *FILES[FILECOUNT] = {"100_1.csv","100_3.csv","100_5.csv","100_7.csv","100_2.csv","100_4.csv","100_6.csv","100.csv","cartron.csv","Guillaume_G.csv","HughB1.csv","Pablo.csv","d3nd3-o0.csv","HughB0.csv","HughB2.csv","HughB-nosteps1.csv","HughB-nosteps2.csv","HughB-nosteps3.csv"};
int EXPECTED_STEPS[FILECOUNT] = {100,      100,          100,       100,        100,         100,       100,        100,      150,          150,               150,         150,        150,           150,         150, 0, 0, 0};
// how much do wer care about these?
int HOWMUCH[FILECOUNT] = {1,      1,          1,       1,        1,         1,       1,        1,      1,          1,               1,         1,        1,           1,         1, 10, 10, 10};

#define DEBUG 0
#define STEPCOUNT_CONFIGURABLE

#include "../Espruino/libs/misc/stepcount.c"

typedef struct {
  short x,y,z;
} Vector3;

/// accelerometer data
Vector3 acc;
/// squared accelerometer magnitude
int accMagSquared, accMag;
uint32_t stepCounter = 0;

bool origStepWasLow;
/// How low must acceleration magnitude squared get before we consider the next rise a step?
int origStepCounterThresholdLow = (8192-80)*(8192-80);
/// How high must acceleration magnitude squared get before we consider it a step?
int origStepCounterThresholdHigh = (8192+80)*(8192+80);
int origStepCounter = 0;

void stepCount(int newx, int newy, int newz) {
  int dx = newx-acc.x;
  int dy = newy-acc.y;
  int dz = newz-acc.z;
  acc.x = newx;
  acc.y = newy;
  acc.z = newz;
  accMagSquared = acc.x*acc.x + acc.y*acc.y + acc.z*acc.z;
  // original step counter
  if (accMagSquared < origStepCounterThresholdLow)
    origStepWasLow = true;
  else if ((accMagSquared > origStepCounterThresholdHigh) && origStepWasLow) {
    origStepWasLow = false;
    origStepCounter++;
  }
  // Espruino step counter
  if (stepcount_new(accMagSquared))
    stepCounter++;
}


void testStepCount(char *filename, char *outfile) {
  // init
  origStepCounter = 0;
  origStepWasLow = 0;
  stepCounter  = 0;
  stepcount_init();
  // go
  char * line = NULL;
  size_t len = 0;
  int read;
  int n = 0;
  FILE *fp = fopen(filename, "r");
  FILE *fop = 0;
  if (outfile) {
    fop = fopen(outfile, "w");
    fprintf(fop, "n,x,y,z,scaled,filtered,origSteps,steps,thresh\n");
  }
  int x,y,z;
  bool first = true;
  while ((read = getline(&line, &len, fp)) != -1) {
    long time = strtol(strtok(line, ","), NULL, 10);
    x = (int)(strtol(strtok(NULL, ","), NULL, 10));
    y = (int)(strtol(strtok(NULL, ","), NULL, 10));
    z = (int)(strtol(strtok(NULL, ","), NULL, 10));
    if (first) {
      first = false;
      continue;
    }
    int origStepCounterP = origStepCounter;
    int stepCounterP = stepCounter;
    stepCount(x,y,z);
    if (fop) {
      int M = 50;
      int a = (origStepCounter-origStepCounterP)*20 + M; // old - high
      int b = -(stepCounter-stepCounterP)*20 - M; // new - low
      fprintf(fop, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", n++,x,y,z,accScaled>>1,accFiltered>>7,a,b,stepCounterThreshold>>15);
    }
  }
  // ensure we flush filter to get final steps out
  for (int i=0;i<ACCELFILTER_TAP_NUM;i++) {
    int origStepCounterP = origStepCounter;
    int stepCounterP = stepCounter;
    stepCount(x,y,z);
    if (fop) {
      int M = 50;
      int a = (origStepCounter-origStepCounterP)*20 + M; // old - high
      int b = -(stepCounter-stepCounterP)*20 - M; // new - low
      fprintf(fop, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", n++,x,y,z,accScaled>>1,accFiltered>>7,a,b,stepCounterThreshold>>15);
    }
  }
  if (fop) fclose(fop);
  fclose(fp);
  if (line)
    free(line);
}

static int testAll(bool outputFiles) {
  int fileCnt = 0;
  int differences = 0;
  while (fileCnt < FILECOUNT) {
    char buf[256], obuf[256];
    strcpy(buf, PATH);
    strcat(buf, FILES[fileCnt]);
    strcpy(obuf, buf);
    strcat(obuf, ".out.csv");
    if (outputFiles) printf("VVV %s\n", FILES[fileCnt]);
    testStepCount(buf, outputFiles ? obuf : NULL);
    if (outputFiles) printf("^^^ %s steps %d (orig %d, expected %d)\n\n", FILES[fileCnt], stepCounter, origStepCounter, EXPECTED_STEPS[fileCnt]);

    int d = stepCounter - EXPECTED_STEPS[fileCnt];
    differences += d*d*HOWMUCH[fileCnt];

    fileCnt++;
  }

  return differences;
}


void main() {
  int d = testAll(true);
  printf("TOTAL DIFFERENCE %d\n", int_sqrt32(d));
  // =======================
  // comment this out to brute-force over the data to find the best coefficients
  return;
  // =======================
  int bestDiff = 0xFFFFFFF;
  int best_stepCounterThresholdMin = 0;
  int best_stepCounterAvr = 0;

  for (stepCounterThresholdMin = STEPCOUNTERTHRESHOLD_MIN; stepCounterThresholdMin<=STEPCOUNTERTHRESHOLD_MAX; stepCounterThresholdMin+=STEPCOUNTERTHRESHOLD_STEP) {
    for (stepCounterAvr = STEPCOUNTERAVR_MIN; stepCounterAvr<=STEPCOUNTERAVR_MAX; stepCounterAvr+=STEPCOUNTERAVR_STEP) {
      printf("testing %d %d\n", stepCounterThresholdMin, stepCounterAvr);
      int d = testAll(false);
      if (d<bestDiff) {
        bestDiff = d;
        best_stepCounterThresholdMin = stepCounterThresholdMin;
        best_stepCounterAvr = stepCounterAvr;
      }
    }
  }

  printf("best difference %d\n", int_sqrt32(d));
  printf("stepCounterThresholdMin %d\n", best_stepCounterThresholdMin);
  printf("stepCounterAvr %d\n", best_stepCounterAvr);
}
