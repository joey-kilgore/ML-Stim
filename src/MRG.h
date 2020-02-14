// MRG.h is a header file for the model based on the MRG model described in
// McIntyre, Cameron C., et al. “Modeling the Excitability of Mammalian Nerve Fibers: 
//  Influence of Afterpotentials on the Recovery Cycle.” Journal of Neurophysiology, 1 Feb. 2002, 
//  www.physiology.org/doi/10.1152/jn.00353.2001.

// Structs
struct node
{   // Nodes contain specific capacitance, conductance, nernst potential values
    float gNa, gNaP, gK, gL;    // fast sodium, persistent sodium, potassium, and leak conductances
    float cap;                  // membrane capacitance
    float vNa, vNaP, vK, vL;    // fast sodium, persistent sodium, potassium, and leak nernst potentials

    // there are various state variables that will change during the simulation
    float m, h, mp, s;  // gating variables
    float v;            // voltage

}


