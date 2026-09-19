#ifndef WOLAPI_COMMAONDEFS_H
#define WOLAPI_COMMAONDEFS_H

#define WOLAPI_OK 0

#define WOLAPI_SEVERITY_ERROR 1
#define WOLAPI_SEVERITY_SUCCESS 0
#define WOLAPI_FACILITY_ITF 4

#define WOLAPI_MAKE_HRESULT(sev,fac,code) \
	((WOL::WOLAPI_RESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

#endif /* WOLAPI_COMMAONDEFS_H */
