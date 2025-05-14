#include "serialporthandler.h"

serialPortHandler::serialPortHandler(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort;
    connect(serial, &QSerialPort::readyRead, this, &serialPortHandler::readData);

    // Initialize the timeout timer
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true); // Timer should fire only once
    connect(timeoutTimer, &QTimer::timeout, this, &serialPortHandler::handleTimeout);

}

serialPortHandler::~serialPortHandler()
{
    delete serial;
}

QStringList serialPortHandler::availablePorts()
{
    QStringList ports;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ports<<info.portName();
    }
    return ports;
}

void serialPortHandler::setPORTNAME(const QString &portName)
{
    buffer.clear();

    if(serial->isOpen())
    {
        serial->close();
    }

    serial->setPortName(portName);
    serial->setBaudRate(921600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);


    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug()<<"Failed to open port"<<serial->portName();
        emit portOpening("Failed to open port "+serial->portName());
    }
    else
    {
        qDebug() << "Serial port "<<serial->portName()<<" opened successfully at baud rate 921600";
        emit portOpening("Serial port "+serial->portName()+" opened successfully at baud rate 921600");
    }
}



void serialPortHandler::readData()
{
    timeoutTimer->stop(); // Stop the timer if data is received

    qDebug()<<"------------------------------------------------------------------------------------";

    QByteArray ResponseData;

    buffer.append(serial->readAll());

    qDebug()<<buffer.toHex()<<" Raw buffer data";
    qDebug()<<buffer.size()<<" :size";
    emit portOpening("Raw readyRead data: "+buffer.toHex());


    //Direct taking msgId from mainWindow
    quint8 msgId = id;

    //for special cases : large data
    if(msgId == 0x01)
    {
        qDebug()<<"In Enter Factory Settings";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 4)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in Enter Factory Settings";
        }
    }
    else  if(msgId == 0x02)
    {
        qDebug()<<"In G_Set Value";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 4)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in G_Set Value";
        }
    }
    else  if(msgId == 0x03)
    {
        qDebug()<<"In Delay_Set Value";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 4)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in G_Set Value";
        }
    }
    else  if(msgId == 0x04)
    {
        qDebug()<<"In Set Vmin and Vmax";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 4)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in Set Vmin Vmax";
        }
    }
    else  if(msgId == 0x05)
    {
        qDebug()<<"In Exit Factory Settings";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 4)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in Exit Factory Settings";
        }
    }
    else  if(msgId == 0x06)
    {
        qDebug()<<"In Know Your Settings";
        qDebug() << "Extracted msgId:" <<hex<<msgId;

        if(buffer.size() == 14)
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else if(buffer.size() == 5 && buffer.left(5).toHex() == "4e41434b07")
        {
            ResponseData = buffer;
            buffer.clear();
        }
        else
        {
            qDebug()<<"Unknown size in Know Your Settings";
        }
    }
    else
    {
        //do nothing
        qDebug()<<"do nothing not a specified size";
        emit portOpening("Invalid msdId");
    }


    switch(msgId)
    {
    case 0x01:
    {
        if(ResponseData.left(4).toHex() == "41434b49")
        {
            emit portOpening("Enter Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_01");
            emit portOpening("SEND2");
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("Enter Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_01");
        }
        else
        {
            emit portOpening("Enter Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for Enter Factory Settings");
        }

    }
        break;

    case 0x02:
    {
        if(ResponseData.left(4).toHex() == "41434b49")
        {
            emit portOpening("G_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_02");
            emit portOpening("SEND3");
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("G_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_02");
        }
        else
        {
            emit portOpening("G_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for G_Set Value");
        }

    }
        break;

    case 0x03:
    {
        if(ResponseData.left(4).toHex() == "41434b49")
        {
            emit portOpening("Delay_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_03");
            emit portOpening("SEND4");
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("Delay_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_03");
        }
        else
        {
            emit portOpening("Delay_Set Value bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for Delay_Set Value");
        }

    }
        break;

    case 0x04:
    {
        if(ResponseData.left(4).toHex() == "41434b49")
        {
            emit portOpening("Set Vmin Vmax Value bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_04");
            emit portOpening("SEND5");
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("Set Vmin Vmax Value bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_04");
        }
        else
        {
            emit portOpening("Set Vmin Vmax Value bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for Set Vmin Vmax Value");
        }

    }
        break;
    case 0x05:
    {
        if(ResponseData.left(4).toHex() == "41434b49")
        {
            emit portOpening("Exit Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_05");
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("Exit Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_05");
        }
        else
        {
            emit portOpening("Exit Factory Settings bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for Exit Factory Settings");
        }
    }
        break;
    case 0x06:
    {
        if(ResponseData.left(3).toHex() == "4a060a")
        {
            emit portOpening("Know Your Settings bytes received : "+ResponseData.toHex());
            emit portOpening("ACK_06");
            emit portOpening(ResponseData.toHex());
        }
        else if(ResponseData.left(5).toHex() == "4e41434b07")
        {
            emit portOpening("Know Your Settings bytes received : "+ResponseData.toHex());
            emit portOpening("NACK_06");
        }
        else
        {
            emit portOpening("Know Your Settings bytes received : "+ResponseData.toHex());
            emit portOpening("Unknown data# for Know Your Settings");
        }
    }
        break;


    default:
    {
        qDebug() << "Unknown msgId: " <<hex << msgId << " with data: " << ResponseData.toHex();
        emit portOpening("Unknown msgId : " + QString::number(msgId,16)+" with data: "+ResponseData.toHex());
    }
    }

}

void serialPortHandler::recvMsgId(quint8 id)
{
    qDebug() << "Received id:" <<hex<< id;
    this->id = id;
    buffer.clear();
}

void serialPortHandler::handleTimeout()
{
    qDebug() << "Timeout: No data received!";
    emit hardwareTimeout(); // Emit signal to inform about the timeout
}
