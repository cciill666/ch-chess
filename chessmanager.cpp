#include "chessmanager.h"

#include <QDataStream>
#include <QHostAddress>
#include <QDebug>
#include <QtMath>

namespace {

    bool isGeneral(ChessPiece::PieceType type)
    {
        return type == ChessPiece::PieceType::Jiang || type == ChessPiece::PieceType::Shuai;
    }

    QString winnerName(ChessPiece::Camp camp)
    {
        return camp == ChessPiece::Camp::Red ? QStringLiteral("红方胜利") : QStringLiteral("黑方胜利");
    }

}

ChessManager::ChessManager(QObject *parent)
    : QObject(parent)
    , m_board(new ChessBoard(this))
    , m_selectedPiece(nullptr)
    , m_curTurn(ChessPiece::Camp::Red)
    , m_online(false)
    , m_myCamp(ChessPiece::Camp::Red)
    , m_tcpServer(nullptr)
    , m_tcpSocket(new QTcpSocket(this))
    , m_ai(new ChessAI(this))
    , m_aiTimer(new QTimer(this))
{
    setupSocketSignals(m_tcpSocket);

    // 延迟500ms后执行AI走棋，让UI有时间更新
    m_aiTimer->setSingleShot(true);
    m_aiTimer->setInterval(500);
    connect(m_aiTimer, &QTimer::timeout, this, &ChessManager::onAiTimeout);
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

ChessPiece *ChessManager::selectedPiece() const
{
    return m_selectedPiece;
}

QList<ChessPiece *> &ChessManager::pieceList()
{
    return m_pieces;
}

ChessPiece::Camp ChessManager::currentTurn() const
{
    return m_curTurn;
}

bool ChessManager::isOnline() const
{
    return m_online;
}

ChessPiece::Camp ChessManager::myCamp() const
{
    return m_myCamp;
}

QString ChessManager::connectionStatus() const
{
    return m_connectionStatus;
}

void ChessManager::setOnline(bool online)
{
    if (m_online != online) {
        m_online = online;
        emit onlineChanged();
    }
}

void ChessManager::setConnectionStatus(const QString& status)
{
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        emit connectionStatusChanged();
    }
}

bool ChessManager::isAiMode() const
{
    return m_aiMode;
}

bool ChessManager::isAiThinking() const
{
    return m_aiThinking;
}

void ChessManager::setAiMode(bool enabled)
{
    if (m_aiMode != enabled) {
        m_aiMode = enabled;
        emit aiModeChanged();
    }
}

void ChessManager::setAiThinking(bool thinking)
{
    if (m_aiThinking != thinking) {
        m_aiThinking = thinking;
        emit aiThinkingChanged();
    }
}

void ChessManager::setupSocketSignals(QTcpSocket* socket)
{
    if (!socket) return;

    connect(socket, &QTcpSocket::disconnected, this, &ChessManager::onSocketDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &ChessManager::onSocketError);
    connect(socket, &QTcpSocket::readyRead, this, &ChessManager::onSocketReadyRead);
}

void ChessManager::onSocketDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket != m_tcpSocket) return;

    setOnline(false);
    setConnectionStatus(QStringLiteral("连接已断开"));
    qDebug() << "网络连接已断开";
}

void ChessManager::onSocketError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket != m_tcpSocket) return;

    QString errStr;
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        errStr = QStringLiteral("连接被拒绝"); break;
    case QAbstractSocket::RemoteHostClosedError:
        errStr = QStringLiteral("远程主机关闭连接"); break;
    case QAbstractSocket::HostNotFoundError:
        errStr = QStringLiteral("主机未找到"); break;
    case QAbstractSocket::SocketTimeoutError:
        errStr = QStringLiteral("连接超时"); break;
    default:
        errStr = QStringLiteral("网络错误: ") + m_tcpSocket->errorString();
    }
    setConnectionStatus(errStr);
    setOnline(false);
    qDebug() << "网络错误:" << errStr;
}

void ChessManager::onSocketReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket != m_tcpSocket) return;

    QDataStream ds(socket);
    ds.setVersion(QDataStream::Qt_6_0);

    while (true) {

        if (socket->bytesAvailable() < static_cast<qint64>(sizeof(int))) {
            break;
        }
        int cmd = -1;
        ds >> cmd;

        if (cmd == 0) {

            if (socket->bytesAvailable() < static_cast<qint64>(sizeof(int) * 3)) {
                qDebug() << "走棋数据不完整，等待后续数据";
                break;
            }
            int id = -1;
            int x = 0;
            int y = 0;
            ds >> id >> x >> y;
            syncRemoteMove(id, x, y);
        } else if (cmd == 1) {
            // 开始游戏命令
            qDebug() << "客户端收到开始游戏命令，初始化棋盘";
            initChess();
        } else {
            qDebug() << "未知命令:" << cmd;
        }
    }
}

void ChessManager::onClientConnected()
{
    if (!m_tcpServer) return;

    // 关闭旧连接
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
    }

    m_tcpSocket = m_tcpServer->nextPendingConnection();
    setupSocketSignals(m_tcpSocket);
    setOnline(true);
    setConnectionStatus(QStringLiteral("客户端已连接"));
    m_myCamp = ChessPiece::Camp::Red;
    emit myCampChanged();
    qDebug() << "客户端已连接，本方为红方";

    // 发送开始游戏命令给客户端
    QDataStream ds(m_tcpSocket);
    ds.setVersion(QDataStream::Qt_6_0);
    ds << 1;  // cmd = 1 表示开始游戏
    m_tcpSocket->flush();
    qDebug() << "服务端发送开始游戏命令";

    // 初始化棋盘并进入游戏
    initChess();
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
    const bool isRed = piece->camp() == ChessPiece::Camp::Red;

    switch (piece->pieceType()) {
    case ChessPiece::PieceType::Jiang:
    case ChessPiece::PieceType::Shuai:
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

    case ChessPiece::PieceType::Shi:
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

    case ChessPiece::PieceType::Xiang: {
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

    case ChessPiece::PieceType::Ma: {
        const bool validStep = (adx == 2 && ady == 1) || (adx == 1 && ady == 2);
        if (!validStep) {
            return false;
        }
        const int legCol = srcCol + (adx == 2 ? (dx > 0 ? 1 : -1) : 0);
        const int legRow = srcRow + (ady == 2 ? (dy > 0 ? 1 : -1) : 0);
        return getPieceByPos(legCol, legRow) == nullptr;
    }

    case ChessPiece::PieceType::Ju: {
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

    case ChessPiece::PieceType::Pao: {
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

    case ChessPiece::PieceType::Bing:
        if (piece->camp() != ChessPiece::Camp::Red) {
            return false;
        }
        if (srcRow >= 5) {
            return dx == 0 && dy == -1;
        }
        return (dx == 0 && dy == -1) || (ady == 0 && adx == 1);

    case ChessPiece::PieceType::Zu:
        if (piece->camp() != ChessPiece::Camp::Black) {
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

void ChessManager::initChess(bool resetAiMode)
{
    if (resetAiMode) {
        setAiMode(false);
    }
    setAiThinking(false);
    m_aiTimer->stop();
    clearAllSelect();
    qDeleteAll(m_pieces);
    m_pieces.clear();
    m_curTurn = ChessPiece::Camp::Red;

    int id = 0;
    auto addPiece = [&](ChessPiece::Camp camp, ChessPiece::PieceType type, int col, int row) {
        ChessPiece *piece = new ChessPiece(id++, camp, type, this);
        piece->setLogicPos(col, row);
        m_pieces.append(piece);
    };

    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Ju, 0, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Ma, 1, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Xiang, 2, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Shi, 3, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Shuai, 4, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Shi, 5, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Xiang, 6, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Ma, 7, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Ju, 8, 0);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Pao, 1, 2);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Pao, 7, 2);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Zu, 0, 3);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Zu, 2, 3);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Zu, 4, 3);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Zu, 6, 3);
    addPiece(ChessPiece::Camp::Black, ChessPiece::PieceType::Zu, 8, 3);

    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Ju, 0, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Ma, 1, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Xiang, 2, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Shi, 3, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Jiang, 4, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Shi, 5, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Xiang, 6, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Ma, 7, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Ju, 8, 9);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Pao, 1, 7);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Pao, 7, 7);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Bing, 0, 6);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Bing, 2, 6);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Bing, 4, 6);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Bing, 6, 6);
    addPiece(ChessPiece::Camp::Red, ChessPiece::PieceType::Bing, 8, 6);

    emit piecesChanged();
    emit turnChanged();
    emit goGamePage();
}

void ChessManager::initAiChess()
{
    m_myCamp = ChessPiece::Camp::Red;
    m_ai->setAiCamp(ChessPiece::Camp::Black);
    initChess(false);  // 不重置AI模式
    setAiMode(true);
    emit myCampChanged();
    setConnectionStatus(QStringLiteral("人机对战 - 你执红方"));
}

void ChessManager::triggerAiMove()
{
    if (!m_aiMode || m_aiThinking) return;
    if (m_curTurn != m_ai->aiCamp()) return;

    setAiThinking(true);
    setConnectionStatus(QStringLiteral("AI 思考中..."));
    m_aiTimer->start();
}

void ChessManager::onAiTimeout()
{
    if (!m_aiMode || !m_aiThinking) return;

    AiMove bestMove = m_ai->findBestMove(m_pieces, m_curTurn);

    if (bestMove.fromId < 0) {
        setAiThinking(false);
        setConnectionStatus(QStringLiteral("AI 无可用走法"));
        return;
    }

    // 找到要移动的棋子
    for (ChessPiece *piece : m_pieces) {
        if (piece && piece->isAlive() && piece->pieceId() == bestMove.fromId) {
            clearAllSelect();
            m_selectedPiece = piece;
            piece->setSelected(true);
            moveSelectedPiece(bestMove.toCol, bestMove.toRow);
            break;
        }
    }

    setAiThinking(false);
    setConnectionStatus(QStringLiteral("人机对战 - 你执红方"));
}

void ChessManager::selectPiece(ChessPiece *piece)
{
    if (piece == nullptr || !piece->isAlive()) {
        return;
    }

    // AI模式下，不允许直接选中AI方的棋子（但允许吃子）
    if (m_aiMode && piece->camp() == m_ai->aiCamp()) {
        if (m_selectedPiece == nullptr || m_selectedPiece->camp() != m_curTurn) {
            return;
        }
    }

    // AI思考中不允许操作
    if (m_aiThinking) {
        return;
    }

    if (m_online && piece->camp() != m_myCamp) {
        if (m_selectedPiece != nullptr && m_selectedPiece->camp() == m_myCamp && m_curTurn == m_myCamp) {
            const QPoint targetPos = piece->getLogicPos();
            moveSelectedPiece(targetPos.x(), targetPos.y());
        }
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


    if (m_online && !m_isRemoteSync && m_curTurn != m_myCamp) {
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
            if (piece->pieceType() == ChessPiece::PieceType::Jiang) {
                redGeneral = piece;
            } else if (piece->pieceType() == ChessPiece::PieceType::Shuai) {
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

    const ChessPiece::Camp moverCamp = m_selectedPiece->camp();
    const bool capturedGeneral = targetPiece != nullptr && isGeneral(targetPiece->pieceType());

    // 发送网络数据
    if (m_online && !m_isRemoteSync) {
        sendMoveData(m_selectedPiece->pieceId(), dstCol, dstRow);
    }

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

    m_curTurn = (m_curTurn == ChessPiece::Camp::Red) ? ChessPiece::Camp::Black : ChessPiece::Camp::Red;
    emit turnChanged();

    // AI模式：轮到AI走棋时触发AI
    if (m_aiMode && m_curTurn == m_ai->aiCamp()) {
        triggerAiMove();
    }
}

void ChessManager::createServer(quint16 port)
{
    if (m_tcpServer) {
        m_tcpServer->close();
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    }

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ChessManager::onClientConnected);

    if (m_tcpServer->listen(QHostAddress::Any, port)) {
        setConnectionStatus(QStringLiteral("等待客户端连接..."));
        qDebug() << "服务端正在监听端口" << port;
    } else {
        setConnectionStatus(QStringLiteral("创建房间失败"));
        qDebug() << "创建房间失败:" << m_tcpServer->errorString();
    }
}

void ChessManager::connectServer(const QString &ip, quint16 port)
{
    if (m_tcpSocket == nullptr) {
        m_tcpSocket = new QTcpSocket(this);
    }

    // 断开旧连接
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
    }

    // 清理旧信号连接，重新连接
    disconnect(m_tcpSocket, nullptr, this, nullptr);
    setupSocketSignals(m_tcpSocket);

    connect(m_tcpSocket, &QTcpSocket::connected, this, [this]() {
        setOnline(true);
        setConnectionStatus(QStringLiteral("已连接到服务器，等待开始..."));
        m_myCamp = ChessPiece::Camp::Black;
        emit myCampChanged();
        qDebug() << "已连接到服务器，本方为黑方，等待开始游戏命令";
    });

    setConnectionStatus(QStringLiteral("正在连接..."));
    m_tcpSocket->connectToHost(ip, port);
}

void ChessManager::disconnectNetwork()
{
    if (m_tcpServer) {
        m_tcpServer->close();
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    }
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
    }
    setOnline(false);
    setConnectionStatus(QStringLiteral("已断开"));
}

void ChessManager::sendMoveData(int id, int x, int y)
{
    if (m_tcpSocket == nullptr || !m_tcpSocket->isOpen()) {
        qDebug() << "发送失败: socket 未打开";
        return;
    }
    QDataStream ds(m_tcpSocket);
    ds.setVersion(QDataStream::Qt_6_0);
    ds << 0 << id << x << y;
    m_tcpSocket->flush();
    qDebug() << "发送走棋数据:" << id << x << y;
}

void ChessManager::syncRemoteMove(int id, int x, int y)
{
    qDebug() << "收到远程走棋:" << id << x << y;
    for (ChessPiece *piece : m_pieces) {
        if (piece == nullptr || piece->pieceId() != id) {
            continue;
        }
        const int dstCol = x;
        const int dstRow = y;
        qDebug() << "远程走棋目标:" << dstCol << dstRow;
        m_isRemoteSync = true;
        clearAllSelect();
        m_selectedPiece = piece;
        piece->setSelected(true);
        moveSelectedPiece(dstCol, dstRow);
        m_isRemoteSync = false;
        break;
    }
}

void ChessManager::requestGoStartPage()
{
    setAiMode(false);
    setAiThinking(false);
    m_aiTimer->stop();
    emit goStartPage();
}
