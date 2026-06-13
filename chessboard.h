#pragma once

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QUrl>
// Qt6 QML 模块声明（整个项目统一模块）
// QML_MODULE(ChessModule, 1.0)

const int GRID_SIZE = 50;
const int COL_COUNT = 9;
const int ROW_COUNT = 10;
const int BOARD_OFFSET_X = 25;
const int BOARD_OFFSET_Y = 25;

class ChessBoard : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(QUrl source READ source CONSTANT)
    Q_PROPERTY(int z READ zValue CONSTANT)

    QML_ELEMENT
public:
    explicit ChessBoard(QObject *parent = nullptr);
    int width() const;
    int height() const;
    QUrl source() const;
    int zValue() const;
private:
    int m_width = GRID_SIZE * COL_COUNT;
    int m_height = GRID_SIZE * ROW_COUNT;
    QUrl m_imgSrc = QUrl("qrc:/img/chess_board.png");
    int m_z = 0;
};
