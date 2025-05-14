#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->groupBox_4->hide();

    ui->lineEdit_serialNo->setText("1");

    // Set tooltips for the valid ranges
    ui->spinBox_Detla_SetVal->setToolTip("Valid range: 60-150");
    ui->spinBox_gSetLimit->setToolTip("Valid range: 100-200");
    ui->doubleSpinBox_VminVal->setToolTip("Valid range: 11.5-12.5");
    ui->doubleSpinBox_vMaxVal->setToolTip("Valid range: 11.5-12.5");

    // Connect valueChanged signals to the respective slots
    connect(ui->spinBox_Detla_SetVal, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::checkDeltaSetValRange);
    connect(ui->spinBox_gSetLimit, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::checkGSetLimitRange);
    connect(ui->doubleSpinBox_VminVal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::checkVminValRange);
    connect(ui->doubleSpinBox_vMaxVal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::checkVmaxValRange);


    ui->comboBox_ports->setFocus();
    // Set the tab order
    setTabOrder(ui->comboBox_ports, ui->lineEdit_serialNo);             // From comboBoxPorts to serial
    setTabOrder(ui->lineEdit_serialNo, ui->spinBox_Detla_SetVal);       // From serial to spinBox_Detla_SetVal
    setTabOrder(ui->spinBox_Detla_SetVal, ui->spinBox_gSetLimit);       // From spinBox_Detla_SetVal to spinBox_gSetLimit
    setTabOrder(ui->spinBox_gSetLimit, ui->doubleSpinBox_VminVal);      // From spinBox_gSetLimit to doubleSpinBox_VminVal
    setTabOrder(ui->doubleSpinBox_VminVal, ui->doubleSpinBox_vMaxVal);  // From doubleSpinBox_VminVal to doubleSpinBox_vMaxVal


    integrityChecker.clear();

    setWindowTitle("Factory Settings");

    serialObj =   new serialPortHandler(this);

    connect(ui->pushButton_clear,&QPushButton::clicked,ui->textEdit_rawBytes,&QTextEdit::clear);

    ui->comboBox_ports->addItems(serialObj->availablePorts());

    connect(ui->pushButton_portsRefresh,&QPushButton::clicked,this,&MainWindow::refreshPorts);

    connect(ui->comboBox_ports,SIGNAL(activated(const QString &)),this,SLOT(onPortSelected(const QString &)));

    connect(this,&MainWindow::sendMsgId,serialObj,&serialPortHandler::recvMsgId);


    //debugging signals
    connect(serialObj,&serialPortHandler::portOpening,this,&MainWindow::portStatus);

    // In MainWindow constructor or initialization
    connect(serialObj, &serialPortHandler::hardwareTimeout, this, &MainWindow::showTimeoutMessage);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete serialObj;
}

void MainWindow::refreshPorts()
{
    QString currentPort = ui->comboBox_ports->currentText();

    qDebug()<<"Refreshing ports...";
    ui->comboBox_ports->clear();
    QStringList availablePorts;
    ui->comboBox_ports->addItems(serialObj->availablePorts());

    ui->comboBox_ports->setCurrentText(currentPort);
}

float MainWindow::convertBytesToFloat(const QByteArray &data)
{

    // Assuming little-endian format
    QByteArray floatBytes = data;
    qDebug()<<floatBytes.toHex()<<"hello";
    std::reverse(floatBytes.begin(), floatBytes.end()); // Convert to big-endian if needed

    float voltage;
    memcpy(&voltage, floatBytes.constData(), sizeof(float));
    return voltage;
}

void MainWindow::setDefault()
{
    ui->label_gSetStat->setText("");
    ui->label_DeltaSetVal->setText("");
    ui->label_setVminVmax->setText("");

    ui->label_gSetStat->setStyleSheet("background-color : none; border : 1px solid gray");
    ui->label_DeltaSetVal->setStyleSheet("background-color : none; border : 1px solid gray");
    ui->label_setVminVmax->setStyleSheet("background-color : none; border : 1px solid gray");
}

void MainWindow::onPortSelected(const QString &portName)
{
    serialObj->setPORTNAME(portName);
}

quint8 MainWindow::calculateChecksum(const QByteArray &data)
{
    quint8 checkSum = 0;
    for(quint8 byte : data)
    {
        checkSum ^= byte;
    }

    ui->textEdit_rawBytes->append("CheckSum : "+QString::number(checkSum,16).toUpper());
    return checkSum;
}


void MainWindow::portStatus(const QString &data)
{
    if(data.startsWith("Serial object is not initialized/port not selected"))
    {
        QMessageBox::critical(this,"Port Error","Please Select Port Using Above Dropdown");
    }

    if(data.startsWith("Serial port ") && data.endsWith(" opened successfully at baud rate 921600"))
    {
        QMessageBox::information(this,"Success",data);
    }

    if(data.startsWith("Failed to open port"))
    {
        QMessageBox::critical(this,"Error",data);
    }

    ui->textEdit_rawBytes->append(data);
    if(data == "ACK_01")
    {
       ui->textEdit_rawBytes->append("Enter Factory Settings Command Executed Successfully");
    }
    else if(data == "NACK_01")
    {
        ui->textEdit_rawBytes->append("Enter Factory Settings Command Failed");
    }
    else if(data == "ACK_02")
    {
        integrityChecker.append(data);

        ui->label_gSetStat->setText("ACK");
        ui->label_gSetStat->setStyleSheet("QLabel { background-color : lightgreen; }");
        QTimer::singleShot(300, this, [this]() {
                   ui->label_gSetStat->setStyleSheet("QLabel { background-color : #c7ffc7; }");
               });
    }
    else if(data == "NACK_02")
    {
        ui->label_gSetStat->setText("NACK");
        ui->label_gSetStat->setStyleSheet("QLabel { background-color : #FF6666; }");
        QTimer::singleShot(300,this,[this]() {
            ui->label_gSetStat->setStyleSheet("QLabel { background-color : #ffc9d1; }");
        });
    }
    else if(data == "ACK_03")
    {
        integrityChecker.append(data);

        ui->label_DeltaSetVal->setText("ACK");
        ui->label_DeltaSetVal->setStyleSheet("QLabel { background-color : lightgreen; }");
        QTimer::singleShot(300, this, [this]() {
                   ui->label_DeltaSetVal->setStyleSheet("QLabel { background-color : #c7ffc7; }");
               });
    }
    else if(data == "NACK_03")
    {
        ui->label_DeltaSetVal->setText("NACK");
        ui->label_DeltaSetVal->setStyleSheet("QLabel { background-color : #FF6666; }");
        QTimer::singleShot(300,this,[this]() {
            ui->label_DeltaSetVal->setStyleSheet("QLabel { background-color : #ffc9d1; }");
        });
    }
    else if(data == "ACK_04")
    {
        integrityChecker.append(data);

        ui->label_setVminVmax->setText("ACK");
        ui->label_setVminVmax->setStyleSheet("QLabel { background-color : lightgreen; }");
        QTimer::singleShot(300, this, [this]() {
                   ui->label_setVminVmax->setStyleSheet("QLabel { background-color : #c7ffc7; }");
               });
    }
    else if(data == "NACK_04")
    {
        ui->label_setVminVmax->setText("NACK");
        ui->label_setVminVmax->setStyleSheet("QLabel { background-color : #FF6666; }");
        QTimer::singleShot(300,this,[this]() {
            ui->label_setVminVmax->setStyleSheet("QLabel { background-color : #ffc9d1; }");
        });
    }
    else if(data == "ACK_05")
    {
        integrityChecker.append(data);

        ui->textEdit_rawBytes->append("Exit Factory Settings Command Executed Successfully");

        qDebug()<<integrityChecker<<"------------------------------";

        if(integrityChecker[0] == "ACK_02" && integrityChecker[1] == "ACK_03" &&
                integrityChecker[2] == "ACK_04" && integrityChecker[3] == "ACK_05")
        {
            QTimer::singleShot(500, this, [this]() {
                QMessageBox::information(this,"Success","Data Uploaded Successfully");
                integrityChecker.clear();
            });

            QTimer::singleShot(600, this, [this]() {
                setDefault();
            });

        }
        else
        {
            QTimer::singleShot(500, this, [this]() {
                QMessageBox::critical(this,"Error","Data Not Uploaded");
                integrityChecker.clear();
            });
        }

    }
    else if(data == "NACK_05")
    {
        ui->textEdit_rawBytes->append("Exit Factory Settings Command Failed");
    }
    else if(data == "ACK_06" || data.startsWith("4a060a"))
    {
        ui->textEdit_rawBytes->append("Get Data Command Executed Successfully");

        //For blinking effect
        QTimer::singleShot(300, this, [this]() {
                   ui->spinBox_Detla_SetVal->setStyleSheet("QSpinBox { background-color: none; }");
               });
        QTimer::singleShot(300, this, [this]() {
                   ui->spinBox_gSetLimit->setStyleSheet("QSpinBox { background-color: none; }");
               });
        QTimer::singleShot(300, this, [this]() {
                   ui->doubleSpinBox_VminVal->setStyleSheet("QDoubleSpinBox { background-color: none; }");
               });
        QTimer::singleShot(300, this, [this]() {
                   ui->doubleSpinBox_vMaxVal->setStyleSheet("QDoubleSpinBox { background-color: none; }");
               });


        ui->spinBox_Detla_SetVal->setValue((data.mid(6,2).toInt(nullptr, 16)));
        ui->spinBox_Detla_SetVal->setStyleSheet("QSpinBox { background-color: lightgreen; }");

        ui->spinBox_gSetLimit->setValue(data.mid(8,2).toInt(nullptr,16));
        ui->spinBox_gSetLimit->setStyleSheet("QSpinBox { background-color: lightgreen; }");

        float Vmin = convertBytesToFloat(QByteArray::fromHex(data.mid(10,8).toUtf8()));
        ui->doubleSpinBox_VminVal->setValue(Vmin);
        ui->doubleSpinBox_VminVal->setStyleSheet("QDoubleSpinBox { background-color: lightgreen; }");

        float Vmax = convertBytesToFloat(QByteArray::fromHex(data.mid(18,8).toUtf8()));
        ui->doubleSpinBox_vMaxVal->setValue(Vmax);
        ui->doubleSpinBox_vMaxVal->setStyleSheet("QDoubleSpinBox { background-color: lightgreen; }");

    }
    else if(data == "NACK_06")
    {
        ui->textEdit_rawBytes->append("Get Data Command Failed");
    }
    else if(data.startsWith("Unknown data#"))
    {
        if(myId == 0x01)
        {
            ui->textEdit_rawBytes->append("Unknown Data in Enter Factory Settings");
        }
        else if(myId == 0x02)
        {
            ui->label_gSetStat->setText("Status");
            ui->label_gSetStat->setStyleSheet("QLabel { background-color : lightgray; }");
        }
        else if(myId == 0x03)
        {
            ui->label_DeltaSetVal->setText("Status");
            ui->label_DeltaSetVal->setStyleSheet("QLabel { background-color : lightgray; }");
        }
        else if(myId == 0x04)
        {
            ui->label_setVminVmax->setText("Status");
            ui->label_setVminVmax->setStyleSheet("QLabel { background-color : lightgray; }");
        }
        else if(myId == 0x05)
        {
            ui->textEdit_rawBytes->append("Unknown data in Exit Factory Settings");
        }
        else if(myId == 0x06)
        {
            ui->textEdit_rawBytes->append("Unknown data in Get Data Command");
        }
    }
    else if(data == "SEND2")
    {
        on_pushButton_GSetValue_clicked();
    }
    else if(data == "SEND3")
    {
        on_pushButton_DeltaSetVal_clicked();
    }
    else if(data == "SEND4")
    {
        on_pushButton_SetVminVmax_clicked();
    }
    else if(data == "SEND5")
    {
        on_pushButton_ExitFacSet_clicked();

    }

}


void MainWindow::on_pushButton_enterFacSet_clicked()
{

    quint8 myId;
    ui->textEdit_rawBytes->append("-----------------------------------------------------------");
    QByteArray command;

    //header
    command.append(0x4a);

    //command code
    command.append(0x01);
    command.append(0xff);

    quint8 checkSum = calculateChecksum(command);

    //checksum
    command.append(checkSum); //total 4 bytes

    ui->textEdit_rawBytes->append("Enter Factory Settings sent bytes: "+command.toHex());

    emit sendMsgId(0x01);
    serialObj->writeData(command);

    myId = 0x01;
    this->myId = myId;
}

void MainWindow::on_pushButton_GSetValue_clicked()
{
    quint8 myId;
    ui->textEdit_rawBytes->append("-----------------------------------------------------------");

    QByteArray command;

    //header
    command.append(0x4a);
    command.append(0x02);

    //G value
    command.append(static_cast<quint8>(ui->spinBox_gSetLimit->value()));

    quint8 checkSum = calculateChecksum(command);

    //checksum
    command.append(checkSum); //total 4 bytes

    ui->textEdit_rawBytes->append("G_Set Value sent bytes: "+command.toHex());

    emit sendMsgId(0x02);
    serialObj->writeData(command);

    myId = 0x02;
    this->myId = myId;
}

void MainWindow::on_pushButton_DeltaSetVal_clicked()
{
    quint8 myId;
    ui->textEdit_rawBytes->append("-----------------------------------------------------------");

    QByteArray command;

    //header
    command.append(0x4a);
    command.append(0x03);

    //Delay_Set value
    command.append(static_cast<quint8>(ui->spinBox_Detla_SetVal->value()));

    quint8 checkSum = calculateChecksum(command);

    //checksum
    command.append(checkSum); //total 4 bytes

    ui->textEdit_rawBytes->append("Delay_Set Value sent bytes: "+command.toHex());

    emit sendMsgId(0x03);
    serialObj->writeData(command);

    myId = 0x03;
    this->myId = myId;


}

void MainWindow::on_pushButton_SetVminVmax_clicked()
{

    quint8 myId;
    ui->textEdit_rawBytes->append("-----------------------------------------------------------");

    QByteArray command;

    //header
    command.append(0xe4);
    command.append(0x04);

    // Convert the float to its binary representation
    float vMinValue = static_cast<float>(ui->doubleSpinBox_VminVal->value());
    float vMaxValue = static_cast<float>(ui->doubleSpinBox_vMaxVal->value());

    // Use a QDataStream to write the floats to a QByteArray
    QByteArray vMinBytes;
    QDataStream vMinStream(&vMinBytes, QIODevice::WriteOnly);
    vMinStream.setFloatingPointPrecision(QDataStream::SinglePrecision); // Single precision (4 bytes)
    vMinStream << vMinValue;

    QByteArray vMaxBytes;
    QDataStream vMaxStream(&vMaxBytes, QIODevice::WriteOnly);
    vMaxStream.setFloatingPointPrecision(QDataStream::SinglePrecision); // Single precision (4 bytes)
    vMaxStream << vMaxValue;

    // Append the binary data to the command
    command.append(vMinBytes); // 4 bytes
    command.append(vMaxBytes); // 4 bytes

    quint8 checkSum = calculateChecksum(command);

    //checksum
    command.append(checkSum); //total 11 bytes

    ui->textEdit_rawBytes->append("Set Vmin Vmax sent bytes: "+command.toHex());

    emit sendMsgId(0x04);
    serialObj->writeData(command);


    myId = 0x04;
    this->myId = myId;
}

void MainWindow::on_pushButton_ExitFacSet_clicked()
{
        quint8 myId;
        ui->textEdit_rawBytes->append("-----------------------------------------------------------");

        QByteArray command;

        //header
        command.append(0x4a);

        //command code
        command.append(0x05);
        command.append(0xaa);

        quint8 checkSum = calculateChecksum(command);

        //checksum
        command.append(checkSum); //total 4 bytes

        ui->textEdit_rawBytes->append("Exit Factory Settings sent bytes: "+command.toHex());

        emit sendMsgId(0x05);
        serialObj->writeData(command);

        myId = 0x05;
        this->myId = myId;

}

void MainWindow::initialZeros()
{
    ui->spinBox_Detla_SetVal->setValue(0);
    ui->spinBox_gSetLimit->setValue(0);
    ui->doubleSpinBox_VminVal->setValue(0.00);
    ui->doubleSpinBox_vMaxVal->setValue(0.00);
}

void MainWindow::on_pushButton_KnowYourSet_clicked()
{
    ui->spinBox_Detla_SetVal->setFixedHeight(24);
    ui->spinBox_gSetLimit->setFixedHeight(24);
    ui->doubleSpinBox_VminVal->setFixedHeight(24);
    ui->doubleSpinBox_vMaxVal->setFixedHeight(24);

    quint8 myId;
    ui->textEdit_rawBytes->append("-----------------------------------------------------------");

    QByteArray command;

    //header
    command.append(0x4a);

    command.append(0x06);
    command.append(0xff);

    quint8 checkSum = calculateChecksum(command);

    //checksum
    command.append(checkSum); //total 4 bytes

    ui->textEdit_rawBytes->append("Know Your Settings sent bytes: "+command.toHex());

    emit sendMsgId(0x06);
    serialObj->writeData(command);


    myId = 0x06;
    this->myId = myId;
}


void MainWindow::on_pushButton_loadData_clicked()
{
    // Variables to store the current values
    int gSetLimit = ui->spinBox_gSetLimit->value();
    int deltaSetVal = ui->spinBox_Detla_SetVal->value();
    double VminVal = ui->doubleSpinBox_VminVal->value();
    double VmaxVal = ui->doubleSpinBox_vMaxVal->value();

    // Check if any of the values are out of range
    bool isOutOfRange = false;
    QString outOfRangeMessage;

    if (deltaSetVal < 60 || deltaSetVal > 150) {
        outOfRangeMessage += "Delta Value out of range! Valid range: 60-150.\n";
        isOutOfRange = true;
    }
    if (gSetLimit < 100 || gSetLimit > 200) {
        outOfRangeMessage += "G Value out of range! Valid range: 100-200.\n";
        isOutOfRange = true;
    }
    if (VminVal < 11.5 || VminVal > 12.5) {
        outOfRangeMessage += "Vmin Value out of range! Valid range: 11.5-12.5.\n";
        isOutOfRange = true;
    }
    if (VmaxVal < 11.5 || VmaxVal > 12.5) {
        outOfRangeMessage += "Vmax Value out of range! Valid range: 11.5-12.5.\n";
        isOutOfRange = true;
    }

    // If any value is out of range, show a message box
    if (isOutOfRange) {
        QMessageBox::warning(this, "Values Out of Range", outOfRangeMessage);
        return; // Stop further execution if values are out of range
    }

    // If all values are within range, proceed

    integrityChecker.clear();
    on_pushButton_enterFacSet_clicked();
}

void MainWindow::on_pushButton_saveAsPDF_clicked()
{

    QString serialNo = ui->lineEdit_serialNo->text();

    if(ui->lineEdit_serialNo->text().isEmpty())
    {
        QMessageBox::warning(this,"Warning","Please Enter Serial No To Save As PDF");
    }
    else
    {
        // Capture data from spinboxes
        int spinBoxDelayVal = ui->spinBox_Detla_SetVal->value();
        int spinBoxGValue = ui->spinBox_gSetLimit->value();
        double doubleSpinBoxVminVal = ui->doubleSpinBox_VminVal->value();
        double doubleSpinBoxVmaxVal = ui->doubleSpinBox_vMaxVal->value();

        // File dialog to select the save location
        QString fileName = QFileDialog::getSaveFileName(this, "Save PDF", "", "*.pdf");
        if (fileName.isEmpty()) return;

        // Ensure the file has a .pdf extension
        if (QFileInfo(fileName).suffix().isEmpty()) {
            fileName.append(".pdf");
        }

        // Create and configure the PDF writer
        QPdfWriter pdfWriter(fileName);
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setPageMargins(QMargins(30, 30, 30, 30));

        QPainter painter(&pdfWriter);


        // Set thicker pen for table lines
        QPen pen(Qt::black);
        pen.setWidth(3);  // Set the line thickness here
        painter.setPen(pen);

        int x = 200;
        int y = 300;
        int ySpace = 400;

        // Draw border around the entire page
        QRect pageRect(0, 0, pdfWriter.width(), pdfWriter.height());
        painter.drawRect(pageRect);

        // Add the header information

        // Set bold font with 14 px size
        QFont fontBold("Times New Roman", 14, QFont::Bold);
        painter.setFont(fontBold);

        painter.drawText(x+1450, y+=ySpace, "MRPKS AT FUZE INTERFACE CONTROL REPORT");

        // Set normal font and start writing text
        QFont font("Times New Roman", 12);
        painter.setFont(font);


        painter.drawText(x+6200, y+=ySpace+300, "Date : " + QDate::currentDate().toString("dd-MM-yyyy"));
        painter.drawText(x+6200, y+=ySpace, "Time : " + QTime::currentTime().toString("hh:mm:ss AP"));

        painter.drawText(x, y+=ySpace, "Serial No : " + serialNo);

        // Define table dimensions
        int tableStartX = x;
        int tableStartY = y + ySpace;
        int rowHeight = 400;
        int colWidth1 = 1000;
        int colWidth2 = 2000;
        int colWidth3 = 2000;
        int colWidth4 = 1000;

        // Draw the table grid
        int currentY = tableStartY;
        for (int i = 0; i <= 4; ++i) {  // 5 rows (1 header + 4 data rows)
            painter.drawLine(tableStartX, currentY, tableStartX + colWidth1 + colWidth2 + colWidth3 + colWidth4, currentY);
            currentY += rowHeight;
        }

        painter.drawLine(tableStartX, currentY, tableStartX + colWidth1 + colWidth2 + colWidth3 + colWidth4, currentY);


        // Draw vertical lines
        painter.drawLine(tableStartX, tableStartY, tableStartX, currentY);  // First vertical line
        painter.drawLine(tableStartX + colWidth1, tableStartY, tableStartX + colWidth1, currentY);  // Second vertical line
        painter.drawLine(tableStartX + colWidth1 + colWidth2, tableStartY, tableStartX + colWidth1 + colWidth2, currentY);  // Third vertical line
        painter.drawLine(tableStartX + colWidth1 + colWidth2 + colWidth3, tableStartY, tableStartX + colWidth1 + colWidth2 + colWidth3, currentY);  // Fourth vertical line
        painter.drawLine(tableStartX + colWidth1 + colWidth2 + colWidth3 + colWidth4, tableStartY, tableStartX + colWidth1 + colWidth2 + colWidth3 + colWidth4, currentY);  // Last vertical line

        // Add table header
        painter.drawText(tableStartX + 100, tableStartY + rowHeight / 2 + 50, "S.No");
        painter.drawText(tableStartX + colWidth1 + 100, tableStartY + rowHeight / 2 + 50, "Parameters");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + 100, tableStartY + rowHeight / 2 + 50, "Values");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + colWidth3 + 100, tableStartY + rowHeight / 2 + 50, "Units");

        // Add table content
        int contentStartY = tableStartY + rowHeight;
        painter.drawText(tableStartX + 100, contentStartY + rowHeight / 2 + 50, "1");
        painter.drawText(tableStartX + colWidth1 + 100, contentStartY + rowHeight / 2 + 50, "Delay Value");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + 100, contentStartY + rowHeight / 2 + 50, QString::number(spinBoxDelayVal));
        painter.drawText(tableStartX + colWidth1 + colWidth2 + colWidth3 + 100, contentStartY + rowHeight / 2 + 50, "µs");

        painter.drawText(tableStartX + 100, contentStartY + rowHeight + rowHeight / 2 + 50, "2");
        painter.drawText(tableStartX + colWidth1 + 100, contentStartY + rowHeight + rowHeight / 2 + 50, "G Value");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + 100, contentStartY + rowHeight + rowHeight / 2 + 50, QString::number(spinBoxGValue));
        painter.drawText(tableStartX + colWidth1 + colWidth2 + colWidth3 + 100, contentStartY + rowHeight + rowHeight / 2 + 50, "g");

        painter.drawText(tableStartX + 100, contentStartY + 2*rowHeight + rowHeight / 2 + 50, "3");
        painter.drawText(tableStartX + colWidth1 + 100, contentStartY + 2*rowHeight + rowHeight / 2 + 50, "Vmin Value");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + 100, contentStartY + 2*rowHeight + rowHeight / 2 + 50, QString::number(doubleSpinBoxVminVal));
        painter.drawText(tableStartX + colWidth1 + colWidth2 + colWidth3 + 100, contentStartY + 2*rowHeight + rowHeight / 2 + 50, "Volts");

        painter.drawText(tableStartX + 100, contentStartY + 3*rowHeight + rowHeight / 2 + 50, "4");
        painter.drawText(tableStartX + colWidth1 + 100, contentStartY + 3*rowHeight + rowHeight / 2 + 50, "Vmax Value");
        painter.drawText(tableStartX + colWidth1 + colWidth2 + 100, contentStartY + 3*rowHeight + rowHeight / 2 + 50, QString::number(doubleSpinBoxVmaxVal));
        painter.drawText(tableStartX + colWidth1 + colWidth2 + colWidth3 + 100, contentStartY + 3*rowHeight + rowHeight / 2 + 50, "Volts");

        // Finish the painter to finalize the PDF
        painter.end();

        //change to initial values
        initialZeros();
        ui->lineEdit_serialNo->clear();
    }
}

void MainWindow::checkDeltaSetValRange() {

    ui->spinBox_Detla_SetVal->setToolTip("Valid range: 60-150");
    QToolTip::showText(ui->spinBox_Detla_SetVal->mapToGlobal(QPoint()), ui->spinBox_Detla_SetVal->toolTip());

}

void MainWindow::checkGSetLimitRange() {

    ui->spinBox_gSetLimit->setToolTip("Valid range: 100-200");
    QToolTip::showText(ui->spinBox_gSetLimit->mapToGlobal(QPoint()), ui->spinBox_gSetLimit->toolTip());
}

void MainWindow::checkVminValRange() {

    ui->doubleSpinBox_VminVal->setToolTip("Valid range: 11.5-12.5");
    QToolTip::showText(ui->doubleSpinBox_VminVal->mapToGlobal(QPoint()), ui->doubleSpinBox_VminVal->toolTip());

}

void MainWindow::checkVmaxValRange() {

    ui->doubleSpinBox_vMaxVal->setToolTip("Valid range: 11.5-12.5");
    QToolTip::showText(ui->doubleSpinBox_vMaxVal->mapToGlobal(QPoint()), ui->doubleSpinBox_vMaxVal->toolTip());

}

void MainWindow::showTimeoutMessage()
{
    QMessageBox::warning(this, "Timeout", "Time out!!! Hardware not responding");
}
