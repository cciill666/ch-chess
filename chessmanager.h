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
    QML_SINGLETON
    Q_PROPERTY(ChessBoard* board       READ boardObj CONSTANT)
    Q_PROPERTY(QList<ChessPiece*> pieces READ pieceList NOTIFY piecesChanged)
    Q_PROPERTY(Camp currentTurn        READ currentTurn NOTIFY turnChanged)
    Q_PROPERTY(bool onlineState       READ isOnline NOTIFY onlineChanged)
    Q_PROPERTY(ChessPiece* selectedPiece READ selectedPiece NOTIFY selectChanged)

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
    ChessPiece* selectedPiece() const;
public slots:
    void initChess();
    // void onPieceClick(ChessPiece* piece);
    // void doMove(ChessPiece* piece, int dstX, int dstY);


    void selectPiece(ChessPiece* piece);   // 新增：点击棋子选中
    void moveSelectedPiece(int dstCol, int dstRow); // 新增：点击棋盘落子
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
    void selectChanged();
private:
    ChessBoard*        m_board;
    QList<ChessPiece*>m_pieces;
    ChessPiece*        m_selectedPiece;
    Camp               m_curTurn;
    bool               m_online;
    bool               m_isRemoteSync = false;
    QTcpServer*        m_tcpServer;
    QTcpSocket*        m_tcpSocket;
};