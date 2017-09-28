#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>

#include "game.h"
#include "publisher.h"
#include "subscriber.h"

extern Subscriber* subscriber;
extern Publisher* publisher;
extern Publisher* map_publisher;

struct GameConfig{

    QString player1;
    QString player2;
    QString player3;
    QString player4;
    int lives;
    int velocity;
    int map_type;
    int map_size;

};

extern GameConfig game_config;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showScenario();

public slots:
    void loop();
    void debug(QString msg);
    void messageReceived(QString msg);
private:
    Ui::MainWindow *ui;
    Game* game_;
    QTimer* loop_timer_;

    void handleJoinMessage(QString msg);


    void handleChatMessage(QString msg);


    void handleKeyMessage(QString msg);


protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    int active_player_;

};

#endif // MAINWINDOW_H
