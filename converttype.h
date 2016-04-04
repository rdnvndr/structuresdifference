#ifndef CONVERTTYPE_H
#define CONVERTTYPE_H

#include <QVariant>
#include <comutil.h>
#include <windows.h>

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
