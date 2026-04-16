import matplotlib.pyplot as plt


x = [1, 16, 32, 48]
y = [105.09753222580648/105.09753222580648 / x[0] * 100,
     105.09753222580648/40.85837812903226 / x[1] * 100,
     105.09753222580648/39.986860161290316 / x[2] * 100,
     105.09753222580648/36.094505645161284  / x[3] * 100]


plt.plot(x, y, linewidth=2.0)
plt.grid(True)
plt.xticks(x)
plt.xlabel("Number of threads (n)")
plt.ylabel("Efficiency (%)")
plt.title('Simulation efficiency  over different thread counts')
plt.savefig("sim_efficieny.png")