#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void saveLogFile();
    void runDiff();
    void openFirstFile();
    void openSecondFile();

private:
    QString m_firstFileName;
    QString m_secondFileName;
};

#endif // MAINWINDOW_H
