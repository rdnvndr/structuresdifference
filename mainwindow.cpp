#include "mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include "structuresdifference.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    connect(diffAction, SIGNAL(triggered(bool)), this, SLOT(runDiff()));
    connect(saveLogFileAction, SIGNAL(triggered(bool)), this, SLOT(saveLogFile()));
    connect(openFirstFileAction, SIGNAL(triggered(bool)), this, SLOT(openFirstFile()));
    connect(openSecondFileAction, SIGNAL(triggered(bool)),this, SLOT(openSecondFile()));
    connect(exitAction, SIGNAL(triggered(bool)),this, SLOT(close()));
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

    StructuresDifference *diffModel = new StructuresDifference();
    if (diffModel->connect()) {
         logPlainTextEdit->clear();
         vkernelLib::IVModel *vModelSrc = diffModel->loadFile(m_firstFileName);
         vkernelLib::IVModel *vModelDst = diffModel->loadFile(m_secondFileName);
         logPlainTextEdit->appendPlainText("Первый файл: " + m_firstFileName);
         logPlainTextEdit->appendPlainText("Второй файл: " + m_secondFileName);
         if (vModelDst!=NULL && vModelSrc!=NULL) {
            logPlainTextEdit->appendPlainText(diffModel->groupsDiff(vModelSrc, vModelDst));
            logPlainTextEdit->appendPlainText(diffModel->modelDiff(vModelSrc, vModelDst));
            logPlainTextEdit->appendPlainText("\nПроверка закончена");
         }
    } else
        QMessageBox::warning(this, tr("Ошибка соединения"), tr("Ошибка соединения с сервером приложений"));
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
