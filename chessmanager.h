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

public slots:
    void initChess(bool resetAiMode = true);
    void initAiChess();       // 人机对战初始化
    void selectPiece(ChessPiece* piece);
    void moveSelectedPiece(int dstCol, int dstRow);

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
};
