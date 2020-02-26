#pragma once

#define PI 3.141592654f
#define e  2.718281828f

float linearGenerator();
float fastSigmoid(float x);
float fastSigmoidBounded(float x, float lowerBound, float upperBound);
float gaussGenerator();
float gaussActivation(float x);
