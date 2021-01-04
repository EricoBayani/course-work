precision mediump float;

attribute vec4 a_Position;
attribute vec4 a_Color;
attribute vec3 a_Place;
varying vec4 v_Color;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

uniform mat4 u_NormalMatrix;

attribute vec2 a_TexCoord;
varying vec2 v_TexCoord;

attribute vec3 a_Normal;
varying vec3 v_Normal;
varying vec4 v_NormalDebug;

uniform vec3 u_LightPos;
varying vec3 v_LightPos;

varying vec4 v_WorldPos;
uniform vec3 u_CameraPos;
varying vec3 v_CameraPos;


void main() {
  vec4 combinedPosition = vec4(a_Place,1) + a_Position;
  // Set the vertex coordinates of the point
  gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * combinedPosition;
  v_Color = a_Color;
  v_TexCoord = a_TexCoord;
  v_Normal = vec3(u_NormalMatrix * (vec4(a_Normal, 0)) + combinedPosition);
  v_NormalDebug = vec4(a_Normal, 1);
  v_LightPos = u_LightPos;
  v_CameraPos = u_CameraPos;
  v_WorldPos = u_ModelMatrix * combinedPosition;
}
