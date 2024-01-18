#include "fallbackTileBuilder.h"
#include "mytile.h"

QGVDrawItem* FallbackTileBuilder::build(const QGV::GeoTilePos& tilePos){
    auto tile = new MyTile(tilePos, tileColor);
    tile->setOpacity(0.7);
    return tile;

}

void FallbackTileBuilder::setFallbackText(QString text){
    this->tileText = text;
}

void FallbackTileBuilder::setFallbackColor(QColor color){
    this->tileColor = color;
}
