#include <AI/Steering.h>
#include <Graphics/DebugRenderer.h>
#include <imgui.h>
#include <Core/World.h>
#include <Core/Game.h>

using namespace Osm;

Steering::Steering(Entity& entity)
	: Component(entity)
	, _flags(STEERING_NONE)
{
	_physicsManager = GetOwner().GetWorld().GetComponent<PhysicsManager2D>();
	_physicsBody = entity.GetComponent<PhysicsBody2D>();
}

Vector2 Steering::GetSteering()
{
	CalculatePrioritized();

	Vector2 prev = _current;

#if DEBUG_RENDER
	DebugRender();
#endif

	_current = Lerp(prev, _current, 0.1f);
	//_current = Lerp(prev, _current, 0.9f);

	return _current;
}

bool Steering::AccumulateForce(Vector2 add)
{
	// Calculate how much steering force the vehicle has used so far
	float  magnitudeSoFar = _current.Magnitude();

	// Calculate how much steering force remains to be used by this vehicle
	float magnitudeRemaining = MaxForce - magnitudeSoFar;

	// Return false if there is no more force left to use
	if (magnitudeRemaining <= 0.0) return false;

	// Calculate the magnitude of the force we want to add
	double magnitudeToAdd = add.Magnitude();

	// If the magnitude of the sum of ForceToAdd and the running total
	// does not exceed the maximum force available to this vehicle, just
	// add together. Otherwise add as much of the ForceToAdd vector is
	// possible without going over the max.
	if (magnitudeToAdd < magnitudeRemaining)
	{
		_current += add;
	}
	else
	{
		// Add it to the steering force
		add.Normalize();
		_current += add * magnitudeRemaining;
	}

	return true;
}

void Steering::CalculateWeightedSum()
{
}

void Steering::CalculatePrioritized()
{
	_current.Clear();
	vector<PhysicsBody2D*> neighbors;
	if (IsOn(STEERING_SEPARATION) || IsOn(STEERING_ALIGNMENT) || IsOn(STEERING_COHESION))
	{
		Vector2 v = _physicsBody->GetPosition();
		neighbors = GetFlockingNeighbors();
	}

	if (!_physicsManager->IsPhysicsBodyValid(Agent))
		Agent = nullptr;

	Vector2 force;

	if(IsOn(STEERING_OBSTACLE_AVOIDANCE))
	{
		force = ObstacleAvoidance() * ObstacleAvoidanceWeight;
		//force = ObstacleAvoidance2() * ObstacleAvoidanceWeight;
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_SEEK))
	{
		force = Seek(Target) * SeekWeight;
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_ARRIVE))
	{
		force = Arrive(Target, ArriveAcceleration)* ArriveWeight;
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_EVADE) && Agent)
	{
		force = Evade(Agent);
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_OFFSET_PURSUIT) && Agent)
	{
		force = OffsetPursuit(Agent, Offset) * OffsetPursuitWeight;
		if (!AccumulateForce(force)) return;
	}	
	if (IsOn(STEERING_SEPARATION))
	{
		force = Separation(neighbors) * SeparationWeight;
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_COHESION))
	{
		force = Cohesion(neighbors) * CohesionWeight;
		if (!AccumulateForce(force)) return;
	}	
	if (IsOn(STEERING_ALIGNMENT))
	{
		force = Alignment(neighbors) * AlignmentWeight;
		if (!AccumulateForce(force)) return;
	}
	if (IsOn(STEERING_WANDER))
	{
		force = Wander() * WanderWeight;
		if (!AccumulateForce(force)) return;
	}
}

Vector2 Steering::Seek(Vector2& targetPos)
{
	Vector2 seek = targetPos - _physicsBody->GetPosition();
	seek.Normalize();
	seek *= MaxAccelearation;
	return seek;
}

Vector2 Steering::Flee(Vector2& targetPos)
{
	Vector2 flee = targetPos - _physicsBody->GetPosition();
	flee.Normalize();
	flee *= MaxAccelearation;
	flee = flee - _physicsBody->GetVelocity();
	return flee;
}

Vector2 Steering::Arrive(Vector2& targetPos, float deceleration)
{
	Vector2 toTarget = Target - _physicsBody->GetPosition();

	Vector2 arrive;

	// Calculate the distance to the target
	float dist = toTarget.Magnitude();

	if (dist > 0)
	{
		// Because Deceleration is enumerated as an int, this value is required
		// to provide fine tweaking of the deceleration..
		const float DecelerationTweaker = 0.3f;

		// Calculate the speed required to reach the target given the desired
		// deceleration
		float speed = dist / (deceleration * DecelerationTweaker);

		// Make sure the velocity does not exceed the max
		speed = min(speed, MaxSpeed);

		// From here proceed just like Seek except we don't need to normalize 
		// the ToTarget vector because we have already gone to the trouble
		// of calculating its length: dist.
		Vector2 desiredVelocity = toTarget * speed / dist;

		arrive = (desiredVelocity - _physicsBody->GetVelocity());
	}

	return arrive;
}

Vector2 Steering::OffsetPursuit(const PhysicsBody2D* agent, const Vector2& offset)
{
	if (agent == nullptr)
		return Vector2();

	// Get vehicle coordinate frame
	Vector2 target = agent->GetToWorld(offset);		

	// If the evader is ahead and facing the agent then we can just seek
	// for the evader's current position.
	Vector2 toTarget = target - _physicsBody->GetPosition();

	gDebugRenderer.AddLine(DebugRenderer::AI, ToVector3(target), ToVector3(_physicsBody->GetPosition()));

	// The lookahead time is propotional to the distance between the evader
	// and the pursuer; and is inversely proportional to the sum of the
	// agent's velocities
	float LookAheadTime = toTarget.Magnitude() /
		(MaxSpeed + agent->GetVelocity().Magnitude()) * 0.1f;

	// Now seek to the predicted future position of the evader
	target = target + agent->GetVelocity() * LookAheadTime;

	//gDebugRenderer.AddCircle(ToVector3(target));

	return Seek(target);
}

Vector2 Steering::Evade(const PhysicsBody2D* agent)
{
	// Not necessary to include the check for facing direction this time
	Vector2 toPursuer = agent->GetPosition() - _physicsBody->GetPosition();

	//uncomment the following two lines to have Evade only consider pursuers 
	//within a 'threat range'
	if (toPursuer.SquareMagnitude() > EvadeDistance * EvadeDistance)
		return Vector2();

	//the lookahead time is propotional to the distance between the pursuer
	//and the pursuer; and is inversely proportional to the sum of the
	//agents' velocities
	float lookAheadTime = toPursuer.Magnitude() / (MaxSpeed + agent->GetVelocity().Magnitude());
	lookAheadTime *= 0.1f;

	//now flee away from predicted future position of the pursuer
	Vector2 target = agent->GetPosition() + agent->GetVelocity() * lookAheadTime;
	return -1.0f * Flee(target);
}

Vector2 Steering::Wander()
{
	Vector2 fwd = _physicsBody->GetVelocity();
	if (fwd.SquareMagnitude() < 0.001)
		fwd = _physicsBody->GetForward();
	fwd.Normalize();
	Matrix33 transform;
	transform.SetTransform(
		fwd,
		Vector2(1.0f, 1.0f),
		_physicsBody->GetPosition());

	Vector2 wander;

	// This behavior is dependent on the update rate, so this line must
	// be included when using time independent framerate.
	float jitterThisTimeSlice = WanderJitter * Game.Time().ElapsedTime;

													   // First, add a small random vector to the target's position
	_wanderTarget += Vector2(
		RandInRange(-1.0f, 1.0f) * jitterThisTimeSlice,
		RandInRange(-1.0f, 1.0f) * jitterThisTimeSlice);

	// Reproject this new vector back on to a unit circle
	_wanderTarget.Normalize();

	//increase the length of the vector to the same as the radius
	//of the wander circle
	_wanderTarget *= WanderRadius;

	//move the target into a position WanderDist in front of the agent
	//Vector2 target = _wanderTarget + Vector2(0, WanderDistance);
	Vector2 target = _wanderTarget + Vector2(0, WanderDistance);

	//project the target into world space
	target = transform.TransformVector(target); // _physicsBody->GetToWorld(target);

	//Vector2 center = _physicsBody->GetToWorld(Vector2(0, WanderDistance));
	Vector2 center = transform.TransformVector(Vector2(0, WanderDistance));

	// gDebugRenderer.AddCircle(ToVector3(center), WanderRadius);
	// gDebugRenderer.AddCircle(ToVector3(target), 0.5f, Color::Yellow);

	// And steer towards it
	wander = target - _physicsBody->GetPosition();

	/*
	Vector2 wander;

	// This behavior is dependent on the update rate, so this line must
	// be included when using time independent framerate.
	float JitterThisTimeSlice = WanderJitter * 0.016f; // m_pVehicle->TimeElapsed();

	// First, add a small random vector to the target's position
	_wanderTarget += Vector2(	RandInRange(-1.0f, 1.0f) * JitterThisTimeSlice,
								RandInRange(-1.0f, 1.0f) * JitterThisTimeSlice);

	// Reproject this new vector back on to a unit circle
	_wanderTarget.Normalize();

	//increase the length of the vector to the same as the radius
	//of the wander circle
	_wanderTarget *= WanderRadius;

	//move the target into a position WanderDist in front of the agent
	//Vector2 target = _wanderTarget + Vector2(0, WanderDistance);
	Vector2 target = _wanderTarget + Vector2(0, WanderDistance);

	//project the target into world space
	target = _physicsBody->GetToWorld(target);

	Vector2 center = _physicsBody->GetToWorld(Vector2(0, WanderDistance));

	gDebugRenderer.AddCircle(ToVector3(center), WanderRadius);

	gDebugRenderer.AddCircle(ToVector3(target), 0.5f, Color::Yellow);

	// And steer towards it
	wander = target - _physicsBody->GetPosition();
	*/

	return wander;
}

Vector2 Steering::ObstacleAvoidance()
{
	float speed = _physicsBody->GetSpeed();

	// The detection box length is proportional to the agent's velocity
	float minDetectionBoxLength = ObstacleAvoidanceRaduis * 0.25f;
	float boxLength = (minDetectionBoxLength)+(speed / MaxSpeed) * minDetectionBoxLength;

	Vector2 v = _physicsBody->GetPosition();
	auto obstacles = _physicsManager->GetInRadius(v, boxLength);

	gDebugRenderer.AddCircle(
		DebugRenderer::AI,
		ToVector3(_physicsBody->GetPosition()),
		ObstacleAvoidanceRaduis,
		Color::Yellow);

	gDebugRenderer.AddCircle(
		DebugRenderer::AI,
		ToVector3(_physicsBody->GetPosition()),
		boxLength,
		Color::Yellow);

	// Early out
	if (obstacles.size() == 0)
		return Vector2();

	if (ObstacleTag != 0)
	{
		obstacles.erase(remove_if(obstacles.begin(),
			obstacles.end(),
			[this](PhysicsBody2D* b)
		{
			return (b == _physicsBody) || !CheckBitFlagOverlap(b->GetOwner().GetTag(), ObstacleTag);
			//return b == _physicsBody;
		}), obstacles.end());
	}

	// This will keep track of the closest intersecting obstacle (CIB)
	PhysicsBody2D* closestIntersectingObstacle = nullptr;

	// This will be used to track the distance to the CIB
	float distToClosestIP = FLT_MAX;

	// This will record the transformed local coordinates of the CIB
	Vector2 localPosOfClosestObstacle;

	for (auto ob : obstacles)
	{
		auto pos = ob->GetPosition();	

		// Calculate this obstacle's position in local space
		Vector2 localPos = _physicsBody->WorldToLocal(pos);		
			
		// If the local position has a negative x value then it must lay
		// behind the agent. (in which case it can be ignored)
		if (localPos.y >= 0)
		{			
			// If the distance from the x axis to the object's position is less
			// than its radius + half the width of the detection box then there
			// is a potential intersection.
			float expandedRadius = ob->GetRadius() + _physicsBody->GetRadius();

			if (fabs(localPos.x) < expandedRadius)
			{
				//if(_inspect)
				{
					gDebugRenderer.AddCircle(
						DebugRenderer::AI,
						//ToVector3(localPos),
						ToVector3(pos),
						ob->GetRadius(),
						Color::White);

					gDebugRenderer.AddCircle(
						DebugRenderer::AI,
						ToVector3(localPos),
						ob->GetRadius(),
						Color::Orange);
				}

				// now to do a line/circle intersection test. The center of the 
				// circle is represented by (cX, cY). The intersection points are 
				// given by the formula x = cX +/-sqrt(r^2-cY^2) for y=0. 
				// We only need to look at the smallest positive value of x because
				// that will be the closest point of intersection.
				float cX = localPos.x;
				float cY = localPos.y;

				// we only need to calculate the sqrt part of the above equation once
				float sqrtPart = sqrt(expandedRadius * expandedRadius - cX * cX);

				float ip = cY - sqrtPart;

				if (ip <= 0.0f)
				{
					ip = cY + sqrtPart;
				}

				// test to see if this is the closest so far. If it is keep a
				// record of the obstacle and its local coordinates
				if (ip < distToClosestIP)
				{
					distToClosestIP = ip;
					closestIntersectingObstacle = ob;
					localPosOfClosestObstacle = localPos;
				}
			}
		}
	}

	// if we have found an intersecting obstacle, calculate a steering 
	// force away from it
	Vector2 steeringForce;

	if (closestIntersectingObstacle)
	{
		auto pos = closestIntersectingObstacle->GetPosition();

		//if(_inspect)
		{
			gDebugRenderer.AddCircle(
				//ToVector3(localPos),
				DebugRenderer::AI,
				ToVector3(pos),
				closestIntersectingObstacle->GetRadius() * 1.2f,
				Color::Yellow);
		}

		// the closer the agent is to an object, the stronger the 
		// steering force should be
		float multiplier = 1.0f + (boxLength - localPosOfClosestObstacle.y) / boxLength;

		//calculate the lateral force
		steeringForce.x = (closestIntersectingObstacle->GetRadius() -
							localPosOfClosestObstacle.x)  * multiplier;

		// Apply a braking force proportional to the obstacles distance from
		// the vehicle. 
		const float BrakingWeight = 0.2f;

		steeringForce.y = (closestIntersectingObstacle->GetRadius() -
			localPosOfClosestObstacle.y) * BrakingWeight;
	}

	gDebugRenderer.AddLine(
		DebugRenderer::AI,
		ToVector3(_physicsBody->GetPosition()),
		ToVector3(_physicsBody->GetPosition()) + ToVector3(steeringForce), Color::Green);

	// finally, convert the steering vector from local to world space
	return _physicsBody->GetToWorldDirection(steeringForce);
}

Vector2 Steering::ObstacleAvoidance2()
{
	const Vector2& position = _physicsBody->GetPosition();
	const Vector2& velocity = _physicsBody->GetVelocity();
	Vector2 direction(velocity);
	direction.Normalize();
	Vector2 perp = direction.Perpendicular();

	Matrix33 local;
	local.SetUp(direction);
	local.SetRight(perp);
	float a = DegToRad(ObstacleSideFeelerSpread + 90.0f);

	vector<Vector2> feelers =
	{
		Vector2(0.0f, 1.0f),
		Vector2(cos(a), sin(a)),
		Vector2(0.0f, 1.0f)
	};
	feelers[2].x = -feelers[1].x;
	feelers[2].y = feelers[1].y;

	vector<float> lenghts =
	{
		ObstacleFrontFeelerLength,
		ObstacleSideFeelerLength,
		ObstacleSideFeelerLength
	};

	Intersection2D closestIntersection;	
	float closestDist = FLT_MAX;
	int closestIdx = -1;

	for (int i = 0; i < 3; i++)
	{
		Vector2 feeler = local.TransformNormal(feelers[i]);

		auto intersection = _physicsManager->RayIntersect(
			position,
			feeler,
			lenghts[i],
			ObstacleTag);

		if (intersection.IsValid())
		{
			if (intersection.Depth < closestDist)
			{				
				closestDist = intersection.Depth;
				closestIntersection = intersection;
				closestIdx = i;
			}			
		}
	}

	if (closestIntersection.IsValid())
	{
		gDebugRenderer.AddCircle(
			DebugRenderer::AI,
			ToVector3(closestIntersection.Position),
			2.0f,
			Color::Orange);

		float overshot = lenghts[closestIdx] - closestDist;

		gDebugRenderer.AddLine(
			DebugRenderer::AI,
			ToVector3(closestIntersection.Position),
			ToVector3(closestIntersection.Position + closestIntersection.Normal * overshot),
			Color::Orange);

		return closestIntersection.Normal * overshot;
	}

	return Vector2();
}

vector<PhysicsBody2D*> Steering::GetFlockingNeighbors()
{
	vector<PhysicsBody2D*> neighbors;
	Vector2 v = _physicsBody->GetPosition();
	neighbors = _physicsManager->GetInRadius(v, FlockingRadius);
	if (FlockingTag != 0)
	{
		neighbors.erase(
			remove_if(	neighbors.begin(), 
						neighbors.end(), 
						[this](PhysicsBody2D* b)
			{
				return b->GetOwner().GetTag() != FlockingTag;
			}),
			neighbors.end()
		);
	}
	return neighbors;
}

Vector2 Steering::Cohesion(const std::vector<PhysicsBody2D*>& agents)
{
	Vector2 cohesion;

	// First find the center of mass of all the agents
	Vector2 centerOfMass, steeringForce;

	size_t neighborCount = 0;

	// Iterate through the neighbors and sum up all the position vectors
	for(auto a : agents)
	{
		if(a != _physicsBody && a != Agent)
		{
			centerOfMass += a->GetPosition();
			neighborCount++;
		}
	}

	if (neighborCount > 0)
	{
		// The center of mass is the average of the sum of positions
		centerOfMass *= 1.0f / (float)neighborCount;

		//now seek towards that position
		cohesion = Seek(centerOfMass);
	}

	cohesion.Normalize();
	
	return cohesion;
}

Vector2 Steering::Separation(const std::vector<PhysicsBody2D*>& agents)
{
	Vector2 steeringForce;
	
	// iterate through the neighbors and sum up all the position vectors
	for (auto a : agents)
	{
		// make sure this agent isn't included in the calculations and that
		// the agent being examined is close enough
		if (a != _physicsBody && a != Agent)
		{
			Vector2 toAgent =_physicsBody->GetPosition() - a->GetPosition();

			// scale the force inversely proportional to the agents distance  
			// from its neighbor.
			float d = toAgent.Magnitude();
			if (d > 0.01)
			{
				toAgent.Normalize();
				steeringForce += toAgent / d;
			}
		}

	}

	return steeringForce;
}

Vector2 Steering::Alignment(const std::vector<PhysicsBody2D*>& agents)
{
	//This will record the average heading of the neighbors
	Vector2 averageHeading;

	//This count the number of vehicles in the neighborhood
	float neighborCount = 0.0;

	Vector2 currentHeading = _physicsBody->GetForward();
	currentHeading.Normalize();

	//iterate through the neighbors and sum up all the position vectors
	for (auto a : agents)
	{
		// make sure this agent isn't included in the calculations and that
		// the agent being examined is close enough
		if (a != _physicsBody && a != Agent)
		{
			Vector2 heading = a->GetForward();
			heading.Normalize();
			averageHeading += heading;
			neighborCount++;
		}

	}

	// If the neighborhood contained one or more vehicles, average their
	// heading vectors.
	if (neighborCount > 0.0)
	{
		averageHeading *= 1.0f / neighborCount;
		averageHeading -= currentHeading;
	}

	return averageHeading;
}

#if DEBUG_RENDER
void Steering::DebugRender()
{
	if (_inspect)
	{		
		auto pos = _physicsBody->GetPosition();
		/*
		gDebugRenderer.AddCircle(
			DebugRenderer::AI,
			ToVector3(pos),
			_physicsBody->GetRadius(),
			Color::Purple);
		*/
		gDebugRenderer.AddLine(
			DebugRenderer::AI,
			ToVector3(pos),
			ToVector3(pos + _current),
			Color::Orange);
		/*
		gDebugRenderer.AddCircle(
			DebugRenderer::AI,
			ToVector3(pos),
			FlockingRadius,
			Color::Purple);
		*/
	}
	_inspect = false;
}
#endif

#ifdef INSPECTOR
void Steering::Inspect()
{
	_inspect = true;

	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.4f);
	ImGui::InputFloat("Max Accelearation", &MaxAccelearation);
	ImGui::InputFloat("Max Force", &MaxForce);
	ImGui::InputFloat("Max Speed", &MaxSpeed);
	ImGui::InputFloat("Wander Jitter", &WanderJitter);
	ImGui::InputFloat("Wander Radius", &WanderRadius);
	ImGui::InputFloat("Wander Distance", &WanderDistance);

	ImGui::InputFloat("Seek Weight", &SeekWeight);

	// Obstacle avoidance
	ImGui::InputFloat("Obstacle Avoidance Weight", &ObstacleAvoidanceWeight);
	ImGui::InputFloat("Obstacle Avoidance Radius", &ObstacleAvoidanceRaduis);
	ImGui::InputFloat("Obstacle Avd Front Feeler Length", &ObstacleFrontFeelerLength);
	ImGui::InputFloat("Obstacle Avd Side Feeler Length", &ObstacleSideFeelerLength);
	ImGui::InputFloat("Obstacle Avd Side Feeler Spread", &ObstacleSideFeelerSpread);

	/*
	// Weights
	float FleeWeight = 1.0f;
	float ArriveWeight = 1.0f;
	float WanderWeight = 1.0f;
	float CohesionWeight = 1.0f;
	float SeparationWeight = 1.0f;
	float WallAvoidanceWeight = 1.0f;
	float PursuitWeight = 1.0f;
	float EvadeWeight = 1.0f;
	float FoollowPathWeight = 1.0f;
	float HideWeight = 1.0f;
	float OffsetPursuitWeight = 1.0f;
	float AlignmentWeight = 10.0f;
	*/

	float maxed = _current.Magnitude() / MaxSpeed;
	ImGui::ProgressBar(maxed);
	// ImGui::SameLine();
	//ImGui::LabelText("Total Used ","");	
	ImGui::PopItemWidth();
}
#endif

/*
PhysicsBody2D& SteeringBehaviour::GetPhysicsBody() const
{
	auto physics = GetOwner().GetPhysicsBody();
	ASSERT(physics);
	return *physics;
}

SteeringOutput Seek::GetSteering()
{
	SteeringOutput so;

	so.Linear = Target - GetPhysicsBody().GetPosition();
	so.Linear.Normalize();
	so.Linear *= GetOwner().MaxAccelearation;

	return so;
}
*/