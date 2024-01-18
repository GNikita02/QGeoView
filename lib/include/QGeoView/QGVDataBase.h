#pragma once

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QSharedPointer>
#include <QtSql/QSqlDatabase>

#include <QGVGlobal.h>


struct LoadTileTask {
    QGV::GeoTilePos tile_pos;
    QString url;
};

struct SaveTileTask {
    QString url;
    QByteArray bytes;
};

class QGVDataBase : public QThread {

    Q_OBJECT

public:
    explicit QGVDataBase(QString const& db_name);
    ~QGVDataBase();
    void RequestLoadTile(QString const & url, const QGV::GeoTilePos& tilePos);
    void RequestSaveTile(QString const & url, QByteArray byte_array);;

signals:
    void OnLoadTile(QString const & url, QGV::GeoTilePos tile_pos, QByteArray raw_image);
    void OnLoadProgress(size_t size);
    void OnSaveProgress(size_t size);

protected:
    void run() override;

private:
    QSharedPointer<LoadTileTask> nextLoadTask();
    QSharedPointer<SaveTileTask> nextSaveTask();

private:
    QSqlDatabase db;
    QString db_name;
    QMutex mutex;
    QWaitCondition wait_condition;
    QQueue<QSharedPointer<LoadTileTask>> load_tile_tasks;
    QQueue<QSharedPointer<SaveTileTask>> save_tile_tasks;
};
