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
#include <omp.h>

double testFBNN(FBNN *net, float **data);

int main(){
    srand(time(0));
    float **data = (float **)malloc(TSTEPS * sizeof(float *));
    for(int i=0; i<TSTEPS; i++){
        data[i] = (float *)malloc(205 * sizeof(float));
    }
    
    // setup initial generation
    FBNN* gen[GEN_SIZE];
    for(int i=0; i<GEN_SIZE; i++){
        gen[i] = initNet();
    }

    // setup variables needed to track each gen
    double scores[GEN_SIZE];
    for(int genNum=0; genNum<NUM_GENS; genNum++){
        printf("TESTING GEN %d\n",genNum);
        // test all networks
        #pragma omp parallel
        {
            #pragma omp for
            for(int i=0; i<GEN_SIZE; i++){
                printf("TESTING SPECIES %d\n",i);
                scores[i] = 0;
                for(int j=0; j<5; j++){
                    scores[i] += testFBNN(gen[i], data);
                }
            }
        }
        
        // find the best 2 scores
        double s1, s2;
        int i1, i2;
        if(scores[0]>scores[1]){
            s1 = scores[0]; i1 = 0;
            s2 = scores[1]; i2 = 1;
        }
        else{
            s1 = scores[1]; i1 = 1;
            s2 = scores[0]; i2 = 0;
        }
        for(int i=2; i<GEN_SIZE; i++){
            if(scores[i]>s1){
                s2 = s1; i2 = i1;
                s1 = scores[i]; i1 = i;
            }
            else if(scores[i]>s2){
                s2 = scores[i]; i2 = i;
            }
        }
        
        printf("BEST SCORES %f, %f\n",s1,s2);

        // move best networks to first two indices
        FBNN* n1 = gen[i1];
        FBNN* n2 = gen[i2];

        // free all but the best two networks
        for(int i=0; i<GEN_SIZE; i++){
            if(i!=i1 && i!=i2){
                freeFBNN(gen[i]);
            }
        }
        
        // move the saved pointers to the best networks into the first two indices
        gen[0] = n1;
        gen[1] = n2;

        // make a new generation
        for(int i=2; i<GEN_SIZE; i++){           
            gen[i] = spawnNet(gen[0], gen[1]);
        }
    }
    // run the best network one more time
    double finalScore = testFBNN(gen[0], data);
    
    char buf[] = "data.csv";
    writeToFile(data, buf);
    
    // free the next gen and data
    for(int i=0; i<GEN_SIZE; i++){
        freeFBNN(gen[i]);
    }
    for(int i=0; i<TSTEPS;i++){
        free(data[i]);
    }
    free(data);
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
            env.compartments[i]->vext = electrode * extracellular[i] * 50000.0 * 100.0;
        }

        // take a single time step (.01ms)
        takeTimeStep(env, .01, inject, timeSteps, data);

        // Scoring logic
        // not blocking an AP is -10 points
        // equaling charges is +100 points
        if(!firing && env.compartments[50]->v > 0){
            firing = true;
            score -= 10;
        }
        else if(firing && env.compartments[50]->v < 0){
            firing = false;
        }

        if(charge<0 && electrode>(charge*-1.0)){   // if there was a negative charge
            score += 100;
        }
        else if(charge>0 && (electrode*-1.0)>charge){
            score += 100;
        }
        charge += electrode;
    }

    for(int i=0; i<51; i++){
        free(env.compartments[i]);
    }

    return score;
}
