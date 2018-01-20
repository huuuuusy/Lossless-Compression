#include "StdAfx.h"
#include "windows.h"
#include "./ImageLoader.h"
#include "./AviLoader.h"
#include "./VideoCapture.h"
#include "./ImgParam.h"
#include "Tools/ImageConvert.h"
#include "Tools/ImageFilter.h"

#ifdef H264_FILE_SUPPORT
#include "decoder.h"
#endif

DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

char CImageLoader::pbm_getc(FILE *fp) {

	char ch;

	ch = getc(fp);

	if (ch=='#')
	{
		do {
			ch = getc(fp);
		} while (ch!='\n' && ch!='\r');
	}

	return ch;
}

int CImageLoader::pbm_getint(FILE *fp) {

	char ch;
	int i;

	do {
		ch = pbm_getc(fp);
	} while (ch==' ' || ch=='\t' || ch=='\n' || ch=='\r');

	i = 0;
	do {
		i = i*10 + ch-'0';
		ch = pbm_getc(fp);
	} while (ch>='0' && ch<='9');

	return i;
}

unsigned char *CImageLoader::pbm_read(FILE *fp) {

	char p6[2] ;
	int size ;

	p6[0] = getc(fp); p6[1] = getc(fp); // magic number (P6)
	if(p6[0] != 'P' || p6[1] != '6') return NULL ;
	width = pbm_getint(fp); height = pbm_getint(fp); pbm_getint(fp); // width height maxcolors
	size = width * height * 3 ;

	if(bits) delete [] bits ;
	bits = new unsigned char[size] ;

	if(bits) {
		int i, j ;
		int temp ;

		fread(bits, 1, 3 * width * height, fp) ;
		/*
		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				bits[(j * width + i) * 3 + 2] = getc(fp) ;
				bits[(j * width + i) * 3 + 1] = getc(fp) ;
				bits[(j * width + i) * 3 + 0] = getc(fp) ;
			}
		}
		*/
		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				temp = bits[(j * width + i) * 3 + 2] ;
				bits[(j * width + i) * 3 + 2] = bits[(j * width + i) * 3 + 0] ;
				bits[(j * width + i) * 3 + 0] = temp ;
			}
		}
	}

	return bits ;
}

CImageLoader::CImageLoader(void)
{
	width = height = 0 ;
	startIndex = endIndex = index = 0 ;
	type = IMG_TYPE_UDF ;
	bits = NULL ;
	fmtIndex = 0 ;
	fmtCount = 0 ;
	FMT = NULL ;
#ifdef H264_FILE_SUPPORT
	pH264File = NULL ;
#endif
}

CImageLoader::~CImageLoader(void)
{
	if(bits) delete [] bits ;
	if(FMT) delete [] FMT ;

#ifdef H264_FILE_SUPPORT
	if(pH264File) {
		H264FileClose(pH264File) ;
		pH264File = NULL ;
	}
#endif
}

HRESULT CImageLoader::Save(const TCHAR *path) {

	CImage image ;
	unsigned char *dest, *src ;
	int j, d_pitch ;

	image.Create(width, height, 24, 0) ;
	d_pitch = image.GetPitch() ;
	if(d_pitch < 0) d_pitch = -d_pitch ;
	dest = (unsigned char *) image.GetBits() - d_pitch * (height - 1);
	src = bits + (height - 1) * width * 3 ;

	for(j = 0; j < height; j++) {
		memcpy(dest, src, width * 3) ;
		dest += d_pitch ;
		src -= width * 3 ;
	}

	return image.Save(path, ImageFormatBMP) ;
}

HRESULT CImageLoader::CapInit(HWND hwnd) {

	unsigned char *cap_ptr ;
	if(!videoCapture.init(hwnd)) return E_NOTIMPL ;

	if(bits) delete [] bits ;
	width = videoCapture.width ;
	height = videoCapture.height ;
	bits = new unsigned char [width * height * 3] ;
	if(!bits) return E_ABORT ;
	startIndex = endIndex = 0 ;

	do {
		cap_ptr = videoCapture.getFrame() ;
	} while(!cap_ptr) ;
	memcpy(bits, cap_ptr, videoCapture.width * videoCapture.height * 3) ;
	return S_OK ;
}

HRESULT CImageLoader::Load(const TCHAR *path, BOOL init) {

	HRESULT hresult ;
	TCHAR ext[16] ;

	const TCHAR *dotPtr, *ndotPtr ;

	dotPtr = ndotPtr = _tcschr(path, '.') ;
	while(ndotPtr) {
		dotPtr = ndotPtr ;
		ndotPtr = _tcschr(ndotPtr + 1, '.') ;
	}

	if(dotPtr) {
		dotPtr++ ;
		_tcsncpy_s(ext, dotPtr, 15) ;
	} else {
		ext[0] = 0 ;
	}

	if(!_tcsicmp(ext, _T("PPM")) || type == IMG_TYPE_PPM) {
		FILE *fp ;

		type = IMG_TYPE_PPM ;

		if(_tfopen_s(&fp, path, _T("rb"))) return E_ABORT;
		bits = pbm_read(fp) ;

		return S_OK ;
	} else if(!_tcsicmp(ext, _T("YUV")) || (type != IMG_TYPE_UDF && type != IMG_TYPE_Y_U_V)) {
		CImgParam ImgParam ;
		CImageConvert ImgConvert ;
		unsigned char *yuv, *y, *uv ;
		FILE *fp ;

		if(init) {
			ImgParam.DoModal() ;
			width = ImgParam.width ;
			height = ImgParam.height ;
			type = ImgParam.format ;
		}

		if(_tfopen_s(&fp, path, _T("rb"))) return E_ABORT ;
		if(bits) delete [] bits ;

		if(type == IMG_TYPE_420_V) {
			long loc ;
			loc = ftell(fp) ;
			if(fseek(fp, index * width * height * 3 / 2, SEEK_SET)) {
				fseek(fp, loc, SEEK_SET) ;
				return E_ABORT ;
			}
		}
		if(type == IMG_TYPE_422_V || type == IMG_TYPE_VYUY_V) {
			long loc ;
			loc = ftell(fp) ;
			if(fseek(fp, index * width * height * 2, SEEK_SET)) {
				fseek(fp, loc, SEEK_SET) ;
				return E_ABORT ;
			}
		}

		if(type == IMG_TYPE_420 || type == IMG_TYPE_420_V) {
			bits = new unsigned char [width * height * 3] ;
			yuv = new unsigned char [ImgConvert.SizeYUV420(width, height)] ;

			fread(yuv, ImgConvert.SizeYUV420(width, height), 1, fp) ;
			ImgConvert.YUV420_to_RGB24(yuv, bits, width, height) ;
			delete [] yuv ;
			fclose(fp) ;
			return S_OK ;
		}

		if(type == IMG_TYPE_VYUY || type == IMG_TYPE_VYUY_V) {

			bits = new unsigned char [width * height * 3] ;
			yuv = new unsigned char [ImgConvert.SizeYUYV(width, height)] ;

			fread(yuv, ImgConvert.SizeYUYV(width, height), 1, fp) ;
			ImgConvert.VYUY_to_RGB24(yuv, bits, width, height, 1, 0, 2) ;
			delete [] yuv ;
			fclose(fp) ;
			return S_OK ;
		}

		if(type ==  IMG_TYPE_422 || type == IMG_TYPE_422_V) {

			int i/*, j*/ ;

			bits = new unsigned char [width * height * 3] ;
			y = new unsigned char [width * height] ;
			uv = new unsigned char [width * height] ;
			yuv = new unsigned char [ImgConvert.SizeYUYV(width, height)] ;

			fread(y, 1, width * height, fp) ;
			fread(uv, 1, width * height, fp) ;
			for(i = 0; i < width * height; i++) {
				yuv[2 * i + 1] = y[i] ;
			}

			for(i = 0; i < width * height / 2; i++) {
				yuv[4 * i + 0] = uv[i] ;
			}
		
			for(i = 0; i < width * height / 2; i++) {
				yuv[4 * i + 2] = uv[i + width * height / 2] ;
			}
/*
			for(j = 0; j < height; j+=2) {
				for(i = 0; i < width; i++) {
					yuv[2 * i + 1 + (j) * width * 2] = yuv[4 * (i/2) + 2 + j * width * 2];
					yuv[2 * i + 1 + (j + 1) * width * 2] = yuv[4 * (i / 2) + 2 + j * width * 2];
				}
			}
*/
			ImgConvert.VYUY_to_RGB24(yuv, bits, width, height, 1, 0, 2) ;
			delete [] yuv ;
			delete [] y ;
			delete [] uv ;
			fclose(fp) ;
			return S_OK ;
		}

//		type = IMG_TYPE_422 ;
		return E_NOTIMPL ;
	} else if(0 && !_tcsicmp(ext, _T("AVI"))) {
		if(aviLoader.init(path) == S_OK) {
			aviLoader.fetch(aviLoader.index) ;
			startIndex = index = aviLoader.index ;
			endIndex = aviLoader.index_end ;
			if(bits) {
				delete [] bits ;
			}
			bits = new unsigned char[aviLoader.size] ;
			if(!bits) return E_ABORT ;
			memcpy(bits, aviLoader.buffer, aviLoader.size) ;
			width = aviLoader.width ;
			height = aviLoader.height ;

		}
		type = IMG_TYPE_AVI ;

		return S_OK ;
	} else if(!_tcsicmp(ext, _T("AVI")) || !_tcsicmp(ext, _T("WMV")) || !_tcsicmp(ext, _T("RMVB"))) {
#ifdef DIRECTSHOW_SUPPORT
		if(dsVideoGrabber.Init(AfxGetApp()->GetMainWnd()->GetSafeHwnd()) == S_OK) {
/*
			{
				int i ;
				CDSVideoGrabber *vg ;

				for(i = 0; i < 1; i++) {
					vg = new CDSVideoGrabber ;

					dsVideoGrabber.LoadFile(path) ;

					delete vg ;
				}
			}
*/
			if(dsVideoGrabber.LoadFile(path)) {
				return E_ABORT ;
			}
			width = dsVideoGrabber.GetVideoWidth() ;
			height = dsVideoGrabber.GetVideoHeight() ;
			if(bits) {
				delete [] bits ;
			}
			bits = new unsigned char [width * height * 3] ;

			if(!bits) return E_ABORT ;

			dsVideoGrabber.WaitForFrame(bits) ;

			startIndex = index = 0 ;
			endIndex = (int) (dsVideoGrabber.frameCount - 1) ;

			type = IMG_TYPE_DSV ;
			return S_OK ;
		}
#endif
		return E_ABORT ;
	} else if(!_tcsicmp(ext, _T("H264"))) {
#ifdef H264_FILE_SUPPORT
		char mbPath[512] ;
		size_t numConverted ;
		int decStatus ;
		CImageConvert ImgConvert ;

		wcstombs_s(&numConverted, mbPath, 512, path, 512) ;
		pH264File = H264FileOpen(mbPath) ;
		if(pH264File) {
			startIndex = index = 0 ;
			endIndex = pH264File->tFrameNum - 1 ;

			decStatus = H264FileDecode(pH264File) ;
			if(decStatus != DEC_PARSE_OK) {
				H264FileClose(pH264File) ;
				pH264File = NULL ;
				return E_ABORT ;
			}

			width = pH264File->maxWidth ;
			height = pH264File->maxHeight ;
			if(bits) {
				delete [] bits ;
			}
			bits = new unsigned char [width * height * 3] ;

			if(!bits) {
				H264FileClose(pH264File) ;
				pH264File = NULL ;
				return E_ABORT ;
			}

			ImgConvert.YUV420_to_RGB24(pH264File->pDec->pFrameContext->recon, bits, width, height) ;
			type = IMG_TYPE_264 ;

			return S_OK ;
		}
#endif
		return E_ABORT ;
	} else if(!_tcsicmp(ext, _T("Y")) || type == IMG_TYPE_Y_U_V) {
		CImgParam ImgParam ;
		CImageConvert ImgConvert ;
		unsigned char *yuv ;
		FILE *fp ;
		TCHAR path_uv[256] ;
		TCHAR *dotptr_uv ;

		if(init) {
			ImgParam.DoModal() ;
			width = ImgParam.width ;
			height = ImgParam.height ;
			type = ImgParam.format ;
		}

		_tcscpy_s(path_uv, path) ;

		if(_tfopen_s(&fp, path, _T("rb"))) return E_ABORT ;
		if(bits) delete [] bits ;

		bits = new unsigned char [width * height * 3] ;
		yuv = new unsigned char [ImgConvert.SizeYUV420(width, height)] ;

		fread(yuv, 1, width * height, fp) ;
		fclose(fp) ;

		dotptr_uv = _tcschr(path_uv, '.') ;
		dotptr_uv++ ;
		*dotptr_uv = 'U' ;

		
		if(_tfopen_s(&fp, path_uv, _T("rb"))) {
			delete [] yuv ;
			return E_ABORT ;
		}
		fread(yuv + width * height, 1, width * height / 4, fp) ;
		fclose(fp) ;

		*dotptr_uv = 'V' ;

		if(_tfopen_s(&fp, path_uv, _T("rb"))) {
			delete [] yuv ;
			return E_ABORT ;
		}
		fread(yuv + width * height + width * height / 4, 1, width * height / 4, fp) ;
		fclose(fp) ;

		ImgConvert.YUV420_to_RGB24(yuv, bits, width, height) ;

		delete [] yuv ;

		type = IMG_TYPE_Y_U_V ;

		return S_OK ;
	} else if(!_tcsicmp(ext, _T("IPT")) || !_tcsicmp(ext, _T("IPT"))) {
		FILE *fp ;
		int numLines ;
		TCHAR buf[1024] ;
		TCHAR FMTstr[256] ;
		TCHAR IMGtype[256] ;
		int start, end, numParam ;
		int status ;

		if(_tfopen_s(&fp, path, _T("rt"))) return E_ABORT ;

		numLines = 0 ;
		while(_fgetts(buf, 256, fp)) {
			numLines++ ;
		}
		
		if(FMT) {
			delete [] FMT ;
		}

		FMT = (struct IPTFormat *) malloc(sizeof(struct IPTFormat) * numLines) ;
		if(!FMT) return E_ABORT ;

		fseek(fp, SEEK_SET, 0) ;
		fmtCount = 0 ;
		startIndex = endIndex = 0 ;
		FMTstr[0] = 0 ;
		while(_fgetts(buf, 256, fp)) {
			numParam = _stscanf_s(buf, _T("\"%[^\"]s %d %d %s %d %d"), FMTstr, 255, &start, &end, IMGtype, 255, &width, &height) ;
			numParam += _stscanf_s(buf + _tcslen(FMTstr) + 2, _T("%d %d %s %d %d"), &start, &end, IMGtype, 255, &width, &height) ;
			if(numParam <= 0) {
				numParam = _stscanf_s(buf, _T("%s %d %d %s %d %d"), FMTstr, 255, &start, &end, IMGtype, 255, &width, &height) ;
			}
			if(numParam > 0) {
				FMT[fmtCount].type = IMG_TYPE_UDF ;
				_tcscpy_s(FMT[fmtCount].format, FMTstr) ;
				if(numParam < 2) {
					FMT[fmtCount].current = FMT[fmtCount].start = FMT[fmtCount].end = 0 ;
				} else if(numParam < 3) {
					FMT[fmtCount].current = FMT[fmtCount].start = FMT[fmtCount].end = start ;
				} else if(numParam < 4) {
					FMT[fmtCount].current = FMT[fmtCount].start = start ;
					FMT[fmtCount].end = end ;
				} else if(numParam == 6) {
					FMT[fmtCount].current = FMT[fmtCount].start = start ;
					FMT[fmtCount].end = end ;
					if(!_tcsicmp(IMGtype, _T("YUV420"))) {
						FMT[fmtCount].type = IMG_TYPE_420 ;
					} else if(!_tcsicmp(IMGtype, _T("YUV420_V"))) {
						FMT[fmtCount].type = IMG_TYPE_420_V ;
					} else if(!_tcsicmp(IMGtype, _T("YUV422"))) {
						FMT[fmtCount].type = IMG_TYPE_422 ;
					} else if(!_tcsicmp(IMGtype, _T("YUV422_V"))) {
						FMT[fmtCount].type = IMG_TYPE_422_V ;
					} else if(!_tcsicmp(IMGtype, _T("YUYV"))) {
						FMT[fmtCount].type = IMG_TYPE_VYUY ;
					} else if(!_tcsicmp(IMGtype, _T("YUYV_V"))) {
						FMT[fmtCount].type = IMG_TYPE_VYUY_V ;
					} else if(!_tcsicmp(IMGtype, _T("PPM"))) {
						FMT[fmtCount].type = IMG_TYPE_PPM ;
					} else if(!_tcsicmp(IMGtype, _T("Y_U_V"))) {
						FMT[fmtCount].type = IMG_TYPE_Y_U_V ;
					}
				}
				fmtCount++ ;
			}
			endIndex += (end - start + 1) ;
		}
		endIndex-- ;

		fmtIndex = 0 ;
		_stprintf_s(buf, FMT[0].format, FMT[0].current) ;
		type = FMT[0].type ;
		status = Load(buf, FMT[0].type == IMG_TYPE_UDF) ;

		if(status == S_OK) {
			if(!_tcsicmp(ext, _T("IPT"))) {
				type = IMG_TYPE_IPT ;
			} else {
				type = IMG_TYPE_VPT ;
			}
		}

		fclose(fp) ;
		return status ;
	} else {
		int spitch, dpitch, bpp ;
		unsigned char *dest, *src ;
		CImage image ;

		if(!_tcsicmp(ext, _T("PNG"))) {
			type = IMG_TYPE_PNG ;
		} else if(!_tcsicmp(ext, _T("JPG"))) {
			type = IMG_TYPE_JPG ;
		} else if(!_tcsicmp(ext, _T("GIF"))) {
			type = IMG_TYPE_GIF ;
		} else if(!_tcsicmp(ext, _T("BMP"))) {
			type = IMG_TYPE_BMP ;
		}
		hresult = image.Load(path) ;
		if(hresult) return hresult ;

		width = image.GetWidth() ;
		height = image.GetHeight() ;
		spitch = image.GetPitch() ;
		bpp = image.GetBPP() ;

		if(bits) {
			delete [] bits ;
		}
		if(bpp == 24 || bpp == 32) {
			bits = new unsigned char[width * height * 3] ;
			if(!bits) return E_ABORT ;

			if(spitch > 0) {
				dest = bits + width * (height - 1) * 3;
				dpitch = -width * 3 ;
			} else {
				dest = bits ;
				dpitch = width * 3 ;
			}
			src = (unsigned char *) image.GetBits() ;

			for(int i = 0; i < height; i++) {
				memcpy(dest, src, width * 3) ;
				dest += dpitch ;
				src += spitch ;
			}
		} else if(bpp == 8) {

			bits = new unsigned char[width * height * 3] ;
			if(!bits) return E_ABORT ;

			if(spitch > 0) {
				dest = bits + width * (height - 1) * 3;
				dpitch = -width * 3 ;
			} else {
				dest = bits ;
				dpitch = width * 3 ;
			}
			src = (unsigned char *) image.GetBits() ;

			for(int i = 0; i < height; i++) {
				for(int j = 0; j < width; j++) {
					dest[j * 3 + 0] = dest[j * 3 + 1] = dest[j * 3 + 2] = src[j] ;
				}
				dest += dpitch ;
				src += spitch ;
			}
		} else {
			int x, y ;

			bits = new unsigned char[width * height * 3] ;
			if(!bits) return E_ABORT ;

			for(y = 0; y < height; y++) {
				for(x = 0; x < width; x++) {
					int color ;
					int dy ;

					dy = spitch < 0 ? y : height - 1 - y ;

					color = image.GetPixel(x, y) ;
					bits[(x + dy * width) * 3 + 2] = color & 0xFF ;
					color >>= 8 ;
					bits[(x + dy * width) * 3 + 1] = color & 0xFF ;
					color >>= 8 ;
					bits[(x + dy * width) * 3 + 0] = color & 0xFF ;
				}
			}
		}

		return hresult ;
	}
	return E_NOTIMPL ;
}

HRESULT CImageLoader::Create(int cwidth, int cheight, unsigned char *copy) {

	if(bits) delete bits ;
	bits = new unsigned char [cwidth * cheight * 3 + 4] ;
	if(!bits) return E_ABORT ;

	width = cwidth ;
	height = cheight ;
	index = startIndex = endIndex = 0 ;
	if(copy) {
		memcpy(bits, copy, cwidth * cheight * 3) ;
	}
	type = IMG_TYPE_UDF ;
	return S_OK ;
}

HRESULT CImageLoader::Next() {

	if(type == IMG_TYPE_AVI) {
		if(aviLoader.fetch(index + 1)) {
			memcpy(bits, aviLoader.buffer, aviLoader.size) ;
			index++ ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
	} else if(type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
		HRESULT hr ;

		if(!(hr = dsVideoGrabber.NextFrame(bits))) {
			index++ ;
			return S_OK ;
		} else {
			if(hr == E_PENDING) return hr ;
			if(index < endIndex) return E_PENDING ;
			else {
				return E_POINTER ;
			}
		}
#endif
		return E_POINTER ;
	} else if(type == IMG_TYPE_264) {
#ifdef H264_FILE_SUPPORT
		CImageConvert ImgConvert ;

		if(!H264FileDecode(pH264File)) {
			ImgConvert.YUV420_to_RGB24(pH264File->pDec->pFrameContext->recon, bits, width, height) ;
			index++ ;
			return S_OK ;
		}
#endif
		return E_POINTER ;
	} else if(type == IMG_TYPE_CAP) {
		unsigned char *cap_ptr ;

		cap_ptr = videoCapture.getFrame() ;
		if(cap_ptr) {
			memcpy(bits, cap_ptr, width * height * 3) ;
			index++ ;
		}
	} else if(type == IMG_TYPE_IPT || type == IMG_TYPE_VPT) {
		TCHAR path[256] ;
		int pType = type ;

		if(FMT[fmtIndex].current >= FMT[fmtIndex].end) {
			if(fmtIndex < (fmtCount - 1)) {
				fmtIndex++ ;
				FMT[fmtIndex].current = FMT[fmtIndex].start ;
			} else return E_POINTER ;
		} else {
			FMT[fmtIndex].current++ ;
		}
		_stprintf_s(path, FMT[fmtIndex].format, FMT[fmtIndex].current) ;
		type = FMT[fmtIndex].type ;
		index++ ;
		if(Load(path, FMT[fmtIndex].type == IMG_TYPE_UDF) == S_OK) {
			if(pType == IMG_TYPE_IPT)
				type = IMG_TYPE_IPT ;
			else if(pType == IMG_TYPE_VPT)
				type = IMG_TYPE_VPT ;
			return S_OK ;
		} else {
			index-- ;
		}
		if(pType == IMG_TYPE_IPT)
			type = IMG_TYPE_IPT ;
		else if(pType == IMG_TYPE_VPT)
			type = IMG_TYPE_VPT ;
		return E_POINTER ;
	}

	return S_OK ;
}

HRESULT CImageLoader::StepNext() {

	if(type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
		if(!dsVideoGrabber.StepNextFrame(bits)) {
			index++ ;
			return S_OK ;
		} else {
			if(index < endIndex) return E_PENDING ;
			else return E_POINTER ;
		}
#endif
		return E_POINTER ;
	} else {
		return Next() ;
	}
}

HRESULT CImageLoader::StepPrevious() {

	if(type == IMG_TYPE_AVI) {
		if(aviLoader.fetch(index - 1)) {
			memcpy(bits, aviLoader.buffer, aviLoader.size) ;
			index-- ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
	} else if(type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
		if(!dsVideoGrabber.StepPrevFrame(bits)) {
			index-- ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
#endif
		return E_POINTER ;
	}  else if(type == IMG_TYPE_264) {
#ifdef H264_FILE_SUPPORT
		CImageConvert ImgConvert ;

		if(!H264FileSeek(pH264File, index - 1)) {
			ImgConvert.YUV420_to_RGB24(pH264File->pDec->pFrameContext->recon, bits, width, height) ;
			index-- ;

			return S_OK ;
		}
#endif
		return E_POINTER ;
	} else if(type == IMG_TYPE_IPT || type == IMG_TYPE_VPT) {
		TCHAR path[256] ;
		int pType = type ;

		if(FMT[fmtIndex].current <= FMT[fmtIndex].start) {
			if(fmtIndex > 0) {
				fmtIndex-- ;
				FMT[fmtIndex].current = FMT[fmtIndex].end ;
			} else return E_POINTER ;
		} else {
			FMT[fmtIndex].current-- ;
		}
		_stprintf_s(path, FMT[fmtIndex].format, FMT[fmtIndex].current) ;
		type = FMT[fmtIndex].type ;
		index-- ;
		if(index >= 0 && (Load(path, FMT[fmtIndex].type == IMG_TYPE_UDF) == S_OK)) {
			if(pType == IMG_TYPE_IPT)
				type = IMG_TYPE_IPT ;
			else if(pType == IMG_TYPE_VPT)
				type = IMG_TYPE_VPT ;
			return S_OK ;
		} else {
			index++ ;
		}
		if(pType == IMG_TYPE_IPT)
			type = IMG_TYPE_IPT ;
		else if(pType == IMG_TYPE_VPT)
			type = IMG_TYPE_VPT ;
		return E_POINTER ;
	}
	return S_OK ;
}

HRESULT CImageLoader::Previous() {

	return StepPrevious() ;
}

HRESULT CImageLoader::LoadFrame(int idx) {

	if(type == IMG_TYPE_AVI) {
		if(aviLoader.fetch(idx)) {
			memcpy(bits, aviLoader.buffer, aviLoader.size) ;
			index = idx ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
	} if(type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
		if(!dsVideoGrabber.SeekToFrameIdx(idx)) {
			dsVideoGrabber.WaitForFrame(bits) ;
			index = idx ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
#endif
		return E_POINTER ;
	} if(type == IMG_TYPE_264) {
#ifdef H264_FILE_SUPPORT
		CImageConvert ImgConvert ;

		if(!H264FileSeek(pH264File, idx)) {
			ImgConvert.YUV420_to_RGB24(pH264File->pDec->pFrameContext->recon, bits, width, height) ;

			return S_OK ;
		}
#endif
		return E_POINTER ;
	}
	else if(type == IMG_TYPE_IPT || type == IMG_TYPE_VPT) {
		TCHAR path[256] ;
		int startIdx, endIdx, fetchIdx ;
		int i ;
		int pType = type ;

		startIdx = 0 ;
		endIdx = FMT[0].end ;
		for(i = 0; i < fmtCount; i++) {
			endIdx += (FMT[i].end - FMT[i].start) ;
			if(startIdx <= idx && idx <= endIdx) {
				fetchIdx = idx - startIdx ;
				break ;
			}
			startIdx = endIdx + 1 ;
		}

		if(i < fmtCount) {
			fmtIndex = i ;
			FMT[fmtIndex].current = fetchIdx ;
		} else {
			return E_POINTER ;
		}

		_stprintf_s(path, FMT[fmtIndex].format, FMT[fmtIndex].current) ;
		type = FMT[fmtIndex].type ;
		index = idx ;

		if(Load(path, FMT[fmtIndex].type == IMG_TYPE_UDF) == S_OK) {
			if(pType == IMG_TYPE_IPT)
				type = IMG_TYPE_IPT ;
			else if(pType == IMG_TYPE_VPT)
				type = IMG_TYPE_VPT ;
			type = IMG_TYPE_IPT ;
			return S_OK ;
		}

		if(pType == IMG_TYPE_IPT)
			type = IMG_TYPE_IPT ;
		else if(pType == IMG_TYPE_VPT)
			type = IMG_TYPE_VPT ;
		return E_POINTER ;
	}
	return S_OK ;
}

HRESULT CImageLoader::Rewind() {

	if(type == IMG_TYPE_AVI) {
		if(aviLoader.fetch(startIndex)) {
			memcpy(bits, aviLoader.buffer, aviLoader.size) ;
			index = startIndex ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
	} if(type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
		if(!dsVideoGrabber.SeekToFrameIdx(0)) {
			dsVideoGrabber.WaitForFrame(bits) ;
			index = 0 ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
#endif
		return E_POINTER ;
	} if(type == IMG_TYPE_264) {
#ifdef H264_FILE_SUPPORT
		CImageConvert ImgConvert ;

		if(!H264FileSeek(pH264File, 0)) {
			ImgConvert.YUV420_to_RGB24(pH264File->pDec->pFrameContext->recon, bits, width, height) ;
			index = 0 ;
			return S_OK ;
		} else {
			return E_POINTER ;
		}
#endif
	}
	else if(type == IMG_TYPE_IPT || type == IMG_TYPE_VPT) {
		TCHAR path[256] ;
		int prev_index ;
		int pType = type ;

		fmtIndex = 0 ;
		FMT[fmtIndex].current = FMT[fmtIndex].start ;
		_stprintf_s(path, FMT[fmtIndex].format, FMT[fmtIndex].current) ;
		type = FMT[fmtIndex].type ;
		prev_index = index ;
		index = 0 ;
		if(Load(path, FMT[fmtIndex].type == IMG_TYPE_UDF) == S_OK) {
			if(pType == IMG_TYPE_IPT)
				type = IMG_TYPE_IPT ;
			else if(pType == IMG_TYPE_VPT)
				type = IMG_TYPE_VPT ;
			return S_OK ;
		} else {
			index = prev_index ;
		}
		if(pType == IMG_TYPE_IPT)
			type = IMG_TYPE_IPT ;
		else if(pType == IMG_TYPE_VPT)
			type = IMG_TYPE_VPT ;
		return E_POINTER ;
	}
	return S_OK ;
}

void CImageLoader::PutDC(HDC hDC) {

	unsigned char *nbits ;
	int nwidth, nheight, i ;
	BITMAPINFO bmi ;

	nwidth = (width + 3) / 4 * 4;
	nheight = height ;

	memset(&bmi, 0, sizeof(bmi)) ;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
	bmi.bmiHeader.biCompression = BI_RGB ;
	bmi.bmiHeader.biBitCount = 24 ;
	bmi.bmiHeader.biPlanes = 1 ;
	bmi.bmiHeader.biWidth = nwidth ;
	bmi.bmiHeader.biHeight = nheight ;

	nbits = new unsigned char[nwidth * nheight * 3] ;
/* For debugging purpose only
	{
	CImageConvert ImgConvert ;
	unsigned char *vyuy ;

	vyuy = new unsigned char[width * height * 3] ;
//	ImgConvert.RGB24_to_YUV420(bits, vyuy, width, height) ;
//	ImgConvert.YUV420_to_RGB24(vyuy, bits, width, height) ;
	ImgConvert.RGB24_to_VYUY(bits, vyuy, width, height) ;
	ImgConvert.VYUY_to_RGB24(vyuy, bits, width, height) ;
	delete [] vyuy ;
	}
*/
	for(i = 0; i < height; i++) {
		memcpy(nbits + (height - 1 - i) * nwidth * 3, bits + i * width * 3, width * 3) ;
	}
	/* For debugging purpose only
	{
	CImageConvert ImgConvert ;
	CImageFilter ImgFilter ;
	unsigned char *yuv ;
	double *d_buf ;
	double *s1_buf, *s2_buf ;

	yuv = new unsigned char[ImgConvert.SizeYUV420(width, height)] ;
	d_buf = new double [width * height] ;
	s1_buf = new double [width * height] ;
	s2_buf = new double [width * height] ;

	ImgConvert.RGB24_to_YUV420(bits, yuv, width, height) ;
	ImgConvert.Char8_to_Double(yuv, d_buf, width, height) ;
	ImgFilter.Sobel_H(d_buf, s1_buf, width, height) ;
	ImgFilter.Sobel_V(d_buf, s2_buf, width, height) ;
	for(int i = 0; i < width * height; i++)
	s1_buf[i] = abs(s1_buf[i]) + abs(s2_buf[i]) ;
	ImgConvert.Double_to_Char8(s1_buf, yuv, width, height, TRUE) ;
	ImgConvert.Char8_to_RGB24(yuv, bits, width, height) ;

	delete [] yuv ;
	delete [] d_buf ;
	delete [] s1_buf ;
	delete [] s2_buf ;
	}
	*/
	SetDIBitsToDevice(
			hDC,
//			pDC->GetSafeHdc() ,           // handle to DC
			0,
			0,
			width,
			height,
			0,
			0,
			0,   // first scan line to set
			height,   // number of scan lines to copy
			nbits,    // array for bitmap bits
			&bmi, // bitmap data buffer
			DIB_RGB_COLORS        // RGB or palette index
	) ;


	delete [] nbits ;
}
