# -*- coding:utf-8 -*-
# A simple code for plotting out.dat from my _xpm2txt program

import matplotlib.pyplot as plt
import numpy as np

#read data 
fp = open('out.dat', 'r')
x, data = [], []
for line in fp:
    line = line.rstrip('\n')
    if not line.strip():
        continue
    try:
        if "#" in line:
            temp = line.split()
            grid = int(temp[-1])
        else:
            temp = line.split()
            x.append(np.array(temp))
    except: 
        pass

for j in range(0, 3):
    x_temp =[]
    for i in range(len(x)):
        try:
            value = float(x[i][j])
        except:
            raise FloatingPointError('\nCould not convert {0} to floating point number..\n' .format(x[i][j]))
        x_temp.append(value)
    data.append(x_temp)
fp.close()

#reshape data according to grid * grid
fesvalue=np.array(data[2])
plotvalue=np.reshape(fesvalue,(grid,-1))

#plot figure
plt.figure(figsize=(8,7))
h = plt.contourf(np.linspace(np.min(data[0]), np.max(data[0]), num = grid), 
                np.linspace(np.min(data[1]), np.max(data[1]), num = grid), 
                plotvalue, cmap=plt.cm.rainbow)
cb = plt.colorbar(h)
cb.set_label(' ', fontsize=16)

plt.xlabel('PC1',fontsize=16)
plt.ylabel('PC2',fontsize=16)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
plt.show()
