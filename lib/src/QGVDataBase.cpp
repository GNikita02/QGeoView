#include "QGVDataBase.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>


QGVDataBase::QGVDataBase(const QString &db_name){
    this->db_name = db_name;
}

QGVDataBase::~QGVDataBase(){
    db.close();
}

void QGVDataBase::RequestLoadTile(const QString &url, const QGV::GeoTilePos &tilePos) {
    auto task = QSharedPointer<LoadTileTask>(new LoadTileTask());
    task->url = url;
    task->tile_pos = tilePos;

    QMutexLocker lock(&mutex);
    load_tile_tasks.enqueue(task);
    emit OnLoadProgress(load_tile_tasks.size());
    wait_condition.notify_all();
}

void QGVDataBase::RequestSaveTile(const QString &url, QByteArray byte_array){
    auto task = QSharedPointer<SaveTileTask>(new SaveTileTask());
    task->url = url;
    task->bytes = byte_array;

    QMutexLocker lock(&mutex);
    save_tile_tasks.enqueue(task);
    emit OnSaveProgress(save_tile_tasks.size());
    wait_condition.notify_all();
}

void QGVDataBase::run() {
    // создание базы
    QSqlDatabase db= QSqlDatabase::addDatabase("QSQLITE", db_name);
    db.setDatabaseName(db_name + ".db");
    db.open();
    QSqlQuery sql_query(db);
    sql_query.exec(
        "CREATE TABLE IF NOT EXISTS tiles ("
        "    tile_url text PRIMARY KEY NOT NULL,"
        "    tile_bytes blob"
        ") WITHOUT ROWID");

    while (isRunning()) {
        {
            QMutexLocker lk(&mutex);
            if (load_tile_tasks.size() == 0 && save_tile_tasks.size() == 0) {
                wait_condition.wait(&mutex);
            }
        }

        if(QSharedPointer<LoadTileTask> task = nextLoadTask(); task != nullptr) {
            //qDebug() << "SELECT" << task->url;
            QSqlQuery sql_query(db);
            sql_query.prepare("SELECT tile_bytes FROM tiles WHERE tile_url = :url LIMIT 1;");
            sql_query.bindValue(":url", task->url);
            QByteArray raw_image;
            if (sql_query.exec()) {
                while(sql_query.next()) {
                    raw_image = sql_query.value(0).toByteArray();
                    break;
                }
            }
            emit OnLoadTile(task->url, task->tile_pos, raw_image);
            continue;
        }

        int chank = 20;
        if(save_tile_tasks.size() >= chank){
            QMutexLocker locker(&mutex);
            QString sql_insert_query = "INSERT INTO tiles (tile_url, tile_bytes)"
                                       "   VALUES";
            for(int i = 0; i < chank; i++){
                QSharedPointer<SaveTileTask> task = save_tile_tasks.dequeue();
                sql_insert_query += " (" + task->url + ", " + task->bytes  + "),";
            }
            sql_insert_query.remove(sql_insert_query.size() - 1,sql_insert_query.size());
            sql_insert_query += "   ON CONFLICT(tile_url)"
                                "   DO UPDATE SET tile_bytes=excluded.tile_bytes;";
            sql_query.exec(sql_insert_query);
            locker.~QMutexLocker();
            continue;
        }
        if(QSharedPointer<SaveTileTask> task = nextSaveTask(); task != nullptr) {
            //qDebug() << "INSERT" << task->url;
            QSqlQuery sql_query(db);

            sql_query.prepare(
                "INSERT INTO tiles (tile_url, tile_bytes)"
                "   VALUES(:url, :bytes)"
                "   ON CONFLICT(tile_url)"
                "   DO UPDATE SET tile_bytes=excluded.tile_bytes;");
            sql_query.bindValue(":url", task->url);
            sql_query.bindValue(":bytes", task->bytes);
            sql_query.exec();
            continue;
        }
    }

    // закрытие быза
    db.close();
}

QSharedPointer<LoadTileTask> QGVDataBase::nextLoadTask() {
    QMutexLocker lk(&mutex);
    if(load_tile_tasks.size() > 0) {
        QSharedPointer<LoadTileTask> task = load_tile_tasks.back();
        load_tile_tasks.pop_back();
        emit OnLoadProgress(load_tile_tasks.size());
        return task;
    }
    return nullptr;
}

QSharedPointer<SaveTileTask> QGVDataBase::nextSaveTask() {
    QMutexLocker lk(&mutex);
    if(save_tile_tasks.size() > 0) {
        QSharedPointer<SaveTileTask> task = save_tile_tasks.dequeue();
        emit OnSaveProgress(save_tile_tasks.size());
        return task;
    }
    return nullptr;
}
