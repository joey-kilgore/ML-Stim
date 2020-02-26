// Simulation environment
#include "defs.h"
#include "Sim.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fenv.h>
#include <omp.h>

void takeTimeStep(struct simEnv sim, float dt, float injected[], int rowNum, float **data)
{   // Taking a time step requires doing aproximation for all variables governed
    //  by first order ODE's
    // This is done by using backward euler method

    //printf("\nTIME STEP : %f\n",dt);
    // Loop through all variables and calculate forward euler approx
    for (int i = 0; i < 51; i++)
    {
        //printf("compartment : %d\n",i);
        struct compartment *current = sim.compartments[i];

        // values for voltage at internal node need to be in reference to ground
        //float vinLeft = (current->left) ? current->left->v + current->left->vext : current->v + current->vext;
        //float vinRight = (current->right) ? current->right->v + current->right->vext : current->v + current->vext;
        float vinLeft = (current->left) ? current->left->v : current->v;
        float vinRight = (current->right) ? current->right->v : current->v;


        // forward euler all vars to get the next point
        //current->fev = current->v + dt * derivV(current->v, current->m, current->h, current->n, injected[i], current->vext + current->v, vinLeft, vinRight);
        
        current->fev = current->v + dt * derivV(current->v, current->m, current->h, current->n, injected[i], current->v, vinLeft, vinRight, current->vext);
        current->fen = current->n + dt * derivN(current->v, current->n);
        current->fem = current->m + dt * derivM(current->v, current->m);
        current->feh = current->h + dt * derivH(current->v, current->h);
    }

    // Loop through all variables and calculate backward euler approx
    for (int i = 0; i < 51; i++)
    {
        struct compartment *current = sim.compartments[i];

        // values for voltage at internal node need to be in reference to ground
        //float vinLeft = (current->left) ? current->left->v + current->left->vext : current->v + current->vext;
        //float vinRight = (current->right) ? current->right->v + current->right->vext : current->v + current->vext;
        float vinLeft = (current->left) ? current->left->v : current->v;
        float vinRight = (current->right) ? current->right->v : current->v;

        // do backward euler approximation (using the values approximated from forward euler)
        //current->bev = current->v + dt * derivV(current->fev, current->fem, current->feh, current->fen, injected[i], current->vext + current->fev, vinLeft, vinRight);
        current->bev = current->v + dt * derivV(current->fev, current->fem, current->feh, current->fen, injected[i], current->fev, vinLeft, vinRight, current->vext);
        
        current->ben = current->n + dt * derivN(current->fev, current->fen);
        current->bem = current->m + dt * derivM(current->fev, current->fem);
        current->beh = current->h + dt * derivH(current->fev, current->feh);
    }

    // Copy the values from backward euler into data buffer and the original variables 
    data[rowNum][0] = rowNum*dt; 
    for (int i = 0; i < 51; i++)
    {
        struct compartment *current = sim.compartments[i];

        // set normal array to the backward euler array
        current->v = current->bev;
        data[rowNum][1+i] = current->v;
        current->n = current->ben;
        data[rowNum][52+i] = current->n;
        current->m = current->bem;
        data[rowNum][103 + i] = current->m;
        current->h = current->beh;
        data[rowNum][154 + i] = current->h;
        data[rowNum][205 + i] = current->vext;
    }
}


float derivV(float v, float m, float h, float n, float i, float vinCenter, float vinLeft, float vinRight, float vExt)
{
    // calculates the voltage derivative given current state values
    //dv/dt=1/cm*(i -          ina                            -       ik                             -     il             +            ileft              +        iright)
    //return (i - 120.0F * m*m*m * h * (v - 55.17F) - 36.0F * n*n*n*n * (v + 72.14F) - .3F * (v + 49.42F) + (vinLeft - vinCenter) / 1.0F + (vinRight - vinCenter) / 1.0F);

    float ret = (i - 1.25f * m*m*m * h * (v - vExt - 45.0f) - .36f * n*n*n*n * (v - vExt + 82.0f) - .3f * (v - vExt + 70.0f) + (vinLeft - vinCenter) / 1.0f + (vinRight - vinCenter) / 1.0f);
    if(v > 10000.0f || m == 0.0f || n < .0001f){
        printf("derivV calc FAILED\n");
        printf("v : %f\n", v);
        printf("m : %f\n", m);
        printf("h : %f\n", h);
        printf("n : %f\n", n);
        printf("i : %f\n", i);
        printf("vinCenter : %f\n", vinCenter);
        printf("vinLeft : %f\n", vinLeft);
        printf("vinRight : %f\n", vinRight);
        exit(1);
    }
    /*
    printf("v : %f\n", v);
    printf("m : %f\n", m);
    printf("h : %f\n", h);
    printf("n : %f\n", n);
    printf("i : %f\n", i);
    printf("vinLeft : %f\n", vinLeft);
    printf("vinCenter : %f\n", vinCenter);
    printf("vinRight : %f\n", vinRight);
    printf("ret : %f\n", ret);
    */
    return ret;
}
float derivN(float v, float n)
{
    // calculate the derivative of n given state values
    //float alpha = (.01F * (10.0f - v) / (expf(-.1F * (10.0f - v)) - 1.0f));
    //if(v==10.0f){
    //    alpha = 0.1f;
    //}
    //float beta = .125f * expf(-1.0f * v / 80.0f);
    //return (alpha * (1.0f - n)) - (beta * n);
    float alpha = 0.01f * vtrap(-1.0f * (v + 55.0f), 10.0f);
    float beta = 0.125f * expf(-1.0f * (v + 65.0f) / 80.0f);
    float sum = alpha + beta;
    float ntau = 1.0f / sum;
    float ninf = alpha / sum;
    return (ninf - n) / ntau;
}
float derivM(float v, float m)
{
    // calculate the derivative of m given state values
    //float alpha = (v!=25.0f) ? 0.1f * (25.0f - v) / (expf(0.1f * (25.0f - v)) - 1.0f) : 1.0f;
    //float beta = 4.0f * expf(-1.0f * v / 18.0f);
    //return (alpha * (1.0f - m)) - (beta * m);
    float alpha = 0.1f * vtrap(-1.0f * (v + 40.0f), 10.0f);
    float beta = 4.0f * expf(-1.0f * (v + 65.0f) / 18.0f);
    float sum = alpha + beta;
    float mtau = 1.0f / sum;
    float minf = alpha / sum;
    return (minf - m) / mtau;
}
float derivH(float v, float h)
{
    // calculate the derivative of h given state values
    //dh/dt=             alpha(v)             * (1-h)      -      beta(v)*h
    //return (.07F * expf(-.05F * (v + 65.0F))) * (1.0F - h) - 1.0F / (1.0F + expf(-.1F * (v + 35.0F))) * h;
    //float alpha = 0.07f * expf(-1.0f * v / 20.0f);
    //float beta = 1.0f / (expf((30.0f - v) / 10.0f) + 1.0f);
    //return (alpha * (1.0f - h)) - (beta * h);
    float alpha = 0.07f * expf(-1.0f * (v + 65.0f) / 20.0f);
    float beta = 1.0f / (expf(-1.0f * (v + 35.0f) / 10.0f) + 1.0f);
    float sum = alpha + beta;
    float htau = 1.0f / sum;
    float hinf = alpha / sum;
    return (hinf - h) / htau;
}

float vtrap(float x, float y)
{   // there are specific values that can cause the system of equations to have 0 denominators
    //  this method solves that problem by checking if an error would occur
    if(fabsf(x/y) < 0.000001f){
        return y * (1.0f - (x / y / 2));
    }else{
        return x / (expf(x / y) - 1);
    }
}

void writeToFile(float** data, char* name)
{
    FILE* f = fopen(name, "w");
    for(int r = 0; r < TSTEPS; r++){
        for(int c = 0; c<256; c++){
            fprintf(f, "%.4f,", data[r][c]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
