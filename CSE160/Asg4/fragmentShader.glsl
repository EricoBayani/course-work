precision mediump float;
varying vec4 v_Color;

uniform int u_WhichTex;
uniform sampler2D u_SamplerWall;
uniform sampler2D u_SamplerSky;
uniform sampler2D u_SamplerGround;
varying vec2 v_TexCoord;
uniform float u_TexColorWeight;

varying vec3 v_Normal;
uniform bool u_NormalViewOn;
varying vec4 v_NormalDebug;

varying float v_Lighting;
varying vec3 v_LightPos;
varying vec3 v_CameraPos;
varying vec4 v_WorldPos;
uniform bool u_LightingOn;

void main() {
  vec4 texColorWall = texture2D(u_SamplerWall, v_TexCoord);
  vec4 texColorSky = texture2D(u_SamplerSky, v_TexCoord);
  vec4 texColorGround = texture2D(u_SamplerGround, v_TexCoord);
  vec4 texColor;

  if(u_WhichTex ==  0)
    texColor = texColorWall;
  else if(u_WhichTex ==  1)
    texColor = texColorSky;
  else if(u_WhichTex ==  2)
    texColor = texColorGround;

  vec4 finalColor = (u_TexColorWeight * texColor) + ((1.0 - u_TexColorWeight) * v_Color);
  if (u_NormalViewOn == true)    
    finalColor = abs(v_NormalDebug);

  vec4 cameraPos = vec4(v_CameraPos, 1.0);
  /* vec4 worldPos = vec4(v_WorldPos, 1.0); */
  if(u_LightingOn){
    // calculator the lighting things    
    vec4 lightVector = v_WorldPos + vec4(v_LightPos,1);
    vec4 normLightPos = normalize(lightVector);
    vec4 normWorldPos = normalize(vec4(v_Normal, 0.0));
    float lighting = max(dot(normLightPos, normWorldPos),0.0);
    vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);

    
    vec4 diffuse = finalColor * max(dot(normLightPos, normWorldPos),0.0);

    float specExp = 2.0;
    vec4 specColor = vec4(0.8, 0.8,0.8, 1.0);
    vec4 normCameraPos = normalize(cameraPos + v_WorldPos);
    vec4 bisectorVector = ((normLightPos + normCameraPos))/length(normLightPos + normCameraPos);
    float specValue = pow(max(dot(normWorldPos,bisectorVector),0.0),specExp);
    vec4 specular = specColor * specValue;

    
    
    if(u_WhichTex == 1){
      gl_FragColor = finalColor;
    }
    else{
      /* float distanceFromLight = 0.5*(max(distance(cameraPos,v_WorldPos),2.0)); */
      /* gl_FragColor = (ambient + diffuse + specular) / distanceFromLight; */
      gl_FragColor = (ambient + diffuse + specular);
    }
  }
  else {
    gl_FragColor = finalColor;
  }
}
