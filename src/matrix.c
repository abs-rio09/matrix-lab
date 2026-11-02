/*
 * matrix.c - Implementation of the matrix-lab library.
 */
#include "matrix.h"

#include <math.h>
#include <stdlib.h>

/* Convenience: address of element (r, c). Only used after bounds are known. */
#define AT(m, r, c) ((m)->data[(r) * (m)->cols + (c)])

Matrix *mat_create(size_t rows, size_t cols)
{
    if (rows == 0 || cols == 0 || rows > MAT_MAX_DIM || cols > MAT_MAX_DIM) {
        return NULL;
    }

    Matrix *m = malloc(sizeof *m);
    if (m == NULL) {
        return NULL;
    }

    /* calloc zero-fills and checks rows * cols for overflow internally */
    m->data = calloc(rows * cols, sizeof *m->data);
    if (m->data == NULL) {
        free(m);
        return NULL;
    }

    m->rows = rows;
    m->cols = cols;
    return m;
}

void mat_free(Matrix *m)
{
    if (m != NULL) {
        free(m->data);
        free(m);
    }
}

MatStatus mat_get(const Matrix *m, size_t r, size_t c, double *out)
{
    if (m == NULL || out == NULL) return MAT_ERR_NULL;
    if (r >= m->rows || c >= m->cols) return MAT_ERR_RANGE;
    *out = AT(m, r, c);
    return MAT_OK;
}

MatStatus mat_set(Matrix *m, size_t r, size_t c, double value)
{
    if (m == NULL) return MAT_ERR_NULL;
    if (r >= m->rows || c >= m->cols) return MAT_ERR_RANGE;
    AT(m, r, c) = value;
    return MAT_OK;
}

/* Shared body of add and sub: sign = +1 for add, -1 for sub */
static MatStatus elementwise(const Matrix *a, const Matrix *b, double sign, Matrix **out)
{
    if (a == NULL || b == NULL || out == NULL) return MAT_ERR_NULL;
    if (a->rows != b->rows || a->cols != b->cols) return MAT_ERR_DIM;

    Matrix *res = mat_create(a->rows, a->cols);
    if (res == NULL) return MAT_ERR_ALLOC;

    size_t n = a->rows * a->cols;
    for (size_t i = 0; i < n; i++) {
        res->data[i] = a->data[i] + sign * b->data[i];
    }

    *out = res;
    return MAT_OK;
}

MatStatus mat_add(const Matrix *a, const Matrix *b, Matrix **out)
{
    return elementwise(a, b, 1.0, out);
}

MatStatus mat_sub(const Matrix *a, const Matrix *b, Matrix **out)
{
    return elementwise(a, b, -1.0, out);
}

MatStatus mat_mul(const Matrix *a, const Matrix *b, Matrix **out)
{
    if (a == NULL || b == NULL || out == NULL) return MAT_ERR_NULL;
    /* (m x n) * (n x p) -> (m x p): inner dimensions must match */
    if (a->cols != b->rows) return MAT_ERR_DIM;

    Matrix *res = mat_create(a->rows, b->cols);
    if (res == NULL) return MAT_ERR_ALLOC;

    /* i-k-j loop order walks both a and b row-wise, which is cache friendly */
    for (size_t i = 0; i < a->rows; i++) {
        for (size_t k = 0; k < a->cols; k++) {
            double aik = AT(a, i, k);
            for (size_t j = 0; j < b->cols; j++) {
                AT(res, i, j) += aik * AT(b, k, j);
            }
        }
    }

    *out = res;
    return MAT_OK;
}

MatStatus mat_transpose(const Matrix *a, Matrix **out)
{
    if (a == NULL || out == NULL) return MAT_ERR_NULL;

    Matrix *res = mat_create(a->cols, a->rows);
    if (res == NULL) return MAT_ERR_ALLOC;

    for (size_t r = 0; r < a->rows; r++) {
        for (size_t c = 0; c < a->cols; c++) {
            AT(res, c, r) = AT(a, r, c);
        }
    }

    *out = res;
    return MAT_OK;
}

/*
 * Determinant by Gaussian elimination with partial pivoting: O(n^3).
 * Works on a private copy so the caller's matrix is never modified.
 *   1. For each column, pick the row with the largest absolute pivot
 *      (reduces rounding error) and swap it up; each swap flips the sign.
 *   2. Eliminate everything below the pivot.
 *   3. The determinant is the product of the diagonal, times the sign.
 */
MatStatus mat_det(const Matrix *a, double *out)
{
    if (a == NULL || out == NULL) return MAT_ERR_NULL;
    if (a->rows != a->cols) return MAT_ERR_NOT_SQUARE;

    size_t n = a->rows;
    Matrix *t = mat_create(n, n);
    if (t == NULL) return MAT_ERR_ALLOC;
    for (size_t i = 0; i < n * n; i++) t->data[i] = a->data[i];

    double det = 1.0;
    for (size_t col = 0; col < n; col++) {
        /* 1. partial pivoting */
        size_t pivot = col;
        for (size_t r = col + 1; r < n; r++) {
            if (fabs(AT(t, r, col)) > fabs(AT(t, pivot, col))) pivot = r;
        }
        if (fabs(AT(t, pivot, col)) < 1e-12) { /* column is all zero: singular */
            mat_free(t);
            *out = 0.0;
            return MAT_OK;
        }
        if (pivot != col) {
            for (size_t c = 0; c < n; c++) {
                double tmp = AT(t, col, c);
                AT(t, col, c) = AT(t, pivot, c);
                AT(t, pivot, c) = tmp;
            }
            det = -det;
        }

        /* 2. eliminate below the pivot */
        double p = AT(t, col, col);
        det *= p;
        for (size_t r = col + 1; r < n; r++) {
            double factor = AT(t, r, col) / p;
            for (size_t c = col; c < n; c++) {
                AT(t, r, c) -= factor * AT(t, col, c);
            }
        }
    }

    mat_free(t);
    *out = det;
    return MAT_OK;
}

/*
 * File format (whitespace separated):
 *     rows cols
 *     v11 v12 ... v1c
 *     ...
 *     vr1 vr2 ... vrc
 */
MatStatus mat_read_file(const char *path, Matrix **out)
{
    if (path == NULL || out == NULL) return MAT_ERR_NULL;

    FILE *fp = fopen(path, "r");
    if (fp == NULL) return MAT_ERR_IO;

    long rows = 0, cols = 0;
    if (fscanf(fp, "%ld %ld", &rows, &cols) != 2) {
        fclose(fp);
        return MAT_ERR_PARSE;
    }
    if (rows <= 0 || cols <= 0 || rows > MAT_MAX_DIM || cols > MAT_MAX_DIM) {
        fclose(fp);
        return MAT_ERR_RANGE;
    }

    Matrix *m = mat_create((size_t)rows, (size_t)cols);
    if (m == NULL) {
        fclose(fp);
        return MAT_ERR_ALLOC;
    }

    size_t n = m->rows * m->cols;
    for (size_t i = 0; i < n; i++) {
        if (fscanf(fp, "%lf", &m->data[i]) != 1) { /* too few or non-numeric values */
            mat_free(m);
            fclose(fp);
            return MAT_ERR_PARSE;
        }
    }

    fclose(fp);
    *out = m;
    return MAT_OK;
}

void mat_print(FILE *fp, const Matrix *m)
{
    if (fp == NULL || m == NULL) return;
    for (size_t r = 0; r < m->rows; r++) {
        for (size_t c = 0; c < m->cols; c++) {
            fprintf(fp, "%s%10.4g", c ? " " : "", AT(m, r, c));
        }
        fputc('\n', fp);
    }
}

const char *mat_strerror(MatStatus status)
{
    switch (status) {
    case MAT_OK:             return "success";
    case MAT_ERR_NULL:       return "missing argument";
    case MAT_ERR_ALLOC:      return "out of memory";
    case MAT_ERR_DIM:        return "matrix dimensions do not match for this operation";
    case MAT_ERR_NOT_SQUARE: return "matrix must be square";
    case MAT_ERR_RANGE:      return "size or index out of range";
    case MAT_ERR_IO:         return "cannot open file";
    case MAT_ERR_PARSE:      return "file is not a valid matrix";
    }
    return "unknown error";
}
