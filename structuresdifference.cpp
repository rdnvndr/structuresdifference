#include "structuresdifference.h"
#include "converttype.h"
#include <QString>
#include <QDebug>
#include <QFile>

const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

const GUID ID_INT    = to_guid("{DDF29044-F2DA-4457-9CA2-CA0F1E6501A6}");
const GUID ID_FLT    = to_guid("{51AF1A06-C000-4ed4-940D-ADD3B199EB5E}");
const GUID ID_STR    = to_guid("{751A3E7F-86DC-4527-91C8-79919F3B3FA3}");
const GUID ID_TEXT   = to_guid("{B7672ACF-D5AA-4432-AA5C-ACF9FF8A27F7}");
const GUID ID_BOOL   = to_guid("{1739DAD4-D175-4b0a-B2C7-55759916F7EF}");
const GUID ID_DATE   = to_guid("{BDB20E23-B5D7-4966-8110-6F4EFD8764BB}");
const GUID ID_NULL   = to_guid("{00000000-0000-0000-0000-000000000000}");
const GUID ID_FILE   = to_guid("{2B02B52B-7C4A-40a7-845A-DA553A1B0DA1}");
const GUID ID_STREAM = to_guid("{F0CA433A-74E7-47b0-958C-CC1EBAC73DA8}");

const GUID ID_DIM             = to_guid("{428CC298-47D3-4D09-8B7C-E6F36518C724}");
const GUID ID_ROUGHNESS       = to_guid("{B6C89FE7-2B27-492C-962E-9940365E6201}");
const GUID ID_ANGLE           = to_guid("{2FBF9A81-03B2-4AE8-BF46-3BE7240E7722}");
const GUID ID_MARKER          = to_guid("{068E304F-E0C5-4B75-8785-B893CD910740}");
const GUID ID_VARIANT         = to_guid("{EA1481CE-A241-47DB-BAB8-77EA17C51239}");
const GUID ID_THREAD          = to_guid("{627AE639-7DBE-4420-880E-A1373F0CA4AF}");
const GUID ID_OBJECTSET       = to_guid("{355A97C0-1066-406D-BDEE-46BE1C72EB4E}");
const GUID ID_FORMTOLERANCE   = to_guid("{878CFC01-4EBD-4189-A0F8-A25C087394EA}");
const GUID ID_DEFLECTEDDOUBLE = to_guid("{34F029CE-71B9-433D-BD0C-A888750D1884}");

const int classPermMask = vkernelLib::SF_LCK
                        | vkernelLib::SF_WRT
                        | vkernelLib::SF_VSB
                        | vkernelLib::SF_EXE
                        | vkernelLib::SF_DEL
                        | vkernelLib::SF_CRT;

const int smplAttrPermMask = vkernelLib::SF_WRT
                           | vkernelLib::SF_VSB;

const int calcAttrPermMask = vkernelLib::SF_WRT
                           | vkernelLib::SF_VSB
                           | vkernelLib::SF_EXE;

const int funcAttrPermMask = vkernelLib::SF_EXE;

QString dataTypeGuidToString(GUID guid) {

    if (guid == ID_INT)
        return QString("Integer");
    else if (guid == ID_FLT)
        return QString("Float");
    else if (guid == ID_STR)
        return QString("String");
    else if (guid == ID_TEXT)
        return QString("Text");
    else if (guid == ID_BOOL)
        return QString("Boolean");
    else if (guid == ID_DATE)
        return QString("Date");
    else if (guid == ID_NULL)
        return QString("NULL");
    else if (guid == ID_FILE)
        return QString("File");
    else if (guid == ID_STREAM)
        return QString("Stream");
    else if (guid == ID_DIM)
        return QString("Dimension");
    else if (guid == ID_ROUGHNESS)
        return QString("Roughness");
    else if (guid == ID_ANGLE)
        return QString("Angle");
    else if (guid == ID_MARKER)
        return QString("Marker");
    else if (guid == ID_VARIANT)
        return QString("Variant");
    else if (guid == ID_THREAD)
        return QString("Thread");
    else if (guid == ID_OBJECTSET)
        return QString("ObjectSet");
    else if (guid == ID_FORMTOLERANCE)
        return QString("FormTolerance");
    else if (guid == ID_DEFLECTEDDOUBLE)
        return QString("DeflectedDouble");

    return QString("Неизвестный тип данных");
}

QString typeAttrToString(int typeAttr) {
    switch (typeAttr) {
        case 0:  return QString("Простой");
        case 1:  return QString("Функция");
        case 2:  return QString("Вычисляемый");
        default: return QString("Неизвестный тип");
    }
}

StructuresDifference::StructuresDifference(QObject *parent) : QObject(parent)
{
    ::CoInitialize(NULL);
    ::CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                           RPC_C_IMP_LEVEL_DELEGATE, NULL, 0, NULL);
    uniRef = NULL;

    m_classBaseClass  = true;
    m_classBlocked    = true;
    m_classChild      = true;
    m_classFilter     = true;
    m_classPerms      = true;
    m_classScreenName = true;
    m_permGroup       = true;
    m_classId         = true;

    m_attrId            = true;
    m_attrDataType      = true;
    m_attrType          = true;
    m_attrScreenName    = true;
    m_attrAliasName     = true;
    m_attrFuncRead      = true;
    m_attrFuncWrite     = true;
    m_attrBlocked       = true;
    m_attrMeasureUnit   = true;
    m_attrMeasureEntity = true;
    m_attrForbidInput   = true;
    m_attrLimit         = true;
    m_attrBaseClass     = true;
    m_attrPrecision     = true;
    m_attrGroup         = true;
    m_attrProp          = true;
    m_attrPerms         = true;
    m_attrInherited     = true;

    m_objId         = true;
    m_objOwnerId    = true;
    m_objWasChanged = true;
    m_objReadOnly   = true;
    m_objChilds     = true;

    m_objAttrName        = true;
    m_objAttrValue       = true;
    m_objAttrMeasureUnit = true;
    m_objAttrPrecision   = true;
    m_objAttrOwnerId     = true;
}

bool StructuresDifference::connect()
{
    HRESULT Hr = ::CoCreateInstance(__uuidof(UniReference::UniRefer),
                                    NULL, CLSCTX_INPROC_SERVER,
                                    __uuidof(UniReference::IUniRefer),
                                    (void**)&uniRef);
    if (Hr == S_OK) {
        uniRef->GlobalVars()->Logon()->LogonAsParams(
                    to_bstr_t("Администратор"),
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
                                    __uuidof(vkernelLib::IVModel),
                                    (void**)&vModel);
     if (Hr == S_OK) {
         vModel->vrLoadModel(to_bstr_t(filename),
                             NULL, vkernelLib::OPEN_FULL_STR_LOCAL);
         vModel->vrApplySecurity();
         return vModel;
     } else
         return NULL;
}

QString StructuresDifference::differenceAttrGroups(
        vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst)
{
    QString result;

    m_scrtSrc = vModelSrc->vrGetLocalSecurity();
    m_scrtDst = vModelDst->vrGetLocalSecurity();
    long countGroupUser =
            uniRef->GlobalVars()->Logon()->ListGroupsUser()->CountGroupsUser;
    for (int i= 0; i < countGroupUser; ++i)
    {
        UniReference::IGroupUserComPtr group
             = uniRef->GlobalVars()->Logon()->ListGroupsUser()->GroupByIndex(i);
        _bstr_t groupGuid = group->GuidGroup;
        _bstr_t groupName = group->NameGroup;
        unsigned long groupIdSrc = m_scrtSrc->vlsGetItemByGUID(groupGuid);
        unsigned long groupIdDst = m_scrtDst->vlsGetItemByGUID(groupGuid);

        if (groupIdSrc == 1 || groupIdDst == 0) {
            if (!m_permGroup)
                result += "\nУдалена группа: "
                        + groupName + "("
                        + groupGuid + ")";
            continue;
        }

        if (groupIdSrc == 0 || groupIdDst == 1) {

            if (!m_permGroup)
                result += "\nДобавлена группа: "
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

QString StructuresDifference::differenceClassPerms(
        vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
{
    if (!m_classPerms)
        return "";

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
                   + QString("\n        %1 !=  %2 - %3")
                    .arg(rightsSrc,2)
                    .arg(rightsDst,2)
                    .arg(m_groupNames.at(i));
         }
     }
     if (!result.isEmpty())
         result = "\n    Права доступа изменены:" + result;

     return result;
}

QString StructuresDifference::addingClassPerms(vkernelLib::IVClass *vClassDst)
{
    if (!m_classPerms)
        return "";

    QString result;
    vkernelLib::LDIGEST digestDst = {{0,0,0,0}};
    int rightsDst;

    for (int i=0; i < m_groupsDst.count(); i++) {
        vClassDst->vrGetDigest(&(digestDst.data[0]), &(digestDst.data[1]),
                               &(digestDst.data[2]), &(digestDst.data[3]));
        m_scrtDst->vlsGetRights(m_groupsDst.at(i), &digestDst, &rightsDst);
        rightsDst = rightsDst & classPermMask;

        result = result
                + QString("\n        %1 - %2")
                .arg(rightsDst,2)
                .arg(m_groupNames.at(i));
    }
    if (!result.isEmpty())
        result = "\n    Права доступа:" + result;

    return result;
}

QString StructuresDifference::differenceAttrPerms(
        vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
{
    if (!m_attrPerms)
        return "";

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
                   + QString("\n            %1 !=  %2 - %3")
                   .arg(rightsSrc,2)
                   .arg(rightsDst,2)
                   .arg(m_groupNames.at(i));
        }
    }
    if (!result.isEmpty())
        result = "\n        Права доступа изменены:" + result;

    return result;
}

QString StructuresDifference::addingAttrPerms(vkernelLib::IVClassValue *vAttrDst)
{
    if (!m_attrPerms)
        return "";

    int attrPermMask;
    switch (vAttrDst->vrType) {
        case 0: attrPermMask = smplAttrPermMask;
                break;
        case 1: attrPermMask = funcAttrPermMask;
            break;
        case 2: attrPermMask = calcAttrPermMask;
            break;
    }

    QString result;
    vkernelLib::LDIGEST digestDst = {{0,0,0,0}};

    int rightsDst;

    for (int i=0; i < m_groupsDst.count(); i++) {
        vAttrDst->vrGetDigest(&(digestDst.data[0]), &(digestDst.data[1]),
                              &(digestDst.data[2]), &(digestDst.data[3]));

        m_scrtDst->vlsGetRights(m_groupsDst.at(i), &digestDst, &rightsDst);

        rightsDst = rightsDst & attrPermMask;
        result = result
                + QString("\n            %1 - %2")
                .arg(rightsDst,2)
                .arg(m_groupNames.at(i));
    }
    if (!result.isEmpty())
        result = "\n        Права доступа:" + result;

    return result;
}

QString StructuresDifference::differenceObjects(
        vkernelLib::IVObject *vObjectSrc,  vkernelLib::IVObject *vObjectDst)
{
    QString result = "";
    if (vObjectDst == NULL)
    {
        result += "\n\nУдалён объект класса \""
                + from_bstr_t(vObjectSrc->vrClass()->vrName)
                + "\": "
                + from_bstr_t(vObjectSrc->vrObjStrID());
        return result;
    }

    if (vObjectSrc->vrObjStrID() != vObjectDst->vrObjStrID() && m_objId) {
        result += "\n    Идентификатор: "
                + from_bstr_t(vObjectSrc->vrObjStrID())
                + " != "
                + from_bstr_t(vObjectDst->vrObjStrID());
    }
    if (vObjectSrc->vrOwnerID != vObjectDst->vrOwnerID && m_objOwnerId) {
        result += "\n    Владелец: "
                + from_bstr_t(vObjectSrc->vrOwnerID)
                + " != "
                + from_bstr_t(vObjectDst->vrOwnerID);
    }
    if (vObjectSrc->vrWasChanged != vObjectDst->vrWasChanged && m_objWasChanged)
        result += "\n    Возможность изменения: изменено";
    if (vObjectSrc->vrReadOnly != vObjectDst->vrReadOnly && m_objReadOnly) {
        result += QString("\n    Только чтение: %1 != %2")
                .arg(vObjectSrc->vrReadOnly)
                .arg(vObjectDst->vrReadOnly);
    }

    result += differenceObjectLinks(vObjectSrc, vObjectDst);

    if (m_objAttrName || m_objAttrValue || m_objAttrMeasureUnit
            || m_objAttrPrecision || m_objAttrOwnerId)
    {
        for (int i = 0; i < vObjectSrc->vrAttrCount(); i++) {
            vkernelLib::IVAttribute *attrSrc = vObjectSrc->vrAttrByIndex(i);
            for (int j = 0; j < vObjectDst->vrAttrCount(); j++) {
                vkernelLib::IVAttribute *attrDst = vObjectDst->vrAttrByIndex(j);
                if (attrSrc->vrName == attrDst->vrName) {
                    result += differenceAttrObjects(attrSrc, attrDst);
                    break;
                }
                if (j == vObjectDst->vrAttrCount()-1)
                    result = "\n    Удалён атрибут: "
                            + from_bstr_t(attrSrc->vrName);
            }
        }
    }

    if (!result.isEmpty()) {
        result = "\n\nОбъект класса \""
                + from_bstr_t(vObjectSrc->vrClass()->vrName)
                + "\": "
                + from_bstr_t(vObjectSrc->vrObjStrID())
                + result;
    }

    return result;
}

QString StructuresDifference::addingObjects(
        vkernelLib::IVObject *vObjectSrc, vkernelLib::IVObject *vObjectDst)
{
    if (!m_objAttrName || vObjectSrc == NULL)
        return "";

    QString result = "";

    result += "\n\nДобавлен объект класса \""
            + from_bstr_t(vObjectDst->vrClass()->vrName)
            + "\": ";
    result += "\n    Идентификатор: "
            + from_bstr_t(vObjectDst->vrObjStrID());
    result += "\n    Владелец: "
            + from_bstr_t(vObjectDst->vrOwnerID);

    result += addingObjectLinks(vObjectDst);

    if (m_objAttrName || m_objAttrValue || m_objAttrMeasureUnit
            || m_objAttrPrecision || m_objAttrOwnerId)
    {
        for (int i = 0; i < vObjectDst->vrAttrCount(); i++) {
            vkernelLib::IVAttribute *attrDst = vObjectDst->vrAttrByIndex(i);
            for (int j = 0; j < vObjectSrc->vrAttrCount(); j++) {
                vkernelLib::IVAttribute *attrSrc = vObjectSrc->vrAttrByIndex(j);
                if (attrSrc->vrName == attrDst->vrName) {
                    break;
                }
                if (j==vObjectSrc->vrAttrCount()-1)
                    result = "\n    Добавлен атрибут: "
                            + from_bstr_t(attrDst->vrName);
            }
        }
    }

    return result;
}

QString StructuresDifference::differenceObjectLinks(
        vkernelLib::IVObject *vObjectSrc ,vkernelLib::IVObject *vObjectDst)
{
    if (!m_objChilds)
        return "";

    QString result;
    vkernelLib::IVObjectVector *vObjsSrc = vObjectSrc->vrObjectsVector();
    vkernelLib::IVObjectVector *vObjsDst = vObjectDst->vrObjectsVector();

    vkernelLib::IVIterator *vIterSrc;
    vkernelLib::IVIterator *vIterDst;
    if (vObjsSrc->raw_vrCreateIterator(L"", vObjectSrc, true, &vIterSrc)!=S_OK)
        return result;
    if (vObjsDst->raw_vrCreateIterator(L"", vObjectDst, true, &vIterDst)!=S_OK)
        return "\n    Дочерние объекты удалены";


    for (bool flagSrc = vIterSrc->vrFirst();
         flagSrc == true; flagSrc = vIterSrc->vrNext())
    {
        vkernelLib::IVObject *vChildObjectSrc =  vIterSrc->vrGetObject();
        bool noneChild = true;
        for (bool flagDst = vIterDst->vrFirst();
             flagDst == true; flagDst = vIterDst->vrNext())
        {
            vkernelLib::IVObject *vChildObjectDst =  vIterDst->vrGetObject();
            if (vChildObjectSrc->vrObjStrID()==vChildObjectDst->vrObjStrID()) {
                noneChild = false;
                break;
            }
        }
        if (noneChild)
              result = result + "\n    Удален дочерний объект класса \""
                      + from_bstr_t(vObjectSrc->vrClass()->vrName)
                      + "\": "
                      + from_bstr_t(vChildObjectSrc->vrObjStrID());
    }

    return result;
}

QString StructuresDifference::addingObjectLinks(vkernelLib::IVObject *vObjectDst)
{
    if (!m_objChilds)
        return "";

    QString result;
    vkernelLib::IVObjectVector *vObjsDst = vObjectDst->vrObjectsVector();

    vkernelLib::IVIterator *vIterDst;
    if (vObjsDst->raw_vrCreateIterator(L"", vObjectDst, true, &vIterDst)!=S_OK)
        return "";

    for (bool flagDst = vIterDst->vrFirst();
         flagDst == true; flagDst = vIterDst->vrNext())
    {
        vkernelLib::IVObject *vChildObjectDst =  vIterDst->vrGetObject();
        result = result + "\n    Дочерний объект класса \""
                + from_bstr_t(vChildObjectDst->vrClass()->vrName)
                + "\": "
                + from_bstr_t(vChildObjectDst->vrObjStrID());
    }

    return result;
}

bool StructuresDifference::differenceIDispatchs(
        _variant_t varSrc, _variant_t varDst, GUID dataType)
{
    if (dataType == ID_DIM) {
        vkernel_aLib::ISize *iObjSrc
                = from_vdispatch<vkernel_aLib::ISize>(varSrc);
        vkernel_aLib::ISize *iObjDst
                = from_vdispatch<vkernel_aLib::ISize>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrValue           != iObjDst->vrValue
              || iObjSrc->vrBraces         != iObjDst->vrBraces
              || iObjSrc->vrEI             != iObjSrc->vrEI
              || iObjSrc->vrES             != iObjSrc->vrES
              || iObjSrc->vrFormType       != iObjDst->vrFormType
              || iObjSrc->vrFrame          != iObjDst->vrFrame
              || iObjSrc->vrQuality        != iObjSrc->vrQuality
              || iObjSrc->vrQualityNumber  != iObjSrc->vrQualityNumber
              || iObjSrc->vrQualityShow    != iObjDst->vrQualityShow
              || iObjSrc->vrQualityType    != iObjDst->vrQualityType
              || iObjSrc->vrShowDeflection != iObjSrc->vrShowDeflection
              || iObjSrc->vrSpecialSymbol  != iObjSrc->vrSpecialSymbol
              || iObjSrc->vrTextAfter      != iObjSrc->vrTextAfter
              || iObjSrc->vrTextBefore     != iObjSrc->vrTextBefore
              || iObjSrc->vrTextValue      != iObjSrc->vrTextValue
              || iObjSrc->vrUnderline      != iObjSrc->vrUnderline
            )
                return true;
        } else
            return true;
    }

    if (dataType == ID_ROUGHNESS) {
        vkernel_aLib::IRoughness *iObjSrc
                = from_vdispatch<vkernel_aLib::IRoughness>(varSrc);
        vkernel_aLib::IRoughness *iObjDst
                = from_vdispatch<vkernel_aLib::IRoughness>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrRA != iObjDst->vrRA
                     || iObjSrc->vrRoughClass != iObjDst->vrRoughClass
                     || iObjSrc->vrRZ         != iObjDst->vrRZ
                     || iObjSrc->vrTextValue  != iObjDst->vrTextValue
                     || iObjSrc->vrUseRA      != iObjDst->vrUseRA
             )
                  return true;
        } else
            return true;
    }

    if (dataType == ID_ANGLE) {
        vkernel_aLib::IAngle *iObjSrc
                = from_vdispatch<vkernel_aLib::IAngle>(varSrc);
        vkernel_aLib::IAngle *iObjDst
                = from_vdispatch<vkernel_aLib::IAngle>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrBraces                != iObjDst->vrBraces
                     || iObjSrc->vrEI             != iObjDst->vrEI
                     || iObjSrc->vrES             != iObjDst->vrES
                     || iObjSrc->vrFrame          != iObjDst->vrFrame
                     || iObjSrc->vrShowDeflection != iObjDst->vrShowDeflection
                     || iObjSrc->vrShowType       != iObjDst->vrShowType
                     || iObjSrc->vrTextAfter      != iObjDst->vrTextAfter
                     || iObjSrc->vrTextBefore     != iObjDst->vrTextBefore
                     || iObjSrc->vrTextValue      != iObjDst->vrTextValue
                     || iObjSrc->vrUnderline      != iObjDst->vrUnderline
                     || iObjSrc->vrValue          != iObjDst->vrValue
             )
                  return true;
        } else
            return true;
    }

    if (dataType == ID_MARKER) {
        vkernel_aLib::IMarker *iObjSrc
                = from_vdispatch<vkernel_aLib::IMarker>(varSrc);
        vkernel_aLib::IMarker *iObjDst
                = from_vdispatch<vkernel_aLib::IMarker>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrTextValue != iObjDst->vrTextValue
                 || iObjSrc->vrValue != iObjDst->vrValue
            )
                 return true;
        } else
            return true;

    }

    if (dataType == ID_VARIANT) {
        vkernelLib::IVariant *iObjSrc
                = from_vdispatch<vkernelLib::IVariant>(varSrc);
        vkernelLib::IVariant *iObjDst
                = from_vdispatch<vkernelLib::IVariant>(varDst);
        if (iObjSrc && iObjDst) {
            switch (iObjSrc->vrDataType) {
            case vkernelLib::VARIANTDATATYPE::VRD_BOOL:
                if (iObjSrc->vrBool != iObjDst->vrBool)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_DATE:
                if (iObjSrc->vrDate != iObjDst->vrDate)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_FLT:
                if (iObjSrc->vrFloat != iObjDst->vrFloat)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_INT:
                if (iObjSrc->vrInteger != iObjDst->vrInteger)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_INTERFACE:
                //iObjSrc->vrObject
                return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_STR:
                if (iObjSrc->vrString != iObjDst->vrString)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_TEXT:
                if (iObjSrc->vrString != iObjDst->vrString)
                    return true;
                break;
            case vkernelLib::VARIANTDATATYPE::VRD_VARIANT:
            default:
                return differenceIDispatchs(
                            iObjSrc->vrVariant,
                            iObjDst->vrVariant,
                            iObjSrc->vrClassID);
            }

        } else
            return true;
    }

    if (dataType == ID_THREAD) {
        vkernel_aLib::IThread *iObjSrc
                = from_vdispatch<vkernel_aLib::IThread>(varSrc);
        vkernel_aLib::IThread *iObjDst
                = from_vdispatch<vkernel_aLib::IThread>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrCountStart            != iObjDst->vrCountStart
                     || iObjSrc->vrDExt           != iObjDst->vrDExt
                     || iObjSrc->vrDExtEi         != iObjDst->vrDExtEi
                     || iObjSrc->vrDExtEs         != iObjDst->vrDExtEs
                     || iObjSrc->vrDInt           != iObjDst->vrDInt
                     || iObjSrc->vrDIntEi         != iObjDst->vrDIntEi
                     || iObjSrc->vrDIntEs         != iObjDst->vrDIntEs
                     || iObjSrc->vrDMiddle        != iObjDst->vrDMiddle
                     || iObjSrc->vrDMiddleEi      != iObjDst->vrDMiddleEi
                     || iObjSrc->vrDMiddleEs      != iObjDst->vrDMiddleEs
                     || iObjSrc->vrLengthThread   != iObjDst->vrLengthThread
                     || iObjSrc->vrLengthwise     != iObjDst->vrLengthwise
                     || iObjSrc->vrOboznD         != iObjDst->vrOboznD
                     || iObjSrc->vrShiftBasePlane != iObjDst->vrShiftBasePlane
                     || iObjSrc->vrStep           != iObjDst->vrStep
                     || iObjSrc->vrTextValue      != iObjDst->vrTextValue
                     || iObjSrc->vrTolerance      != iObjDst->vrTolerance
                     || iObjSrc->vrType           != iObjDst->vrType
                     || iObjSrc->vrTypeForm       != iObjDst->vrTypeForm
                     || iObjSrc->vrTypeLR         != iObjDst->vrTypeLR
                     || iObjSrc->vrTypeStep       != iObjDst->vrTypeStep
                     || iObjSrc->vrValue          != iObjDst->vrValue
                     || iObjSrc->vrLengthwiseNormally
                             != iObjDst->vrLengthwiseNormally
                     || iObjSrc->vrLengthwiseSmallerThread
                             != iObjDst->vrLengthwiseSmallerThread
                     || iObjSrc->vrToleranceShiftBasePlane
                             != iObjDst->vrToleranceShiftBasePlane
             )
                  return true;
        } else return true;
    }

    if (dataType == ID_OBJECTSET) {
        vkernel_aLib::IObjectSet *iObjSrc
                = from_vdispatch<vkernel_aLib::IObjectSet>(varSrc);
        vkernel_aLib::IObjectSet *iObjDst
                = from_vdispatch<vkernel_aLib::IObjectSet>(varDst);
        if (iObjSrc && iObjDst) {
            for (int i = 0; i < iObjSrc->vrSize(); i++) {
                _bstr_t guidSrc = iObjSrc->vrObjectStr(i);
                for (int j = 0; j < iObjDst->vrSize(); j++) {
                    _bstr_t guidDst = iObjDst->vrObjectStr(j);
                    if (guidSrc == guidDst)
                        break;
                    if (j == iObjDst->vrSize()-1)
                        return true;
                }
            }
        } else
            return true;
    }

    if (dataType == ID_FORMTOLERANCE) {
        vkernel_aLib::IFormTolerance *iObjSrc
                = from_vdispatch<vkernel_aLib::IFormTolerance>(varSrc);
        vkernel_aLib::IFormTolerance *iObjDst
                = from_vdispatch<vkernel_aLib::IFormTolerance>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrBase1              != iObjDst->vrBase1
                     || iObjSrc->vrBase2       != iObjDst->vrBase2
                     || iObjSrc->vrBaseCode1   != iObjDst->vrBaseCode1
                     || iObjSrc->vrBaseCode2   != iObjDst->vrBaseCode2
                     || iObjSrc->vrLengthwise  != iObjDst->vrLengthwise
                     || iObjSrc->vrSymbol      != iObjDst->vrSymbol
                     || iObjSrc->vrSymbolAdded != iObjDst->vrSymbolAdded
                     || iObjSrc->vrTextAfter   != iObjDst->vrTextAfter
                     || iObjSrc->vrTextBefore  != iObjDst->vrTextBefore
                     || iObjSrc->vrTextValue   != iObjDst->vrTextValue
                     || iObjSrc->vrType        != iObjDst->vrType
                     || iObjSrc->vrTypeAdded   != iObjDst->vrTypeAdded
                     || iObjSrc->vrValue       != iObjDst->vrValue
             )
                  return true;
        } else
            return true;
    }

    if (dataType == ID_DEFLECTEDDOUBLE) {
        vkernel_aLib::IDeflectedDouble *iObjSrc
                = from_vdispatch<vkernel_aLib::IDeflectedDouble>(varSrc);
        vkernel_aLib::IDeflectedDouble *iObjDst
                = from_vdispatch<vkernel_aLib::IDeflectedDouble>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrShowDeflection   != iObjDst->vrShowDeflection
                    || iObjSrc->vrEI        != iObjDst->vrEI
                    || iObjSrc->vrES        != iObjDst->vrES
                    || iObjSrc->vrTextValue != iObjDst->vrTextValue
                    || iObjSrc->vrValue     != iObjDst->vrValue
            )
                 return true;
        } else
            return true;
    }

    if (dataType == ID_FILE) {
        vkernelLib::IVFile *iObjSrc
                = from_vdispatch<vkernelLib::IVFile>(varSrc);
        vkernelLib::IVFile *iObjDst
                = from_vdispatch<vkernelLib::IVFile>(varDst);

        if (iObjSrc && iObjDst) {
            bool openedSrc = false;
            bool openedDst = false;

            QString filenameSrc(from_bstr_t(iObjSrc->vsInternalFullName));
            QString filenameDst(from_bstr_t(iObjDst->vsInternalFullName));
            QFile fileSrc(filenameSrc);
            QFile fileDst(filenameDst);

            if (fileSrc.exists()) openedSrc = fileSrc.open(QIODevice::ReadOnly);
            if (fileDst.exists()) openedDst = fileDst.open(QIODevice::ReadOnly);
            if (openedSrc) {
                if (openedDst) {
                    while (!fileSrc.atEnd() && !fileDst.atEnd()) {
                        QByteArray arrSrc = fileSrc.read(1024000);
                        QByteArray arrDst = fileDst.read(1024000);
                        if (arrSrc!=arrDst) {
                            fileSrc.close();
                            fileDst.close();
                            return true;
                        }
                    }
                    fileDst.close();
                } else {
                    fileSrc.close();
                    return true;
                }
                fileSrc.close();
            } else if (openedDst) {
                fileDst.close();
                return true;
            }
        }
    }

    return false;
}

bool StructuresDifference::classId() const
{
    return m_classId;
}

void StructuresDifference::setClassId(bool classId)
{
    m_classId = classId;
}

bool StructuresDifference::classBlocked() const
{
    return m_classBlocked;
}

void StructuresDifference::setClassBlocked(bool classBlocked)
{
    m_classBlocked = classBlocked;
}

bool StructuresDifference::classScreenName() const
{
    return m_classScreenName;
}

void StructuresDifference::setClassScreenName(bool classScreenName)
{
    m_classScreenName = classScreenName;
}

bool StructuresDifference::classBaseClass() const
{
    return m_classBaseClass;
}

void StructuresDifference::setClassBaseClass(bool classBaseClass)
{
    m_classBaseClass = classBaseClass;
}

bool StructuresDifference::attrId() const
{
    return m_attrId;
}

void StructuresDifference::setAttrId(bool attrId)
{
    m_attrId = attrId;
}

bool StructuresDifference::attrDataType() const
{
    return m_attrDataType;
}

void StructuresDifference::setAttrDataType(bool attrDataType)
{
    m_attrDataType = attrDataType;
}

bool StructuresDifference::attrType() const
{
    return m_attrType;
}

void StructuresDifference::setAttrType(bool attrType)
{
    m_attrType = attrType;
}

bool StructuresDifference::attrScreenName() const
{
    return m_attrScreenName;
}

void StructuresDifference::setAttrScreenName(bool attrScreenName)
{
    m_attrScreenName = attrScreenName;
}

bool StructuresDifference::attrAliasName() const
{
    return m_attrAliasName;
}

void StructuresDifference::setAttrAliasName(bool attrAliasName)
{
    m_attrAliasName = attrAliasName;
}

bool StructuresDifference::attrFuncRead() const
{
    return m_attrFuncRead;
}

void StructuresDifference::setAttrFuncRead(bool attrFuncRead)
{
    m_attrFuncRead = attrFuncRead;
}

bool StructuresDifference::attrFuncWrite() const
{
    return m_attrFuncWrite;
}

void StructuresDifference::setAttrFuncWrite(bool attrFuncWrite)
{
    m_attrFuncWrite = attrFuncWrite;
}

bool StructuresDifference::attrBlocked() const
{
    return m_attrBlocked;
}

void StructuresDifference::setAttrBlocked(bool attrBlocked)
{
    m_attrBlocked = attrBlocked;
}

bool StructuresDifference::attrMeasureUnit() const
{
    return m_attrMeasureUnit;
}

void StructuresDifference::setAttrMeasureUnit(bool attrMeasureUnit)
{
    m_attrMeasureUnit = attrMeasureUnit;
}

bool StructuresDifference::attrMeasureEntity() const
{
    return m_attrMeasureEntity;
}

void StructuresDifference::setAttrMeasureEntity(bool attrMeasureEntity)
{
    m_attrMeasureEntity = attrMeasureEntity;
}

bool StructuresDifference::attrForbidInput() const
{
    return m_attrForbidInput;
}

void StructuresDifference::setAttrForbidInput(bool attrForbidInput)
{
    m_attrForbidInput = attrForbidInput;
}

bool StructuresDifference::attrBaseClass() const
{
    return m_attrBaseClass;
}

void StructuresDifference::setAttrBaseClass(bool attrBaseClass)
{
    m_attrBaseClass = attrBaseClass;
}

bool StructuresDifference::attrPrecision() const
{
    return m_attrPrecision;
}

void StructuresDifference::setAttrPrecision(bool attrPrecision)
{
    m_attrPrecision = attrPrecision;
}

bool StructuresDifference::attrGroup() const
{
    return m_attrGroup;
}

void StructuresDifference::setAttrGroup(bool attrGroup)
{
    m_attrGroup = attrGroup;
}

bool StructuresDifference::classChild() const
{
    return m_classChild;
}

void StructuresDifference::setClassChild(bool classChild)
{
    m_classChild = classChild;
}

bool StructuresDifference::classFilter() const
{
    return m_classFilter;
}

void StructuresDifference::setClassFilter(bool classFilter)
{
    m_classFilter = classFilter;
}

bool StructuresDifference::classPerms() const
{
    return m_classPerms;
}

void StructuresDifference::setClassPerms(bool classPerms)
{
    m_classPerms = classPerms;
}

bool StructuresDifference::attrProp() const
{
    return m_attrProp;
}

void StructuresDifference::setAttrProp(bool attrProp)
{
    m_attrProp = attrProp;
}

bool StructuresDifference::attrPerms() const
{
    return m_attrPerms;
}

void StructuresDifference::setAttrPerms(bool attrPerms)
{
    m_attrPerms = attrPerms;
}

bool StructuresDifference::permGroup() const
{
    return m_permGroup;
}

void StructuresDifference::setPermGroup(bool permGroup)
{
    m_permGroup = permGroup;
}

bool StructuresDifference::objId() const
{
    return m_objId;
}

void StructuresDifference::setObjId(bool objId)
{
    m_objId = objId;
}

bool StructuresDifference::objOwnerId() const
{
    return m_objOwnerId;
}

void StructuresDifference::setObjOwnerId(bool objOwnerId)
{
    m_objOwnerId = objOwnerId;
}

bool StructuresDifference::objWasChanged() const
{
    return m_objWasChanged;
}

void StructuresDifference::setObjWasChanged(bool objWasChanged)
{
    m_objWasChanged = objWasChanged;
}

bool StructuresDifference::objReadOnly() const
{
    return m_objReadOnly;
}

void StructuresDifference::setObjReadOnly(bool objReadOnly)
{
    m_objReadOnly = objReadOnly;
}

bool StructuresDifference::objAttrName() const
{
    return m_objAttrName;
}

void StructuresDifference::setObjAttrName(bool objAttrName)
{
    m_objAttrName = objAttrName;
}

bool StructuresDifference::objAttrValue() const
{
    return m_objAttrValue;
}

void StructuresDifference::setObjAttrValue(bool objAttrValue)
{
    m_objAttrValue = objAttrValue;
}

bool StructuresDifference::objAttrMeasureUnit() const
{
    return m_objAttrMeasureUnit;
}

void StructuresDifference::setObjAttrMeasureUnit(bool objAttrMeasureUnit)
{
    m_objAttrMeasureUnit = objAttrMeasureUnit;
}

bool StructuresDifference::objAttrPrecision() const
{
    return m_objAttrPrecision;
}

void StructuresDifference::setObjAttrPrecision(bool objAttrPrecision)
{
    m_objAttrPrecision = objAttrPrecision;
}

bool StructuresDifference::objAttrOwnerId() const
{
    return m_objAttrOwnerId;
}

void StructuresDifference::setObjAttrOwnerId(bool objAttrOwnerID)
{
    m_objAttrOwnerId = objAttrOwnerID;
}

bool StructuresDifference::objChilds() const
{
    return m_objChilds;
}

void StructuresDifference::setObjChilds(bool objChilds)
{
    m_objChilds = objChilds;
}

bool StructuresDifference::attrLimit() const
{
    return m_attrLimit;
}

void StructuresDifference::setAttrLimit(bool attrLimit)
{
    m_attrLimit = attrLimit;
}

bool StructuresDifference::attrInherited() const
{
    return m_attrInherited;
}

void StructuresDifference::setAttrInherited(bool attrInherited)
{
    m_attrInherited = attrInherited;
}

QString StructuresDifference::differenceAttrObjects(
        vkernelLib::IVAttribute *attrSrc,vkernelLib::IVAttribute *attrDst)
{
    QString result;

    if (attrDst==NULL) {
        result = "\n    Удалён атрибут: " + from_bstr_t(attrSrc->vrName);
        return result;
    }
    if (attrSrc->vrName != attrDst->vrName && m_objAttrName)
        result += "\n        Наименование: "
                + from_bstr_t(attrSrc->vrName)
                + " != "
                + from_bstr_t(attrDst->vrName);

    if (attrSrc->vrValue != attrDst->vrValue && m_objAttrValue) {
        if (attrSrc->vrValue.vt & VT_DISPATCH) {
            if (differenceIDispatchs(attrSrc->vrValue,
                                     attrDst->vrValue,
                                     attrSrc->vrClassValue()->GetvrDataType()))
                result += "\n        Значение изменено";

        } else
            result += "\n        Значение изменено: \""
                    + from_variant_t(attrSrc->vrValue).toString()
                    + "\" != \""
                    + from_variant_t(attrDst->vrValue).toString()
                    + "\"";
    }

    if (attrSrc->vrMeasureUnit != attrDst->vrMeasureUnit && m_objAttrMeasureUnit)
        result += "\n        ЕИ: "
                + from_bstr_t(attrSrc->vrMeasureUnit)
                + " != "
                + from_bstr_t(attrDst->vrMeasureUnit);

    if (attrSrc->vrPrecision != attrDst->vrPrecision && m_objAttrPrecision)
        result += "\n        Точность: изменена";

    if (attrSrc->vrOwnerID != attrDst->vrOwnerID && m_objAttrOwnerId)
        result += "\n        Владелец: "
                + from_bstr_t(attrSrc->vrOwnerID)
                + " != "
                + from_bstr_t(attrDst->vrOwnerID);

    if (!result.isEmpty())
        result = "\n    Атрибут: "
                + from_bstr_t(attrSrc->vrName)
                + result;

    return result;
}

QString StructuresDifference::differenceAttrs(
        vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
{

    QString result;
    QString attrType = "Неизвестынй тип атрибута";
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
        result = "\n    " + attrType+" удалён(а): " + nameAttr;
        return result;
    }

    vkernelLib::IVClassValue *vInheritedAttrSrc = vAttrSrc->vrInheritedFrom;
    vkernelLib::IVClassValue *vInheritedAttrDst = vAttrDst->vrInheritedFrom;
    GUID baseClassGuidSrc = (vInheritedAttrSrc != NULL)
            ? vInheritedAttrSrc->vrClassValueID : GUID_NULL;
    GUID baseClassGuidDst = (vInheritedAttrDst != NULL)
            ? vInheritedAttrDst->vrClassValueID : GUID_NULL;

    if (baseClassGuidSrc == GUID_NULL)
        result += this->differenceAttrPerms(vAttrSrc, vAttrDst);
    else if (!m_attrInherited)
        return result;

    bool sysFunc = vAttrSrc->vrType != 1
            && nameAttr.compare("showme", Qt::CaseInsensitive)==0
            && nameAttr.compare("showinplan", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterconstruction", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterfirstlink", Qt::CaseInsensitive)==0
            && nameAttr.compare("beforedelete", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterupdate", Qt::CaseInsensitive)==0
            && nameAttr.compare("canchange", Qt::CaseInsensitive)==0
            && nameAttr.startsWith("diag_", Qt::CaseInsensitive)==0;
    if (vAttrSrc->vrClassValueID != vAttrDst->vrClassValueID
            && sysFunc && m_attrId) {
        result += "\n        Идентификатор: "
                + from_guid(vAttrSrc->vrClassValueID)
                + " != "
                + from_guid(vAttrDst->vrClassValueID);
    }

    GUID dataType = vAttrSrc->GetvrDataType();
    if (vAttrSrc->GetvrDataType() !=  vAttrDst->GetvrDataType() && m_attrDataType)
         result += "\n        Тип данных: "
                 + dataTypeGuidToString(vAttrSrc->GetvrDataType()) + " != "
                 + dataTypeGuidToString(vAttrDst->GetvrDataType());

    if (vAttrSrc->vrType != vAttrDst->vrType && m_attrType)
        result += "\n        Тип атрибута: "
                + typeAttrToString(vAttrSrc->vrType) + " != "
                + typeAttrToString(vAttrDst->vrType);

    if (vAttrSrc->vrScreenName != vAttrDst->vrScreenName && m_attrScreenName)
        result += "\n        Экранное имя: "
                + from_bstr_t(vAttrSrc->vrScreenName)
                + " != "
                + from_bstr_t(vAttrDst->vrScreenName);

    if (vAttrSrc->vrAliasName != vAttrDst->vrAliasName && m_attrAliasName)
        result += "\n        Справочники: "
                + from_bstr_t(vAttrSrc->vrAliasName) + " != "
                + from_bstr_t(vAttrDst->vrAliasName);


    if (vAttrSrc->vrType == 2) {
        vkernelLib::IVClass *vClassSrc = vAttrSrc->vrClass();
        vkernelLib::IVClassValue *vAttrCodeSrc = NULL;

        for (int k = 0; k < vClassSrc->vrClassValuesCount(); k++){
            vAttrCodeSrc = vClassSrc->vriClassValueItem(k);
            if (vAttrCodeSrc->vrName == vAttrSrc->vrFunctionCode)
                break;
        }

        vkernelLib::IVClass *vClassDst = vAttrDst->vrClass();
        vkernelLib::IVClassValue *vAttrCodeDst = NULL;
        for (int k = 0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrName == vAttrDst->vrFunctionCode)
                break;
        }

        if (vAttrCodeSrc != NULL && vAttrCodeDst != NULL && m_attrFuncRead)
            if (vAttrCodeSrc->vrFunctionCode != vAttrCodeDst->vrFunctionCode)
                result += "\n        Функция чтения: изменена";

        for (int k = 0; k < vClassSrc->vrClassValuesCount(); k++){
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
            if (vAttrCodeSrc->vrCalcSetFunction
                    != vAttrCodeDst->vrCalcSetFunction && m_attrFuncWrite)
                result += "\n        Функция записи: изменена";

    } else if (vAttrSrc->vrType == 1) {
        if (vAttrSrc->vrFunctionCode
                != vAttrDst->vrFunctionCode && m_attrFuncRead)
            result += "\n        Код функции: изменён";

    } else if (vAttrSrc->vrType == 0) {
        if (m_attrForbidInput
               && (dataType == ID_STR
                || dataType == ID_TEXT
                || dataType == ID_BOOL
                || dataType == ID_DATE))
        {
            if (vAttrSrc->vrForbidInput != vAttrDst->vrForbidInput )
                result += QString("\n        Ввод только из списка: ")
                        .arg(vAttrSrc->vrForbidInput)
                        .arg(vAttrDst->vrForbidInput);
            if (vAttrSrc->vrFunctionCode != vAttrDst->vrFunctionCode)
                result += "\n        Список значений: изменён";
        }

        if (m_attrLimit
               && (dataType == ID_INT
                || dataType == ID_FLT
                || dataType == ID_VARIANT))
        {
            if (vAttrSrc->vrFunctionCode != vAttrDst->vrFunctionCode)
                result += "\n        Ограничение: изменено";
        }
    }

    if (vAttrSrc->vrBlocked != vAttrDst->vrBlocked && m_attrBlocked)
        result += QString("\n        Блокировка: %1 != %2")
                .arg(vAttrSrc->vrBlocked)
                .arg(vAttrDst->vrBlocked);

    if (vAttrSrc->vrMeasureUnit != vAttrDst->vrMeasureUnit && m_attrMeasureUnit)
        result += "\n        ЕИ: "
                + from_bstr_t(vAttrSrc->vrMeasureUnit)
                + " != "
                + from_bstr_t(vAttrDst->vrMeasureUnit);

    if (vAttrSrc->vrMeasureEntity != vAttrDst->vrMeasureEntity
            && m_attrMeasureEntity)
        result +="\n        ЕВ: "
                + from_bstr_t(vAttrSrc->vrMeasureEntity)
                + " != "
                + from_bstr_t(vAttrDst->vrMeasureEntity);   

    if (baseClassGuidSrc != baseClassGuidDst && sysFunc && m_attrBaseClass)
        result += "\n        Базовый класс: "
                + from_guid(baseClassGuidSrc)
                + " != "
                + from_guid(baseClassGuidDst);

    if (vAttrSrc->vrPrecision != vAttrDst->vrPrecision && vAttrSrc->vrType != 1
           && m_attrPrecision && (dataType == ID_FLT || dataType == ID_VARIANT))
        result += "\n        Точность:"
                + QString("%1").arg(vAttrSrc->vrPrecision)
                + " != "
                + QString("%1").arg(vAttrDst->vrPrecision);

    if (vAttrSrc->vrGroup != vAttrDst->vrGroup && m_attrGroup)
        result = result + "\n        Группа: "
                + from_bstr_t(vAttrSrc->vrGroup) + " != "
                + from_bstr_t(vAttrDst->vrGroup);

    result += differencePropAttrs(vAttrSrc, vAttrDst);

    if (!result.isEmpty())
        result = "\n    "+ attrType +": " +  nameAttr + result;
    return result;
}

QString StructuresDifference::addingAttr(vkernelLib::IVClassValue *vAttrDst)
{
    QString result;
    QString attrType = "Неизвестынй тип атрибута";
    switch (vAttrDst->vrType) {
        case 0: attrType = "Добавлен атрибут";
                break;
        case 1: attrType = "Добавлена функция";
            break;
        case 2: attrType = "Добавлен вычисляемый атрибут";
            break;
    }

    QString nameAttr = from_bstr_t(vAttrDst->vrName);
    if (vAttrDst->vrType == 1) {
        vkernelLib::IVClass *vClassDst = vAttrDst->vrClass();
        vkernelLib::IVClassValue *vAttrCodeDst = NULL;
        for (int k = 0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrFunctionCode == vAttrDst->vrName
                    || vAttrCodeDst->vrCalcSetFunction == vAttrDst->vrName)
                return result;
        }
    }

    vkernelLib::IVClassValue *vInheritedAttrDst = vAttrDst->vrInheritedFrom;
    GUID baseClassGuidDst = (vInheritedAttrDst != NULL)
            ? vInheritedAttrDst->vrClassValueID : GUID_NULL;

    if (baseClassGuidDst == GUID_NULL)
        result += this->addingAttrPerms(vAttrDst);
    else if (!m_attrInherited)
        return result;

    bool sysFunc = vAttrDst->vrType != 1
            && nameAttr.compare("showme", Qt::CaseInsensitive)==0
            && nameAttr.compare("showinplan", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterconstruction", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterfirstlink", Qt::CaseInsensitive)==0
            && nameAttr.compare("beforedelete", Qt::CaseInsensitive)==0
            && nameAttr.compare("afterupdate", Qt::CaseInsensitive)==0
            && nameAttr.compare("canchange", Qt::CaseInsensitive)==0
            && nameAttr.startsWith("diag_", Qt::CaseInsensitive)==0;
    if (sysFunc && m_attrId)
        result += "\n        Идентификатор: "
                + from_guid(vAttrDst->vrClassValueID);

    GUID dataType = vAttrDst->GetvrDataType();
    if (m_attrDataType)
         result += "\n        Тип данных: "
                 + dataTypeGuidToString(vAttrDst->GetvrDataType());

    if (m_attrType)
        result += "\n        Тип атрибута: "
                + typeAttrToString(vAttrDst->vrType);

    if (m_attrScreenName)
        result += "\n        Экранное имя: "
                + from_bstr_t(vAttrDst->vrScreenName);

    if (m_attrAliasName) {
        QString aliasName = from_bstr_t(vAttrDst->vrAliasName);
        if (!aliasName.isEmpty())
            result += "\n        Справочники: " + aliasName;
    }

    if (vAttrDst->vrType == 2) {
        vkernelLib::IVClass *vClassDst = vAttrDst->vrClass();
        vkernelLib::IVClassValue *vAttrCodeDst = NULL;

        for (int k=0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrName == vAttrDst->vrFunctionCode)
                break;
        }

        if (vAttrCodeDst != NULL && m_attrFuncRead)
            result += "\n        Функция чтения: изменена";

        vClassDst = vAttrDst->vrClass();
        vAttrCodeDst = NULL;
        for (int k=0; k < vClassDst->vrClassValuesCount(); k++){
            vAttrCodeDst = vClassDst->vriClassValueItem(k);
            if (vAttrCodeDst->vrName == vAttrDst->vrCalcSetFunction)
                break;
        }
        if (vAttrCodeDst != NULL && m_attrFuncWrite)
                result += "\n        Функция записи: изменена";

    } else if (vAttrDst->vrType == 0) {
        if (m_attrForbidInput
               && from_bstr_t(vAttrDst->vrFunctionCode) != ""
               && (dataType == ID_STR
                || dataType == ID_TEXT
                || dataType == ID_BOOL
                || dataType == ID_DATE))
        {
            result += QString("\n        Ввод только из списка: %1")
                    .arg(vAttrDst->vrForbidInput);
            result += "\n        Список значений: изменён";
        }

        if (m_attrLimit
                && (dataType == ID_INT
                 || dataType == ID_FLT
                 || dataType == ID_VARIANT))
        {
            QString limit = from_bstr_t(vAttrDst->vrFunctionCode);
            QStringList limitList = limit.split("\r\n");
            if (limitList.count() >= 3) {
                bool ok = true;
                double down, up, step;
                if (ok) down = limitList.value(0).toDouble(&ok);
                if (ok) up   = limitList.value(1).toDouble(&ok);
                if (ok) step = limitList.value(2).toDouble(&ok);
                if (ok)
                    if (down < up && step >0)
                        result += "\n        Ограничение: добавлено";
            }
        }
    }

    if (m_attrBlocked)
        result += QString("\n        Блокировка: %1")
                .arg(vAttrDst->vrBlocked);

    if (m_attrMeasureUnit) {
        QString measureUnit = from_bstr_t(vAttrDst->vrMeasureUnit);
        if (!measureUnit.isEmpty())
            result += "\n        ЕИ: " + measureUnit;
    }

    if (m_attrMeasureEntity) {
        QString measureEntity = from_bstr_t(vAttrDst->vrMeasureEntity);
        if (!measureEntity.isEmpty())
            result += "\n        ЕВ: " + measureEntity;
    }

    if (sysFunc && m_attrBaseClass)
        result += "\n        Базовый класс: "
                + from_guid(baseClassGuidDst);

    if (vAttrDst->vrType != 1 && m_attrPrecision
            && (dataType == ID_FLT || dataType == ID_VARIANT))
        result += "\n        Точность: "
                + QString("%1").arg(vAttrDst->vrPrecision);

    if (m_attrGroup) {
        QString group = from_bstr_t(vAttrDst->vrGroup);
        if (!group.isEmpty())
            result += "\n        Группа: " + group;
    }

    result += addingPropAttrs(vAttrDst);

    if (!result.isEmpty())
        result = "\n    "+ attrType +": " +  nameAttr + result;
    return result;
}

QString StructuresDifference::differencePropAttrs(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
{
    if (!m_attrProp) return "";

    QString result;
    vkernelLib::IVProperties  *vPropsSrc = vAttrSrc->vrProperties();
    vkernelLib::IVProperties  *vPropsDst = vAttrDst->vrProperties();
    for (int i=0; i<vPropsSrc->vrCount(); i++) {
        BSTR nameSrc;
        VARIANT tmp;
        vPropsSrc->vrItem(i, &nameSrc, &tmp);
        _variant_t valueDst = vPropsDst->vrPropVal[nameSrc];
        _variant_t valueSrc = vPropsSrc->vrPropVal[nameSrc];
        if (valueSrc != valueDst)
            result += "\n        Свойство \""
                    + from_bstr_t(nameSrc)
                    +"\" изменено: "
                    + from_variant_t(valueSrc).toString()
                    + " != "
                    + from_variant_t(valueDst).toString();
    }
    return result;
}

QString StructuresDifference::addingPropAttrs(vkernelLib::IVClassValue *vAttrDst)
{
    if (!m_attrProp)
        return "";

    QString result;
    vkernelLib::IVProperties  *vPropsDst = vAttrDst->vrProperties();
    for (int i = 0; i < vPropsDst->vrCount(); i++) {
        BSTR nameDst;
        VARIANT tmp;
        vPropsDst->vrItem(i, &nameDst, &tmp);
        _variant_t valueDst = vPropsDst->vrPropVal[nameDst];
        result += "\n        Добавлено свойство \""
                + from_bstr_t(nameDst)
                +"\": "
                + from_variant_t(valueDst).toString();
    }
    return result;
}

QString StructuresDifference::differenceClasses(
        vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst) {

    QString result;
    if (vClassDst == NULL) {
        result += "\n\nУдалён класс: "+ from_bstr_t(vClassSrc->vrName);
        return result;
    }

    result += this->differenceClassPerms(vClassSrc, vClassDst);

    if (vClassSrc->vrClassID != vClassDst->vrClassID && m_classId) {
        result += "\n    Идентификатор: "
                + from_guid(vClassSrc->vrClassID)
                + "!="
                + from_guid(vClassDst->vrClassID);
    }

    if (vClassSrc->vrBlocked != vClassDst->vrBlocked)
        result += QString("\n    Блокировка: %1 != %2")
                .arg(vClassSrc->vrBlocked)
                .arg(vClassDst->vrBlocked);

    if (vClassSrc->vrScreenName != vClassDst->vrScreenName && m_classScreenName)
        result += "\n    Экранноё имя: "
                + from_bstr_t(vClassSrc->vrScreenName)
                + "!="
                + from_bstr_t(vClassDst->vrScreenName);

    vkernelLib::IVClass *vBaseClassSrc = vClassSrc->vrBaseClass;
    vkernelLib::IVClass *vBaseClassDst = vClassDst->vrBaseClass;
    GUID baseClassGuidSrc = (vBaseClassSrc != NULL)
            ? vBaseClassSrc->vrClassID : GUID_NULL;
    GUID baseClassGuidDst = (vBaseClassDst != NULL)
            ? vBaseClassDst->vrClassID : GUID_NULL;
    if (baseClassGuidSrc != baseClassGuidDst && m_classBaseClass)
        result += "\n        Базовый класс: "
                + from_guid(baseClassGuidSrc) + " != "
                + from_guid(baseClassGuidDst);

    result += differenceClassLinks(vClassSrc, vClassDst);

    if (m_attrId || m_attrDataType || m_attrType || m_attrScreenName
            || m_attrAliasName || m_attrFuncRead || m_attrFuncWrite
            || m_attrBlocked || m_attrMeasureUnit || m_attrMeasureEntity
            || m_attrForbidInput || m_attrBaseClass || m_attrPrecision
            || m_attrLimit || m_attrGroup || m_attrProp || m_attrPerms)
    {
        for (int j = 0; j < vClassSrc->vrClassValuesCount(); j++){
            vkernelLib::IVClassValue *vAttrSrc = vClassSrc->vriClassValueItem(j);
            for (int k = 0; k<vClassDst->vrClassValuesCount(); k++){
                vkernelLib::IVClassValue *vAttrDst
                        = vClassDst->vriClassValueItem(k);
                if (vAttrSrc->vrName == vAttrDst->vrName) {
                    result += differenceAttrs(vAttrSrc, vAttrDst);
                    break;
                } else if (k == vClassDst->vrClassValuesCount()-1)
                    result += differenceAttrs(vAttrSrc, NULL);
            }
        }

        for (int j=0; j<vClassDst->vrClassValuesCount(); j++){
            vkernelLib::IVClassValue *vAttrDst = vClassDst->vriClassValueItem(j);
            for (int k = 0; k<vClassSrc->vrClassValuesCount(); k++){
                vkernelLib::IVClassValue *vAttrSrc
                        = vClassSrc->vriClassValueItem(k);
                if (vAttrSrc->vrName == vAttrDst->vrName) {
                    break;
                } else if (k == vClassSrc->vrClassValuesCount()-1)
                    result += addingAttr(vAttrDst);
            }
        }
    }

    if (!result.isEmpty())
        result = "\n\nКласс: " +  from_bstr_t(vClassSrc->vrName) + result;
    return result;
}

QString StructuresDifference::addingClass(vkernelLib::IVClass *vClassDst)
{
    QString result;

    result = "\n\nДобавлен класс: "+ from_bstr_t(vClassDst->vrName);
    if (m_classId)
        result += "\n    Идентификатор: " + from_guid(vClassDst->vrClassID);
    if (m_classBlocked)
        result += QString("\n    Блокировка: %1").arg(vClassDst->vrBlocked);
    if (m_classScreenName)
        result += "\n    Экранноё имя: " + from_bstr_t(vClassDst->vrScreenName);
    if (m_classBaseClass) {
        vkernelLib::IVClass *vBaseClassDst = vClassDst->vrBaseClass;
        GUID baseClassGuidDst = (vBaseClassDst != NULL)
                ? vBaseClassDst->vrClassID : GUID_NULL;
        result += "\n    Базовый класс: " + from_guid(baseClassGuidDst);
    }
    result += this->addingClassPerms(vClassDst);
    result += this->addingClassLinks(vClassDst);

    if (m_attrId || m_attrDataType || m_attrType || m_attrScreenName
            || m_attrAliasName || m_attrFuncRead || m_attrFuncWrite
            || m_attrBlocked || m_attrMeasureUnit || m_attrMeasureEntity
            || m_attrForbidInput || m_attrBaseClass || m_attrPrecision
            || m_attrLimit || m_attrGroup || m_attrProp || m_attrPerms)
    {
        for (int k=0; k<vClassDst->vrClassValuesCount(); k++){
            vkernelLib::IVClassValue *vAttrDst = vClassDst->vriClassValueItem(k);
            result += addingAttr(vAttrDst);
        }
    }

    return result;
}

QString StructuresDifference::differenceClassLinks(
        vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
{
    if (!m_classChild)
        return "";

    QString result;
    for (int i=0; i < vClassSrc->vrChildsCount(); i++){
        vkernelLib::IVClass *vClassChildSrc
                = vClassSrc->vriChildClassItem(i);
        vkernelLib::IVClass *vClassChildDst
                = vClassDst->vrnChildClassItem(vClassChildSrc->vrName);
        if (vClassChildDst==NULL)
            result = result + "\n    Удалена связь: "
                    + from_bstr_t(vClassSrc->vrName)
                    + " <- "
                    + from_bstr_t(vClassChildSrc->vrName);
    }

    for (int i=0; i < vClassDst->vrChildsCount(); i++){
        vkernelLib::IVClass *vClassChildDst
                = vClassDst->vriChildClassItem(i);
        vkernelLib::IVClass *vClassChildSrc
                = vClassSrc->vrnChildClassItem(vClassChildDst->vrName);
        if (vClassChildSrc == NULL)
            result = result + "\n    Добавлена связь: "
                    + from_bstr_t(vClassDst->vrName)
                    + " <- "
                    + from_bstr_t(vClassChildDst->vrName);
    }

    return result;
}

QString StructuresDifference::addingClassLinks(vkernelLib::IVClass *vClassDst)
{
    if (!m_classChild)
        return "";

    QString result = "";
    for (int i=0; i < vClassDst->vrChildsCount(); i++){
        vkernelLib::IVClass *vClassChildDst = vClassDst->vriChildClassItem(i);
        result += "\n    Добавлена связь: "
                + from_bstr_t(vClassDst->vrName)
                + " <- "
                + from_bstr_t(vClassChildDst->vrName);
    }
    return result;
}

QString StructuresDifference::differenceModels(
        vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {
    QString result;

    vkernelLib::IVClassVector *vClassVectorSrc = vModelSrc->vrGetClassVector();
    vkernelLib::IVClassVector *vClassVectorDst = vModelDst->vrGetClassVector();
    for (int i=0; i<vClassVectorSrc->vrCount(); i++){
        vkernelLib::IVClass *vClassSrc = vClassVectorSrc->vrItem(i);
        vkernelLib::IVClass *vClassDst = vClassVectorDst->vrLocate(
                    vClassSrc->vrName);
        result += differenceClasses(vClassSrc, vClassDst);
    }
    for (int i=0; i<vClassVectorDst->vrCount(); i++){
        vkernelLib::IVClass *vClassDst = vClassVectorDst->vrItem(i);
        vkernelLib::IVClass *vClassSrc = vClassVectorSrc->vrLocate(
                    vClassDst->vrName);
        if (vClassSrc == NULL)
            result += addingClass(vClassDst);
    }
    result += differenceFilters(vModelSrc, vModelDst);
    vkernelLib::IVObjectVector *vObjsSrc = vModelSrc->vrGetObjVector();
    vkernelLib::IVObjectVector *vObjsDst = vModelDst->vrGetObjVector();
    for (int i=0; i<vObjsSrc->vrObjectsCount(); i++) {
         vkernelLib::IVObject *vObjectSrc = vObjsSrc->vrItem(i);
         vkernelLib::IVObject *vObjectDst = vObjsDst->vrGetObjByStrID(
                     vObjectSrc->vrObjStrID());
         result += differenceObjects(vObjectSrc, vObjectDst);
    }
    for (int i=0; i<vObjsDst->vrObjectsCount(); i++) {
         vkernelLib::IVObject *vObjectDst = vObjsDst->vrItem(i);
         vkernelLib::IVObject *vObjectSrc = vObjsSrc->vrGetObjByStrID(
                     vObjectDst->vrObjStrID());
         result += addingObjects(vObjectSrc, vObjectDst);
    }

    return result;
}

QString StructuresDifference::differenceFilters (vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {

    if (!m_classFilter)
        return "";

    QString result;

    vkernelLib::IVClassVector *vClassVectorSrc = vModelSrc->vrGetClassVector();
    vkernelLib::IVClassVector *vClassVectorDst = vModelDst->vrGetClassVector();

    vkernelLib::IVFilterVector *vFilterVectorSrc
            = vClassVectorSrc->vrFilterVector();
    vkernelLib::IVFilterVector *vFilterVectorDst
            = vClassVectorDst->vrFilterVector();

    for (int i=0; i<vFilterVectorSrc->vrFiltersCount(); i++){
        _bstr_t filterSrc = vFilterVectorSrc->vrFilterName(i);

        if (vFilterVectorDst->vrLocateFilter(filterSrc)) {
            QString filterResult("");
            for (int j = 0;
                 j < vFilterVectorSrc->vrConstrainsCount(filterSrc); j++)
            {
                _bstr_t classSrc = vFilterVectorSrc->vrConstrains(filterSrc,j);
                if (!(vFilterVectorDst->vrLocateConstraint(filterSrc, classSrc)))
                    filterResult += "\n    Удалён класс: "
                                  + from_bstr_t(classSrc);
            }
            if (!filterResult.isEmpty())
                result += "\n\nФильтр: " +  from_bstr_t(filterSrc);

            result += filterResult;
        } else {
            result += "\nУдалён фильтр: "+ from_bstr_t(filterSrc);
        }

    }

    for (int i = 0; i < vFilterVectorDst->vrFiltersCount(); i++){
        _bstr_t filterDst = vFilterVectorDst->vrFilterName(i);

        if (vFilterVectorSrc->vrLocateFilter(filterDst))
        {
            QString filterResult("");
            for (int j = 0;
                 j < vFilterVectorDst->vrConstrainsCount(filterDst); j++)
            {
                _bstr_t classDst = vFilterVectorDst->vrConstrains(filterDst,j);
                if (!(vFilterVectorSrc->vrLocateConstraint(filterDst, classDst)))
                    filterResult += "\n    Добавлен класс: "
                                 + from_bstr_t(classDst);
            }
            if (!filterResult.isEmpty())
                result += "\n\nФильтр: " +  from_bstr_t(filterDst);

            result += filterResult;
        } else {
            result += "\n\nДобавлен фильтр: " + from_bstr_t(filterDst);
            for (int j = 0;
                 j < vFilterVectorDst->vrConstrainsCount(filterDst); j++)
            {
                _bstr_t classDst = vFilterVectorDst->vrConstrains(filterDst,j);
                result += "\n    Добавлен класс: "+ from_bstr_t(classDst);
            }
        }

    }
    return result;
}
