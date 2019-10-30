#include <math.h>
#include <time.h>
#include <stdio.h>

float linearGenerator(){
    // return a float between -1 and 1 with a linear distrubution
    return (2.0f * (float)rand()/(float)RAND_MAX) -1.0f;
}

float fastSigmoid(float x){
    // uses the sigmoid activation function to scale x between -1 and 1
    // see the discussion https://stackoverflow.com/questions/10732027/fast-sigmoid-algorithm
    x = x / (1.0f + fabs(x));
    return x;
}
