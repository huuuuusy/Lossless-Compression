#include "StdAfx.h"
#include "matrix.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "complex.h"
#include "fixedpoint.h"

#define ABS(x) (( x >= 0) ? (x) : (-(x)))

inline double Modulus(double c) {
	return ABS(c) ;
}

inline float Modulus(float c) {
	return ABS(c) ;
}

inline int Modulus(int c) {
	return ABS(c) ;
}

inline long Modulus(long c) {
	return ABS(c) ;
}

inline long long Modulus(long long c) {
	return ABS(c) ;
}

template <class TYPE>
inline TYPE Modulus(Complex<TYPE> c) {
	return c.Mod() ;
}

#undef ABS

template <class TYPE>
Matrix<TYPE>::Matrix() {
	nRow = 0 ;
	nCol = 0 ;
	data = NULL ;
}

template <class TYPE>
Matrix<TYPE>::Matrix(int r, int c, TYPE v) {

	int i ;

	nRow = r ;
	nCol = c ;
	data = new TYPE[r * c] ;
	for(i = 0; i < nRow * nCol; i++)
		data[i] = (TYPE) v ;
}

template <class TYPE>
Matrix<TYPE>::Matrix(int r, int c, TYPE *v) {

	int i ;

	nRow = r ;
	nCol = c ;
	data = new TYPE[r * c] ;
	for(i = 0; i < nRow * nCol; i++)
		data[i] = (TYPE) v[i] ;
}

template <class TYPE>
Matrix<TYPE>::Matrix(int r, int c) {

	int i ;

	nRow = r ;
	nCol = c ;
	data = new TYPE[r * c] ;
	for(i = 0; i < nRow * nCol; i++)
		data[i] = (TYPE) 0 ;
}

template <class TYPE>
Matrix<TYPE>::Matrix(const Matrix<TYPE>& M) {
	nRow = M.nRow ;
	nCol = M.nCol ;
	data = new TYPE[nRow * nCol] ;
	memcpy(data, M.data, sizeof(TYPE) * nRow * nCol) ;
}

template <class TYPE>
Matrix<TYPE>::~Matrix() {
	if(data)
		delete [] data ;
	data = NULL ;
}

template <class TYPE>
Matrix<TYPE>& Matrix<TYPE>::SetZero(void) {

	int i ;

	if(nCol && nRow && data) {
		for(i = 0; i < Size() ; i++)
			data[i] = (TYPE) 0 ;
	}
	return *this ;
}

template <class TYPE>
int Matrix<TYPE>::Resize(int r, int c) {

	if(!((r * c == nRow * nCol) && data)) {
		if(data)
			delete [] data ;
		if(r * c != 0)
			data = new TYPE[r * c] ;
		else data = NULL ;
	}
	if(!data) {
		nRow = 0 ;
		nCol = 0 ; 
		return 0 ;
	} else {
		nRow = r ;
		nCol = c ;
	}

	return 1 ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::Transpose(void) {

	Matrix<TYPE> r ;
	int m, n ;

//	r = new Matrix<TYPE>(nCol, nRow) ;
	if(r.Resize(nCol, nRow)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[m + n * nRow] = data[n + m * nCol] ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE>& Matrix<TYPE>::Transpose(Matrix<TYPE>& M) {

	int m, n ;
	int r, c ;
	TYPE *src ;

	if(this == &M) {
		src = new TYPE[nRow * nCol] ;
		memcpy(src, data, sizeof(TYPE) * nRow * nCol) ;
	} else {
		src = data ;
	}
	r = nRow ;
	c = nCol ;
	if(M.Resize(c, r)) {
		for(m = 0; m < r; m++) {
			for(n = 0; n < c; n++) {
				M.data[m + n * r] = src[n + m * c] ;
			}
		}
	}
	if(this == &M) {
		delete [] src ;
	}
	return M ;
}

template <class TYPE>
int Matrix<TYPE>::Identity(int size) {
	
	int i ;

	if(Resize(size, size)) {
		SetZero() ;
		for(i = 0; i < size; i++) data[i + i * size] = (TYPE) 1 ;
		return 1 ;
	}
	return 0 ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::Inverse(void) {

	Matrix<TYPE> r, orig ;
	int k, i, j, m ;
	TYPE value, max ;

	if(nRow == nCol && data) {
		orig = *this ;
		if(r.Identity(nRow)) {
			for(k = 0; k < nRow; k++) {
				max = orig.data[k + k * nCol] + orig.data[k + k * nCol] ;
				m = k ;
				for(i = k+1; i < nRow; i++) {
					value = orig.data[k + k * nCol] + orig.data[k + i * nCol] ;
					if(Modulus(value) > Modulus(max)) {
						m = i ;
						max = value ;
					}
				}
				
				if(Modulus(max) != Modulus((TYPE) 0)) {
					for(j = 0; j < nCol; j++) {
						orig.data[j + k * nCol] = (orig.data[j + k * nCol] + orig.data[j + m * nCol]) / max ;
						r.data[j + k * nCol] = (r.data[j + k * nCol] + r.data[j + m * nCol]) / max ;
					}

					for(i = 0; i < nRow; i++) {
						if(i != k) {
							value = orig.data[k + i * nCol] ;
							for(j = 0; j < nCol; j++) {
								orig.data[j + i * nCol] = orig.data[j + i * nCol] - (orig.data[j + k * nCol] * value) ;
								r.data[j + i * nCol] = r.data[j + i * nCol] - (r.data[j + k * nCol] * value) ;
							}
						}
					}
				} else {
					r.SetNaM() ;
				}
			}
		}
	}
	return r ;
}

template <class TYPE>
TYPE Matrix<TYPE>::Inverse(Matrix<TYPE>& INV) {

	Matrix<TYPE> orig ;
	int k, i, j, m ;
	TYPE value, max, det ;

#define ABS(x) (((TYPE) x >= (TYPE) 0) ? ((TYPE) (x)) : ((TYPE) (-(x))))

	if(nRow == nCol && data) {
		orig = *this ;
		det = (TYPE) 1 ;
		if(INV.Identity(nRow)) {
			for(k = 0; k < nRow; k++) {
				max = orig.data[k + k * nCol] + orig.data[k + k * nCol] ;
				m = k ;
				for(i = k+1; i < nRow; i++) {
					value = orig.data[k + k * nCol] + orig.data[k + i * nCol] ;
					if(Modulus(value) > Modulus(max)) {
						m = i ;
						max = value ;
					}
				}
				
				if(Modulus((TYPE) max) != Modulus((TYPE) 0)) {
					for(j = 0; j < nCol; j++) {
						orig.data[j + k * nCol] = (orig.data[j + k * nCol] + orig.data[j + m * nCol]) / max ;
						INV.data[j + k * nCol] = (INV.data[j + k * nCol] + INV.data[j + m * nCol]) / max ;
					}

					det = det * max ;
					if(m == k) det = det / (TYPE) 2 ;

					for(i = 0; i < nRow; i++) {
						if(i != k) {
							value = orig.data[k + i * nCol] ;
							for(j = 0; j < nCol; j++) {
								orig.data[j + i * nCol] = orig.data[j + i * nCol] - (orig.data[j + k * nCol] * value) ;
								INV.data[j + i * nCol] = INV.data[j + i * nCol] - (INV.data[j + k * nCol] * value) ;
							}
						}
					}
				} else {
					INV.SetNaM() ;
					return (TYPE) 0 ;
				}
			}
		} else return (TYPE) 0 ;
	} else {
		INV.SetNaM() ;
		return (TYPE) 0 ;
	}
	return det ;
#undef ABS
}

template <class TYPE>
TYPE Matrix<TYPE>::Det(void) {

	Matrix<TYPE> r ;

	return Inverse(r) ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::PseudoInverse(void) {

	Matrix<TYPE> src, srcT ;

	src = *this ;
	src.Transpose(srcT) ;
	return (srcT * src).Inverse() * srcT ;
}

template <class TYPE>
Matrix<TYPE>& Matrix<TYPE>::PseudoInverse(Matrix<TYPE>& PINV) {

	Matrix<TYPE> src, srcT ;

	src = *this ;
	src.Transpose(srcT) ;
	PINV = ((srcT * src).Inverse() * srcT) ;
	return PINV ;
}

template <class TYPE>
int Matrix<TYPE>::isNaM(void) {

	if(data && nRow && nCol) return 0 ;
	return 1 ;
}

template <class TYPE>
Matrix<TYPE>& Matrix<TYPE>::operator=(Matrix<TYPE> M)
{
	if (this!=&M && Resize(M.nRow, M.nCol))
	{
		memcpy(data, M.data, sizeof(TYPE)*M.nRow*M.nCol);
	}

	return *this;
}

template <class TYPE>
Matrix<TYPE>& Matrix<TYPE>::operator=(TYPE v) {

	int i ;

	if(data && nRow && nCol) {
		for(i = 0; i < Size(); i++) data[i] = (TYPE) v ;
	}
	return *this ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator*(Matrix<TYPE> M)
{
	Matrix<TYPE> r ;
	int m, n, k ;

	if(nCol == M.nRow && M.data && data && r.Resize(nRow, M.nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < M.nCol; n++) {
				r.data[n + m * M.nCol] = (TYPE) 0 ;
				for(k = 0; k < nCol; k++) {
					r.data[n + m * M.nCol] = r.data[n + m * M.nCol] + data[k + m * nCol] * M.data[n + k * M.nCol] ;
				}
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator*(TYPE v)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] * v ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> operator*(TYPE v, Matrix<TYPE> M)
{
	return M * v ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator+(Matrix<TYPE> M)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(nCol == M.nCol && nRow == M.nRow && data && M.data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] + M.data[n + m * nCol] ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator+(TYPE v)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] + v ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> operator+(TYPE v, Matrix<TYPE> M)
{
	return M + v ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator-(Matrix<TYPE> M)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(nCol == M.nCol && nRow == M.nRow && data && M.data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] - M.data[n + m * nCol] ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator-(TYPE v)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] - v ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> operator-(TYPE v, Matrix<TYPE> M)
{
	return -M + v ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator-(void)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = -data[n + m * nCol] ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> Matrix<TYPE>::operator/(TYPE v)
{
	Matrix<TYPE> r ;
	int m, n ;

	if(data && r.Resize(nRow, nCol)) {
		for(m = 0; m < nRow; m++) {
			for(n = 0; n < nCol; n++) {
				r.data[n + m * nCol] = data[n + m * nCol] / v ;
			}
		}
	}
	return r ;
}

template <class TYPE>
Matrix<TYPE> operator/(TYPE v, Matrix<TYPE> M)
{
	return M.Inverse() * v ;
}

PreDefine_Matrix(double) ;
PreDefine_Matrix(float) ;
PreDefine_Matrix(int) ;
PreDefine_Matrix(long) ;
PreDefine_Matrix(long long) ;

template class Matrix < FixedPoint<long long> > ;
//template class Matrix<int>;