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

sigma_x, sigma_z = (np.array([0.93, 0.8750195 , 0.85767178, 0.78259438])/2,
 np.array([0.91, 0.88226554, 0.84599161, 0.75238057])/2)

sigma_x_err, sigma_z_err = (np.array([0, 0.00995554, 0.01617861, 0.00460491])/2,
 np.array([0, 0.00845422, 0.00692809, 0.00461571])/2)

reduced_EL_field = np.array([1200, 1400, 1600, 1800])/1000  # kV/cm

Kr_resolution_absorbent3mm = np.array([8.03, 7.20, 6.11, 5.64])
Kr_resolution_absorbent6mm = np.array([5.71, 4.89, 4.46, 4.08])

Kr_resolution_reflective3mm = np.array([5.90, 5.14, 4.78, 4.21])
Kr_resolution_reflective6mm = np.array([4.22, 3.44, 3.10, 3.03])

charge_max_absorbent3mm_Kr_200ns = np.array([260, 470, 710, 805])
charge_max_absorbent6mm_Kr_200ns = np.array([ 330, 650, 930, 1150])

charge_max_reflective3mm_Kr_200ns = np.array([360, 620, 1010, 1200])
charge_max_reflective6mm_Kr_200ns = np.array([510, 950, 1420, 1750])

fig, ax1 = plt.subplots(figsize=(12, 8))

# Eje izquierdo: sigma_x, sigma_z (radio efectivo GALA)
l1 = ax1.errorbar(reduced_EL_field, 2*sigma_z, yerr=2*sigma_z_err, marker='s',
                   color='tab:blue', label=r'$\sigma_z$')
l2 = ax1.errorbar(reduced_EL_field, 2*sigma_x, yerr=2*sigma_x_err, marker='o',
                   color='tab:orange', label=r'$\sigma_x$')

ax1.set_xlabel('Reduced EL Field (kV/cm/bar)')
ax1.set_ylabel(r'Effective GALA radius (mm)')

# Eje derecho (clonado, mismo x): resolución
ax2 = ax1.twinx()
l3, = ax2.plot(reduced_EL_field, Kr_resolution_absorbent3mm, color='black',
               marker='*', linestyle='solid', label='Absorbent 3mm', markersize = 12)
l4, = ax2.plot(reduced_EL_field, Kr_resolution_absorbent6mm, color='black',
               marker='*', linestyle='dashed', label='Absorbent 6mm', markersize = 12)
l5, = ax2.plot(reduced_EL_field, Kr_resolution_reflective3mm, color='dimgray',
               marker='^', linestyle='solid', label='Reflective 3mm', markersize = 12)
l6, = ax2.plot(reduced_EL_field, Kr_resolution_reflective6mm, color='dimgray',
               marker='^', linestyle='dashed', label='Reflective 6mm', markersize = 12)

ax2.set_ylabel('Kr resolution (%)')

# ax2 no debe heredar el grid de ax1
ax2.grid(False)

# Leyenda combinada de ambos ejes
lines = [l1, l2, l3, l4, l5, l6]
labels = [ln.get_label() for ln in lines]
ax1.legend(lines, labels, loc='best')

ax1.set_title('Effective Radius and Kr Resolution vs Reduced EL Field')
fig.tight_layout()
plt.savefig('/Users/pablovazquez/GALA_hole/results/effective_radius_and_Kr_resolution.pdf')
plt.show()

plt.figure(figsize=(12, 8))
plt.plot(reduced_EL_field, charge_max_absorbent3mm_Kr_200ns, color='black', marker='*', linestyle='solid', label='Absorbent 3mm', markersize = 12)
plt.plot(reduced_EL_field, charge_max_absorbent6mm_Kr_200ns, color='black', marker='*', linestyle='dashed', label='Absorbent 6mm', markersize = 12)
plt.plot(reduced_EL_field, charge_max_reflective3mm_Kr_200ns, color='dimgray', marker='^', linestyle='solid', label='Reflective 3mm', markersize = 12)
plt.plot(reduced_EL_field, charge_max_reflective6mm_Kr_200ns, color='dimgray', marker='^', linestyle='dashed', label='Reflective 6mm', markersize = 12)
plt.xlabel('Reduced EL Field (kV/cm/bar)')
plt.ylabel('Maximum temporal bin charge (pes)')
plt.title('200ns bin sampling: Maximum temporal bin charge vs Reduced EL Field')
plt.legend(loc='best')
plt.grid(True, alpha = 0.4, linestyle = '--')
plt.savefig('/Users/pablovazquez/GALA_hole/results/charge_max_vs_reduced_EL_field.pdf')
plt.show()
