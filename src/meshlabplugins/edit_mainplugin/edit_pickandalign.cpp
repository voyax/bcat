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
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include "edit_pickandalign.h"
#include <wrap/qt/gl_label.h>


#include <wrap/qt/device_to_logical.h>
#include <wrap/gl/picking.h>
#include <vcg/complex/algorithms/create/platonic.h>
#include <QTextCodec>

using namespace std;
using namespace vcg;

void drawSphere(GLfloat xx, GLfloat yy, GLfloat zz, GLfloat radius)
{
	GLfloat M = 5;
	GLfloat N = 5;
	float step_z = FPI / M;
	float step_xy = 2 * FPI / N;
	float x[4], y[4], z[4];

	float angle_z = 0.0;
	float angle_xy = 0.0;
	int i = 0, j = 0;
	glBegin(GL_QUADS);
	for (i = 0; i < M; i++)
	{
		angle_z = i * step_z;

		for (j = 0; j < N; j++)
		{
			angle_xy = j * step_xy;

			x[0] = radius * sin(angle_z) * cos(angle_xy);
			y[0] = radius * sin(angle_z) * sin(angle_xy);
			z[0] = radius * cos(angle_z);

			x[1] = radius * sin(angle_z + step_z) * cos(angle_xy);
			y[1] = radius * sin(angle_z + step_z) * sin(angle_xy);
			z[1] = radius * cos(angle_z + step_z);

			x[2] = radius * sin(angle_z + step_z) * cos(angle_xy + step_xy);
			y[2] = radius * sin(angle_z + step_z) * sin(angle_xy + step_xy);
			z[2] = radius * cos(angle_z + step_z);

			x[3] = radius * sin(angle_z) * cos(angle_xy + step_xy);
			y[3] = radius * sin(angle_z) * sin(angle_xy + step_xy);
			z[3] = radius * cos(angle_z);

			for (int k = 0; k < 4; k++)
			{
				glVertex3f(xx + x[k], yy + y[k], zz + z[k]);
			}
		}
	}
	glEnd();
}

PickAndAlignEditPlugin::PickAndAlignEditPlugin() 
{
	mode = EMPTY;
	haveAdded	= true;
	havePicked	= true;
	leftDown = false;
	pIndex = -1;
	parameterWindow = NULL;
	transX  = transY  = transZ  = 0.0;
	rotateX = rotateY = rotateZ = 0.0;
	showCoordinates = false;
	planeLevel0 = nullptr;
	cont = nullptr;
}

const QString PickAndAlignEditPlugin::info() 
{
	return tr("pick up three feature points and align the model according them.");
}
 
void PickAndAlignEditPlugin::mouseReleaseEvent(QMouseEvent * event, MeshModel &/*m*/, GLArea * gla)
{
	leftDown	= false;
	havePicked  = true;
}
  
void PickAndAlignEditPlugin::decorate(MeshModel& m, GLArea* gla, QPainter* p)
{
	if (showCoordinates) {
		CoordinateFrame(m.cm.bbox.Diag() / 2.0).Render(gla, p);
	}

	drawPlaneMeshLevel0();

	switch (mode) {
		case EMPTY:      return;
		case PICK_ALIGN: drawAndChangePickPoints(p); return;
	};
}

void PickAndAlignEditPlugin::mousePressEvent(QMouseEvent* event, MeshModel& m, GLArea* gla)
{

	curPos = QTLogicalToOpenGL(gla, event->pos());
	havePicked = false;
	leftDown   = true;
}

void PickAndAlignEditPlugin::mouseMoveEvent(QMouseEvent*event, MeshModel&, GLArea*)
{
	curPos = QTLogicalToOpenGL(gla, event->pos());
}

void PickAndAlignEditPlugin::keyPressEvent(QKeyEvent *e, MeshModel &m, GLArea *gla)
{
}

void PickAndAlignEditPlugin::mouseDoubleClickEvent(QMouseEvent*, MeshModel&, GLArea*)
{
	std::cout << __FUNCTION__ << std::endl;
	haveAdded = false;
}

void PickAndAlignEditPlugin::drawAndChangePickPoints(QPainter* p)
{
	if (!gla->suspendedEditor) {
		if (leftDown) { //如果是在suspended状态则不能选点和移动
			Point3f point3;
			bool result = vcg::Pick<vcg::Point3f>(curPos.X(), curPos.Y(), point3);

			if (result) {
				if (!havePicked) {
					havePicked = true;
					pIndex = -1;

					double minDis = std::numeric_limits<double>::max();
					for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
						double dis = vcg::Distance(md->mm()->featurePoints[i], point3);
						if (minDis > dis && dis < 3.0) {
							minDis = dis;
							pIndex = i;
						}
					}
				}

				if (pIndex != -1) {
					md->mm()->featurePoints[pIndex] = point3;
				}
			}
		}

		if (!haveAdded) {
			haveAdded = true;
			Point3f point3;
			bool result = vcg::Pick<vcg::Point3f>(curPos.X(), curPos.Y(), point3);
			if (result && md->mm()->featurePoints.size() < 3) {
				std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
				md->mm()->featurePoints.push_back(point3);
			}
		}

		// 绘制提示信息
		QString title = QString::fromLocal8Bit("操作提示：");
		QString instruction;
		switch (md->mm()->featurePoints.size()) {
		case 0: instruction = QString::fromLocal8Bit("<p style='font-size:20px'> 请双击选择右侧耳屏点 </p>"); break;
		case 1: instruction = QString::fromLocal8Bit("<p style='font-size:20px'> 请双击选择鼻根点 </p>"); break;
		case 2: instruction = QString::fromLocal8Bit("<p style='font-size:20px'> 请双击选择左侧耳屏点 </p>"); break;
		case 3: instruction = QString::fromLocal8Bit("<p style='font-size:20px'> 单击拖动修改点的位置 </p>"); break;
		}
		this->realTimeLog(title, md->mm()->shortName(), "%s", instruction.toStdString().c_str());
	}

	//绘制三个特征点
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glColor4f(1, 0, 0, 0.3);
	for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
		drawSphere(md->mm()->featurePoints[i].X(), md->mm()->featurePoints[i].Y(), md->mm()->featurePoints[i].Z(), 3.0);
		QTextCodec* codec = QTextCodec::codecForName("utf-8");

		QString labelName;
		switch (i) {
		case 0: labelName = QString::fromLocal8Bit("右侧耳屏点"); break;
		case 1: labelName = QString::fromLocal8Bit("鼻根点"); break;
		case 2: labelName = QString::fromLocal8Bit("左侧耳屏点"); break;
		};
		vcg::glLabel::render(p, md->mm()->featurePoints[i], labelName);
	}
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);

	gla->update();
}

// deprecated...
void PickAndAlignEditPlugin::createPlaneMeshLevel0()
{
	if (planeLevel0) {
		MeshModel* m = md->addNewMesh("", "Level0 Plane");

		vcg::Point3f dirH, dirV;
		if (Point3f(1.0, 0.0, 0.0) * planeLevel0->Direction() == 1.0)
		{
			dirH = Point3f(0.0, 1.0, 0.0);
			dirV = Point3f(0.0, 0.0, 1.0);
		}
		else {
			dirH = Point3f(1.0, 0.0, 0.0) ^ planeLevel0->Direction();
			dirH.Normalize();
			dirV = dirH ^ planeLevel0->Direction();
			dirV.Normalize();
		}

		float dimH, dimV; //平面大小
		dimH = dimV = md->mm()->cm.bbox.Diag() / 2.0;

		int numV, numH;
		numV = numH = 3; //平面每行每列均有3个点

		vcg::Point3f centerP;
		for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
			centerP += md->mm()->featurePoints[i];
		}
		centerP /= md->mm()->featurePoints.size();

		// 构建模型
		for (int ir = 0; ir < numV; ir++)
			for (int ic = 0; ic < numH; ic++)
			{
				Point3m newP = (centerP + (dirV * -dimV) + (dirH * -dimH));
				newP = newP + (dirH * ic * (2.0 * dimH / (numH - 1))) + (dirV * ir * (2.0 * dimV / (numV - 1)));
				tri::Allocator<CMeshO>::AddVertex(m->cm, newP, planeLevel0->Direction());
			}

		FaceGrid(m->cm, numH, numV);
		m->setVisible(true);
	}
}

void PickAndAlignEditPlugin::drawPlaneMeshLevel0()
{
	if (planeLevel0 == NULL) {
		return;
	}

	glEnable(GL_BLEND); //Enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.

	glColor4f(0.3, 0.3, 0.3, 0.5);
	glBegin(GL_POLYGON);
	for (int i = 0; i < planePoints.size(); i++) {
		glVertex3f(planePoints[i].X(), planePoints[i].Y(), planePoints[i].Z());
	}
	glEnd();

	glDisable(GL_BLEND); 
}

void PickAndAlignEditPlugin::slotTransModel(double d)
{
	showCoordinates = true;

	QDoubleSpinBox* valueChangedBox = qobject_cast<QDoubleSpinBox*>(sender());
	QSlider* valueChangedSlider;
	vcg::Matrix44f mat;
	vcg::Point3f axis;
	double transDis;
	double* transValue;

	if (valueChangedBox) {
		if (valueChangedBox->objectName() == "doubleSpinBox_transX") {
			axis = vcg::Point3f(1.0, 0.0, 0.0);
			transDis = valueChangedBox->value() - transX;
			transValue = &transX;
			valueChangedSlider = parameterUI.horizontalSlider_transX;
		}
		else if (valueChangedBox->objectName() == "doubleSpinBox_transY") {
			axis = vcg::Point3f(0.0, 1.0, 0.0);
			transDis = valueChangedBox->value() - transY;
			transValue = &transY;
			valueChangedSlider = parameterUI.horizontalSlider_transY;
		}
		else if (valueChangedBox->objectName() == "doubleSpinBox_transZ") {
			axis = vcg::Point3f(0.0, 0.0, 1.0);
			transDis = valueChangedBox->value() - transZ;
			transValue = &transZ;
			valueChangedSlider = parameterUI.horizontalSlider_transZ;
		}
		else {
			return;
		}
	}

	if (fabs(transDis) > 0.05) {
		*transValue = valueChangedBox->value();

		int intValue = valueChangedBox->value() > 0 ? valueChangedBox->value() + 0.0001 : valueChangedBox->value() - 0.0001;
		valueChangedSlider->setValue(intValue); // 在这里加上0.0001 防止double到int转换时由于精度损失出现问题

		mat.SetTranslate(axis * transDis);
		md->mm()->cm.Tr = mat * md->mm()->cm.Tr;

		// 更新特征点
		for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
			md->mm()->featurePoints[i] = mat * md->mm()->featurePoints[i];
		}

		gla->update();
	}
}

void PickAndAlignEditPlugin::slotRotateModel(double g)
{
	showCoordinates = true;

	QDoubleSpinBox* valueChangedBox = qobject_cast<QDoubleSpinBox*>(sender());
	QSlider* valueChangedSlider = NULL;
	vcg::Matrix44f mat;
	vcg::Point3f axis;
	double roateDegree;
	double* rotateValue;

	if (valueChangedBox) {
		if (valueChangedBox->objectName() == "doubleSpinBox_rotateX") {
			axis = vcg::Point3f(1.0, 0.0, 0.0);
			roateDegree = valueChangedBox->value() - rotateX;
			rotateValue = &rotateX;
			valueChangedSlider = parameterUI.horizontalSlider_rotateX;
		}
		else if (valueChangedBox->objectName() == "doubleSpinBox_rotateY") {
			axis = vcg::Point3f(0.0, 1.0, 0.0);
			roateDegree = valueChangedBox->value() - rotateY;
			rotateValue = &rotateY;
			valueChangedSlider = parameterUI.horizontalSlider_rotateY;
		}
		else if (valueChangedBox->objectName() == "doubleSpinBox_rotateZ") {
			axis = vcg::Point3f(0.0, 0.0, 1.0);
			roateDegree = valueChangedBox->value() - rotateZ;
			rotateValue = &rotateZ;
			valueChangedSlider = parameterUI.horizontalSlider_rotateZ;
		}
		else {
			return;
		}
	}

	if (fabs(roateDegree) > 0.05) { // 防止反复更新模型，只有在手动操作spinbox时 才去更新模型以及slider的位置
		*rotateValue = valueChangedBox->value();

		int intValue = valueChangedBox->value() > 0.0 ? valueChangedBox->value() + 0.0001 : valueChangedBox->value() - 0.0001;
		valueChangedSlider->setValue(intValue);

		mat.SetRotateDeg(roateDegree, axis);
		md->mm()->cm.Tr = mat * md->mm()->cm.Tr;

		// 更新特征点
		for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
			md->mm()->featurePoints[i] = mat * md->mm()->featurePoints[i];
		}

		gla->update();
	}
}

// 拖动条相应函数 Qt中拖动条只能拖动整数
void PickAndAlignEditPlugin::slotTransModel(int i)
{
	showCoordinates = true;

	QSlider* valueChangedSlider = qobject_cast<QSlider*>(sender());
	QDoubleSpinBox* valueChangedBox;

	vcg::Matrix44f mat;
	vcg::Point3f axis;
	double transDis;
	double* transValue;

	if (valueChangedSlider) {
		if (valueChangedSlider->objectName() == "horizontalSlider_transX") {
			axis = vcg::Point3f(1.0, 0.0, 0.0);
			transDis = valueChangedSlider->value() - transX;
			transValue = &transX;
			valueChangedBox = parameterUI.doubleSpinBox_transX;
		}
		else if (valueChangedSlider->objectName() == "horizontalSlider_transY") {
			axis = vcg::Point3f(0.0, 1.0, 0.0);
			transDis = valueChangedSlider->value() - transY;
			transValue = &transY;
			valueChangedBox = parameterUI.doubleSpinBox_transY;
		}
		else if (valueChangedSlider->objectName() == "horizontalSlider_transZ") {
			axis = vcg::Point3f(0.0, 0.0, 1.0);
			transDis = valueChangedSlider->value() - transZ;
			transValue = &transZ;
			valueChangedBox = parameterUI.doubleSpinBox_transZ;
		}
		else {
			return;
		}
	}

	// 如果不是主动调节UI造成的value改变 不对数据做任何操作
	if (fabs(transDis) > 0.95) {
		*transValue = valueChangedSlider->value();

		// 移动模型
		mat.SetTranslate(axis * transDis);
		md->mm()->cm.Tr = mat * md->mm()->cm.Tr;

		// 更新特征点
		for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
			md->mm()->featurePoints[i] = mat * md->mm()->featurePoints[i];
		}
		gla->update();

		valueChangedBox->setValue(valueChangedSlider->value());
	}
}

void PickAndAlignEditPlugin::slotRotateModel(int g)
{
	showCoordinates = true;

	QSlider* valueChangedSlider = qobject_cast<QSlider*>(sender());
	QDoubleSpinBox* valueChangedBox = NULL;
	vcg::Matrix44f mat;
	vcg::Point3f axis;
	double roateDegree;
	double* rotateValue;

	if (valueChangedSlider) {
		if (valueChangedSlider->objectName() == "horizontalSlider_rotateX") {
			axis = vcg::Point3f(1.0, 0.0, 0.0);
			roateDegree = valueChangedSlider->value() - rotateX;
			rotateValue = &rotateX;
			valueChangedBox = parameterUI.doubleSpinBox_rotateX;
		}
		else if (valueChangedSlider->objectName() == "horizontalSlider_rotateY") {
			axis = vcg::Point3f(0.0, 1.0, 0.0);
			roateDegree = valueChangedSlider->value() - rotateY;
			rotateValue = &rotateY;
			valueChangedBox = parameterUI.doubleSpinBox_rotateY;
		}
		else if (valueChangedSlider->objectName() == "horizontalSlider_rotateZ") {
			axis = vcg::Point3f(0.0, 0.0, 1.0);
			roateDegree = valueChangedSlider->value() - rotateZ;
			rotateValue = &rotateZ;
			valueChangedBox = parameterUI.doubleSpinBox_rotateZ;
		}
		else {
			return;
		}
	}

	if (fabs(roateDegree) > 0.95) {
		*rotateValue = valueChangedSlider->value();
		valueChangedBox->setValue(valueChangedSlider->value());

		//更新模型位置
		mat.SetRotateDeg(roateDegree, axis);
		md->mm()->cm.Tr = mat * md->mm()->cm.Tr;

		// 更新特征点
		for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
			md->mm()->featurePoints[i] = mat * md->mm()->featurePoints[i];
		}

		gla->update();
	}
}

bool PickAndAlignEditPlugin::startEdit(MeshModel &m, GLArea * gla_, MLSceneGLSharedDataContext* cont)
{
	gla = gla_; //仅仅为了减少传参
	gla->setCursor(QCursor(QPixmap(":/images/cur_info.png"),1,1));

	md = &cont->meshDoc();
	mode = PICK_ALIGN;

	this->cont = cont;

	if (parameterWindow == NULL) {
		parameterWindow = new QDialog();
		parameterUI.setupUi(parameterWindow);

		connect(parameterUI.pushButton_alignModel, SIGNAL(clicked()), this, SLOT(slotAlignModel()));

		connect(parameterUI.doubleSpinBox_transX, SIGNAL(valueChanged(double)), this, SLOT(slotTransModel(double)));
		connect(parameterUI.doubleSpinBox_transY, SIGNAL(valueChanged(double)), this, SLOT(slotTransModel(double)));
		connect(parameterUI.doubleSpinBox_transZ, SIGNAL(valueChanged(double)), this, SLOT(slotTransModel(double)));

		connect(parameterUI.doubleSpinBox_rotateX, SIGNAL(valueChanged(double)), this, SLOT(slotRotateModel(double)));
		connect(parameterUI.doubleSpinBox_rotateY, SIGNAL(valueChanged(double)), this, SLOT(slotRotateModel(double)));
		connect(parameterUI.doubleSpinBox_rotateZ, SIGNAL(valueChanged(double)), this, SLOT(slotRotateModel(double)));
		//connect(parameterUI.pushButton_confirm, SIGNAL(clicked()), this, SLOT(slotConfirm()));

		connect(parameterUI.horizontalSlider_transX, SIGNAL(valueChanged(int)), this, SLOT(slotTransModel(int)));
		connect(parameterUI.horizontalSlider_transY, SIGNAL(valueChanged(int)), this, SLOT(slotTransModel(int)));
		connect(parameterUI.horizontalSlider_transZ, SIGNAL(valueChanged(int)), this, SLOT(slotTransModel(int)));

		connect(parameterUI.horizontalSlider_rotateX, SIGNAL(valueChanged(int)), this, SLOT(slotRotateModel(int)));
		connect(parameterUI.horizontalSlider_rotateY, SIGNAL(valueChanged(int)), this, SLOT(slotRotateModel(int)));
		connect(parameterUI.horizontalSlider_rotateZ, SIGNAL(valueChanged(int)), this, SLOT(slotRotateModel(int)));

		connect(this, SIGNAL(suspendEditToggle()), gla, SLOT(suspendEditToggle()));
		connect(this, SIGNAL(resetTrackBall()), gla, SLOT(resetTrackBall()));
	}
	parameterWindow->setWindowFlags(Qt::WindowStaysOnTopHint);
	parameterWindow->show();

	return true;
}

void PickAndAlignEditPlugin::endEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/)
{
	haveAdded		= true;
	showCoordinates = false;
	parameterWindow->hide();
}

void PickAndAlignEditPlugin::slotAlignModel() {
	if (md->mm()->featurePoints.size() < 3) {
		return;
	}

	//先将feature点恢复到原始位置
	vcg::Matrix44f inverseTr = vcg::Inverse(md->mm()->cm.Tr);
	for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
		md->mm()->featurePoints[i] = inverseTr * md->mm()->featurePoints[i];
	}

	//将微调界面恢复
	parameterUI.doubleSpinBox_transX->setValue(0.0);
	parameterUI.doubleSpinBox_transY->setValue(0.0);
	parameterUI.doubleSpinBox_transZ->setValue(0.0);
	parameterUI.doubleSpinBox_rotateX->setValue(0.0);
	parameterUI.doubleSpinBox_rotateY->setValue(0.0);
	parameterUI.doubleSpinBox_rotateZ->setValue(0.0);

	//再做变换
	vcg::Point3f R = md->mm()->featurePoints[0]; // 右侧耳点
	vcg::Point3f N = md->mm()->featurePoints[1]; // 鼻根点
	vcg::Point3f L = md->mm()->featurePoints[2]; // 左侧耳点

	//vcg::Point3f x = (R - L).Normalize();
	vcg::Point3f z = (L - N) ^ (R - N);
	z = z.Normalize();
	vcg::Point3f M = (R + L) * 0.5; // 按M点摆正则Y轴穿过左右耳屏点的中点
	vcg::Point3f y = (N - M).Normalize();
	vcg::Point3f x = y ^ z;

	vcg::Matrix44f transFormMatrix; // 变换矩阵 由平移和旋转矩阵组成

	vcg::Matrix44f transM; //平移矩阵
	transM.SetTranslate(-M);

	vcg::Matrix44f rotateM;
	rotateM.SetIdentity();
	rotateM[0][0] = x[0];
	rotateM[1][0] = x[1];
	rotateM[2][0] = x[2];

	rotateM[0][1] = y[0];
	rotateM[1][1] = y[1];
	rotateM[2][1] = y[2];

	rotateM[0][2] = z[0];
	rotateM[1][2] = z[1];
	rotateM[2][2] = z[2];

	rotateM = vcg::Inverse(rotateM);	
	transFormMatrix = rotateM * transM;
	md->mm()->cm.Tr = transFormMatrix;

	//更新模型点的位置
	//tri::UpdatePosition<CMeshO>::Matrix(originMesh->cm, transFormMatrix, true);

	// 更新特征点
	for (int i = 0; i < md->mm()->featurePoints.size(); ++i) {
		md->mm()->featurePoints[i] = transFormMatrix * md->mm()->featurePoints[i];
	}

	// 自动显示局部坐标轴
	showCoordinates = true;

	//构建平面
	if (planeLevel0) {
		delete planeLevel0;
	}
	planeLevel0 = new Plane3f();
	planeLevel0->Init(md->mm()->featurePoints[0], md->mm()->featurePoints[1], md->mm()->featurePoints[2]);

	// 计算平面上的点 圆形平面
	//vcg::Point3f centerP = (featurePoints[0] + featurePoints[2]) * 0.5;
	//float diameter = originMesh->cm.bbox.Diag();
	//vcg::Point3f dir = (featurePoints[0] - featurePoints[2]).Normalize();
	//vcg::Matrix44f mat;
	//mat.SetIdentity();
	//for (int i = 20; i >= 1; i--) {
	//	vcg::Point3f p = centerP + dir * diameter * 0.5;
	//	planePoints.push_back(p);
	//	mat.SetRotateDeg(360.0 / 20, planeLevel0->Direction());
	//	dir = mat * dir;
	//}

	// 计算平面上的点
	planePoints.clear();
	vcg::Point3f centerP = (md->mm()->featurePoints[0] + md->mm()->featurePoints[2]) * 0.5;
	float distance = md->mm()->cm.bbox.Diag() * 0.5;
	vcg::Point3f dirNW, dirNE, dirSW, dirSE;
	dirNW = vcg::Point3f(1.0,-1.0,0.0).Normalize();
	dirNE = vcg::Point3f(-1.0,-1.0,0.0).Normalize();
	dirSW = vcg::Point3f(1.0,1.0,0.0).Normalize();
	dirSE = vcg::Point3f(-1.0,1.0,0.0).Normalize();

	planePoints.push_back(centerP + dirNW * distance);
	planePoints.push_back(centerP + dirNE * distance);
	planePoints.push_back(centerP + dirSE * distance);
	planePoints.push_back(centerP + dirSW * distance);

	emit resetTrackBall();
	gla->update();
}
