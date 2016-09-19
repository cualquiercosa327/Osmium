#pragma once
#include <Core/Component.h>
#include <Core/Resource.h>
#include <unordered_map>

namespace Osm
{

class Resources
{
public:
	Resources() {}

	~Resources() { /* TODO: Make sure there are no resources */ }

	template<typename T, typename... Args>
	T* LoadResource(Args... args);

	void ReleaseResource(Resource* res);

protected:

	template<typename T>
	T* GetLoadedResource(ullong id);

	/// Where the resources are
	std::string										_resourcesPath;

	/// A map of resources
	std::unordered_map<ullong, Resource*>			_resources;

	/// A map of reference counters
	std::unordered_map<ullong, uint>				_refCounters;
};

extern Resources* pResources;

template<typename T, typename... Args>
T* Resources::LoadResource(Args... args)
{	
	ullong id = T::CalculateResourceID(args...);

	T* resource = GetLoadedResource<T>(id);
	if(resource)
		return resource;

	resource = new T(args...);
	_resources.insert(std::make_pair(id, resource));
	_refCounters.insert(std::make_pair(id, 1));
	resource->_resourceID = id;
	return resource;
}

template <typename T>
T* Resources::GetLoadedResource(ullong id)
{
	auto it = _resources.find(id);

	// Check cache
	if (it != _resources.end())
	{
		++_refCounters[id];
		return dynamic_cast<T*>(it->second);
	}

	return nullptr;
}

}
