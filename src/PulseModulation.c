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
#include "MathUtil.h"
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
        data[i] = (float *)malloc(256 * sizeof(float));
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
            if(i < GEN_SIZE/2){
                gen[i] = spawnNet(gen[0], gen[1]);   
            }
            else if(i < GEN_SIZE*3/4){
                gen[i] = mutateNet(gen[0]);
            }
            else{
                gen[i] = mutateNet(gen[1]);
            }
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
        env.compartments[i]->v = -72.0;
        env.compartments[i]->m = 0.025;
        env.compartments[i]->h = 0.810;
        env.compartments[i]->n = 0.22;
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
    //inject[0] = 1000.0;   // only the first node will have an injected current

    float extracellular[51];    // used to precalculate the drop in voltage due to
                                //  distance for all nodes
    for(int i=0; i<51; i++){
        extracellular[i] = 1.0/(4.0*3.14159265* (1000000.0 + ((float)i-25.0)*((float)i-25.0)*1000000.0));
    }
    
    // Run the trial
    int phasesPerChargeBalance = 10;    // pulse modulation parameters
    int phasesSinceBalance = 10;
    int phasePolarity = -1;
    float phaseDuration = 0.0;
    float phaseFrequency = 0.0;
    float phaseAmplitude = 0.0;
    float phaseCharge = 0.0;
    float startPhase = 0.0;     // time when the phase started
    float endPhase = 0.0;       // time when the phase will end

    bool firing = false;    // environment params
    float charge = 0.0;     // cumulative charge
    float* output;          // pointer to output from FBNN            
    float electrode;
    float fires = 0.0;
    float balances = 0.0;

    float currentTime;
    int timeSteps;
    for(timeSteps=0, currentTime=0.0; timeSteps<TSTEPS; timeSteps++, currentTime+=TIME_STEP)
    {   // calculate the stimulus voltage
        // to do this we need to tell if we have to either continue calculating the phase, or start a new phase
        inject[0] = (fmod(currentTime, 10.0f) < 1.0f) ? 100.0f : 0.0f;
        if(currentTime >= endPhase){
            // we need to calculate a new phase
            phasePolarity *= -1;
            if(phasesSinceBalance >= phasesPerChargeBalance){   // after a set number of unbalanced phases
                // we must check if the next phase is the correct polarity to balance the wave
                if(phasePolarity<0 && charge>0 || phasePolarity>0 && charge<0){
                    phaseCharge = -1*charge;                        // we will force the next phase to balance 
                    phaseDuration = BALANCE_WIDTH;
                }else{  // if we cannot balance with this polarity we will wait for the next phase
                    output = calcOutput(net);   // the output parameters are defined for this simulation as:
                    phaseDuration = fastSigmoidBounded(output[1],MIN_PULSE_WIDTH,MAX_PULSE_WIDTH); // output[1] -> phase duration
                    phaseCharge = gaussActivation(output[2]) * phasePolarity * MAX_CHARGE;  // output[2] -> charge of phase
                }
            }else{                          // if we do not need to force charge balance then we can use the FBNN
                output = calcOutput(net);   // the output parameters are defined for this simulation as:
                phaseDuration = fastSigmoidBounded(output[1],MIN_PULSE_WIDTH,MAX_PULSE_WIDTH); // output[1] -> phase duration
                phaseCharge = gaussActivation(output[2]) * phasePolarity * MAX_CHARGE;  // output[2] -> charge of phase
            }                                                                           // output[0] -> feedback parameter

            phaseDuration = 0.1;
            startPhase = endPhase;                      // calc all other phase params from the main two
            endPhase += phaseDuration;
            phaseFrequency = 2.0f/phaseDuration;
            phaseCharge = MAX_CHARGE;
            phaseAmplitude =phasePolarity * phaseCharge/phaseDuration * 1000000.0f;

            // process cumulitive charge params
            if((charge<0 && charge+phaseCharge>=0) || (charge>0 && charge-phaseCharge<=0)){
                // if the new phase will rebalance the charge then we can reset out unbalanced count
                phasesSinceBalance = 0;
            }else{
                // otherwise we must increment out unbalance counter
                phasesSinceBalance++;
            }
            charge += phaseCharge;  // update the cumulative charge
        }
        // now we calculate the electrode value for the given time step
        electrode = phaseAmplitude * sin(phaseFrequency*(currentTime - startPhase));

        for(int i=0; i<51; i++){
            // calculate extracellular voltage for electrode placed above center node
            // assume nodes are 1mm apart, and the elctrode is 1mm from the axon
            env.compartments[i]->vext = electrode * extracellular[i];
        }

        // take a single time step (.01ms)
        takeTimeStep(env, TIME_STEP, inject, timeSteps, data);

        // Scoring logic
        // Blocking and charge balancing are two metrics to optimize
        // during simulation each is tracked and combined at the final score
        if(!firing && env.compartments[50]->v > 0){
            firing = true;
            fires += 1.0;
        }
        else if(firing && env.compartments[50]->v < 0){
            firing = false;
        }
        score += 1 - (env.compartments[25]->m*env.compartments[25]->m*env.compartments[25]->m*env.compartments[25]->h);
        
    }

    for(int i=0; i<51; i++){
        free(env.compartments[i]);
    }
    // scores = (#blocks * #balances) + (#balances) + (#blocks)
    score += (MAX_FIRES - fires) * balances + balances + (MAX_FIRES - fires);

    return score;
}
