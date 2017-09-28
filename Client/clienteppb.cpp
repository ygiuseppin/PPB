#include "clienteppb.h"
#include "ui_clienteppb.h"

ClientePPB::ClientePPB(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientePPB)
{
    ui->setupUi(this);
    connect(subscriber,SIGNAL(messageReceived(QString)),this,SLOT(messageReceived(QString)));
    connect(map_subscriber,SIGNAL(messageReceived(QString)),this,SLOT(mapMessageReceived(QString)));

connect(ui->input,SIGNAL(returnPressed()),this,SLOT(on_send_clicked()));
ui->send->hide();
}

ClientePPB::~ClientePPB()
{
    delete ui;
}

void ClientePPB::mapMessageReceived(QString msg){
     if(msg != ""){
         if(msg.split(" ").size()>0){
             QString tag = msg.split(" ")[0];
             if(tag == "frame")
                 handleNewFrame(msg);
         }
     }
}

void ClientePPB::messageReceived(QString msg){

    if(msg != ""){

        if(msg.split(" ").size()>0){

            QString tag = msg.split(" ")[0];

            if(tag == "frame")
                handleNewFrame(msg);


            if(tag == "finish")
                handleGameOver(msg);


            if(tag == "chat")
                handleNewChatMessage(msg);

        }
    }
}


Scenario messageToScenario(QString msg){

    Scenario res;

    QStringList scenario_list = msg.split(" ")[1].split(",");
    res.size = scenario_list[0].toInt();

    for(uint i=1;i<scenario_list.size();i++){

        int num = scenario_list[i].toInt();

        // es un brick
        if(num == 1){
            Brick b;
            b.pos = i-1;
            res.bricks.push_back(b);
        }

        // es un block
        if(num == 2){
            Block b;
            b.pos = i-1;
            res.blocks.push_back(b);
        }

        // es una bomba
        if(num>99 && num <200){
            Bomb b;
            b.pos = i-1;
            b.frame = num-100;
            res.bombs.push_back(b);
        }

        // es una explosion
        if(num>199 && num <300){
            Explosion e;
            e.pos = i-1;
            e.type = (num-200)/10;
            e.frame = num%10;
            res.explosions.push_back(e);
        }

        // es un item
        if(num>999 && num <2000){
            Item t;
            t.pos = i-1;
            t.frame = num%100;
            t.type = (num-1000)/100;
            res.items.push_back(t);
        }

    }

    QStringList players_list = msg.split(" ")[2].split(",");
    int players_count =players_list[0].toInt();
    for(uint i=0;i<players_count;i++){

        Player p;
        p.id = players_list[1+i*10].toInt();
        p.name = players_list[2+i*10];
        p.posx = players_list[3+i*10].toInt();
        p.posy = players_list[4+i*10].toInt();
        p.frame = players_list[5+i*10].toInt();
        p.side = players_list[6+i*10].toInt();
        p.inmune = players_list[7+i*10].toInt() == 1;
        p.bombs = players_list[8+i*10].toInt();
        p.lives = players_list[9+i*10].toInt();
        p.velocity = players_list[10+i*10].toInt();
        res.players.push_back(p);

    }
    return res;
}


QPoint getTilePos(int index,int scenario_size){
    int x = index%scenario_size;
    int y = index/scenario_size;
    QPoint res(x*TILE_AREA,y*TILE_AREA);
    return res;
}

std::vector<Player> sortByY(std::vector<Player> ps){

    std::vector<Player> in = ps;
    std::vector<Player> res;

    while(res.size()!= ps.size()){

        if(in.size()>0){

            int idx = 0;
            int min_y = in[0].posy;

            for(uint i=1;i<in.size();i++){
                if(min_y >= in[i].posy){
                    idx = i;
                    min_y = in[i].posy;
                }
            }

            // agregar a res ..
            res.push_back(in[idx]);

            // eliminar de in ..
            in.erase(in.begin()+idx);

        }
    }

    return res;
}

QImage getBaseImage(int scenario_size){

    QImage res;
    int width_height = scenario_size*TILE_AREA;
    res = QImage(width_height,width_height,QImage::Format_RGB32);
    res.fill(Qt::white);
    QPainter painter(&res);

    // dibujar tiles
    QImage tile_img;
    for(uint i=0;i<scenario_size*scenario_size;i++){

        int x_index = i%scenario_size;
        int y_index = i/scenario_size;

        if(x_index%2==0 && y_index%2==0)
            tile_img = QImage(":/images/field/3/1.png");
        if(x_index%2!=0 && y_index%2==0)
            tile_img = QImage(":/images/field/3/2.png");
        if(x_index%2==0 && y_index%2!=0)
            tile_img = QImage(":/images/field/3/3.png");
        if(x_index%2!=0 && y_index%2!=0)
            tile_img = QImage(":/images/field/3/4.png");

        tile_img = tile_img.scaled(QSize(TILE_AREA,TILE_AREA));
        QPoint pos = getTilePos(i,scenario_size);
        painter.drawImage(pos, tile_img);

    }

    painter.end();
    return res;

}

void ClientePPB::drawScenario(Scenario scenario){

    QImage img = getBaseImage(scenario.size);
    QPainter painter(&img);

    // dibujar blocks
    QImage block_img(":/images/block/border.png");
    block_img = block_img.scaled(QSize(TILE_AREA,TILE_AREA));
    for(uint i=0;i<scenario.blocks.size();i++){
        Block b = scenario.blocks[i];
        QPoint pos = getTilePos(b.pos,scenario.size);
        painter.drawImage(pos, block_img);
    }

    // dibujar bricks
    QImage brick_img(":/images/brick/brick.png");
    brick_img = brick_img.scaled(QSize(TILE_AREA,TILE_AREA));
    for(uint i=0;i<scenario.bricks.size();i++){
        Brick b = scenario.bricks[i];
        QPoint pos = getTilePos(b.pos,scenario.size);
        painter.drawImage(pos, brick_img);
    }

    // dibujar bombas
    for(uint i=0;i<scenario.bombs.size();i++){
        Bomb b = scenario.bombs[i];
        QString image_file = ":/images/bomb/bomba00";
        if(b.frame+1<10)
            image_file = image_file+"0";
        image_file = image_file + QString::number(b.frame+1) + ".png";
        QImage bomb_img(image_file);
        bomb_img = bomb_img.scaled(BOMB_AREA_W,BOMB_AREA_H);
        QPoint pos = getTilePos(b.pos,scenario.size);
        painter.drawImage(pos, bomb_img);
    }

    // ordenar jugadores por Y de menor a mayor
    std::vector<Player> players_sorted = sortByY(scenario.players);

    // dibujar jugadores
    for(uint i=0;i<players_sorted.size();i++){
        Player p = players_sorted[i];

        if(p.lives>0){
            QImage player_img;
            if(p.inmune){
                if(p.side == BACK_SIDE)
                    player_img = QImage(":/images/players/inmune/atras/"+QString::number(p.frame+1)+".png");
                if(p.side == FRONT_SIDE)
                    player_img = QImage(":/images/players/inmune/frente/"+QString::number(p.frame+1)+".png");
                if(p.side == RIGHT_SIDE)
                    player_img = QImage(":/images/players/inmune/der/"+QString::number(p.frame+1)+".png");
                if(p.side == LEFT_SIDE)
                    player_img = QImage(":/images/players/inmune/izq/"+QString::number(p.frame+1)+".png");
            }else{
                if(p.side == BACK_SIDE)
                    player_img = QImage(":/images/pepes/"+QString::number(p.id+1)+"/back/"+QString::number(p.frame+1)+".png");
                if(p.side == FRONT_SIDE)
                    player_img = QImage(":/images/pepes/"+QString::number(p.id+1)+"/front/"+QString::number(p.frame+1)+".png");
                if(p.side == RIGHT_SIDE)
                    player_img = QImage(":/images/pepes/"+QString::number(p.id+1)+"/right/"+QString::number(p.frame+1)+".png");
                if(p.side == LEFT_SIDE)
                    player_img = QImage(":/images/pepes/"+QString::number(p.id+1)+"/left/"+QString::number(p.frame+1)+".png");
            }
            player_img = player_img.scaled(PLAYER_AREA_W,PLAYER_AREA_H);
            QPoint pos(p.posx,p.posy);
            painter.drawImage(pos, player_img);

            /*
            // si es el jugador activo, dibujar triangulito
            if(p.name == username){

                QImage triangle_img(":/images/players/triangle.png");
                triangle_img = triangle_img.scaled(PLAYER_AREA_W,PLAYER_AREA_W);
                QPoint pos(p.posx,p.posy-PLAYER_AREA_W);
                painter.drawImage(pos,triangle_img);

            }*/
        }
    }

    // dibujar items
    for(uint i=0;i<scenario.items.size();i++){
        Item y = scenario.items[i];
        QPoint pos = getTilePos(y.pos,scenario.size);
        QImage item_img;
        if(y.type == ITEM_BOMB)
            item_img = QImage(":/images/items/bomb/"+QString::number(y.frame+1)+".png");
        if(y.type == ITEM_DELAY)
            item_img = QImage(":/images/items/cooldown/"+QString::number(y.frame+1)+".png");
        if(y.type == ITEM_RANGE)
            item_img = QImage(":/images/items/rango/"+QString::number(y.frame+1)+".png");
        if(y.type == ITEM_VELOCITY)
            item_img = QImage(":/images/items/velocidad/"+QString::number(y.frame+1)+".png");
        painter.drawImage(pos, item_img);

    }

    // dibujar techos de los bricks
    QImage brick_ceil(":/images/brick/techo.png");
    brick_ceil = brick_ceil.scaled(QSize(TILE_AREA,TILE_AREA));
    for(uint i=0;i<scenario.bricks.size();i++){
        Brick b = scenario.bricks[i];
        QPoint pos = getTilePos(b.pos,scenario.size);
        pos.setY(pos.y()-TILE_AREA+1);
        if(pos.y()>=0)
            painter.drawImage(pos, brick_ceil);
    }

    // dibujar techos de los blocks
    QImage block_ceil(":/images/block/techo_border.png");
    block_ceil = block_ceil.scaled(QSize(TILE_AREA,TILE_AREA));
    for(uint i=0;i<scenario.blocks.size();i++){
        Block b = scenario.blocks[i];
        QPoint pos = getTilePos(b.pos,scenario.size);

        pos.setY(pos.y()-TILE_AREA+1);
        if(pos.y()>=0)
            painter.drawImage(pos, block_ceil);
    }

    // dibujar explosiones
    for(uint i=0;i<scenario.explosions.size();i++){
        Explosion e = scenario.explosions[i];
        QPoint pos = getTilePos(e.pos,scenario.size);

        QImage exp_img;
        if(e.type == EXP_CENTRAL)
            exp_img = QImage(":/images/explosion/centro/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_HORIZONTAL)
            exp_img = QImage(":/images/explosion/hor_este/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_VERTICAL)
            exp_img = QImage(":/images/explosion/ver_norte/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_END_UP)
            exp_img = QImage(":/images/explosion/fin_norte/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_END_RIGHT)
            exp_img = QImage(":/images/explosion/fin_este/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_END_DOWN)
            exp_img = QImage(":/images/explosion/fin_sur/"+QString::number(e.frame+1)+".png");
        if(e.type == EXP_END_LEFT)
            exp_img = QImage(":/images/explosion/fin_oeste/"+QString::number(e.frame+1)+".png");

        exp_img = exp_img.scaled(TILE_AREA,TILE_AREA);
        painter.drawImage(pos, exp_img);
    }

    painter.end();

    // TODO mostrar estado de los jugadores
    for(uint i=0;i<scenario.players.size();i++)
        drawPlayerStatus(img,scenario.players[i],scenario.size);

    // mostrar imagen en la interfaz
    ui->image->setPixmap(QPixmap::fromImage(img));

}

void ClientePPB::drawPlayerStatus(QImage& img, Player p, int scenario_size){

    QPainter painter(&img);

    // cargar imagenes
    QImage lives_img(":/images/status/live.png");
    lives_img = lives_img.scaled(TILE_AREA/2,TILE_AREA/2);
    QImage bombs_img(":/images/status/bomb.png");
    bombs_img = bombs_img.scaled(TILE_AREA/2,TILE_AREA/2);
    QImage roller_img(":/images/status/roller.png");
    roller_img = roller_img.scaled(TILE_AREA/2,TILE_AREA/2);

    // obtener posiciones
    QPoint bar_pos;
    if(p.id==0)bar_pos = QPoint(0,0);
    if(p.id==1)bar_pos = QPoint(scenario_size*TILE_AREA-TILE_AREA*4-61,0);
    if(p.id==2)bar_pos = QPoint(0,scenario_size*TILE_AREA-TILE_AREA-1);
    if(p.id==3)bar_pos = QPoint(scenario_size*TILE_AREA-TILE_AREA*4-61,scenario_size*TILE_AREA-TILE_AREA-1);

    // posiciones de las imagenes
    QPoint livespos(bar_pos.x()+60+TILE_AREA/2,bar_pos.y());
    QPoint bombpos(bar_pos.x()+60+TILE_AREA/2+TILE_AREA,bar_pos.y());
    QPoint rollerspos(bar_pos.x()+60+TILE_AREA/2+TILE_AREA*2,bar_pos.y());

    // dibujar iconos
    painter.drawImage(livespos, lives_img);
    painter.drawImage(bombpos, bombs_img);
    painter.drawImage(rollerspos, roller_img);

    QPen penHLines(QColor("#FFFFFF")/*, 10, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin*/);
    painter.setPen(penHLines);

    // escribir nombre
    QString player_name = p.name;
    painter.drawText(QRect(bar_pos.x()+5,bar_pos.y()+5,61,TILE_AREA/2),player_name);

    // escribir numeros
    QString lives_str = QString::number( p.lives);
    painter.drawText(QRect(bar_pos.x()+TILE_AREA+5+61,bar_pos.y()+0+5,TILE_AREA/2,TILE_AREA/2),lives_str);
    QString bombs_str = QString::number( p.bombs );
    painter.drawText(QRect(bar_pos.x()+TILE_AREA*2+5+61,bar_pos.y()+0+5,TILE_AREA/2,TILE_AREA/2),bombs_str);
    QString roller_str = QString::number( p.velocity );
    painter.drawText(QRect(bar_pos.x()+TILE_AREA*3+5+61,bar_pos.y()+0+5,TILE_AREA/2,TILE_AREA/2),roller_str);
    painter.end();

}

void ClientePPB::handleNewFrame(QString msg){

    if(msg.split(" ").size()>0){

        Scenario scenario = messageToScenario(msg);

        ui->image->setMinimumWidth(20+scenario.size*TILE_AREA);
        ui->image->setMinimumHeight(20+scenario.size*TILE_AREA);

        drawScenario(scenario);

    }

}

void ClientePPB::handleGameOver(QString msg){

    // TODO dibujar pantalla de ganador

}


void ClientePPB::handleNewChatMessage(QString msg){

    // mensaje:  chat <player_name> <message>
    QStringList qss = msg.split(" ");

    if(qss.size() > 2){
        QString player_name = qss[1];
        QString message;

        for(uint i=2;i<qss.size();i++){
            message = message+" "+qss[i];
        }

        ui->chat->append("- "+player_name + ":"+message);

    }

}

void ClientePPB::on_send_clicked()
{
    if(ui->input->text()!=""){
        publisher->publishOne("chat "+username + " "+ ui->input->text());
        ui->input->clear();

        if(ui->input->text().split(" ")[0] == "@leave"){
            close();
        }
    }
}

void ClientePPB::keyPressEvent(QKeyEvent *event){

    if(!event->isAutoRepeat()){
        if(event->key() == Qt::Key_Up)
            publisher->publishOne("key press "+QString::number(KEY_UP)+" "+username);
        if(event->key() == Qt::Key_Down)
            publisher->publishOne("key press "+QString::number(KEY_DOWN)+" "+username);
        if(event->key() == Qt::Key_Left)
            publisher->publishOne("key press "+QString::number(KEY_LEFT)+" "+username);
        if(event->key() == Qt::Key_Right)
            publisher->publishOne("key press "+QString::number(KEY_RIGHT)+" "+username);
        if(event->key() == Qt::Key_M)
            publisher->publishOne("key press "+QString::number(KEY_BOMB)+" "+username);
    }
}

void ClientePPB::keyReleaseEvent(QKeyEvent *event){

    if(!event->isAutoRepeat()){
        if(event->key() == Qt::Key_Up)
            publisher->publishOne("key release "+QString::number(KEY_UP)+" "+username);
        if(event->key() == Qt::Key_Down)
            publisher->publishOne("key release "+QString::number(KEY_DOWN)+" "+username);
        if(event->key() == Qt::Key_Left)
            publisher->publishOne("key release "+QString::number(KEY_LEFT)+" "+username);
        if(event->key() == Qt::Key_Right)
            publisher->publishOne("key release "+QString::number(KEY_RIGHT)+" "+username);
    }
}

