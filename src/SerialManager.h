/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QtSerialPort>
#include <QTextCursor>
#include <QQuickTextDocument>

class SerialManager : public QObject
{
   Q_OBJECT

   Q_PROPERTY(QString portName READ portName NOTIFY portChanged)
   Q_PROPERTY(bool readOnly READ readOnly NOTIFY connectedChanged)
   Q_PROPERTY(bool readWrite READ readWrite NOTIFY connectedChanged)
   Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
   Q_PROPERTY(QString receivedBytes READ receivedBytes NOTIFY receivedBytesChanged)
   Q_PROPERTY(int maxBufferSize READ maxBufferSize WRITE setMaxBufferSize NOTIFY maxBufferSizeChanged)
   Q_PROPERTY(QString startSequence READ startSequence WRITE setStartSequence NOTIFY startSequenceChanged)
   Q_PROPERTY(QString finishSequence READ finishSequence WRITE setFinishSequence NOTIFY finishSequenceChanged)
   Q_PROPERTY(quint8 portIndex READ portIndex WRITE setPort NOTIFY portChanged)
   Q_PROPERTY(quint8 parityIndex READ parityIndex WRITE setParity NOTIFY parityChanged)
   Q_PROPERTY(quint8 baudRateIndex READ baudRateIndex WRITE setBaudRate NOTIFY baudRateChanged)
   Q_PROPERTY(quint8 dataBitsIndex READ dataBitsIndex WRITE setDataBits NOTIFY dataBitsChanged)
   Q_PROPERTY(quint8 stopBitsIndex READ stopBitsIndex WRITE setStopBits NOTIFY stopBitsChanged)
   Q_PROPERTY(quint8 flowControlIndex READ flowControlIndex WRITE setFlowControl NOTIFY flowControlChanged)
   Q_PROPERTY(QStringList portList READ portList NOTIFY availablePortsChanged)
   Q_PROPERTY(QStringList parityList READ parityList CONSTANT)
   Q_PROPERTY(QStringList baudRateList READ baudRateList CONSTANT)
   Q_PROPERTY(QStringList dataBitsList READ dataBitsList CONSTANT)
   Q_PROPERTY(QStringList stopBitsList READ stopBitsList CONSTANT)
   Q_PROPERTY(QStringList flowControlList READ flowControlList CONSTANT)

signals:
   void portChanged();
   void parityChanged();
   void baudRateChanged();
   void dataBitsChanged();
   void stopBitsChanged();
   void connectedChanged();
   void flowControlChanged();
   void textDocumentChanged();
   void maxBufferSizeChanged();
   void startSequenceChanged();
   void receivedBytesChanged();
   void finishSequenceChanged();
   void availablePortsChanged();
   void connectionError(const QString &name);
   void packetReceived(const QByteArray &packet);
   void rx(const QByteArray &data, const quint64 bytes);
   void tx(const QByteArray &data, const quint64 bytes);

public:
   static SerialManager *getInstance();

   QSerialPort *port() const;

   bool readOnly() const;
   bool readWrite() const;
   bool connected() const;
   QString portName() const;
   int maxBufferSize() const;
   QString receivedBytes() const;
   QString startSequence() const;
   QString finishSequence() const;

   quint8 portIndex() const;
   quint8 parityIndex() const;
   quint8 baudRateIndex() const;
   quint8 dataBitsIndex() const;
   quint8 stopBitsIndex() const;
   quint8 flowControlIndex() const;

   QStringList portList() const;
   QStringList parityList() const;
   QStringList baudRateList() const;
   QStringList dataBitsList() const;
   QStringList stopBitsList() const;
   QStringList flowControlList() const;

   QSerialPort::Parity parity() const;
   QSerialPort::BaudRate baudRate() const;
   QSerialPort::DataBits dataBits() const;
   QSerialPort::StopBits stopBits() const;
   QSerialPort::FlowControl flowControl() const;

   QQuickTextDocument *textDocument() const;

public slots:
   void clearTempBuffer();
   void sendData(const QString &data);
   void setPort(const quint8 portIndex);
   void setParity(const quint8 parityIndex);
   void setBaudRate(const quint8 baudRateIndex);
   void setDataBits(const quint8 dataBitsIndex);
   void setStopBits(const quint8 stopBitsIndex);
   void setMaxBufferSize(const int maxBufferSize);
   void setStartSequence(const QString &sequence);
   void setFinishSequence(const QString &sequence);
   void setFlowControl(const quint8 flowControlIndex);
   void setTextDocument(QQuickTextDocument *textDocument);

private slots:
   void onDataReceived();
   void disconnectDevice();
   void reduceDocumentSize();
   void refreshSerialDevices();

private:
   SerialManager();
   ~SerialManager();

private:
   QSerialPort *m_port;
   QSerialPort::Parity m_parity;
   QSerialPort::BaudRate m_baudRate;
   QSerialPort::DataBits m_dataBits;
   QSerialPort::StopBits m_stopBits;
   QSerialPort::FlowControl m_flowControl;

   QTextCursor *m_textCursor;
   QQuickTextDocument *m_textDocument;

   quint8 m_portIndex;
   quint8 m_parityIndex;
   quint8 m_baudRateIndex;
   quint8 m_dataBitsIndex;
   quint8 m_stopBitsIndex;
   quint64 m_receivedBytes;
   quint8 m_flowControlIndex;

   int m_maxBufferSize;

   QString m_startSeq;
   QString m_finishSeq;
   QByteArray m_tempBuffer;
   QStringList m_portList;
};

#endif
