#!/usr/bin/env python3
'''Compare symbols within two object (or archive) files'''
import argparse, subprocess

#lib_old = '/usr/local/lib/libliquid.ar'
lib_old = '../liquid-dsp-1.3.2/libliquid.ar'
lib_new = 'libliquid.ar'
opts = '-A'

def nmparse(lib,opts='-A',filt='T'):
    '''run command-line 'nm' program and parse results, filtering out unwanted symbols'''
    out = subprocess.check_output(['nm',opts,lib]).decode('utf-8').split('\n')[:-1]
    out = filter(lambda v: v.split()[-2] in filt, out)
    return set(v.split()[-1] for v in out)

syms_old = nmparse(lib_old)
syms_new = nmparse(lib_new)

# compute removed & added symbols
syms_removed = syms_old.difference(syms_new)
syms_added   = syms_new.difference(syms_old)

# print basic report
print('%32s [old] : %u symbols' % (lib_old,  len(syms_old)))
print('%32s [new] : %u symbols' % (lib_new,  len(syms_new)))
print('      %32s : %u symbols' % ('removed',len(syms_removed)))
print('      %32s : %u symbols' % ('added',  len(syms_added)))

#print('removed:', syms_removed)
#print('added:', syms_added)

print('removed:')
for s in sorted(syms_removed):
    print(' ',s)
