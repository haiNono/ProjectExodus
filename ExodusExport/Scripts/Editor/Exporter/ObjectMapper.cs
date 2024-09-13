using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	// 主要作用是管理和映射资源对象，提供对资源的注册、验证和访问功能。它通常用于处理游戏对象或其他资源的 ID 和索引，以便在资源管理系统中进行有效的跟踪和操作。
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);
		// ResId：一个类，用来管理资源ID
		public Dictionary<Resource, ResId> objectMap = new Dictionary<Resource, ResId>();
		public List<Resource> objectList = new List<Resource>();
		// ResourceStorageWatcher类用来监视和管理资源存储的状态变化，特别是在处理可能存在循环引用的资源图时
		public ResourceStorageWatcher<ObjectMapper<Resource>, Resource> createWatcher(){
			return new ResourceStorageWatcher<ObjectMapper<Resource>, Resource>(
				this, 
				(obj) => obj.numObjects, 
				(obj, idx) => obj.getObjectByIndex(idx)
			);
		}

		public bool isValidObjectId(ResId id){
			return (id.objectIndex >= 0) && (id.objectIndex < objectList.Count);
		}
		
		public bool isValidObjectIndex(int index){
			return (index >= 0) && (index < objectList.Count);
		}
		
		public int numObjects{
			get{
				return objectList.Count;
			}
		}

		/*
		public Watcher makeWatcher(){
			return new Watcher(this);
		}
		*/

		public IEnumerable<ResId> getObjectIds(){
			return null;
		}
		
		public Resource getObject(ResId id){
			if (!isValidObjectId(id))
				throw new System.ArgumentException(string.Format("Invalid object id {0}", id));
			return objectList[id.rawId];
		}
		
		public Resource getObjectByIndex(int index){
			if ((index < 0) || (index >= objectList.Count))
				throw new System.ArgumentException(string.Format("Invalid object index {0}", index));
			return objectList[index];
		}

		public bool hasObject(Resource obj){
			return ExportUtility.isValidId(getId(obj, false));
		}

		public bool isValidId(ResId id){
			return (id.objectIndex >= 0) && (id.objectIndex < objectList.Count);
		}
			
		public ResId findId(Resource obj){
			return getId(obj, false, null);
		}
		
		public ResId getId(Resource obj){
			return getId(obj, true, null);
		}
		
		protected bool isValidObject(Resource obj){
			return !Object.Equals(obj, null);
		}
		// 在资源列表中新增
		protected ResId addNewObjectInternal(Resource obj, OnNewObjectAdded onAddCallback = null){
			if (objectMap.ContainsKey(obj))
				throw new System.ArgumentException("Logic error: duplicate registertation");
			var result = new ResId(objectList.Count);
			objectMap.Add(obj, result);
			objectList.Add(obj);
			
			if (onAddCallback != null)
				onAddCallback(obj);
			return result;						
		}
		// 注册一个新的资源对象，并返回其 ID
		public ResId registerObject(Resource obj, OnNewObjectAdded onAddCallback = null){
			return getId(obj, true, onAddCallback, false, false);			
		}
		// 获取资源id
		public ResId getId(Resource obj, bool createMissing, OnNewObjectAdded onAddCallback = null, bool throwIfMissing = false, bool throwIfExists = false){
			//int result = ExportUtility.invalidId;
			var result = ResId.invalid; // 构建一个id为-1的ResId实例
			if (!isValidObject(obj))
				return result;
				// TryGetValue返回布尔值，同时通过out定义的result会在函数内部被赋值
			if (objectMap.TryGetValue(obj, out result)){
				if (throwIfExists){
					throw new System.InvalidOperationException(
						string.Format("Object {0} already exists with id {1}", obj, result)
					);
				}
				return result;
			}
			if (!createMissing){
				if (throwIfMissing){
					throw new System.InvalidOperationException(
						string.Format("Could not find id for resource {0}", obj)
					);
				}
				return ResId.invalid;
			}
			
			return addNewObjectInternal(obj, onAddCallback);
		}
	}
}
