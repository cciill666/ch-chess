#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QDebug>
#include "chessboard.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // ChessBoard board;
    // qDebug() << "图片路径：" << board.boardImg();
    // qDebug() << "文件是否存在：" << QFile::exists(board.boardImg());

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
        );

    engine.loadFromModule("chinesechess", "Main");
    return app.exec();
}
