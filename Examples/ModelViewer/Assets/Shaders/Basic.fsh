#version 430 core

in vec3 v_color;
in vec2 v_texture;

uniform sampler2D u_texture;

out vec4 fragColor;

void main()
{
  fragColor = texture(u_texture, v_texture).a *
              vec4(v_color.r, v_color.g, v_color.b, 1.0);
}
