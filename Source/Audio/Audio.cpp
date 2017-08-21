#include <Audio/Audio.h>
#include <fmod_studio.hpp>
#include <Defines.h>
#include <fmod_errors.h>
#include <Core/Transform.h>
#include <Physics/Physics2D.h>
#include <imgui/imgui.h>

#define VERBOSE_LEVEL 4

using namespace Osm;
using namespace std;
using namespace FMOD::Studio;

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line)
{
	if (result != FMOD_OK)
	{
		LOG("%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString(result));
	}
}

FMOD_VECTOR Osm::VectorToFmod(const Vector3& vector)
{
	float f = 1.0f / 80.0f;
	FMOD_VECTOR v = { vector.x * f, vector.y * f, vector.z * f };
	return v;
}


FMOD_VECTOR Osm::VectorToFmod(const Vector2& vector)
{
	return VectorToFmod(ToVector3(vector));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudioManagerComponent
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AudioManager::AudioManager(CGame& owner) : Component(owner)
{	
	ERRCHECK(FMOD::Studio::System::create(&_studioSystem));
	ERRCHECK(_studioSystem->getLowLevelSystem(&_system));
	ERRCHECK(_system->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

	ERRCHECK(_studioSystem->initialize(
		128,
		FMOD_STUDIO_INIT_LIVEUPDATE,
		FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED,
		nullptr));
	ERRCHECK(_studioSystem->getLowLevelSystem(&_system));

#if VERBOSE_LEVEL > 0
	LOG("FMOD Studio System Initialization Complete.");
#endif

	float size = 80.0f;
	
	//_system->set3DSettings(1.0f, 1.0f, 0.05f);
	//_system->set3DSettings(1.0f / size, size, 1.0f / size);

	/*
	FMOD_VECTOR pos = { 0.0f, 20.0f, 0.0f };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR fwd = { 0.0f, 0.0f, 1.0f };
	FMOD_VECTOR fwd = { 0.0f, 0.0f, 1.0f };
	_system->set3DListenerAttributes(0, &pos, &vel,   )


	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = { 0.0f, 0.0f, 0.0f };
	attributes.velocity = { 0.0f, 0.0f, 0.0f };
	attributes.forward = { 0.0f, 0.0f, 1.0f };
	attributes.up = { 0.0f, 1.0f, 0.0f };

	_studioSystem->setListenerAttributes(0, &attributes);

	*/
	
}

AudioManager::~AudioManager()
{
	ERRCHECK(_studioSystem->unloadAll());
	ERRCHECK(_studioSystem->release());
}

void AudioManager::Update(float dt)
{
	// Update listener
	if(_listener)
	{		
		_listener->Update(dt);
		const auto& data =_listener->GetPositionalData();
		_studioSystem->setListenerAttributes(0, &data);
	}
	else
	{
		// Do nothing for now
	}

	// Update sources
	for (auto s : _sources)
	{
		s->Update(dt);	
		s->UpdatePositionalData();
	}

	// Update one-off sounds
	for (auto itr = _events.begin(); itr != _events.end(); )
	{
		EventInstance* instance = *itr;
		FMOD_STUDIO_PLAYBACK_STATE state;
		ERRCHECK(instance->getPlaybackState(&state));
		if (state == FMOD_STUDIO_PLAYBACK_STOPPED)
		{
			instance->release();
			itr = _events.erase(itr);
		}
		else
		{
			++itr;
		}
	}

	for (auto& itr : _descriptions)
	{
		itr.second.Cooldown -= dt;
	}

	ERRCHECK(_studioSystem->update());
}

void AudioManager::LoadBank(const std::string& bankName,
							FMOD_STUDIO_LOAD_BANK_FLAGS flags)
{
	auto foundIt = _banks.find(bankName);
	if (foundIt != _banks.end())
		return;

	FMOD::Studio::Bank* bank;
	ERRCHECK(_studioSystem->loadBankFile(bankName.c_str(), flags, &bank));
	if (bank) 		
		_banks[bankName] = bank;
}

void AudioManager::PlayEvent(const std::string& eventName, const Vector3& position)
{
	FMOD::Studio::EventDescription* description = nullptr;
	auto found = _descriptions.find(eventName);
	if (found != _descriptions.end())
	{
		description = found->second.Description;
		if (found->second.Cooldown > 0.0f)
			return;
	}
	else
	{
		ERRCHECK(_studioSystem->getEvent(eventName.c_str(), &description));
		_descriptions[eventName].Description = description;
	}
	
	if (description)
	{
		FMOD::Studio::EventInstance* instance = nullptr;
		ERRCHECK(description->createInstance(&instance));
		if (instance)
		{
			_events.push_back(instance);
			instance->start();
			_descriptions[eventName].Cooldown = 0.066f;
			bool is3D = false;
			ERRCHECK(description->is3D(&is3D));
			if (is3D)
			{
				FMOD_3D_ATTRIBUTES attributes;
				attributes.position =	VectorToFmod(position);
				attributes.forward =	{ 0.0f, 0.0f, 1.0f };
				attributes.up =			{ 0.0f, 1.0f, 0.0f };
				attributes.velocity =	{ 0.0f, 0.0f, 0.0f };
				instance->set3DAttributes(&attributes);
			}
		}
		else
		{
			LOG("Unable to create istance of sound event: %s", eventName.c_str());
		}
	}
	else
	{
		LOG("Unable to locate sound event description: %s", eventName.c_str());
	}
}

void AudioManager::LoadEvent(const std::string& eventName)
{
	/*
	auto found = _events.find(eventName);
	if (found != _events.end())
		return;

	FMOD::Studio::EventDescription* pEventDescription = nullptr;
	ERRCHECK(_studioSystem->getEvent(eventName.c_str(), &pEventDescription));
	if (pEventDescription)
	{
		FMOD::Studio::EventInstance* pEventInstance = nullptr;
		ERRCHECK(pEventDescription->createInstance(&pEventInstance));
		if (pEventInstance)
		{
			_events[eventName] = pEventInstance;
		}
	}
	*/
}

EventDescription* AudioManager::LoadDescription(const std::string& eventName)
{
	auto found = _descriptions.find(eventName);
	if (found != _descriptions.end())
		return found->second.Description;

	EventDescription* description = nullptr;
	ERRCHECK(_studioSystem->getEvent(eventName.c_str(), &description));
	_descriptions[eventName].Description = description;
	return description;
}

#ifdef INSPECTOR
void AudioManager::Inspect()
{
	size_t evCount = _events.size();
	ImGui::LabelText("Event Count", "%d", evCount);

	size_t srCount = _sources.size();
	ImGui::LabelText("Sources Count", "%d", srCount);

	EventDescription* des;
	char path[256];
	int retrieved;
	for (auto ev : _events)
	{
		ERRCHECK(ev->getDescription(&des));
		ERRCHECK(des->getPath(path, 256, &retrieved));
		ImGui::LabelText("Path", path);
	}
}
#endif


void AudioManager::Add(AudioSource* source)
{
	_sources.push_back(source);
}

void AudioManager::Remove(AudioSource* source)
{
	_sources.erase(remove(_sources.begin(), _sources.end(), source));
}

void AudioManager::Set(AudioListener* listener)
{
	_listener = listener;
}

void AudioManager::Remove(AudioListener* listener)
{
	_listener = nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudioManagerComponent
//
////////////////////////////////////////////////////////////////////////////////////////////////////
AudioManagerComponent::AudioManagerComponent(Entity & entity)
	: Component(entity)
{
	Init();
}


void AudioManagerComponent::Update(float dt)
{
	if (_body2D)
	{
		_attributes.position = VectorToFmod(_body2D->GetPosition());
		_attributes.forward = VectorToFmod(_body2D->GetForward());
		_attributes.velocity = VectorToFmod(_body2D->GetVelocity());
		_attributes.up = { 0.0f, 1.0f, 0.0f };
	}
	else if (_transform)
	{
		Vector3 prevPos(_attributes.position.x, _attributes.position.y, _attributes.position.z);
		Matrix44 world = _transform->GetWorld();
		_attributes.position = VectorToFmod(_transform->GetPosition());
		_attributes.forward = VectorToFmod(world.TransformDirectionVector(Vector3(0.0f, 0.0f, 1.0f)));
		_attributes.velocity = { 0.0f, 0.0f, 0.0f };
		_attributes.up = VectorToFmod(world.TransformDirectionVector(Vector3(0.0f, 1.0f, 0.0f)));
	}
}

void AudioManagerComponent::Init()
{
	_transform = GetOwner().GetComponent<Transform>();
	_body2D = GetOwner().GetComponent<PhysicsBody2D>();

	if (_transform && !_body2D)
	{
		_attributes.position = VectorToFmod(_transform->GetPosition());
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudioSource
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AudioSource::AudioSource(Entity& entity) : AudioManagerComponent(entity)
{
	Game.Audio().Add(this);
}

AudioSource::~AudioSource()
{
	Game.Audio().Remove(this);
}

bool AudioSource::LoadEvent(const std::string& eventPath)
{
	EventDescription* description = Game.Audio().LoadDescription(eventPath);
	if (description)
	{		
		ERRCHECK(description->createInstance(&_event));
		if (_event)
		{
			bool _is3D = false;
			ERRCHECK(description->is3D(&_is3D));

			return true;
		}
	}

	return false;
}

void AudioSource::Play() const
{		
	ERRCHECK(_event->start());
}

void AudioSource::SetVolume(float volume) const
{
	_event->setVolume(volume);
}

void AudioSource::SetPitch(float pitch) const
{
	_event->setPitch(pitch);
}

void AudioSource::SetParameter(const std::string& name, float value) const
{
	ERRCHECK(_event->setParameterValue(name.c_str(), value));
}

void AudioSource::UpdatePositionalData() const
{
	_event->set3DAttributes(&_attributes);
}

AudioListener::AudioListener(Entity& entity) : AudioManagerComponent(entity)
{
	Game.Audio().Set(this);
}

AudioListener::~AudioListener()
{
	Game.Audio().Remove(this);
}
