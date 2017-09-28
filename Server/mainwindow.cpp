#include "mainwindow.h"
#include "ui_mainwindow.h"

int getPlayerCount(){
    int res = 0;
    if(game_config.player1!="")
        res++;
    if(game_config.player2!="")
        res++;
    if(game_config.player3!="")
        res++;
    if(game_config.player4!="")
        res++;
    return res;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(subscriber,SIGNAL(messageReceived(QString)),this,SLOT(messageReceived(QString)));

    int player_count = getPlayerCount();

    game_ = new Game(player_count,game_config.map_size,game_config.map_type,game_config.lives,0/*TODO*/,"","","","");

    connect(game_,SIGNAL(debug(QString)),this,SLOT(debug(QString)));
    setFocusPolicy(Qt::StrongFocus);

    loop_timer_ = new QTimer(this);
    connect(loop_timer_,SIGNAL(timeout()),this,SLOT(loop()));
    loop_timer_->start(100);

    active_player_ = 0;
}


void MainWindow::messageReceived(QString msg){

    if(msg!=""){

        if(msg.split(" ").size()>0 && msg.split(" ")[0] == "join")
            handleJoinMessage(msg);

        if(msg.split(" ").size()>0 && msg.split(" ")[0] == "chat")
            handleChatMessage(msg);

        if(msg.split(" ").size()>0 && msg.split(" ")[0] == "key")
            handleKeyMessage(msg);

    }
}

void MainWindow::handleJoinMessage(QString msg){

    // esta repetido?
    QString name = msg.split(" ")[1];
    if(game_config.player1==name ||
       game_config.player2==name ||
       game_config.player3==name ||
       game_config.player4==name)
        return;

    // buscar un jugador vacio ..
    if(game_config.player1==""){
        game_config.player1 = name;
        publisher->publishOne("joined "+name);
        return;
    }

    if(game_config.player2==""){
        game_config.player2 = name;
        publisher->publishOne("joined "+name);
        return;
    }

    if(game_config.player3==""){
        game_config.player3 = name;
        publisher->publishOne("joined "+name);
        return;
    }

    if(game_config.player4==""){
        game_config.player4 = name;
        publisher->publishOne("joined "+name);
    }
}

void MainWindow::handleChatMessage(QString msg){

    QStringList qss = msg.split(" ");

    QString command = msg.split(" ")[2];

    if(command == "@restart"){

        int player_count = getPlayerCount();
        game_ = new Game(player_count,game_config.map_size,game_config.map_type,game_config.lives,0/*TODO*/,
                         game_config.player1,game_config.player2,game_config.player3,game_config.player4);
        connect(game_,SIGNAL(debug(QString)),this,SLOT(debug(QString)));
        publisher->publishOne("chat SERVER Restarted, "+QString::number(player_count) + " users");

    }

    if(command == "@lives" && qss.size()>3){

        QString lives = qss[3];
        if(lives == "1" || lives == "2" || lives == "3" || lives == "4" || lives == "5"){
            game_config.lives = lives.toInt();
            publisher->publishOne("chat SERVER Lives changed");
        }

    }

    if(command == "@size" && qss.size()>3){

        QString size_s = qss[3];
        if(size_s == "5" ||
                size_s == "6" ||
                size_s == "7" ||
                size_s == "8" ||
                size_s == "9" ||
                size_s == "10" ||
                size_s == "11" ||
                size_s == "12" ||
                size_s == "13" ||
                size_s == "14" ||
                size_s == "15"
                ){
            game_config.map_size = size_s.toInt();
            publisher->publishOne("chat SERVER Size changed");
        }
    }

    if(command == "@kick" && qss.size()>3){

        QString kicked_player = qss[3];
        if(game_config.player1==kicked_player)
            game_config.player1="";
        if(game_config.player2==kicked_player)
            game_config.player2="";
        if(game_config.player3==kicked_player)
            game_config.player3="";
        if(game_config.player4==kicked_player)
            game_config.player4="";

    }

    if(command == "@kickall"){
        game_config.player1="";
        game_config.player2="";
        game_config.player3="";
        game_config.player4="";
    }
}

void MainWindow::handleKeyMessage(QString msg){
    QStringList qss = msg.split(" ");

    if(qss.size()>3){

        QString press_or_release = qss[1];
        if(press_or_release == "press"){
            int key = qss[2].toInt();
            QString name = qss[3];
            game_->keyPressed(name,key);
        }

        if(press_or_release == "release"){

            int key = qss[2].toInt();
            QString name = qss[3];
            game_->keyReleased(name,key);

        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showScenario(){

    QImage img = game_->getScenarioImage();

    // mostrar imagen
    ui->img->setPixmap(QPixmap::fromImage(img));

}

void MainWindow::loop(){
    game_->loop();
    QString msg = game_->getFrameMessage();
    map_publisher->publishOne(msg);
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    /*
    if(!event->isAutoRepeat()){

        // jugador 1
        if(event->key() == Qt::Key_Up)
            game_->keyPressed(active_player_,KEY_UP);
        if(event->key() == Qt::Key_Down)
            game_->keyPressed(active_player_,KEY_DOWN);
        if(event->key() == Qt::Key_Left)
            game_->keyPressed(active_player_,KEY_LEFT);
        if(event->key() == Qt::Key_Right)
            game_->keyPressed(active_player_,KEY_RIGHT);
        if(event->key() == Qt::Key_M)
            game_->keyPressed(active_player_,KEY_BOMB);

        if(event->key() == Qt::Key_Tab){
            if(active_player_ == 0)
                active_player_ =1;
            else
                active_player_=0;
        }
    }
    */
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
/*
    if(!event->isAutoRepeat()){

        // jugador 1
        if(event->key() == Qt::Key_Up)
            game_->keyReleased(active_player_,KEY_UP);
        if(event->key() == Qt::Key_Down)
            game_->keyReleased(active_player_,KEY_DOWN);
        if(event->key() == Qt::Key_Left)
            game_->keyReleased(active_player_,KEY_LEFT);
        if(event->key() == Qt::Key_Right)
            game_->keyReleased(active_player_,KEY_RIGHT);
    }
*/
}


void MainWindow::debug(QString msg){
    ui->debug_list->addItem(msg);
}
