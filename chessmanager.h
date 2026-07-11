#pragma once

#include <QObject>
#include <QList>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtQml/qqmlregistration.h>
#include "chessboard.h"
#include "chesspiece.h"
#include "chessai.h"

// 悔棋记录：保存一步走棋的全部信息，用于撤销
struct MoveRecord {
    int pieceId = -1;                         // 移动的棋子ID
    int fromCol = -1;                         // 起点列
    int fromRow = -1;                         // 起点行
    int toCol = -1;                           // 终点列
    int toRow = -1;                           // 终点行
    ChessPiece::Camp moverCamp = ChessPiece::Camp::Red;  // 走棋方
    // 被吃棋子信息（用于恢复）
    int capturedId = -1;                      // 被吃棋子ID
    ChessPiece::Camp capturedCamp = ChessPiece::Camp::Red;
    ChessPiece::PieceType capturedType = ChessPiece::PieceType::Ju;
    bool hadCapture = false;                  // 是否有吃子
};

class ChessManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(ChessBoard* board       READ boardObj CONSTANT)
    Q_PROPERTY(QList<ChessPiece*> pieces READ pieceList NOTIFY piecesChanged)
    Q_PROPERTY(ChessPiece::Camp currentTurn        READ currentTurn NOTIFY turnChanged)
    Q_PROPERTY(bool onlineState        READ isOnline NOTIFY onlineChanged)
    Q_PROPERTY(ChessPiece* selectedPiece READ selectedPiece NOTIFY selectChanged)
    Q_PROPERTY(ChessPiece::Camp myCamp             READ myCamp NOTIFY myCampChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool aiMode             READ isAiMode NOTIFY aiModeChanged)
    Q_PROPERTY(bool aiThinking         READ isAiThinking NOTIFY aiThinkingChanged)
    Q_PROPERTY(bool canUndo            READ canUndo NOTIFY canUndoChanged)

public:
    explicit ChessManager(QObject *parent = nullptr);
    ~ChessManager();
    ChessBoard*        boardObj() const;
    QList<ChessPiece*>& pieceList();
    ChessPiece::Camp   currentTurn() const;
    bool               isOnline() const;
    ChessPiece::Camp   myCamp() const;
    QString            connectionStatus() const;
    ChessPiece* getPieceByPos(int col, int row);
    bool        checkMoveRule(ChessPiece* piece, int dstCol, int dstRow);
    void        clearAllSelect();
    ChessPiece* selectedPiece() const;
    bool        isAiMode() const;
    bool        isAiThinking() const;
    bool        canUndo() const;           // 是否可以悔棋

public slots:
    void initChess(bool resetAiMode = true);
    void initAiChess();       // 人机对战初始化
    void selectPiece(ChessPiece* piece);
    void moveSelectedPiece(int dstCol, int dstRow);
    void undoMove();                       // 悔棋（本地/人机）
    void requestUndo();                    // 联机模式：发起悔棋请求
    void respondToUndo(bool agree);        // 联机模式：响应悔棋请求
    void doOnlineUndo();                   // 联机模式：执行悔棋

    // 联机接口
    void createServer(quint16 port);
    void connectServer(const QString& ip, quint16 port);
    void disconnectNetwork();
    void sendMoveData(int id, int dstCol, int dstRow);
    void syncRemoteMove(int id, int dstCol, int dstRow);
    void requestGoStartPage();

signals:
    void goGamePage();
    void goStartPage();
    void piecesChanged();
    void turnChanged();
    void onlineChanged();
    void gameOver(QString result);
    void selectChanged();
    void myCampChanged();
    void connectionStatusChanged();
    void aiModeChanged();
    void aiThinkingChanged();
    void canUndoChanged();
    void undoRequested();                  // 收到对方悔棋请求（QML弹窗）
    void undoResponded(bool agreed);       // 收到对方悔棋响应（同意/拒绝）

private slots:
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onClientConnected();
    void onAiTimeout();

private:
    void setupSocketSignals(QTcpSocket* socket);
    void setOnline(bool online);
    void setConnectionStatus(const QString& status);
    void setAiMode(bool enabled);
    void setAiThinking(bool thinking);
    void triggerAiMove();

    ChessBoard*        m_board;
    QList<ChessPiece*>m_pieces;
    ChessPiece*        m_selectedPiece;
    ChessPiece::Camp   m_curTurn;
    bool               m_online;
    ChessPiece::Camp   m_myCamp;
    QString            m_connectionStatus;
    bool               m_isRemoteSync = false;
    QTcpServer*        m_tcpServer;
    QTcpSocket*        m_tcpSocket;

    // AI相关
    ChessAI*           m_ai = nullptr;
    bool               m_aiMode = false;
    bool               m_aiThinking = false;
    QTimer*            m_aiTimer = nullptr;

    // 悔棋历史记录
    QList<MoveRecord>  m_moveHistory;
};
