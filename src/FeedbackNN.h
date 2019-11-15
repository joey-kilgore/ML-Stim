#pragma once
#define HIDDEN_LAYERS 5   // Number of total hidden layers in the network 
#define INPUTS 5      // Number of input nodes
#define HIDDEN 10     // Number of nodes in EACH hidden layer
#define OUTPUTS 1     // Number of output nodes

typedef struct Layer{
    // Each layer is set with the ability to hold the data necesary for processing
    // The setup for each array of values is set so during a feed forward calculation
    //  The node value is the last variable of the layer
    struct Layer* prevLayer;
    struct Layer* nextLayer;
    float** weight;     // weight[i][j] connects i-previous node, j-current node
    float* node;
    float* bias;
    int numNodes;
} Layer;

typedef struct FBNN{
    // FeedBack Neural Networks are similar to traditional FeedForward
    //  NN but in this case the outputs from one iteration are fed
    //  back into the network as inputs for the next iteration
    //  Allowing the NN to have 'memory'
    //  This is important for electrical stimulation so the NN can learn
    //   more complex waveforms
    Layer* inputLayer;
    Layer* outputLayer;
    float feedback[INPUTS-1];
} FBNN;

FBNN *initNet();
void freeFBNN(FBNN* net);
float* calcOutput(FBNN* net);
FBNN *spawnNet(FBNN* p1, FBNN* p2);
