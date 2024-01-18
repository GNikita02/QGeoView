#pragma once

#include "QGeoView/QGVTileBuilder.h"

class FallbackTileBuilder : public QGVTileBuilder{
public:
    QGVDrawItem* build(const QGV::GeoTilePos& tilePos) override;
    void setFallbackText(QString text);
    void setFallbackColor(QColor color);

private:
    QString tileText;
    QColor tileColor;
};
