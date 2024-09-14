#include "JsonImportPrivatePCH.h"
#include "JsonProject.h"
#include "macros.h"

void JsonProject::load(JsonObjPtr data){
	using namespace JsonObjects;
	// 从给定的 JSON 数据中提取一个对象，并将其赋值给第二个参数
	getJsonObj(data, config, "config");
	getJsonObj(data, externResources, "externResources");
}
