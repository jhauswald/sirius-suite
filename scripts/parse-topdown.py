#!/usr/bin/env python

import sys, os, re, subprocess, json, csv
from collections import defaultdict
import numpy as np

def get_data(fname, metrics):
    data = []
    with open(fname, 'rb') as f:
        d = csv.DictReader(f)
        for row in d:
            for k,v in row.iteritems():
                if any(v[0] in s for s in metrics):
                    data.append(v)
    
    return data

def main( args ):
    kernels = [ 'fe', 'fd', 'gmm', 'regex', 'stemmer', 'crf', 'dnn-asr']
    # kernels = [ 'dnn-asr']
    platforms = [ 'baseline']
    metrics = ['Retiring', 'CPI Rate', 'Back-End Bound', 'Front-end Bound', 'Bad Speculation']

    # top directory of kernels
    kdir = args[1]
    os.chdir(kdir)

    # for each kernel and platform.
    root = os.getcwd()

    data = []
    for k in kernels:
        d = os.getcwd() + '/' + k
        os.chdir(d)
        kroot = os.getcwd() 
        for plat in platforms:
            if not os.path.isdir(plat):
                continue
            os.chdir(plat)
            name = k + '_' + plat + '.csv'
            data = get_data(name, metrics)
            print k
            print data
            os.chdir(kroot)
        os.chdir(root)

if __name__=='__main__':
    sys.exit(main(sys.argv))
