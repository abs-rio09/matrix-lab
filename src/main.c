/*
 * main.c - Command-line front end for matrix-lab.
 *
 * Usage:
 *   matrix add A.txt B.txt
 *   matrix sub A.txt B.txt
 *   matrix mul A.txt B.txt
 *   matrix transpose A.txt
 *   matrix det A.txt
 *
 * Exit codes: 0 success, 1 usage error, 2 operation/input error.
 */
#include "matrix.h"

#include <stdio.h>
#include <string.h>

#define MATLAB_VERSION "1.0.0"

static void usage(FILE *fp)
{
    fprintf(fp,
            "matrix-lab %s - bounds-checked matrix calculator\n\n"
            "Usage:\n"
            "  matrix add A.txt B.txt     A + B\n"
            "  matrix sub A.txt B.txt     A - B\n"
            "  matrix mul A.txt B.txt     A x B\n"
            "  matrix transpose A.txt     A^T\n"
            "  matrix det A.txt           determinant of A\n\n"
            "File format: first line 'rows cols', then the values row by row.\n",
            MATLAB_VERSION);
}

/* Load a matrix file and print a clear error naming the file if it fails */
static Matrix *load(const char *path)
{
    Matrix *m = NULL;
    MatStatus st = mat_read_file(path, &m);
    if (st != MAT_OK) {
        fprintf(stderr, "error: %s: %s\n", path, mat_strerror(st));
        return NULL;
    }
    return m;
}

int main(int argc, char **argv)
{
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage(stdout);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("matrix-lab %s\n", MATLAB_VERSION);
        return 0;
    }
    if (argc < 3) {
        usage(stderr);
        return 1;
    }

    const char *op = argv[1];
    int binary = strcmp(op, "add") == 0 || strcmp(op, "sub") == 0 || strcmp(op, "mul") == 0;
    int unary  = strcmp(op, "transpose") == 0 || strcmp(op, "det") == 0;

    if ((!binary && !unary) || (binary && argc != 4) || (unary && argc != 3)) {
        usage(stderr);
        return 1;
    }

    int exit_code = 2;
    Matrix *a = load(argv[2]);
    Matrix *b = NULL;
    Matrix *res = NULL;
    MatStatus st = MAT_OK;

    if (a == NULL) goto cleanup;
    if (binary && (b = load(argv[3])) == NULL) goto cleanup;

    if (strcmp(op, "add") == 0)            st = mat_add(a, b, &res);
    else if (strcmp(op, "sub") == 0)       st = mat_sub(a, b, &res);
    else if (strcmp(op, "mul") == 0)       st = mat_mul(a, b, &res);
    else if (strcmp(op, "transpose") == 0) st = mat_transpose(a, &res);
    else {
        double det = 0.0;
        st = mat_det(a, &det);
        if (st == MAT_OK) printf("%.6g\n", det);
    }

    if (st != MAT_OK) {
        fprintf(stderr, "error: %s: %s\n", op, mat_strerror(st));
        goto cleanup;
    }

    if (res != NULL) mat_print(stdout, res);
    exit_code = 0;

cleanup: /* single exit path so every allocation is freed exactly once */
    mat_free(res);
    mat_free(b);
    mat_free(a);
    return exit_code;
}
