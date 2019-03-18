Truecrypt-Passwort: lntest

lntest_fileid_src.tc enthält das Laufwerk, das als Source fungiert. Es enthält eine Junction zu P:\Testjunction

lntest_fileid_junctions.tc enthält das Verzeichnis Testjunction, dessen File-IDs sich mit Dir1 in lntest_fileid_src.tc überschneiden. Wenn lntest_fileid_junctions.tc als P: gemountet wird, funktioniert die Junction aus lntest_fileid_src.tc.

Wenn nun die gemountete lntest_fileid_src.tc mit --copy --unroll kopiert wird, werden ca. 50% der Dateien wegen ID-Überlappung falsch verlinkt.