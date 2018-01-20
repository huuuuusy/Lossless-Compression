#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#define DEFAULT_NUMQ ((8 * sizeof(TYPE)) / 4)

template <class TYPE>
class FixedPoint {
//	friend const Matrix<TYPE> operator*(const Matrix<TYPE>& M1, const Matrix<TYPE>& M2) ;
private:
	TYPE value ;
	unsigned short numQ ;

public:
	FixedPoint() { value = (TYPE) 0L ; numQ = DEFAULT_NUMQ ; } ;
	FixedPoint(TYPE v, unsigned short n) { value = v << n ; numQ = n ; } ;
	FixedPoint(TYPE v) { numQ = DEFAULT_NUMQ ; value = (TYPE) v << numQ ; } ;

	inline TYPE Mod(void) {
		double mod ;
		FixedPoint<TYPE> r ;

		mod = sqrt((double) (*this)) ;
		r = mod ;

		return r.value ;
	} ;

	inline TYPE& Value(void) {
		return value ;
	} ;
	inline unsigned short& NumQ(void) {
		return numQ ;
	} ;
	inline FixedPoint<TYPE> operator+(FixedPoint<TYPE> F) {
		FixedPoint<TYPE> r ;

		r.numQ = (numQ <= F.numQ) ? numQ : F.numQ ;
		r.value = (F.value >> (F.numQ - r.numQ)) + (value >> (numQ - r.numQ)) ;
		return r ;
	} ;
	inline FixedPoint<TYPE> operator-(FixedPoint<TYPE> F) {
		FixedPoint<TYPE> r ;

		r.numQ = (numQ <= F.numQ) ? numQ : F.numQ ;
		r.value = (value >> (numQ - r.numQ)) - (F.value >> (F.numQ - r.numQ)) ;
		return r ;
	} ;
	inline FixedPoint<TYPE> operator-(void) {
		FixedPoint<TYPE> r ;

		r.numQ = numQ ;
		r.value = -value ;
		return r ;
	} ;
	inline FixedPoint<TYPE> operator*(FixedPoint<TYPE> F) {
		FixedPoint<TYPE> r ;

		r.numQ = (numQ <= F.numQ) ? numQ : F.numQ ;
		r.value = ((F.value >> (F.numQ - r.numQ)) * (value >> (numQ - r.numQ))) >> r.numQ ;
		return r ;
	} ;
	inline FixedPoint<TYPE> operator/(FixedPoint<TYPE> F) {
		FixedPoint<TYPE> r ;

		r.numQ = (numQ <= F.numQ) ? numQ : F.numQ ;
		r.value = ((value >> (numQ - r.numQ)) << r.numQ) / (F.value >> (F.numQ - r.numQ)) ;
		return r ;
	} ;
	inline FixedPoint<TYPE>& operator=(double c) {

		value = (TYPE) (c * ((double) (1L << numQ))) ;
		return *this ;
	}
	inline FixedPoint<TYPE>& operator=(float c) {

		value = (TYPE) (c * ((float) (1L << numQ))) ;
		return *this ;
	}
	inline FixedPoint<TYPE>& operator=(int c) {

		value = (TYPE) (c * ((int) (1L << numQ))) ;
		return *this ;
	}
	inline FixedPoint<TYPE>& operator=(long c) {

		value = (TYPE) (c * ((long) (1L << numQ))) ;
		return *this ;
	}
	inline FixedPoint<TYPE>& operator=(long long c) {

		value = (TYPE) (c * ((long long) (1L << numQ))) ;
		return *this ;
	}
	inline operator double() {
		return (double) (((double) (value)) / ((double) (1L << numQ))) ;
	}
	inline operator float() {
		return (float) (((float) (value)) / ((float) (1L << numQ))) ;
	}
	inline operator int() {
		return (int) ((value >= 0) ? (value >> numQ) : -((-value) >> numQ)) ;
	}
	inline operator long() {
		return (long) ((value >= 0) ? (value >> numQ) : -((-value) >> numQ)) ;
	}
	inline operator long long() {
		return (long long) ((value >= 0) ? (value >> numQ) : -((-value) >> numQ)) ;
	}
	inline int operator==(FixedPoint<TYPE>& F) {
		int nq ;

		nq = (numQ <= F.numQ) ? numQ : F.numQ ;
		return (value >> (numQ - nq)) == (F.value >> (F.numQ - nq)) ;
	}
	inline int operator!=(FixedPoint<TYPE>& F) {
		int nq ;

		nq = (numQ <= F.numQ) ? numQ : F.numQ ;
		return (value >> (numQ - nq)) != (F.value >> (F.numQ - nq)) ;
	}
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator+(FixedPoint<TYPE> C, E_TYPE c) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C + v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator+(E_TYPE c, FixedPoint<TYPE> C) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C + v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator-(FixedPoint<TYPE> C, E_TYPE c) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C - v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator-(E_TYPE c, FixedPoint<TYPE> C) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return v - C ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator*(FixedPoint<TYPE> C, E_TYPE c) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C * v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator*(E_TYPE c, FixedPoint<TYPE> C) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C * v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator/(FixedPoint<TYPE> C, E_TYPE c) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return C / v ;
} ;

template <class TYPE, class E_TYPE>
FixedPoint<TYPE> operator/(E_TYPE c, FixedPoint<TYPE> C) {

	FixedPoint<TYPE> v ;

	v = (E_TYPE) c ;
	return v / C ;
} ;

template <class TYPE>
TYPE Modulus(FixedPoint<TYPE> F) {
	return F.Mod() ;
}

template int Modulus(FixedPoint<int>) ;
template long Modulus(FixedPoint<long>) ;
template long long Modulus(FixedPoint<long long>) ;

#define PreDefine_FixedPoint(TYPE)\
	template class FixedPoint<TYPE> ; \
	template FixedPoint<TYPE> operator+(FixedPoint<TYPE> C, double c) ; \
	template FixedPoint<TYPE> operator+(FixedPoint<TYPE> C, float c) ; \
	template FixedPoint<TYPE> operator+(FixedPoint<TYPE> C, int c) ; \
	template FixedPoint<TYPE> operator+(FixedPoint<TYPE> C, long c) ; \
	template FixedPoint<TYPE> operator+(double c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator+(float c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator+(int c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator+(long c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator-(FixedPoint<TYPE> C, double c) ; \
	template FixedPoint<TYPE> operator-(FixedPoint<TYPE> C, float c) ; \
	template FixedPoint<TYPE> operator-(FixedPoint<TYPE> C, int c) ; \
	template FixedPoint<TYPE> operator-(FixedPoint<TYPE> C, long c) ; \
	template FixedPoint<TYPE> operator-(double c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator-(float c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator-(int c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator-(long c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator*(FixedPoint<TYPE> C, double c) ; \
	template FixedPoint<TYPE> operator*(FixedPoint<TYPE> C, float c) ; \
	template FixedPoint<TYPE> operator*(FixedPoint<TYPE> C, int c) ; \
	template FixedPoint<TYPE> operator*(FixedPoint<TYPE> C, long c) ; \
	template FixedPoint<TYPE> operator*(double c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator*(float c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator*(int c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator*(long c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator/(FixedPoint<TYPE> C, double c) ; \
	template FixedPoint<TYPE> operator/(FixedPoint<TYPE> C, float c) ; \
	template FixedPoint<TYPE> operator/(FixedPoint<TYPE> C, int c) ; \
	template FixedPoint<TYPE> operator/(FixedPoint<TYPE> C, long c) ; \
	template FixedPoint<TYPE> operator/(double c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator/(float c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator/(int c, FixedPoint<TYPE> C) ; \
	template FixedPoint<TYPE> operator/(long c, FixedPoint<TYPE> C) ;
#endif