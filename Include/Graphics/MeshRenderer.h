#pragma once

#include <Render.h>
#include <Mesh.h>

namespace igad
{

class LightShaderParameter;

///
/// MeshRenderer
///
class MeshRenderer : public Renderable
{
public:
	const int kMaxDirecationalLights = 5;
	const int kMaxPointLights = 5;

public:

	MeshRenderer(Entity& entity);

	virtual ~MeshRenderer();

	Mesh* GetMesh() { return _mesh; }

	void SetMesh(Mesh* mesh) { _mesh = mesh; }

	virtual void SetShader(Shader* shader) override;

	GLuint GetVAO() { return _vao; }

	Texture* GetTexture() { return _texture; }

	void SetTexture(Texture* texture) { _texture = texture; }

	Color GetDiffuse() const { return _diffuse; }

	void SetDiffuse(Color diffuse) { _diffuse = diffuse; }

	Color GetAmbient() const { return _ambient; }

	void SetAmbient(Color ambient) { _ambient = ambient; }

	virtual	void ActivateShader(
		const Matrix44& view,
		const Matrix44& proj,
		const std::vector<Light*> lights) override;

	virtual void Draw() override;

protected:
	GLuint	_vao = 0;
	Mesh*	_mesh = nullptr;
	Texture* _texture = nullptr;
	ShaderParameter* _projParam = nullptr;
	ShaderParameter* _modelParam = nullptr;
	ShaderParameter* _viewParam = nullptr;
	ShaderParameter* _textureParam = nullptr;
	ShaderParameter* _directionaLightsCountParam = nullptr;
	ShaderParameter* _pointLightsCountParam = nullptr;
	ShaderParameter* _modelViewProjParam = nullptr;
	ShaderParameter* _modelViewParam = nullptr;
	ShaderParameter* _eyePosParam = nullptr;
	ShaderParameter* _diffuseParam = nullptr;
	ShaderParameter* _ambientParam = nullptr;
	ShaderAttribute* _positionAttrib = nullptr;
	ShaderAttribute* _normalAttrib = nullptr;
	ShaderAttribute* _textureAttrib = nullptr;
	std::vector<std::unique_ptr<LightShaderParameter>> _dirLightParams;
	std::vector<std::unique_ptr<LightShaderParameter>> _pointLightParams;

	Transform* _transform = nullptr;
	Matrix44 _viewMatrix;
	Matrix44 _projectionMatrix;
	Color _diffuse;
	Color _ambient;
};

///
/// A shortcut to setting up lights quickly
///
class LightShaderParameter
{
public:

	/// Create a shader
	LightShaderParameter(Shader* shader, const string& name);

	/// Set a light. The type will be set from the type
	void SetValue(const Light& light);

protected:
	ShaderParameter* _positionParam;
	ShaderParameter* _directionParam;
	ShaderParameter* _colorParam;
	ShaderParameter* _attenuationParam;
};

}