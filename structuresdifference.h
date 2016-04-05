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


private:
    UniReference::IUniRefer* uniRef;

    QList<unsigned long> m_groupsSrc;
    QList<unsigned long> m_groupsDst;
    QStringList m_groupNames;
    vkernelLib::IVLocalSecurity *m_scrtSrc;
    vkernelLib::IVLocalSecurity *m_scrtDst;

};

#endif // DIFFMODEL_H
