#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#include "JsonObjects.h"
#include "UnrealUtilities.h"

#include "Engine/SkeletalMesh.h"

#include "builders/LightBuilder.h"
#include "builders/ReflectionProbeBuilder.h"
#include "builders/TerrainComponentBuilder.h"
#include "builders/SkeletalMeshComponentBuilder.h"
#include "builders/GeometryComponentBuilder.h"

#include "DesktopPlatformModule.h"

/*
Notes:
* An object without an owner will be created as transient, that includes components.
* A transient object will poof eventually.
* In order to make component visible, it has to be registered in an Actor, so the actor creates render/physics state for it.
* In order to show up in unreal editor, object must be registered as an instance component.
* Changing outer to another object can be safely done when object is not registered yet. There's an owner check within the component, it throws an assert when owners mismatch.

Basically, we can totally create all objects as transient, and then alter owner, BUT, we'll need to take care of registering them and instance registering them.
Likewise the components can be created in vacuum, but if we want to see them, we msut register them, if they should be editable in hierarchy, they should be added as instance component,
and when they're registered, they can no longer be safely moved to another outer (can unregister them though).

Currently the system tries use correct outer right at the beginning, as a result the Outer is provided via a callback. This can be and likely should be simplified in the future.
没有owner的对象将被创建为瞬态，这包括组件。
瞬态对象最终会消失。
要使组件可见，它必须在 Actor 中注册，这样 Actor 才能为它创建渲染 / 物理状态。
要在 Unreal 编辑器中显示对象，必须将其注册为实例组件。
在对象尚未注册之前，更改其外部对象（Outer）是安全的。组件内部有一个所有者检查，当所有者不匹配时会抛出断言。
基本上，我们可以完全创建所有对象为瞬态，然后更改所有者，但是，我们需要处理注册它们，并将它们实例注册。
同样，组件可以在真空中创建，但如果我们想看到它们，我们必须注册它们，如果它们应该在层级中可编辑，它们应该被添加为实例组件，
并且当它们注册后，它们就不能再安全地移动到另一个外部对象（可以注销它们）。
目前的系统试图在一开始就使用正确的外部对象，结果是外部对象通过回调提供。这在未来可以也应该被简化。

*/

/*
We've ... actually I've run into a pickle.

It goes like this:
* Scene graph constructed from multiple actors cannot be automatically harvested into a blueprint.
* Harvesting objects into blueprint opens blueprint editor (troublesome with a lot of blueprints)
* Apparently skinned mesh can crash the whole process.

So, there's a need of major overhaul.

First, there's a category of situations where objects will need to be reconstructed as a single actor, with actor node being enabled for root.
This happens in case of prefabs, or prefab instances.

Question remains how to track non-prefab objects linked to prefab instances. But this can wait for later.

In a situation where we're dealing with a prefab instance, we'll have to create a root at the bottom AND spawn empty scene nodes as components.
As contents are no longer working as folders.

Additionally, we would want to track situations where there's a single object with a single component that spawns a specialized actor.
This is not strictly necessary, but desirable, so lights will be point light actors and not actors.

Additionally, there's trouble with ownership. It seems that creating component as part of transient package and then switching ownership 
to a non-transient package causes that component to disappear. I kinda wonder why, perhaps I missed a part where I update number of gc links or something.

Therefore, we need to create a blank actor IN ADVANCE, and use that blank actor as an owner to all the child components.

Additioanlly, previously discarded version where I attempted to create chidl components with specifying owners had a glitch where the components created this way were not properly registering in the scene.

So. I scrapped the whole branch, and started over.
我们遇到了一个难题。
情况是这样的：
由多个 Actor 构成的场景图不能自动地收集到蓝图中。
将对象收集到蓝图中会打开蓝图编辑器（对于大量的蓝图来说很麻烦）。
显然，骨骼网格可能会导致整个过程崩溃。
因此，需要进行一次重大的重构。
首先，存在一类情况，其中对象需要被重构为单一 Actor，且 Actor 节点被启用为根节点。
这种情况发生在预制件（prefabs）或预制件实例中。
问题仍然存在，如何追踪与预制件实例相关联的非预制件对象。但这个问题可以稍后再考虑。
在处理预制件实例的情况下，我们需要在底部创建一个根节点，并且生成空的场景节点作为组件。
因为内容不再作为文件夹工作。
此外，我们希望追踪那些具有单个组件并生成特定 Actor 的单个对象的情况。
这并不是严格必要的，但很理想，例如，光源将是点光源 Actor 而不是 Actor。
另外，所有权的问题也很棘手。看起来，将组件作为瞬态包的一部分创建，然后切换所有权到非瞬态包会导致该组件消失。
我有点好奇为什么会这样，可能我错过了更新垃圾回收链接数量的部分。
因此，我们需要提前创建一个空白 Actor，并将这个空白 Actor 作为所有子组件的所有者。
另外，我之前尝试创建子组件时指定了所有者，但存在一个缺陷，这样创建的组件没有在场景中正确注册。
所以，我放弃了整个分支，重新开始。

To summarize the rules:
* an object spawns actor instance, if it is not part of a prefab instance and has components.
* an objects spawns nothing, if it is not a part of a prefab instance, and has no components.
* an objects spawns components, if it is a part of prefab instance. 
* prefab instance has AActor as its base class, EXCEPT the situation where it is dealing with a single component object...? 
总结规则如下：
如果对象不是预制件实例的一部分，并且拥有组件，则会生成 Actor 实例。
如果对象不是预制件实例的一部分，并且没有组件，则不会生成任何东西。
如果对象是预制件实例的一部分，则会生成组件。
预制件实例以 AActor 为基类，除非它处理的是单个组件对象的情况...？
Hmm... (-_-)

Current plan - disable creation of prefabs.
Add handling of prefab instances according to those rules.
当前计划 - 禁用预制件的创建。
根据这些规则添加对预制件实例的处理。
*/

/*
*/
ImportedObject JsonImporter::importObject(const JsonGameObject &jsonGameObj, ImportContext &workData, bool createEmptyTransforms){
	using namespace UnrealUtilities;

	auto* parentObject = workData.findImportedObject(jsonGameObj.parentId);
	
	auto folderPath = workData.processFolderPath(jsonGameObj);
	UE_LOG(JsonLog, Log, TEXT("importing object: %d(%s), folder: %s, Num Components: %d"), 
		jsonGameObj.id, *jsonGameObj.name, *folderPath, jsonGameObj.getNumComponents());


	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return ImportedObject(); 
	}

	ImportedObjectArray createdObjects;

	bool rebuildPrefabsWithComponents = true;//a switch to disable prefab rebuilding functionality while I debug something else

	//auto objectType = DesiredObjectType::Default;
	/*
	When importing prefabs, child actor nodes are not being harvested correctly. They have to be rebuilt as component-only structures
	在导入预制件时，子 Actor 节点没有被正确地收集。它们必须被重建为仅包含组件的结构
	*/
	bool objectIsPrefab = jsonGameObj.usesPrefab();
	objectIsPrefab = objectIsPrefab && rebuildPrefabsWithComponents;

	bool mustCreateBlankNodes = objectIsPrefab || createEmptyTransforms;//prefab objects can have no folders, so we're going to faithfully rebuild node hierarchy.
	
	bool createActorNodes = jsonGameObj.isPrefabRoot() || !objectIsPrefab;
	//createActorNodes = true;

	//In situation where there's no parent, we have to create an actor. Otherwise we will have no valid outer
	// 是prefab的根组件并且不是prefab，同时没有父物体.即，只是一个座位容器的Actor
	createActorNodes = createActorNodes || !parentObject;

	//createActorNodes = true;
	bool rootMustBeActor = createActorNodes;
	// outer是指一个actor的根actor
	// findSuitableOuter从已导入的objects中找到当前jsonGameObj的根actor
	UObject *existingOuter = workData.findSuitableOuter(jsonGameObj);
	AActor *existingRootActor = nullptr;
	AActor *createdRootActor = nullptr;
	// [...]：方括号内的内容是捕获列表。在这里，& 表示按引用捕获外部作用域中的所有变量。这意味着在 Lambda 内部可以直接使用外部变量的引用，而不需要复制它们
	auto outerCreator = [&]() -> UObject*{
		if (existingOuter)
			return existingOuter;
		if (existingRootActor)
			return existingRootActor;
		if (!createdRootActor){
			createdRootActor = workData.createBlankActor(jsonGameObj, false);
		}
		check(createdRootActor != nullptr);
		return createdRootActor;
	};

	/*
	Here we handle creation of display geometry and colliders. This particular function call harvests colliders, reigidbody properties, builds them into a somewhat sensible hierarchy,
	and returns root object to us
	在这里，我们处理显示几何体和碰撞体的创建。这个特定的函数调用收集碰撞体、刚体属性，将它们构建成一个相对合理的层级，并返回根对象给我们。
	*/
	ImportedObject rootObject = GeometryComponentBuilder::processMeshAndColliders(workData, jsonGameObj, parentObject, folderPath, 
		!createActorNodes,
		//createActorNodes ? DesiredObjectType::Actor: objectType, 
		this, outerCreator);
	if (rootObject.isValid()){
		createdObjects.Add(rootObject);
		if (rootObject.hasActor()){
			//leaky abstractions, leaky abstractions everywhere....
			existingRootActor = rootObject.actor;
			check(existingRootActor != nullptr);
		}
	}

	// 76行至此，在创建当前JsonGameObject的rootObject 但是rootObject未必是有效的

	//Oh, I know. This si kinda nuts, but let's initialize root actor using lazy evaluation.

	/*
	The block below walks through every component type we currently support, and spawns unreal-side representation.
	*/
	// 反射探头
	if (jsonGameObj.hasProbes()){
		ReflectionProbeBuilder::processReflectionProbes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}
	
	if (jsonGameObj.hasLights()){
		LightBuilder::processLights(workData, jsonGameObj, parentObject, folderPath, &createdObjects, createActorNodes, outerCreator);
	}

	if (jsonGameObj.hasTerrain()){
		TerrainComponentBuilder::processTerrains(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}

	if (jsonGameObj.hasSkinMeshes()){
		SkeletalMeshComponentBuilder::processSkinMeshes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}
	// 146至此，处理actor的probes、lights、terrain和skinmesh

	/*
	At this point, one of the scenarios is true:

	1. Root actor has been created by mesh component. (existingRootActor set)
	2. Root actor has been created by actor creator and has no scene root component.
	3. Root actor hasn't been created, but outerPtr was located on request.

	So....

	It seems we only need to process the root normally, and an actor is created, but lacks root component, we set the root object as its component.
	在这一点上，可能存在以下几种情况：

1. 根演员已经由网格组件创建。（`existingRootActor` 已设置）
2. 根演员已经由演员创建器创建，但没有场景根组件。
3. 根演员尚未创建，但根据请求找到了外部指针。

所以……

看起来我们只需要正常处理根对象，如果演员已经创建但缺少根组件，我们将根对象设置为其组件。
	*/

	if (createdObjects.Num() > 1){
		if (!rootObject.isValid()){
			//More than one object exists, however, in case this is a mesh node, then the mesh actor is registered as a root by default.
			//rootObject = workData.createBlankActor(jsonGameObj);
			rootObject = workData.createBlankNode(jsonGameObj, createActorNodes, true, outerCreator);
			check(rootObject.isValid());
		}
	}
	else if (createdObjects.Num() == 1){
		//Oh. There  is a bug. In case we have a mesh with a single component parented to it, the check wil fail.
		//check(!rootObject.isValid());
		rootObject = createdObjects[0];
	}
	else if (createdObjects.Num() == 0){
		//No objects has been created. In this scenario, we do not make any nodes, unless requested.
		if (mustCreateBlankNodes){
			rootObject = workData.createBlankNode(jsonGameObj, createActorNodes, true, outerCreator);
				//workData.createBlankActor(jsonGameObj);
			check(rootObject.isValid());
		}
	}
	else{
		check(false);//This shouldn't happen, buuut....
	}
	// 194至此：根据不同情况，设置rootObject
	if (rootObject.isValid()){
		for (auto& cur : createdObjects){
			if (!cur.isValid())
				continue;
			if (cur == rootObject)
				continue;
			/*
			And then we parent created objects to the root object. The objects returned by previous methods do not form a hierarchy, 
			and are all treated as "sibling" nodes.
			*/
			setObjectHierarchy(cur, &rootObject, folderPath, workData, jsonGameObj);
		}

		if (rootMustBeActor && !rootObject.hasActor()){
			//whoops. Creating blank node.
			check(rootObject.hasComponent());
			check(createdRootActor != nullptr);//This HAS to be created by this point.
			//if (!)
			//auto blankNode = workData.createBlankActor(jsonGameObj, rootObject.component, true);
			//rootObject = blankNode;
			check(createdRootActor->GetRootComponent() == nullptr);
			createdRootActor->SetRootComponent(rootObject.component);

			auto blankNode = ImportedObject(createdRootActor);
			rootObject = blankNode;
		}

		workData.registerGameObject(jsonGameObj, rootObject);
		setObjectHierarchy(rootObject, parentObject, folderPath, workData, jsonGameObj);
		rootObject.setFolderPath(folderPath, true);

		rootObject.fixEditorVisibility();
		rootObject.convertToInstanceComponent();
		for (auto& cur : createdObjects){
			if (!cur.isValid() || (cur == rootObject))
				continue;
			cur.fixEditorVisibility();
			cur.convertToInstanceComponent();
		}
	}

	if (rootObject.isValid() && parentObject && parentObject->isValid()){
		//We need to change owner if the object tree was made without root actor. Otherwise it'll poof.
		if (!rootObject.hasActor()){
		}
	}

	/*
	Let's summarize collision approach and differences.

	Unity has separate rigidbody component.
	Unreal does not. Instead, random primitives can hold properties which are transferred to rigidbody.
	Unity has separate mesh collider component.
	Unreal does not.
	Also, apparently in unreal mesh may not have collision data generated?

	Unity creates compound bodiess by attaching colliders to the rigibody found.

	Unreal.... apparently can mere colliders down to some degree.

	So, what's the approach.

	In situation where phytsics controlled entities are present - or if there are collides present...
	Most likely resulting colliders should be joined into a hierarchy, where the first collider holds properties of the rigidbody from unity.
	The rest must be attached to it in order to be merged down as colliders.
	What's more, all the other components such as lights should be attached to that first "rigidbody" component, which will serve as root.

	So, in order to do it properly, we'll need to first build colliders. Then build static mesh.
	Then make a list out of them, and parent everything to the first collider then pray to cthulhu that this works as intended.
	The root component should also be set as actor root.

	Following components, such as probes, lights and whatever else are ALL going to be parented to that root physic component, whether they're actors or components themselves.

	This is ... convoluted.

	Let's see if I can, after all, turn rigibody into a component instead of relying on semi-random merges.
	让我们总结一下碰撞处理方法和差异。
	Unity 具有独立的刚体组件。
	Unreal 没有。相反，随机的几何体可以拥有属性，这些属性会被转移到刚体上。
	Unity 具有独立的网格碰撞器组件。
	Unreal 没有。
	另外，在 Unreal 中，网格可能不会生成碰撞数据？
	Unity 通过将碰撞器附加到找到的刚体上来创建复合体。
	Unreal... 显然可以将碰撞器降低到一定程度。
	那么，我们的方法是什么呢？
	在存在物理控制实体的情况下，或者如果有碰撞存在...
	可能产生的碰撞器应该被合并成一个层级结构，其中第一个碰撞器持有 Unity 中刚体的属性。
	其他的必须附加到它上面，以便作为碰撞器合并。
	更重要的是，所有其他组件，如灯光，都应该附加到那个第一个 “刚体” 组件上，它将作为根。
	因此，为了正确地做到这一点，我们需要首先构建碰撞器。然后构建静态网格。
	然后将它们列出来，并将一切都附加到第一个碰撞器上，然后祈祷这能按预期工作。
	根组件也应该被设置为 actor 的根。
	接下来的组件，如探针、灯光等，无论是 Actor 还是组件本身，都将被附加到那个根物理组件上。
	这... 是复杂的。
	让我们看看我是否能够，毕竟，将刚体转换为一个组件，而不是依赖于半随机的合并。
	*/

	/*
	This portion really doesn't mesh well with the rest of functionality. Needs to be changed.
	*/
	if (jsonGameObj.hasAnimators()){
		processAnimators(workData, jsonGameObj, parentObject, folderPath);

		/*
		We're creating blank hiearchy nodes in situation where the object is controlled by animator. This is to accomodate for unitys' floating bones...
		And this needs to be changed as well.
		*/
		if (!workData.importedObjects.Contains(jsonGameObj.id)){
			auto blankActor = workData.createBlankActor(jsonGameObj);
			setObjectHierarchy(blankActor, parentObject, folderPath, workData, jsonGameObj);
			workData.registerGameObject(jsonGameObj, blankActor);
		}
	}

	return rootObject;
}

USkeletalMesh* JsonImporter::loadSkeletalMeshById(ResId id) const{
	auto foundPath = skinMeshIdMap.Find(id);
	if (!foundPath){
		UE_LOG(JsonLog, Warning, TEXT("Could not load skin mesh %d"), id.toIndex());
		return nullptr;
	}

	auto result = LoadObject<USkeletalMesh>(nullptr, **foundPath);
	return result;
}

void JsonImporter::registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg){
	if (!outArray)
		return;
	if (!arg.isValid())
		return;
	outArray->Push(arg);
}

