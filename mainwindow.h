#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <serialporthandler.h>
#include <QMessageBox>
#include <QByteArray>
#include <QTimer>
#include <QThread>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QDateTime>
#include <QToolTip>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refreshPorts();

    quint8 calculateChecksum(const QByteArray &data);

    float convertBytesToFloat(const QByteArray &data);

    void setDefault();

    void on_pushButton_enterFacSet_clicked();

    void on_pushButton_GSetValue_clicked();

    void on_pushButton_DeltaSetVal_clicked();

    void on_pushButton_SetVminVmax_clicked();

    void on_pushButton_ExitFacSet_clicked();

    void initialZeros();

private slots:
        void onPortSelected(const QString &portName);

        void portStatus(const QString&);

        void on_pushButton_KnowYourSet_clicked();

        void on_pushButton_loadData_clicked();

        void on_pushButton_saveAsPDF_clicked();

        void checkDeltaSetValRange();

        void checkGSetLimitRange();

        void checkVminValRange();

        void checkVmaxValRange();

        void showTimeoutMessage();

signals:
    void sendMsgId(quint8 id);

private:
    Ui::MainWindow *ui;
    serialPortHandler *serialObj;
    quint8 myId;

    QStringList integrityChecker;

};
#endif // MAINWINDOW_H
