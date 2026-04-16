import re
import sys

with open(sys.argv[1], 'r') as f:
    data = f.read()

total = [float(x) for x in re.findall(r'Total time: ([\d.]+)s', data)]
init = [float(x) for x in re.findall(r'Init time: ([\d.]+)s', data)]
sim = [float(x) for x in re.findall(r'Sim time: ([\d.]+)s', data)]
free = [float(x) for x in re.findall(r'Free time: ([\d.]+)s', data)]

print(f"Average Total Time: {sum(total)/len(total)}s")
print(f"Average Init Time:  {sum(init)/len(init)}s")
print(f"Average Sim Time:   {sum(sim)/len(sim)}s")
print(f"Average Free Time:  {sum(free)/len(free)}s")