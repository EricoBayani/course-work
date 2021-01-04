// class file for cube

// Global definition of vertices of a Cube

let v0 = [0.5,0.5,0.5];
let v1 = [-0.5,0.5,0.5];
let v2 = [-0.5,-0.5,0.5];
let v3 = [0.5,-0.5,0.5];
let v4 = [0.5,-0.5,-0.5];
let v5 = [0.5,0.5,-0.5];
let v6 = [-0.5,0.5,-0.5];
let v7 = [-0.5,-0.5,-0.5];

// allRawShapes holds raw shape data for all shapes in scene to be
// converted into the Float32Array and passed as a single vertex buffer
// for a single monolithic drawArrays() call
//
// this might need to be turned into a array of allRawCubes where a loop would
// make subsequent monolithic calls to drawArrays()
let allRawCubes = [];
let allCubes = [];
// let cubesCount = 0;

// The size of the 1 raw triangle with vertex info + color + texture coord must be 3*(3+3+2+3) = 24 elements
// The order of verts must be "top right", then "top left", "bottom left", "bottom right"
class Face {
    
    constructor(verts, color, place, ID){
        this.rawTriangles = [];
        this.faceColor = color;
        this.ID = ID;
        let triangle1 = verts[0].concat(this.faceColor,[1,1],place,
                                        verts[1],this.faceColor,[0,1],place,
                                        verts[2],this.faceColor,[0,0],place);
        this.rawTriangles.push(triangle1);
                
        let triangle2 = verts[0].concat(this.faceColor,[1,1],place,
                                        verts[2],this.faceColor,[0,0],place,
                                        verts[3],this.faceColor,[1,0],place);
        this.rawTriangles.push(triangle2);

        this.rawMerged = [].concat.apply([],this.rawTriangles);
    }
    
}

class Cube {
    // takes a color, and position on map
    constructor(color, x, y, z){
        // console.log('cube being created');
        // this.matrix = new Matrix4();
        // this.matrix.setTranslate(x, y, z);
        this.place = [x, y, z];
        this.faces = [];
        this.faces.push(new Face([v0,v1,v2,v3],color,this.place,1));
        this.faces.push(new Face([v5,v0,v3,v4],color,this.place,2));
        this.faces.push(new Face([v6,v5,v4,v7],color,this.place,3));
        this.faces.push(new Face([v1,v6,v7,v2],color,this.place,4));
        this.faces.push(new Face([v5,v6,v1,v0],color,this.place,5));
        this.faces.push(new Face([v3,v2,v7,v4],color,this.place,6));
        let toConcat = this.faces[0].rawMerged.slice();
        for (let i = 1; i < this.faces.length; i++){            
            toConcat = toConcat.concat(this.faces[i].rawMerged.slice());
            // this.facesMerged.concat(toConcat);
        }
        this.facesMerged = toConcat.slice();
        // // preserve this shape's location in the big array for future access if necessary.
        this.beginPositionInAllCubes = allRawCubes.length;
        this.endPositionInAllCubes = this.beginPositionInAllCubes + this.facesMerged.length - 1;
        allRawCubes = allRawCubes.concat(this.facesMerged);
        allCubes.push(this);

    }
    draw(opts){
        // gl.uniformMatrix4fv(u_ModelMatrix , false, this.matrix.elements);
        // drawCube(this.Triangles);
    }    
}
// let referenceCube = new Cube(red);

// // Model is just the position and info of a cube
// class Model {
//     constructor(){
//         this.matrix = new Matrix4();

//         this.beginPositionInAllCubes = allRawCubes.length;
//         this.endPositionInAllCubes = this.beginPositionInAllCubes + this.facesMerged.length - 1;
//         allRawCubes = allRawCubes.concat(this.facesMerged);
//         allCubes.push(this);
//     }

// }
// Create the float array with the colors interleaved in the same buffer
// function findTriangles(verts, color){        
//     let rawTriangles = [];
//     let faceColor = color;
//     for (let i = 1; i < 7; i++){        
//         if (i != 6){
//             let triangle = verts[0].concat(faceColor,verts[i],faceColor,verts[i+1],faceColor);
//             rawTriangles.push(triangle);
//         }
//         else{
//             let triangle = verts[0].concat(faceColor,verts[6],faceColor,verts[1],faceColor);
//             rawTriangles.push(triangle);
//         }
//         if (i % 2 == 0)
//             faceColor = faceColor.map(x => x * .9);        
//     }
//     faceColor = color;
//     for (let i = 6; i > 0; i--){
//         if (i != 1){
//             let triangle = verts[7].concat(faceColor,verts[i],faceColor,verts[i-1],faceColor);
//             rawTriangles.push(triangle);
//         }
//         else{
//             let triangle = verts[7].concat(faceColor,verts[6],faceColor,verts[1],faceColor);
//             rawTriangles.push(triangle);
//         }        
//         if (i % 2 == 1)
//             faceColor = faceColor.map(x => x * .9);
//     }
//     // https://stackoverflow.com/a/10865042
//     let rawMerged = [].concat.apply([],rawTriangles);    
//     let Triangles = new Float32Array(rawMerged);
    
//     return Triangles;
// }


function drawAllCubes(){

    //gl.uniformMatrix4fv(u_ModelMatrix , false, allCubes[0].matrix.elements);
    gl.uniform1f(u_TexColorWeight,  1.0);
    let verts = new Float32Array(allRawCubes);
    
    
     // create, bind, write to buffer
    let vertexBuffer = gl.createBuffer();
    if (!vertexBuffer) {
        console.log('Failed to create the buffer object ');
        return -1;
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, verts, gl.STATIC_DRAW);
    
    let FSIZE = verts.BYTES_PER_ELEMENT;
    enableAttribVars(FSIZE);
    enableUniformVars();
    let vertsToDraw = allCubes.length * 36;
    // draw the array
    gl.drawArrays(gl.TRIANGLES, 0, vertsToDraw);

    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);   

    disableAttribVars();

    // console.log('The raw vertex+color+tex+place buffer is ' + allRawCubes.length + ' elements long');
    
}


// 'helpers'
function enableAttribVars(bytesPerElem){
    let FSIZE = bytesPerElem;
    
    gl.vertexAttribPointer(a_Position, 3, gl.FLOAT, false, FSIZE*11, 0);
    gl.enableVertexAttribArray(a_Position);
    
    gl.vertexAttribPointer(a_Color, 3, gl.FLOAT, false, FSIZE*11, FSIZE*3);
    gl.enableVertexAttribArray(a_Color);

    gl.vertexAttribPointer(a_TexCoord, 2, gl.FLOAT, false, FSIZE*11, FSIZE*6);
    gl.enableVertexAttribArray(a_TexCoord);
    
    gl.vertexAttribPointer(a_Place, 3, gl.FLOAT, false, FSIZE*11, FSIZE*8);
    gl.enableVertexAttribArray(a_Place);        
}

function disableAttribVars(){
    // disable the assignment so it can be reused for other shapes
    gl.disableVertexAttribArray(a_Position);
    gl.disableVertexAttribArray(a_Color);
    gl.disableVertexAttribArray(a_TexCoord);
    gl.disableVertexAttribArray(a_Place);
}

function enableUniformVars(){
    
}


