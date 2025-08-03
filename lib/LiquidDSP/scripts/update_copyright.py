#!/usr/bin/python

'''
This script updates the copyright license lines for all files within a
particular directory.

usage:
  $ python scripts/update_copyright.py <dir>
'''

import os
import sys
import re

def usage():
    print __doc__

if len(sys.argv) != 2:
    sys.stderr.write("error: please provide an input directory\n")
    usage()
    sys.exit(-1)
elif sys.argv[1] in ('-h','--help'):
    usage()
    sys.exit(0)

# new copyright
newcopy = '''\
Copyright (c) 2007 - 2015 Joseph Gaeddert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.'''.splitlines()

# add new line character to end of each line
for i in xrange(len(newcopy)):
    newcopy[i] += '\n'

rootdir = sys.argv[1]

# specific files to ignore
ignore_files = ['update_copyright.py',]

# directories to ignore
ignore_directories = ['.git',]

# only look at these extensions
include_extensions = ['.h', '.c', '.md', '.tex', '.ac', '.in', '.m',]

# print alignment for status
align = 56

#
def update_copyright(filename=""):
    """
    Update copyright on file.
    """
    # try to open file
    contents = []
    with open( filename, 'r' ) as f:
        # read lines from file (removing newline character at end)
        for line in f:
            #contents.append(line.strip('\n\r'))
            contents.append(line)

    # parse contents
    index_start = -1
    index_stop  = -1

    # search for copyright; starts at top of file
    for i in range(min(10, len(contents))):
        if re.search(r'Copyright .* Joseph Gaeddert',contents[i]):
            index_start = i
            break

    if index_start == -1:
        print "  " + filename + ":" + " "*(align-len(filename)) + "no copyright found"
        return

    # look for end of copyright
    #for i in range(index_start+15,index_start+15+min(10, len(contents))):
    i = index_start + 15
    if re.search(r'along with liquid.  If not, see',contents[i]):
        index_stop = i
    elif re.search(r'AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM',contents[i]):
        print "  " + filename + ":" + " "*(align-len(filename)) + "copyright already updated"
        return
    else:
        print "  " + filename + ":" + " "*(align-len(filename)) + "could not find end of copyright"
        return

    # check comment type
    m = re.search(r'^( \*|#+) +Copyright',contents[index_start])
    if m:
        comment = m.group(1)
    else:
        raise Exception('unexpected error')

    # delete items in list
    contents.__delslice__(index_start, index_stop+1)

    # insert new copyright
    for i in range(len(newcopy)):
        # only add space after comment characters if string is not empty
        # (e.g. print ' *' instead of ' * ')
        space = ' ' if len(newcopy[i].strip()) > 0 else ''

        # insert new comment
        contents.insert(index_start+i, comment + space + newcopy[i])

    # open original file for writing
    with open( filename, 'w' ) as f:
        for line in contents:
            f.write(line)
            
    print "  " + filename + ":" + " "*(align-len(filename)) + "updated"

#
for root, subFolders, files in os.walk(rootdir):

    # strip off leading './' if it exists
    if root.startswith('./'):
        root = root[2:]

    # print root directory
    print root
    
    # ignore certain directories
    if root in ignore_directories:
        print "(skipping directory)"
        continue

    # print subfolders
    #for folder in subFolders:
    #    print "%s has subdirectory %s" % (root, folder)

    # parse each filename in directory
    for filename in files:
        filePath = os.path.join(root, filename)

        # check filename
        if filePath in ignore_files:
            print "  " + filePath + ":" + " "*(align-len(filePath)) + "ignoring this specific file"
            continue;

        # check filename extension
        baseName, extension = os.path.splitext(filename)
        if extension not in include_extensions:
            print "  " + filePath + ":" + " "*(align-len(filePath)) + "improper extension; ignoring file"
            continue;

        # continue on with this file
        update_copyright(filePath)


