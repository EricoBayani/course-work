// Some code lifted from textbook
// Vertex shader program
var VSHADER_SOURCE = 
    'attribute vec4 a_Position;\n\
     attribute float a_PointSize;\n \
     void main() {\n\
      gl_Position = a_Position;  // Set the vertex coordinates of the point\n\
      gl_PointSize = a_PointSize; // Set the point size\n\
    }\n';

// Fragment shader program
var FSHADER_SOURCE =
    'precision mediump float;\n\
     uniform vec4 u_FragColor;\n\
     void main() {\n      \
      gl_FragColor = u_FragColor; // Set the point color\n\
    }\n';

// Globals

// HTML vars
var canvas;
var but_clear;

var but_square;
var but_triangle;
var but_circle;

var in_red;
var in_green;
var in_blue;

var in_size;
var in_segments;

var out_numShapes; 

// GL vars
var gl;
var a_Position, a_PointSize;
var u_FragColor;


// properties of shapes to be drawn
var red, green, blue;
var size;

var segments;

var currentShape = 'point';
var g_shapes = [];


function setUpWebGL(){
    // Retrieve <canvas> element
    canvas = document.getElementById('webgl');

    // // Get the rendering context for WebGL
    // gl = getWebGLContext(canvas);

    gl = canvas.getContext("webgl", {preserveDrawingBuffer:true});
    if (!gl) {
        console.log('Failed to get the rendering context for WebGL');
        return;
    }
}


function setUpDocElements(){
    // Retrieve clear_button element
    but_clear = document.getElementById('clear');

    // Retrive shape button elements
    but_square = document.getElementById('square');
    but_triangle = document.getElementById('triangle');
    but_circle = document.getElementById('circle');

    // Retrieve color slider elements
    in_red = document.getElementById('red');
    in_green = document.getElementById('green');
    in_blue = document.getElementById('blue');

    in_size = document.getElementById('size');
    in_segments = document.getElementById('segments');

    out_numShapes = document.getElementById('numShapes');

    // initialize to unchanged values
    red = document.getElementById('red').value;
    green = document.getElementById('green').value;
    blue = document.getElementById('blue').value;

    size = document.getElementById('size').value;
    segments = document.getElementById('segments').value;

    

    
}

function connectDocElementsToHandlers(){
    // Register function ( event handler) to be called on a mouse press
    canvas.onmousedown = click;
    canvas.onmousemove = click;
    
    but_clear.onclick = clearCanvas;
    but_square.onclick = setShapeToSquare;
    but_triangle.onclick = setShapeToTriangle;
    but_circle.onclick = setShapeToCircle;

    in_red.oninput = setRed;
    in_red.onchange = setRed;

    in_green.oninput = setGreen;
    in_green.onchange = setGreen;

    in_blue.oninput = setBlue;
    in_blue.onchange = setBlue;

    in_size.oninput = setSize;
    in_size.onchange = setSize;

    in_segments.oninput = setSegments;
    in_segments.onchange = setSegments;

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

    a_PointSize = gl.getAttribLocation(gl.program, 'a_PointSize');
    if(a_Position < 0){
        console.log('Failed to get the storage location of a_PointSize');
        return;
    }

    u_FragColor = gl.getUniformLocation(gl.program, 'u_FragColor');
    if(u_FragColor < 0){
        console.log('Failed to get the storage location of u_FragColor');
        return;
    }
}

function renderAllShapes(){

    // // Clear <canvas>
    var len = g_shapes.length;
    
    g_shapes[len-1].render();
    
    out_numShapes.value = len;
}



function click(ev) {
    
    if (ev.buttons == 1){
    
        var x = ev.clientX; // x coordinate of a mouse pointer
        var y = ev.clientY; // y coordinate
        var rect = ev.target.getBoundingClientRect();

        x = ((x - rect.left) - canvas.width/2)/(canvas.width/2);
        y = (canvas.height/2 - (y - rect.top))/(canvas.height/2);

        // Retrieve the slider elements

        var shape;
        
        switch(currentShape){

        case 'point':
            shape = new Shape([x,y], [red, green, blue, 1.0], size);
            break;
        case 'triangle':
            shape = new Triangle([x,y], [red, green, blue, 1.0], size);
            break;
        case 'circle':
            // console.log('cannott draw a circle yet');
            shape = new Circle([x,y], [red, green, blue, 1.0], size);
            break;            
        default:
            console.log('This should not happen, this means shape assignment was messed up');
            break;
        }
        
        if (shape != undefined || shape != null){
            
            g_shapes.push(shape);
        
            renderAllShapes();
        }
        
    }

}

function setShapeToSquare(ev){

    currentShape = 'point';
}

function setShapeToTriangle(ev){

    currentShape = 'triangle';
}
function setShapeToCircle(ev){

    currentShape = 'circle';
}

function setRed(ev){
    red = document.getElementById('red').value;
}

function setGreen(ev){
    green = green = document.getElementById('green').value;
}

function setBlue(ev){
    blue = document.getElementById('blue').value;
}

function setSize(ev){
    size = document.getElementById('size').value;
}

function setSegments(ev){
    segments = document.getElementById('segments').value;
}

function clearCanvas(ev){
    g_shapes = [];

    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT);
    
    out_numShapes.value = len;
    renderAllShapes();
}

function main() {
    
    setUpWebGL();
    setUpDocElements();
    connectVariablesToGLSL();
    connectDocElementsToHandlers();
    
    // Specify the color for clearing <canvas>
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT);

 
}

