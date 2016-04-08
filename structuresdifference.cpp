#include "structuresdifference.h"
#include "converttype.h"
#include <QString>
#include <QDebug>
#include <QFile>

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

QString StructuresDifference::differenceAttrGroups(vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst)
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

QString StructuresDifference::differenceClassPerms(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
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

QString StructuresDifference::differenceAttrPerms(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
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

QString StructuresDifference::differenceObjects(vkernelLib::IVObject *vObjectSrc,  vkernelLib::IVObject *vObjectDst)
{
    QString result;
    if (vObjectDst==NULL)
    {
        result = result + "\n\nНе существует объект: "
                + from_bstr_t(vObjectSrc->vrObjStrID());
        return result;
    }

    if (vObjectSrc->vrObjStrID() != vObjectDst->vrObjStrID())
        result = result + "\n    Идентификатор: "
                + from_bstr_t(vObjectSrc->vrObjStrID()) + " != "
                + from_bstr_t(vObjectDst->vrObjStrID());
    if (vObjectSrc->vrOwnerID != vObjectDst->vrOwnerID)
        result = result + "\n    Владелец: "
                + from_bstr_t(vObjectSrc->vrOwnerID) + " != "
                + from_bstr_t(vObjectDst->vrOwnerID);
    if (vObjectSrc->vrWasChanged != vObjectDst->vrWasChanged)
        result = result + "\n    Возможность изменения: изменено";
    if (vObjectSrc->vrReadOnly != vObjectDst->vrReadOnly)
        result = result + "\n    Только чтение: "
                + vObjectSrc->vrReadOnly + " != "
                + vObjectDst->vrReadOnly;

    result += differenceObjectLinks(vObjectSrc, vObjectDst);

    for (int i=0; i<vObjectSrc->vrAttrCount(); i++) {
        vkernelLib::IVAttribute *attrSrc = vObjectSrc->vrAttrByIndex(i);
        vkernelLib::IVAttribute *attrDst = vObjectDst->vrAttrByName(attrSrc->vrName);
        result += differenceAttrObjects(attrSrc, attrDst);
    }

    if (!result.isEmpty())
        result = "\n\nОбъект: "
                + from_bstr_t(vObjectSrc->vrObjStrID())
                + result;

    return result;
}

QString StructuresDifference::differenceObjectLinks(vkernelLib::IVObject *vObjectSrc ,vkernelLib::IVObject *vObjectDst)
{
    QString result;
    vkernelLib::IVObjectVector *vObjsSrc = vObjectSrc->vrObjectsVector();
    vkernelLib::IVObjectVector *vObjsDst = vObjectDst->vrObjectsVector();

    vkernelLib::IVIterator *vIterSrc;
    vkernelLib::IVIterator *vIterDst;
    if (vObjsSrc->raw_vrCreateIterator(L"", vObjectSrc, true, &vIterSrc)!=S_OK)
        return result;
    if (vObjsDst->raw_vrCreateIterator(L"", vObjectSrc, true, &vIterDst)!=S_OK)
        return "\n    Дочерние объекты не существуют";


    for (bool flagSrc = vIterSrc->vrFirst(); flagSrc == true; flagSrc = vIterSrc->vrNext())
    {
        vkernelLib::IVObject *vChildObjectSrc =  vIterSrc->vrGetObject();
        bool noneChild = true;
        for (bool flagDst = vIterDst->vrFirst(); flagDst == true; flagDst = vIterDst->vrNext())
        {
            vkernelLib::IVObject *vChildObjectDst =  vIterDst->vrGetObject();
            if (vChildObjectSrc->vrObjStrID()==vChildObjectDst->vrObjStrID()) {
                noneChild = false;
                break;
            }
        }
        if (noneChild)
              result = result + "\n    Дочерний объект не существует: "
                      + from_bstr_t(vChildObjectSrc->vrObjStrID());
    }

    return result;
}

bool StructuresDifference::differenceIDispatchs(_variant_t varSrc, _variant_t varDst, GUID dataType)
{
    const GUID ID_INT = to_guid("{DDF29044-F2DA-4457-9CA2-CA0F1E6501A6}");
    const GUID ID_FLT = to_guid("{51AF1A06-C000-4ed4-940D-ADD3B199EB5E}");
    const GUID ID_STR = to_guid("{751A3E7F-86DC-4527-91C8-79919F3B3FA3}");
    const GUID ID_TEXT = to_guid("{B7672ACF-D5AA-4432-AA5C-ACF9FF8A27F7}");
    const GUID ID_BOOL = to_guid("{1739DAD4-D175-4b0a-B2C7-55759916F7EF}");
    const GUID ID_DATE = to_guid("{BDB20E23-B5D7-4966-8110-6F4EFD8764BB}");
    const GUID ID_NULL = to_guid("{00000000-0000-0000-0000-000000000000}");
    const GUID ID_FILE = to_guid("{2B02B52B-7C4A-40a7-845A-DA553A1B0DA1}");
    const GUID ID_STREAM = to_guid("{F0CA433A-74E7-47b0-958C-CC1EBAC73DA8}");

    const GUID ID_DIM = to_guid("{428CC298-47D3-4D09-8B7C-E6F36518C724}");
    const GUID ID_ROUGHNESS = to_guid("{B6C89FE7-2B27-492C-962E-9940365E6201}");
    const GUID ID_ANGLE = to_guid("{2FBF9A81-03B2-4AE8-BF46-3BE7240E7722}");
    const GUID ID_MARKER = to_guid("{068E304F-E0C5-4B75-8785-B893CD910740}");
    const GUID ID_VARIANT = to_guid("{EA1481CE-A241-47DB-BAB8-77EA17C51239}");
    const GUID ID_THREAD = to_guid("{627AE639-7DBE-4420-880E-A1373F0CA4AF}");
    const GUID ID_OBJECTSET = to_guid("{355A97C0-1066-406D-BDEE-46BE1C72EB4E}");
    const GUID ID_FORMTOLERANCE = to_guid("{878CFC01-4EBD-4189-A0F8-A25C087394EA}");
    const GUID ID_DEFLECTEDDOUBLE = to_guid("{34F029CE-71B9-433D-BD0C-A888750D1884}");

    if (dataType == ID_DIM) {
        vkernel_aLib::ISize *iObjSrc = from_vdispatch<vkernel_aLib::ISize>(varSrc);
        vkernel_aLib::ISize *iObjDst = from_vdispatch<vkernel_aLib::ISize>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrValue != iObjDst->vrValue
              || iObjSrc->vrBraces != iObjDst->vrBraces
              || iObjSrc->vrEI != iObjSrc->vrEI
              || iObjSrc->vrES != iObjSrc->vrES
              || iObjSrc->vrFormType != iObjDst->vrFormType
              || iObjSrc->vrFrame != iObjDst->vrFrame
              || iObjSrc->vrQuality != iObjSrc->vrQuality
              || iObjSrc->vrQualityNumber != iObjSrc->vrQualityNumber
              || iObjSrc->vrQualityShow != iObjDst->vrQualityShow
              || iObjSrc->vrQualityType != iObjDst->vrQualityType
              || iObjSrc->vrShowDeflection != iObjSrc->vrShowDeflection
              || iObjSrc->vrSpecialSymbol != iObjSrc->vrSpecialSymbol
              || iObjSrc->vrTextAfter != iObjSrc->vrTextAfter
              || iObjSrc->vrTextBefore != iObjSrc->vrTextBefore
              || iObjSrc->vrTextValue != iObjSrc->vrTextValue
              || iObjSrc->vrUnderline != iObjSrc->vrUnderline
            )
                return true;
        } else return true;

    }

    if (dataType == ID_ROUGHNESS) {
        vkernel_aLib::IRoughness *iObjSrc = from_vdispatch<vkernel_aLib::IRoughness>(varSrc);
        vkernel_aLib::IRoughness *iObjDst = from_vdispatch<vkernel_aLib::IRoughness>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrRA != iObjDst->vrRA
                     || iObjSrc->vrRoughClass != iObjDst->vrRoughClass
                     || iObjSrc->vrRZ != iObjDst->vrRZ
                     || iObjSrc->vrTextValue != iObjDst->vrTextValue
                     || iObjSrc->vrUseRA != iObjDst->vrUseRA
             )
                  return true;
        } else return true;

    }

    if (dataType == ID_ANGLE) {
        vkernel_aLib::IAngle *iObjSrc = from_vdispatch<vkernel_aLib::IAngle>(varSrc);
        vkernel_aLib::IAngle *iObjDst = from_vdispatch<vkernel_aLib::IAngle>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrBraces != iObjDst->vrBraces
                     || iObjSrc->vrEI != iObjDst->vrEI
                     || iObjSrc->vrES != iObjDst->vrES
                     || iObjSrc->vrFrame != iObjDst->vrFrame
                     || iObjSrc->vrShowDeflection != iObjDst->vrShowDeflection
                     || iObjSrc->vrShowType != iObjDst->vrShowType
                     || iObjSrc->vrTextAfter != iObjDst->vrTextAfter
                     || iObjSrc->vrTextBefore != iObjDst->vrTextBefore
                     || iObjSrc->vrTextValue != iObjDst->vrTextValue
                     || iObjSrc->vrUnderline != iObjDst->vrUnderline
                     || iObjSrc->vrValue != iObjDst->vrValue
             )
                  return true;
        } else return true;
    }

    if (dataType == ID_MARKER) {
        vkernel_aLib::IMarker *iObjSrc = from_vdispatch<vkernel_aLib::IMarker>(varSrc);
        vkernel_aLib::IMarker *iObjDst = from_vdispatch<vkernel_aLib::IMarker>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrTextValue != iObjDst->vrTextValue
                    || iObjSrc->vrValue != iObjDst->vrValue
            )
                 return true;
        } else return true;

    }

    if (dataType == ID_VARIANT) {
        vkernelLib::IVariant *iObjSrc = from_vdispatch<vkernelLib::IVariant>(varSrc);
        vkernelLib::IVariant *iObjDst = from_vdispatch<vkernelLib::IVariant>(varDst);
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
                return differenceIDispatchs(iObjSrc->vrVariant, iObjDst->vrVariant,
                                            iObjSrc->vrClassID);
            }

        } else return true;
    }

    if (dataType == ID_THREAD) {
        vkernel_aLib::IThread *iObjSrc = from_vdispatch<vkernel_aLib::IThread>(varSrc);
        vkernel_aLib::IThread *iObjDst = from_vdispatch<vkernel_aLib::IThread>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrCountStart != iObjDst->vrCountStart
                     || iObjSrc->vrDExt != iObjDst->vrDExt
                     || iObjSrc->vrDExtEi != iObjDst->vrDExtEi
                     || iObjSrc->vrDExtEs != iObjDst->vrDExtEs
                     || iObjSrc->vrDInt != iObjDst->vrDInt
                     || iObjSrc->vrDIntEi != iObjDst->vrDIntEi
                     || iObjSrc->vrDIntEs != iObjDst->vrDIntEs
                     || iObjSrc->vrDMiddle != iObjDst->vrDMiddle
                     || iObjSrc->vrDMiddleEi != iObjDst->vrDMiddleEi
                     || iObjSrc->vrDMiddleEs != iObjDst->vrDMiddleEs
                     || iObjSrc->vrLengthThread != iObjDst->vrLengthThread
                     || iObjSrc->vrLengthwise != iObjDst->vrLengthwise
                     || iObjSrc->vrLengthwiseNormally != iObjDst->vrLengthwiseNormally
                     || iObjSrc->vrLengthwiseSmallerThread != iObjDst->vrLengthwiseSmallerThread
                     || iObjSrc->vrOboznD != iObjDst->vrOboznD
                     || iObjSrc->vrShiftBasePlane != iObjDst->vrShiftBasePlane
                     || iObjSrc->vrStep != iObjDst->vrStep
                     || iObjSrc->vrTextValue != iObjDst->vrTextValue
                     || iObjSrc->vrTolerance != iObjDst->vrTolerance
                     || iObjSrc->vrToleranceShiftBasePlane != iObjDst->vrToleranceShiftBasePlane
                     || iObjSrc->vrType != iObjDst->vrType
                     || iObjSrc->vrTypeForm != iObjDst->vrTypeForm
                     || iObjSrc->vrTypeLR != iObjDst->vrTypeLR
                     || iObjSrc->vrTypeStep != iObjDst->vrTypeStep
                     || iObjSrc->vrValue != iObjDst->vrValue
             )
                  return true;
        } else return true;
    }

    if (dataType == ID_OBJECTSET) {
        vkernel_aLib::IObjectSet *iObjSrc = from_vdispatch<vkernel_aLib::IObjectSet>(varSrc);
        vkernel_aLib::IObjectSet *iObjDst = from_vdispatch<vkernel_aLib::IObjectSet>(varDst);
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
        } else return true;
    }

    if (dataType == ID_FORMTOLERANCE) {
        vkernel_aLib::IFormTolerance *iObjSrc = from_vdispatch<vkernel_aLib::IFormTolerance>(varSrc);
        vkernel_aLib::IFormTolerance *iObjDst = from_vdispatch<vkernel_aLib::IFormTolerance>(varDst);
        if (iObjSrc && iObjDst) {
             if (iObjSrc->vrBase1 != iObjDst->vrBase1
                     || iObjSrc->vrBase2 != iObjDst->vrBase2
                     || iObjSrc->vrBaseCode1 != iObjDst->vrBaseCode1
                     || iObjSrc->vrBaseCode2 != iObjDst->vrBaseCode2
                     || iObjSrc->vrLengthwise != iObjDst->vrLengthwise
                     || iObjSrc->vrSymbol != iObjDst->vrSymbol
                     || iObjSrc->vrSymbolAdded != iObjDst->vrSymbolAdded
                     || iObjSrc->vrTextAfter != iObjDst->vrTextAfter
                     || iObjSrc->vrTextBefore != iObjDst->vrTextBefore
                     || iObjSrc->vrTextValue != iObjDst->vrTextValue
                     || iObjSrc->vrType != iObjDst->vrType
                     || iObjSrc->vrTypeAdded != iObjDst->vrTypeAdded
                     || iObjSrc->vrValue != iObjDst->vrValue
             )
                  return true;
        } else return true;
    }

    if (dataType == ID_DEFLECTEDDOUBLE) {
        vkernel_aLib::IDeflectedDouble *iObjSrc = from_vdispatch<vkernel_aLib::IDeflectedDouble>(varSrc);
        vkernel_aLib::IDeflectedDouble *iObjDst = from_vdispatch<vkernel_aLib::IDeflectedDouble>(varDst);
        if (iObjSrc && iObjDst) {
            if (iObjSrc->vrEI != iObjDst->vrEI
                    || iObjSrc->vrES != iObjDst->vrES
                    || iObjSrc->vrShowDeflection != iObjDst->vrShowDeflection
                    || iObjSrc->vrTextValue != iObjDst->vrTextValue
                    || iObjSrc->vrValue != iObjDst->vrValue
            )
                 return true;
        } else return true;
    }

    if (dataType == ID_FILE) {
        vkernelLib::IVFile *iObjSrc = from_vdispatch<vkernelLib::IVFile>(varSrc);
        vkernelLib::IVFile *iObjDst = from_vdispatch<vkernelLib::IVFile>(varDst);

        if (iObjSrc && iObjDst) {
            QFile fileSrc(from_bstr_t(iObjSrc->vsInternalFullName));
            QFile fileDst(from_bstr_t(iObjDst->vsInternalFullName));
            bool openedSrc = fileSrc.open(QIODevice::ReadOnly);
            bool openedDst = fileDst.open(QIODevice::ReadOnly);
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

QString StructuresDifference::differenceAttrObjects(vkernelLib::IVAttribute *attrSrc,vkernelLib::IVAttribute *attrDst)
{
    QString result;

    if (attrDst==NULL) {
        result = "\n    Не существует атрибут: " + from_bstr_t(attrSrc->vrName);
        return result;
    }
    if (attrSrc->vrName != attrDst->vrName)
        result = result + "\n        Наименование: "
                + from_bstr_t(attrSrc->vrName) + " != "
                + from_bstr_t(attrDst->vrName);
    if (attrSrc->vrValue != attrDst->vrValue) {
        if (attrSrc->vrValue.vt & VT_DISPATCH) {
            if (differenceIDispatchs(attrSrc->vrValue,
                                    attrDst->vrValue,
                                    attrSrc->vrClassValue()->GetvrDataType()))
                result = result + "\n        Значение изменено";

        } else
            result = result + "\n        Значение изменено: \""
                    + from_variant_t(attrSrc->vrValue).toString() + "\" != \""
                    + from_variant_t(attrDst->vrValue).toString() + "\"";
    }

    if (attrSrc->vrMeasureUnit != attrDst->vrMeasureUnit)
        result = result + "\n        ЕИ: "
                + from_bstr_t(attrSrc->vrMeasureUnit) + " != "
                + from_bstr_t(attrDst->vrMeasureUnit);
    if (attrSrc->vrPrecision != attrDst->vrPrecision)
        result = result + "\n        Точность: изменена";

    if (attrSrc->vrOwnerID != attrDst->vrOwnerID)
        result = result + "\n        Владелец: "
                + from_bstr_t(attrSrc->vrOwnerID) + " != "
                + from_bstr_t(attrDst->vrOwnerID);
    if (!result.isEmpty())
        result = "\n    Атрибут: "
                + from_bstr_t(attrSrc->vrName)
                + result;
    return result;
}

QString StructuresDifference::differenceAttrs(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst) {

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
        result += this->differenceAttrPerms(vAttrSrc, vAttrDst);

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
//            result = result + "\n" +"\""+ from_bstr_t(vAttrSrc->vrFunctionCode)+"\"";
//            result = result + "\n !=";
//            result = result + "\n" +"\"" + from_bstr_t(vAttrDst->vrFunctionCode)+"\"";
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

    result += differencePropAttrs(vAttrSrc, vAttrDst);

    if (!result.isEmpty())
        result = "\n    "+ attrType +": " +  nameAttr + result;
    return result;
}

QString StructuresDifference::differencePropAttrs(vkernelLib::IVClassValue *vAttrSrc, vkernelLib::IVClassValue *vAttrDst)
{
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
            result = result + "\n        Свойство \"" + from_bstr_t(nameSrc) +"\" изменено: "
                    + from_variant_t(valueSrc).toString() + " != "
                    + from_variant_t(valueDst).toString();
    }
    return result;
}

QString StructuresDifference::differenceClasses(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst) {

    QString result;
    if (vClassDst == NULL) {
        result =  result + "\nУдалён класс: "+ from_bstr_t(vClassSrc->vrName);
        return result;
    }

    result += this->differenceClassPerms(vClassSrc, vClassDst);

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

    result += differenceClassLinks(vClassSrc, vClassDst);

    for (int j=0; j<vClassSrc->vrClassValuesCount(); j++){
        vkernelLib::IVClassValue *vAttrSrc = vClassSrc->vriClassValueItem(j);
        for (int k=0; k<vClassDst->vrClassValuesCount(); k++){
            vkernelLib::IVClassValue *vAttrDst = vClassDst->vriClassValueItem(k);
            if (vAttrSrc->vrName == vAttrDst->vrName) {
                result += differenceAttrs(vAttrSrc, vAttrDst);
                break;
            } else if (k == vClassDst->vrClassValuesCount()-1)
                result += differenceAttrs(vAttrSrc, NULL);
        }
    }

    if (!result.isEmpty())
        result = "\n\nКласс: " +  from_bstr_t(vClassSrc->vrName) + result;
    return result;
}

QString StructuresDifference::differenceClassLinks(vkernelLib::IVClass *vClassSrc, vkernelLib::IVClass *vClassDst)
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

QString StructuresDifference::differenceModels(vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {
    QString result;

    vkernelLib::IVClassVector *vClassVectorSrc = vModelSrc->vrGetClassVector();
    vkernelLib::IVClassVector *vClassVectorDst = vModelDst->vrGetClassVector();
    for (int i=0; i<vClassVectorSrc->vrCount(); i++){
        vkernelLib::IVClass *vClassSrc = vClassVectorSrc->vrItem(i);
        vkernelLib::IVClass *vClassDst = vClassVectorDst->vrLocate(vClassSrc->vrName);
        result += differenceClasses(vClassSrc, vClassDst);
    }
    result += differenceFilters(vModelSrc, vModelDst);
    vkernelLib::IVObjectVector *vObjsSrc = vModelSrc->vrGetObjVector();
    vkernelLib::IVObjectVector *vObjsDst = vModelDst->vrGetObjVector();
    for (int i=0; i<vObjsSrc->vrObjectsCount(); i++) {
         vkernelLib::IVObject *vObjectSrc = vObjsSrc->vrItem(i);
         vkernelLib::IVObject *vObjectDst = vObjsDst->vrGetObjByStrID(vObjectSrc->vrObjStrID());
         result += differenceObjects(vObjectSrc, vObjectDst);
    }

    return result;
}

QString StructuresDifference::differenceFilters (vkernelLib::IVModel *vModelSrc, vkernelLib::IVModel *vModelDst) {
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
