using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonGameObject: IFastJsonValue{
		public string name;
		public string scenePath;
		public int instanceId = -1;
		public ResId id = ResId.invalid;
		public Vector3 localPosition = Vector3.zero;
		public Quaternion localRotation = Quaternion.identity;
		public Vector3 localScale = Vector3.one;
		public Matrix4x4 worldMatrix = Matrix4x4.identity;
		public Matrix4x4 localMatrix = Matrix4x4.identity;
		
		public List<ResId> children = new List<ResId>();
		public List<string> childNames = new List<string>();
		public ResId parent = ResId.invalid;
		public string parentName = "";
		public ResId mesh = ResId.invalid;
		
		public bool activeSelf = true;
		public bool activeInHierarchy = true;
		
		public JsonRendererData[] renderer = null;
		public JsonLight[] light = null;
		public JsonReflectionProbe[] reflectionProbes = null;
		public List<JsonSkinRendererData> skinRenderers = new List<JsonSkinRendererData>();
		public List<JsonAnimator> animators = new List<JsonAnimator>();
		public List<JsonCollider> colliders = new List<JsonCollider>();
		public List<JsonRigidbody> rigidbodies = new List<JsonRigidbody>();
		public List<JsonPhysicsJoint> joints = new List<JsonPhysicsJoint>();
		
		public JsonTerrain[] terrains = null;
		
		public bool isStatic = true;
		public bool lightMapStatic = true;
		public bool occluderStatic = true;
		public bool occludeeStatic = true;
		public bool navigationStatic = true;
		public bool reflectionProbeStatic = true;
			
		public bool nameClash = false;
		public string uniqueName = "";
		
		public ResId prefabRootId = ResId.invalid;
		public ResId prefabObjectId = ResId.invalid;
		public bool prefabInstance = false;
		public bool prefabModelInstance = false;
		public string prefabType = "";
			
		public static void registerLinkedData(GameObject obj, ResourceMapper resMap){
			if (!obj)
				return;
		}
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("instanceId", instanceId);
			writer.writeKeyVal("id", id);
			
			writer.writeKeyVal("scenePath", scenePath);
			
			writer.writeKeyVal("localPosition", localPosition);
			writer.writeKeyVal("localRotation", localRotation);
			writer.writeKeyVal("localScale", localScale);
			writer.writeKeyVal("worldMatrix", worldMatrix);
			writer.writeKeyVal("localMatrix", localMatrix);
			
			writer.writeKeyVal("localTransform", new JsonTransform(localMatrix));
			writer.writeKeyVal("globalTransform", new JsonTransform(worldMatrix));
			
			writer.writeKeyVal("children", children);
			writer.writeKeyVal("childNames", childNames);
			writer.writeKeyVal("parent", parent);
			writer.writeKeyVal("parentName", parentName);
			writer.writeKeyVal("mesh", mesh);
			writer.writeKeyVal("isStatic", isStatic);
			writer.writeKeyVal("lightMapStatic", lightMapStatic);
			writer.writeKeyVal("navigationStatic", navigationStatic);
			writer.writeKeyVal("occluderStatic", occluderStatic);
			writer.writeKeyVal("occludeeStatic", occludeeStatic);
			writer.writeKeyVal("reflectionProbeStatic", reflectionProbeStatic);
			
			writer.writeKeyVal("activeSelf", activeSelf);
			writer.writeKeyVal("activeInHierarchy", activeInHierarchy);
					
			writer.writeKeyVal("nameClash", nameClash);
			writer.writeKeyVal("uniqueName", uniqueName);
				
			writer.writeKeyVal("prefabRootId", prefabRootId);
			writer.writeKeyVal("prefabObjectId", prefabObjectId);
			writer.writeKeyVal("prefabInstance", prefabInstance);
			writer.writeKeyVal("prefabModelInstance", prefabModelInstance);
			writer.writeKeyVal("prefabType", prefabType);

			//TODO remove empty arrays				
			writer.writeKeyVal("renderer", renderer, true);
			writer.writeKeyVal("light", light, true);
			writer.writeKeyVal("reflectionProbes", reflectionProbes, true);
			writer.writeKeyVal("skinRenderers", skinRenderers, true);
			writer.writeKeyVal("animators", animators, true);
			writer.writeKeyVal("terrains", terrains, true);
			writer.writeKeyVal("colliders", colliders, true);
			writer.writeKeyVal("rigidbodies", rigidbodies, true);
			writer.writeKeyVal("joints", joints, true);
			
			writer.endObject();
		}
			
		public JsonGameObject(GameObject obj, GameObjectMapper objMap, ResourceMapper resMap){
			name = obj.name;
			// 获取当前游戏对象在场景中的完整路径
			scenePath = obj.getScenePath();
			instanceId = obj.GetInstanceID();
			id = objMap.getId(obj);
			localPosition = obj.transform.localPosition;
			localRotation = obj.transform.localRotation;
			localScale = obj.transform.localScale;
			worldMatrix = obj.transform.localToWorldMatrix;
			localMatrix = worldMatrix;
			if (obj.transform.parent){
				localMatrix = obj.transform.parent.worldToLocalMatrix * worldMatrix;
			}
			
			isStatic = obj.isStatic;
			// 获取gameobject的静态编辑器标志，标志决定了如何处理对象，如是否将其包含在光照计算中、是否视为静态碰撞器等
			var flags = GameObjectUtility.GetStaticEditorFlags(obj);
		#if UNITY_2019_2_OR_NEWER
		// 枚举值，用于指示一个游戏对象是否应该参与全局光照（Global Illumination，简称 GI）的计算
			lightMapStatic = (flags & StaticEditorFlags.ContributeGI) != 0;
		#else
		// 枚举值，指示游戏对象是否应该被包含在光照贴图（lightmapping）中
			lightMapStatic = (flags & StaticEditorFlags.LightmapStatic) != 0;
		#endif
		// 与遮挡相关的属性
			// 用于控制游戏对象在编辑器中的静态状态。
			occluderStatic = (flags & StaticEditorFlags.OccluderStatic) != 0;
			occludeeStatic = (flags & StaticEditorFlags.OccludeeStatic) != 0;
			// 枚举中的一个标志，用于指示游戏对象是否应该被包含在导航网格（Navigation Mesh，简称 NavMesh）的计算中。
			navigationStatic = (flags & StaticEditorFlags.NavigationStatic) != 0;
			// 枚举中的一个标志，用于指示游戏对象是否应该被用作反射探针（Reflection Probe）的静态环境。
			reflectionProbeStatic = (flags & StaticEditorFlags.ReflectionProbeStatic) != 0;

			// 用于检查游戏对象（GameObject）是否处于激活状态。
			activeSelf = obj.activeSelf;
			// 定义 GameObject 在 Scene 中是否处于活动状态。
			activeInHierarchy = obj.activeInHierarchy;
			
			//var prefType = PrefabUtility.GetPrefabType(obj);
			//prefabType = prefType.ToString();
			prefabRootId = resMap.getRootPrefabId(obj, true);
			prefabObjectId = resMap.getPrefabObjectId(obj, true);
			prefabInstance = Utility.isPrefabInstance(obj) || Utility.isPrefabModelInstance(obj);
			//prefabInstance = (prefType == PrefabType.PrefabInstance) || (prefType == PrefabType.ModelPrefabInstance);			
			// 渲染相关信息
			renderer = JsonRendererData.makeRendererArray(obj.GetComponent<Renderer>(), resMap);
			light = JsonLight.makeLightArray(obj.GetComponent<Light>());
			reflectionProbes = 
				ExportUtility.convertComponents<ReflectionProbe, JsonReflectionProbe>(obj,
					(c) => new JsonReflectionProbe(c, resMap)
				);
			terrains = 
				ExportUtility.convertComponents<Terrain, JsonTerrain>(obj,
					(c) => new JsonTerrain(c, resMap)
				);
				
			skinRenderers = 
				ExportUtility.convertComponentsList<SkinnedMeshRenderer, JsonSkinRendererData>(obj, 
					(c) => new JsonSkinRendererData(c, objMap, resMap)
			);

			int colliderIndex = 0;
			colliders = ExportUtility.convertComponentsList<Collider, JsonCollider>(
				obj, (arg) => new JsonCollider(arg, colliderIndex++, resMap)
			).Where(c => c.isSupportedType()).ToList();

			for(int i = colliders.Count - 1; i >= 0; i--){
				var curCollider = colliders[i];
				if ((curCollider == null) || !curCollider.isSupportedType()){
					colliders.RemoveAt(i);
				}
			}

			rigidbodies = ExportUtility.convertComponentsList<Rigidbody, JsonRigidbody>(
				obj, (arg) => new JsonRigidbody(arg)
			);

			joints = ExportUtility.convertComponentsList<Joint, JsonPhysicsJoint>(
				obj, (arg) => new JsonPhysicsJoint(arg)
			);

			/*
			if (rigidbodies.Count > 1){
				//Logger.log
			}
			*/
					
			///..... I think those can be replaced with linq queries (-_-)
			/*
			animators = obj.GetComponents<Animator>().Where((arg) => arg)
				.Select((Animator arg) => new JsonAnimator(arg, resMap))
				.ToList();*/
				
			animators = 
				ExportUtility.convertComponentsList<Animator, JsonAnimator>(obj, 
					(c) => new JsonAnimator(c, resMap));
					
			// 读取网格信息，并向resMap中查询或者注册网格信息
			var meshFilter = obj.GetComponent<MeshFilter>();
			if (meshFilter){
				mesh = resMap.getOrRegMeshId(meshFilter);
			}

			foreach(Transform curChild in obj.transform){
				var childId = objMap.getId(curChild.gameObject); 
				/* ????
				if (childId < 0){
					//throw new System.ArgumentException("Could not find child id
				}
				*/
				if (!childId.isValid){					
				}
				//var childId = objMap.getId(curChild.gameObject); 
				children.Add(childId);
				childNames.Add(curChild.name);
			}
			if (obj.transform.parent){
				parentName = obj.transform.parent.name;
				parent = objMap.findId(obj.transform.parent.gameObject);
			}
		}
	}
}
