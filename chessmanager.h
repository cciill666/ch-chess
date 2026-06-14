#pragma once

#include <QObject>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtQml>
#include "chessboard.h"
#include "chesspiece.h"
class ChessManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ChessBoard* board       READ boardObj CONSTANT)
    Q_PROPERTY(QList<ChessPiece*> pieces READ pieceList NOTIFY piecesChanged)
    Q_PROPERTY(Camp currentTurn        READ currentTurn NOTIFY turnChanged)
    Q_PROPERTY(bool onlineState       READ isOnline NOTIFY onlineChanged)
public:
    explicit ChessManager(QObject *parent = nullptr);
    ~ChessManager();
    ChessBoard*        boardObj() const;
    QList<ChessPiece*>& pieceList();
    Camp               currentTurn() const;
    bool               isOnline() const;
    ChessPiece* getPieceByPos(int col, int row);
    bool        checkMoveRule(ChessPiece* piece, int dstCol, int dstRow);
    void        clearAllSelect();
public slots:
    void initChess();
    void onPieceClick(ChessPiece* piece);
    void doMove(ChessPiece* piece, int dstX, int dstY);
    // 联机接口
    void createServer(quint16 port);
    void connectServer(const QString& ip, quint16 port);
    void sendMoveData(int id, int x, int y);
    void syncRemoteMove(int id, int x, int y);
signals:
    void goGamePage();
    void piecesChanged();
    void turnChanged();
    void onlineChanged();
    void gameOver(QString result);
private:
    ChessBoard*        m_board;
    QList<ChessPiece*>m_pieces;
    ChessPiece*        m_selectedPiece;
    Camp               m_curTurn;
    bool               m_online;
    QTcpServer*        m_tcpServer;
    QTcpSocket*        m_tcpSocket;
};