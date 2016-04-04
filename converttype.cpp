#include "converttype.h"
#include <QUuid>

LPWSTR toLPWSTR(QString str){
 return (LPWSTR)(str.utf16());
}

QString from_guid(GUID guid) {
    QUuid uuid(guid);
    return uuid.toString();
}

_bstr_t to_bstr_t(QString str){
    //    return (_bstr_t)(str.utf16());
    BSTR result= SysAllocStringLen(0, str.length());
    str.toWCharArray(result);
    return result;
}

QString from_bstr_t(_bstr_t str){
    return QString::fromUtf16(reinterpret_cast<const ushort*>((const WCHAR*)str));
}

QVariant from_variant_t(const _variant_t &arg){
    QVariant var;
    switch(arg.vt) {
    case VT_BSTR:
        var = QString::fromWCharArray(arg.bstrVal);
        break;
    case VT_BSTR|VT_BYREF:
        var = QString::fromWCharArray(*arg.pbstrVal);
        break;
    case VT_BOOL:
        var = QVariant((bool)arg.boolVal);
        break;
    case VT_BOOL|VT_BYREF:
        var = QVariant((bool)*arg.pboolVal);
        break;
    case VT_I1:
        var = arg.cVal;
        /*if (typeName == "char")
            type = QVariant::Int;*/
        break;
    case VT_I1|VT_BYREF:
        var = *arg.pcVal;
        /*if (typeName == "char")
            type = QVariant::Int;*/
        break;
    case VT_I2:
        var = arg.iVal;
        /*if (typeName == "short")
            type = QVariant::Int;*/
        break;
    case VT_I2|VT_BYREF:
        var = *arg.piVal;
        /*if (typeName == "short")
            type = QVariant::Int;*/
        break;
    case VT_I4:
        var = (int)arg.lVal;
        break;
    case VT_I4|VT_BYREF:
        var = (int)*arg.plVal;
        break;
    case VT_INT:
        var = arg.intVal;
        break;
    case VT_INT|VT_BYREF:
        var = *arg.pintVal;
        break;
    case VT_UI1:
        var = arg.bVal;
        break;
    case VT_UI1|VT_BYREF:
        var = *arg.pbVal;
        break;
    case VT_UI2:
        var = arg.uiVal;
        break;
    case VT_UI2|VT_BYREF:
        var = *arg.puiVal;
        break;
    case VT_UI4:
        var = (int)arg.ulVal;
        break;
    case VT_UI4|VT_BYREF:
        var = (int)*arg.pulVal;
        break;
    case VT_UINT:
        var = arg.uintVal;
        break;
    case VT_UINT|VT_BYREF:
        var = *arg.puintVal;
        break;
    case VT_CY:
        var = arg.cyVal.int64;
        break;
    case VT_CY|VT_BYREF:
        var = arg.pcyVal->int64;
        break;
    case VT_R4:
        var = arg.fltVal;
        break;
    case VT_R4|VT_BYREF:
        var = *arg.pfltVal;
        break;
    case VT_R8:
        var = arg.dblVal;
        break;
    case VT_R8|VT_BYREF:
        var = *arg.pdblVal;
        break;
    }
    return var;
}



