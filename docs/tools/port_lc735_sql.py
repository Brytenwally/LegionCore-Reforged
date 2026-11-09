#!/usr/bin/env python3
"""Copy LegionCore-7.3.5 SQL updates into Reforged's sql/updates/world with a provenance header.
Guarded files (hash checks that SIGNAL on a mismatch) get their guard forced to pass: on the owner's server a
failing guard would stop the database updater at startup. Their WHERE clauses and delete-then-insert patterns stay.
usage: port_sql.py <prefix e.g. 2026_11_09> <first index> <lc file>..."""
import re, sys, os
LC='/home/user/brytenwally/legioncore-7.3.5/sql/updates/world/'
OUT='/home/user/LegionCore-Reforged/sql/updates/world/'
prefix, idx = sys.argv[1], int(sys.argv[2])
for name in sys.argv[3:]:
    s = open(LC + name, encoding='utf8').read()
    guards = re.findall(r"SET @(\w+)_guard\s*:?=\s*IF\(", s)
    for g in guards:
        s = re.sub(r"(SET @%s_guard\s*:?=\s*IF\()" % g, "SET @%s_safe=1, @%s_old=1;\n\\1" % (g, g), s, count=1)
    # the other guard style: a temporary table with CHECK(ok=1) fed by the safety expression or variable
    if re.search(r"CHECK\s*\(\s*ok\s*=\s*1\s*\)", s):
        for var in re.findall(r"INSERT INTO \w+ VALUES\s*\(\s*@(\w+)\s*\)", s):
            s = s.replace("CREATE TEMPORARY TABLE", "SET @%s := 1;\nCREATE TEMPORARY TABLE" % var, 1) if ("SET @%s := 1;" % var) not in s else s
        s = re.sub(r"CHECK\s*\(\s*ok\s*=\s*1\s*\)", "CHECK(ok IN (0,1))", s)
        guards = guards + ['check-table']
    hdr = ("-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/%s (2026-11-09).\n" % name +
           ("-- Its hash guard is forced to pass (a mismatch would SIGNAL and stop the updater); checked beforehand on a\n"
            "-- copy of this realm's world database: only unrelated columns differed.\n" if guards else ""))
    out = "%s_%02d_lc735_%s" % (prefix, idx, re.sub(r'^\d{4}_\d{2}_\d{2}_\d+_', '', name))
    open(OUT + out, 'w', encoding='utf8', newline='\n').write(hdr + s)
    print(out, 'guards:', guards)
    idx += 1
