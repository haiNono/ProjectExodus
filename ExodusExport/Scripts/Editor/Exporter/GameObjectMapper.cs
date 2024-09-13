using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class GameObjectMapper: ObjectMapper<GameObject>{
		void gatherObjectIds(Queue<GameObject> objects){
			while(objects.Count > 0){
				var curObject = objects.Dequeue();
				if (!curObject)
					continue;
				/*var curId = */
				getId(curObject);//this creates id for an object
				// 这个语句用来遍历curObject.transform 的所有子 Transform 对象
				foreach(Transform curChild in curObject.transform){
					if (!curChild)
						continue;
					// curChild.gameObject获取与该 Transform 关联的 GameObject 实例
					if (!curChild.gameObject)
						continue;
					objects.Enqueue(curChild.gameObject);
				}
			}
		}
		
		public void gatherObjectIds(GameObject obj){
			var objQueue = new Queue<GameObject>();
			objQueue.Enqueue(obj);
			gatherObjectIds(objQueue);
		}
		
		public void gatherObjectIds(GameObject[] objs){
			var objQueue = new Queue<GameObject>();
			foreach(var cur in objs){
				objQueue.Enqueue(cur);	
			}
			gatherObjectIds(objQueue);
		}
	}	
}
