#include "chessmanager.h"

#include <QDataStream>
#include <QHostAddress>
#include <QDebug>
#include <QtMath>

namespace {

    bool isGeneral(PieceType type)
    {
        return type == PieceType::Jiang || type == PieceType::Shuai;
    }

    QString winnerName(Camp camp)
    {
        return camp == Camp::Red ? QStringLiteral("红方胜利") : QStringLiteral("黑方胜利");
    }

}

ChessManager::ChessManager(QObject *parent)
    : QObject(parent)
    , m_board(new ChessBoard(this))
    , m_selectedPiece(nullptr)
    , m_curTurn(Camp::Red)
    , m_online(false)
    , m_tcpServer(nullptr)
    , m_tcpSocket(new QTcpSocket(this))
{
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, [this]() {
        QDataStream ds(m_tcpSocket);
        int cmd = -1;
        ds >> cmd;
        if (cmd == 0) {
            int id = -1;
            int x = 0;
            int y = 0;
            ds >> id >> x >> y;
            syncRemoteMove(id, x, y);
        }
    });
}

ChessManager::~ChessManager()
{
    qDeleteAll(m_pieces);
    m_pieces.clear();
}

ChessBoard *ChessManager::boardObj() const
{
    return m_board;
}

QList<ChessPiece *> &ChessManager::pieceList()
{
    return m_pieces;
}

Camp ChessManager::currentTurn() const
{
    return m_curTurn;
}

bool ChessManager::isOnline() const
{
    return m_online;
}

ChessPiece *ChessManager::selectedPiece() const
{
    return m_selectedPiece;
}

ChessPiece *ChessManager::getPieceByPos(int col, int row)
{
    for (ChessPiece *piece : m_pieces) {
        if (piece == nullptr || !piece->isAlive()) {
            continue;
        }
        const QPoint pos = piece->getLogicPos();
        if (pos.x() == col && pos.y() == row) {
            return piece;
        }
    }
    return nullptr;
}

bool ChessManager::checkMoveRule(ChessPiece *piece, int dstCol, int dstRow)
{
    if (piece == nullptr || !piece->isAlive()) {
        return false;
    }

    if (dstCol < 0 || dstCol >= COL_COUNT || dstRow < 0 || dstRow >= ROW_COUNT) {
        return false;
    }

    const QPoint srcPos = piece->getLogicPos();
    const int srcCol = srcPos.x();
    const int srcRow = srcPos.y();
    if (srcCol == dstCol && srcRow == dstRow) {
        return false;
    }

    ChessPiece *target = getPieceByPos(dstCol, dstRow);
    if (target != nullptr && target->camp() == piece->camp()) {
        return false;
    }

    const int dx = dstCol - srcCol;
    const int dy = dstRow - srcRow;
    const int adx = qAbs(dx);
    const int ady = qAbs(dy);
    const bool isRed = piece->camp() == Camp::Red;

    switch (piece->pieceType()) {
    case PieceType::Jiang:
    case PieceType::Shuai:
        if (dstCol < 3 || dstCol > 5) {
            return false;
        }
        if (isRed && (dstRow < 7 || dstRow > 9)) {
            return false;
        }
        if (!isRed && (dstRow < 0 || dstRow > 2)) {
            return false;
        }
        return (adx == 1 && dy == 0) || (ady == 1 && dx == 0);

    case PieceType::Shi:
        if (dstCol < 3 || dstCol > 5) {
            return false;
        }
        if (isRed && (dstRow < 7 || dstRow > 9)) {
            return false;
        }
        if (!isRed && (dstRow < 0 || dstRow > 2)) {
            return false;
        }
        return adx == 1 && ady == 1;

    case PieceType::Xiang: {
        if (adx != 2 || ady != 2) {
            return false;
        }
        if (isRed && dstRow < 5) {
            return false;
        }
        if (!isRed && dstRow > 4) {
            return false;
        }
        const int eyeCol = (srcCol + dstCol) / 2;
        const int eyeRow = (srcRow + dstRow) / 2;
        return getPieceByPos(eyeCol, eyeRow) == nullptr;
    }

    case PieceType::Ma: {
        const bool validStep = (adx == 2 && ady == 1) || (adx == 1 && ady == 2);
        if (!validStep) {
            return false;
        }
        const int legCol = srcCol + (adx == 2 ? (dx > 0 ? 1 : -1) : 0);
        const int legRow = srcRow + (ady == 2 ? (dy > 0 ? 1 : -1) : 0);
        return getPieceByPos(legCol, legRow) == nullptr;
    }

    case PieceType::Ju: {
        if (dx != 0 && dy != 0) {
            return false;
        }
        const int stepCol = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
        const int stepRow = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
        int col = srcCol + stepCol;
        int row = srcRow + stepRow;
        while (col != dstCol || row != dstRow) {
            if (getPieceByPos(col, row) != nullptr) {
                return false;
            }
            col += stepCol;
            row += stepRow;
        }
        return true;
    }

    case PieceType::Pao: {
        if (dx != 0 && dy != 0) {
            return false;
        }
        const int stepCol = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
        const int stepRow = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
        int col = srcCol + stepCol;
        int row = srcRow + stepRow;
        int blockCount = 0;
        while (col != dstCol || row != dstRow) {
            if (getPieceByPos(col, row) != nullptr) {
                ++blockCount;
            }
            col += stepCol;
            row += stepRow;
        }
        return target == nullptr ? blockCount == 0 : blockCount == 1;
    }

    case PieceType::Bing:
        if (piece->camp() != Camp::Red) {
            return false;
        }
        if (srcRow >= 5) {
            return dx == 0 && dy == -1;
        }
        return (dx == 0 && dy == -1) || (ady == 0 && adx == 1);

    case PieceType::Zu:
        if (piece->camp() != Camp::Black) {
            return false;
        }
        if (srcRow <= 4) {
            return dx == 0 && dy == 1;
        }
        return (dx == 0 && dy == 1) || (ady == 0 && adx == 1);

    default:
        return false;
    }
}

void ChessManager::clearAllSelect()
{
    for (ChessPiece *piece : m_pieces) {
        if (piece != nullptr) {
            piece->setSelected(false);
        }
    }
    m_selectedPiece = nullptr;
    emit selectChanged();
}

void ChessManager::initChess()
{
    clearAllSelect();
    qDeleteAll(m_pieces);
    m_pieces.clear();
    m_curTurn = Camp::Red;

    int id = 0;
    auto addPiece = [&](Camp camp, PieceType type, int col, int row) {
        ChessPiece *piece = new ChessPiece(id++, camp, type, this);
        piece->setLogicPos(col, row);
        m_pieces.append(piece);
    };

    addPiece(Camp::Black, PieceType::Ju, 0, 0);
    addPiece(Camp::Black, PieceType::Ma, 1, 0);
    addPiece(Camp::Black, PieceType::Xiang, 2, 0);
    addPiece(Camp::Black, PieceType::Shi, 3, 0);
    addPiece(Camp::Black, PieceType::Shuai, 4, 0);
    addPiece(Camp::Black, PieceType::Shi, 5, 0);
    addPiece(Camp::Black, PieceType::Xiang, 6, 0);
    addPiece(Camp::Black, PieceType::Ma, 7, 0);
    addPiece(Camp::Black, PieceType::Ju, 8, 0);
    addPiece(Camp::Black, PieceType::Pao, 1, 2);
    addPiece(Camp::Black, PieceType::Pao, 7, 2);
    addPiece(Camp::Black, PieceType::Zu, 0, 3);
    addPiece(Camp::Black, PieceType::Zu, 2, 3);
    addPiece(Camp::Black, PieceType::Zu, 4, 3);
    addPiece(Camp::Black, PieceType::Zu, 6, 3);
    addPiece(Camp::Black, PieceType::Zu, 8, 3);

    addPiece(Camp::Red, PieceType::Ju, 0, 9);
    addPiece(Camp::Red, PieceType::Ma, 1, 9);
    addPiece(Camp::Red, PieceType::Xiang, 2, 9);
    addPiece(Camp::Red, PieceType::Shi, 3, 9);
    addPiece(Camp::Red, PieceType::Jiang, 4, 9);
    addPiece(Camp::Red, PieceType::Shi, 5, 9);
    addPiece(Camp::Red, PieceType::Xiang, 6, 9);
    addPiece(Camp::Red, PieceType::Ma, 7, 9);
    addPiece(Camp::Red, PieceType::Ju, 8, 9);
    addPiece(Camp::Red, PieceType::Pao, 1, 7);
    addPiece(Camp::Red, PieceType::Pao, 7, 7);
    addPiece(Camp::Red, PieceType::Bing, 0, 6);
    addPiece(Camp::Red, PieceType::Bing, 2, 6);
    addPiece(Camp::Red, PieceType::Bing, 4, 6);
    addPiece(Camp::Red, PieceType::Bing, 6, 6);
    addPiece(Camp::Red, PieceType::Bing, 8, 6);

    emit piecesChanged();
    emit turnChanged();
    emit goGamePage();
}

void ChessManager::selectPiece(ChessPiece *piece)
{
    if (piece == nullptr || !piece->isAlive()) {
        return;
    }

    if (m_selectedPiece == nullptr) {
        if (piece->camp() != m_curTurn) {
            return;
        }
        m_selectedPiece = piece;
        m_selectedPiece->setSelected(true);
        emit selectChanged();
        return;
    }

    if (piece->camp() == m_curTurn) {
        m_selectedPiece->setSelected(false);
        m_selectedPiece = piece;
        m_selectedPiece->setSelected(true);
        emit selectChanged();
        return;
    }

    const QPoint targetPos = piece->getLogicPos();
    moveSelectedPiece(targetPos.x(), targetPos.y());
}

void ChessManager::moveSelectedPiece(int dstCol, int dstRow)
{
    if (m_selectedPiece == nullptr || !m_selectedPiece->isAlive()) {
        return;
    }
    if (m_selectedPiece->camp() != m_curTurn) {
        return;
    }

    ChessPiece *targetPiece = getPieceByPos(dstCol, dstRow);
    if (!checkMoveRule(m_selectedPiece, dstCol, dstRow)) {
        qDebug() << "走棋规则非法";
        return;
    }

    if (targetPiece == nullptr || !isGeneral(targetPiece->pieceType())) {
        ChessPiece *redGeneral = nullptr;
        ChessPiece *blackGeneral = nullptr;
        for (ChessPiece *piece : m_pieces) {
            if (piece == nullptr || !piece->isAlive()) {
                continue;
            }
            if (piece->pieceType() == PieceType::Jiang) {
                redGeneral = piece;
            } else if (piece->pieceType() == PieceType::Shuai) {
                blackGeneral = piece;
            }
        }

        QPoint redPos = redGeneral ? redGeneral->getLogicPos() : QPoint(-1, -1);
        QPoint blackPos = blackGeneral ? blackGeneral->getLogicPos() : QPoint(-1, -1);
        if (m_selectedPiece == redGeneral) {
            redPos = QPoint(dstCol, dstRow);
        } else if (m_selectedPiece == blackGeneral) {
            blackPos = QPoint(dstCol, dstRow);
        }

        if (redPos.x() == blackPos.x() && redPos.x() >= 0) {
            const int minRow = qMin(redPos.y(), blackPos.y());
            const int maxRow = qMax(redPos.y(), blackPos.y());
            int blockCount = 0;
            for (ChessPiece *piece : m_pieces) {
                if (piece == nullptr || !piece->isAlive() || piece == m_selectedPiece || piece == targetPiece) {
                    continue;
                }
                const QPoint pos = piece->getLogicPos();
                if (pos.x() == redPos.x() && pos.y() > minRow && pos.y() < maxRow) {
                    ++blockCount;
                }
            }
            if (m_selectedPiece != redGeneral && m_selectedPiece != blackGeneral
                && dstCol == redPos.x() && dstRow > minRow && dstRow < maxRow) {
                ++blockCount;
            }
            if (blockCount == 0) {
                qDebug() << "将帅不能直接照面";
                return;
            }
        }
    }

    const Camp moverCamp = m_selectedPiece->camp();
    const bool capturedGeneral = targetPiece != nullptr && isGeneral(targetPiece->pieceType());

    if (targetPiece != nullptr) {
        targetPiece->setAlive(false);
        m_pieces.removeOne(targetPiece);
        emit piecesChanged();
        targetPiece->deleteLater();
    }

    m_selectedPiece->setLogicPos(dstCol, dstRow);
    m_selectedPiece->setSelected(false);
    m_selectedPiece = nullptr;
    emit selectChanged();

    if (capturedGeneral) {
        emit gameOver(winnerName(moverCamp));
        return;
    }

    m_curTurn = (m_curTurn == Camp::Red) ? Camp::Black : Camp::Red;
    emit turnChanged();
}

void ChessManager::createServer(quint16 port)
{
    if (m_tcpServer) {
        m_tcpServer->close();
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    }

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, [this]() {
        if (m_tcpSocket && m_tcpSocket->parent() == this) {
            m_tcpSocket->deleteLater();
        }
        m_tcpSocket = m_tcpServer->nextPendingConnection();
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, [this]() {
            QDataStream ds(m_tcpSocket);
            int cmd = -1;
            ds >> cmd;
            if (cmd == 0) {
                int id = -1;
                int x = 0;
                int y = 0;
                ds >> id >> x >> y;
                syncRemoteMove(id, x, y);
            }
        });
    });

    m_online = m_tcpServer->listen(QHostAddress::Any, port);
    emit onlineChanged();
}

void ChessManager::connectServer(const QString &ip, quint16 port)
{
    if (m_tcpSocket == nullptr) {
        m_tcpSocket = new QTcpSocket(this);
    }
    connect(m_tcpSocket, &QTcpSocket::connected, this, [this]() {
        m_online = true;
        emit onlineChanged();
    });
    m_tcpSocket->connectToHost(ip, port);
}

void ChessManager::sendMoveData(int id, int x, int y)
{
    if (m_tcpSocket == nullptr || !m_tcpSocket->isOpen()) {
        return;
    }
    QDataStream ds(m_tcpSocket);
    ds << 0 << id << x << y;
}

void ChessManager::syncRemoteMove(int id, int x, int y)
{
    for (ChessPiece *piece : m_pieces) {
        if (piece == nullptr || piece->pieceId() != id) {
            continue;
        }
        const int dstCol = qRound((x + piece->pieceW() / 2.0 - BOARD_OFFSET_X) / GRID_SIZE);
        const int dstRow = qRound((y + piece->pieceH() / 2.0 - BOARD_OFFSET_Y) / GRID_SIZE);
        clearAllSelect();
        m_selectedPiece = piece;
        piece->setSelected(true);
        moveSelectedPiece(dstCol, dstRow);
        break;
    }
}