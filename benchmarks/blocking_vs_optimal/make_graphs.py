import numpy as np
from matplotlib import pyplot as plt


mem_allocated = [100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 900000, 1000000]

time_beginning = [1622078.0, 1626224.0, 1622365.0, 1621253.0,
                  1621811.0, 1551620.0, 1599972.0, 1812114.0,
                  2027884.0, 2297336.0]

time_end = [1662283.0, 1670071.0, 1672667.0, 1727722.0, 2252891.0,
            2328007.0, 2378371.0, 2558466.0, 2806641.0, 3260338.0]

time_blocking = [1454793.0, 1669379.0, 1892009.0, 2066372.0,
                 2418756.0, 2611012.0, 2724917.0, 2954766.0,
                 3187977.0, 3485672.0]

time_control = [193554.0, 193554.0, 193554.0, 193554.0, 193554.0,
                193554.0, 193554.0, 193554.0, 193554.0, 193554.0]

# convert to millisec
time_beginning = [x / 1000.0 for x in time_beginning]
time_end = [x / 1000.0 for x in time_end]
time_blocking = [x / 1000.0 for x in time_blocking]
time_control = [x / 1000.0 for x in time_control]

# Make the plot
plt.plot(mem_allocated, time_beginning, color='red', label='write in beginning of checkpoint-memory')

plt.plot(mem_allocated, time_end, color='blue', label='write in end of checkpoint-memory')
plt.plot(mem_allocated, time_blocking, color='green', label='checkpoint implemented as blocking')
#plt.plot(mem_allocated, time_control, color='gray', label='no checkpoint (control)')

 
plt.xlabel('memory saved( in bytes)', fontweight='bold')
plt.ylabel('total time to completion(in milliseconds)', fontweight='bold')
plt.title("Saving various sized checkpoint-memory")
# Create legend & Show graphic
plt.legend()
plt.show()

