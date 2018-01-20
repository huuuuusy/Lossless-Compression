#ifndef COMPLEX_H
#define COMPLEX_H

template <class TYPE>
class Complex {
//	friend const Matrix<TYPE> operator*(const Matrix<TYPE>& M1, const Matrix<TYPE>& M2) ;
private:
	TYPE real ;
	TYPE imag ;

public:
	Complex() { real = imag = (TYPE) 0 ; } ;
	Complex(TYPE r, TYPE i) { real = (TYPE) r ; imag = (TYPE) i ; } ;
	Complex(TYPE r) { real = (TYPE) r ; imag = (TYPE) 0 ; } ;
	inline TYPE& Real(void) {
		return real ;
	} ;
	inline TYPE& Imag(void) {
		return imag ;
	} ;
	inline TYPE Arg(void) {
		return atan(imag/real) ;
	}
	inline Complex<TYPE> Exp(TYPE theta) {
		real = cos(theta) ;
		imag = sin(theta) ;
		return *this ;
	}
	inline TYPE Mod(void) {
		return (TYPE) sqrt((double) (real * real + imag * imag)) ;
	} ;
	inline Complex<TYPE> Conjugate(void) {
		Complex<TYPE> v ;
		v.real = real ;
		v.imag = -imag ;
		return v ;
	} ;
	inline Complex<TYPE> operator+(Complex<TYPE> C) {
		Complex<TYPE> v ;
		v.real = real + C.real ;
		v.imag = imag + C.imag ;
		return v ;
	} ;
	inline Complex<TYPE> operator+=(Complex<TYPE> C) {
		real += C.real ;
		imag += C.imag ;
		return *this ;
	} ;
	inline Complex<TYPE> operator-(Complex<TYPE> C) {
		Complex<TYPE> v ;
		v.real = real - C.real ;
		v.imag = imag - C.imag ;
		return v ;
	} ;
	inline Complex<TYPE> operator-=(Complex<TYPE> C) {
		real -= C.real ;
		imag -= C.imag ;
		return *this ;
	} ;
	inline Complex<TYPE> operator-(void) {
		Complex<TYPE> v ;
		v.real = -real ;
		v.imag = -imag ;
		return v ;
	} ;
	inline Complex<TYPE> operator*(Complex<TYPE> C) {
		Complex<TYPE> v ;
		v.real = real * C.real - imag * C.imag ;
		v.imag = real * C.imag + imag * C.real ;
		return v ;
	} ;
	inline Complex<TYPE> operator/(Complex<TYPE> C) {
		Complex<TYPE> v ;
		TYPE norm ;

		norm = C.real * C.real + C.imag * C.imag ;
		v.real = (real * C.real + imag * C.imag) / norm ;
		v.imag = (imag * C.real - real * C.imag) / norm ;
		return v ;
	} ;
	inline Complex<TYPE>& operator=(double c) {
		real = (TYPE) c ;
		imag = (TYPE) 0 ;
		return *this ;
	}
	inline Complex<TYPE>& operator=(float c) {
		real = (TYPE) c ;
		imag = (TYPE) 0 ;
		return *this ;
	}
	inline Complex<TYPE>& operator=(int c) {
		real = (TYPE) c ;
		imag = (TYPE) 0 ;
		return *this ;
	}
	inline Complex<TYPE>& operator=(long c) {
		real = (TYPE) c ;
		imag = (TYPE) 0 ;
		return *this ;
	}
	inline operator double() {
		return (double) real ;
	}
	inline operator float() {
		return (float) real ;
	}
	inline operator int() {
		return (int) real ;
	}
	inline operator long() {
		return (long) real ;
	}
	inline int operator==(Complex<TYPE> C) {
		return ((real == C.real) && (imag == C.imag)) ;
	}
	inline int operator!=(Complex<TYPE> C) {
		return ((real != C.real) || (imag != C.imag)) ;
	}
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator+(Complex<TYPE> C, E_TYPE c) {
	Complex<TYPE> v ;
	v.Real() = C.Real() + (TYPE) c ;
	v.Imag()  = C.Imag() ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator+(E_TYPE c, Complex<TYPE> C) {
	Complex<TYPE> v ;
	v.Real() = C.Real() + (TYPE) c ;
	v.Imag()  = C.Imag() ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator+=(Complex<TYPE> C, E_TYPE c) {
	C.Real() = C.Real() + (TYPE) c ;
	return C ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator-(Complex<TYPE> C, E_TYPE c) {
	Complex<TYPE> v ;
	v.Real() = C.Real() - (TYPE) c ;
	v.Imag()  = C.Imag() ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator-(E_TYPE c, Complex<TYPE> C) {
	Complex<TYPE> v ;
	v.Real() = (TYPE) c - C.Real() ;
	v.Imag() = -C.Imag() ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator-=(Complex<TYPE> C, E_TYPE c) {
	C.Real() = C.Real() - (TYPE) c ;
	return C ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator*(Complex<TYPE> C, E_TYPE c) {
	Complex<TYPE> v ;
	v.Real() = C.Real() * (TYPE) c ;
	v.Imag()  = C.Imag() * (TYPE) c ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator*(E_TYPE c, Complex<TYPE> C) {
	Complex<TYPE> v ;
	v.Real() = C.Real() * (TYPE) c ;
	v.Imag()  = C.Imag() * (TYPE) c ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator/(Complex<TYPE> C, E_TYPE c) {
	Complex<TYPE> v ;
	v.Real() = C.Real() / (TYPE) c ;
	v.Imag()  = C.Imag() / (TYPE) c ;
	return v ;
} ;

template <class TYPE, class E_TYPE>
Complex<TYPE> operator/(E_TYPE c, Complex<TYPE> C) {
	Complex<TYPE> cc((TYPE) c) ;

	return cc / C;
} ;

#define PreDefine_Complex(TYPE)\
	template class Complex<TYPE> ; \
	template Complex<TYPE> operator+(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator+(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator+(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator+(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator+=(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator+=(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator+=(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator+=(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator+(double c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator+(float c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator+(int c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator+(long c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator-(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator-(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator-(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator-(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator-=(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator-=(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator-=(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator-=(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator-(double c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator-(float c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator-(int c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator-(long c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator*(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator*(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator*(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator*(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator*(double c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator*(float c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator*(int c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator*(long c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator/(Complex<TYPE> C, double c) ; \
	template Complex<TYPE> operator/(Complex<TYPE> C, float c) ; \
	template Complex<TYPE> operator/(Complex<TYPE> C, int c) ; \
	template Complex<TYPE> operator/(Complex<TYPE> C, long c) ; \
	template Complex<TYPE> operator/(double c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator/(float c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator/(int c, Complex<TYPE> C) ; \
	template Complex<TYPE> operator/(long c, Complex<TYPE> C) ;
#endif