# matrix-lab

![language](https://img.shields.io/badge/language-C11-007ec6) ![tests](https://img.shields.io/badge/tests-21%20passing-44cc11) ![license](https://img.shields.io/badge/license-MIT-97ca00)

> A small, bounds-checked matrix calculator in C, tested with Python `unittest`.

matrix-lab adds, subtracts, multiplies and transposes matrices and computes determinants from the command line. Every function checks dimensions, file input and memory allocation and returns a clear error instead of crashing or reading out of bounds.

## Features

- Addition, subtraction, multiplication and transpose of any size up to 1000 × 1000
- Determinant by Gaussian elimination with partial pivoting, O(n³)
- Reads matrices from plain text files and rejects malformed ones (too few, too many or non-numeric values, negative or huge sizes)
- Clear error messages and exit codes: `0` success, `1` usage error, `2` input or math error
- No memory leaks (checked with Valgrind) and clean under AddressSanitizer and UndefinedBehaviorSanitizer
- 21 automated tests, run on every push with GitHub Actions

## Getting Started

### Prerequisites

- GCC (C11) and GNU Make
- Python 3 (only for the tests)
- Valgrind (optional, for `make memcheck`)

On Ubuntu:

```bash
sudo apt install build-essential python3 valgrind
```

### Build

```bash
git clone https://github.com/abs-rio09/matrix-lab.git
cd matrix-lab
make
```

The binary is created at `build/matrix`.

## Usage

```bash
./build/matrix add A.txt B.txt      # A + B
./build/matrix sub A.txt B.txt      # A - B
./build/matrix mul A.txt B.txt      # A x B
./build/matrix transpose A.txt      # A^T
./build/matrix det A.txt            # determinant of A
```

### Matrix file format

The first line gives the size (`rows cols`), followed by the values row by row:

```text
2 3
1 2 3
4 5 6
```

### Example

```bash
$ ./build/matrix mul examples/a.txt examples/b.txt
          58           64
         139          154

$ ./build/matrix det examples/square.txt
49

$ ./build/matrix add examples/a.txt examples/b.txt
error: add: matrix dimensions do not match for this operation
```

## Testing

```bash
make test        # 21 unittest cases against the compiled binary
make memcheck    # Valgrind leak check
```

## Project Structure

```text
matrix-lab/
├── src/
│   ├── matrix.h          # public API and status codes
│   ├── matrix.c          # library implementation
│   └── main.c            # command-line interface
├── tests/
│   └── test_matrix.py    # Python unittest suite
├── examples/             # sample matrix files
├── .github/workflows/    # CI: build + tests + Valgrind on every push
├── Makefile
├── CHANGELOG.md
└── LICENSE
```

## License

Distributed under the MIT License. See `LICENSE` for details.

## Author

**Abu Bakkar Siddik** · GitHub: [@abs-rio09](https://github.com/abs-rio09)
