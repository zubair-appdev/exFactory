#include "loginwindow.h"
#include "ui_loginwindow.h"

loginWindow::loginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginWindow),myMainWindow(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Login");
}

loginWindow::~loginWindow()
{
    delete ui;
}


void loginWindow::on_pushButton_login_clicked()
{
    if(ui->lineEdit_userName->text() == "eel" && ui->lineEdit_Password->text() == "eel1234") //eel eel1234
    {
        myMainWindow = new MainWindow();
        myMainWindow->setAttribute(Qt::WA_DeleteOnClose);
        myMainWindow->show();
        this->close();
    }
    else
    {
        QMessageBox::warning(this,"Error","Invalid user");
    }
}
