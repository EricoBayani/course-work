// Some code lifted from textbook
// Vertex shader program
var VSHADER_SOURCE = 
    'attribute vec4 a_Position;\n\
     attribute vec4 a_Color;\n\
     attribute vec3 a_Place;\n\
     varying vec4 v_Color;\n\
     \n\
     uniform mat4 u_ModelMatrix;\n\
     uniform mat4 u_ProjectionMatrix;\n\
     uniform mat4 u_ViewMatrix;\n\
     \n\
     attribute vec2 a_TexCoord;\n\
     varying vec2 v_TexCoord;\n\
     void main() {\n\
      vec4 combinedPosition = vec4(a_Place,1) + a_Position;\n\
      gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * combinedPosition;  // Set the vertex coordinates of the point\n\
      v_Color = a_Color;\n\
      v_TexCoord = a_TexCoord;\n\
    }\n';


// there's a uniform int variable that tells what texture unit to use: 0 is for wall, 1 is for sky, 2 is for ground
// Fragment shader program
var FSHADER_SOURCE =
    'precision mediump float;\n\
     varying vec4 v_Color;\n\
     uniform int u_WhichTex;\n\
     uniform sampler2D u_SamplerWall;\n\
     uniform sampler2D u_SamplerSky;\n\
     uniform sampler2D u_SamplerGround;\n\
     varying vec2 v_TexCoord;\n\
     uniform float u_TexColorWeight;\n\
     void main() {\n      \
      //gl_FragColor = v_Color; // Set the point color\n\
      vec4 texColorWall = texture2D(u_SamplerWall, v_TexCoord);\n\
      vec4 texColorSky = texture2D(u_SamplerSky, v_TexCoord);\n\
      vec4 texColorGround = texture2D(u_SamplerGround, v_TexCoord);\n\
      vec4 texColor; \n\
      \n\
      if(u_WhichTex ==  0) \n\
         texColor = texColorWall;\n\
      else if(u_WhichTex ==  1) \n\
         texColor = texColorSky;\n\
      else if(u_WhichTex ==  2)\n\
         texColor = texColorGround;\n\
      \n\
      gl_FragColor = (u_TexColorWeight * texColor) + ((1.0 - u_TexColorWeight) * v_Color);\n\
    }\n';
// == Globals ==

// Annoying constants
var red = [1.0, 0.0, 0.0];
var green = [0.0, 1.0, 0.0];
var blue = [0.0, 0.0, 1.0];
var pink = [1.0, 0.41, 0.71];
// HTML vars
let canvas;
let in_debugAngle;
let out_fps;
// GL vars
let gl;
let a_Position, a_PointSize, a_Place;
let u_ViewMatrix, u_ModelMatrix, u_ProjectionMatrix;
let a_Color;
let a_TexCoord, u_TexColorWeight;
let u_SamplerWall, u_SamplerSky, u_SamplerGround;
let u_WhichTex;
// Texture vars
let wallImage, wallTexture;
let skyImage, skyTexture;
let groundImage, groundTexture;
let wallSrc = './Textures/blocks.jpg';
let groundSrc = './Textures/sand_resized.jpg';
let skySrc = './Textures/sky.jpg';

// Camera Vars
let camera;
let speed = 0.06;
let rotateBy = 2;
// world vars
let world;
// input vars

function setUpWebGL(){
    // Retrieve <canvas> element
    canvas = document.getElementById('webgl');
    // Get the rendering context for WebGL
    gl = canvas.getContext("webgl", {preserveDrawingBuffer:true});
    if (!gl) {
        console.log('Failed to get the rendering context for WebGL');
        return;
    }
    gl.enable(gl.DEPTH_TEST);
}

function setUpDocElements(){
    out_fps = document.getElementById('fps');
}

function connectDocElementsToHandlers(){
    // in_debugAngle.oninput = function(){g_debugAngle = in_debugAngle.value;renderAllShapes();};
}

function connectVariablesToGLSL(){
   // Initialize shaders
    if (!initShaders(gl, VSHADER_SOURCE, FSHADER_SOURCE)) {
        console.log('Failed to intialize shaders.');
        return;
    }
    a_Position = gl.getAttribLocation(gl.program, 'a_Position');
    if(a_Position < 0){
        console.log('Failed to get the storage location of a_Position');
        return;
    }
    a_Color = gl.getAttribLocation(gl.program, 'a_Color');
    if(a_Color < 0){
        console.log('Failed to get the storage location of a_Color');
        return;
    }    
    u_ProjectionMatrix = gl.getUniformLocation(gl.program, 'u_ProjectionMatrix');
    if(u_ProjectionMatrix < 0){
        console.log('Failed to get the storage location of u_ProjectionMatrix');
        return;
    }
    u_ModelMatrix = gl.getUniformLocation(gl.program, 'u_ModelMatrix');
    if(u_ModelMatrix < 0){
        console.log('Failed to get the storage location of u_ModelMatrix');
        return;
    }
    u_ViewMatrix = gl.getUniformLocation(gl.program, 'u_ViewMatrix');
    if(u_ViewMatrix < 0){
        console.log('Failed to get the storage location of u_ViewMatrix');
        return;
    }
    let identityM = new Matrix4();
    gl.uniformMatrix4fv(u_ModelMatrix, false, identityM.elements);
    gl.uniformMatrix4fv(u_ViewMatrix, false, identityM.elements);
    gl.uniformMatrix4fv(u_ProjectionMatrix, false, identityM.elements);

    u_SamplerGround = gl.getUniformLocation(gl.program, 'u_SamplerGround');
    if(u_SamplerGround < 0){
        console.log('Failed to get the storage location of u_SamplerGround');
        return;
    }

    u_SamplerSky = gl.getUniformLocation(gl.program, 'u_SamplerSky');
    if(u_SamplerSky < 0){
        console.log('Failed to get the storage location of u_SamplerSky');
        return;
    }

    u_SamplerWalls = gl.getUniformLocation(gl.program, 'u_SamplerWalls');
    if(u_SamplerWalls < 0){
        console.log('Failed to get the storage location of u_SamplerWalls');
        return;
    }
    u_WhichTex = gl.getUniformLocation(gl.program, 'u_WhichTex');
    if(u_WhichTex < 0){
        console.log('Failed to get the storage location of u_WhichTex');
        return;
    }
    a_TexCoord = gl.getAttribLocation(gl.program, 'a_TexCoord');
    if(a_TexCoord < 0){
        console.log('Failed to get the storage location of a_TexCoord');
        return;
    }
    u_TexColorWeight = gl.getUniformLocation(gl.program, 'u_TexColorWeight');
    if(u_TexColorWeight < 0){
        console.log('Failed to get the storage location of u_TexColorWeight');
        return;
    }
    let texturesCheck = setupTextures();
    if(!texturesCheck){
        console.log('Failed to initialize textures');
        return;
    }
    
    a_Place = gl.getAttribLocation(gl.program, 'a_Place');
    if(a_Place < 0){
        console.log('Failed to get the storage location of a_Place');
        return;
    }

}

function setupTextures(){
    wallTexture = gl.createTexture();   // Create a texture object
    if (!wallTexture) {
        console.log('Failed to create the wall texture object');
        return false;
    }
    wallImage = new Image();
    if (!wallImage) {
        console.log('Failed to create the wall image object');
        return false;
    }
    // Register the event handler to be called on loading an image
    wallImage.onload = function(){ loadTexture(gl.TEXTURE0, wallTexture, wallImage, u_SamplerWall, 0); };
    // Tell the browser to load an image
    wallImage.src = wallSrc;

    groundTexture = gl.createTexture();   // Create a texture object
    if (!groundTexture) {
        console.log('Failed to create the ground texture object');
        return false;
    }
    groundImage = new Image();
    if (!groundImage) {
        console.log('Failed to create the ground image object');
        return false;
    }
    // Register the event handler to be called on loading an image
    groundImage.onload = function(){ loadTexture(gl.TEXTURE2, groundTexture, groundImage, u_SamplerGround, 2); };
    // Tell the browser to load an image
    groundImage.src = groundSrc;

    skyTexture = gl.createTexture();   // Create a texture object
    if (!skyTexture) {
        console.log('Failed to create the sky texture object');
        return false;
    }
    skyImage = new Image();
    if (!skyImage) {
        console.log('Failed to create the sky image object');
        return false;
    }
    // Register the event handler to be called on loading an image
    skyImage.onload = function(){ loadTexture(gl.TEXTURE1, skyTexture, skyImage, u_SamplerSky, 1); };
    // Tell the browser to load an image
    skyImage.src = skySrc;    
    return true;
}

function loadTexture(textureUnit, texture, image, sampler, which){
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1); // Flip the image's y axis
    // Enable texture unit0
    gl.activeTexture(textureUnit);
    // Bind the texture object to the target
    gl.bindTexture(gl.TEXTURE_2D, texture);
    // Set the texture parameters
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // Set the texture image
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, gl.RGB, gl.UNSIGNED_BYTE, image);  
    // // Set the texture unit 0 to the sampler
    gl.uniform1i(sampler, which);
}



function main() {    
    setUpWebGL();
    setUpDocElements();
    connectVariablesToGLSL();
    connectDocElementsToHandlers();    
    // Specify the color for clearing <canvas>
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    // generateAllShapes();
    world = new World();
    refreshLoop(); // stolen from https://www.growingwiththeweb.com/2017/12/fast-simple-js-fps-counter.html
    setupInputEvents();
    world.renderAllShapes();
    
}

// shamelessly stolen from https://www.growingwiththeweb.com/2017/12/fast-simple-js-fps-counter.html
const times = [];
let fps;

function refreshLoop() {
    window.requestAnimationFrame(() => {
        const now = performance.now();
        while (times.length > 0 && times[0] <= now - 1000) {
            times.shift();
        }
        times.push(now);
        fps = times.length;
        out_fps.value = fps;
        refreshLoop();
    });
}
