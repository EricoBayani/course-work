// == Globals ==

// HTML vars
let canvas;

let out_fps;
let in_controlToggle;
let in_rotatingLightToggle;
let in_ambientLightToggle;

// = three.js vars =
let renderer;
// camera vars
const fov = 75;
const aspect = 1;  // maybe calculate later using known dimensions
const near = 0.000001;
const far = 10000;
let camera;
let cameraHelper;

let cameraDirection; // must me initialized to empty vector before use
// control vars
let orbitControls;
let pointerLockControls;
let pointerLockEnabled = true;
let controls;

// scene vars
let scene;

// geometry vars
let boxWidth = 1;
let boxHeight = 1;
let boxDepth = 1;
// world vars
let world;

// light vars
let rotatingColor = 0xE0EB1E;
let rotatingLightIntensity = 1;
let rotatingLight;
let rotatingLightOn = false;

let ambientColor = 0xFFFFFF;
let ambientLightIntensity = 1;
let ambientLight;
let ambientLightOn = true;

let spotColor = 0xe31b1b;
let spotLightIntensity = 1;
let spotLight;
let spotLightOn = false;

// textures
let loader;
let cubeLoader;
let wallSrc = './Textures/blocks.jpg';
let groundSrc = './Textures/sand_resized.jpg';
let skySrc = './Textures/dark_sky.jpg';

// videoCanvas
let video;
let videoImage;
let videoContext;
let videoTexture;
let videoSrc = './Videos/DancingSkeleton.mp4'
// models
let objLoader;
let mtlLoader;
let donkeyOBJSrc = './Models/Donkey/Donkey.obj';
let donkeyObject;

function setUpThreejs(){
    // Retrieve <canvas> element
    canvas = document.getElementById('three');
    renderer = new THREE.WebGLRenderer({canvas, alpha:true, logarithmicDepthBuffer: true,});
    
    loader = new THREE.TextureLoader();
    cubeLoader = new THREE.CubeTextureLoader();
    
}

function setUpDocElements(){
    out_fps = document.getElementById('fps');
    in_controlToggle = document.getElementById('controlsToggle');
    in_rotatingLightToggle = document.getElementById('rotatingLightToggle');
    in_ambientLightToggle = document.getElementById('ambientLightToggle');
}


function connectDocElementsToHandlers(){
    in_controlToggle.onclick =
        function()    {
        toggleControls();
    };
    
    in_rotatingLightToggle.onclick =
        function()    {
        rotatingLightOn ? world.scene.remove(rotatingLight) : world.scene.add(rotatingLight);
        rotatingLightOn = !rotatingLightOn;
    };

    in_ambientLightToggle.onclick =
        function()    {
        ambientLightOn ? world.scene.remove(ambientLight) : world.scene.add(ambientLight);
        ambientLightOn = !ambientLightOn;
    };    
}

function setUpCamera(){
    camera = new THREE.PerspectiveCamera(fov, aspect, near, far);
    camera.position.z = 2;

    cameraDirection = new THREE.Vector3();

    
}

function setUpControls(){
    orbitControls = new THREE.OrbitControls(camera, renderer.domElement);
    orbitControls.enableKeys = true;
    
    orbitControls.enableDamping = true;
	orbitControls.dampingFactor = 0.05;
    
    orbitControls.enablePan = true;
    orbitControls.update();
    orbitControls.enabled = false;
    
    pointerLockControls = new THREE.PointerLockControls(camera, canvas);
    // pointerLockControls.maxPolarAngle = pointerLockControls.maxPolarAngle*1;




    setUpKeyInput();
    
}

function toggleControls(){
    orbitControls.enabled = !orbitControls.enabled;
    pointerLockEnabled = !pointerLockEnabled;

}

function setUpLights(){
    rotatingLight = new THREE.DirectionalLight(rotatingColor, rotatingLightIntensity);
    rotatingLight.position.set(-1, 2, 4);
    world.scene.add(rotatingLight);

    ambientLight = new THREE.AmbientLight(ambientColor, ambientLightIntensity);    
    world.scene.add(ambientLight);

    spotLight = new THREE.SpotLight(spotColor, spotLightIntensity);
    spotLight.angle = Math.PI/12;
    spotLight.penumbra = 1;
    spotLight.position.set(camera.position.x, camera.position.y, camera.position.z);

    cameraDirection = pointerLockControls.getDirection(cameraDirection);
    spotLight.target.position.set(cameraDirection.x, cameraDirection.y, cameraDirection.z);
    world.scene.add(spotLight);
    world.scene.add(spotLight.target);
}


// I needed to adapt https://threejs.org/examples/webgl_loader_obj_mtl.html
// because the tutorial uses OBJLoader2, which I can't use
function setUpModels(){

    const onProgress = function ( xhr ) {
        if ( xhr.lengthComputable ) {
            const percentComplete = xhr.loaded / xhr.total * 100;
            console.log( Math.round( percentComplete, 2 ) + '% downloaded' );
        }
    };
    const onError = function () { };
    const manager = new THREE.LoadingManager();
    mtlLoader = new THREE.MTLLoader( manager )
        .setPath( 'Models/Donkey/' )
        .load( 'Donkey.mtl', function ( materials ) {
            materials.preload();
            objLoader = new THREE.OBJLoader( manager )
                .setMaterials( materials )
                .setPath( 'Models/Donkey/' )
                .load( 'Donkey.obj', function ( object ) {
                    object.position.x = -0.5;
                    object.position.y = -0.5;
                    object.scale.set(0.1,0.1,0.1);
                    object.updateMatrixWorld();
                    object.name = 'donkey';
                    donkeyObject = object;
                    world.scene.add( donkeyObject );
                }, onProgress, onError );
        } );
}
// video embedding into Three.js from  https://stemkoski.github.io/Three.js/Video.html
function setUpVideo(){
    video = document.createElement( 'video' );
    video.src = videoSrc;
	video.load(); // must call after setting/changing source
	// video.play();

    videoTexture = new THREE.VideoTexture( video );

    var movieMaterial = new THREE.MeshBasicMaterial( { map: videoTexture,  side:THREE.DoubleSide } );

	var movieGeometry = new THREE.PlaneGeometry( 12, 5, 4, 4 );
	var movieScreen = new THREE.Mesh( movieGeometry, movieMaterial );
	movieScreen.position.set(0,3,-5);
	world.scene.add(movieScreen);
    
}

let raycaster;
let intersectedObject;
let pickedObject = null;
let normalizedCameraToObject;
let pickedObjectSavedColor = 0;
let pickDirection;

function setUpPicker(){
    raycaster = new THREE.Raycaster();
    pickDirection = new THREE.Vector3();    
}

function pick(normalizedPosition, scene, camera, time) {

    if (isPicking){
        // restore the color if there is a picked object
        if (pickedObject){
            raycaster.setFromCamera(normalizedPosition, camera);            
            raycaster.ray.at(4, pickDirection);
            // let savedScale = pickedObject.scale;
            // pickedObject.scale = new THREE.Vector3(1,1,1);
            pickedObject.position.set(pickDirection.x, pickDirection.y, pickDirection.z);
            if(typeof pickedObject.material.emissive !== 'undefined'){
                // set its emissive color to flashing red/yellow
                
                pickedObject.material.emissive.setHex((time * 8) % 2 > 1 ? 0xFFFF00 : 0xFF0000);
            }
            return;
        }
        
        // cast a ray through the frustum
        raycaster.setFromCamera(normalizedPosition, camera);
        // get the list of objects the ray intersected
        const intersectedObjects = raycaster.intersectObjects(scene.children, true);
        if (intersectedObjects.length) {
            // pick the first object. It's the closest one
            intersectedObject = intersectedObjects[0];            
            pickedObject = intersectedObjects[0].object;
            // save its color
            if(typeof pickedObject.material.emissive !== 'undefined'){
                pickedObjectSavedColor = pickedObject.material.emissive.getHex();
                pickedObject.material.emissive.setHex((time * 8) % 2 > 1 ? 0xFFFF00 : 0xFF0000);
            }
            
            // normalizedCameraToObject = 
            
        }
    }
    else if (!isPicking){
        if(pickedObject){
            if(typeof pickedObject.material.emissive !== 'undefined')
                pickedObject.material.emissive.setHex(pickedObjectSavedColor);
        }
        pickedObject = null;
        
    }
    
}

function calculateDisplacements(time){
    let goodTime = time/1000;
    rotatingLight.position.set(Math.sin(goodTime), 2, Math.cos(goodTime));
}



function renderAll(time){


    spotLight.position.set(camera.position.x, camera.position.y, camera.position.z);
    cameraDirection = pointerLockControls.getDirection(cameraDirection);
    cameraDirection.add(camera.position);
    spotLight.target.position.set(cameraDirection.x, cameraDirection.y, cameraDirection.z);
    spotLight.target.updateMatrixWorld();

    pick(pickPosition, world.scene, camera, time);
    
    calculateDisplacements(time);
    world.renderWorld(time);
}

// shamelessly stolen from https://www.growingwiththeweb.com/2017/12/fast-simple-js-fps-counter.html
const times = [];
let fps;

function refreshLoop() {
    window.requestAnimationFrame(() => {
        const now = performance.now();
        while (times.length > 0 && times[0] <= now - 1000) {
            times.shift();
        }
        times.push(now);
        fps = times.length;
        out_fps.value = fps;
        
        // my own stuff

        renderAll(now);
        refreshLoop();
    });

}

function main() {
    setUpThreejs();
    setUpDocElements();
    connectDocElementsToHandlers();    

    // camera.position.z = 2;
    world = new World();
    setUpCamera()  
    setUpControls();   
    setUpLights();
    setUpModels();
    setUpVideo();
    setUpPicker();

    world.setSkyTexture(skySrc);

    refreshLoop(); 
}
 
