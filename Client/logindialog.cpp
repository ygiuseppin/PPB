#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connected_ = false;
    close_ = false;
    msg_received_ = false;
    has_to_publish_ = false;

    publish_timer_ = new QTimer(this);
    connect(publish_timer_,SIGNAL(timeout()),this,SLOT(publish()));
    publish_timer_->start(1000);
    waiting_for_response_ = false;
}

LoginDialog::~LoginDialog()
{
    delete ui;
}



void LoginDialog::on_connect_clicked()
{
    msg_received_ = false;
    connected_ = false;

    ui->status->setText("");

    if(ui->name->text()!="" &&
       ui->name->text()!="SERVIDOR" &&
       ui->name->text()!="SERVER" &&
       ui->name->text().size() < 8 &&
       !ui->name->text().contains(' ')){

        // conectar con servidor
        quint16 port = 1883;
        subscriber = new Subscriber(QHostAddress::LocalHost,port,"ppb");
        map_subscriber = new Subscriber(QHostAddress::LocalHost,port,"ppb/map");

        publisher = new Publisher(QHostAddress::LocalHost,port,"ppb");
        connect(subscriber,SIGNAL(messageReceived(QString)),this,SLOT(messageReceived(QString)));

        map_subscriber->connectToHost();
        subscriber->connectToHost();
        publisher->connectToHost();

        // esperar a que conecte
        QThread::msleep(500);
        has_to_publish_ = true;
        publish_timer_->start(1000);
    }else{
        ui->status->setText("Nombre inválido");
    }
}

void LoginDialog::closeEvent(QCloseEvent *event) {
    close_ = true;
    event->accept();
}

void LoginDialog::publish(){
    if(has_to_publish_){


        publisher->publishOne("join " + ui->name->text());
        has_to_publish_ = false;
    }
}


void LoginDialog::messageReceived(QString msg){

    if(msg.split(" ").size() == 2 &&
       msg.split(" ")[0] == "joined" &&
       msg.split(" ")[1] == ui->name->text()){

            username = ui->name->text();
            connected_ = true;
            msg_received_ = true;
            close();
    }

}


