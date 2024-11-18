// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"

#include "JsonImportStyle.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FJsonImportStyle::StyleInstance = NULL;

void FJsonImportStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FJsonImportStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FJsonImportStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("JsonImportStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FJsonImportStyle::Create()
{
	// MakeShareable创建指针，能够自动管理内存。使用智能指针可以避免内存泄漏和悬挂指针等问题，因为它们会在不再需要时自动释放内存。
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("JsonImportStyle"));
	// 用于设置样式的内容根目录
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ExodusImport")->GetBaseDir() / TEXT("Resources"));

// 为 FSlateStyleSet 对象设置一个名为 "ExodusImport.PluginImportAction" 的样式项，并指定其使用的图像资源
// "ExodusImport.PluginImportAction" 是样式项的名称。这个名称通常与命令或功能相关联，表示该样式项是为 PluginImportAction 命令定义的。
	Style->Set("ExodusImport.PluginImportAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ExodusImport.PluginDebugAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ExodusImport.PluginTestAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ExodusImport.PluginLandscapeTestAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ExodusImport.PluginSkinMeshTestAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FJsonImportStyle::ReloadTextures()
{
	// 获取slate实例的渲染器，并强制重新加载用户界面纹理。它针对的是插件窗口、工具栏、菜单等 UI 元素的纹理，而不是游戏中的纹理资源。
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FJsonImportStyle::Get()
{
	return *StyleInstance;
}
