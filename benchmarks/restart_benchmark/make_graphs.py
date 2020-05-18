import numpy as np
from matplotlib import pyplot as plt


mem_allocated = [100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 900000, 1000000]

time = [345844, 650710, 977148, 1472321, 1805524, 2007440, 2242450, 2564095, 3083416, 3448754]

time = [x * 0.001 for x in time]
print(time)
# Make the plot
plt.plot(mem_allocated, time, color='blue', label='application restart timer')

 
plt.xlabel('memory saved (in bytes)', fontweight='bold')
plt.ylabel('total time to completion (in milliseconds)', fontweight='bold')
plt.title("Application restart with checkpoint memory of increasing size")
# Create legend & Show graphic
plt.legend(bbox_to_anchor=(0.2,0.9))
plt.show()

