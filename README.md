# ML Stim  
This project is built to help bring neuroscience into the 21st century. All current simulation environments are either slow, or written by the neuroscientists themselves, which makes sharing research difficult. We create a simulation environment that could be deployed on cloud computing or high performance computing for easier sharing of data.  
On top of our environment we created a machine learning algorithm to control electrical stimulation of the model. This is then trained to try and block propagation of electrical signals.  
### To run simulation  
```
make
cd bin
./controller.exe
```
This will run the machine learning simulation. The algorithm attempts to find the optimum method for blocking action potentials. The best neural net from each generation is run again, and the data is saved to file.  

### Pulse Width Modulation ML Explained  
In an attempt to have the machine learning algorithm find waveforms that are similar to current waveforms, it made sense to create a method for which the ML agent modulates a sinusoidal wave. Each half cycle of the wave could be modulated by any of three parameters: Pulse Width, Amplitude, or Charge. Because any two of these can calculate the third, it seemed to make the most sense to put a direct output between the ML agent and the Charge and Pulse Width. Limiting these will keep the algorithm within safety limits (limiting charge) and within physical constraints of creating waveforms (limiting pulse width). Additionally a feedback output is used to let the agent learn to pass data back to itself for creating waveforms.  
The optimum fitness function of the agents is still unknown, but the behaviors being optimized include:  
 * Blocking the propagation of action potentials  
 * Create no action potentials when stimulation begins (known as a no onset waveform)  
 * Minimize needed unbalanced charge  
 * Recreatable in experiments
