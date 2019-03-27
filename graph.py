# graph.py
import matplotlib
import pandas as pd
import matplotlib.pyplot as plt
import sys
data = pd.read_csv(sys.argv[1], header=None, sep='\t')
data.plot()
plt.show()
