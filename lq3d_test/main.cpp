/*
    lq3D_test   : interfacing Qt and Graphviz library

    Author        : Carlo Capelli
    E-mail        : cc.carlo.cap@gmail.com
    Copyright (C): 2013,2014,2015,2016

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <QApplication>
#include "mainwindow.h"
#include "lq3d.h"
#include "lq3dView.h"
#include "lq3d_configure.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    lq3dView view;
    auto rootEntity = view.scene->rootEntity;

    Qt3DRender::QCylinderMesh *cylinder = new Qt3DRender::QCylinderMesh();
    cylinder->setRadius(1);
    cylinder->setLength(3);
    cylinder->setRings(100);
    cylinder->setSlices(20);

    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform;
    cylinderTransform->setScale(1.5f);
    cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 45.0f));

    // Cylinder
    Qt3DCore::QEntity *cylinderEntity = new Qt3DCore::QEntity(rootEntity);
    cylinderEntity->addComponent(cylinder);
    cylinderEntity->addComponent(cylinderTransform);

    view.scene->cg->engine.setRootEntity(rootEntity);

    // Show window
    view.show();
    return app.exec();
}
