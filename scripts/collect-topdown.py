#!/usr/bin/env python

import sys, os, re, subprocess

threads = 4
overlap = 50

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
            cmd = './surf-fe ' + str(threads) + ' ' + str(overlap) + ' ../input/2048x2048.jpg'
        else:
            cmd = './surf-fe ../input/2048x2048.jpg'
    elif k == 'fd':
        if plat == 'pthread':
            cmd = './surf-fd ' + str(threads) + ' ' + str(overlap) + ' ../input/2048x2048.jpg'
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
            cmd = './stem_porter 10000000' + str(threads) + ' ../input/voc-10M.txt'
        else:
            cmd = './stem_porter 10000000 ../input/voc-10M.txt'
    elif k == 'crf':
        if plat == 'pthread':
            cmd = './crf_tag ' + str(threads) + ' ../input/model.la ../input/test-input.txt'
        else:
            cmd = './crf_tag ../input/model.la ../input/test-input.txt'
    elif k == 'dnn-asr':
        if plat == 'pthread':
            cmd = './dnn_asr ' + str(threads) + ' ../model/asr.prototxt ' \
                                              + ' ../model/asr.caffemodel' \
                                              + ' ../input/features.in'
        else:
            cmd = './dnn_asr'  + ' ../model/asr.prototxt ' \
                               + ' ../model/asr.caffemodel' \
                               + ' ../input/features.in'
    
    return cmd

def main( args ):
    if len(args) < 2:
        print "Usage: ./collect-stats.py <top-directory of kernels>"
        return

    kernels = [ 'fe', 'fd', 'gmm', 'regex', 'stemmer', 'crf', 'dnn-asr']
    kernels = [ 'stemmer']
    platforms = [ 'baseline']

    # top directory of kernels
    kdir = args[1]
    os.chdir(kdir)

    # for each kernel and platform.
    root = os.getcwd()

    for k in kernels:
        d = os.getcwd() + '/' + k
        os.chdir(d)
        kroot = os.getcwd() 
        for plat in platforms:
            if not os.path.isdir(plat):
                continue
            os.chdir(plat)
            vtune = 'amplxe-cl -collect general-exploration -quiet taskset -c 1 '
            cmd = vtune + ' ' + run_kernel(k, plat) + ' > sirius-suite.out 2> sirius-suite.err'
            print cmd
            shcmd(cmd)
            report ='amplxe-cl -report summary -report-output sirius-suite.report -format csv -csv-delimiter ,'
            shcmd(report)
            # remove leading/trailing spaces, fucking vtune
            shcmd("cat sirius-suite.report | sed 's/^[ \t]*//;s/[ \t]*$//' > temp.txt && mv temp.txt sirius-suite.report")
            shcmd('rm -rf r00*')
            os.chdir(kroot)
        os.chdir(root)

if __name__=='__main__':
    sys.exit(main(sys.argv))
