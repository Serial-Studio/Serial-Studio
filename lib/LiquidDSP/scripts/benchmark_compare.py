#!/usr/bin/env python3
'''Compare benchmark results'''
import argparse, json, numpy as np

# parse command-line arguments
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('old', help='old benchmark file (.json)')
p.add_argument('new', help='new benchmark file (.json)')
p.add_argument('-thresh', default=1.5, type=float, help='threshold for displaying deltas')
args = p.parse_args()

# load json files
v0 = json.load(open(args.old,'r'))
v1 = json.load(open(args.new,'r'))

# convert to dictionary and get set of common names
b0 = {v['name']:v for v in v0['benchmarks']}
b1 = {v['name']:v for v in v1['benchmarks']}
set_common = set(b0.keys()).intersection(set(b1.keys()))

# iterate over common benchmarks and print results
for key in set_common:
    r0,r1 = b0[key], b1[key]
    if 0 in (r0['trials'],r1['trials']):
        continue
    rate = r1['rate'] / r0['rate']
    if np.exp(np.abs(np.log(rate))) < args.thresh:
        continue

    print(' %32s, rate = %12.8f' % (key, rate))

