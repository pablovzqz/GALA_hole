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

y_long = np.array([350, 298, 298, 310, 299, 293, 279, 271, 248, 273, 256, 214, 193,
       178, 157, 100,  83,  41,  17,  12])

x_long = np.array([-136.79840302, -136.58019108, -136.36197913, -136.14376717,
       -135.92555523, -135.70734327, -135.48913133, -135.27091938,
       -135.05270742, -134.83449548, -134.61628353, -134.39807158,
       -134.17985963, -133.96164767, -133.74343573, -133.52522378,
       -133.30701183, -133.08879988, -132.87058792, -132.65237598]) 

def exp_potencia(X, A, lambdaa, n):
    return A * np.exp(-lambdaa * X**n)

popt_long, pcov_long = curve_fit(exp_potencia, x_long, y_long, p0=(300, -0.1, 5))
    
plt.figure()
plt.plot(x_long, y_long, marker = 'o', linestyle = 'None', label = 'Theoretical points', color = 'black')
plt.plot(x_long, exp_potencia(x_long, *popt_long), label = 'Fitted curve', color = 'red')
plt.xlabel('Radial distance to center [mm]')
plt.ylabel('Normalized light production')
plt.title('Light Production Fluctuation')
plt.legend()
plt.grid(True, alpha=0.3, linestyle = '--')
plt.show()

print(popt_long)