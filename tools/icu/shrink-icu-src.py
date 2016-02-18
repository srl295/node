#!/usr/bin/env python
import optparse
import os
import pprint
import re
import shlex
import subprocess
import sys
import shutil
import string

parser = optparse.OptionParser()

parser.add_option('--icu-small',
    action='store',
    dest='icusmall',
    default='deps/icu-small',
    help='path to target ICU directory to shrink. Will be deleted.')

parser.add_option('--icu-src',
    action='store',
    dest='icusrc',
    default='deps/icu',
    help='path to source ICU directory.')

parser.add_option('--icutmp',
    action='store',
    dest='icutmp',
    default='out/Release/gen/icutmp',
    help='path to icutmp dir.')


(options, args) = parser.parse_args()

if os.path.isdir(options.icusmall):
    print 'Deleting existing icusmall %s' % (options.icusmall)
    shutil.rmtree(options.icusmall)

if not os.path.isdir(options.icusrc):
    print 'Missing source ICU dir --icusrc=%' % (options.icusrc)
    sys.exit(1)



ignore_regex = re.compile('^.*\.(vcxproj|filters|nrm|icu|dat)$')

def icu_ignore(dir, files):
    subdir = dir[len(options.icusrc)+1::]
    ign = []
    if len(subdir) == 0:
        ign = ign + files
        ign.remove('source')
        ign.remove('license.html')
    elif subdir == 'source':
        ign = ign + ['layout','samples','test','extra','config','layoutex','allinone']
    elif subdir == 'source/tools':
        ign = ign + ['tzcode','ctestfw','gensprep','gennorm2','gendict','icuswap',
        'genbrk','gencfu','gencolusb','genren','memcheck']
    elif subdir == 'source/data':
        ign = ign + ['unidata','curr','zone','unit','lang','region','misc','sprep']
    # else:
        # print '!%s! [%s]' % (subdir, files)
    ign = ign + ['.DS_Store', 'Makefile', 'Makefile.in']
    
    for file in files:
        if ignore_regex.match(file):
            ign = ign + [file]
    
    # print '>%s< [%s]' % (subdir, ign)
    return ign

# copied from configure
def icu_info(icu_full_path):
    uvernum_h = os.path.join(icu_full_path, 'source/common/unicode/uvernum.h')
    if not os.path.isfile(uvernum_h):
        print ' Error: could not load %s - is ICU installed?' % uvernum_h
        sys.exit(1)
    icu_ver_major = None
    matchVerExp = r'^\s*#define\s+U_ICU_VERSION_SHORT\s+"([^"]*)".*'
    match_version = re.compile(matchVerExp)
    for line in open(uvernum_h).readlines():
        m = match_version.match(line)
        if m:
            icu_ver_major = m.group(1)
    if not icu_ver_major:
        print ' Could not read U_ICU_VERSION_SHORT version from %s' % uvernum_h
        sys.exit(1)
    icu_endianness = sys.byteorder[0];  # TODO(srl295): EBCDIC should be 'e'
    return (icu_ver_major, icu_endianness)

(icu_ver_major, icu_endianness) = icu_info(options.icusrc)
print "icudt%s%s" % (icu_ver_major, icu_endianness)

src_datafile = os.path.join(options.icutmp, "icusmdt%s.dat" % (icu_ver_major))
dst_datafile = os.path.join(options.icusmall, "source","data","in", "icudt%s%s.dat" % (icu_ver_major, icu_endianness))

if not os.path.isfile(src_datafile):
    print "Could not find source datafile %s - did you build small-icu node?" % src_datafile
    sys.exit(1)
else:
    print "will use small datafile %s" % (src_datafile)
print '%s --> %s' % (options.icusrc, options.icusmall)
shutil.copytree(options.icusrc, options.icusmall, ignore=icu_ignore)
print '%s --> %s' % (src_datafile, dst_datafile)
shutil.copy(src_datafile, dst_datafile)



# OK, now copy the data file