#pragma once

#include <QGVGlobal.h>
#include <QGVDrawItem.h>

class QGVTileBuilder{

public:
    virtual ~QGVTileBuilder() = default;
    virtual QGVDrawItem* build(const QGV::GeoTilePos& tilePos) = 0;
};
