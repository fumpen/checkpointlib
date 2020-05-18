import numpy as np
from matplotlib import pyplot as plt


mem_allocated = [40000, 400000, 4000000, 40000000, 400000000]

time_stream = [5, 57, 1058, 11098, 116704]

time_FILE = [5, 55, 1054, 11139, 118234]

time_syscall = [5, 55, 1052, 10893, 118824]

time_malloc = [1, 15, 541, 6483, 66283]


# Make the plot
plt.plot(mem_allocated, time_stream, color='red', label='c++ std::stream')
plt.plot(mem_allocated, time_FILE, color='blue', label='c++ FILE')
plt.plot(mem_allocated, time_syscall, color='green', label='linux syscalls')
plt.plot(mem_allocated, time_malloc, color='gray', label='copy in memory')

 
plt.xlabel('memory copied (in bytes)', fontweight='bold')
plt.ylabel('total time to completion (in microseconds)', fontweight='bold')
plt.title("Illustration of the time taken to copy data to disk and in memory.")
# Create legend & Show graphic
plt.legend()
plt.show()

