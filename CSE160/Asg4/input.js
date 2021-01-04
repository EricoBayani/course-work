// file containing code to handle user input

// keepMoving: forward, backward, left, right, panRight, panLeft
// let keys = [];
let keepMoving = [false,false,false,false,false,false];
let movingAtAll = false;
let movingOne, movingTwo;
movingOne = movingTwo = false;
let mouseMoving = false;
let mouseX = 0;
function setupInputEvents(){
    document.addEventListener('keydown',(event) => {

        let allMoving = keepMoving.filter(value => value == true);
        if(!keepMoving.includes(true)){
            if((keepMoving[0] == false) && event.key == 'w'){
                keepMoving[0] = true;
                movingAtAll = true;
                move();
            }
            if((keepMoving[1] == false) && event.key == 's'){
                keepMoving[1] = true;
                movingAtAll = true;
                move();
            }
            if((keepMoving[2] == false) && event.key == 'a'){
                keepMoving[2] = true;
                movingAtAll = true;
                move();
            }
            if((keepMoving[3] == false) && event.key == 'd'){
                keepMoving[3] = true;
                movingAtAll = true;
                move();
            }
            if((keepMoving[4] == false) && event.key == 'e'){
                keepMoving[4] = true;
                movingAtAll = true;
                move();
            }
            if((keepMoving[5] == false) && event.key == 'q'){
                keepMoving[5] = true;
                movingAtAll = true;
                move();
            }
        }

        else if(keepMoving.includes(true)){
            if((keepMoving[0] == false) && event.key == 'w'){
                keepMoving[0] = true;
            }
            if((keepMoving[1] == false) && event.key == 's'){
                keepMoving[1] = true;
            }
            if((keepMoving[2] == false) && event.key == 'a'){
                keepMoving[2] = true;
            }
            if((keepMoving[3] == false) && event.key == 'd'){
                keepMoving[3] = true;
            }
            if((keepMoving[4] == false) && event.key == 'e'){
                keepMoving[4] = true;  
            }
            if((keepMoving[5] == false) && event.key == 'q'){
                keepMoving[5] = true;
                
            }
        }
        
    }, false);

    document.addEventListener('keyup',(event) => {
        let allMoving = keepMoving.filter(value => value == true);
        
        if(event.key == 'w'){
            keepMoving[0] = false;
        }
        if(event.key == 's'){
            keepMoving[1] = false;
        }
        if(event.key == 'a'){
            keepMoving[2] = false;
        }
        if(event.key == 'd'){
            keepMoving[3] = false;
        }
        if(event.key == 'e'){
            keepMoving[4] = false;
        }
        if(event.key == 'q'){
            keepMoving[5] = false;
        }
        if (!keepMoving.includes(true)){
            movingAtAll = false;
        }
        // }
    }, false);

    // https://github.com/mdn/dom-examples/tree/master/pointer-lock
    // pointer lock object forking for cross browser

    canvas.requestPointerLock = canvas.requestPointerLock ||
        canvas.mozRequestPointerLock;

    document.exitPointerLock = document.exitPointerLock ||
        document.mozExitPointerLock;

    canvas.onclick = function() {
        canvas.requestPointerLock();
    };
    // pointer lock event listeners
    // Hook pointer lock state change events for different browsers
    document.addEventListener('pointerlockchange', lockChangeAlert, false);
    document.addEventListener('mozpointerlockchange', lockChangeAlert, false);    
}


function lockChangeAlert() {
    if (document.pointerLockElement === canvas ||
        document.mozPointerLockElement === canvas) {
        console.log('The pointer lock status is now locked');
        canvas.addEventListener("mousemove", moveWithMouse, false);
    } else {
        console.log('The pointer lock status is now unlocked');  
        canvas.removeEventListener("mousemove", moveWithMouse, false);
    }
}

let moveThreshhold = 1;
function moveWithMouse(event){
    let x = event.movementX;
    document.getElementById('mouseX').value = x;
    if(moveThreshhold < x){
        keepMoving[4] = true;
        if(!movingAtAll){
            movingAtAll = true;
            move();
        }
        mouseMoving = true;
    }
    else if(-moveThreshhold > x){
        keepMoving[5] = true;
        if(!movingAtAll){
            movingAtAll = true;
            move();
        }
        mouseMoving = true;
    }
    else{
        keepMoving[4] = keepMoving[5] = false;
        mouseMoving = false;
        if(!keepMoving.includes(true)){
            movingAtAll = false;
        }
    }
    mouseX = x;
}

let moveTimeoutIntervalKeys = 30;
let moveTimeoutIntervalMouse = 20;
function move(){
    if(movingAtAll){

        world.camera.move(...keepMoving);
        // console.log(keepMoving);
        world.renderAllShapes();
        
        setTimeout('move()', mouseMoving ? moveTimeoutIntervalMouse : moveTimeoutIntervalKeys);
    }
}
