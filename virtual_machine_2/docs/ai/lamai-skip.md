# Skip source interpretation for units with interfaces

Applied scope: update the test runner only.

- When a test source has a sibling `.i` file, do not run or benchmark `lamai`.
- Report that implementation as `SKIP`, without a runtime measurement.
- Compare `lamavm` and `lamavm2` normally. Skipping `lamai` does not downgrade
  a passing case or hide VM failures.
- Without a `lamai` measurement, display relative timings as `n/a`.
- Sources without a sibling `.i` file retain the existing behavior.

Validate import fixtures and ordinary examples with two runs per implementation,
and run the Python style checks.
