#include "structuresdifference.h"
#include "converttype.h"
#include <QString>
#include <QDebug>

const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

StructuresDifference::StructuresDifference(QObject *parent) : QObject(parent)
{
    ::CoInitialize(NULL);
    ::CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                           RPC_C_IMP_LEVEL_DELEGATE, NULL, 0, NULL);
    uniRef = NULL;
}

bool StructuresDifference::connect()
{
    HRESULT Hr = ::CoCreateInstance(__uuidof(UniReference::UniRefer),
                                    NULL, CLSCTX_INPROC_SERVER,
                                    __uuidof(UniReference::IUniRefer), (void**)&uniRef);
    if (Hr == S_OK) {
        uniRef->GlobalVars()->Logon()->LogonAsParams(to_bstr_t("Администратор"),
                                                     to_bstr_t("111"),
                                                     to_bstr_t("Администраторы"));

        return true;
    }
    return false;
}

vkernelLib::IVModel *StructuresDifference::loadFile(QString filename)
{
    vkernelLib::IVModel *vModel = NULL;
    HRESULT Hr = ::CoCreateInstance(__uuidof(vkernelLib::VModel),
                                       NULL, CLSCTX_INPROC_SERVER,
                                       __uuidof(vkernelLib::IVModel), (void**)&vModel);
     if (Hr == S_OK) {
         vModel->vrLoadModel(to_bstr_t(filename),
                                NULL, vkernelLib::OPEN_FULL_STR_LOCAL);
         vModel->vrApplySecurity();
         return vModel;
     } else
         return NULL;
}

QString StructuresDifference::groupsDiff(vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst)
{
    QString result;
    m_scrtSrc = vModelSrc->vrGetLocalSecurity();
    m_scrtDst = vModelDst->vrGetLocalSecurity();
    for (int i= 0;i < uniRef->GlobalVars()->Logon()->ListGroupsUser()->CountGroupsUser; i++)
    {
        _bstr_t groupGuid = uniRef->GlobalVars()->Logon()->ListGroupsUser()->GroupByIndex(i)->GuidGroup;
        _bstr_t groupName = uniRef->GlobalVars()->Logon()->ListGroupsUser()->GroupByIndex(i)->NameGroup;
        unsigned long groupIdSrc = m_scrtSrc->vlsGetItemByGUID(groupGuid);
        unsigned long groupIdDst = m_scrtDst->vlsGetItemByGUID(groupGuid);

        if (groupIdSrc==0 || groupIdDst==0) {

            result = result + "\nНе существует группа: "
                    + groupName + "("
                    + groupGuid + ")";
            continue;
        }

        m_groupsSrc.append(groupIdSrc);
        m_groupsDst.append(groupIdDst);
        m_groupNames.append(from_bstr_t(groupName));
    }
    return result;
}

QString StructuresDifference::classPermDiff(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
{
    const int classPermMask = vkernelLib::SF_LCK
                            | vkernelLib::SF_WRT
                            | vkernelLib::SF_VSB
                            | vkernelLib::SF_EXE
                            | vkernelLib::SF_DEL
                            | vkernelLib::SF_CRT;

     QString result;
     vkernelLib::LDIGEST digestSrc = {{0,0,0,0}};
     vkernelLib::LDIGEST digestDst = {{0,0,0,0}};
     int rightsSrc;
     int rightsDst;

     for (int i=0; i < m_groupsSrc.count(); i++) {
         vClassSrc->vrGetDigest(&(digestSrc.data[0]), &(digestSrc.data[1]),
                                &(digestSrc.data[2]), &(digestSrc.data[3]));
         vClassDst->vrGetDigest(&(digestDst.data[0]), &(digestDst.data[1]),
                                &(digestDst.data[2]), &(digestDst.data[3]));

         m_scrtSrc->vlsGetRights(m_groupsSrc.at(i), &digestSrc, &rightsSrc);
         m_scrtDst->vlsGetRights(m_groupsDst.at(i), &digestDst, &rightsDst);

         rightsSrc = rightsSrc & classPermMask;
         rightsDst = rightsDst & classPermMask;

         if (rightsSrc != rightsDst) {
            result = result
                   + QString("\n        %1 !=  %2 - %3").arg(rightsSrc,2).arg(rightsDst,2).arg(m_groupNames.at(i));
         }
     }
     if (!result.isEmpty())
         result = "\n    Права доступа изменены:" + result;

     return result;
}

QString StructuresDifference::attrPermDiff(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
{
    const int smplAttrPermMask = vkernelLib::SF_WRT
                               | vkernelLib::SF_VSB;

    const int calcAttrPermMask = vkernelLib::SF_WRT
                               | vkernelLib::SF_VSB
                               | vkernelLib::SF_EXE;

    const int funcAttrPermMask = vkernelLib::SF_EXE;

    int attrPermMask;
    switch (vAttrSrc->vrType) {
        case 0: attrPermMask = smplAttrPermMask;
                break;
        case 1: attrPermMask = funcAttrPermMask;
            break;
        case 2: attrPermMask = calcAttrPermMask;
            break;
    }

    QString result;
    vkernelLib::LDIGEST digestSrc = {{0,0,0,0}};
    vkernelLib::LDIGEST digestDst = {{0,0,0,0}};

    int rightsSrc;
    int rightsDst;

    for (int i=0; i < m_groupsSrc.count(); i++) {
        vAttrSrc->vrGetDigest(&(digestSrc.data[0]), &(digestSrc.data[1]),
                               &(digestSrc.data[2]), &(digestSrc.data[3]));
        vAttrDst->vrGetDigest(&(digestDst.data[0]), &(digestDst.data[1]),
                               &(digestDst.data[2]), &(digestDst.data[3]));

        m_scrtSrc->vlsGetRights(m_groupsSrc.at(i), &digestSrc, &rightsSrc);
        m_scrtDst->vlsGetRights(m_groupsDst.at(i), &digestDst, &rightsDst);

        rightsSrc = rightsSrc & attrPermMask;
        rightsDst = rightsDst & attrPermMask;

        if (rightsSrc != rightsDst) {
           result = result
                  + QString("\n            %1 !=  %2 - %3").arg(rightsSrc,2).arg(rightsDst,2).arg(m_groupNames.at(i));
        }
    }
    if (!result.isEmpty())
        result = "\n        Права доступа изменены:" + result;

    return result;
}

QString StructuresDifference::attrDiff(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst) {

    QString result;
    QString attrType = "Неизвестынй объект";
    switch (vAttrSrc->vrType) {
        case 0: attrType = "Атрибут";
                break;
        case 1: attrType = "Функция";
            break;
        case 2: attrType = "Вычисляемый атрибут";
            break;
    }

    QString nameAttr = from_bstr_t(vAttrSrc->vrName);
    if (vAttrDst == NULL) {
        if (vAttrSrc->vrType == 1) {
            vkernelLib::IVClass *vClassSrc = vAttrSrc->vrClass();
            vkernelLib::IVClassValue *vAttrCodeSrc = NULL;
            for (int k=0; k < vClassSrc->vrClassValuesCount(); k++){
                vAttrCodeSrc = vClassSrc->vriClassValueItem(k);
                if (vAttrCodeSrc->vrFunctionCode == vAttrSrc->vrName
                        || vAttrCodeSrc->vrCalcSetFunction == vAttrSrc->vrName)
                    return result;
            }
        }
        result = "\n    "+attrType+" удалён(а): " + nameAttr;
        return result;

    }

    vkernelLib::IVClassValue *vInheritedAttrSrc = vAttrSrc->vrInheritedFrom;
    vkernelLib::IVClassValue *vInheritedAttrDst = vAttrDst->vrInheritedFrom;
    GUID baseClassGuidSrc = (vInheritedAttrSrc != NULL) ? vInheritedAttrSrc->vrClassValueID : GUID_NULL;
    GUID baseClassGuidDst = (vInheritedAttrDst != NULL) ? vInheritedAttrDst->vrClassValueID : GUID_NULL;

    if (baseClassGuidSrc == GUID_NULL)
        result += this->attrPermDiff(vAttrSrc, vAttrDst);

    bool sysFunc = vAttrSrc->vrType != 1
            && nameAttr.compare("showme", Qt::CaseInsensitive)==0
            && nameAttr.compare("showinplan", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterconstruction", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterfirstlink", Qt::CaseInsensitive)==0
            && nameAttr.compare("beforedelete", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterupdate", Qt::CaseInsensitive)==0
            && nameAttr.compare("canchange", Qt::CaseInsensitive)==0
            && nameAttr.startsWith("diag_", Qt::CaseInsensitive)==0;
    if (vAttrSrc->vrClassValueID != vAttrDst->vrClassValueID && sysFunc)
        result = result + "\n        Идентификатор: "
               + from_guid(vAttrSrc->vrClassValueID) + " != "
               + from_guid(vAttrDst->vrClassValueID);

    if (vAttrSrc->GetvrDataType() !=  vAttrDst->GetvrDataType())
         result = result + "\n        Тип данных: "
                 + from_guid(vAttrSrc->GetvrDataType()) + " != "
                 + from_guid(vAttrDst->GetvrDataType());

    if (vAttrSrc->vrType != vAttrDst->vrType)
        result = result + "\n        Тип атрибута: "
                + vAttrSrc->vrType + " != "
                + vAttrDst->vrType;

    if (vAttrSrc->vrScreenName != vAttrDst->vrScreenName)
        result = result + "\n        Экранное имя: "
                + from_bstr_t(vAttrSrc->vrScreenName) + " != "
                + from_bstr_t(vAttrDst->vrScreenName);

    if (vAttrSrc->vrAliasName != vAttrDst->vrAliasName)
        result = result + "\n        Справочники: "
                + from_bstr_t(vAttrSrc->vrAliasName) + " != "
                + from_bstr_t(vAttrDst->vrAliasName);


    if (vAttrSrc->vrType == 2) {
        vkernelLib::IVClass *vClassSrc = vAttrSrc->vrClass();
        vkernelLib::IVClassValue *vAttrCodeSrc = NULL;

        for (int k=0; k < vClassSrc->vrClassValuesCount(); k++){
            vAttrCodeSrc = vClassSrc->vriClassValueItem(k);
            if (vAttrCodeSrc->vrName == vAttrSrc->vrFunctionCode)
                break;
        }
        vkernelLib::IVClass *vClassDst = vAttrDst->vrClass();
        vkernelLib::IVClassValue *vAttrCodeDst = NULL;
        for (int k=0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrName == vAttrDst->vrFunctionCode)
                break;
        }
        if (vAttrCodeSrc != NULL && vAttrCodeDst != NULL)
            if (vAttrCodeSrc->vrFunctionCode != vAttrCodeDst->vrFunctionCode)
                result = result + "\n        Функция чтения: изменена";

        for (int k=0; k < vClassSrc->vrClassValuesCount(); k++){
            vAttrCodeSrc = vClassSrc->vriClassValueItem(k);
            if (vAttrCodeSrc->vrName == vAttrSrc->vrCalcSetFunction)
                break;
        }
        vClassDst = vAttrDst->vrClass();
        vAttrCodeDst = NULL;
        for (int k=0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrName == vAttrDst->vrCalcSetFunction)
                break;
        }
        if (vAttrCodeSrc != NULL && vAttrCodeDst != NULL)
            if (vAttrCodeSrc->vrCalcSetFunction != vAttrCodeDst->vrCalcSetFunction)
                result = result + "\n        Функция записи: изменена";

    } else {
        if (vAttrSrc->vrFunctionCode != vAttrDst->vrFunctionCode) {
            result = result + "\n        Код функции: изменён";
            result = result + "\n" +"\""+ from_bstr_t(vAttrSrc->vrFunctionCode)+"\"";
            result = result + "\n !=";
            result = result + "\n" +"\"" + from_bstr_t(vAttrDst->vrFunctionCode)+"\"";
        }

        if (vAttrSrc->vrCalcSetFunction != vAttrDst->vrCalcSetFunction)
            result = result + "\n        Функция записи: изменена";
    }

    if (vAttrSrc->vrBlocked != vAttrDst->vrBlocked)
        result = result + "\n        Блокировка: "
                + vAttrSrc->vrBlocked + " != "
                + vAttrDst->vrBlocked;

    if (vAttrSrc->vrMeasureUnit != vAttrDst->vrMeasureUnit)
        result = result + "\n        ЕИ: "
                + from_bstr_t(vAttrSrc->vrMeasureUnit) + " != "
                + from_bstr_t(vAttrDst->vrMeasureUnit);

    if (vAttrSrc->vrMeasureEntity != vAttrDst->vrMeasureEntity)
        result = result + "\n        ЕВ: "
                + from_bstr_t(vAttrSrc->vrMeasureEntity) + " != "
                + from_bstr_t(vAttrDst->vrMeasureEntity);

    if (vAttrSrc->vrForbidInput != vAttrDst->vrForbidInput)
        result = result + "\n        Ввод только из списка: "
                + vAttrSrc->vrForbidInput + " != "
                + vAttrDst->vrForbidInput;

    if (baseClassGuidSrc != baseClassGuidDst && sysFunc)
        result = result + "\n        Базовый класс: "
                + from_guid(baseClassGuidSrc) + " != "
                + from_guid(baseClassGuidDst);

    if (vAttrSrc->vrPrecision != vAttrDst->vrPrecision)
        result = result + "\n        Точность: изменена";

    if (vAttrSrc->vrGroup != vAttrDst->vrGroup)
        result = result + "\n        Группа: "
                + from_bstr_t(vAttrSrc->vrGroup) + " != "
                + from_bstr_t(vAttrDst->vrGroup);

    if (!result.isEmpty())
        result = "\n    "+ attrType +": " +  nameAttr + result;
    return result;
}

QString StructuresDifference::classDiff(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst) {

    QString result;
    if (vClassDst == NULL) {
        result =  result + "\nУдалён класс: "+ from_bstr_t(vClassSrc->vrName);
        return result;
    }

    result += this->classPermDiff(vClassSrc, vClassDst);

    if (vClassSrc->vrClassID != vClassDst->vrClassID) {
        result = result + "\n    Идентификатор: "
                + from_guid(vClassSrc->vrClassID) + "!="
                + from_guid(vClassDst->vrClassID);
    }

    if (vClassSrc->vrBlocked != vClassDst->vrBlocked)
        result = result + "\n    Блокировка: "
                + vClassSrc->vrBlocked + "!="
                + vClassDst->vrBlocked;

    if (vClassSrc->vrScreenName != vClassDst->vrScreenName)
        result = result + "\n    Экранноё имя: "
                + from_bstr_t(vClassSrc->vrScreenName) + "!="
                + from_bstr_t(vClassDst->vrScreenName);

    vkernelLib::IVClass *vBaseClassSrc = vClassSrc->vrBaseClass;
    vkernelLib::IVClass *vBaseClassDst = vClassDst->vrBaseClass;
    GUID baseClassGuidSrc = (vBaseClassSrc != NULL) ? vBaseClassSrc->vrClassID : GUID_NULL;
    GUID baseClassGuidDst = (vBaseClassDst != NULL) ? vBaseClassDst->vrClassID : GUID_NULL;
    if (baseClassGuidSrc != baseClassGuidDst)
        result = result + "\n        Базовый класс: "
                + from_guid(baseClassGuidSrc) + " != "
                + from_guid(baseClassGuidDst);

    result += childDiff(vClassSrc, vClassDst);

    for (int j=0; j<vClassSrc->vrClassValuesCount(); j++){
        vkernelLib::IVClassValue *vAttrSrc = vClassSrc->vriClassValueItem(j);
        for (int k=0; k<vClassDst->vrClassValuesCount(); k++){
            vkernelLib::IVClassValue *vAttrDst = vClassDst->vriClassValueItem(k);
            if (vAttrSrc->vrName == vAttrDst->vrName) {
                result += attrDiff(vAttrSrc, vAttrDst);
                break;
            } else if (k == vClassDst->vrClassValuesCount()-1)
                result += attrDiff(vAttrSrc, NULL);
        }
    }

    if (!result.isEmpty())
        result = "\n\nКласс: " +  from_bstr_t(vClassSrc->vrName) + result;
    return result;
}

QString StructuresDifference::childDiff(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
{
    QString result;
    for (int i=0; i < vClassSrc->vrChildsCount(); i++){
        vkernelLib::IVClass *vClassChildSrc = vClassSrc->vriChildClassItem(i);
        vkernelLib::IVClass *vClassChildDst = vClassDst->vrnChildClassItem(vClassChildSrc->vrName);
        if (vClassChildDst==NULL)
            result = result + "\n    Удалена связь: "
                    + from_bstr_t(vClassSrc->vrName) + " <- "
                    + from_bstr_t(vClassChildSrc->vrName);
    }
    return result;
}

QString StructuresDifference::modelDiff (vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {
    QString result;

    vkernelLib::IVClassVector *vClassVectorSrc = vModelSrc->vrGetClassVector();
    vkernelLib::IVClassVector *vClassVectorDst = vModelDst->vrGetClassVector();
    for (int i=0; i<vClassVectorSrc->vrCount(); i++){
        vkernelLib::IVClass *vClassSrc = vClassVectorSrc->vrItem(i);
        vkernelLib::IVClass *vClassDst = vClassVectorDst->vrLocate(vClassSrc->vrName);
        result += classDiff(vClassSrc, vClassDst);
    }
    result += filterDiff(vModelSrc, vModelDst);
    return result;
}

QString StructuresDifference::filterDiff (vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {
    QString result;

    vkernelLib::IVClassVector *vClassVectorSrc = vModelSrc->vrGetClassVector();
    vkernelLib::IVClassVector *vClassVectorDst = vModelDst->vrGetClassVector();

    vkernelLib::IVFilterVector *vFilterVectorSrc = vClassVectorSrc->vrFilterVector();
    vkernelLib::IVFilterVector *vFilterVectorDst = vClassVectorDst->vrFilterVector();

    for (int i=0; i<vFilterVectorSrc->vrFiltersCount(); i++){
        _bstr_t filterSrc = vFilterVectorSrc->vrFilterName(i);

        if (vFilterVectorDst->vrLocateFilter(filterSrc)) {
            QString filterResult("");
            for (int j = 0; j < vFilterVectorSrc->vrConstrainsCount(filterSrc); j++) {
                _bstr_t classSrc = vFilterVectorSrc->vrConstrains(filterSrc,j);
                if (!(vFilterVectorDst->vrLocateConstraint(filterSrc, classSrc)))
                    filterResult =  filterResult + "\n    Удалён класс: "+ from_bstr_t(classSrc);
            }
            if (!filterResult.isEmpty())
                result = result + "\n\nФильтр: " +  from_bstr_t(filterSrc);
            result += filterResult;
        } else {
            result =  result + "\nУдалён фильтр: "+ from_bstr_t(filterSrc);
        }

    }
    return result;
}
