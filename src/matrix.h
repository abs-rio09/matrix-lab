/*
 * matrix.h - Small, bounds-checked matrix library in C.
 *
 * Every operation returns a MatStatus code instead of crashing, and every
 * matrix is heap-allocated with its dimensions stored alongside the data,
 * so callers can never index past the end of a row or column.
 */
#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdio.h>

/* Hard upper limit on rows/cols, protects against absurd allocations
 * coming from a corrupt or malicious input file. */
#define MAT_MAX_DIM 1000

typedef struct {
    size_t rows;
    size_t cols;
    double *data; /* row-major: element (r, c) lives at data[r * cols + c] */
} Matrix;

typedef enum {
    MAT_OK = 0,
    MAT_ERR_NULL,       /* a required pointer argument was NULL          */
    MAT_ERR_ALLOC,      /* malloc/calloc failed                          */
    MAT_ERR_DIM,        /* dimensions do not fit the operation           */
    MAT_ERR_NOT_SQUARE, /* operation needs a square matrix               */
    MAT_ERR_RANGE,      /* index or size outside the allowed range       */
    MAT_ERR_IO,         /* file could not be opened                      */
    MAT_ERR_PARSE       /* file content is not a valid matrix            */
} MatStatus;

/* Creation and cleanup */
Matrix   *mat_create(size_t rows, size_t cols);   /* zero-filled, NULL on failure */
void      mat_free(Matrix *m);                    /* safe to call with NULL       */

/* Checked element access */
MatStatus mat_get(const Matrix *m, size_t r, size_t c, double *out);
MatStatus mat_set(Matrix *m, size_t r, size_t c, double value);

/* Operations: on success *out points to a new matrix the caller must free */
MatStatus mat_add(const Matrix *a, const Matrix *b, Matrix **out);
MatStatus mat_sub(const Matrix *a, const Matrix *b, Matrix **out);
MatStatus mat_mul(const Matrix *a, const Matrix *b, Matrix **out);
MatStatus mat_transpose(const Matrix *a, Matrix **out);
MatStatus mat_det(const Matrix *a, double *out);

/* Input / output */
MatStatus mat_read_file(const char *path, Matrix **out);
void      mat_print(FILE *fp, const Matrix *m);

/* Human-readable message for a status code */
const char *mat_strerror(MatStatus status);

#endif /* MATRIX_H */
