#pragma once

#include <QObject>
#include <QDebug> 
#include <common/plugins/interfaces/edit_plugin.h>
//#include <ui_sliceDialog.h>
#include "common/pluginCommonData.h"
#include <vcg/complex/algorithms/refine_loop.h>
#include <QtWidgets/qdockwidget.h>
#include <vcg/space/index/aabb_binary_tree/aabb_binary_tree.h>

typedef vcg::AABBBinaryTreeIndex<CFaceO, float, vcg::EmptyClass> AABBTree;
class SliceEditPlugin : public QObject, public EditTool
{
    Q_OBJECT

public:
    SliceEditPlugin() {};
    virtual ~SliceEditPlugin() {}

    static const QString info() {
        return tr("pick up three feature points and align the model according them.");
    };

    bool startEdit(MeshModel&/*m*/, GLArea* /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void endEdit(MeshModel&/*m*/, GLArea* /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void decorate(MeshModel&/*m*/, GLArea* /*parent*/, QPainter* p);
    void decorate(MeshModel&/*m*/, GLArea* gla) {};
    void mousePressEvent(QMouseEvent*, MeshModel&, GLArea*) {};
    void mouseMoveEvent(QMouseEvent*, MeshModel&, GLArea*) {};
    void mouseReleaseEvent(QMouseEvent* event, MeshModel&/*m*/, GLArea*) {};
    void keyPressEvent(QKeyEvent*, MeshModel&, GLArea*) {};
    void mouseDoubleClickEvent(QMouseEvent*, MeshModel&, GLArea*) {};

private:
    GLArea* gla         = nullptr;
    MeshDocument* md    = nullptr;
    MeshModel* originMesh       = nullptr;
    AABBTree* originMeshTree    = nullptr;
    double headHeight   = 0.0;// distance form level_0 to level_10
    bool isFirstClicked = true;

private:
    void fillHole(CMeshO* m, vcg::Point3f direction);
    //将模型沿plane分成上下两部分
    void divideMeshByPlane(CMeshO* source, CMeshO*& targetUp, CMeshO*& targetUnder, vcg::Plane3f slicingPlane, bool needUp = true, bool needUnder = true);

    //头颅数据测量
    void sliceModel(); //将摆正的头颅用level0~level0 11个平面切分成10部分
    void computeC(int level); // Circumference
    void computeCR(int level);
    void computeRSI_CVAI(int level);
    void computeASR_PSR();

signals:
    void suspendEditToggle();
};
