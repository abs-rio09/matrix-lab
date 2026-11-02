# Changelog

All notable changes to this project are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-11-02

### Added
- Matrix addition, subtraction, multiplication and transpose
- Determinant using Gaussian elimination with partial pivoting
- Reading matrices from text files with full validation
- Command-line interface with `--help`, `--version` and clear exit codes
- 21 Python `unittest` cases, `make memcheck` with Valgrind
- GitHub Actions workflow that builds, tests and leak-checks every push

### Fixed (found during testing, before release)
- Output kept only 4 significant digits, so `12345.678` printed as `1.235e+04`. Output now keeps 10 significant digits.
- A file with more values than its header declared was silently accepted. Extra values are now rejected as an invalid matrix.
- A result of negative zero could print as `-0`. It now prints as `0`.
