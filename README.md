# NMDA_INFO_STUDY

This repository contains the code to reproduce all figures and analyses presented in our paper:

**"A study of NMDA Receptors contribution to information encoding in aggregate signals"**  
*Gabriele Mancini, Hedyeh Reazei, Stefano Panzeri*  
Submitted to *PLOS Computational Biology*

## Overview

In this study, we investigate the role of NMDA receptor dynamics in shaping information encoding at the population level. The simulations and analyses span multiple computational experiments aimed at quantifying how NMDA conductances modulate signal correlations, decoding performance, and mutual information.

The code in this repository is structured to enable full reproducibility of the figures and key results presented in the manuscript.

## Repository Structure

```

NMDA\_INFO\_STUDY/
├── data/                    # Raw or simulated data files (not included in repo, see below)
├── scripts/                 # Core simulation and analysis scripts
├── notebooks/               # Jupyter notebooks for figure generation
├── results/                 # Output figures and processed data
├── env/                     # Conda environment or requirements file
└── README.md                # This file

````

## Requirements

We recommend using a virtual environment. The code was developed and tested using Python 3.9 with the following main packages:

- NumPy
- SciPy
- Matplotlib
- scikit-learn
- [NEST Simulator](https://nest-simulator.readthedocs.io/en/stable/) v3.3
- [NEURON](https://neuron.yale.edu/neuron/)


# Instructions to simulate the LIF model network with NMDA

## Building the neuron model 

Instructions to compile the neuron model in NEST are based on the tutorial about “Writing an extension module” (https://nest.github.io/nest-simulator/extension_modules). 

1 - Define the environment variable 'NEST_INSTALL_DIR' to contain the path to which you have installed NEST, e.g. using bash:

export NEST_INSTALL_DIR=/Users/pablo/NEST/ins

2 - Create a build directory in the folder 'neuron_model':

cd neuron_model
mkdir build
cd build

3 - The configure process uses the script 'nest-config' to find out where NEST is installed, where the source code resides, and which compiler options were used for compiling NEST. If 'nest-config' is not in your path, you need to provided it explicitly like this (don't forget to add '..' at the end):

cmake -Dwith-nest=${NEST_INSTALL_DIR}/bin/nest-config ..

4 - Compile:

make

make install

It might be also necessary to update the LD_LIBRARY_PATH, e.g.:

export LD_LIBRARY_PATH=${NEST_INSTALL_DIR}/lib/python2.7/site-packages/nest:$LD_LIBRARY_PATH


## Running a simulation 

Simulation scripts are in folder 'LIF_network/analysis'. Adjust the simulation parameters in the script and execute it using the Python interpreter, e.g.:

python3 save_results_1.py

# Instructions to simulate the multicompartment model network 
Simulation scripts are in folder 'multicompartment_network/analysis'. Adjust the simulation parameters in the script and execute it using the Python interpreter, e.g.:

python3 simulation_1.py

or launch different MPI jobs to execute it much faster:

mpirun -np 8 python3 simulation_1.py


## Running the Code

1. **Data Generation**
   Run the relevant scripts in the `scripts/` folder to generate synthetic or simulated data. For example:

   ```bash
   python scripts/run_simulation.py
   ```

2. **Data Analysis**
   Process and analyze the data with:

   ```bash
   python scripts/analyze_info_metrics.py
   ```

3. **Figure Generation**
   Use the notebooks in `notebooks/` to generate the figures as shown in the paper:

   ```bash
   jupyter notebook notebooks/Figure_2.ipynb
   ```

Each script and notebook is self-documented and corresponds to specific figures or analyses described in the paper.

## Data Availability

Due to size limitations, raw and processed data are not included directly in the repository. To obtain the data:

* Download from \[link to dataset if available online]
* Or generate it by running the simulation scripts

## Citation

If you use this code, please cite our paper:

> Mancini G., Reazei H., Panzeri S. *A study of NMDA Receptors contribution to information encoding in aggregate signals*. PLOS Computational Biology (submitted).

## License

This project is licensed under the [Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/) license.

## Contact

For questions, please contact:
Gabriele Mancini - \[[gabriele.mancini@iit.it](gabriele.mancini@iit.it)]

---

Thank you for your interest in our work!

