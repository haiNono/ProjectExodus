#include "JsonImportPrivatePCH.h"
#include "JsonScene.h"
#include "macros.h"

// 将scene中的object分别存储到数组和map中
void JsonScene::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, buildIndex);

	getJsonObjArray(data, objects, "objects");

	buildInstanceIdMap();
}
// 使用map存储objects数组中的元素
void JsonScene::buildInstanceIdMap(){
	gameObjectInstanceIdMap.Empty();
	for (const auto& obj : objects){
		auto instId = obj.instanceId;
		auto objId = obj.id;

		auto foundId = gameObjectInstanceIdMap.Find(instId);
		if (foundId){
			UE_LOG(JsonLog, Warning,
				TEXT("Duplicate instance id found. New id objectId: %d; existing objectId: %d; instanceId: %d"),
				objId, *foundId, instId
			);
			gameObjectInstanceIdMap.Remove(instId);
		}
		gameObjectInstanceIdMap.Add(instId, objId);
	}
}

bool JsonScene::containsTerrain() const{
	for(const auto& cur: objects){
		if (cur.hasTerrain())
			return true;
	}
	return false;
}

const JsonGameObject* JsonScene::findJsonObject(JsonId id) const{
	using namespace JsonObjects;
	if (!isValidId(id))
		return nullptr;
	return &objects[id];
}

const JsonGameObject* JsonScene::findJsonObjectByInstId(InstanceId instId) const{
	auto foundId = gameObjectInstanceIdMap.Find(instId);
	if (foundId){
		return findJsonObject(*foundId);
	}
	return nullptr;
}
