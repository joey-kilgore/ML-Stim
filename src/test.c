#include "defs.h"
#include "Sim.h"
#include <stdlib.h>
#include <stdio.h>
#include "ML.h"
#include "FeedbackNN.h"
#include <time.h>

void testNet();
void testEnv();
void testFeedback();

int main(){
    srand(time(0));
    //testNet();
    //testEnv();
    testFeedback();
}

void testNet(){
    NeuralNetwork net;
    initNN(&net);
    calcFwd(&net);
    printf("%f\n", net.vExt[0]);
    NeuralNetwork net2;
    createVar(&net, &net2);
    calcFwd(&net2);
    printf("%f\n", net2.vExt[0]);
    printf("%f\n", net2.vExt[3]);
    printf("%f\n", net2.vExt[6]);
    printf("\n");
    printf("N1 - %f\n", net.outWeightVector[0][0]);
    printf("N1 - %f\n", net2.outWeightVector[0][0]);

    struct simEnv env;
}

void testEnv(){
    struct simEnv env;
    float **data = (float **)malloc(TSTEPS * sizeof(float *));
    for(int i=0; i<TSTEPS; i++)
        data[i] = (float *)malloc(205 * sizeof(float));

    for(int i=0; i<51; i++){
        // set initial conditions for each compartment
        // init v=-65,m=.052,h=.596,n=.317
        // env.compartments[i];
        env.compartments[i] = (struct compartment *)malloc(sizeof(struct compartment));
        env.compartments[i]->v = -65;
        env.compartments[i]->m = .052;
        env.compartments[i]->h = .596;
        env.compartments[i]->n = .317;
        env.compartments[i]->vext = 0;
    }
    for(int i=0; i<51; i++){
        env.compartments[i]->left = (i>0) ? env.compartments[i-1] : NULL;
        env.compartments[i]->right = (i<50) ? env.compartments[i+1] : NULL;
    }

    float inject[51];
    for(int i=0; i<51; i++){
        inject[i] = 0.0;
    }
    float testPulse[51];
    for(int i=0; i<51; i++){
        testPulse[i] = 0.0;
    }
    testPulse[0] = 10;

    for(int timeSteps=0; timeSteps<10000; timeSteps++){
        if(timeSteps>4000 && timeSteps<4100){
            takeTimeStep(env, .01, testPulse, timeSteps, data);
        }else{
           takeTimeStep(env, .01, inject, timeSteps, data);
        }
    }
    printf("WRITING TO FILE");
    writeToFile(data, "output.csv");
}

void testFeedback(){
    FBNN* net;
    net = initNet();
    printf("NET 1\n");
    for(int testNum=0; testNum<10; testNum++){
        float* out = calcOutput(net);
        printf("OUTPUT %d: %f\n", testNum, out[0]);
    }
    FBNN* net2;
    net2 = initNet();
    printf("NET 2\n");
    for(int testNum=0; testNum<10; testNum++){
        float* out = calcOutput(net2);
        printf("OUTPUT %d: %f\n", testNum, out[0]);
    }
    FBNN* net3;
    net3 = spawnNet(net, net2);
    printf("NET 3\n");
    for(int testNum=0; testNum<10; testNum++){
        float* out = calcOutput(net3);
        printf("OUTPUT %d: %f\n", testNum, out[0]);
    }
}
