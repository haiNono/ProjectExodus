using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{

	public enum ExportType{
		CurrentObject,
		SelectedObjects,
		CurrentScene,
		WholeProject
	}
	
[System.Serializable]
public class ExporterWindow: EditorWindow{
	// 当EditorWindow 类被挂载到 Unity编辑器上时，会默认调用OnEnable、OnGUI方法
	[SerializeField]bool disclaimerVisible = true;
	[SerializeField]Vector2 windowScrollPos;
	[SerializeField]Vector2 disclaimerScrollPos;
	[SerializeField]string targetPath = "";
	
	void OnEnable(){
		minSize = new Vector2(640.0f, 480.0f);
	}
	
	void OnDisable(){
	}
	
	[SerializeField]ExportGuiStyles guiStyles = new ExportGuiStyles();
	[SerializeField]ExportType exportType = ExportType.CurrentScene;

	static string getDefaultJsonFileName(ExportType exportType){
		switch(exportType){
			case(ExportType.CurrentObject):{
				var obj = Selection.activeGameObject;
				if (obj)
					return obj.name;
				break;
			}
			case(ExportType.SelectedObjects):{
				foreach(var cur in Selection.transforms){
					if (cur == null)
						continue;
					var curObj = cur.gameObject;
					if (!curObj)
						continue;
					return curObj.name;
				}
				break;
			}
			case(ExportType.CurrentScene):{
				// 通过EditorSceneManager.GetActiveScene()读取场景
				var scene = EditorSceneManager.GetActiveScene();
				if (scene.IsValid())
					return scene.name;
				break;
			}
			case(ExportType.WholeProject):{
				return Application.productName;
			}
		}
		return Application.productName;
	}	
	
	[SerializeField] List<string> messages = new List<string>();
	void logMsg(string message){
		messages.Add(message);
	}
	
	bool isValidExportState(){
		string tmp;
		return isValidExportState(out tmp);
	}
	
	bool isValidExportState(out string msg){
		msg = "";
		switch(exportType){
			case(ExportType.CurrentObject):{
				if (!Selection.activeGameObject){
					msg = "No active object! Select an object for this export type";
					return false;
				}
				return true;
			}
			case(ExportType.SelectedObjects):{
				if ((Selection.gameObjects == null) || (Selection.gameObjects.Length == 0)){
					msg = "No objects selected. Selected objects for this export type.";
					return false;
				}
				return true;
			}
		}
		return true;
	}

	[SerializeField] public TextAsset disclaimerTextAsset;//this needs to be set in inspector
	public static class GuiTexts{
		public static readonly string disclaimer = @"This is a placeholder disclaimer. Replace or specify a text asset file";
	}
	
	public static class GuiContents{
		public static readonly GUIContent disclaimerSection = 
			new GUIContent(
				"Disclaimer", "Click to open the section and read the disclaimer for the plugin"
			);
			
		public static readonly GUIContent exportType =			
			new GUIContent("1. Select Export Type", 
@"Select export type here: 
	* single object (currently selected object will be exported), 
	* selected object (selected objects will be exported),
	* current scene (currently selected scene will be exported)
	* whole project (entire project will be processed)"
			);
	}
	
	string getDisclaimerText(){
		if (disclaimerTextAsset)
			return disclaimerTextAsset.text;
		return GuiTexts.disclaimer;
	}
	
	void initGuiStyles(){
		//minSize = new Vector2(320.0f, 240.0f);
		if (guiStyles == null){
			guiStyles = new ExportGuiStyles();
		}
		guiStyles.init();
	}
	
	void processDisclaimerArea(){
		disclaimerVisible = 
		EditorGUILayout.Foldout(disclaimerVisible, GuiContents.disclaimerSection);
		if (!disclaimerVisible)
			return;
		using(var tmp = ExportGuiUtility.scopedGuiEnabled(false)){
			EditorGUILayout.TextArea(getDisclaimerText(), guiStyles.disclaimerTextArea);
		}
		if (GUILayout.Button("View full Disclaimer")){
			DisclaimerWindow.openDisclaimerWindow();
		}
	}
	
	void processTargetPathLabel(){
		if (string.IsNullOrEmpty(targetPath)){
			GUILayout.Label("Target path not selected. Press \"select target\" button to select desintation file", guiStyles.errorLabel);
		}
		else{
			GUILayout.Label(string.Format("Target path: \"{0}\"", targetPath), guiStyles.normalLabel);
			if (Utility.isInProjectPath(targetPath)){
				GUILayout.Label("Target path is within project folder. *.tif textures will not be automatically converted to png.", guiStyles.warningLabel);
			}
			else{
				GUILayout.Label("Target path is outside of project folder. Textures will be copied and converted to png when needed. " +
					"Please note that previous data will be silently overwritten.", guiStyles.normalLabel);
			}
		}
	}
	
	void processSelectionWarningControls(){
		string msg;
		var result = isValidExportState(out msg);
		if (!result){
			EditorGUILayout.LabelField(msg, guiStyles.errorLabel);
		}
	}
	// 点击 select target oath按钮的时候调用，设置目标路径
	void processExportButtons(){
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("2. Select Target Path")){
			var defaultName = getDefaultJsonFileName(exportType);
			var filePath = EditorUtility.SaveFilePanel("Export selected objects", "", defaultName, "json");
			if (!string.IsNullOrEmpty(filePath)){
				targetPath = filePath;
				var pathInProject = Utility.isInProjectPath(filePath);
				if (pathInProject)
					Debug.LogWarningFormat("File \'{0}\' is in project directory.\nTexture conversion willl not be performed", filePath);
			}
		}

		using(var tmp = ExportGuiUtility.scopedGuiEnabled(canExport())){
			if (GUILayout.Button("3. Begin export")){
				processExport();
			}
		}
		
		GUILayout.EndHorizontal();
	}
	
	void processCloseButton(){
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("Close")){
			Close();
		}
		GUILayout.EndHorizontal();
	}
	// 初始化GUI样式，绘制窗口内容
	// 每一帧中被调用
	void OnGUI(){
		initGuiStyles();
		
		windowScrollPos = EditorGUILayout.BeginScrollView(windowScrollPos);
		
		GUILayout.BeginVertical();//GUILayout.MinWidth(640), GUILayout.MinHeight(400));
		GUILayout.Label("Export Settings", EditorStyles.boldLabel);
		
		processDisclaimerArea();
		
		exportType = (ExportType)EditorGUILayout.EnumPopup(GuiContents.exportType, exportType);

		processSelectionWarningControls();		
		processTargetPathLabel();
		
		processExportButtons();
		processCloseButton();	
		
		GUILayout.EndVertical();
		EditorGUILayout.EndScrollView();
	}
	
	bool canExport(){
		return isValidExportState() && !string.IsNullOrEmpty(targetPath);
	}
	
	bool checkTargetPath(string targetPath){
		if (string.IsNullOrEmpty(targetPath)){
			Debug.LogErrorFormat("No target path, cannot proceed");
			return false;
		}
		return true;
	}
	
	void beginSingleObjectExport(){
		if (Selection.activeObject == null){
			Debug.LogErrorFormat("No active object, cannot proceed with export");
			return;
		}
		var obj = Selection.activeGameObject;
		// targetPath是个文件路径，不是目录
		if (!checkTargetPath(targetPath))
			return;
		var logger = new Logger();	
		// 返回 JsonProject类型
		var proj = JsonProject.fromObject(obj, true);
		proj.saveToFile(targetPath, true, true, logger);
		ExportResultWindow.openWindow(logger);
	}
	
	void beginSelectedObjectsExport(){
		var objects = ExportUtility.getSelectedGameObjects();
		if (objects.Count <= 0)
			return;

		if (!checkTargetPath(targetPath))
			return;			
				
		var logger = new Logger();						
		var proj = JsonProject.fromObjects(objects.ToArray(), true);
		proj.saveToFile(targetPath, true, true, logger);
		ExportResultWindow.openWindow(logger);
	}
	
	void beginCurrentSceneExport(){
		var scene = EditorSceneManager.GetActiveScene();
		if (!scene.IsValid()){
			Debug.LogWarningFormat("No active scene, cannot export.");
		}
		
		if (!checkTargetPath(targetPath))
			return;
					
		var logger = new Logger();						
		var proj = JsonProject.fromScene(scene, true);
		proj.saveToFile(targetPath, true, true, logger);
		ExportResultWindow.openWindow(logger);
	}
	
	void beginFullProjectExport(){
		var logger = new Logger();		
		// 创建一个 JsonProject实例
		var proj = JsonProject.fromCurrentProject(true, logger);
		if (proj != null){
			proj.saveToFile(targetPath, true, true, logger);
		}
		ExportResultWindow.openWindow(logger);
	}
	// 处理任务入口
	void processExport(){
		switch(exportType){
			case(ExportType.CurrentObject):{
				beginSingleObjectExport();
				break;
			}
			case(ExportType.SelectedObjects):{
				beginSelectedObjectsExport();
				break;
			}
			case(ExportType.CurrentScene):{
				beginCurrentSceneExport();
				break;
			}
			case(ExportType.WholeProject):{
				beginFullProjectExport();
				break;
			}
		}		
	}
}

}//namespace SceneExport