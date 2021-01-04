// asg0.js

// globals and checks
var canvas1, canvas2, ctx, gl;

var x1, y1, v1;
var x2, y2, v2;

function drawVector(v, color){

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineTo

    
    ctx.strokeStyle = color;
    ctx.beginPath();
    // move start to center of the 400x400 canvas
    ctx.moveTo(200,200);
    // adjust to 2d style coordinates
    ctx.lineTo(200+(20*(v.elements[0])),200-(20*(v.elements[1])));
    ctx.stroke();
    return;
}


function handleDrawEvent(){

    // clear the canvas with the vectors
    ctx.clearRect(0, 0, canvas2.width, canvas2.height);
    // create vector using input values
    x1 = document.getElementById('x1').value;
    y1 = document.getElementById('y1').value;

    x2 = document.getElementById('x2').value;
    y2 = document.getElementById('y2').value;
    // call drawVector
    v1 = new Vector3([x1, y1, 0]);
    v2 = new Vector3([x2, y2, 0]);

    drawVector(v1, 'red');
    drawVector(v2, 'blue');
}

function handleDrawOperationEvent(){

    var op = document.getElementById('opt-select').value;
    
    // Get the scalar value
    var scalar = document.getElementById('scalar').value;

    
    switch(op){
    case 'add':
        console.log('adding v1+v2');
        drawVector(v1.add(v2), 'green');
        break;
    case 'subtract':
        console.log('subbing v1-v2');
        drawVector(v1.sub(v2), 'green');
        break;
    case 'multiply':
        console.log('multiplying v1, v2 by scalar');
        if (typeof scalar != 'number'){
            console.log('scalar input error, no input');
            return;
        }        
        drawVector(v1.mul(scalar), 'green');
        drawVector(v2.mul(scalar), 'green');
        break;
    case 'divide':
        console.log('dividing v1,v2 by scalar');
        if (typeof scalar != 'number'){
            console.log('scalar input error, no input');
            return;
        }        
        drawVector(v1.div(scalar), 'green');
        drawVector(v2.div(scalar), 'green');
        break;
    case 'normalize':
        console.log('normalizing v1, v2');
        console.log('Magnitude v1: ' + v1.magnitude());
        console.log('Magnitude v2: ' + v2.magnitude());
        
        drawVector(v1.normalize(), 'green');
        drawVector(v2.normalize(), 'green');
        
        break;
    case 'angle_between':
        console.log('finding angle between v1 v2');
        // find angle in radians
        var angleRad = Math.acos(Vector3.dot(v1,v2)/((v1.magnitude())*(v2.magnitude())));
        var angleDeg = (180*angleRad)/Math.PI;
        console.log('Angle: ' + angleDeg);
        break;
    case 'area':
        console.log('finding area of triangle made by v1 andv2');
        // find angle in radians
        var cross = Vector3.cross(v1,v2);
        var area = cross.magnitude()/2;
        console.log('Area of the Triangle: ' + area)
        break;
    default:
        console.log('this is not supposed to happen');
        break;
    }
    
}

function main(){
    // Retrive <canvas> element
    canvas1 = document.getElementById('cleared');
    if (!canvas1){
        console.log('Failed to retrieve the <canvas> element');
        return;
    }

    gl = getWebGLContext(canvas1);
    
    // Get the render context
    
    if (!gl){
        console.log('Failed to get the rendering context for WebGL');
        return;
    }

    // specify the color black
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT);
    
    // Get the rendering context for 2DCG
    
    canvas2 = document.getElementById('drawn');
    ctx = canvas2.getContext('2d');
    if (!ctx){
        console.log('Failed to retrieve canvas context');
        return;
    }

    
}
