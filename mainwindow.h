#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QVector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyReleaseEvent(QKeyEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void openBag() ;
private slots:
    void updatePlayerState(); // 更新玩家状态（定时器调用）

private:
    QSet<int> activeKeys; // 当前按下的按键集合
    QVector<QVector<int>> generateContraStyleMap(int rows, int cols); // 生成地图

    QVector<QVector<int>> tileMap; // 地图数据
    const int tileSize = 100;      // 瓦片大小

    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsRectItem *player;
    QPointF playerVelocity; // 玩家速度
    bool isJumping = false; // 是否正在跳跃
    QTimer *gameTimer;      // 定时器
};

#endif // MAINWINDOW_H
