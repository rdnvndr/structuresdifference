#include "mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    connect(diffAction, SIGNAL(triggered(bool)), this, SLOT(runDiff()));
    connect(saveLogFileAction, SIGNAL(triggered(bool)), this, SLOT(saveLogFile()));
    connect(openFirstFileAction, SIGNAL(triggered(bool)), this, SLOT(openFirstFile()));
    connect(openSecondFileAction, SIGNAL(triggered(bool)),this, SLOT(openSecondFile()));
    connect(exitAction, SIGNAL(triggered(bool)),this, SLOT(close()));
    m_diffModel = NULL;

    QSettings settings("ASCON", "StructuresDifference");
    settings.beginGroup("DiffOption");

    classIdCheckBox->setChecked(settings.value("classId",true).toBool());
    classBlockedCheckBox->setChecked(settings.value("classBlocked",true).toBool());
    classScreenNameCheckBox->setChecked(settings.value("classScreenName",true).toBool());
    classBaseClassCheckBox->setChecked(settings.value("classBaseClass",true).toBool());
    classChildCheckBox->setChecked(settings.value("classChild",true).toBool());
    classFilterCheckBox->setChecked(settings.value("classFilter",true).toBool());
    classPermsCheckBox->setChecked(settings.value("classPerms",true).toBool());
    permGroupCheckBox->setChecked(settings.value("permGroup",true).toBool());

    attrIdCheckBox->setChecked(settings.value("attrId",true).toBool());
    attrDataTypeCheckBox->setChecked(settings.value("attrDataType",true).toBool());
    attrTypeCheckBox->setChecked(settings.value("attrType",true).toBool());
    attrScreenNameCheckBox->setChecked(settings.value("attrScreenName",true).toBool());
    attrAliasNameCheckBox->setChecked(settings.value("attrAliasName",true).toBool());
    attrFuncReadCheckBox->setChecked(settings.value("attrFuncRead",true).toBool());
    attrFuncWriteCheckBox->setChecked(settings.value("attrFuncWrite",true).toBool());
    attrBlockedCheckBox->setChecked(settings.value("attrBlocked",true).toBool());
    attrMeasureUnitCheckBox->setChecked(settings.value("attrMeasureUnit",true).toBool());
    attrMeasureEntityCheckBox->setChecked(settings.value("attrMeasureEntity",true).toBool());
    attrForbidInputCheckBox->setChecked(settings.value("attrForbidInput",true).toBool());
    attrBaseClassCheckBox->setChecked(settings.value("attrBaseClass",true).toBool());
    attrPrecisionCheckBox->setChecked(settings.value("attrPrecision",true).toBool());
    attrGroupCheckBox->setChecked(settings.value("attrGroup",true).toBool());
    attrPropCheckBox->setChecked(settings.value("attrProp",true).toBool());
    attrPermsCheckBox->setChecked(settings.value("attrPerms",true).toBool());

    objIdCheckBox->setChecked(settings.value("objId",true).toBool());
    objOwnerIdCheckBox->setChecked(settings.value("objOwnerId",true).toBool());
    objWasChangedCheckBox->setChecked(settings.value("objWasChanged",true).toBool());
    objReadOnlyCheckBox->setChecked(settings.value("objReadOnly",true).toBool());
    objChildsCheckBox->setChecked(settings.value("objChilds",true).toBool());

    objAttrNameCheckBox->setChecked(settings.value("objAttrName",true).toBool());
    objAttrValueCheckBox->setChecked(settings.value("objAttrValue",true).toBool());
    objAttrMeasureUnitCheckBox->setChecked(settings.value("objAttrMeasureUnit",true).toBool());
    objAttrPrecisionCheckBox->setChecked(settings.value("objAttrPrecision",true).toBool());
    objAttrOwnerIdCheckBox->setChecked(settings.value("objAttrOwnerId",true).toBool());

    settings.endGroup();

}


void MainWindow::saveLogFile()
{
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Сохранение лога сравнения"),
                                              "", tr("Текстовые файлы (*.txt)"));
    QFile file(fileName);
    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream out(&file);
        out << logPlainTextEdit->toPlainText();
        file.close();
    }
}

void MainWindow::runDiff()
{
    if (!QFile(m_firstFileName).exists()){
        logPlainTextEdit->appendPlainText("Не существует первый файл: " + m_firstFileName);
        return;
    }
    if (!QFile(m_secondFileName).exists()) {
        logPlainTextEdit->appendPlainText("Не существует второй файл: " + m_secondFileName);
        return;
    }

    m_diffModel = new StructuresDifference();
    setCheckBox();
    if (m_diffModel->connect()) {
         logPlainTextEdit->clear();
         vkernelLib::IVModel *vModelSrc = m_diffModel->loadFile(m_firstFileName);
         vkernelLib::IVModel *vModelDst = m_diffModel->loadFile(m_secondFileName);
         logPlainTextEdit->appendPlainText("Первый файл: " + m_firstFileName);
         logPlainTextEdit->appendPlainText("Второй файл: " + m_secondFileName);
         if (vModelDst!=NULL && vModelSrc!=NULL) {
            logPlainTextEdit->appendPlainText(m_diffModel->differenceAttrGroups(vModelSrc, vModelDst));
            logPlainTextEdit->appendPlainText(m_diffModel->differenceModels(vModelSrc, vModelDst));
            logPlainTextEdit->appendPlainText("\nПроверка закончена");
         }
    } else
        QMessageBox::warning(this, tr("Ошибка соединения"), tr("Ошибка соединения с сервером приложений"));
    delete m_diffModel;
}

void MainWindow::openFirstFile()
{
    m_firstFileName = QFileDialog::getOpenFileName(
                this, tr("Открытие файла "),
                "", tr("VTP файл (*.vtp)"));
    logPlainTextEdit->appendPlainText("Выбран первый файл: " + m_firstFileName);
}

void MainWindow::openSecondFile()
{
    m_secondFileName = QFileDialog::getOpenFileName(
                this, tr("Открытие файла "),
                "", tr("VTP файл (*.vtp)"));
    logPlainTextEdit->appendPlainText("Выбран второй файл: " + m_secondFileName);
}

void MainWindow::setCheckBox()
{
    m_diffModel->setClassId(classIdCheckBox->isChecked());
    m_diffModel->setClassBlocked(classBlockedCheckBox->isChecked());
    m_diffModel->setClassScreenName(classScreenNameCheckBox->isChecked());
    m_diffModel->setClassBaseClass(classBaseClassCheckBox->isChecked());
    m_diffModel->setClassChild(classChildCheckBox->isChecked());
    m_diffModel->setClassFilter(classFilterCheckBox->isChecked());
    m_diffModel->setClassPerms(classPermsCheckBox->isChecked());
    m_diffModel->setPermGroup(permGroupCheckBox->isChecked());

    m_diffModel->setAttrId(attrIdCheckBox->isChecked());
    m_diffModel->setAttrDataType(attrDataTypeCheckBox->isChecked());
    m_diffModel->setAttrType(attrTypeCheckBox->isChecked());
    m_diffModel->setAttrScreenName(attrScreenNameCheckBox->isChecked());
    m_diffModel->setAttrAliasName(attrAliasNameCheckBox->isChecked());
    m_diffModel->setAttrFuncRead(attrFuncReadCheckBox->isChecked());
    m_diffModel->setAttrFuncWrite(attrFuncWriteCheckBox->isChecked());
    m_diffModel->setAttrBlocked(attrBlockedCheckBox->isChecked());
    m_diffModel->setAttrMeasureUnit(attrMeasureUnitCheckBox->isChecked());
    m_diffModel->setAttrMeasureEntity(attrMeasureEntityCheckBox->isChecked());
    m_diffModel->setAttrForbidInput(attrForbidInputCheckBox->isChecked());
    m_diffModel->setAttrBaseClass(attrBaseClassCheckBox->isChecked());
    m_diffModel->setAttrPrecision(attrPrecisionCheckBox->isChecked());
    m_diffModel->setAttrGroup(attrGroupCheckBox->isChecked());
    m_diffModel->setAttrProp(attrPropCheckBox->isChecked());
    m_diffModel->setAttrPerms(attrPermsCheckBox->isChecked());

    m_diffModel->setObjId(objIdCheckBox->isChecked());
    m_diffModel->setObjOwnerId(objOwnerIdCheckBox->isChecked());
    m_diffModel->setObjWasChanged(objWasChangedCheckBox->isChecked());
    m_diffModel->setObjReadOnly(objReadOnlyCheckBox->isChecked());
    m_diffModel->setObjChilds(objChildsCheckBox->isChecked());

    m_diffModel->setObjAttrName(objAttrNameCheckBox->isChecked());
    m_diffModel->setObjAttrValue(objAttrValueCheckBox->isChecked());
    m_diffModel->setObjAttrMeasureUnit(objAttrMeasureUnitCheckBox->isChecked());
    m_diffModel->setObjAttrPrecision(objAttrPrecisionCheckBox->isChecked());
    m_diffModel->setObjAttrOwnerId(objAttrOwnerIdCheckBox->isChecked());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("ASCON", "StructuresDifference");
    settings.beginGroup("DiffOption");
    settings.setValue("classId", classIdCheckBox->isChecked());
    settings.setValue("classBlocked", classBlockedCheckBox->isChecked());
    settings.setValue("classScreenName", classScreenNameCheckBox->isChecked());
    settings.setValue("classBaseClass", classBaseClassCheckBox->isChecked());
    settings.setValue("classChild", classChildCheckBox->isChecked());
    settings.setValue("classFilter", classFilterCheckBox->isChecked());
    settings.setValue("classPerms", classPermsCheckBox->isChecked());
    settings.setValue("permGroup", permGroupCheckBox->isChecked());

    settings.setValue("attrId", attrIdCheckBox->isChecked());
    settings.setValue("attrDataType", attrDataTypeCheckBox->isChecked());
    settings.setValue("attrType", attrTypeCheckBox->isChecked());
    settings.setValue("attrScreenName", attrScreenNameCheckBox->isChecked());
    settings.setValue("attrAliasName", attrAliasNameCheckBox->isChecked());
    settings.setValue("attrFuncRead", attrFuncReadCheckBox->isChecked());
    settings.setValue("attrFuncWrite", attrFuncWriteCheckBox->isChecked());
    settings.setValue("attrBlocked", attrBlockedCheckBox->isChecked());
    settings.setValue("attrMeasureUnit", attrMeasureUnitCheckBox->isChecked());
    settings.setValue("attrMeasureEntity", attrMeasureEntityCheckBox->isChecked());
    settings.setValue("attrForbidInput", attrForbidInputCheckBox->isChecked());
    settings.setValue("attrBaseClass", attrBaseClassCheckBox->isChecked());
    settings.setValue("attrPrecision", attrPrecisionCheckBox->isChecked());
    settings.setValue("attrGroup", attrGroupCheckBox->isChecked());
    settings.setValue("attrProp", attrPropCheckBox->isChecked());
    settings.setValue("attrPerms", attrPermsCheckBox->isChecked());

    settings.setValue("objId", objIdCheckBox->isChecked());
    settings.setValue("objOwnerId", objOwnerIdCheckBox->isChecked());
    settings.setValue("objWasChanged", objWasChangedCheckBox->isChecked());
    settings.setValue("objReadOnly", objReadOnlyCheckBox->isChecked());
    settings.setValue("objChilds", objChildsCheckBox->isChecked());

    settings.setValue("objAttrName", objAttrNameCheckBox->isChecked());
    settings.setValue("objAttrValue", objAttrValueCheckBox->isChecked());
    settings.setValue("objAttrMeasureUnit", objAttrMeasureUnitCheckBox->isChecked());
    settings.setValue("objAttrPrecision", objAttrPrecisionCheckBox->isChecked());
    settings.setValue("objAttrOwnerId", objAttrOwnerIdCheckBox->isChecked());

    settings.endGroup();
    event->accept();
}
