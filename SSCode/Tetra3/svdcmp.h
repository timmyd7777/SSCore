#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

double *dvector ( size_t n );
double **dmatrix ( size_t m, size_t n );
void free_dvector ( double *v );
void free_dmatrix ( double **m );
void svdcmp( double **A, unsigned int M, unsigned int N, double *W, double **V );
double determinant ( double **a, int n );

#ifdef __cplusplus
}
#endif
