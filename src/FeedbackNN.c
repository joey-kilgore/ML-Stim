#include "FeedbackNN.h"
#include <stdlib.h>
#include "MathUtil.c"

FBNN* initNet(){
    // initNet essentially goes through and sets up each layer
    // Allocating memory for a layer consists of
    //  - setting pointers
    //  - weight, bias, and node arrays
    //  - setting numNodes
    // finally the feeback array needs to be set to zeros

    FBNN* net = (FBNN*)malloc(sizeof(FBNN));
    
    // Setting up input layer
    net->inputLayer = (Layer*)malloc(sizeof(Layer));
    net->inputLayer->node = (float *)malloc(INPUTS * sizeof(float));
    for(int i=0; i<INPUTS; i++) net->inputLayer->node[i] = 0.0f;
    net->inputLayer->numNodes = INPUTS;

    // build the hidden layers
    Layer* preLayer = net->inputLayer;
    Layer* hiddenLayer;
    for(int i=0; i<HIDDEN_LAYERS; i++){
        Layer* hiddenLayer = (Layer *)malloc(sizeof(Layer));    // allocate layer      
        hiddenLayer->prevLayer = preLayer; // Set links to connect layers
        preLayer->nextLayer = hiddenLayer;

        // Allocate float arrays
        hiddenLayer->weight = malloc(preLayer->numNodes * sizeof(float *));
        for(int j=0; j<preLayer->numNodes; j++){
            hiddenLayer->weight[j] = (float *)malloc(HIDDEN * sizeof(float));
            for(int l=0; l<HIDDEN; l++){
                hiddenLayer->weight[j][l] = linearGenerator();
            }
        }
        
        hiddenLayer->bias = (float *)malloc(HIDDEN * sizeof(float));
        for(int j=0; j<HIDDEN; j++)
            hiddenLayer->bias[j] = linearGenerator();
        
        hiddenLayer->node = (float *)malloc(HIDDEN * sizeof(float));

        // set numNodes
        hiddenLayer->numNodes = HIDDEN;
        
        // move to the next layer
        preLayer = hiddenLayer;
    }

    // Setup the output layer
    net->outputLayer = (Layer *)malloc(sizeof(Layer));
    net->outputLayer->prevLayer = preLayer; // the previous layer is the last hidden layer
    preLayer->nextLayer = net->outputLayer; 

    net->outputLayer->weight = (float **)malloc(HIDDEN * sizeof(float *));
    for(int i=0; i<HIDDEN; i++){
        net->outputLayer->weight[i] = (float *)malloc(OUTPUTS * sizeof(float));
        for(int j=0; j<OUTPUTS; j++){
            net->outputLayer->weight[i][j] = linearGenerator();
        }
    }
    
    net->outputLayer->bias = (float *)malloc(OUTPUTS * sizeof(float));
    for(int i=0; i<OUTPUTS; i++)
        net->outputLayer->bias[i] = linearGenerator();

    net->outputLayer->node = (float *)malloc(OUTPUTS * sizeof(float));
    net->outputLayer->numNodes = OUTPUTS;

    // set the feedback values to 0
    for(int i=0; i<INPUTS-1; i++)
        net->feedback[i] = 0.0f;
}

float* calcOutput(FBNN* net){
    // Calculating output for the net
    // Normally an input vector would be passed, but this network
    // uses random noise, and feedback values as input
    
    // Setting put the input layer
    Layer* input = net->inputLayer;
    for(int i=0; i<INPUTS-1; i++){  // loop and set the feedback values
        *(input->node+i) = net->feedback[i];
    }
    *(input->node+INPUTS-1) = 0.5f; // Setting the random noise value

    Layer* curLayer = input;
    Layer* nextLayer = input->nextLayer; // Layer that is being calculated
    float sum;
    while(nextLayer){
        for(int nex=0; nex<nextLayer->numNodes; nex++){ // loop through all nodes in next layer
            sum = 0;
            for(int cur=0; cur<curLayer->numNodes; cur++){
                // sum += previous layer node * weight
                sum += *(curLayer->node+cur) * *(*(nextLayer->weight+cur)+nex);
            }
            sum += *(nextLayer->bias+nex);
            *(nextLayer->node+nex) = fastSigmoid(sum);  // node = activation(prevNodes * weights + bias)
        }
        curLayer = nextLayer;
        nextLayer = nextLayer->nextLayer;
    }
    // At this point the entire network has been calculated and curLayer=net->outputLayer
    // Calculate the last layer
    for(int out=0; out<OUTPUTS; out++){
        sum = 0;
        for(int hid=0; hid<HIDDEN; hid++){
            // sum += previous layer node * weight
            sum += *(curLayer->prevLayer->node + hid) * *(*(curLayer->weight+hid)+out);
        }
        sum += *(curLayer->bias+out);
        *(curLayer->node+out) = fastSigmoid(sum);   // node = activation(prevNodes * weights + bias)
    }   

    // Move all feedback values up one index and set the last index to the first output of the network
    for(int i=0; i<INPUTS-1; i++){
        net->feedback[i] = net->feedback[i+1];
    }
    net->feedback[INPUTS-1] = net->outputLayer->node[0];
    return net->outputLayer->node;
}