using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	// 构建和写入 JSON 格式的数据，分别用beginRawObject、beginRawArray、writeKey写入json中的对象、数组、键名
	public partial class FastJsonWriter{
		public int indent = 0;
		//  StringBuilder 提供了一种可变的字符串对象
		public StringBuilder builder = new StringBuilder();
		public Stack<int> valCount = new Stack<int>();
		
		public delegate void RawValueWriter<Value>(Value val);
		public delegate void RawStaticValueWriter<Value>(FastJsonWriter writer, Value val);
		// 返回构建的 JSON 字符串
		public string getString(){
			return builder.ToString();
		}
		// 根据当前的 indent 值向 StringBuilder 中添加缩进字符
		public void writeIndent(){
			for (int i = 0; i < indent; i++)
				builder.Append("\t");
		}

		public void beginDocument(){
			beginRawObject();
		}

		public void endDocument(){
			endObject();
		}
		// 处理逗号的插入，确保在 JSON 对象或数组中正确地插入逗号
		public void processComma(){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0)
				builder.AppendLine(",");
			writeIndent();
		}
		
		public void processComma(bool indent){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0){
				if (indent)
					builder.AppendLine(",");
				else
					builder.Append(", ");
			}
			if (indent)
				writeIndent();
		}
		// 开始一个新的 JSON 对象，增加缩进并在 StringBuilder 中添加{
		public void beginRawObject(){
			builder.AppendLine("{");
			indent++;
			valCount.Push(0);
		}

		public void beginRawObject(bool newLine){
			if (newLine)
				builder.AppendLine("{");
			else			
				builder.Append("{");
			indent++;
			valCount.Push(0);
		}
		// 结束当前的 JSON 对象，减少缩进并在 StringBuilder 中添加 }
		public void endObject(bool writeLineIndent){
			indent--;
			if (writeLineIndent){
				builder.AppendLine();
				writeIndent();
			}
			builder.Append("}");
			valCount.Pop();
		}

		public void endObject(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("}");
			valCount.Pop();
		}

		public void beginRawArray(){
			builder.AppendLine("[");
			indent++;
			valCount.Push(0);
		}

		public void endArray(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("]");
			valCount.Pop();
		}

		public void writeKey(string key, bool indent){
			processComma(indent);
			writeString(key);
			builder.Append(": ");
		}
		
		public void writeKey(string key){
			writeKey(key, true);
		}
		
		public void beginKeyArray(string key){
			writeKey(key);
			beginRawArray();
		}

		public void beginKeyObject(string key){
			writeKey(key);
			beginRawObject();
		}
	}
}