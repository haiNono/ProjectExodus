using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.SceneManagement;


namespace SceneExport{
	public static class ExportUtility{
		// 根据给定的 MeshCollider 返回其使用标志（如是否为凸形碰撞体或三角形碰撞体）
		public static MeshUsageFlags getMeshUsageFlags(MeshCollider collider){
			if (!collider)
				return MeshUsageFlags.None;
			if (collider.convex)
				return MeshUsageFlags.ConvexCollider;
			return MeshUsageFlags.TriangleCollider;
		}
	// 检查给定的 GameObject 是否具有支持的组件（如 MeshRenderer、Light、SkinnedMeshRenderer 等
		public static bool hasSupportedComponents(GameObject obj){
			if (!obj)
				throw new System.ArgumentNullException("obj");
			return 
				obj.hasComponent<MeshRenderer>()
				||obj.hasComponent<Light>()
				||obj.hasComponent<SkinnedMeshRenderer>()
				||obj.hasComponent<ReflectionProbe>()
				||obj.hasComponent<Terrain>()
				//||obj.hasComponent<AudioSource>()
			;
		}
	
		public static readonly int invalidId = -1;
		
		// 用于遍历游戏对象层次结构时的回调方法。
		public delegate bool HierarchyWalker(GameObject curObj);
		// 获取与给定 GameObject 关联的根预制体资产
		public static GameObject getLinkedRootPrefabAsset(GameObject obj){
			var prefab = getLinkedPrefab(obj);
			if (!prefab)
				return null;
			// PrefabUtility是Unity提供的
			var root = PrefabUtility.FindPrefabRoot(prefab);
			return root;
		}
		// 获取与给定 GameObject 关联的预制体。如果对象是预制体实例，则返回其源预制体。
		public static GameObject getLinkedPrefab(GameObject obj){
			var prefType = PrefabUtility.GetPrefabType(obj);
			if ((prefType == PrefabType.ModelPrefab) || (prefType == PrefabType.Prefab)){
				return obj;
			}
			if ((prefType != PrefabType.ModelPrefabInstance) && (prefType != PrefabType.PrefabInstance))
				return null;
			var source = PrefabUtility.GetCorrespondingObjectFromSource(obj);
			if (!source)
				return null;
			var sourceObj = source as GameObject;
			return sourceObj;
		}
		// 遍历给定的 GameObject 队列，执行回调方法以处理每个对象及其子对象
		public static void walkHierarchy(Queue<GameObject> objects, HierarchyWalker callback){
			if (objects == null)
				throw new System.ArgumentNullException("objects");
				
			if (callback == null)
				throw new System.ArgumentNullException("callback");
				
			while(objects.Count > 0){
				var curObject = objects.Dequeue();
				if (!curObject)
					continue;
					
				if (!callback(curObject))
					continue;
					
				foreach(Transform curChild in curObject.transform){
					if (!curChild)
						continue;
					if (!curChild.gameObject)
						continue;
					objects.Enqueue(curChild.gameObject);
				}
			}
		}
		
		public static void walkHierarchy(GameObject obj, HierarchyWalker callback){
			var q = new Queue<GameObject>();
			walkHierarchy(q, callback);
		}
		
		public static void walkHierarchy(GameObject[] objs, HierarchyWalker callback){
			var q = new Queue<GameObject>();
			if (objs != null){
				foreach(var cur in objs)
					q.Enqueue(cur);
			}
			walkHierarchy(q, callback);
		}
		
		public static bool isValidId(ResId id){
			return id.isValid;
		}

		public static bool isValidId(int id){
			return id >= 0;
		}
		// 获取给定 GameObject 的完整路径（包括其父对象的名称）。
		public static string getObjectPath(GameObject obj){
			if (!obj)
				return "(null)";
			if (!obj.transform.parent)
				return obj.name;
			return getObjectPath(obj.transform.parent.gameObject) + "/" + obj.name;
		}
		
		public static string formatString(string fmt, params object[] args){
			return string.Format(fmt, args);
		}
		
		public static bool showCancellableProgressBar(string title, string info, int progress, int maxProgress){
			float floatProgress = 0.0f;
			if (maxProgress != 0)
				floatProgress = (float)progress/(float)maxProgress;
			return EditorUtility.DisplayCancelableProgressBar(title, info, floatProgress);
		}
		
		public static void showProgressBar(string title, string info, int progress, int maxProgress){
			float floatProgress = 0.0f;
			if (maxProgress != 0)
				floatProgress = (float)progress/(float)maxProgress;
			EditorUtility.DisplayProgressBar(title, info, floatProgress);
		}
		
		public static void hideProgressBar(){
			EditorUtility.ClearProgressBar();
		}
		// 将给定 GameObject 的组件转换为 JSON 类型的列表，使用提供的转换函数
		public static List<JsonType> convertComponentsList<CompType, JsonType>(GameObject gameObject, 
				System.Func<CompType, JsonType> converter) //thanks for not letting me specify constructor with parameters constraint, I guess?
				where CompType: Component {				
			if (!gameObject)
				throw new System.ArgumentNullException("gameObject");
			if (converter == null)
				throw new System.ArgumentNullException("converter");				
				
			var components = gameObject.GetComponents<CompType>();
			var jsonObjects = new List<JsonType>();
			var componentIndex = 0;
			foreach(var curComponent in components){
				var curIndex = componentIndex;
				componentIndex++;
				if (!curComponent){
					Debug.LogWarningFormat("Component {0} is null on object {1}. ComponentType: {2}"
						, curIndex, gameObject, typeof(CompType).Name);
					continue;
				}
				
				var jsonObject = converter(curComponent);
				jsonObjects.Add(jsonObject);
			}
			return jsonObjects;
		}
		
		public static JsonType[] convertComponents<CompType, JsonType>(GameObject gameObject, 
				System.Func<CompType, JsonType> converter) //thanks for not letting me specify constructor with parameters constraint, I guess?
				where CompType: Component {				
				
			return convertComponentsList<CompType, JsonType>(gameObject, converter).ToArray();
		}

		public static List<GameObject> getSelectedGameObjects(){
			List<GameObject> result = new List<GameObject>();
			if (Selection.transforms.Length <= 0)
				return result;

			foreach(var cur in Selection.transforms){
				if (cur == null)
					continue;
				var curObj = cur.gameObject;
				if (!curObj)
					continue;
				result.Add(curObj);
			}
			return result;
		}
	}
}
