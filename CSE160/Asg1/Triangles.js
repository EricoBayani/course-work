// Triangles.js class file

class Triangle extends Shape {
    
    constructor(xy, color, size){
        
        super(xy, color, size);
        
        // Use center position and size to calculate vertices of triangle
        
        this.size = this.size/200;
        
        var top = [this.xy[0],this.xy[1]+this.size];

        var bottom_y = (this.size * Math.sin(Math.PI/6));
        var bottom_x = (this.size * Math.cos(Math.PI/6));
        
        var bottom_right = [this.xy[0]+bottom_x,this.xy[1]-bottom_y];
        var bottom_left = [this.xy[0]-bottom_x,this.xy[1]-bottom_y];
        
        this.vertPoints = new Float32Array([
            top[0], top[1],
            bottom_right[0], bottom_right[1],
            bottom_left[0], bottom_left[1]
        ]);

        // this.verts = 3;
        
    }

    render(){
        // console.log(this.Vertices.toString());
        gl.uniform4f(u_FragColor, this.color[0], this.color[1], this.color[2], this.color[3]);
        drawTriangle(this.vertPoints);
    }
    
    
}

// needs Float32Array object
function drawTriangle(verts){

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
    // draw the array
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    // disable the assignment so it can be reused for other shapes
    gl.disableVertexAttribArray(a_Position);
    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);

    
}
