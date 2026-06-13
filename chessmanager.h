#pragma once

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QList>
#include "chessboard.h"
#include "chesspiece.h"

// QML_MODULE(ChessModule, 1.0)

class ChessManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isOnline READ isOnline NOTIFY onlineStateChanged)
    Q_PROPERTY(ChessBoard* board READ boardObj CONSTANT)
    Q_PROPERTY(int turn READ currentTurn NOTIFY turnChanged)
    Q_PROPERTY(QList<ChessPiece*> pieceList READ pieceList CONSTANT)

    QML_ELEMENT

public:
    explicit ChessManager(QObject *parent = nullptr);
    ~ChessManager();
    bool isOnline() const;
    ChessBoard* boardObj() const;
    QList<ChessPiece*>& pieceList();
    int currentTurn() const;
    ChessPiece* getPieceAt(int col, int row);
    bool checkMove(ChessPiece* piece, int dstCol, int dstRow);
public slots:
    void initChessLayout();
    void movePiece(ChessPiece* piece, int dstX, int dstY);
    void enterGame();
    void startServer(quint16 port);
    void connectServer(const QString& ip, quint16 port);
    void sendPieceMove(int id, int x, int y);
    void syncPieceMove(int id, int x, int y);
signals:
    void goToGame();
    void onlineStateChanged();
    void turnChanged();
    void gameOver(QString result);
private:
    bool m_online = false;
    int m_curTurn = CAMP_RED;
    QTcpServer* m_tcpServer = nullptr;
    QTcpSocket* m_tcpSocket = nullptr;
    ChessBoard* m_board = nullptr;
    QList<ChessPiece*> m_pieces;
};