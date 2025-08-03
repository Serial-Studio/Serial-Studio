#!/usr/bin/env python3
'''Run valgrind on each autotest and catalog memory errors'''
import argparse, json, os, sys, subprocess

def generate_json(path: str='valgrind', overwrite=False):
    filename = path + '/init.json'
    if overwrite or not os.path.exists(filename):
        os.system('./xautotest -t0 -o %s' % (filename,))
    return json.load(open(filename,'r'))

def run_test(test: dict, opts: str='--tool=memcheck --leak-check=full --track-origins=yes',
        path: str='valgrind', seed: int=1, debug: bool=True):
    '''run a specific test'''
    filename = '%s/memcheck.%.4u.%s.log' % (path, test['id'], test['name'])
    cmd = 'valgrind %s --log-file=%s ./xautotest -t %u -R %u' % (opts, filename, test['id'], seed)
    if debug: print(cmd)
    os.system(cmd)
    # TODO parse result

def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('-output', default='valgrind', type=str, help='output directory')
    p.add_argument('-search', default=None,       type=str, help='search')
    p.add_argument('-test',   default=None,       type=int, help='run a specific test')
    args = p.parse_args()

    # generate and load .json object
    v = generate_json(args.output)

    # set additional configuration options
    opts = '--tool=memcheck --leak-check=full --track-origins=yes'

    if args.test is not None:
        # run a specific test
        run_test(v['tests'][args.test], opts, seed=v['rseed'])
    elif args.search is not None:
        # run all tests matching search string
        print("running all tests containing '%s'...\n" % (args.search))
        for i,test in enumerate(v['tests']):
            if test['name'].lower().__contains__(args.search.lower()):
                run_test(test, opts, seed=v['rseed'])
    else:
        # iterate over all tests and execute
        for i,test in enumerate(v['tests']):
            run_test(test, opts, seed=v['rseed'])

if __name__ == '__main__':
    sys.exit(main())

