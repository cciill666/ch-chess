#pragma once

#include <QObject>
#include <QUrl>
#include <QPoint>
#include <QString>
#include "chessboard.h"

class ChessPiece : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    // 阵营枚举
    enum class Camp {
        Red,
        Black
    };
    Q_ENUM(Camp)

    // 棋子类型枚举
    enum class PieceType {
        Jiang,  // 红将
        Shuai,  // 黑帅
        Ju,     // 车
        Ma,     // 马
        Xiang,  // 相/象
        Shi,    // 士
        Pao,    // 炮
        Bing,   // 兵
        Zu      // 卒
    };
    Q_ENUM(PieceType)

    Q_PROPERTY(int      pieceId    READ pieceId    CONSTANT)
    Q_PROPERTY(QString     imgSource  READ imgSource  NOTIFY stateChanged)
    Q_PROPERTY(int      posX       READ posX       WRITE setPosX NOTIFY posChanged)
    Q_PROPERTY(int      posY       READ posY       WRITE setPosY NOTIFY posChanged)
    Q_PROPERTY(int      pieceW     READ pieceW     CONSTANT)
    Q_PROPERTY(int      pieceH     READ pieceH     CONSTANT)
    Q_PROPERTY(int      zOrder     READ zOrder     CONSTANT)
    Q_PROPERTY(bool     isAlive    READ isAlive    WRITE setAlive NOTIFY aliveChanged)
    Q_PROPERTY(bool     isSelected READ isSelected WRITE setSelected NOTIFY stateChanged)
    Q_PROPERTY(Camp     camp       READ camp       CONSTANT)
    Q_PROPERTY(PieceType pieceType READ pieceType  CONSTANT)
public:
    explicit ChessPiece(QObject *parent = nullptr);
    explicit ChessPiece(int id, Camp camp, PieceType type, QObject *parent = nullptr);
    int         pieceId() const;
    Camp        camp() const;
    PieceType   pieceType() const;
    bool        isAlive() const;
    bool        isSelected() const;
    int         posX() const;
    int         posY() const;
    int         pieceW() const;
    int         pieceH() const;
    int         zOrder() const;
    QString        imgSource() const;
    QPoint      getLogicPos() const;
    void        setLogicPos(int col, int row);
public slots:
    void setPosX(int x);
    void setPosY(int y);
    void setAlive(bool alive);
    void setSelected(bool selected);
signals:
    void posChanged();
    void aliveChanged();
    void stateChanged();
private:
    int         m_id;
    Camp        m_camp;
    PieceType   m_type;
    bool        m_alive;
    bool        m_selected;
    int         m_x;
    int         m_y;
    const int   m_w = 57;
    const int   m_h = 57;
    const int   m_z = 1;
    QString        m_normalImg;
    QString       m_selectImg;
};
