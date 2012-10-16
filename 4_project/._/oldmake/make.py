#!/usr/bin/env python

import os
import sys
import os.path
import string

CFLAGS = '-std=c++0x -Wall -O3 -Isrc'

mkdirs = lambda d: os.path.isdir(d) or os.makedirs(d)

def cmd(s):
    print s
    sys.stdout.flush()
    return os.system(s)

def panic():
    print '(panic: exit=-1)'
    sys.stdout.flush()
    sys.exit(-1)

def last_modify(f):
    assert os.path.isfile(f)
    return os.stat(f).st_mtime

def make_obj(cppfile, header_files, force=False):
    assert os.path.exists(cppfile)
    last_time = last_modify(cppfile)
    for header_file in header_files:
        assert os.path.exists(header_file)
        last_time = max(last_time, last_modify(header_file))

    assert cppfile.endswith('.cpp')
    assert cppfile.startswith('src/')
    objfile = 'obj/%s.o' % cppfile[len('src/'):-len('.cpp')]

    if not force \
            and os.path.exists(objfile) \
            and last_modify(objfile) > last_time:
        return 0, objfile # the objfile is already up to date

    return cmd('g++ %s %s -c -o %s' % (CFLAGS, cppfile, objfile)), objfile

def lsdir(d):
    files = os.listdir(d); files.sort()
    return [ '%s/%s' % (d, f) for f in files ]

is_header = lambda f: os.path.isfile(f) and f.endswith('.h')
is_cpp = lambda f: os.path.isfile(f) and f.endswith('.cpp')
list_header_files = lambda pack: [ f for f in lsdir('src/%s' % pack) if is_header(f) ]

def make_objs(pack, dependencies=None, force=False):
    files = lsdir('src/%s' % pack)
    cpp_files = [ f for f in files if is_cpp(f) ]
    header_files = [ f for f in files if is_header(f) ]
    if dependencies is not None: 
        for p in dependencies:
            header_files += list_header_files(p)
    
    obj_dir = 'obj/%s' % pack
    mkdirs(obj_dir)

    ret = list()

    for cpp_file in cpp_files:
        retval, objfile = make_obj(cpp_file, header_files, force)
        if retval != 0: 
            panic()
        ret.append(objfile)
    return ret

def make_output(output, objs, force=False):
    if not force \
            and os.path.exists(output) \
            and last_modify(output) > max([ last_modify(f) for f in objs ]):
        return 
    retval = cmd('g++ %s %s -o %s' % (CFLAGS, string.join(objs, ' '), output))
    if retval != 0:
        panic()

def make_all():
    objs = make_objs('h8')
    objs += make_objs('navy', ['h8'])
    objs += make_objs('simu', ['h8', 'navy'])
    objs += make_objs('main', ['h8', 'navy', 'simu'])
    make_output('run', objs)

if __name__ == '__main__':
    make_all()
