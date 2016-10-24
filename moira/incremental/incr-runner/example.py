#!/moira/bin/python

import os
import struct
import sys
from subprocess import call
from time import ctime, sleep

# Replace this with the stop file path for this incremental.
stop_file = '/moira/myincremental/nomyincremental'

# Display an informational message to incr-runner.log.
def log(msg):
    print progname + ': ' + msg


# Write a message to critical.log and send a zephyr message to -c moira.
def critical_log(msg):
    log('critical error: ' + msg)
    timestr = ctime()[4:-5]
    with open('/moira/critical.log', 'a') as f:
        f.write('%s <%d>%s: %s\n' % (timestr, os.getpid(), progname, msg))
    zwrite = '/usr/local/bin/zwrite'
    if os.path.exists(zwrite):
        call([zwrite, '-q', '-d', '-n', '-c', 'moira', '-i', 'incremental',
              '-m', progname + ': ' + msg])


def do_incremental(args):
    print progname + ' ' + ' '.join(args)

    # Pause while the stop file exists.  Complain every half hour if
    # it exists for a long time.
    stop_check_count = 0
    while os.path.exists(stop_file):
        sleep(60)
        if stop_check_count % 30 == 1:
            critical_log('%s exists (half-hour warning)' % stop_file)
        stop_check_count += 1

    table = args[0]
    beforec = int(args[1])
    afterc = int(args[2])
    before = sys.argv[3:3+beforec]
    after = sys.argv[3+beforec:3+beforec+afterc]
    # Replace this with a call to process the incremental for real.
    # real_incremental_function_here(table, before, after)


progname = os.path.basename(sys.argv[0])
if len(sys.argv) > 1:
    # We were run as a per-incremental child process, or run by hand.
    do_incremental(sys.argv[1:])
else:
    # We were run as a pipe child process.
    print progname + ' pipe child starting'
    sys.stdout.flush()
    while True:
        # Each update is a four-byte length followed by the arguments,
        # with a null byte after each argument.
        lenbytes = sys.stdin.read(4)
        if len(lenbytes) == 0:
            print progname + ' pipe child exiting'
            break
        l, = struct.unpack('>L', lenbytes)
        msg = sys.stdin.read(l)
        args = msg.split('\0')[:-1]
        do_incremental(args)
