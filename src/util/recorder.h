#ifndef RECORDER_H
#define RECORDER_H

#include "protocol.h"

#include <QObject>
#include <QTime>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QImage>
#include <QMap>

class Recorder : public QObject
{
    Q_OBJECT
public:
    explicit Recorder(QObject *parent);
    static QImage TXT2PNG(QByteArray data);
    bool save(const QString &filename) const;
    void recordLine(const QString &line);

public slots:
    void record(char *line);

private:
    QTime watch;
    QByteArray data;
};

class Replayer: public QThread
{
    Q_OBJECT

public:
    explicit Replayer(QObject *parent, const QString &filename);
    static QByteArray PNG2TXT(const QString filename);

    void initCommandPair();
    QString &commandTranslation(QString &cmd);
    QString &commandProceed(QString &cmd);
    int getDuration() const;
    qreal getSpeed();

    bool m_isOldVersion;
    int m_commandSeriesCounter;

public slots:
    void uniform();
    void toggle();
    void speedUp();
    void slowDown();

protected:
    virtual void run();

private:
    QString filename;
    qreal speed;
    bool playing;
    QMutex mutex;
    QSemaphore play_sem;

    struct Pair{
        int elapsed;
        QString cmd;
    };
    QList<Pair> pairs;

    QMap<QString, QString> m_nameTranslation;
    QMap<QString, QSanProtocol::CommandType> m_commandMapping;
    QMap<QSanProtocol::PacketType, QList<QSanProtocol::CommandType> > m_packetTypeMapping;

signals:
    void command_parsed(const QString &cmd);
    void elasped(int secs);
    void speed_changed(qreal speed);
};

#endif // RECORDER_H
