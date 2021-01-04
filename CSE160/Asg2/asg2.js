// Some code lifted from textbook
// Vertex shader program
var VSHADER_SOURCE = 
    'attribute vec4 a_Position;\n\
     attribute vec4 a_Color;\n\
     varying vec4 v_Color;\n\
     uniform mat4 u_ModelMatrix;\n\
     uniform mat4 u_GlobalRotateMatrix;\n\
     void main() {\n\
      gl_Position = u_GlobalRotateMatrix * u_ModelMatrix * a_Position;  // Set the vertex coordinates of the point\n\
      v_Color = a_Color;\n\
    }\n';

// Fragment shader program
var FSHADER_SOURCE =
    'precision mediump float;\n\
     // uniform vec4 u_FragColor;\n\
     varying vec4 v_Color;\n\
     void main() {\n      \
      gl_FragColor = v_Color; // Set the point color\n\
    }\n';


// Annoying constants

var red = [1.0, 0.0, 0.0];
var green = [0.0, 1.0, 0.0];
var blue = [0.0, 0.0, 1.0];

var pink = [1.0, 0.41, 0.71];


// Globals

// HTML vars
var canvas;

var but_on;
var but_off;

var in_globalAngle;
var in_joint1Angle = 1;
var in_joint2Angle = 1;

// GL vars
var gl;
var a_Position, a_PointSize;

var u_ModelMatrix;
var u_GlobalRotateMatrix;
var a_Color;


// properties of shapes to be drawn
var g_globalAngle = document.getElementById('globalAngle').value;;

var joint1Angle = document.getElementById('joint1Angle').value;
var joint2Angle = document.getElementById('joint2Angle').value;

var g_animation = true;



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
    // Retrieve clear_button element

    in_globalAngle = document.getElementById('globalAngle');
    in_joint1Angle = document.getElementById('joint1Angle');
    in_joint2Angle = document.getElementById('joint2Angle');

    but_on = document.getElementById('on');
    but_off = document.getElementById('off');
}

function connectDocElementsToHandlers(){
    // Register function ( event handler) to be called on a mouse press

    in_globalAngle.oninput = setGlobalAngle;
    in_globalAngle.onchange = setGlobalAngle;

    in_joint1Angle.onchange = setjoint1Angle;
    in_joint1Angle.oninput = setjoint1Angle;
    
    in_joint2Angle.onchange = setjoint2Angle;
    in_joint2Angle.oninput = setjoint2Angle;

    
    but_on.onclick = setAnimationTrue;
    
    but_off.onclick = setAnimationFalse;

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

    
    u_GlobalRotateMatrix = gl.getUniformLocation(gl.program, 'u_GlobalRotateMatrix');
    if(u_GlobalRotateMatrix < 0){
        console.log('Failed to get the storage location of u_GlobalRotateMatrix');
        return;
    }


    u_ModelMatrix = gl.getUniformLocation(gl.program, 'u_ModelMatrix');
    if(u_ModelMatrix < 0){
        console.log('Failed to get the storage location of u_ModelMatrix');
        return;
    }

    var identityM = new Matrix4();
    gl.uniformMatrix4fv(u_ModelMatrix, false, identityM.elements);
}



function setGlobalAngle(ev){
    g_globalAngle = document.getElementById('globalAngle').value;
    renderAllShapes();
}

function setjoint1Angle(ev){
    joint1Angle = document.getElementById('joint1Angle').value;
    renderAllShapes();
}

function setjoint2Angle(ev){
    joint2Angle = document.getElementById('joint2Angle').value;
    renderAllShapes();
}


function setAnimationTrue(ev){
    g_animation = true;
}

function setAnimationFalse(ev){
    g_animation = false;
}

var thrust_speed = 3;

function updateAnimationAngles(){
    if(g_animation){
        joint1Angle = ((90*Math.sin(g_seconds * thrust_speed))+110);
        torsoAngle = ((18*Math.sin(g_seconds * thrust_speed))-5);
        legAngle = (-(20*Math.sin(g_seconds * thrust_speed))+5);
    }
    else{
        torsoAngle = 0;
        legAngle = 0;
    }
}

function renderAllShapes(){

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
    var startTime = performance.now();

    var globalRotMat = new Matrix4().rotate(g_globalAngle,0,1,0);
    gl.uniformMatrix4fv(u_GlobalRotateMatrix, false, globalRotMat.elements);



    // body parts

    // torso
    var torso = new Cube (pink);
    //dumbCube.matrix.setRotate(60, 1,1,0);
    torso.matrix.rotate(torsoAngle, 50,0,0);
    torso.matrix.scale(.3,.7,.1);
    torso.draw();

    // torso
    var head = new Cube (pink);
    //dumbCube.matrix.setRotate(60, 1,1,0);
    head.matrix = new Matrix4(torso.matrix);
    // head.matrix.rotate(headAngle, 50,0,0);
    // head.matrix.scale(.3,.7,.1);

    head.matrix.translate(0,.7,0);
    head.matrix.scale (.8,.3,1.5);
    // head.matrix.rotate(30,1,0,0);      

  
    head.draw();
    
    // arms 
    var upperLeftArm = new Cube (pink);    
    upperLeftArm.matrix.translate(.21,0.1,0);
    upperLeftArm.matrix.scale(.1,.2,.1);
    upperLeftArm.matrix.rotate(-joint1Angle+60,1,0,0);
    upperLeftArm.draw();
    var lowerLeftArm = new Cube(green)
    lowerLeftArm.matrix = new Matrix4(upperLeftArm.matrix);
    lowerLeftArm.matrix.rotate((-joint2Angle-180),1,0,0);    
    lowerLeftArm.matrix.translate(0.01,-1.01,0);    
    // lowerLeftArm.matrix.scale(.1,.2,.1);
    lowerLeftArm.draw();
    
    var upperRightArm = new Cube (pink);    
    upperRightArm.matrix.translate(-.21,0.1,0);
    upperRightArm.matrix.scale(.1,.2,.1);
    upperRightArm.matrix.rotate(-joint1Angle+60,1,0,0);    
    upperRightArm.draw();
    var lowrRightArm = new Cube(green)
    lowrRightArm.matrix = new Matrix4(upperRightArm.matrix);
    lowrRightArm.matrix.rotate((-joint2Angle-180),1,0,0);    
    lowrRightArm.matrix.translate(0.01,-1.01,0);
    lowrRightArm.draw();
    
    // legs
    var rightLeg = new Cube (pink);
    
    rightLeg.matrix.translate(-.09,-0.61,0);
    rightLeg.matrix.rotate(legAngle, 1,0,0);    
    rightLeg.matrix.scale(.1,.5,.1);
    rightLeg.draw();
    var leftLeg = new Cube (pink);
    
    leftLeg.matrix.translate(.09,-0.61,0);
    leftLeg.matrix.rotate(legAngle, 1,0,0);        
    leftLeg.matrix.scale(.1,.5,.1);

    leftLeg.draw();

    
    var duration = performance.now() - startTime;
    
    
}


var g_startTime = performance.now()/1000.0;
var g_seconds = performance.now()/1000.0 - g_startTime;

function tick(){


    g_seconds = performance.now()/1000.0 - g_startTime;
    
    

    updateAnimationAngles();
    
    renderAllShapes();

    requestAnimationFrame(tick);
    
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


    // renderAllShapes();

    requestAnimationFrame(tick);
    
    
 
}

// A note on how to mess with joint angles.
//     It's very easy, all that needs to happen is to set the angle to a global variable, and the joint
// shape to be set with respect it's one step up in the "shape hierarchy".

// Calling tick() is just a matter of getting the seconds, and tying the seconds to the global angle,
// using some different logic, you can easily just switch between animation on or off.
