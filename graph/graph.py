import matplotlib.pyplot as plt
import subprocess
import numpy as np
import os
import time

def time_of_executable(arg):
    start = time.time()
    result = subprocess.run(arg, capture_output=True, text=True)
    print(result)
    end = time.time()
    return end - start

def table_of_times(chemin):
    if not os.path.exists(chemin):
        print("Erreur d'exécution")
        return {}

    times = {}
    for i in range(100):
        arg = [chemin, str(i)]
        times[i] = time_of_executable(arg)
    return times

times_dict = table_of_times("./a")

x_values = list(times_dict.keys()) 
y_values = list(times_dict.values())

plt.plot(x_values, y_values, marker='o', linestyle='-')
plt.xlabel("Paramètre d'entrée")
plt.ylabel("Temps d'exécution (s)")
plt.title("Temps d'exécution en fonction de l'entrée")
plt.show()
