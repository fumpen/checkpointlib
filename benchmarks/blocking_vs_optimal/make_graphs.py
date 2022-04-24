import numpy as np
from matplotlib import pyplot as plt


mem_allocated = [100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 900000, 1000000,
                 1100000, 1200000, 1300000, 1400000]

time_beginning = [1711829, 1713488, 1723865, 1739075, 1733932, 1731423, 1734006, 1750109, 1745650,
                  1758049, 1781344, 1825320, 1881137, 1923902]

time_end = [1739457, 1737809, 1728767, 1738050, 1740643, 1833965, 1966773, 2015825, 2080127, 2137384,
            2185650, 2242634, 2297403, 2355665]

time_blocking = [1683672, 1828844, 1962893, 2107789, 2216859, 2355986, 2463832, 2572675, 2691470, 2814105, 2927513, 3036821, 3159405, 3303521]

#time_control = [193554.0, 193554.0, 193554.0, 193554.0, 193554.0,
#                193554.0, 193554.0, 193554.0, 193554.0, 193554.0]

# convert to millisec
time_beginning = [float(x) / 1000.0 for x in time_beginning]
time_end = [float(x) / 1000.0 for x in time_end]
time_blocking = [float(x) / 1000.0 for x in time_blocking]
#time_control = [x / 1000.0 for x in time_control]


plt.rcParams.update({'font.size': 22})

# Make the plot
plt.plot(mem_allocated, time_beginning, color='red', marker='o', label='write in beginning of checkpoint-memory')

plt.plot(mem_allocated, time_end, color='blue', marker='o', label='write in end of checkpoint-memory')
plt.plot(mem_allocated, time_blocking, color='green', marker='o', label='checkpoint implemented as blocking')
#plt.plot(mem_allocated, time_control, color='gray', label='no checkpoint (control)')

 
plt.xlabel('memory saved( in bytes)', fontweight='bold')
plt.ylabel('total time to completion(in milliseconds)', fontweight='bold')
plt.title("Saving various sized checkpoint-memory")
# Create legend & Show graphic
plt.legend()
plt.show()

