#include "mainwindow.h"
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QDialog>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVector>
#include <QRandomGenerator>
#include <cmath>
#include <QTimer>


QVector<QVector<int>> MainWindow::generateContraStyleMap(int rows, int cols) {
    QVector<QVector<int>> tileMap(rows, QVector<int>(cols, 0)); // 初始化地图（0 表示空地）

    QVector<int> groundHeights(cols); // 每列的地形高度
    int baseHeight = rows - 5;        // 地面起始高度

    for (int col = 0; col < cols; ++col) {
        // 80% 概率生成陆地，20% 概率留空作为悬崖
        if (QRandomGenerator::global()->bounded(0, 100) < 20) {
            groundHeights[col] = -1; // 该列为空（悬崖）
            continue;
        }

        // 生成地形高度（带随机偏移）
        double noise = std::sin(col * 0.1) * 3.0; // 正弦波生成平滑地形
        int randomOffset = QRandomGenerator::global()->bounded(-2, 3); // 随机偏移
        groundHeights[col] = qBound(1, baseHeight + static_cast<int>(noise) + randomOffset, rows - 1);
    }

    // 填充地形
    for (int col = 0; col < cols; ++col) {
        int groundHeight = groundHeights[col];
        if (groundHeight == -1) {
            continue; // 空列，跳过填充
        }

        for (int row = groundHeight; row < rows; ++row) {
            tileMap[row][col] = 1; // 填充陆地
        }
    }
    for(int i=0;i<8;i++)
    {
        tileMap[i][cols-1]=2;
    }
    return tileMap;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scene(new QGraphicsScene(this)), view(new QGraphicsView(scene, this)) {
    setCentralWidget(view);
    resize(800, 600);



    // 初始化地图
    const int rows = 8;  // 地图行数
    const int cols = 50;  // 地图列数
    // 设置场景大小
    scene->setSceneRect(0, 0, cols*tileSize, rows*tileSize);
    tileMap = generateContraStyleMap(rows, cols);

    // 绘制地图瓦片
    for (int row = 0; row < tileMap.size(); ++row) {
        for (int col = 0; col < tileMap[row].size(); ++col) {
            int tile = tileMap[row][col];
            QColor color;

            // 根据瓦片类型设置颜色
            if (tile == 0) {
                color = Qt::white; // 空地
            } else if (tile == 1) {
                color = Qt::green; // 陆地
            } else if (tile == 2) {
                color = Qt::red; // 特殊瓦片（红色）
            } else {
                continue; // 未知瓦片类型，跳过绘制
            }

            // 创建瓦片并设置颜色
            QGraphicsRectItem *tileItem =
                new QGraphicsRectItem(col * tileSize, row * tileSize, tileSize, tileSize);
            tileItem->setBrush(color);
            tileItem->setPen(Qt::NoPen); // 移除边框
            scene->addItem(tileItem);
        }
    }


    // 添加玩家
    player = new QGraphicsRectItem(0, 0, 50, 50);
    player->setBrush(Qt::blue);
    player->setPen(Qt::NoPen);
    scene->addItem(player);

    player->setPos(100, 100); // 初始位置
    playerVelocity = QPointF(0, 0); // 初始速度

    // 初始化定时器
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updatePlayerState);
    gameTimer->start(16); // 每帧更新（约60帧/秒）

    // 视图设置
    view->setRenderHint(QPainter::Antialiasing);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->centerOn(player);
    // 确保主窗口接收按键事件
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    // 创建背包按钮
    QPushButton *openBagButton = new QPushButton("打开背包", this);
    openBagButton->setGeometry(10, 10, 100, 30);
    connect(openBagButton, &QPushButton::clicked, this, &MainWindow::openBag);

}
void MainWindow::openBag() {
    // 创建背包对话框
    QDialog *bagDialog = new QDialog(this);
    bagDialog->setWindowTitle("背包");
    bagDialog->resize(300, 400);

    // 设置对话框背景颜色
    bagDialog->setStyleSheet("background-color: #f0f0f0;");

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(bagDialog);

    // 添加标题
    QLabel *titleLabel = new QLabel("背包内容列表", bagDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);

    // 添加滚动区域
    QScrollArea *scrollArea = new QScrollArea(bagDialog);
    scrollArea->setWidgetResizable(true);

    // 创建物品列表
    QListWidget *itemList = new QListWidget(scrollArea);
    itemList->setStyleSheet(
        "QListWidget { background: white; border: 1px solid #ccc; }"
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #eee; }"
        "QListWidget::item:hover { background: #f5f5f5; }"
        );

    // 添加示例物品
    for (int i = 1; i <= 10; ++i) {
        QListWidgetItem *item = new QListWidgetItem(
            QIcon(":/icons/item.png"), // 图标（需替换为有效的资源路径）
            QString("物品 %1").arg(i), itemList);
        itemList->addItem(item);
    }

    scrollArea->setWidget(itemList);
    mainLayout->addWidget(scrollArea);

    // 设置对话框布局
    bagDialog->setLayout(mainLayout);

    // 显示背包对话框
    bagDialog->exec();
}



MainWindow::~MainWindow() {
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    activeKeys.insert(event->key()); // 记录按键
    const qreal JUMP_SPEED = -20; // 跳跃初速度（向上移动，负值）
    const qreal SPEED = 10; // 玩家移动速度
    switch (event->key()) {
    case Qt::Key_Left:
        playerVelocity.rx() = -SPEED;
        break;
    case Qt::Key_Right:
         playerVelocity.rx() = SPEED;
        break;
    case Qt::Key_Space:
        if (!isJumping) { // 仅当未跳跃时允许跳跃
            isJumping = true;
            playerVelocity.ry() = JUMP_SPEED; // 设置向上的速度
        }
    default:
        QMainWindow::keyPressEvent(event);
    }

}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    playerVelocity.rx() = 0;
    activeKeys.remove(event->key()); // 移除按键
}

void MainWindow::updatePlayerState() {
    // 常量定义
    const qreal gravity = 0.5;       // 重力加速度
    const qreal maxFallSpeed = 10;  // 最大下落速度

    // 当前玩家位置
    qreal playerX = player->x();
    qreal playerY = player->y();
    // 计算新的位置
    int newX = playerX + playerVelocity.rx();
    int newY = playerY + playerVelocity.ry();

    // 撞墙检测（左右检测）
    int later_y = (newY + player->rect().height() - 1) / tileSize;
    int later_x_left = newX / tileSize;
    int later_x_right = (newX + player->rect().width() - 1) / tileSize;

    if (later_y >= 0 && later_y < tileMap.size() &&
        later_x_left >= 0 && later_x_right < tileMap[0].size()) {
        int tile_left = tileMap[later_y][later_x_left];
        int tile_right = tileMap[later_y][later_x_right];

        if (tile_left == 1 || tile_right == 1) {
            playerVelocity.rx() = 0;
        }
    }
    // 更新垂直速度（重力作用）
    playerVelocity.ry() = qMin(playerVelocity.y() + gravity, maxFallSpeed);

    // 落地检测（脚下检测）
    int tileRow = (newY + player->rect().height()) / tileSize;
    int tileCol_left = newX / tileSize;
    int tileCol_right = (newX + player->rect().width() - 1) / tileSize;
    if (tileRow >= 0 && tileRow < tileMap.size() &&
        tileCol_left >= 0 && tileCol_right < tileMap[0].size()) {
        int tileType_left = tileMap[tileRow][tileCol_left];
        int tileType_right = tileMap[tileRow][tileCol_right];
        if (tileType_right == 2) {
            QMessageBox::information(this, "胜利", "你赢得了胜利！");
            close(); // 关闭窗口，结束游戏
            return;
        }
        if ((tileType_left != 0 || tileType_right != 0) && playerVelocity.y() > 0) {
            // 停止下落，调整到地面
            playerVelocity.ry() = 0;
            isJumping = false; // 重置跳跃状态
        }
    }

    // 更新玩家位置
    player->setPos(playerX+playerVelocity.rx(), playerY + playerVelocity.ry());

    // 掉出地图检测
    if (playerY > scene->sceneRect().height()) {
        QMessageBox::information(this, "失败","你摔死了！");
        player->setPos(100, 100);    // 重置位置
        playerVelocity = QPointF(0, 0); // 重置速度
    }

    view->centerOn(player);
}

