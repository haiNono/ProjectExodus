#include "JsonImportPrivatePCH.h"
#include "JsonObjects.h"
#include "JsonLog.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

using namespace JsonObjects;

JsonObjPtr JsonObjects::loadJsonFromFile(const FString &filename){
	FString jsonString;
	// LoadFileToString从指定的文件中加载内容并将其存储为字符串。
	if (!FFileHelper::LoadFileToString(jsonString, *filename)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load json file \"%s\""), *filename);
		return 0;
	}

	UE_LOG(JsonLog, Log, TEXT("Loaded json file \"%s\""), *filename);
	// 创建一个 JSON 解析器，用于读取和解析 JSON 字符串
	JsonReaderRef reader = TJsonReaderFactory<>::Create(jsonString);

	JsonObjPtr jsonData = MakeShareable(new FJsonObject());
	if (!FJsonSerializer::Deserialize(reader, jsonData)){
		UE_LOG(JsonLog, Warning, TEXT("Could not parse json file \"%s\""), *filename);
		return 0;
	}
	return jsonData;
}

#undef LOCTEXT_NAMESPACE
