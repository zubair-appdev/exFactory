#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class loginWindow;
}

class loginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit loginWindow(QWidget *parent = nullptr);
    ~loginWindow();

private slots:

    void on_pushButton_login_clicked();

private:
    Ui::loginWindow *ui;

    MainWindow *myMainWindow;
};

#endif // LOGINWINDOW_H
