// a different cube class file that tweaks the face generation for the cubes back into a function call
let v0 = [0.5,0.5,0.5];
let v1 = [-0.5,0.5,0.5];
let v2 = [-0.5,-0.5,0.5];
let v3 = [0.5,-0.5,0.5];
let v4 = [0.5,-0.5,-0.5];
let v5 = [0.5,0.5,-0.5];
let v6 = [-0.5,0.5,-0.5];
let v7 = [-0.5,-0.5,-0.5];

class Cube {
    // takes a color, and position on map
    constructor(color, x, y, z){
        this.raw = [].concat(v0,color,[1,1],x,y,z,v1,color,[0,1],x,y,z,v2,color,[0,0],x,y,z,
                             v0,color,[1,1],x,y,z,v2,color,[0,0],x,y,z,v3,color,[1,0],x,y,z, // one face
                             v5,color,[1,1],x,y,z,v0,color,[0,1],x,y,z,v3,color,[0,0],x,y,z,
                             v5,color,[1,1],x,y,z,v3,color,[0,0],x,y,z,v4,color,[1,0],x,y,z, // one face
                             v6,color,[1,1],x,y,z,v5,color,[0,1],x,y,z,v4,color,[0,0],x,y,z,
                             v6,color,[1,1],x,y,z,v4,color,[0,0],x,y,z,v7,color,[1,0],x,y,z, // one face
                             v1,color,[1,1],x,y,z,v6,color,[0,1],x,y,z,v7,color,[0,0],x,y,z,
                             v1,color,[1,1],x,y,z,v7,color,[0,0],x,y,z,v2,color,[1,0],x,y,z, // one face
                             v5,color,[1,1],x,y,z,v6,color,[0,1],x,y,z,v1,color,[0,0],x,y,z,
                             v5,color,[1,1],x,y,z,v1,color,[0,0],x,y,z,v0,color,[1,0],x,y,z, // one face
                             v3,color,[1,1],x,y,z,v2,color,[0,1],x,y,z,v7,color,[0,0],x,y,z,
                             v3,color,[1,1],x,y,z,v7,color,[0,0],x,y,z,v4,color,[1,0],x,y,z, // one face
                            );


    }
}


function drawAllCubes(allVerts, count, textureWeight, which){

    gl.uniform1f(u_TexColorWeight,  textureWeight);
    gl.uniform1i(u_WhichTex, which);
    let verts = new Float32Array(allVerts);
    
    
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
    let vertsToDraw = count * 36;
    // draw the array
    gl.drawArrays(gl.TRIANGLES, 0, vertsToDraw);

    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);   

    disableAttribVars();

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

let allRawWalls = [];
let wallsCount = 0;
class wallCube extends Cube{
    constructor(color, x, y, z){
        super(color, x, y, z);

        allRawWalls = [...allRawWalls,...this.raw];
        wallsCount++;
        this.modelMatrix = new Matrix4();
    }
    drawWalls(){
        gl.uniformMatrix4fv(u_ModelMatrix, false, this.modelMatrix.elements);
        drawAllCubes(allRawWalls, wallsCount, 1, 0);
    }
}

let rawSky = [];
let skyCount = 0;
class skyCube extends Cube{
    constructor(color, x, y, z){
        super(color, x, y, z);

        rawSky = [...rawSky,...this.raw];
        skyCount++;
        this.modelMatrix = new Matrix4();
        this.modelMatrix.scale(32, 32, 32);
    }

    drawSky(){
        gl.uniformMatrix4fv(u_ModelMatrix, false, this.modelMatrix.elements);
        drawAllCubes(rawSky, skyCount, 1, 1);
    }
}

let rawGround = [];
let groundCount = 0;
class groundCube extends Cube{
    constructor(color, x, y, z){
        super(color, x, y, z);

        rawGround = [...rawGround,...this.raw];
        groundCount++;
        this.modelMatrix = new Matrix4();
        this.modelMatrix.setTranslate(0,-.25,0);
        this.modelMatrix.scale(32,0,32);
    }

    drawGround(){
        gl.uniformMatrix4fv(u_ModelMatrix, false, this.modelMatrix.elements);
        drawAllCubes(rawGround, groundCount, 0.3, 2);
    }
}

class World {
    constructor(){
        this.sky = new skyCube(blue, 0,0,0); // skybox texture
        this.ground = new groundCube(green, 0,0,0); // ground texture
        this.wall = new wallCube(green, 0,0,5);
        this.world = [];

        for(let i = 0; i < 30; i++){
            let randomX = (Math.random()*30) - 15;
            let randomZ = (Math.random()*30) - 15;
            makeTower(randomX, randomZ);
        }

        this.camera = new Camera();

        console.log('world generation done');
    }

    renderAllShapes(){
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        gl.uniformMatrix4fv(u_ViewMatrix, false, this.camera.viewMatrix.elements);
        gl.uniformMatrix4fv(u_ProjectionMatrix, false, this.camera.projectionMatrix.elements);
        
        this.wall.drawWalls();
        this.sky.drawSky();
        this.ground.drawGround();
    }
    
}

// building generators
function makeTower(x,z){
    let size = 1;
    for(let i = 0; i < size+3; i++){
        for(let j = 0; j < size; j++){
            new wallCube(red, x - size, i, z - j);
            new wallCube(red, x - size, i, z + j);
            new wallCube(red, x + size, i, z - j);
            new wallCube(red, x + size, i, z + j);

            new wallCube(red, x - j, i, z - size);
            new wallCube(red, x - j, i, z + size);
            new wallCube(red, x + j, i, z - size);
            new wallCube(red, x + j, i, z + size);
        }
    }
}
