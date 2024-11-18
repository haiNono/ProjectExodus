// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"
#include "JsonImportCommands.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

void FJsonImportCommands::RegisterCommands()
{
	// UI_COMMAND 是 Unreal Engine 中用于定义用户界面命令的宏。它用于注册一个命令，以便在编辑器或游戏中使用。
	// PluginImportAction：这是命令的标识符，通常是一个 FName 类型的变量。
	// "ExodusImport"：这是命令的名称，表示该命令在用户界面中显示的文本
	// "Execute ExodusImport action"：这是命令的描述，
	// EUserInterfaceActionType::Button：这是一个枚举值，表示命令的类型。在这里，它指定该命令是一个按钮类型的命令，意味着它可以在用户界面中作为按钮使用。
	// FInputGesture()：这是一个输入手势对象，表示与该命令关联的输入手势。
	UI_COMMAND(PluginImportAction, "ExodusImport", "Execute ExodusImport action", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginDebugAction, "ExodusImport Debug", "Execute ExodusImport debug action (do not use)",
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginTestAction, "Exodus object test", "Run ExodusImport Test",
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginLandscapeTestAction, "Exodus landscape crash test", "Run ExodusImport landscape test", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginSkinMeshTestAction, "SkinMesh Test", "Run skin mesh test", 
		EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
