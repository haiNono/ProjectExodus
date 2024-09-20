// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "JsonImportStyle.h"
#include "JsonImportCommands.h"

#include "JsonImporter.h"

#include "LevelEditor.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "DesktopPlatformModule.h"

#include "JsonLog.h"

#include "Tests/LandscapeTest.h"
#include "Tests/OutlinerTest.h"
#include "Tests/SkinMeshTest.h"
#include "Tests/PluginDebugTest.h"

#include "LocTextNamespace.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

static const FName JsonImportTabName("JsonImport");

// 在模块加载到内存后执行初始化操作
void FJsonImportModule::StartupModule(){
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	LOCTEXT("Importing textures", "Importing textures");
	FJsonImportStyle::Initialize();
	FJsonImportStyle::ReloadTextures();
	// 注册命令到 Unreal Engine 的命令系统中
	FJsonImportCommands::Register();
	// MakeShareable 是 Unreal Engine (UE) 提供的一个函数。它用于创建 TSharedPtr 的实例。TSharedPtr 是一个智能指针，用于管理对象的生命周期，确保在没有引用时自动释放内存
	// FUICommandList用于管理用户界面命令，用来给按钮绑定响应函数
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginImportAction,
		// 用于创建一个 FExecuteAction 对象，将一个类的成员函数与一个对象实例绑定，以便在特定事件（如按钮点击）发生时调用该成员函数
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginImportButtonClicked),
		// 是 Unreal Engine 中用于定义命令是否可以执行的条件的一个结构，控制某个命令（如按钮点击、菜单项等）在特定情况下是否可用。
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginDebugAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginDebugButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginTestButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginLandscapeTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginLandscapeTestButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginSkinMeshTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginSkinMeshTestButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, 
			FMenuExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, 
			FToolBarExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FJsonImportModule::ShutdownModule(){
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FJsonImportStyle::Shutdown();

	FJsonImportCommands::Unregister();
}

void FJsonImportModule::PluginDebugButtonClicked(){
	PluginDebugTest test;
	test.run();
}

void FJsonImportModule::PluginTestButtonClicked(){
	OutlinerTest test;
	test.run();
}

void FJsonImportModule::PluginLandscapeTestButtonClicked(){
	LandscapeTest test;
	test.run();
}

void FJsonImportModule::PluginImportButtonClicked(){
	// Put your "OnButtonClicked" stuff here
	FStringArray files;
	// files 用于存储用户选择的文件路径的字符串数组
	// OpenFileDialog返回布尔值
	if (!FDesktopPlatformModule::Get()->OpenFileDialog(0,
			FString("Open JSON File"), FString(), FString(), FString("JSON file|*.json"), EFileDialogFlags::None, files))
			return;

	if (!files.Num()){
			UE_LOG(JsonLog, Log, TEXT("No file to open"));
			return;
	}
	// 将文件的相对路径转换为完整的绝对路径。
	auto filename = FPaths::ConvertRelativePathToFull(files[0]);
	JsonImporter importer;
	importer.importProject(filename);
	//importer.importScene(filename);
}

void FJsonImportModule::PluginSkinMeshTestButtonClicked(){
	SkinMeshTest skinTest;
	skinTest.run();
}

void FJsonImportModule::AddMenuExtension(FMenuBuilder& Builder){
	Builder.AddMenuEntry(FJsonImportCommands::Get().PluginImportAction);
}

void FJsonImportModule::AddToolbarExtension(FToolBarBuilder& Builder){
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginImportAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginDebugAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginSkinMeshTestAction);
	/*
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginTestAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginLandscapeTestAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginSkinMeshTestAction);
	*/
}

IMPLEMENT_MODULE(FJsonImportModule, ExodusImport)

#undef LOCTEXT_NAMESPACE
