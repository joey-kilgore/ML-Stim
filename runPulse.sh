#!/bin/bash
echo "Running Pulse Width Modulation"
make
./bin/PulseModulation.exe
if [ $? -eq 0 ]
then
    mv data.csv ./R/Sims/Sim9/output.csv
    cd ./R
    R -e "shiny::runApp('app.R', port=6462)"
else
    echo "Failed Simulation"
fi
