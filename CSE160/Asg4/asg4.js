// Some code lifted from textbook
// Vertex shader program
var VSHADER_SOURCE;

// there's a uniform int variable that tells what texture unit to use: 0 is for wall, 1 is for sky, 2 is for ground
// Fragment shader program
var FSHADER_SOURCE;

// == Globals ==

// Annoying constants
var red = [1.0, 0.0, 0.0];
var green = [0.0, 1.0, 0.0];
var blue = [0.0, 0.0, 1.0];
var pink = [1.0, 0.41, 0.71];
// HTML vars
let canvas;
let in_debugNormalView;
let in_debugNumberOfPlainCubes;
let in_lightingOn;
let out_fps;
// GL vars
let gl;
let ext;
let a_Position, a_PointSize, a_Place;
let u_ViewMatrix, u_ModelMatrix, u_ProjectionMatrix;
let a_Color;
let a_TexCoord, u_TexColorWeight;
let u_SamplerWall, u_SamplerSky, u_SamplerGround;
let u_WhichTex;
let a_Normal;
let u_NormalMatrix;
let u_NormalViewOn;
let u_LightPos;
let u_LightingOn;
let u_CameraPos;
// Texture vars
let wallImage, wallTexture;
let skyImage, skyTexture;
let groundImage, groundTexture;
let wallSrc = './Textures/blocks.jpg';
let groundSrc = './Textures/sand_resized.jpg';
let skySrc = './Textures/dark_sky.jpg';

let chooseWall = 0;
let chooseSky = 1;
let chooseGround = 2;
// Camera Vars
let camera;
let speed = 0.06;
let rotateBy = 2;
// world vars
let world;
let isNormalViewOn = false;
let setNumberPlainCubes = 0;
// input vars

// light vars
let lightColor = [1.0,1.0,1.0];
let lightPosition;
let isLightingOn = false;

function setUpWebGL(){
    // Retrieve <canvas> element
    canvas = document.getElementById('webgl');
    // Get the rendering context for WebGL
    // gl = canvas.getContext("webgl", {preserveDrawingBuffer:true});
    gl = getWebGLContext(canvas, false);
    if (!gl) {
        console.log('Failed to get the rendering context for WebGL');
        return;
    }
    gl.enable(gl.DEPTH_TEST);
    ext = gl.getExtension('ANGLE_instanced_arrays');
}

function setUpDocElements(){
    out_fps = document.getElementById('fps');
    in_debugNormalView = document.getElementById('NormalView');
    in_lightingOn = document.getElementById('LightingOn');
    in_debugNumberOfPlainCubes = document.getElementById('SetPlainCubes');
}

function connectDocElementsToHandlers(){
    in_debugNormalView.onclick = function(){isNormalViewOn = !isNormalViewOn;
                                            world.renderAllShapes();
                                           };
    in_lightingOn.onclick = function(){isLightingOn = !isLightingOn;
                                            world.renderAllShapes();
                                           };
    in_debugNumberOfPlainCubes.oninput = function(){setNumberPlainCubes = in_debugNumberOfPlainCubes.value;
                                                    world.generateWorld();
                                                    world.renderAllShapes();
                                                   };
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

    a_Normal = gl.getAttribLocation(gl.program, 'a_Normal');
    if(a_Normal < 0){
        console.log('Failed to get the storage location of a_Normal');
        return;
    }

    u_NormalMatrix = gl.getUniformLocation(gl.program, 'u_NormalMatrix');
    if(u_NormalMatrix < 0){
        console.log('Failed to get the storage location of u_NormalMatrix');
        return;
    }    

    u_NormalViewOn = gl.getUniformLocation(gl.program, 'u_NormalViewOn');
    if(u_NormalViewOn < 0){
        console.log('Failed to get the storage location of u_NormalViewOn');
        return;
    }

    u_LightPos = gl.getUniformLocation(gl.program, 'u_LightPos');
    if(u_LightPos < 0){
        console.log('Failed to get the storage location of u_LightPos');
        return;
    }
    u_LightingOn = gl.getUniformLocation(gl.program, 'u_LightingOn');
    if(u_LightingOn < 0){
        console.log('Failed to get the storage location of u_LightingOn');
        return;
    }

     u_CameraPos = gl.getUniformLocation(gl.program, 'u_CameraPos');
    if(u_CameraPos < 0){
        console.log('Failed to get the storage location of u_CameraPos');
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


// code stolen from https://stackoverflow.com/a/53374514
function loadTextFile(url) {
  return fetch(url).then(response => response.text());
}
const urls = [
    './vertexShader.glsl',
    './fragmentShader.glsl'
    ]
async function fetchShaders(){
    
    const files = await Promise.all(urls.map(loadTextFile));
    VSHADER_SOURCE = files[0];
    FSHADER_SOURCE = files[1];

    
}

// let g_startTime = performance.now()/1000.0;
// let g_seconds = performance.now()/1000.0 - g_startTime;

// function tick(){
    
//     world.moveLightCube(Math.cos(g_seconds), lightHeight, Math.sin(g_seconds), red);
    
//     world.renderAllShapes();
//     requestAnimationFrame(tick);
    
// }


async function main() {
    const files = await Promise.all(urls.map(loadTextFile));
    VSHADER_SOURCE = files[0];
    FSHADER_SOURCE = files[1];

    // console.log(VSHADER_SOURCE);
    // console.log(FSHADER_SOURCE);    
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
    // world.renderAllShapes();
}

// shamelessly stolen from https://www.growingwiththeweb.com/2017/12/fast-simple-js-fps-counter.html
const times = [];
let fps;

let lightHeight = 10;
let rawLightPosition = new Float32Array([0,0,0]);
function refreshLoop() {
    window.requestAnimationFrame(() => {
        const now = performance.now();
        while (times.length > 0 && times[0] <= now - 1000) {
            times.shift();
        }
        times.push(now);
        fps = times.length;
        out_fps.value = fps;

        // my own stuff
        let g_seconds = now/1000;
        lightPosition = [10*Math.cos(g_seconds), lightHeight, 10*Math.sin(g_seconds)];
        world.moveLightCube(lightPosition[0], lightPosition[1], lightPosition[2], lightColor);
        rawLightPosition[0] = lightPosition[0];
        rawLightPosition[1] = lightPosition[1];
        rawLightPosition[2] = lightPosition[2];

        world.renderAllShapes();
        refreshLoop();
    });
}
