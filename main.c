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

/*

Accel
Integer
10
int8_t
int

*/
/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 12.5 Hz

fixed point precision: 10 bits

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 12.5 Hz

fixed point precision: 10 bits

* 0 Hz - 1.1 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 1.3 Hz - 2.5 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 2.7 Hz - 6.25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define ACCELFILTER_TAP_NUM 57

typedef struct {
  int8_t history[ACCELFILTER_TAP_NUM];
  unsigned int last_index;
} AccelFilter;

static int8_t filter_taps[ACCELFILTER_TAP_NUM] = {
  -2,
  4,
  4,
  1,
  -1,
  0,
  2,
  -3,
  -12,
  -13,
  2,
  24,
  29,
  6,
  -25,
  -33,
  -13,
  10,
  11,
  -1,
  3,
  29,
  41,
  4,
  -62,
  -89,
  -34,
  62,
  110,
  62,
  -34,
  -89,
  -62,
  4,
  41,
  29,
  3,
  -1,
  11,
  10,
  -13,
  -33,
  -25,
  6,
  29,
  24,
  2,
  -13,
  -12,
  -3,
  2,
  0,
  -1,
  1,
  4,
  4,
  -2
};

void AccelFilter_init(AccelFilter* f) {
  int i;
  for(i = 0; i < ACCELFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void AccelFilter_put(AccelFilter* f, int8_t input) {
  f->history[f->last_index++] = input;
  if(f->last_index == ACCELFILTER_TAP_NUM)
    f->last_index = 0;
}

int AccelFilter_get(AccelFilter* f) {
  int acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < ACCELFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : ACCELFILTER_TAP_NUM-1;
    acc += (int)f->history[index] * (int)filter_taps[i];
  };
  return acc >> 2; // MODIFIED - was 10. Now returns 8 bits of fractional data
}

int AccelFilter_getHistory(AccelFilter* f, int index) {
  index = f->last_index - index;
  while (index<0) index += ACCELFILTER_TAP_NUM;
  return f->history[index];
}

AccelFilter accelFilter;

// ===============================================================

typedef struct {
  short x,y,z;
} Vector3;

/// accelerometer data
Vector3 acc;
/// squared accelerometer magnitude
int accMagSquared, accMag;
int accScaled;
int accFiltered;
/// accelerometer difference since last reading
int accdiff;

bool origStepWasLow;
/// How low must acceleration magnitude squared get before we consider the next rise a step?
int origStepCounterThresholdLow = (8192-80)*(8192-80);
/// How high must acceleration magnitude squared get before we consider it a step?
int origStepCounterThresholdHigh = (8192+80)*(8192+80);
int origStepCounter = 0;

#define STEPCOUNTERTHRESHOLD_MIN 1000
#define STEPCOUNTERTHRESHOLD_MAX 6000
#define STEPCOUNTERTHRESHOLD_STEP 20
int stepCounterThresholdMin = 1600;

#define STEPCOUNTERAVR_MIN 0
#define STEPCOUNTERAVR_MAX 0
#define STEPCOUNTERAVR_STEP 1
int stepCounterAvr = 1;



int stepCounterThreshold;
/// Current steps since reset
uint32_t stepCounter = 0;
/// has acceleration counter passed stepCounterThresholdLow?
bool stepWasLow;

// quick integer square root
// https://stackoverflow.com/questions/31117497/fastest-integer-square-root-in-the-least-amount-of-instructions
unsigned short int int_sqrt32(unsigned int x)
{
    unsigned short int res=0;
    unsigned short int add= 0x8000;   
    int i;
    for(i=0;i<16;i++)
    {
        unsigned short int temp=res | add;
        unsigned int g2=temp*temp;      
        if (x>=g2)
        {
            res=temp;           
        }
        add>>=1;
    }
    return res;
}

void stepCount(int newx, int newy, int newz) {
  int dx = newx-acc.x;
  int dy = newy-acc.y;
  int dz = newz-acc.z;
  acc.x = newx;
  acc.y = newy;
  acc.z = newz;
  accMagSquared = acc.x*acc.x + acc.y*acc.y + acc.z*acc.z;
  accMag = int_sqrt32(accMagSquared);
  int v = (accMag-8192)>>5;
  //printf("v %d\n",v);
  //if (v>127 || v<-128) printf("Out of range %d\n", v);
  if (v>127) v = 127;
  if (v<-128) v = -128;
  accScaled = v;
  AccelFilter_put(&accelFilter, v);
  accFiltered = AccelFilter_get(&accelFilter);
  accdiff = dx*dx + dy*dy + dz*dz;
  // origibal step counter
  if (accMagSquared < origStepCounterThresholdLow)
    origStepWasLow = true;
  else if ((accMagSquared > origStepCounterThresholdHigh) && origStepWasLow) {
    origStepWasLow = false;
    origStepCounter++;
  }

  // update threshold
  if (stepCounterAvr) {
    int a = (AccelFilter_getHistory(&accelFilter, (ACCELFILTER_TAP_NUM/2)-2) << 12) * 3;
    if (a<0) a=-a;
    if (a > stepCounterThreshold) 
      stepCounterThreshold = a;//(stepCounterThreshold+a) >> 1;
    stepCounterThreshold -= 256*64;
    if (stepCounterThreshold < stepCounterThresholdMin<<8)
      stepCounterThreshold = stepCounterThresholdMin<<8;
  }

  // check for step counter
  int t = stepCounterThreshold>>8;
  if (accFiltered < -t)
    stepWasLow = true;
  else if ((accFiltered > t) && stepWasLow) {
    stepWasLow = false;   

    stepCounter++;
    if (DEBUG>1) printf("step %d \n", stepCounter);
  }


}

void testStepCount(char *filename, char *outfile) {
  // init
  origStepCounter = 0;
  origStepWasLow = 0;
  stepCounter  = 0;
  stepWasLow = 0;
  stepCounterThreshold = stepCounterThresholdMin<<8;
  AccelFilter_init(&accelFilter);
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
