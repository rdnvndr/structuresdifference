#ifndef STRUCTURESDIFFERENCE_H
#define STRUCTURESDIFFERENCE_H

#include <QObject>
#import "libid:CDD8D735-102E-4C19-9892-5CB186D82CC4"
#ifdef _DEBUG
#include "debug/UniReference.tlh"
#else
#include "release/UniReference.tlh"
#endif

#import "libid:001D54E6-3572-405F-8794-79CE321C5F3D"
#ifdef _DEBUG
#include "debug/vkernel.tlh"
#else
#include "release/vkernel.tlh"
#endif

#import "libid:75BC526D-D630-4F45-93E2-0E2545098768"
#ifdef _DEBUG
#include "debug/vkernel_a.tlh"
#else
#include "release/vkernel_a.tlh"
#endif

class StructuresDifference : public QObject
{
    Q_OBJECT
public:
    explicit StructuresDifference(QObject *parent = 0);

    bool connect();
    vkernelLib::IVModel *loadFile(QString filename);

    QString differenceModels(vkernelLib::IVModel *vModelSrc,
                            vkernelLib::IVModel *vModelDst);

    QString differenceClasses(vkernelLib::IVClass *vClassSrc,
                              vkernelLib::IVClass *vClassDst);
    QString differenceClassLinks(vkernelLib::IVClass *vClassSrc,
                                 vkernelLib::IVClass *vClassDst);
    QString differenceFilters(vkernelLib::IVModel *vModelSrc,
                              vkernelLib::IVModel *vModelDst);
    QString differenceClassPerms(vkernelLib::IVClass *vClassSrc,
                                 vkernelLib::IVClass *vClassDst);

    QString differenceAttrs(vkernelLib::IVClassValue *vAttrSrc,
                            vkernelLib::IVClassValue *vAttrDst);
    QString differencePropAttrs(vkernelLib::IVClassValue *vAttrSrc,
                                vkernelLib::IVClassValue *vAttrDst);
    QString differenceAttrGroups(vkernelLib::IVModel *vModelSrc,
                                 vkernelLib::IVModel *vModelDst);
    QString differenceAttrPerms(vkernelLib::IVClassValue *vAttrSrc,
                                vkernelLib::IVClassValue *vAttrDst);

    QString differenceObjects(vkernelLib::IVObject *vObjectSrc,
                              vkernelLib::IVObject *vObjectDst);
    QString differenceAttrObjects(vkernelLib::IVAttribute *attrSrc,
                                  vkernelLib::IVAttribute *attrDst);
    QString differenceObjectLinks(vkernelLib::IVObject *vObjectSrc,
                                  vkernelLib::IVObject *vObjectDst);

    bool differenceIDispatchs(_variant_t varSrc, _variant_t varDst, GUID dataType);

    bool classId() const;
    void setClassId(bool classId);

    bool classBlocked() const;
    void setClassBlocked(bool classBlocked);

    bool classScreenName() const;
    void setClassScreenName(bool classScreenName);

    bool classBaseClass() const;
    void setClassBaseClass(bool classBaseClass);

    bool attrId() const;
    void setAttrId(bool attrId);

    bool attrDataType() const;
    void setAttrDataType(bool attrDataType);

    bool attrType() const;
    void setAttrType(bool attrType);

    bool attrScreenName() const;
    void setAttrScreenName(bool attrScreenName);

    bool attrAliasName() const;
    void setAttrAliasName(bool attrAliasName);

    bool attrFuncRead() const;
    void setAttrFuncRead(bool attrFuncRead);

    bool attrFuncWrite() const;
    void setAttrFuncWrite(bool attrFuncWrite);

    bool attrBlocked() const;
    void setAttrBlocked(bool attrBlocked);

    bool attrMeasureUnit() const;
    void setAttrMeasureUnit(bool attrMeasureUnit);

    bool attrMeasureEntity() const;
    void setAttrMeasureEntity(bool attrMeasureEntity);

    bool attrForbidInput() const;
    void setAttrForbidInput(bool attrForbidInput);

    bool attrBaseClass() const;
    void setAttrBaseClass(bool attrBaseClass);

    bool attrPrecision() const;
    void setAttrPrecision(bool attrPrecision);

    bool attrGroup() const;
    void setAttrGroup(bool attrGroup);

    bool classChild() const;
    void setClassChild(bool classChild);

    bool classFilter() const;
    void setClassFilter(bool classFilter);

    bool classPerms() const;
    void setClassPerms(bool classPerms);

    bool attrProp() const;
    void setAttrProp(bool attrProp);

    bool attrPerms() const;
    void setAttrPerms(bool attrPerms);

    bool permGroup() const;
    void setPermGroup(bool permGroup);

    bool objId() const;
    void setObjId(bool objId);

    bool objOwnerId() const;
    void setObjOwnerId(bool objOwnerId);

    bool objWasChanged() const;
    void setObjWasChanged(bool objWasChanged);

    bool objReadOnly() const;
    void setObjReadOnly(bool objReadOnly);

    bool objAttrName() const;
    void setObjAttrName(bool objAttrName);

    bool objAttrValue() const;
    void setObjAttrValue(bool objAttrValue);

    bool objAttrMeasureUnit() const;
    void setObjAttrMeasureUnit(bool objAttrMeasureUnit);

    bool objAttrPrecision() const;
    void setObjAttrPrecision(bool objAttrPrecision);

    bool objAttrOwnerId() const;
    void setObjAttrOwnerId(bool objAttrOwnerId);

    bool objChilds() const;
    void setObjChilds(bool objChilds);

    bool attrLimit() const;
    void setAttrLimit(bool attrLimit);

private:
    UniReference::IUniRefer* uniRef;

    QList<unsigned long> m_groupsSrc;
    QList<unsigned long> m_groupsDst;
    QStringList m_groupNames;
    vkernelLib::IVLocalSecurity *m_scrtSrc;
    vkernelLib::IVLocalSecurity *m_scrtDst;

    bool m_classId;
    bool m_classBlocked;
    bool m_classScreenName;
    bool m_classBaseClass;
    bool m_classChild;
    bool m_classFilter;
    bool m_classPerms;
    bool m_permGroup;

    bool m_attrId;
    bool m_attrDataType;
    bool m_attrType;
    bool m_attrScreenName;
    bool m_attrAliasName;
    bool m_attrFuncRead;
    bool m_attrFuncWrite;
    bool m_attrBlocked;
    bool m_attrMeasureUnit;
    bool m_attrMeasureEntity;
    bool m_attrForbidInput;
    bool m_attrLimit;
    bool m_attrBaseClass;
    bool m_attrPrecision;
    bool m_attrGroup;
    bool m_attrProp;
    bool m_attrPerms;

    bool m_objId;
    bool m_objOwnerId;
    bool m_objWasChanged;
    bool m_objReadOnly;
    bool m_objChilds;

    bool m_objAttrName;
    bool m_objAttrValue;
    bool m_objAttrMeasureUnit;
    bool m_objAttrPrecision;
    bool m_objAttrOwnerId;

};

#endif // DIFFMODEL_H
