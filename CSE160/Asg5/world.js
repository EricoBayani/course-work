// file for creating world



class ShapeType {
    constructor(width, length, height, textureSrc){
        this.geometry = new THREE.BoxGeometry(width, length, height); // default is cube
        this.material = new THREE.MeshPhongMaterial({color: 0xf0f0f0});
        if(textureSrc){
            let texture = loader.load(textureSrc);
            texture.magFilter = THREE.LinearFilter;
            
                
            this.material = new THREE.MeshPhongMaterial({map: texture});

            // if (textureSrc == skySrc)
        }
        this.shapes = [];
    }
    setGeometry(geometry){
        this.geometry = geometry;
    }
    setMaterial(material){
        this.material = material;
    }
    addShapeToList(x,y,z, index){
        this.shapes.push({x: x, y: y, z: z, index: index});
    }
}

const meshes = [];

class World {
    constructor(){
        this.scene = new THREE.Scene();


        this.skyMap = new ShapeType(1,1,1);
        this.groundMap = null;
        this.wallMap = new ShapeType(1,1,1, wallSrc);

        this.generateWorld();
    }

    generateWorld(){
        let groundLength = 32;
        let groundWidth = 32;
        // for(let i = -(groundLength/2); i < groundLength/2 + 1; i++){
        //     for(let j = -(groundWidth/2); j < groundWidth/2 + 1; j++){
        //         this.addShape(this.groundMap, i, -1, j);
        //     }
        // }
        this.groundMap = new ShapeType(groundLength, 1, groundWidth, groundSrc);

        let groundTexture = loader.load(groundSrc);
        groundTexture.wrapS = groundTexture.wrapT = THREE.RepeatWrapping; 
	    groundTexture.repeat.set( 10, 10 );
        this.groundMap.setMaterial(new THREE.MeshPhongMaterial({map: groundTexture}));
        
        this.addShape(this.groundMap, 0,-1,0);

        let numTowers = 2;
        for(let i = 0; i < numTowers; i++){
            let posX = i-5;
            let posZ = 8;
            this.makeTower(posX, -1, posZ);
        }

        this.makeTower(-10, -1, -5);
        this.makeTower(10, -1, -5);
        // this.addShape(this.wallMap,0,0,0);

        // this.skyMap = new ShapeType(groundLength*2, groundLength*2, groundLength*2, skySrc);
        // this.addShape(this.skyMap, 0,0,0);
        
        
    }

    addShape(shapeType, x, y, z){
        let shape = new THREE.Mesh(shapeType.geometry, shapeType.material);
        shape.position.x = x;
        shape.position.y = y;
        shape.position.z = z;

        meshes.push(shape);
        shapeType.addShapeToList(x,y,z, meshes.length - 1);
        this.scene.add(shape);
        
    }

    setSkyTexture(imageSrc){
        let skyTexture = cubeLoader.load([imageSrc, imageSrc, imageSrc, imageSrc, imageSrc, imageSrc]);
        this.scene.background = skyTexture;
    }

    // setGroundTexture(imageSrc){
    //     let groundTexture = loader.load(imageSrc);
    //     this.scene.background = groundTexture;
    // }    

    renderWorld(time){
        handleUserMovement(time);
        renderer.render(this.scene, camera);
    }

    makeTower(x,y,z){
        let radius = 1;
        let height = 9;
        for(let i = 0; i < radius+height; i++){
            for(let j = 0; j < radius; j++){
                this.addShape(this.wallMap, x - radius, i, z - j);
                this.addShape(this.wallMap, x - radius, i, z + j);
                this.addShape(this.wallMap, x + radius, i, z - j);
                this.addShape(this.wallMap, x + radius, i, z + j);

                this.addShape(this.wallMap, x - j, i, z - radius);
                this.addShape(this.wallMap, x - j, i, z + radius);
                this.addShape(this.wallMap, x + j, i, z - radius);
                this.addShape(this.wallMap, x + j, i, z + radius);
            }
        }
    }
}
