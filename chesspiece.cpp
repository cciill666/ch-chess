#include "chesspiece.h"
#include <QtMath>

ChessPiece::ChessPiece(QObject *parent)
    : ChessPiece(-1, Camp::Red, PieceType::Ju, parent)
{

    m_alive = false;
}

ChessPiece::ChessPiece(int id, Camp camp, PieceType type, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_camp(camp)
    , m_type(type)
    , m_alive(true)
    , m_selected(false)
    , m_x(0)
    , m_y(0)
{
    QString normal, select;
    const QString pathPrefix = "qrc:/images/WOOD/";
    if (camp == Camp::Red) {
        switch (type) {
        case PieceType::Jiang:
            normal = "RK.GIF";
            select = "RKS.GIF";
            break;
        case PieceType::Ju:
            normal = "RR.GIF";
            select = "RRS.GIF";
            break;
        case PieceType::Ma:
            normal = "RN.GIF";
            select = "RNS.GIF";
            break;
        case PieceType::Xiang:
            normal = "RB.GIF";
            select = "RBS.GIF";
            break;
        case PieceType::Shi:
            normal = "RA.GIF";
            select = "RAS.GIF";
            break;
        case PieceType::Pao:
            normal = "RC.GIF";
            select = "RCS.GIF";
            break;
        case PieceType::Bing:
            normal = "RP.GIF";
            select = "RPS.GIF";
            break;
        default:
            break;
        }
    } else {
        switch (type) {
        case PieceType::Shuai:
            normal = "BK.GIF";
            select = "BKS.GIF";
            break;
        case PieceType::Ju:
            normal = "BR.GIF";
            select = "BRS.GIF";
            break;
        case PieceType::Ma:
            normal = "BN.GIF";
            select = "BNS.GIF";
            break;
        case PieceType::Xiang:
            normal = "BB.GIF";
            select = "BBS.GIF";
            break;
        case PieceType::Shi:
            normal = "BA.GIF";
            select = "BAS.GIF";
            break;
        case PieceType::Pao:
            normal = "BC.GIF";
            select = "BCS.GIF";
            break;
        case PieceType::Zu:
            normal = "BP.GIF";
            select = "BPS.GIF";
            break;
        default:
            break;
        }
    }

    m_normalImg = pathPrefix + normal;
    m_selectImg = pathPrefix + select;
}

int ChessPiece::pieceId() const
{
    return m_id;
}

ChessPiece::Camp ChessPiece::camp() const
{
    return m_camp;
}

ChessPiece::PieceType ChessPiece::pieceType() const
{
    return m_type;
}

bool ChessPiece::isAlive() const
{
    return m_alive;
}

bool ChessPiece::isSelected() const
{
    return m_selected;
}

int ChessPiece::posX() const
{
    return m_x;
}

int ChessPiece::posY() const
{
    return m_y;
}

int ChessPiece::pieceW() const
{
    return m_w;
}

int ChessPiece::pieceH() const
{
    return m_h;
}

int ChessPiece::zOrder() const
{
    return m_z;
}

QString ChessPiece::imgSource() const
{
    return m_selected ? m_selectImg : m_normalImg;
}

QPoint ChessPiece::getLogicPos() const
{
    int col = qRound((m_x + m_w / 2.0 - BOARD_OFFSET_X) / (double)GRID_SIZE);
    int row = qRound((m_y + m_h / 2.0 - BOARD_OFFSET_Y) / (double)GRID_SIZE);
    return QPoint(col, row);
}

void ChessPiece::setLogicPos(int col, int row) {
    int px = BOARD_OFFSET_X + col * GRID_SIZE - m_w / 2;
    int py = BOARD_OFFSET_Y + row * GRID_SIZE - m_h / 2;
    setPosX(px);
    setPosY(py);
}

void ChessPiece::setPosX(int x)
{
    if (m_x != x) {
        m_x = x;
        emit posChanged();
    }
}

void ChessPiece::setPosY(int y)
{
    if (m_y != y) {
        m_y = y;
        emit posChanged();
    }
}

void ChessPiece::setAlive(bool alive)
{
    if (m_alive != alive) {
        m_alive = alive;
        emit aliveChanged();
    }
}

void ChessPiece::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        emit stateChanged();
    }
}

