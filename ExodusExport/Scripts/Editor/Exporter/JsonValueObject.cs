using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	// IFastJsonValue是一个接口，里面只有一个方法
	public class JsonValueObject: IFastJsonValue{
		virtual public void writeJsonObjectFields(FastJsonWriter writer){
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			//writer.beginObjectValue();
			writer.beginRawObject();
			writeJsonObjectFields(writer);
			writer.endObject();
		}
	}
}