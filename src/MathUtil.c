#pragma once
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "MathUtil.h"

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

float fastSigmoidBounded(float x, float lowerBound, float upperBound){
    // use sigmoid activation to scale x between lower and upper bounds
    // see fastSigmoid for reason of calculation
    return (upperBound-lowerBound) * x / (1.0f + fabs(x)) + lowerBound;
}

float gaussGenerator(){
    // return a float with a gaussian distribution
    float u, v, z;
    u = (rand() + 1.0f) / (RAND_MAX + 2.0f);
    v = rand() / (RAND_MAX + 1.0f);
    z = sqrt(-2.0f * log(u)) * sin(2 * PI * v);
    return z;
}

float gaussActivation(float x){
    // returns the gaussian activation function of a float
    // g(x) = e^(-x^2)
    return powf(e, -1.0f*x*x);
}
