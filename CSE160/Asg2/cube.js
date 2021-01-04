// class file for cube

class Cube {

    constructor(color){
        this.verts = [[0.5,0.5,0.5],
                      [-0.5,0.5,0.5],
                      [-0.5,-0.5,0.5],
                      [0.5,-0.5,0.5],
                      [0.5,-0.5,-0.5],
                      [0.5,0.5,-0.5],
                      [-0.5,0.5,-0.5],
                      [-0.5,-0.5,-0.5]
                     ];
        this.color = color;
        this.Triangles = findTriangles(this.verts, this.color);

        this.matrix = new Matrix4();
    }

    draw(opts){
        gl.uniformMatrix4fv(u_ModelMatrix , false, this.matrix.elements);        
        drawCube(this.Triangles);
    }


    
}

function findTriangles(verts, color){
        
    var rawTriangles = [];
    var faceColor = color;
    for (var i = 1; i < 7; i++){
        
        if (i != 6){
            var triangle = verts[0].concat(faceColor,verts[i],faceColor,verts[i+1],faceColor);
            rawTriangles.push(triangle);
        }
        else{
            var triangle = verts[0].concat(faceColor,verts[6],faceColor,verts[1],faceColor);
            rawTriangles.push(triangle);
        }

        if (i % 2 == 0)
            faceColor = faceColor.map(x => x * .9);
        
    }

    faceColor = color;
    
    for (var i = 6; i > 0; i--){

        if (i != 1){
            var triangle = verts[7].concat(faceColor,verts[i],faceColor,verts[i-1],faceColor);
            rawTriangles.push(triangle);
        }
        else{
            var triangle = verts[7].concat(faceColor,verts[6],faceColor,verts[1],faceColor);
            rawTriangles.push(triangle);
        }
        
        if (i % 2 == 1)
            faceColor = faceColor.map(x => x * .9);
    }
    // https://stackoverflow.com/a/10865042
    var rawMerged = [].concat.apply([],rawTriangles);
    
    var Triangles = new Float32Array(rawMerged);

    
    
    return Triangles;

    
}


function drawCube(verts){
    
     // create buffer
    var vertexBuffer = gl.createBuffer();
    if (!vertexBuffer) {
        console.log('Failed to create the buffer object ');
        return -1;
    }

    var FSIZE = verts.BYTES_PER_ELEMENT;
    
    // bind buffer to target
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    // write data into buffer
    gl.bufferData(gl.ARRAY_BUFFER, verts, gl.STATIC_DRAW);
    
    // assign buffer to attrib variable
    gl.vertexAttribPointer(a_Position, 3, gl.FLOAT, false, FSIZE*6, 0);
    // enable the assignment
    gl.enableVertexAttribArray(a_Position);
    
    gl.vertexAttribPointer(a_Color, 3, gl.FLOAT, false, FSIZE*6, FSIZE*3);
    gl.enableVertexAttribArray(a_Color);
    
    
    // var vertsToDraw = verts.length/3;
    // draw the array
    gl.drawArrays(gl.TRIANGLES, 0, 36);
    
    // disable the assignment so it can be reused for other shapes
    gl.disableVertexAttribArray(a_Position);
    gl.disableVertexAttribArray(a_Color);
    
    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);   

    
}
