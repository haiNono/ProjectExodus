﻿using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;
using UnityEngine.SceneManagement;

namespace SceneExport{
	[System.Serializable]
	// 表示 Unity 场景的 JSON 数据结构
	public class JsonScene: IFastJsonValue{
		public string name;
		public string path;
		// 场景的构建索引，通常用于场景加载的顺序。
		public int buildIndex = -1;
		// 存储场景中所有游戏对象的列表
		public List<JsonGameObject> objects = new List<JsonGameObject>();
		// 从 Unity 的 Scene 对象创建一个 JsonScene 实例
		public static JsonScene fromScene(Scene scene, ResourceMapper resMap, bool showGui){
			var rootObjects = scene.GetRootGameObjects();
			var result = fromObjects(rootObjects, resMap, showGui);
			result.name = scene.name;
			result.path = scene.path;
			result.buildIndex = scene.buildIndex;
			return result;
		}
		
		public static JsonScene fromObject(GameObject arg, ResourceMapper resMap, bool showGui){
			return fromObjects(new GameObject[]{arg}, resMap, showGui);
		}
		// showGui 是否显示进度条
		public static JsonScene fromObjects(GameObject[] args, ResourceMapper resMap, bool showGui){
			try{
				var result = new JsonScene();
				// 导出过程中用到的临时类
				var objMap = new GameObjectMapper();
			// 获取或创建id
			// 将args中的每一个对象的子对象分配id，并添加进objMap.objectList和objMap.objectMap
				foreach(var curObj in args){
					objMap.gatherObjectIds(curObj);
				}
			
				foreach(var curObj in objMap.objectList){
					// 获取当前游戏对象的prefab信息并添加到resMapper.prefabs
					resMap.gatherPrefabData(curObj);
				}
			
				for(int i = 0; i < objMap.objectList.Count; i++){
					/*TODO: The constructor CAN add more data, but most of it would've been added prior to this point.
					Contempalting whether to enforce it strictly or not.*/
					if (showGui){
						ExportUtility.showProgressBar("Collecting scene data", 
							string.Format("Adding scene object {0}/{1}", i, objMap.numObjects), i, objMap.objectList.Count);
					}
				
					var newObj = new JsonGameObject(objMap.objectList[i], objMap, resMap);
					result.objects.Add(newObj);
				}
				return result;
			}
			finally{
				if (showGui){
					ExportUtility.hideProgressBar();
				}
			}
		}
			
		public void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("objects", objects);
		}
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("buildIndex", buildIndex);
			writeJsonObjectFields(writer);//the hell? Why?
			writer.endObject();
		}

		public void fixNameClashes(){
			var nameClashes = new Dictionary<NameClashKey, List<ResId>>();
			for(int i = 0; i < objects.Count; i++){
				var cur = objects[i];
				var key = new NameClashKey(cur.name, cur.parent);
				var idList = nameClashes.getValOrGenerate(key, (parId_) => new List<ResId>());
				idList.Add(cur.id);
			}
			
			foreach(var entry in nameClashes){
				var key = entry.Key;
				var list = entry.Value;
				if ((list == null) || (list.Count <= 1))
					continue;

				for(int i = 1; i < list.Count; i++){
					var curId = list[i];
					//if ((curId <= 0) || (curId >= objects.Count)){
					if (!curId.isValid || (curId.objectIndex > objects.Count)){
						Debug.LogErrorFormat("Invalid object id {0}, while processing name clash {1};\"{2}\"", 
							curId, key.parentId, key.name);
						continue;
					}

					var curObj = objects[curId.objectIndex];
					var altName = string.Format("{0}-#{1}", key.name, i);
					while(nameClashes.ContainsKey(new NameClashKey(altName, key.parentId))){
						altName = string.Format("{0}-#{1}({2})", 
							key.name, i, System.Guid.NewGuid().ToString("n"));
						//break;
					}
					curObj.nameClash = true;
					curObj.uniqueName = altName;
				}								
			}			
		}
	}
}