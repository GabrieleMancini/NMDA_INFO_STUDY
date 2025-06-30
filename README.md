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
- Brian2 (for spiking neural simulations, if applicable)
- Jupyter

You can install the full environment using:

```bash
conda env create -f env/environment.yml
conda activate nmda_info_study
````

Or install manually with pip:

```bash
pip install -r env/requirements.txt
```

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
Gabriele Mancini - \[[your.email@example.com](mailto:your.email@example.com)]

---

Thank you for your interest in our work!

