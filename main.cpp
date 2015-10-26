#include <QtSerialPort/QtSerialPort>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

class Main : public QObject{
Q_OBJECT
public:
    QByteArray lbuf;
    QSerialPort sp;
    QTcpServer  sr;
    QTcpSocket  *sk;
    Main(QString spname)
       : sp(spname)
       , sk(0)
    {
        sp.setBaudRate(9600);
        if (!sp.open(QIODevice::ReadWrite)) {
            qFatal("cant open serialport %s", qPrintable(spname));
        }

        if (!sr.listen(QHostAddress::Any, 4632)) {
            qFatal("listen failed");
        }
        connect(&sr, SIGNAL(newConnection()), this, SLOT(newConnection()));
        connect(&sp, SIGNAL(readyRead()), this, SLOT(readySerial()));
    }
private slots:
    void newConnection() {
        QTcpSocket *n = sr.nextPendingConnection();
        if (sk == 0) {
            sk = n;
            connect(sk, SIGNAL(readyRead()), this, SLOT(readyTCP()));
            connect(sk, SIGNAL(disconnected()), this, SLOT(disconnected()));
        } else {
            n->close();
            n->deleteLater();
        }
    }
    void disconnected() {
        sk->deleteLater();
        sk = 0;
    }
    void readyTCP() {
        QByteArray ra = sk->readAll();
        sp.write(ra);
    }
    void readySerial() {
        if (!sk)
            return;
        for(;;) {
            char c;
            if (!sp.getChar(&c))
                return;
            lbuf += c;
            if (c == '\n') {
                sk->write(lbuf);
                lbuf.clear();
            }
        }
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Main m(app.arguments().at(1));
    return app.exec();
}

#include "main.moc"
