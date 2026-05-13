from scipy.optimize import curve_fit    
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams['figure.figsize'] = (10, 6)
plt.rcParams['font.size'] = 14
plt.rcParams['text.usetex'] = False
plt.rcParams['mathtext.fontset'] = 'cm'  # Computer Modern, la fuente de LaTeX
plt.rcParams['font.family'] = 'serif'


def pol(x, a, b, c, d, e):
    return a + b *x + c * x**2 + d * x**3 + e*x**4

x = np.array([0, 0.02, 0.04, 0.05, 0.07, 0.09, 0.1, 0.12, 0.15, 0.17, 0.19, 0.2, 0.22])*10
y = np.array([169.23, 169.48, 170.03, 171, 172.8, 176.52, 177.83, 182.18, 190.44, 197.69, 207.17, 212.31, 233.335])/169

popt, pcov = curve_fit(pol, x, y, (1, 0, 0, 0, 0))

plt.plot(x, y, marker = 'o', linestyle = 'None', label = 'Theoretical points', color = 'black')
plt.plot(x, pol(x,  *popt), label = 'Fitted curve', color = 'red')
plt.xlabel('Radial distance to center [mm]')
plt.ylabel('Normalized light production')
plt.title('Light Production Fluctuation')
plt.legend()
plt.grid(True, alpha=0.3, linestyle = '--')
plt.show()