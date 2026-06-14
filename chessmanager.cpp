#include "chessmanager.h"
#include <QHostAddress>
#include <QDataStream>

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
        int cmd;
        ds >> cmd;
        if (cmd == 0) {
            int id, x, y;
            ds >> id >> x >> y;
            syncRemoteMove(id, x, y);
        }
    });
}

ChessManager::~ChessManager()
{
    qDeleteAll(m_pieces);
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

ChessPiece *ChessManager::getPieceByPos(int col, int row) {
    for (auto p : m_pieces) {
        if (!p->isAlive()) continue;
        QPoint pos = p->getLogicPos();
        if (pos.x() == col && pos.y() == row)
            return p;
    }
    return nullptr;
}

bool ChessManager::checkMoveRule(ChessPiece *piece, int dstCol, int dstRow)
{
    if (!piece || !piece->isAlive()) return false;
    QPoint srcPos = piece->getLogicPos();
    int srcCol = srcPos.x();
    int srcRow = srcPos.y();
    if (dstCol < 0 || dstCol >= COL_COUNT || dstRow <0 || dstRow >= ROW_COUNT)
        return false;
    ChessPiece* target = getPieceByPos(dstCol, dstRow);
    if (target && target->camp() == piece->camp())
        return false;
    int dx = dstCol - srcCol;
    int dy = dstRow - srcRow;
    // 走子规则（沿用原有逻辑）
    switch (piece->pieceType())
    {
    case PieceType::Jiang:
    case PieceType::Shuai:
    {
        if (dstCol <3 || dstCol>5) return false;
        if((piece->camp()==Camp::Red && dstRow>2) || (piece->camp()==Camp::Black && dstRow<7))
            return false;
        return (qAbs(dx)==1 && dy==0) || (qAbs(dy)==1 && dx==0);
    }
    case PieceType::Ju:
    {
        if(dx!=0 && dy!=0) return false;
        int stepX = dx ? (dx>0?1:-1) : 0;
        int stepY = dy ? (dy>0?1:-1) : 0;
        int c = srcCol + stepX;
        int r = srcRow + stepY;
        while(c != dstCol || r != dstRow)
        {
            if(getPieceByPos(c,r)) return false;
            c += stepX;
            r += stepY;
        }
        return true;
    }
    case PieceType::Ma:
    {
        if(!((qAbs(dx)==2 && qAbs(dy)==1) || (qAbs(dx)==1 && qAbs(dy)==2)))
            return false;
        int blockC, blockR;
        if(qAbs(dx)==2)
        {
            blockC = srcCol + (dx>0?1:-1);
            blockR = srcRow;
        }
        else
        {
            blockC = srcCol;
            blockR = srcRow + (dy>0?1:-1);
        }
        return !getPieceByPos(blockC, blockR);
    }
    case PieceType::Xiang:
    {
        if(qAbs(dx)!=2 || qAbs(dy)!=2) return false;
        if((piece->camp()==Camp::Red && dstRow>4) || (piece->camp()==Camp::Black && dstRow<5))
            return false;
        int midC = (srcCol + dstCol)/2;
        int midR = (srcRow + dstRow)/2;
        return !getPieceByPos(midC, midR);
    }
    case PieceType::Shi:
    {
        if(dstCol<3 || dstCol>5) return false;
        if((piece->camp()==Camp::Red && dstRow>2) || (piece->camp()==Camp::Black && dstRow<7))
            return false;
        return qAbs(dx)==1 && qAbs(dy)==1;
    }
    case PieceType::Pao:
    {
        if(dx!=0 && dy!=0) return false;
        int stepX = dx ? (dx>0?1:-1) : 0;
        int stepY = dy ? (dy>0?1:-1) : 0;
        int c = srcCol + stepX;
        int r = srcRow + stepY;
        int count = 0;
        while(c != dstCol || r != dstRow)
        {
            if(getPieceByPos(c,r)) count++;
            c += stepX;
            r += stepY;
        }
        return target ? (count == 1) : (count == 0);
    }
    case PieceType::Bing:
    case PieceType::Zu:
    {
        if(piece->camp() == Camp::Red)
        {
            if(srcRow > 4)
                return (dy == -1 && dx == 0);
            else
                return (dy == -1 && dx == 0) || (qAbs(dx) == 1 && dy == 0);
        }
        else
        {
            if(srcRow < 5)
                return (dy == 1 && dx == 0);
            else
                return (dy == 1 && dx == 0) || (qAbs(dx) == 1 && dy == 0);
        }
    }
    default:
        return false;
    }
}

void ChessManager::clearAllSelect()
{
    for (auto p : m_pieces)
        p->setSelected(false);
    m_selectedPiece = nullptr;
}

void ChessManager::initChess()
{
    qDeleteAll(m_pieces);
    m_pieces.clear();
    clearAllSelect();
    m_curTurn = Camp::Red;
    int id = 0;
    // 红方
    auto addRed = [&](PieceType t, int col, int row){
        ChessPiece* p = new ChessPiece(id++, Camp::Red, t, this);
        p->setLogicPos(col, row);
        m_pieces.append(p);
    };
    addRed(PieceType::Ju,    0,9);
    addRed(PieceType::Ma,    1,9);
    addRed(PieceType::Xiang, 2,9);
    addRed(PieceType::Shi,   3,9);
    addRed(PieceType::Jiang, 4,9);
    addRed(PieceType::Shi,   5,9);
    addRed(PieceType::Xiang, 6,9);
    addRed(PieceType::Ma,    7,9);
    addRed(PieceType::Ju,    8,9);
    addRed(PieceType::Pao,   1,7);
    addRed(PieceType::Pao,   7,7);
    addRed(PieceType::Bing,  0,6);
    addRed(PieceType::Bing,  2,6);
    addRed(PieceType::Bing,  4,6);
    addRed(PieceType::Bing,  6,6);
    addRed(PieceType::Bing,  8,6);
    // 黑方
    auto addBlack = [&](PieceType t, int col, int row){
        ChessPiece* p = new ChessPiece(id++, Camp::Black, t, this);
        p->setLogicPos(col, row);
        m_pieces.append(p);
    };
    addBlack(PieceType::Ju,    0,0);
    addBlack(PieceType::Ma,    1,0);
    addBlack(PieceType::Xiang, 2,0);
    addBlack(PieceType::Shi,   3,0);
    addBlack(PieceType::Shuai, 4,0);
    addBlack(PieceType::Shi,   5,0);
    addBlack(PieceType::Xiang, 6,0);
    addBlack(PieceType::Ma,    7,0);
    addBlack(PieceType::Ju,    8,0);
    addBlack(PieceType::Pao,   1,2);
    addBlack(PieceType::Pao,   7,2);
    addBlack(PieceType::Zu,    0,3);
    addBlack(PieceType::Zu,    2,3);
    addBlack(PieceType::Zu,    4,3);
    addBlack(PieceType::Zu,    6,3);
    addBlack(PieceType::Zu,    8,3);
    emit piecesChanged();
    emit turnChanged();
    emit goGamePage();
}

void ChessManager::onPieceClick(ChessPiece *piece)
{
    if(!piece || !piece->isAlive() || piece->camp() != m_curTurn)
        return;
    clearAllSelect();
    piece->setSelected(true);
    m_selectedPiece = piece;
}

void ChessManager::doMove(ChessPiece *piece, int dstX, int dstY)
{
    if(!piece || !piece->isAlive() || piece->camp() != m_curTurn)
        return;
    int col = qRound((dstX + piece->pieceW()/2 - BOARD_OFFSET_X) / (double)GRID_SIZE);
    int row = qRound((dstY + piece->pieceH()/2 - BOARD_OFFSET_Y) / (double)GRID_SIZE);
    if(!checkMoveRule(piece, col, row)) return;
    ChessPiece* target = getPieceByPos(col, row);
    if(target) target->setAlive(false);
    piece->setLogicPos(col, row);
    clearAllSelect();
    // 切换回合
    m_curTurn = (m_curTurn == Camp::Red) ? Camp::Black : Camp::Red;
    emit turnChanged();
    // 判断胜负
    if(target && (target->pieceType() == PieceType::Jiang || target->pieceType() == PieceType::Shuai))
    {
        QString res = (piece->camp() == Camp::Red) ? "红方胜利！" : "黑方胜利！";
        emit gameOver(res);
    }
}

void ChessManager::createServer(quint16 port)
{
    if (!m_tcpServer) {
        m_tcpServer = new QTcpServer(this);
    }
    if (m_tcpServer->listen(QHostAddress::Any, port)) {
        connect(m_tcpServer, &QTcpServer::newConnection, this, [this]() {
            QTcpSocket* newSocket = m_tcpServer->nextPendingConnection();
            if (!newSocket) return;
            if (m_tcpSocket) {
                m_tcpSocket->disconnectFromHost();
                m_tcpSocket->deleteLater();
            }
            m_tcpSocket = newSocket;
            m_online = true;
            emit onlineChanged();
        });
    }
}
void ChessManager::connectServer(const QString &ip, quint16 port)
{
    if (!m_tcpSocket) {
        m_tcpSocket = new QTcpSocket(this);
    }
    m_tcpSocket->connectToHost(ip, port);
    connect(m_tcpSocket, &QTcpSocket::connected, this, [this]() {
        m_online = true;
        emit onlineChanged();
    });
}

void ChessManager::sendMoveData(int id, int x, int y)
{
    if(!m_tcpSocket || !m_tcpSocket->isOpen()) return;
    QDataStream ds(m_tcpSocket);
    ds << 0 << id << x << y;
}

void ChessManager::syncRemoteMove(int id, int x, int y)
{
    for(auto p : m_pieces)
    {
        if(p->pieceId() == id)
        {
            doMove(p, x, y);
            break;
        }
    }
}
