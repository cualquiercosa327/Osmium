#include <Core/World.h>
#include <Core/Entity.h>
#include <Core/Transform.h>
#include <imgui.h>
#include <algorithm>
#include <Utils.h>

using namespace std;
using namespace Osm;

World::~World()
{
}

void Osm::World::RemoveEntity(Entity * e)
{
	_removeQueue.insert(e);
}

void World::Update(float dt)
{
	// Add entities
	for (auto& e : _addQueue)
		_entities.push_back(move(e));
	_addQueue.clear();

	// Update entites
	for (auto& e : _entities)
		e->Update(dt);

	while (_removeQueue.size() > 0)
	{
		Entity* r = *_removeQueue.begin();
		_removeQueue.erase(_removeQueue.begin());
		auto toRemove = remove_if(_entities.begin(), _entities.end(), [this, r](unique_ptr<Entity>& entity)
		{
			return r == entity.get();
		});
		_entities.erase(toRemove, _entities.end());
	}
}

void World::PostUpdate(float dt)
{
	// Update entites
	for (auto& e : _entities)
		e->PostUpdate(dt);
}

void World::Render()
{
}

void Osm::World::Clear()
{
	_entities.clear();
	_addQueue.clear();
	_removeQueue.clear();
}


Entity* World::GetEntityByID(uint id)
{
	for (auto& e : _entities)
	{
		if (e->GetID() == id)
			return e.get();
	}
	return nullptr;
}

#ifdef INSPECTOR

void InspectEntity(Entity* entity, set<Entity*>& inspected, uint& selected)
{ 
	if(inspected.find(entity) != inspected.end())
		return;	

	static ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow; // | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	string name = entity->GetName();
	if (name.empty())
		name = typeid(*entity).name();
	name = StringReplace(name, "class ", "");
	name += " - ID:" + to_string(entity->GetID());

	auto trn = entity->GetComponent<Transform>();
	if (trn)
	{
		//if (childern.size() > 0)
		//{
		void* id = &entity;
		if (ImGui::TreeNodeEx(id, node_flags, name.c_str()))
		//if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::PushID((int)selected);
			selected = entity->GetID();

			auto childern = trn->GetChildern();
			for (auto child : childern)
			{
				auto e = &child->GetOwner();
				InspectEntity(e, inspected, selected);
			}
			ImGui::PopID();
			ImGui::TreePop();			
		}			
		//}
		//else
		//{
		//	ImGui::Selectable(name.c_str());
		//}
	}
}

void World::Inspect()
{
	for (auto& c : _components)
	{
		string name = typeid(*c).name();
		name = StringReplace(name, "class ", "");
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			c->Inspect();
		}
	}

	ImGui::Separator();

	// vector<bool> selected(_entities.size(), false);
	// static uint Id = -1;
	static uint selectedId = -1;

	/*
	set<Entity*> inspected;	
	for (size_t i = 0; i < _entities.size(); i++)
	{
		InspectEntity(_entities[i].get(), inspected, selectedId);
	}
	*/


	for(size_t i = 0; i < _entities.size(); i++)
	{
		auto& e = _entities[i];
		string name = e->GetName();
		if (name.empty())
			name = typeid(*e).name();
		name = StringReplace(name, "class ", "");
		name += " - ID:" + to_string(e->GetID());

		if(ImGui::Selectable(name.c_str()))
		{
			selectedId = e->GetID();
		}
	}

	if (selectedId != -1)
	{
		Entity* e = GetEntityByID(selectedId);
		if (e)
		{
			ImGui::Begin("Entity Inspector");
			ImGui::Text("Name: %s", e->GetName().c_str());
			ImGui::Text("ID: %d", e->GetID());
			ImGui::Separator();
			e->Inspect();
			ImGui::End();
		}
	}
}
#endif