#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include "edit_slice.h"
#include <vcg/complex/algorithms/intersection.h>
#include <vcg/complex/algorithms/inertia.h>
#include <wrap/io_trimesh/export.h>
#include <QProgressDialog>

using namespace std;
using namespace vcg;

bool SliceEditPlugin::startEdit(MeshModel& m, GLArea* gla_, MLSceneGLSharedDataContext* cont)
{
	if (isFirstClicked) {
		gla = gla_; //仅仅为了减少传参
		md = &cont->meshDoc();

		QProgressDialog* computeProgressDialog = new QProgressDialog();
		computeProgressDialog->setLabelText("Computing...");
		computeProgressDialog->setMinimum(0);
		computeProgressDialog->setMaximum(100);
		computeProgressDialog->setWindowModality(Qt::WindowModal);

		computeProgressDialog->setValue(0);
		int totalCount = md->meshNumber() * 32;
		int meshDealing = 0;

		originMeshTree = new AABBTree();
		for (MeshModel& currentMesh: md->meshIterator()) {
			originMesh = &currentMesh;
			tri::UpdatePosition<CMeshO>::Matrix(originMesh->cm, originMesh->cm.Tr, false);

			sliceModel();
			computeProgressDialog->setValue((meshDealing * 32.0 + 1.0) / totalCount * 100);

			computeASR_PSR();
			computeProgressDialog->setValue((meshDealing * 32.0 + 2.0) / totalCount * 100);

			originMesh->measurement.head_height = headHeight;

			//索引树的初始化必须放在computeASR_PSR函数之后，因为该函数会改变mesh的结构
			originMeshTree->Clear();
			originMeshTree->Set(originMesh->cm.face.begin(), originMesh->cm.face.end());
			for (int level = 0; level < 10; ++level) {
				qDebug() << "Level " << level;
				computeC(level);
				computeProgressDialog->setValue((meshDealing * 32.0 + level * 3 + 3.0) / totalCount * 100);

				computeCR(level);
				computeProgressDialog->setValue((meshDealing * 32.0 + level * 3 + 4.0) / totalCount * 100);

				computeRSI_CVAI(level);
				computeProgressDialog->setValue((meshDealing * 32.0 + level * 3 + 5.0) / totalCount * 100);
			}

			tri::UpdatePosition<CMeshO>::Matrix(originMesh->cm, Inverse(originMesh->cm.Tr), false);
			meshDealing++;
		}
		computeProgressDialog->setValue(100);

		delete originMeshTree;
		originMeshTree = nullptr;
		isFirstClicked = false;
	}

	md->emitAddNewDockWidget(true);//更新主界面
	return true;
}

void SliceEditPlugin::endEdit(MeshModel&, GLArea*, MLSceneGLSharedDataContext*)
{
	md->emitAddNewDockWidget(false);
}

// Decorate the 3D mesh in main window.
void SliceEditPlugin::decorate(MeshModel&, GLArea*, QPainter* p)
{
	std::vector<std::vector<vcg::Point3f>>& pointsFromLevel0to10 = *(md->mm()->pointsFromLevel0to10);
	if (md->mm()->pointsFromLevel0to10 == nullptr) {
		return;
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1, 0, 0, 0);
	glBegin(GL_POINTS);
	for (int j = 0; j < pointsFromLevel0to10[0].size(); ++j) {
		glVertex3f(pointsFromLevel0to10[0][j].X(), pointsFromLevel0to10[0][j].Y(), pointsFromLevel0to10[0][j].Z());
	}
	for (int j = 0; j < pointsFromLevel0to10[3].size(); ++j) {
		glVertex3f(pointsFromLevel0to10[3][j].X(), pointsFromLevel0to10[3][j].Y(), pointsFromLevel0to10[3][j].Z());
	}
	glEnd();

	glColor4f(0, 1, 0, 0);
	glBegin(GL_POINTS);
	for (int j = 0; j < pointsFromLevel0to10[2].size(); ++j) {
		glVertex3f(pointsFromLevel0to10[2][j].X(), pointsFromLevel0to10[2][j].Y(), pointsFromLevel0to10[2][j].Z());
	}
	for (int j = 0; j < pointsFromLevel0to10[8].size(); ++j) {
		glVertex3f(pointsFromLevel0to10[8][j].X(), pointsFromLevel0to10[8][j].Y(), pointsFromLevel0to10[8][j].Z());
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
}

void SliceEditPlugin::computeC(int level) {
	std::cout << __FUNCTION__ << std::endl;

	std::vector<std::vector<vcg::Point3f>>& pointsFromLevel0to10 = *(originMesh->pointsFromLevel0to10);
	double c = 0.0; // 头围circumstance
	int circle_size = pointsFromLevel0to10[level].size();
	for (int i = 1; i < circle_size; i++) {
		c += vcg::Distance(pointsFromLevel0to10[level][i - 1], pointsFromLevel0to10[level][i]);
	}
	c += vcg::Distance(pointsFromLevel0to10[level][circle_size - 1], pointsFromLevel0to10[level][0]);
	originMesh->measurement.c[level] = c;
}

void SliceEditPlugin::computeCR(int level)
{
	//sliceUI.pushButton_computeCR->setEnabled(false);
	std::cout << __FUNCTION__ << std::endl;

	//求交的办法 计算W和L
	float maxDist = std::numeric_limits<float>::max();
	vcg::Point3f offsetCenter(0.0, 0.0, headHeight / 10.0 * level);

	vcg::Point3f oy, _oy, ox, _ox;
	vcg::Ray3<float, false> rayOY(offsetCenter, vcg::Point3f(0.0,   1.0, 0.0));
	vcg::Ray3<float, false> ray_OY(offsetCenter, vcg::Point3f(0.0, -1.0, 0.0));
	vcg::Ray3<float, false> rayOX(offsetCenter, vcg::Point3f(1.0, 0.0, 0.0));
	vcg::Ray3<float, false> ray_OX(offsetCenter, vcg::Point3f(-1.0, 0.0, 0.0));

	float rayTOY, rayT_OY, rayTOX, rayT_OX;
	rayTOX = rayT_OX = rayT_OY = rayTOY = std::numeric_limits<float>::max();
	CFaceO* isectFaceOY, * isectFace_OY, * isectFaceOX, * isectFace_OX;
	isectFaceOY = isectFace_OY = isectFaceOX = isectFace_OX = nullptr;

	vcg::EmptyClass objectMarker;
	vcg::RayTriangleIntersectionFunctor<true> rayIntersector;
	isectFaceOX		= originMeshTree->DoRay(rayIntersector, objectMarker, rayOX, maxDist, rayTOX);
	isectFace_OX	= originMeshTree->DoRay(rayIntersector, objectMarker, ray_OX, maxDist, rayT_OX);
	isectFaceOY		= originMeshTree->DoRay(rayIntersector, objectMarker, rayOY, maxDist, rayTOY);
	isectFace_OY	= originMeshTree->DoRay(rayIntersector, objectMarker, ray_OY, maxDist, rayT_OY);

	if (isectFaceOX && isectFace_OX && isectFaceOY && isectFace_OY) {
		oy	= offsetCenter + vcg::Point3f(0.0, 0.0, 1.0)	* rayTOY;
		_oy = offsetCenter + vcg::Point3f(0.0, 0.0, -1.0)	* rayT_OY;
		ox	= offsetCenter + vcg::Point3f(1.0, 0.0, 0.0)	* rayTOX;
		_ox = offsetCenter + vcg::Point3f(-1.0, 0.0, 0.0)	* rayT_OX;
	}

	std::cout << "Length: " << vcg::Distance(oy, _oy) << "  Width: " << vcg::Distance(ox, _ox) << std::endl;
	float cr = vcg::Distance(ox, _ox) / vcg::Distance(oy, _oy);

	originMesh->measurement.cr_L[level] = vcg::Distance(oy, _oy);
	originMesh->measurement.cr_W[level] = vcg::Distance(ox, _ox);
	originMesh->measurement.cr[level]	= cr;
}

void SliceEditPlugin::computeRSI_CVAI(int level)
{
	std::cout << __FUNCTION__ << std::endl;

	float maxDist = std::numeric_limits<float>::max();
	vcg::EmptyClass objectMarker;
	vcg::RayTriangleIntersectionFunctor<true> rayIntersector;

	vcg::Point3f offsetCenter(0.0, 0.0, headHeight / 10.0 * level + 0.1);//Z方向加个0.1保证求交的时候不会与mesh的点相交
	vcg::Matrix44f matPositive, matNegtive;
	matPositive.SetIdentity();
	matNegtive.SetIdentity();
	vcg::Point3f dirY = vcg::Point3f(0.0, 1.0, 0.0);
	double D1, D2; // D1: 30  -150，x轴朝左，脸朝下/前，y轴朝下
	D1 = D2 = 0.0; // D2: -30 +150，x轴朝左，脸朝下/前，y轴朝下

	double totalDiff = 0.0;
	for (int i = 1; i <= 11; ++i) {
		matPositive.SetRotateDeg(i * 15, vcg::Point3f(0.0, 0.0, 1.0));
		matNegtive.SetRotateDeg(-i * 15, vcg::Point3f(0.0, 0.0, 1.0));
		vcg::Point3f dirPositive = matPositive * dirY;
		vcg::Point3f dirNegtive  = matNegtive  * dirY;
		vcg::Ray3<float, false> rayPositive(offsetCenter, dirPositive);
		vcg::Ray3<float, false> rayNegtive(offsetCenter, dirNegtive);

		float rayTpositive, rayTnegtive;
		rayTpositive = rayTnegtive = std::numeric_limits<float>::max();

		CFaceO* isectFacePos, * isectFaceNeg;
		isectFaceNeg = isectFacePos = nullptr;
		isectFacePos = originMeshTree->DoRay(rayIntersector, objectMarker, rayPositive, maxDist, rayTpositive);
		isectFaceNeg = originMeshTree->DoRay(rayIntersector, objectMarker, rayNegtive, maxDist, rayTnegtive);

		if (isectFaceNeg && isectFacePos) {
			totalDiff += fabs(rayTpositive - rayTnegtive);
			//std::cout << fabs(rayTpositive - rayTnegtive) << std::endl;
		}
		else {
			std::cout << "missing project at degree" << i * 15 << std::endl;
		}

		if (i == 2) // 30 for D1 and -30 for D2
		{
			D1 += rayTpositive;
			D2 += rayTnegtive;
			vcg::Point3f pointPos = offsetCenter + dirPositive * rayTpositive;
			vcg::Point3f pointNeg = offsetCenter + dirNegtive * rayTnegtive;
			originMesh->measurement.cvai_points[level].push_back(pointPos); //point at 30 degree with y axis
			originMesh->measurement.cvai_points[level].push_back(pointNeg); // -30
		}
		if (i == 10) { // 150 for D2, -150 for D1
			D1 += rayTnegtive;
			D2 += rayTpositive;
			vcg::Point3f pointPos = offsetCenter + dirPositive * rayTpositive;
			vcg::Point3f pointNeg = offsetCenter + dirNegtive * rayTnegtive;
			originMesh->measurement.cvai_points[level].push_back(pointPos); // 150
			originMesh->measurement.cvai_points[level].push_back(pointNeg); // -150
		}
	}
	std::cout << "Total diffference is " << totalDiff << std::endl;
	originMesh->measurement.rsi[level] = totalDiff;

	// 当在画图时，x轴朝右，y轴朝下，D1和D2会被镜像，所以D1和D2需要互换
	// 注，从Z轴的正方向望向Z轴的负方向， 根据右手定则，D1是 Z轴和Y轴正方向成30度和-150交点形成的线段
	originMesh->measurement.cvai_d1[level] = D1;
	originMesh->measurement.cvai_d2[level] = D2;
	double cvai = fabs(D1 - D2) / std::max(D1, D2);
	originMesh->measurement.cvai[level] = cvai;

	std::cout << "CVAI " << cvai << std::endl;
}

void SliceEditPlugin::computeASR_PSR()
{
	std::cout << __FUNCTION__ << std::endl;

	CMeshO* upLeft, * upRight;
	CMeshO* upMesh, * underMesh;
	CMeshO* Q1, * Q2, * Q3, * Q4; // 脸朝y轴正方向，头顶朝z轴正方向，Q1是-XOY, Q2是XOY, Q3是XO-Y, Q4=-XO-Y

	upLeft = upRight   = nullptr;
	upMesh = underMesh = nullptr;
	Q1 = Q2 = Q3 = Q4 = nullptr;

	// 如果headLength还没有计算，先计算headLength（参考平面到颅顶的距离）
	if (headHeight == 0.0) {
		// 找到沿Z方向的距离差值
		double maxZ = std::numeric_limits<double>::min();
		for (CMeshO::VertexIterator vit = originMesh->cm.vert.begin(); vit != originMesh->cm.vert.end(); vit++) {
			if (vit->P().Z() > maxZ) {
				maxZ = vit->P().Z();
			}
		}
		headHeight = fabs(maxZ);
	}

	typedef CMeshO::ScalarType S;
	vcg::Point3<S> center(0.0, 0.0, 0.0);
	vcg::Plane3<S> plane2, plane8, YOZ, XOZ;

	plane2.Init(vcg::Point3<S>(0.0, 0.0, headHeight / 10.0 * 2), vcg::Point3<S>(0.0, 0.0, 1.0));
	plane8.Init(vcg::Point3<S>(0.0, 0.0, headHeight / 10.0 * 8), vcg::Point3<S>(0.0, 0.0, 1.0));
	YOZ.Init(center, vcg::Point3<S>(1.0, 0.0, 0.0));
	XOZ.Init(center, vcg::Point3<S>(0.0, 1.0, 0.0));

	divideMeshByPlane(&originMesh->cm, upMesh, underMesh, plane2, true, false);
	divideMeshByPlane(upMesh, upMesh, underMesh, plane8, false, true);
	divideMeshByPlane(underMesh, upRight, upLeft, YOZ); // 分成左右两部分
	divideMeshByPlane(upLeft, Q1, Q4, XOZ); // 分成前后两部分，Q1=左前
	divideMeshByPlane(upRight, Q2, Q3, XOZ);

	tri::Inertia<CMeshO> I_Q1(*Q1);
	tri::Inertia<CMeshO> I_Q2(*Q2);
	tri::Inertia<CMeshO> I_Q3(*Q3);
	tri::Inertia<CMeshO> I_Q4(*Q4);

	// volume. 单位cc，立方厘米
	Scalarm volume_Q1 = I_Q1.Mass() / 1000;
	Scalarm volume_Q2 = I_Q2.Mass() / 1000;
	Scalarm volume_Q3 = I_Q3.Mass() / 1000;
	Scalarm volume_Q4 = I_Q4.Mass() / 1000;

	std::cout <<"volume from Q1 to Q4 "<< volume_Q1 << " " << volume_Q2 << " " << volume_Q3 << " " << volume_Q4 << std::endl;

	// 输出Q1~Q4模型用于检测
	//int result1 = vcg::tri::io::ExporterSTL<CMeshO>::Save(*Q1, "D://Q1.stl", false);
	//int result2 = vcg::tri::io::ExporterSTL<CMeshO>::Save(*Q2, "D://Q2.stl", false);
	//int result3 = vcg::tri::io::ExporterSTL<CMeshO>::Save(*Q3, "D://Q3.stl", false);
	//int result4 = vcg::tri::io::ExporterSTL<CMeshO>::Save(*Q4, "D://Q4.stl", false);

	double asr = volume_Q1 > volume_Q2 ? volume_Q2 / volume_Q1 : volume_Q1 / volume_Q2;
	double psr = volume_Q3 > volume_Q4 ? volume_Q4 / volume_Q3 : volume_Q3 / volume_Q4;

	originMesh->measurement.volume_q1 = volume_Q1;
	originMesh->measurement.volume_q2 = volume_Q2;
	originMesh->measurement.volume_q3 = volume_Q3;
	originMesh->measurement.volume_q4 = volume_Q4;
	originMesh->measurement.asr = asr;
	originMesh->measurement.psr = psr;
	originMesh->measurement.overall_symmetry_ratio = asr / psr;
	originMesh->measurement.ap_volume_ratio = (volume_Q1 + volume_Q2) / (volume_Q3 + volume_Q4);

	delete upMesh;
	delete underMesh;
	delete upRight;
	delete upLeft;
	delete Q1;
	delete Q2;
	delete Q3;
	delete Q4;
}

// 修补边界在同一个平面上的模型
void SliceEditPlugin::fillHole(CMeshO* currentMesh, vcg::Point3f direction)
{
	vcg::tri::UpdateFlags<CMeshO>::FaceBorderFromNone(*currentMesh);
	std::vector<int> borderVItes;// 边界点的索引
	vcg::Point3f centerP(0.0, 0.0, 0.0);
	for (CMeshO::FaceIterator fi = currentMesh->face.begin(); fi != currentMesh->face.end(); ++fi)
	{
		if (!(*fi).IsD())
		{
			bool isB = false;
			for (int i = 0; i < 3; ++i)
			{
				if ((*fi).IsB(i)) {
					borderVItes.push_back(vcg::tri::Index(*currentMesh, fi->V0(i)));
					borderVItes.push_back(vcg::tri::Index(*currentMesh, fi->V1(i)));
					centerP += fi->V0(i)->P();
					centerP += fi->V1(i)->P();
				}
			}
		}
	}

	int addedFaceNumber = borderVItes.size() / 2.0;
	centerP /= borderVItes.size();

	CMeshO::VertexIterator vit = tri::Allocator<CMeshO>::AddVertices(*currentMesh, 1);
	vit->P() = centerP;

	CMeshO::FaceIterator fit = tri::Allocator<CMeshO>::AddFaces(*currentMesh, addedFaceNumber);
	for (int i = 0; i < borderVItes.size() - 1; i += 2) {
		fit->V(0) = &*vit;
		fit->V(1) = &currentMesh->vert.at(borderVItes[i]);
		fit->V(2) = &currentMesh->vert.at(borderVItes[i + 1]);

		vcg::Point3f v0v1 = fit->V(1)->P() - fit->V(0)->P();
		vcg::Point3f v1v2 = fit->V(2)->P() - fit->V(1)->P();
		if ((v0v1 ^ v1v2) * direction < 0) {
			fit->V(2) = &currentMesh->vert.at(borderVItes[i]);
			fit->V(1) = &currentMesh->vert.at(borderVItes[i + 1]);
		}

		fit++;
	}

	//update normal
	tri::UpdateNormal<CMeshO>::PerFaceNormalized(*currentMesh);
	tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(*currentMesh);
}

void SliceEditPlugin::divideMeshByPlane(CMeshO* source, CMeshO*& targetUp, CMeshO*& targetUnder, vcg::Plane3<CMeshO::ScalarType> slicingPlane, bool needUp, bool needUnder)
{
	tri::QualityMidPointFunctor<CMeshO> slicingfunc(0.0);
	tri::QualityEdgePredicate<CMeshO> slicingpred(0.0, 0.0);

	tri::UpdateSelection<CMeshO>::Clear(*source);
	source->face.EnableFFAdjacency();
	tri::UpdateTopology<CMeshO>::FaceFace(*source);
	source->vert.EnableQuality();

	tri::UpdateQuality<CMeshO>::VertexFromPlane(*source, slicingPlane);

	if (tri::Clean<CMeshO>::CountNonManifoldEdgeFF(*source) > 0){
		std::cout << "Divide mesh error: Mesh has some not 2 manifoldfaces, "
					"splitting surfaces requires manifoldness \n";
	}
	else
	{
		tri::RefineE<CMeshO, tri::QualityMidPointFunctor<CMeshO>, tri::QualityEdgePredicate<CMeshO> >(*source, slicingfunc, slicingpred, false);
		tri::UpdateSelection<CMeshO>::VertexFromQualityRange(*source, 0, std::numeric_limits<float>::max());
		tri::UpdateSelection<CMeshO>::FaceFromVertexStrict(*source);

		if (needUp) {
			if (targetUp) {
				delete targetUp;
			}
			targetUp = new CMeshO();
			tri::Append<CMeshO, CMeshO>::Mesh(*targetUp, *source, true);
			tri::UpdateSelection<CMeshO>::Clear(*targetUp);

			targetUp->face.EnableFFAdjacency();
			tri::UpdateTopology<CMeshO>::FaceFace(*targetUp);
			targetUp->vert.EnableVFAdjacency();
			targetUp->face.EnableVFAdjacency();
			tri::UpdateTopology<CMeshO>::VertexFace(*targetUp);

			// 上部分补洞
			fillHole(targetUp, -slicingPlane.Direction());
		}

		if (needUnder) {
			if (targetUnder) {
				delete targetUnder;
			}
			targetUnder = new CMeshO();
			tri::UpdateSelection<CMeshO>::FaceInvert(*source);
			tri::UpdateSelection<CMeshO>::VertexInvert(*source);
			tri::Append<CMeshO, CMeshO>::Mesh(*targetUnder, *source, true);
			tri::UpdateSelection<CMeshO>::Clear(*targetUnder);

			targetUnder->face.EnableFFAdjacency();
			tri::UpdateTopology<CMeshO>::FaceFace(*targetUnder);
			targetUnder->vert.EnableVFAdjacency();
			targetUnder->face.EnableVFAdjacency();
			tri::UpdateTopology<CMeshO>::VertexFace(*targetUnder);

			// 下部分补洞
			fillHole(targetUnder, slicingPlane.Direction());
		}

		tri::UpdateSelection<CMeshO>::Clear(*source);
	}
}

void SliceEditPlugin::sliceModel() {
	std::cout << __FUNCTION__ << std::endl;

	//clear pointsFromLevel0to10
	originMesh->pointsFromLevel0to10 = new std::vector<std::vector<vcg::Point3f>>();
	originMesh->pointsFromLevel0to10->resize(10);

	// 找到沿Z方向的距离差值
	double maxZ = std::numeric_limits<double>::min();
	for (CMeshO::VertexIterator vit = originMesh->cm.vert.begin(); vit != originMesh->cm.vert.end(); vit++) {
		if (vit->P().Z() > maxZ) {
			maxZ = vit->P().Z();
		}
	}

	typedef CMeshO::ScalarType S;
	Point3<S> center = vcg::Point3<S>(0.0, 0.0, 0.0);//(data->featurePoints[0] + data->featurePoints[0] + data->featurePoints[0]) / 3.0
	Point3<S> dir = Point3<S>(0.0, 0.0, 1.0);
	Plane3<S> slicingPlane;
	double distance = fabs(maxZ - center.Z()) / 10.0;
	headHeight = fabs(maxZ - center.Z());

	for (int i = 0; i < 10; ++i) { //level10 只有一个点，忽略
		Point3<S> offset = center + dir * distance * i;
		slicingPlane.Init(offset, dir);

		MeshModel* cap = new MeshModel(i, "", "");
		vcg::IntersectionPlaneMesh<CMeshO, CMeshO, CMeshO::ScalarType>(originMesh->cm, slicingPlane, cap->cm);
		tri::Clean<CMeshO>::RemoveDuplicateVertex(cap->cm);

		// 按顺序遍历交线边界
		vcg::tri::Allocator<CMeshO>::CompactEveryVector(cap->cm);
		tri::UpdateTopology<CMeshO>::VertexEdge(cap->cm);

		CMeshO::VertexPointer startV = &cap->cm.vert.at(0);
		CMeshO::VertexPointer v = startV;
		v->SetS();
		bool hasPath = false;
		do {
			std::vector<CMeshO::VertexPointer> temp;
			vcg::edge::VVStarVE<CVertexO>(v, temp);
			if (temp.size() != 2) {
				v->C() = vcg::Color4b::Red;
				std::cout << "slice circle is not closed; \n";
				return;
			}

			hasPath = false;
			for (int j = 0; j < 2; ++j) {
				if (!temp[j]->IsS()) {
					v = temp[j];
					v->SetS();
					(*originMesh->pointsFromLevel0to10)[i].push_back(v->P());
					hasPath = true;
					break;
				}
			}
		} while (startV != v && hasPath);

		delete cap;
	}
}
