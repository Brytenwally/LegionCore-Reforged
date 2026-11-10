#!/usr/bin/env python3
"""Report C++ script names registered more than once (the worldserver stops at startup with
"Script '<name>' already assigned with the same script name") and AddSC_ functions a loader calls
twice. Run from the repository root after merging or copying scripts."""
import collections, os, re, sys

names = collections.defaultdict(list)
calls = collections.defaultdict(list)
for root in ('src/server/scripts', 'modules'):
    for d, _, files in os.walk(root):
        for f in files:
            if not f.endswith(('.cpp', '.h')):
                continue
            path = os.path.join(d, f)
            text = re.sub(r'//[^\n]*', '', open(path, errors='replace').read())
            for m in re.finditer(r'(?:Script|Loader|AI)\s*\(\s*"(\w+)"\s*\)', text):
                names[m.group(1)].append(path)
            for m in re.finditer(r'\bRegister(?:\w*Script|\w*AI)\(\s*(\w+)\s*(?:,\s*\w+)?\)', text):
                names[m.group(1)].append(path)
            if 'loader' in f:
                for m in re.finditer(r'^\s+(AddSC_\w+)\(\);', text, re.M):
                    calls[(path, m.group(1))].append(path)
bad = 0
for name, where in sorted(names.items()):
    if len(where) > 1:
        bad += 1
        print('script name %s registered %d times: %s' % (name, len(where), ', '.join(sorted(set(where)))))
for (path, fn), where in sorted(calls.items()):
    if len(where) > 1:
        bad += 1
        print('%s calls %s %d times' % (path, fn, len(where)))
print('%d script names checked, %d problem(s)' % (len(names), bad))
sys.exit(1 if bad else 0)
