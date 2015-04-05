#!/usr/bin/env python

import sys, os, re, subprocess

def shcmd(cmd):
    subprocess.call(cmd, shell=True)

def shcom(cmd):
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    out = p.communicate()[0]
    return out

def run_kernel (k, plat):
    '''To change inputs or # of threads'''
    cmd = ''
    if k == 'fe':
        if plat == 'pthread':
            cmd = './surf-fe ' + str(threads) + ' ../input/2048x2048.jpg'
        else:
            cmd = './surf-fe ../input/2048x2048.jpg'
    elif k == 'fd':
        if plat == 'pthread':
            cmd = './surf-fd ' + str(threads) + ' ../input/2048x2048.jpg'
        else:
            cmd = './surf-fd ../input/2048x2048.jpg'
    elif k == 'gmm':
        if plat == 'pthread':
            cmd = './gmm_scoring ' + str(threads) + ' ../input/gmm_data.txt'
        else:
            cmd = './gmm_scoring ../input/gmm_data.txt'
    elif k == 'regex':
        if plat == 'pthread':
            cmd = './regex_slre ' + str(threads) + ' ../input/list ../input/questions'
        else:
            cmd = './regex_slre ../input/list ../input/questions'
    elif k == 'stemmer':
        if plat == 'pthread':
            cmd = './stem_porter ' + str(threads) + ' ../input/voc-1M.txt'
        else:
            cmd = './stem_porter ../input/voc-1M.txt'
    elif k == 'crf':
        if plat == 'pthread':
            cmd = './crf_tag ' + str(threads) + '../input/model.la ../input/test-input.txt'
        else:
            cmd = './crf_tag ../input/model.la ../input/test-input.txt'
    elif k == 'dnn-asr':
        if plat == 'pthread':
            cmd = 'OPENBLAS_NUM_THREADS=%s ./dnn_asr ../model/asr.prototxt \
                                                    ../model/asr.caffemodel \
                                                    ../input/features.in' % threads
        else:
            cmd = 'OPENBLAS_NUM_THREADS=1 ./dnn_asr ../model/asr.prototxt \
                                                    ../model/asr.caffemodel \
                                                    ../input/features.in'
    
    return cmd

def main( args ):
    if len(args) < 3:
        print "Usage: ./collect-stats.py <top-directory of kernels> <# of runs>"
        return

    # kernels = [ 'fe', 'fd', 'gmm', 'regex', 'stemmer', 'crf', 'dnn-asr']
    kernels = [ 'fe']
    platforms = [ 'baseline']

    # top directory of kernels
    kdir = args[1]
    os.chdir(kdir)

    # how many times to run each kernel
    LOOP = int(args[2])

    # for each kernel and platform.
    # uses 'make test' input and config for each kernel
    pmu_cmd = 'toplev -S --all --core C0 taskset -c 0 '
    root = os.getcwd()
    for k in kernels:
        d = os.getcwd() + '/' + k
        os.chdir(d)
        kroot = os.getcwd() 
        for plat in platforms:
            if not os.path.isdir(plat):
                continue
            os.chdir(plat)
            for i in range(1, LOOP):
                cmd = pmu_cmd + run_kernel(k, plat)
                print cmd
                shcmd(cmd)
            os.chdir(kroot)
        os.chdir(root)

if __name__=='__main__':
    sys.exit(main(sys.argv))
