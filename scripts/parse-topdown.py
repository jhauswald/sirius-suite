#!/usr/bin/env python

import sys, os, re, subprocess, json, csv
from collections import defaultdict
import numpy as np

def get_data(fname, metrics):
    data = {}
    with open(fname, 'rb') as f:
        d = csv.DictReader(f)
        for row in d:
            for k,v in row.iteritems():
                if any(v[0] in s for s in metrics):
                    data[v[0]] = v[1]
    
    return data

def sirius_print(data, metrics):
    for m in metrics[:-1]:
        if type(m) is str:
            sys.stdout.write('%s,' % data[m])
        else:
            sys.stdout.write('%.2f,' % data[m])
    else:
        print '%s' % data[metrics[-1]]

def main( args ):
    kernels = [ 'fe', 'fd', 'gmm', 'regex', 'stemmer', 'crf', 'dnn-asr']
    # kernels = [ 'dnn-asr']
    platforms = [ 'baseline']
    metrics = ['Kernel', 'CPI Rate', 'Front-end Bound', 'Bad Speculation', 'Retiring', 'Back-End Bound']

    # top directory of kernels
    kdir = args[1]
    os.chdir(kdir)

    # for each kernel and platform.
    root = os.getcwd()

    # print 'kernel,',
    for m in metrics[:-1]:
        sys.stdout.write('%s,' % m)
    else:
        # sys.stdout.write('%s,' % metrics[-1])
        print '%s' % metrics[-1]

    data = {}
    for k in kernels:
        d = os.getcwd() + '/' + k
        os.chdir(d)
        kroot = os.getcwd() 
        for plat in platforms:
            if not os.path.isdir(plat):
                continue
            os.chdir(plat)
            name = 'sirius-suite.report'
            data = get_data(name, metrics)
            data['Kernel'] = k
            sirius_print(data, metrics)
            os.chdir(kroot)
        os.chdir(root)

if __name__=='__main__':
    sys.exit(main(sys.argv))
