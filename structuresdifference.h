#ifndef STRUCTURESDIFFERENCE_H
#define STRUCTURESDIFFERENCE_H

#include <QObject>
#import "libid:CDD8D735-102E-4C19-9892-5CB186D82CC4"
#include "release/UniReference.tlh"
//#include "debug/UniReference.tlh"

#import "libid:001D54E6-3572-405F-8794-79CE321C5F3D"
#include "release/UniReference.tlh"
//#include "debug/vkernel.tlh"

class StructuresDifference : public QObject
{
    Q_OBJECT
public:
    explicit StructuresDifference(QObject *parent = 0);

    bool connect();
    vkernelLib::IVModel *loadFile(QString filename);

    QString modelDiff(vkernelLib::IVModel *vModelSrc,
                      vkernelLib::IVModel *vModelDst);
    QString filterDiff(vkernelLib::IVModel *vModelSrc,
                      vkernelLib::IVModel *vModelDst);
    QString classDiff(vkernelLib::IVClass *vClassSrc,
                      vkernelLib::IVClass *vClassDst);
    QString childDiff(vkernelLib::IVClass *vClassSrc,
                      vkernelLib::IVClass *vClassDst);
    QString attrDiff(vkernelLib::IVClassValue *vAttrSrc,
                     vkernelLib::IVClassValue *vAttrDst);

    QString groupsDiff(vkernelLib::IVModel *vModelSrc,
                       vkernelLib::IVModel *vModelDst);
    QString classPermDiff(vkernelLib::IVClass *vClassSrc,
                          vkernelLib::IVClass *vClassDst);
    QString attrPermDiff(vkernelLib::IVClassValue *vAttrSrc,
                         vkernelLib::IVClassValue *vAttrDst);
private:
    UniReference::IUniRefer* uniRef;

    QList<unsigned long> m_groupsSrc;
    QList<unsigned long> m_groupsDst;
    QStringList m_groupNames;
    vkernelLib::IVLocalSecurity *m_scrtSrc;
    vkernelLib::IVLocalSecurity *m_scrtDst;

};

#endif // DIFFMODEL_H
