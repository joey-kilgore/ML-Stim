// TestFeedback.c uses the Feedback NN defined in FeedbackNN.h and .c to progressively learn and
// attempt to block action potentials. This is a more advanced version of the Controller.c for
// multiple reasons:
//  The Feedback NN has much more customizability (variable hidden layers, and inputs)
//  Feedback NN theoretically will give more 'wave-like' output compared to a traditional NN
//  The NN and model interface is much closer to standard simulations (with a point source
//      generating an electric field)
#include "defs.h"
#include "FeedbackNN.h"
#include "Sim.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double testFBNN(FBNN *net, float **data);

int main(){
    srand(time(0));
    float **data = (float **)malloc(TSTEPS * sizeof(float *));
    for(int i=0; i<TSTEPS; i++){
        data[i] = (float *)malloc(205 * sizeof(float));
    }

    FBNN* net;
    net = initNet();

    double score = testFBNN(net, data);
    printf("SCORE %f\n",score);

    char buf[] = "data.csv";
    writeToFile(data, buf);
}

double testFBNN(FBNN *net, float **data){
    // Setup the environment
    double score = 0;
    struct simEnv env;
    for(int i=0; i<51; i++){
        // set initial conditions
        env.compartments[i] = (struct compartment *)malloc(sizeof(struct compartment));
        env.compartments[i]->v = -65.0;
        env.compartments[i]->m = 0.052;
        env.compartments[i]->h = 0.596;
        env.compartments[i]->n = 0.317;
        env.compartments[i]->vext = 0;
    }
    for(int i=0; i<51; i++){
        // link compartments
        env.compartments[i]->left = (i>0) ? env.compartments[i-1] : NULL;
        env.compartments[i]->right = (i<50) ? env.compartments[i+1] : NULL;
    }

    float inject[51];
    for(int i=0; i<51; i++){
        // setup inject current
        inject[i] = 0.0;
    }
    inject[0] = 10.0;   // only the first node will have an injected current

    float extracellular[51];    // used to precalculate the drop in voltage due to
                                //  distance for all nodes
    for(int i=0; i<51; i++){
        extracellular[i] = 1.0/(4.0*3.14159265*sqrt(1000000.0 + ((float)i-25.0)*((float)i-25.0)*1000000.0));
    }
    
    // Run the trial
    bool firing = false;
    float charge = 0;
    float* output;
    float electrode;
    for(int timeSteps=0; timeSteps<TSTEPS; timeSteps++)
    {   // calculate the stimulus voltage
        output = calcOutput(net);
        float electrode = output[0];
        for(int i=0; i<51; i++){
            // calculate extracellular voltage for electrode placed above center node
            // assume nodes are 1mm apart, and the elctrode is 1mm from the axon
            env.compartments[i]->vext = electrode * extracellular[i] * 50000.0 * 50.0;
        }

        // take a single time step (.01ms)
        takeTimeStep(env, .01, inject, timeSteps, data);

        // Scoring logic
        // not blocking an AP is -100 points
        // equaling charges is +.01 points
        if(!firing && env.compartments[50]->v > 0){
            firing = true;
            score -= 100;
        }
        else if(firing && env.compartments[50]->v < 0){
            firing = false;
        }

        if(charge<0 && electrode>(charge*-1.0)){   // if there was a negative charge
            printf("%f,%f\n",charge,electrode);
            score += .01;
        }
        else if(charge>0 && (electrode*-1.0)>charge){
            printf("%f,%f\n",charge,electrode);
            score += .01;
        }
        charge += electrode;
        if(timeSteps%100 == 0)
            printf("%d,%f\n",timeSteps,charge);
    }

    return score;
}
