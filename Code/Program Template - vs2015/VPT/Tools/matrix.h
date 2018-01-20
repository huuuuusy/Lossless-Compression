#ifndef MATRIX_H
#define MATRIX_H

#include "complex.h"
#include "fixedpoint.h"

template <class TYPE>
class Matrix {
//	friend const Matrix<TYPE> operator*(const Matrix<TYPE>& M1, const Matrix<TYPE>& M2) ;
private:
	int nRow ;
	int nCol ;
	TYPE *data ;

public:
	Matrix() ;
	Matrix(int r, int c, TYPE v) ; // Create a Matrix with all entries being set to value v
	Matrix(int r, int c, TYPE *v) ;// Create a Matrix with entries being set to those pointed by *v
	Matrix(int r, int c) ;		   // Create a Matrix with dimension r x c (no set zeros are performed)
	Matrix(const Matrix<TYPE> &M) ;// Clone a Matrix M
	~Matrix() ;
	Matrix<TYPE>& SetZero(void) ;
	inline void SetNaM() { Resize(0, 0) ; } ;	// Mark a Matrix to be "Not A Matrix"
	int Resize(int r, int c) ;	// Resize the matrix, return 1 on success
	inline int Resize(int c) { return Resize(1, c) ; } ; // Resize it to be a Row Vector
	inline int DimRow(void) { return nRow ; } ;
	inline int DimCol(void ) { return nCol ; } ;
	inline int DimRow(int &r) { r = nRow; return nRow ; } ;
	inline int DimCol(int &c) { c = nCol; return nCol ; } ;
	inline int Dim(int &r, int &c) { r = nRow; c = nCol; return nRow * nCol ; } ;
	inline int Size(void) { return nRow * nCol ; } ;
	Matrix<TYPE> Transpose(void) ;
	Matrix<TYPE>& Transpose(Matrix<TYPE> &M) ; // Put the transpose result directly to M, avoiding memory copy.
	int Identity(int size) ;
	Matrix<TYPE> Inverse(void) ;
	TYPE Inverse(Matrix<TYPE>& INV) ;
	TYPE Det(void) ;
	Matrix<TYPE> PseudoInverse(void) ;
	Matrix<TYPE>& PseudoInverse(Matrix<TYPE>& PINV) ;
	int isNaM() ;

	Matrix<TYPE>& operator=(Matrix<TYPE> M) ;
	Matrix<TYPE>& operator=(TYPE v) ;
	Matrix<TYPE> operator*(Matrix<TYPE> M) ;
	Matrix<TYPE> operator*(TYPE v) ;
	Matrix<TYPE> operator+(Matrix<TYPE> M) ;
	Matrix<TYPE> operator+(TYPE v) ;
	Matrix<TYPE> operator-(Matrix<TYPE> M) ;
	Matrix<TYPE> operator-(TYPE v) ;
	Matrix<TYPE> operator-(void) ;
	Matrix<TYPE> operator/(TYPE v) ;

	inline TYPE& operator[](int s) { return data[s] ; } ;
	inline TYPE& operator()(int s) { return data[s] ; } ;
	inline TYPE& operator()(int r, int c) {return data[r * nCol + c] ; } ;
};

template <class TYPE>
Matrix<TYPE> operator+(TYPE v, Matrix<TYPE> M) ;

template <class TYPE>
Matrix<TYPE> operator-(TYPE v, Matrix<TYPE> M) ;

template <class TYPE>
Matrix<TYPE> operator*(TYPE v, Matrix<TYPE> M) ;

template <class TYPE>
Matrix<TYPE> operator/(TYPE v, Matrix<TYPE> M) ;

#define PreDefine_Matrix(TYPE)\
	template class Matrix<TYPE> ;\
	template class Matrix< Complex<TYPE> > ;\
	template Matrix<TYPE> operator+(TYPE v, Matrix<TYPE> M) ;\
	template Matrix<TYPE> operator-(TYPE v, Matrix<TYPE> M) ;\
	template Matrix<TYPE> operator*(TYPE v, Matrix<TYPE> M) ;\
	template Matrix<TYPE> operator/(TYPE v, Matrix<TYPE> M) ;
#endif