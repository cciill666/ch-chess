#include "chessboard.h"

ChessBoard::ChessBoard(QObject *parent)
    : QObject(parent)
{
    m_width = GRID_SIZE * COL_COUNT;
    m_height = GRID_SIZE * ROW_COUNT;
    m_imgUrl = QString("qrc:/images/WOOD/WOOD.JPG");
    m_z = 0;
}

int ChessBoard::boardWidth() const
{
    return m_width;
}

int ChessBoard::boardHeight() const
{
    return m_height;
}

QString ChessBoard::boardImg() const
{
    return m_imgUrl;
}

int ChessBoard::zOrder() const
{
    return m_z;
}
