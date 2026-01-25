#include "pluginCommonData.h"

PluginCommonData* PluginCommonData::data = new PluginCommonData();

PluginCommonData* PluginCommonData::getData() {
	std::cout << data << std::endl;
	return data;
}

bool PluginCommonData::ifAligned() {
	return hasAligned;
}

void PluginCommonData::setAligned() {
	hasAligned = true;
}
