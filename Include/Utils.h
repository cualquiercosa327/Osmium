#pragma once

#include <string>
#include <Vector3.h>
#include <Vector2.h>
#include <Matrix33.h>
#include <Matrix44.h>

namespace igad
{

/// Read an entire text file into a string. Only use for small files.
///
/// @filename   Full path to the file
///
std::string ReadFile(const std::string& filename);

/// Save text to a file
/// @filename   Full path to the file
/// @text       Text to save
///
bool SaveFile(const std::string& filename, const std::string& text);

/// Spherical coordinates defined with radius and two angles
struct SphericalCoordinates
{
	/// Radius
	float r;

	/// Inclination
	float theta;

	/// Azimuth
	float fi;
};

inline Vector3 ToVector3(const Vector2& vec2) { return Vector3(vec2.x, 0.0f, vec2.y); }

inline Vector2 ToVector2(const Vector3& vec2) { return Vector2(vec2.x, vec2.z); }

void UpdateMatrix44FromMatrix33(Matrix44& output, const Matrix33& input);

/// Convert a cartesian 3D vector to spherical coordinates
SphericalCoordinates CartesianToSpherical(const Vector3& cartesianVector);

/// Converts from spherical coordinates to cartesian
Vector3 SphericalToCartesian(const SphericalCoordinates& sphericalCoord);

Vector3 RandomOnUnitSphere();

}
