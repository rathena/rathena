# Copyright (c) 2013, Scott Tsai
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


# Usage: gdb -x debugbreak-gdb.py
# (gdb) debugbreak-step
# (gdb) debugbreak-continue
#
# To debug:
# (gdb) set python print-stack full

import gdb
import re
        
def _gdb_show_version_parse(version_str):
    '''
    >>> s0 = 'This GDB was configured as "x86_64-redhat-linux-gnu".'
    >>> s1 = 'This GDB was configured as "--host=i686-build_pc-linux-gnu --target=arm-linux-gnueabihf".'
    >>> s2 = 'This GDB was configured as "x86_64-unknown-linux-gnu".'
    >>> _gdb_show_version_parse(s0) == dict(target='x86_64-redhat-linux-gnu')
    True
    >>> _gdb_show_version_parse(s1) == dict(host='i686-build_pc-linux-gnu', target='arm-linux-gnueabihf')
    True
    >>> _gdb_show_version_parse(s2) == dict(target='x86_64-unknown-linux-gnu')
    True
    '''
    
    t = version_str
    msg = 'This GDB was configured as "'
    s = t.find(msg)
    if s == -1:
        raise ValueError
    s += len(msg)
    e = t.find('".', s)
    if e == -1:
        raise ValueError

    config = t[s:e]
    d = {}
    for i in config.split():
        i = i.strip()
        if i.startswith('--'):
            (k, v) = i[2:].split('=')
            d[k] = v
        else:
            if not i:
                continue
            d['target'] = i
    return d

def _target_triplet():
    '''
    -> 'arm-linux-gnueabihf' or 'x86_64-redhat-linux-gnu' or ...
        
    >>> import re
    >>> not not re.match(r'\w*-\w*-\w*', target_triplet())
    True
    '''
    t = gdb.execute('show version', to_string=True)
    return _gdb_show_version_parse(t)['target']

temp_breakpoint_num = None

def on_stop_event(e):
    global temp_breakpoint_num
    if not isinstance(e, gdb.BreakpointEvent):
        return
    for bp in e.breakpoints:
        if bp.number == temp_breakpoint_num:
            bp.delete()
            gdb.events.stop.disconnect(on_stop_event)
            l = gdb.find_pc_line(int(gdb.parse_and_eval('$pc'))).line
            gdb.execute('list %d, %d' % (l, l))
            break

def _next_instn_jump_len(gdb_frame):
    '-> None means don\'t jump'
    try:
        arch_name = gdb_frame.architecture().name()
    except AttributeError:
        arch_name = None

    if arch_name.startswith('powerpc:'):
        # 'powerpc:common64' on ppc64 big endian
        i = bytes(gdb.selected_inferior().read_memory(gdb.parse_and_eval('$pc'), 4))
        if (i == b'\x7d\x82\x10\x08') or (i == b'\x08\x10\x82\x7d'):
            return 4
        else: # not stopped on a breakpoint instruction
            return None

    triplet = _target_triplet()
    if re.match(r'^arm-', triplet):
        i = bytes(gdb.selected_inferior().read_memory(gdb.parse_and_eval('$pc'), 4))
        if i == b'\xf0\x01\xf0\xe7':
            return 4
        elif i.startswith(b'\x01\xde'):
            return 2
        elif i == b'\xf0\xf7\x00\xa0 ':
            # 'arm_linux_thumb2_le_breakpoint' from arm-linux-tdep.c in GDB 
            return 4
        else: # not stopped on a breakpoint instruction
            return None
    return None

def _debugbreak_step():
    global temp_breakpoint_num
    try:
        frame = gdb.selected_frame()
    except gdb.error as e:
        # 'No frame is currently selected.'
        gdb.write(e.args[0] + '\n', gdb.STDERR)
        return
    instn_len = _next_instn_jump_len(frame)

    if instn_len is None:
        gdb.execute('stepi')
    else:
        loc = '*($pc + %d)' % (instn_len,)
        bp = gdb.Breakpoint(loc, gdb.BP_BREAKPOINT, internal=True)
        bp.silent = True
        temp_breakpoint_num = bp.number
        gdb.events.stop.connect(on_stop_event)
        gdb.execute('jump  ' + loc)

def _debugbreak_continue():
    try:
        frame = gdb.selected_frame()
    except gdb.error as e:
        # 'No frame is currently selected.'
        gdb.write(e.args[0] + '\n', gdb.STDERR)
        return
    instn_len = _next_instn_jump_len(frame)

    if instn_len is None:
        gdb.execute('continue')
    else:
        loc = '*($pc + %d)' % (instn_len,)
        gdb.execute('jump  ' + loc)

class _DebugBreakStep(gdb.Command):
    '''Usage: debugbreak-step
    Step one instruction after a debug_break() breakpoint hit'''

    def __init__(self):
        gdb.Command.__init__(self, 'debugbreak-step', gdb.COMMAND_BREAKPOINTS, gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        _debugbreak_step()

class _DebugBreakContinue(gdb.Command):
    '''Usage: debugbreak-continue
    Continue execution after a debug_break() breakpoint hit'''

    def __init__(self):
        gdb.Command.__init__(self, 'debugbreak-continue', gdb.COMMAND_BREAKPOINTS, gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        _debugbreak_continue()

_DebugBreakStep()
_DebugBreakContinue()
