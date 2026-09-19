"""Regression policy for MDPS-AUDIT2-101 Windows extended-path namespaces."""
from __future__ import annotations

import argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("--source-root", required=True, type=Path)
source = (p.parse_args().source_root / "src/directoryEntries.cpp").read_text(encoding="utf-8")

start = source.index("std::wstring setPath")
end = source.index("\n\tclass handle", start)
body = source[start:end]

# Existing extended paths must be preserved.
assert 'clean.startsWith( "//?/" )' in body

# Normal UNC paths must use the Win32 extended UNC namespace rather than
# the invalid generic prefix form.
assert 'clean.startsWith( "//" )' in body
assert 'qualified = "//?/UNC/" + clean.mid( 2 )' in body

# Drive-qualified absolute paths still use the normal extended namespace.
assert "QDir::isAbsolutePath( clean )" in body
assert 'qualified = "//?/" + clean' in body

# Relative inputs must fail closed instead of being blindly prefixed.
assert "return {} ;" in body

# Guard against reintroducing the original unconditional prefix expression.
assert 'path.startsWith( "\\\\\\\\?\\\\\\" ) ? path' not in body

print("Windows extended path namespace policy: PASS")
