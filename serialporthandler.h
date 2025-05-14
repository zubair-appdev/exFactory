#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QVector>
#include <QTimer>

class serialPortHandler : public QObject
{
    Q_OBJECT
public:
    explicit serialPortHandler(QObject *parent = nullptr);
     ~serialPortHandler();

    void writeData(const QByteArray &data)
    {
        if(!serial->isOpen())
        {
            qDebug() << "Serial object is not initialized";
            emit portOpening("Serial object is not initialized/port not selected");
            return;
        }
        else
        {
            if(serial->isOpen())
            {

                buffer.clear();
                serial->write(data);

                // Start the timeout timer
                if(id == 6)
                {
                    timeoutTimer->start(2000); // 2000ms = 2 seconds timeout for get data
                }

                if(id == 1)
                {
                    timeoutTimer->start(6000); // 6000ms = 6 seconds timeout for load data
                }

            }
        }
    }

    QStringList availablePorts();

    void setPORTNAME(const QString &portName);


signals:

    void portOpening(const QString &); //signal for dumping data from serialPortHandler to textEdit_RawBytes
    void hardwareTimeout(); // Signal for timeout


private slots:

    void readData();
    void handleTimeout(); // Slot to handle timeout

public slots:

    void recvMsgId(quint8 id);

private:
    QSerialPort *serial;
    QByteArray  buffer;

    quint8 id;

    QTimer *timeoutTimer; // Timer for detecting timeout
};

#endif // SERIALPORTHANDLER_H
