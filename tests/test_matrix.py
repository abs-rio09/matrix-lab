"""Black-box tests for the matrix-lab command-line tool.

Each test writes small matrix files to a temporary folder, runs the compiled
binary with subprocess, and checks stdout, stderr and the exit code.
Run with:  make test
"""
import os
import subprocess
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "build", "matrix")


def parse(text):
    """Turn printed matrix output into a list of rows of floats."""
    return [[float(x) for x in line.split()] for line in text.strip().splitlines()]


class MatrixCLITest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tmp.cleanup()

    # ---- helpers -------------------------------------------------------
    def write(self, name, rows):
        """Save a matrix (list of lists) in the tool's file format."""
        path = os.path.join(self.tmp.name, name)
        with open(path, "w") as f:
            f.write(f"{len(rows)} {len(rows[0])}\n")
            for r in rows:
                f.write(" ".join(str(v) for v in r) + "\n")
        return path

    def write_raw(self, name, text):
        path = os.path.join(self.tmp.name, name)
        with open(path, "w") as f:
            f.write(text)
        return path

    def run_cli(self, *args):
        return subprocess.run([BIN, *args], capture_output=True, text=True, timeout=10)

    def assertMatrix(self, result, expected):
        self.assertEqual(result.returncode, 0, msg=result.stderr)
        got = parse(result.stdout)
        self.assertEqual(len(got), len(expected))
        for g_row, e_row in zip(got, expected):
            for g, e in zip(g_row, e_row):
                self.assertAlmostEqual(g, e, places=6)

    # ---- arithmetic ----------------------------------------------------
    def test_add(self):
        a = self.write("a", [[1, 2], [3, 4]])
        b = self.write("b", [[5, 6], [7, 8]])
        self.assertMatrix(self.run_cli("add", a, b), [[6, 8], [10, 12]])

    def test_sub(self):
        a = self.write("a", [[5, 5], [5, 5]])
        b = self.write("b", [[1, 2], [3, 4]])
        self.assertMatrix(self.run_cli("sub", a, b), [[4, 3], [2, 1]])

    def test_mul_rectangular(self):
        a = self.write("a", [[1, 2, 3], [4, 5, 6]])          # 2x3
        b = self.write("b", [[7, 8], [9, 10], [11, 12]])     # 3x2
        self.assertMatrix(self.run_cli("mul", a, b), [[58, 64], [139, 154]])

    def test_mul_identity(self):
        a = self.write("a", [[2, -1], [0.5, 3]])
        i = self.write("i", [[1, 0], [0, 1]])
        self.assertMatrix(self.run_cli("mul", a, i), [[2, -1], [0.5, 3]])

    def test_transpose(self):
        a = self.write("a", [[1, 2, 3], [4, 5, 6]])
        self.assertMatrix(self.run_cli("transpose", a), [[1, 4], [2, 5], [3, 6]])

    # ---- determinant ---------------------------------------------------
    def det(self, rows):
        r = self.run_cli("det", self.write("m", rows))
        self.assertEqual(r.returncode, 0, msg=r.stderr)
        return r.stdout.strip()

    def test_det_1x1(self):
        self.assertAlmostEqual(float(self.det([[7]])), 7)

    def test_det_2x2(self):
        self.assertAlmostEqual(float(self.det([[4, 6], [3, 8]])), 14)

    def test_det_3x3(self):
        self.assertAlmostEqual(float(self.det([[2, -3, 1], [2, 0, -1], [1, 4, 5]])), 49)

    def test_det_needs_row_swap(self):
        # zero in the top-left corner forces a pivot swap (sign flip)
        self.assertAlmostEqual(float(self.det([[0, 1], [1, 0]])), -1)

    def test_det_singular_prints_plain_zero(self):
        # rows are linearly dependent, so the answer is exactly 0 (not "-0")
        self.assertEqual(self.det([[1, 2, 3], [4, 5, 6], [7, 8, 9]]), "0")

    # ---- error handling ------------------------------------------------
    def assertFails(self, result, message):
        self.assertEqual(result.returncode, 2)
        self.assertIn(message, result.stderr)
        self.assertEqual(result.stdout, "")

    def test_add_dimension_mismatch(self):
        a = self.write("a", [[1, 2]])
        b = self.write("b", [[1], [2]])
        self.assertFails(self.run_cli("add", a, b), "dimensions do not match")

    def test_mul_dimension_mismatch(self):
        a = self.write("a", [[1, 2]])
        b = self.write("b", [[1, 2]])
        self.assertFails(self.run_cli("mul", a, b), "dimensions do not match")

    def test_det_not_square(self):
        self.assertFails(self.run_cli("det", self.write("a", [[1, 2, 3]])), "must be square")

    def test_missing_file(self):
        self.assertFails(self.run_cli("det", "/no/such/file.txt"), "cannot open file")

    def test_too_few_values(self):
        bad = self.write_raw("bad", "2 2\n1 2 3\n")
        self.assertFails(self.run_cli("det", bad), "not a valid matrix")

    def test_non_numeric_value(self):
        bad = self.write_raw("bad", "2 2\n1 x\n3 4\n")
        self.assertFails(self.run_cli("det", bad), "not a valid matrix")

    def test_negative_size_rejected(self):
        bad = self.write_raw("bad", "-2 2\n1 2\n")
        self.assertFails(self.run_cli("det", bad), "out of range")

    def test_huge_size_rejected(self):
        bad = self.write_raw("bad", "100000 100000\n1\n")
        self.assertFails(self.run_cli("det", bad), "out of range")

    def test_unknown_command_shows_usage(self):
        r = self.run_cli("inverse", self.write("a", [[1]]))
        self.assertEqual(r.returncode, 1)
        self.assertIn("Usage", r.stderr)


if __name__ == "__main__":
    unittest.main()
