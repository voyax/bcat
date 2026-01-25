/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#ifndef SAMPLEEDITPLUGIN_H
#define SAMPLEEDITPLUGIN_H

#include <QObject>
#include <QDebug> 
#include <common/plugins/interfaces/edit_plugin.h>
#include <wrap/gui/coordinateframe.h>
#include "ui_pickandalign.h"
#include "common/pluginCommonData.h"

enum PluginMode { EMPTY, PICK_ALIGN };
#define FPI 3.14159265354

class PickAndAlignEditPlugin : public QObject, public EditTool
{
	Q_OBJECT

public:
    PickAndAlignEditPlugin();
    virtual ~PickAndAlignEditPlugin() {}

    static const QString info();

    bool startEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void endEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void decorate(MeshModel&/*m*/, GLArea* /*parent*/, QPainter* p);
    void decorate(MeshModel&/*m*/, GLArea* gla) {};
    void mousePressEvent(QMouseEvent *, MeshModel &, GLArea *);
    void mouseMoveEvent(QMouseEvent*, MeshModel&, GLArea*);
    void mouseReleaseEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea * );
	void keyPressEvent(QKeyEvent *, MeshModel &, GLArea *);
    void mouseDoubleClickEvent(QMouseEvent*, MeshModel&, GLArea*);

    void drawAndChangePickPoints(QPainter* p);
    void createPlaneMeshLevel0();
    void drawPlaneMeshLevel0();

public slots:
    void slotAlignModel();
    void slotTransModel(double d);
    void slotRotateModel(double g);
    void slotTransModel(int d);
    void slotRotateModel(int g);

private:
    PluginMode mode;
    vcg::Point2f curPos; //当前鼠标在屏幕坐标

    bool showCoordinates;
    bool haveAdded;     //是否已经增完特征点
    bool havePicked;    //是否完成了pick动作
    bool leftDown;
    // std::vector<vcg::Point3f> featurePoints;//0 右侧耳屏点 1 鼻根点  2 左侧耳屏点 
    int pIndex; // 当前选中的特征点
    vcg::Plane3f* planeLevel0;
    std::vector<vcg::Point3f> planePoints;

    GLArea* gla; // 当前插件对应的三维画布（为了方便函数嵌套）。 注：这里虽然有gla指针但是并不是所有的GLArea类里面的方法都可以被调用
    MeshDocument* md;
    MLSceneGLSharedDataContext* cont;
    double transX; // 相对于原始位置的移动量
    double transY;
    double transZ;
    double rotateX;
    double rotateY;
    double rotateZ;

    QDialog* parameterWindow;
    Ui::DialogPickAndAlign parameterUI;
signals:
	void suspendEditToggle();
    void resetTrackBall();

};

#endif
