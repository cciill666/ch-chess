#pragma once

#include <QObject>
#include <QUrl>
#include <QPoint>
#include <QtQml/qqmlregistration.h>
// QML_MODULE(ChessModule, 1.0)

enum Camp {
    CAMP_RED,
    CAMP_BLACK
};

enum PieceType {
    PIECE_JIANG, PIECE_SHUAI,
    PIECE_JU, PIECE_MA, PIECE_XIANG,
    PIECE_SHI, PIECE_PAO,
    PIECE_BING, PIECE_ZU
};

class ChessPiece : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pieceId READ pieceId CONSTANT)
    Q_PROPERTY(QUrl source READ source CONSTANT)
    Q_PROPERTY(int x READ x WRITE setX NOTIFY posChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY posChanged)
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(int z READ zValue CONSTANT)
    Q_PROPERTY(bool alive READ isAlive WRITE setAlive NOTIFY aliveChanged)
    QML_ELEMENT
public:
    explicit ChessPiece(int id, Camp camp, PieceType type, QObject *parent = nullptr);
    int pieceId() const;
    Camp camp() const;
    PieceType pieceType() const;
    bool isAlive() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    int zValue() const;
    QPoint logicPos() const;
    void setLogicPos(int col, int row);
public slots:
    void setX(int val);
    void setY(int val);
    void setAlive(bool ok);
signals:
    void posChanged();
    void aliveChanged();
private:
    int m_id;
    Camp m_camp;
    PieceType m_type;
    bool m_alive = true;
    int m_x = 0;
    int m_y = 0;
    int m_w = 48;
    int m_h = 48;
    int m_z = 1;
};