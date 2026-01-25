#pragma once
#include <vector>
#include "../../common/ml_document/mesh_document.h"

// 设置成单例模式
class PluginCommonData {

	PluginCommonData() {
		hasAligned = false;
		pointsFromLevel0to10 = new std::vector<std::vector<vcg::Point3f>>();
		std::cout << __LINE__ <<"  "<< this << std::endl;
	}

	bool hasAligned; // 暂时只有一个公共变量

public:
	~PluginCommonData() {};

	static PluginCommonData* data;
	std::vector<vcg::Point3f> featurePoints; // 三个特征点
	std::vector<std::vector<vcg::Point3f>>* pointsFromLevel0to10; // 每个截面contour的点

	bool ifAligned();
	void setAligned();
	static PluginCommonData* getData();

};

