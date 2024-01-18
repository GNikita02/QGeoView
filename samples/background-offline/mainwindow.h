/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2023 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#pragma once

#include <QHash>
#include <QLabel>
#include <QGroupBox>
#include <QMainWindow>

#include <QGeoView/QGVMap.h>

class QueueHelper {
public:
    void update(void* key, size_t size) {
        total_queue_size += size - queue_sizes[key];
        queue_sizes[key] = size;
    }

    int64_t GetQueueSize() const {
        return total_queue_size;
    }

private:
    int64_t total_queue_size {0};
    QHash<void*, int64_t> queue_sizes;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    QGroupBox* createOptionsList();

private:
    void UpdateProgress();


private:
    QGVMap* mMap;
    QLabel* cache_status;
    QueueHelper load_task_queue_size;
    QueueHelper save_task_queue_size;
};
