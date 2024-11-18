// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "JsonImportStyle.h"

class FJsonImportCommands : public TCommands<FJsonImportCommands>
{
public:

	FJsonImportCommands()
	// TEXT("ExodusImport")：这是命令的名称，通常用于在命令系统中标识该命令。
	// NSLOCTEXT("Contexts", "ExodusImport", "ExodusImport Plugin")：这是一个本地化文本宏，用于定义命令的上下文和描述。它的三个参数分别是："Contexts"：上下文的名称。"ExodusImport"：命令的标识符。"ExodusImport Plugin"：命令的显示名称，通常用于用户界面。
	// NAME_None：这是一个标识符，表示没有特定的命令类别。可以用于将命令分组。
	// FJsonImportStyle::GetStyleSetName()：这是一个静态方法调用，返回与该命令相关的样式集名称。样式集用于定义命令在用户界面中的外观。
		: TCommands<FJsonImportCommands>(
			TEXT("ExodusImport"), 
			NSLOCTEXT("Contexts", "ExodusImport", "ExodusImport Plugin"), 
			NAME_None, FJsonImportStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginImportAction;
	TSharedPtr< FUICommandInfo > PluginDebugAction;
	TSharedPtr< FUICommandInfo > PluginTestAction;
	TSharedPtr< FUICommandInfo > PluginLandscapeTestAction;
	TSharedPtr< FUICommandInfo > PluginSkinMeshTestAction;
};
