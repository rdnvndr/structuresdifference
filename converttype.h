#ifndef CONVERTTYPE_H
#define CONVERTTYPE_H

#include <QVariant>
#include <comutil.h>
#include <windows.h>

GUID to_guid(QString suuid);
QString from_guid(GUID guid);

template< class T >
T *from_vdispatch(variant_t arg) {
    IDispatch *disp = to_dispatch(arg);
    T *iObj = 0;
    if (disp)
        disp->QueryInterface(__uuidof(T), (void**)&iObj);
    return iObj;
}

IDispatch* to_dispatch(_variant_t arg);

//! Конвертирование из QString в LPWSTR
LPWSTR toLPWSTR(QString str);

//! Конвертирование из QString в _bstr_t
_bstr_t to_bstr_t(QString str);

//! Конвертирование из _bstr_t в QString
QString from_bstr_t(_bstr_t str);

//! Конвертирование из _variant_t в QVariant
QVariant from_variant_t(const _variant_t &arg);

QString from_guid(GUID guid);

#endif // CONVERTTYPE_H
