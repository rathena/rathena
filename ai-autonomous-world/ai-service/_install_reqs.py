#!/usr/bin/env python3
"""Parse requirements.txt, skip comments/empty/aiofiles, print pip install args."""
import re

with open('requirements.txt') as f:
    lines = f.readlines()

pkgs = []
for line in lines:
    line = line.strip()
    if not line or line.startswith('#'):
        continue
    if 'aiofiles' in line:
        continue
    pkgs.append(line)

print(' '.join(pkgs))
