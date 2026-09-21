import numpy as np
import matplotlib.pyplot as plt

def set_plotStyle():
    plt.rcParams.update({
    'font.size': 14,
    'figure.figsize': (10, 6),
    'axes.labelsize': 14,
    'axes.titlesize': 16,
    'xtick.labelsize': 14,
    'ytick.labelsize': 14,
    'legend.fontsize': 14,
    'lines.linewidth': 2,
    'lines.markersize': 8,
    'lines.color': 'red',
    'patch.facecolor': 'black',
    'figure.dpi': 100,
    'axes.grid': True,
    'grid.linestyle': '--',
    'grid.alpha': 0.7,
    'grid.color': 'gray',
    'grid.linewidth': 0.5,
    'axes.facecolor': 'white',
    'axes.edgecolor': 'black',
    'axes.linewidth': 1.0,
    'xtick.direction': 'out',
    'ytick.direction': 'out',
    'xtick.major.size': 5,
    'ytick.major.size': 5,
    'xtick.minor.size': 2.5,
    'ytick.minor.size': 2.5})

set_plotStyle()

AXEL_run_energy = np.array([29.7, 33.6, 511, 814.1, 898.0, 1461, 1836])  # keV
AXEL_run_pes = np.array([16780.0, 19166.0, 2.9989e5, 4.6512e5, 5.1374e5, 8.3458e5, 1.054e6])  # pes
AXEL_run_pes_err = np.array([4, 11, 220, 220, 220, 420, 600])  # pes

AXEL_run_resolution = np.array([4.389, 4.722, 1.2, 1.194, 1.152, 1.09, 0.73])  # FWHM
AXEL_run_resolution_err = np.array([0.050, 0.125, 0.182, 0.102, 0.119, 0.16, 0.11])  # FWHM

simulation_pes = np.array([13750 * 1.3, 15700 * 1.3, 320000, 548536.12, 1014591.18, 1668433.72]) 

simulation_resolution = np.array([3.70, 3.54, 1.05, 0.707, 0.60, 0.55])

simulation_energy = np.array([29.7, 33.6, 511, 860.6, 1592.5, 2614.5])  # keV
simulation_pes_err = (simulation_resolution * simulation_pes ) / (2 * np.sqrt(2 * np.log(2))) / 100 # pes

reduced_EL_field = np.array([1200, 1400, 1600, 1800])/1000  # kV/cm

Kr_resolution_absorbent3mm = np.array([9.74, 8.50, 7.45, 6.66])
Kr_resolution_absorbent6mm = np.array([7.64, 5.94, 5.36, 4.85])

Kr_resolution_reflective3mm = np.array([8.21, 6.45, 5.73, 5.35])
Kr_resolution_reflective6mm = np.array([6.26, 4.62, 4.07, 3.75])

charge_max_absorbent3mm_Kr_200ns = np.array([120, 220, 330, 420])
charge_max_absorbent6mm_Kr_200ns = np.array([155, 290, 430, 510])

charge_max_reflective3mm_Kr_200ns = np.array([155, 310, 420, 555])
charge_max_reflective6mm_Kr_200ns = np.array([190, 410, 600, 750])

plt.figure(figsize=(12, 8))

plt.errorbar(AXEL_run_energy, AXEL_run_resolution, yerr=AXEL_run_resolution_err, marker='o', color='tab:blue', label='AXEL Run Data')
plt.plot(simulation_energy, simulation_resolution, marker='s', color='tab:orange', label='Simulation Data')
plt.xlabel('Energy [keV]')
plt.ylabel('Energy Resolution [% FWHM]')
plt.savefig('/Users/pablovazquez/GALA_hole/results/AXEL_run_simulation_comparison_resolution.pdf', dpi=300)
plt.legend()


plt.figure(figsize=(12, 8))
plt.errorbar(AXEL_run_energy, AXEL_run_pes, yerr=AXEL_run_pes_err, marker='o', color='tab:blue', label='AXEL Run Data')
plt.errorbar(simulation_energy, simulation_pes, yerr=simulation_pes_err, marker='s', color='tab:orange', label='Simulation Data')
plt.xlabel('Energy [keV]')
plt.ylabel('Photoelectrons [pes]')
plt.savefig('/Users/pablovazquez/GALA_hole/results/AXEL_run_simulation_comparison_pes.pdf', dpi=300)
plt.legend()

plt.figure(figsize=(12, 8))
plt.plot(reduced_EL_field, Kr_resolution_absorbent3mm, marker='*', color='black', label='3 mm Absorbent')
plt.plot(reduced_EL_field, Kr_resolution_absorbent6mm, marker='^', color='black', label='6 mm Absorbent')

plt.plot(reduced_EL_field, Kr_resolution_reflective3mm, marker='*', color='gray', label='3 mm Reflective')
plt.plot(reduced_EL_field, Kr_resolution_reflective6mm, marker='^', color='gray', label='6 mm Reflective')
plt.savefig('/Users/pablovazquez/GALA_hole/results/Kr_energy_resolution.pdf', dpi=300)

plt.xlabel('Reduced EL Field [kV/cm/bar]')
plt.ylabel('Kr Energy Resolution [% FWHM]')
plt.legend()

plt.figure(figsize=(12, 8))
plt.plot(reduced_EL_field, charge_max_absorbent3mm_Kr_200ns, marker='*', color='black', label='3 mm Absorbent')
plt.plot(reduced_EL_field, charge_max_absorbent6mm_Kr_200ns, marker='^', color='black', label='6 mm Absorbent')

plt.plot(reduced_EL_field, charge_max_reflective3mm_Kr_200ns, marker='*', color='gray', label='3 mm Reflective')
plt.plot(reduced_EL_field, charge_max_reflective6mm_Kr_200ns, marker='^', color='gray', label='6 mm Reflective')
plt.savefig('/Users/pablovazquez/GALA_hole/results/charge_max_Kr_200ns.pdf', dpi=300)
plt.xlabel('Reduced EL Field [kV/cm/bar]')
plt.ylabel('Most charged bin [pes]')
plt.legend()

plt.show()