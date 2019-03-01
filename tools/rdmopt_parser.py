#!/usr/bin/env python
# enumvar.lua -> const.txt constants converter
# Author: Secret <secret@rathena.org>
# Copyright (C) 2019 rAthena Development Team - Licensed under GNU GPL
# For more information, see LICENSE in the main folder
import re
import sys

def parse_enumvar(path = 'enumvar.lub'):
    """Parse enumvar.lub file.
    Args:
        path: Path to enumvar.lub

    Returns:
        List of random options and their ID
    """
    enumvar_file = open(path, 'r')
    content = enumvar_file.read()
    p = re.compile('\t(.+) = \{ (\d+), \d+ \},')
    return p.findall(content)

def main(argv):
    matches = parse_enumvar(argv[1]) if len(argv) > 1 else parse_enumvar()
    consts = [x[0] + '\t' + x[1] for x in matches]
    for x in consts:
        print("RDMOPT_" + x)

if __name__ == '__main__':
    main(sys.argv)
