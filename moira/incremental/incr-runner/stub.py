#!/moira/bin/python

import os
import struct
import sys

progname = os.path.basename(sys.argv[0])
if len(sys.argv) > 1:
    print progname + ' ' + ' '.join(sys.argv[1:])
else:
    print progname + ' pipe child starting'
    sys.stdout.flush()
    while True:
        lenbytes = sys.stdin.read(4)
        if len(lenbytes) == 0:
            print progname + ' pipe child exiting'
            break
        l, = struct.unpack('>L', lenbytes)
        print progname + ' ' + ' '.join(sys.stdin.read(l).split('\0')[:-1])
        sys.stdout.flush()
