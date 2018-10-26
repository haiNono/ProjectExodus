#pragma once

#include "JsonTypes.h"
#include "MaterialBuilder/MaterialFingerprint.h"

class JsonImporter;
class UMaterialExpression;

DECLARE_LOG_CATEGORY_EXTERN(JsonLogMatNodeSort, Log, All);

class MaterialBuildData{
public:
	JsonMaterialId matId = -1;
	JsonImporter *importer = 0;

	UMaterialExpression *mainUv = nullptr;
	UMaterialExpression *detailUv = nullptr;

	UMaterialExpression *albedoTexExpression = nullptr;
	UMaterialExpression *albedoColorExpression = nullptr;
	UMaterialExpression *albedoDetailTexExpression = nullptr;

	UMaterialExpression *normalTexExpression = nullptr;
	UMaterialExpression *normalExpression = nullptr;
	UMaterialExpression *detailNormalTexExpression = nullptr;
	UMaterialExpression *detailNormalExpression = nullptr;

	UMaterialExpression *detailMaskExpression = nullptr;

	UMaterialExpression *metallicTexExpression = nullptr;
	UMaterialExpression *specularTexExpression = nullptr;
	UMaterialExpression *smoothTexSource= nullptr;

	UMaterialExpression *albedoExpression = nullptr;
	UMaterialExpression *specularExpression = nullptr;
	UMaterialExpression *metallicExpression = nullptr;
	UMaterialExpression *emissiveExpression = nullptr;

	MaterialBuildData(JsonMaterialId matId_, JsonImporter *importer_)
	:matId(matId_), importer(importer_){
	}
};

class MaterialBuilder{
public:
	UMaterial *importMaterial(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId);
	UMaterial *importMaterial(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId);
	MaterialBuilder() = default;
protected:
	void buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void arrangeNodesGrid(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void arrangeNodesTree(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processOpacity(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processMainUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processDetailUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);

	void processAlbedo(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processNormalMap(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processEmissive(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processOcclusion(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processMetallic(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processSpecular(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processDetailMask(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processRoughness(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
};
