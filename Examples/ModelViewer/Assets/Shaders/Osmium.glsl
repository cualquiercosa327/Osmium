// Geneal shader include file

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
};

// Uniforms
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_modelViewProjection;
uniform int u_directionalLightsCount;
uniform int u_pointLightsCount;
uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_eyePos;
uniform float u_fogFar;
uniform float u_fogNear;
uniform float u_fogExp;
uniform float u_time;
uniform vec4 u_fogColorNear;
uniform vec4 u_fogColorFar;

const float kRimGamma = 4.0;
const float kLightRadius = 5.2;

// Lights
#define DIR_LIGHT_COUNT     5
#define POINT_LIGHT_COUNT   10

// All the directional lights
uniform DirectionalLight    u_directionalLights[DIR_LIGHT_COUNT];
// All the point lights
uniform PointLight          u_pointLights[POINT_LIGHT_COUNT];

vec3 CalculateDirectionalLights(in vec3 normal)
{
  vec3 color = vec3(0.0, 0.0, 0.0);

  for(int i = 0; i < u_directionalLightsCount && i < DIR_LIGHT_COUNT; i++)
  {
    vec3 light_dir = u_directionalLights[i].direction;
    float intensity = max(0.0, dot(normal, light_dir));
    color += u_directionalLights[i].color * intensity * u_diffuse;
  }

  return color;
}

vec3 CalculatePointLights(in vec3 position, in vec3 normal)
{
  vec3 color = vec3(0.0, 0.0, 0.0);

  for(int i = 0; i < u_pointLightsCount && i < POINT_LIGHT_COUNT; i++)
  {
    vec3 light_dir = u_pointLights[i].position - position;
    float dist = length(light_dir);
    light_dir = normalize(light_dir);
    float intensity = max(0.0, dot(normal, light_dir));
    float attenuation = 1.5 / (1.0 + (dist / kLightRadius));
    color = u_pointLights[i].color  * intensity * attenuation * u_diffuse;
  }

  return color;
}

vec3 CalculateDirectionalLightsSpecular(
  in vec3 normal,
  in vec3 position,
  in float shininess)
{
  vec3 color = vec3(0.0, 0.0, 0.0);

  for(int i = 0; i < u_directionalLightsCount && i < DIR_LIGHT_COUNT; i++)
  {
    vec3 lightDir = u_directionalLights[i].direction;
    float diffuse = max(0.0, dot(normal, lightDir));
    color += u_directionalLights[i].color * diffuse * u_diffuse;

    vec3 viewDir = normalize(u_eyePos - position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    color += u_directionalLights[i].color * specular;
  }

  return color;
}

vec3 CalculateRimLight(in vec3 position, in vec3 normal)
{
  vec3 vPos = (u_view * u_model * vec4(position, 1.0)).xyz;
  vec3 toEye = normalize(-vPos);
  vec3 vNormal =  normalize((u_view * u_model * vec4(normal, 0.0)).xyz);
  float rim = 1 - max(0.0, dot(vNormal, toEye));
  rim = pow(rim, kRimGamma);
  return vec3(u_diffuse * rim);
}
