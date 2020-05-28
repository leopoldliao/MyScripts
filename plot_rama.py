#!/usr/bin/python3
# -*- coding:utf-8 -*-
# Modified by Yujie Liu

import sys
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from scipy import ndimage

font = {'family': 'Times New Roman'}
mpl.rc('font', **font)

def ReadRamaXvg(FileName, interval=2, fraction=33,  smoothen=1.15, step=4):
    # Read xvg file and save data, phi and psi
    try:
        infile = open(FileName, "r")
    except:
        raise IOError('Could not open file {0} for reading. \n' .format(FileName))
    data, x = [], []
    for line in infile:
        line = line.rstrip('\n')
        if not line.strip():
            continue
        if(not (line.startswith('@') or line.startswith('#'))):
           temp = line.split()
           data.append(np.array(temp))
    for i in range(2):
        x_temp = []
        for j in range(len(data)):
            try:
                value = float(data[j][i])
            except:
                raise FloatingPointError('\nCould not convert {0} to floating point number.. Something is wrong in {1}..\n' .format(data[j][i], FileName))
            x_temp.append(value)
        x.append(x_temp)
    rama_x = x[0]
    rama_y = x[1]
    
    # Edges is array of [-180, 180] at a certain interval
    edges  = range(-180, 180+interval, interval)
    
    # Generate normalized 2D histogram (array of array)
    Histo, xedges, yedges = np.histogram2d(rama_x, rama_y, 
                                 bins=(edges,edges), density=True)
    
    # Smoothening the 2D histogram data
    Sigma = [ (max(xedges)-min(xedges))*smoothen/(len(xedges)),
            (max(yedges)-min(yedges))*smoothen/(len(yedges)) ]
    smooth_hist = ndimage.filters.gaussian_filter(Histo, sigma=Sigma)
    
    # Get scientif notation, then set the cutoff to a fraction of maximum
    max_nm = float(np.max(smooth_hist))
    powers = int('{:e}'.format(max_nm).split('e')[1])
    digits = np.ceil(float(('{:e}'.format(max_nm)).split('e')[0]))
    digits = int(digits)
    h_max  = np.float('{0}e{1}'.format(digits, powers))
    
    # Introduce a cutoff to histogram data
    histo2d = smooth_hist - h_max/fraction
    
    # Side bar tick, maximum = histogram value
    cbar_ticks = np.linspace(0, h_max, num=(digits*step)+1)
    
    # Contour levels, default is 4x
    levels = np.linspace(0, h_max, num=(digits*step)+1)
    
    # X- and Y-axes min and max, will be stretch to be equal
    extent = [  xedges[0] -1, xedges[-1]+1,
              yedges[-1]+1, yedges[0] -1  ]
    
    # data is transpose to get correct orientation
    # im_extent and im_colors are dummy value to generate holder blank plot
    res_obj = ImageData(histo2d=histo2d.transpose())
    res_obj.cbar_ticks = cbar_ticks
    res_obj.levels = levels
    res_obj.extent = extent
    res_obj.edges  = len(edges)
    res_obj.im_extent = (-181, 181, -181, 181)
    res_obj.im_colors = mpl.colors.ListedColormap(['#FFFFFF'])
    
    return res_obj

def PlotXvg(res_obj):
    plt.figure('Raman', figsize=(7,5.5))
    colors = mpl.cm.jet
    bar_extend  = 'neither'
    plot_extend = 'neither'
    
    # Generate a fake imshow with dummy matrix to get axis ratio 1:1
    plt.imshow(np.zeros(shape=(res_obj.edges-1, res_obj.edges-1)),
      extent=res_obj.im_extent, cmap=res_obj.im_colors)
      
    # Overlay input AA histogram heat map on top of reference map, if exists
    plt.contourf(res_obj.histo2d, 
                origin='upper', extend=plot_extend, alpha=0.6,
                extent=res_obj.extent, levels=res_obj.levels,
                cmap=mpl.cm.get_cmap(colors, len(res_obj.levels)))
    
    # Create colorbar instance on side based on last data input
    cbar = plt.colorbar( ticks=res_obj.cbar_ticks, format=('%.1e'),
                       extend=bar_extend, aspect=20 )
    bar_label = 'Population (%)'
    cbar.ax.set_ylabel(bar_label, rotation=270, fontsize=14, labelpad=20)
    
    # Then overlay contour lines on top of heat map
    plt.contour(res_obj.histo2d, 
              extent=res_obj.extent, levels=res_obj.levels,
               origin='upper', colors='black', linewidths=0.67, alpha=0.4)

    ## Add additional items
    plt.xlim([-180, 180])
    plt.ylim([-180, 180])
    plt.plot([-180, 180], [0, 0], color="black", linewidth=1)
    plt.plot([0, 0], [-180, 180], color="black", linewidth=1)
    plt.xticks(np.arange(-180, 210, step=60), fontsize=14)
    plt.yticks(np.arange(-180, 210, step=60), fontsize=14)
    plt.title('Ramachandran Plot', fontsize=16)
    plt.xlabel(r'Phi $\phi$', fontsize=14)
    plt.ylabel(r'Psi $\psi$', fontsize=14)
    
    plt.grid(linestyle='--')
    #plt.savefig("rama.png", dpi = 300)
    plt.show()

class ImageData(object):
    def __init__(self, histo2d='', **kwargs):
        self.histo2d = histo2d

def Usage():
    print('Usage:')
    print('\t{0}  single_residue_rama.xvg' .format(sys.argv[0]))
    
def main():
    if(len(sys.argv) < 2 or sys.argv[1] == '-h'):
        print('\nError! Need Input file\n')
        Usage()
        sys.exit(1)
    res_obj = ReadRamaXvg(sys.argv[1])
    PlotXvg(res_obj)

if __name__ == '__main__':
    main()
    