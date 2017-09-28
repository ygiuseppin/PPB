#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QWidget>
#include <QHostAddress>
#include <QTextStream>

#include <qmqtt/qmqtt.h>


/*

    Modo de uso

    1) instalar QMQTT

        a) descargar repo

        https://github.com/emqtt/qmqtt

        b) compilar
        Dentro del directorio raiz hacer:

        mkdir build
        cd build
        qmake ..
        make install

    2) agregar al proyecto

        En el .pro incluir las lineas:
        INCLUDEPATH += <QMQTT>/qmqtt/build/include
        INCLUDEPATH += <QMQTT>/qmqtt/build/include/qmqtt
        LIBS += <QMQTT>/qmqtt/build/lib/libqmqtt.so

    3) recibir un mensaje

        a) crear subscriber y conectar

        quint16 port = 1883;
        Subscriber* sub = new Subscriber(QHostAddress("127.0.0.1"), port,"test_topic",false);
        sub->connectToHost();

        b) recibir

        se debe conectar la señal messageReceived de Subscriber a un slot propio

        connect(subscriber_,
                SIGNAL(messageReceived(QString)),
                this,SLOT(mySlot(QString)));

        void mySlot(QString msg){
            cout << "Mensaje recibido: " << msg.toStdString() << endl;
        }

*/

class Subscriber : public QMQTT::Client
{
    Q_OBJECT
public:
    Subscriber(const QHostAddress& host = QHostAddress::LocalHost,
                        const quint16 port = 1883,
                        QString topic = "test",
                        QObject* parent = NULL,
                        bool encrypt = false)
        : QMQTT::Client(host, port, parent)
        , _qout(stdout){
        topic_ = topic;
        encrypt_ = encrypt;
        connect(this, &Subscriber::connected, this, &Subscriber::onConnected);
        connect(this, &Subscriber::subscribed, this, &Subscriber::onSubscribed);
        connect(this, &Subscriber::received, this, &Subscriber::onReceived);
    }
    virtual ~Subscriber() {}

    bool encrypt_;
    QTextStream _qout;
    QString topic_;

    QString encrypt(QString msg){

        // TODO

        return msg;
    }

public slots:
    void onConnected(){
        subscribe(topic_, 0);
    }

    void onSubscribed(const QString& topic){
        emit subscribedTo(topic);
    }

    void onReceived(const QMQTT::Message& message){
        QString msg = QString::fromUtf8(message.payload());

        if(encrypt_)
            msg = encrypt(msg);

        emit messageReceived(msg);
    }

signals:
    void messageReceived(QString msg);
    void subscribedTo(QString topic);
};

#endif // SUBSCRIBER_H
