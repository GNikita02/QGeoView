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

#include "mainwindow.h"

#include <QGeoView/QGVLayerBing.h>
#include <QGeoView/QGVLayerGoogle.h>
#include <QGeoView/QGVLayerOSM.h>
#include <helpers.h>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

#include <QGeoView/QGVDataBase.h>
#include "fallbackTileBuilder.h"

MainWindow::MainWindow()
{
    //QGVDataBase *database = new QGVDataBase();


    qDebug() << 2;
    setWindowTitle("QGeoView Samples - background-offline");

    setCentralWidget(new QWidget());
    centralWidget()->setLayout(new QVBoxLayout());

    Helpers::setupCachedNetworkAccessManager(this);

    mMap = new QGVMap(this);
    centralWidget()->layout()->addWidget(mMap);

    cache_status = new QLabel(this);

    // Options list
    centralWidget()->layout()->addWidget(createOptionsList());


    UpdateProgress();
    centralWidget()->layout()->addWidget(cache_status);

    // Show whole world
    auto target = mMap->getProjection()->boundaryGeoRect();
    mMap->cameraTo(QGVCameraActions(mMap).scaleTo(target));
}

MainWindow::~MainWindow()
{
}

QGroupBox* MainWindow::createOptionsList()
{
    const QString customURI =
            "https://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/${z}/${y}/${x}";

    const QList<QPair<QString, QGVLayerTilesOnline*>> layers = {
        { "GOOGLE_SATELLITE", new QGVLayerGoogle(QGV::TilesType::Satellite) },
        { "GOOGLE_HYBRID", new QGVLayerGoogle(QGV::TilesType::Hybrid) },
        { "GOOGLE_SCHEMA", new QGVLayerGoogle(QGV::TilesType::Schema) },
        { "BING_SATELLITE", new QGVLayerBing(QGV::TilesType::Satellite) },
        { "BING_HYBRID", new QGVLayerBing(QGV::TilesType::Hybrid) },
        { "BING_SCHEMA", new QGVLayerBing(QGV::TilesType::Schema) },
        { "CUSTOM_OSM", new QGVLayerOSM(customURI) },
        { "OSM", new QGVLayerOSM() },
    };

    for (auto& [name, layer] : layers)
    {
        FallbackTileBuilder *fallbackTileBuilder = new FallbackTileBuilder();
        fallbackTileBuilder->setFallbackColor(Qt::gray);
        auto database = QSharedPointer<QGVDataBase>(new QGVDataBase(name));
        database->start();

        connect(database.get(), &QGVDataBase::OnLoadProgress, [this, database](int queue_size) {
            load_task_queue_size.update(database.get(), queue_size);
            UpdateProgress();
        });

        connect(database.get(), &QGVDataBase::OnSaveProgress, [this, database](int queue_size) {
            save_task_queue_size.update(database.get(), queue_size);
            UpdateProgress();
        });

        layer->SetDatabase(database);
        layer->SetTileBuilder(fallbackTileBuilder);
    }

    QGroupBox* groupBox = new QGroupBox(tr("Background maps"));
    groupBox->setLayout(new QVBoxLayout);

    QButtonGroup* group = new QButtonGroup(this);
    QRadioButton* radioButton;

    for (auto pair : layers) {
        auto name = pair.first;
        auto layer = pair.second;

        layer->hide();
        mMap->addItem(layer);

        radioButton = new QRadioButton(name);
        group->addButton(radioButton);

        connect(radioButton, &QRadioButton::clicked, this, [layer, layers](const bool checked) {
            if (!checked)
                return;

            for (auto pair : layers) {
                auto layer = pair.second;
                layer->hide();
            }
            layer->show();
        });

        if (name == "CUSTOM_OSM") {
            auto* widget = new QWidget();
            widget->setLayout(new QHBoxLayout);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            widget->layout()->setContentsMargins(QMargins{ 0, 0, 0, 0 });
#else
            widget->layout()->setMargin(0);
#endif

            widget->layout()->addWidget(radioButton);
            auto* urlEditor = new QLineEdit();
            urlEditor->setText(customURI);
            connect(urlEditor, &QLineEdit::textChanged, this, [layer](const QString& text) {
                dynamic_cast<QGVLayerOSM*>(layer)->setUrl(text);
            });
            widget->layout()->addWidget(urlEditor);

            groupBox->layout()->addWidget(widget);
        } else {
            groupBox->layout()->addWidget(radioButton);
        }
    }

    // Last map will be default
    radioButton->click();

    return groupBox;
}

void MainWindow::UpdateProgress()
{
    cache_status->setText(
        QString("load: %1, save: %2")
            .arg(load_task_queue_size.GetQueueSize())
            .arg(save_task_queue_size.GetQueueSize()));
}
