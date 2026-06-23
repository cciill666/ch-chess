#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
// 全局常量
const int GRID_SIZE      = 50;
const int COL_COUNT      = 9;
const int ROW_COUNT      = 10;
const int BOARD_OFFSET_X = 25;
const int BOARD_OFFSET_Y = 25;

class ChessBoard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int boardWidth  READ boardWidth  CONSTANT)
    Q_PROPERTY(int boardHeight READ boardHeight CONSTANT)
    Q_PROPERTY(QString boardImg   READ boardImg    CONSTANT)
    Q_PROPERTY(int zOrder      READ zOrder      CONSTANT)
public:
    explicit ChessBoard(QObject *parent = nullptr);
    int boardWidth() const;
    int boardHeight() const;
    QString boardImg() const;
    int zOrder() const;
private:
    int     m_width;
    int     m_height;
    QString    m_imgUrl;
    int     m_z;
};
