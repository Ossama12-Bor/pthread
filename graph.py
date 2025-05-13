import matplotlib.pyplot as plt
import subprocess
import numpy as np
import os
import time
import sys

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
	for i in range(20):
		total_time = 0
		for j in range(50):
			arg = [chemin, str(i*50)]
			times[i*50] = time_of_executable(arg)
		times[i*50] = times[i*50]/50
	return times

name = sys.argv[1]  

times_dict_install = table_of_times(f"./install/bin/{name}")
times_dict_tst = table_of_times(f"./install/bin/p{name}")

x_thread = list(times_dict_install.keys())
y_thread = list(times_dict_install.values())

x_pthread = list(times_dict_tst.keys())
y_pthread = list(times_dict_tst.values())

plt.plot(x_thread, y_thread, marker='o', linestyle='-', label="thread")
plt.plot(x_pthread, y_pthread, marker='s', linestyle='--', label="pthread")

plt.xlabel("Paramètre d'entrée")
plt.ylabel("Temps d'exécution (s)")
plt.title("Comparaison des temps d'exécution")
plt.legend()
#plt.yscale("log")
plt.show()
