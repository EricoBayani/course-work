// Circles.js class file

class Circle extends Shape {
    
    constructor(xy, color, size){
        
        super(xy, color, size);

        this.size = this.size/200;
        // this.segments = segments;
        this.step = (2*Math.PI)/segments;
    }

    render(){
       
        gl.uniform4f(u_FragColor, this.color[0], this.color[1], this.color[2], this.color[3]);
        
        // the 0 radians line segment
        var secondVert = [this.xy[0]+this.size, this.xy[1]];

        var vertPoints = [];
        
        // a little more than 2*pi so avoid "pacmanning"
        for(var alpha = 0; alpha <= (2.5*Math.PI); alpha += this.step){
            
            var thirdVertX = (this.size * Math.cos(alpha))+this.xy[0];            
            var thirdVertY = (this.size * Math.sin(alpha))+this.xy[1];
            
            vertPoints.push(this.xy[0]);
            vertPoints.push(this.xy[1]);
            vertPoints.push(secondVert[0]);
            vertPoints.push(secondVert[1]);
            vertPoints.push(thirdVertX);
            vertPoints.push(thirdVertY);
            
            secondVert = [thirdVertX,thirdVertY];            
        }

        var vertPointsTyped = new Float32Array(vertPoints);
        // avoided using drawTriangle more than hundreds of times per render call
        // by using Triangle Fan instead
        drawTriangleFan(vertPointsTyped);
        
    }
    
    
} 
// needs Float32Array object
function drawTriangleFan(verts){

    // create buffer
    var vertexBuffer = gl.createBuffer();
    if (!vertexBuffer) {
        console.log('Failed to create the buffer object ');
        return -1;
    }

    // bind buffer to target
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    // write data into buffer
    gl.bufferData(gl.ARRAY_BUFFER, verts, gl.STATIC_DRAW);
    // assign buffer to attrib variable
    gl.vertexAttribPointer(a_Position, 2, gl.FLOAT, false, 0, 0);
    // enable the assignment
    gl.enableVertexAttribArray(a_Position);
    var vertsToDraw = (verts.length/2);
    // draw the array
    gl.drawArrays(gl.TRIANGLE_FAN, 0, vertsToDraw);
    // disable the assignment so it can be reused for other shapes
    gl.disableVertexAttribArray(a_Position);
    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);

    
}
