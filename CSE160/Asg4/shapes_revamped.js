// a different cube class file that tweaks the face generation for the cubes back into a function call
let v0 = [0.5,0.5,0.5];
let v1 = [-0.5,0.5,0.5];
let v2 = [-0.5,-0.5,0.5];
let v3 = [0.5,-0.5,0.5];
let v4 = [0.5,-0.5,-0.5];
let v5 = [0.5,0.5,-0.5];
let v6 = [-0.5,0.5,-0.5];
let v7 = [-0.5,-0.5,-0.5];


class Vertex{
    // vertices contain 8 floating point numbers 
    constructor(position, texCoord, normal){
        // this.position = new Vector3(position);
        this.position = position;
        this.positionVector = new Vector3(position);
        this.texCoord = texCoord;
        // this.offsetXYZ = offsetXYZ
        this.normal = normal;
        this.raw = [];
        this.makeRaw();
    }

    setNormal(normal){
        this.normal = normal;
    }
    makeRaw(){
        this.raw = [].concat(this.position, this.texCoord, this.normal);
    }
    
}


/*
 Face Diagram:
[0,1]           [1,1]               
A-----------------B
|                 |
|                 |
|                 |
|                 |
|                 |
|                 |
|                 |
C-----------------D
[0,0]           [1,0]
*/


let defaultTexCoords = [[1,1],[0,1],[0,0],[1,0]]
function createFace(a, b, c, d, normal, normalize = false, texCoords = defaultTexCoords){
    let faceValues = [];
    let faceVertA = new Vertex(a, texCoords[0], normal);
    let faceVertB = new Vertex(b, texCoords[1], normal);
    let faceVertC = new Vertex(c, texCoords[2], normal);
    let faceVertD = new Vertex(d, texCoords[3], normal);
    faceValues = faceValues.concat(faceVertA.raw, faceVertB.raw, faceVertC.raw,
                                   faceVertA.raw, faceVertC.raw, faceVertD.raw);
    return faceValues;
}


class Cube{
    // takes a color, and position on map
    constructor(){

        let face0 = createFace(v0, v1, v2, v3, [0,0,-1]);
        let face1 = createFace(v5, v0, v3, v4, [1,0,0]);
        let face2 = createFace(v6, v5, v4, v7, [0,0,1]);
        let face3 = createFace(v1, v6, v7, v2, [-1,0,0]);
        let face4 = createFace(v5, v6, v1, v0, [0,1,0]);
        let face5 = createFace(v3, v2, v7, v4, [0,-1,0]);
        this.combinedVerts = [].concat(face0,face1,face2,face3,face4,face5);
        this.raw = new Float32Array(this.combinedVerts);
        this.numberOfVertsInShape = 36;
    }
}


let steps = 30;
class Sphere{
    constructor(){
        this.numberOfVertsInShape = 0;
        this.combinedVerts = [];
        let d = Math.PI/steps;
        let dd = d;
        for (let t = 0; t < Math.PI; t+=d){
            for (let r = 0; r < (Math.PI*2); r+=d){
                let p1 = [Math.sin(t)*Math.cos(r), Math.sin(t)*Math.sin(r), Math.cos(t)];
                
                let p2 = [Math.sin(t+dd)*Math.cos(r), Math.sin(t+dd)*Math.sin(r), Math.cos(t+dd)];
                let p3 = [Math.sin(t)*Math.cos(r+dd), Math.sin(t)*Math.sin(r+dd), Math.cos(t)];
                let p4 = [Math.sin(t+dd)*Math.cos(r+dd), Math.sin(t+dd)*Math.sin(r+dd), Math.cos(t+dd)];

                let face = createFace(p1, p3, p4, p2, p1, false);
                for(let i = 0; i < face.length; i++){
                    this.combinedVerts.push(face[i]);
                }
                this.numberOfVertsInShape += 6; // 6 verts per face
            }
        }
        this.raw = new Float32Array(this.combinedVerts);
    }
}

// Shape list objects hold the shape objects, which hold the position of its center in the world
// Shapes get defined and pushed onto the list, and when all the shapes are added, the list is commited
// wherein the vertices
class ShapeList{
    constructor(shapeType, textureWeight = 1.0, which = 0){
        this.list = [];
        this.count = 0;
        this.shapeType = shapeType; // e.g. cube object
        this.raw = null;
        this.commited = false;
        // texture info; textureWeight determines how much texture comes through
        // which refers to an integer indicating what textureUnit we're on
        this.textureWeight = textureWeight;
        this.which = which;
        this.modelMatrix = new Matrix4();
        this.normalMatrix = new Matrix4();
    }
    addShape(x,y,z, color){
        let info = [x, y, z, color[0], color[1], color[2] ];
        for (let i = 0; i < info.length; i++){
            this.list.push(info[i]);
        }
        this.count++;
        this.committed = false;
    }
    commit(){
        this.raw = new Float32Array(this.list);
        this.normalMatrix.setInverseOf(this.modelMatrix);
        this.normalMatrix.transpose();
        this.commited = true;
    }
    clearList(){
        this.list = [];
        this.count = 0;
        this.raw = null;
        this.commited = false;
    }
    drawAllShapes(){
        if(!this.raw || !this.commited){
            console.log('ShapeList not commited');
            return;
        }
        else {
            gl.uniformMatrix4fv(u_ModelMatrix, false, this.modelMatrix.elements);
            gl.uniformMatrix4fv(u_NormalMatrix, false, this.normalMatrix.elements);


            drawShapes(this.shapeType.numberOfVertsInShape, this.shapeType.raw, this.raw, this.count, this.textureWeight, this.which);
        }
    }
    setModelMatrix(matrix){
        this.modelMatrix = matrix;
    }
}

function drawShapes(numberOfVertsInShape, convertedInstance, convertedInfo, numberOfShapes, textureWeight, which){
    gl.uniform1f(u_TexColorWeight,  textureWeight);
    gl.uniform1i(u_WhichTex, which);
    let verts = convertedInstance;
    let info = convertedInfo;


    let FSIZE = verts.BYTES_PER_ELEMENT;    


     // setup vertex buffer for instance (initial position, texture coordinates, normals)
    let infoBuffer = gl.createBuffer();
    if (!infoBuffer) {
        console.log('Failed to create the buffer object for info data');
        return -1;
    }
    
    gl.bindBuffer(gl.ARRAY_BUFFER, infoBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, info, gl.STATIC_DRAW);
    let windowSize = 6;
    gl.vertexAttribPointer(a_Place, 3, gl.FLOAT, false, FSIZE*windowSize, 0);
    gl.enableVertexAttribArray(a_Place);

    gl.vertexAttribPointer(a_Color, 3, gl.FLOAT, false, FSIZE*windowSize, FSIZE*3);
    gl.enableVertexAttribArray(a_Color); 

    
     // setup vertex buffer for instance (initial position, texture coordinates, normals)
    let vertexBuffer = gl.createBuffer();
    if (!vertexBuffer) {
        console.log('Failed to create the buffer object ');
        return -1;
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, verts, gl.STATIC_DRAW);
    windowSize = 8;
    gl.vertexAttribPointer(a_Position, 3, gl.FLOAT, false, FSIZE*windowSize, 0);
    gl.enableVertexAttribArray(a_Position);

    gl.vertexAttribPointer(a_TexCoord, 2, gl.FLOAT, false, FSIZE*windowSize, FSIZE*3);
    gl.enableVertexAttribArray(a_TexCoord);    

    gl.vertexAttribPointer(a_Normal, 3, gl.FLOAT, false, FSIZE*windowSize, FSIZE*5);
    gl.enableVertexAttribArray(a_Normal);    

    
    ext.vertexAttribDivisorANGLE(a_Color,1);
    ext.vertexAttribDivisorANGLE(a_Place,1);    
    ext.vertexAttribDivisorANGLE(a_TexCoord,0);
    ext.vertexAttribDivisorANGLE(a_Position,0);
    ext.vertexAttribDivisorANGLE(a_Normal,0);

    let vertsToDraw = numberOfVertsInShape;
    // draw the array
    // gl.drawArrays(gl.TRIANGLES, 0, vertsToDraw, numberOfShapes);
    ext.drawArraysInstancedANGLE(gl.TRIANGLES, 0, vertsToDraw, numberOfShapes);
    
    // delete the buffer after I'm done
    gl.deleteBuffer(vertexBuffer);
    gl.deleteBuffer(infoBuffer);
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
    gl.disableVertexAttribArray(a_Normal);
}

function enableUniformVars(){
}


class World {
    constructor(){
        this.referCube = new Cube();
        this.referSphere = new Sphere();
        this.listOfShapeLists = [];
        
        this.regularSpheres = new ShapeList(this.referSphere, 0.0, chooseWall);
        this.listOfShapeLists.push(this.regularSpheres);
        
        this.regularCubes = new ShapeList(this.referCube, 1.0, chooseWall);
        this.listOfShapeLists.push(this.regularCubes);
        
        this.groundCubes = new ShapeList(this.referCube, 1.0,chooseGround);
        this.listOfShapeLists.push(this.groundCubes);

        this.skyCubes = new ShapeList(this.referCube, 1.0,chooseSky);
        this.listOfShapeLists.push(this.skyCubes);
        
        this.lightCubes = new ShapeList(this.referSphere, 0.0,chooseGround);
        this.listOfShapeLists.push(this.lightCubes);
        
        this.camera = new Camera();
        this.generateWorld();
        
    }

    generateWorld(){
        for(let i = 0; i < this.listOfShapeLists.length; i++){
            this.listOfShapeLists[i].clearList();
        }
        // this.makeTower(5,5, red);
        
        let numberOfPlainCubes = setNumberPlainCubes;
        for(let i = 0; i < numberOfPlainCubes; i++){
            let x = Math.random() * 10 - 5;
            let y = Math.random() * 10 - 5;
            let z = Math.random() * 10 - 5;
            this.makeRegularCube(x,y,z, red);
        }
        this.makeGroundCube(0,-0.53,0, green);
        this.makeSkyCube(0,0,0, blue);
        this.makeSphere(0,0,0, blue);


        // for(let i = 0; i < numberOfPlainCubes; i++){
        //     let x = Math.random() * 10 - 5;
        //     let y = Math.random() * 10 - 5;
        //     let z = Math.random() * 10 - 5;
        //     this.makeSphere(x,y,z, red);
        // }

        this.makeLightCube(5,5,5, red);
        
        this.regularSpheres.commit();
        this.regularCubes.commit();
        this.groundCubes.commit()
        this.skyCubes.commit();
        this.lightCubes.commit();

        
    }

    // building generators
    makeTower(x,z, color){
        let size = 1;
        for(let i = 0; i < size+3; i++){
            for(let j = 0; j < size; j++){
                this.regularCubes.addShape(x - size, i, z - j, color);
                this.regularCubes.addShape(x - size, i, z + j, color);
                this.regularCubes.addShape(x + size, i, z - j, color);
                this.regularCubes.addShape(x + size, i, z + j, color);

                this.regularCubes.addShape(x - j, i, z - size, color);
                this.regularCubes.addShape(x - j, i, z + size, color);
                this.regularCubes.addShape(x + j, i, z - size, color);
                this.regularCubes.addShape(x + j, i, z + size, color);
            }
        }
    }

    makeSphere(x,y,z, color){
        this.regularSpheres.addShape(x,y,z, color);
    }
    
    makeRegularCube(x,y,z, color){
        this.regularCubes.addShape(x,y,z, color);
    }
    makeGroundCube(x,y,z, color){
        this.groundCubes.modelMatrix.setScale(50,50,50);
        this.groundCubes.addShape(x,y,z, color);
    }

    makeSkyCube(x,y,z, color){
        this.skyCubes.modelMatrix.setScale(150,150,150);
        this.skyCubes.addShape(x,y,z, color);
    }    

    makeLightCube(x,y,z, color){
        this.lightCubes.modelMatrix.setScale(0.5,0.5,0.5);
        this.lightCubes.addShape(x,y,z, color);
    }

    moveLightCube(x,y,z, color){
        this.lightCubes.clearList();
        this.lightCubes.addShape(x,y,z,color);
        this.lightCubes.commit();
    }

    renderAllShapes(){
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        gl.uniformMatrix4fv(u_ViewMatrix, false, this.camera.viewMatrix.elements);
        gl.uniformMatrix4fv(u_ProjectionMatrix, false, this.camera.projectionMatrix.elements);

        gl.uniform1i(u_NormalViewOn, isNormalViewOn ? 1 : 0);
        gl.uniform1i(u_LightingOn, isLightingOn ? 1 : 0);
        
        gl.uniform3fv(u_LightPos, rawLightPosition);

        gl.uniform3fv(u_CameraPos, this.camera.eye.elements);        
        for(let i = 0; i < this.listOfShapeLists.length; i++){
            this.listOfShapeLists[i].drawAllShapes();
        }
        // this.regularCubes.drawAllShapes();
    }
}


