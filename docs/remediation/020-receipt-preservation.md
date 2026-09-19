# MDPS-AUDIT2-020 remediation

## Defect
A submitter-provided root `receipt.json` was accepted, omitted from the package-wide evidence hash map, and then overwritten by Archive Mode's generated receipt. Rejected packages were also mutated before preservation.

## Correction
- Reserve root `receipt.json` during validation before package mutation.
- Hash every submitted regular file on accepted packages. There is no `receipt.json` exemption.
- Preserve rejected package directories byte-for-byte and write the Archive-generated rejection receipt as a sibling `<rejected-directory>.receipt.json`.
- Leave retryable tool/storage failures in Pending unchanged.

## Regression
`tests/archive-integration-tests.py::test_submitted_receipt_is_reserved_and_rejection_preserves_every_byte` snapshots every submitted file, injects an operator receipt, verifies deterministic rejection, proves the moved rejected directory is byte-identical, verifies the operator receipt survives, verifies the sibling Archive receipt records the reason, and verifies canonical state is unchanged.

Local reconstructed-current-Archive test result on 2026-09-20: `archive-integration` passed after restoring the retained Qt runtime library path. The first invocation failed before application startup because `libb2.so.1` was absent from the test-process library path; no product assertion executed in that attempt.

Final status remains PR-OPEN until exact-head Linux/Windows CI and post-integration qualification succeed.
