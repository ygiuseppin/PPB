#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <iostream>
#include <QDialog>
#include <QCloseEvent>
#include <QHostAddress>
#include <QThread>

#include "publisher.h"
#include "subscriber.h"

extern Subscriber* subscriber;
extern Subscriber* map_subscriber;
extern Publisher* publisher;
extern QString username;

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    bool connected_;
    bool close_;
    bool msg_received_;
    bool has_to_publish_;

private slots:

    void publish();
    void on_connect_clicked();
    void closeEvent(QCloseEvent *event);
    void messageReceived(QString msg);
private:
    Ui::LoginDialog *ui;
    QTimer* publish_timer_;
    bool waiting_for_response_;

};

#endif // LOGINDIALOG_H
