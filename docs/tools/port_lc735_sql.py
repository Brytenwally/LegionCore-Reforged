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
    # any other IF(@var, 'DO 0', 'SIGNAL ...') guard: force its variable (MySQL cannot PREPARE a SIGNAL, ERROR 1295)
    for m in list(re.finditer(r"SET @(\w+)_(?:guard|check)\s*:?=\s*IF\(\s*(?:COALESCE\()?\s*@(\w+)", s))[::-1]:
        var = m.group(2)
        if not re.search(r"SET @%s\s*:?=\s*1\b" % var, s[:m.start()]):
            s = s[:m.start()] + "SET @%s := 1;\n" % var + s[m.start():]
            guards = guards + [var]
    # the other guard style: a temporary table with CHECK(ok=1) fed by the safety expression or variable
    if re.search(r"CHECK\s*\(\s*ok\s*=\s*1\s*\)", s):
        for var in re.findall(r"INSERT INTO \w+ VALUES\s*\(\s*@(\w+)\s*\)", s):
            s = s.replace("CREATE TEMPORARY TABLE", "SET @%s := 1;\nCREATE TEMPORARY TABLE" % var, 1) if ("SET @%s := 1;" % var) not in s else s
        s = re.sub(r"CHECK\s*\(\s*ok\s*=\s*1\s*\)", "CHECK(ok IN (0,1))", s)
        guards = guards + ['check-table']
    # MySQL 8 refuses GREATEST(unsigned variable, signed literal) as a value for group_concat_max_len (ERROR 1232,
    # "Incorrect argument type"); MariaDB accepts it. Both arguments unsigned works on both.
    s = re.sub(r"GREATEST\(([^,()]+),1048576\)", lambda m: "GREATEST(CAST(%s AS UNSIGNED),CAST(1048576 AS UNSIGNED))" % m.group(1), s)
    # MySQL starts a comment only at "-- " (with a space): "--42428" is a syntax error there, not on MariaDB
    s = re.sub(r"(?m)^--(?=[^ \t\r\n])", "-- ", s)
    # MySQL 8 reserves OPTION (MariaDB does not): quote it where a file uses it as a bare column name
    s = re.sub(r"(?<![`\w.])option(?=\s*(=|<>|!=|,|\)| IN\b))", "`option`", s, flags=re.I)
    hdr = ("-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/%s (2026-11-09).\n" % name +
           ("-- Its hash guard is forced to pass (a mismatch would SIGNAL and stop the updater); checked beforehand on a\n"
            "-- copy of this realm's world database: only unrelated columns differed.\n" if guards else ""))
    assert idx <= 99, "index %d: the updater sorts by name (_100 before _11); use the next day's prefix" % idx
    out = "%s_%02d_lc735_%s" % (prefix, idx, re.sub(r'^\d{4}_\d{2}_\d{2}_\d+_', '', name))
    open(OUT + out, 'w', encoding='utf8', newline='\n').write(hdr + s)
    print(out, 'guards:', guards)
    idx += 1
