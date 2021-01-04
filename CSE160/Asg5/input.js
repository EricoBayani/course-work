// file to handle keyboard input

// most of this is stolen from https://github.com/mrdoob/three.js/blob/master/examples/misc_controls_pointerlock.html

let moveForward = false;
let moveBackward = false;
let moveLeft = false;
let moveRight = false;
// let canJump = false;

let prevTime = performance.now();
let direction;
let velocity;

let isPicking = false;


function setUpKeyInput(){
    velocity = new THREE.Vector3();
    direction = new THREE.Vector3();

    const onKeyDown = function ( event ) {

	    switch ( event.keyCode ) {

	    case 38: // up
	    case 87: // w
		    moveForward = true;
		    break;

	    case 37: // left
	    case 65: // a
		    moveLeft = true;
		    break;

	    case 40: // down
	    case 83: // s
		    moveBackward = true;
		    break;

	    case 39: // right
	    case 68: // d
		    moveRight = true;
		    break;

	    case 70: // d
            spotLightOn ? spotLight.intensity = 0 : spotLight.intensity = 1;
            spotLightOn = !spotLightOn;
		    break;

	    case 90: // d
		    video.play();
		    break;
	    case 88: // d
		    video.pause();
		    break;
	    case 67: // d
		    video.currentTime = 0;
            video.play();
		    break;            

	        // case 32: // space
	        // 	if ( canJump === true ) velocity.y += 350;
	        // 	canJump = false;
	        // 	break;

	    }

    };

    const onKeyUp = function ( event ) {

	    switch ( event.keyCode ) {

	    case 38: // up
	    case 87: // w
		    moveForward = false;
		    break;

	    case 37: // left
	    case 65: // a
		    moveLeft = false;
		    break;

	    case 40: // down
	    case 83: // s
		    moveBackward = false;
		    break;

	    case 39: // right
	    case 68: // d
		    moveRight = false;
		    break;

	    }

    };
    
    document.addEventListener( 'keydown', onKeyDown, false );
	document.addEventListener( 'keyup', onKeyUp, false );

    canvas.addEventListener( 'click', function () {
        if(pointerLockEnabled)
		    pointerLockControls.lock();
        if(!isPicking){
            isPicking = true;
            const pos = getCanvasRelativePosition(event);
            pickPosition.x = (pos.x / canvas.width ) *  2 - 1;
            pickPosition.y = (pos.y / canvas.height) * -2 + 1;  // note we flip Y
        }
        else if(isPicking){
            isPicking = false;
            clearPickPosition();
        }

	}, false );
    
}




function handleUserMovement(time){
    const delta = ( time - prevTime ) / 5000;

	velocity.x -= velocity.x * 10.0 * delta;
	velocity.z -= velocity.z * 10.0 * delta;

	velocity.y -= 9.8 * 100.0 * delta; // 100.0 = mass

	direction.z = Number( moveForward ) - Number( moveBackward );
	direction.x = Number( moveRight ) - Number( moveLeft );
	direction.normalize(); // this ensures consistent movements in all directions

	if ( moveForward || moveBackward ) velocity.z -= direction.z * 400.0 * delta;
	if ( moveLeft || moveRight ) velocity.x -= direction.x * 400.0 * delta;


    pointerLockControls.moveRight( - velocity.x * delta );
	pointerLockControls.moveForward( - velocity.z * delta );

    prevTime = time;
}


const pickPosition = {x: 0, y: 0};
clearPickPosition();

function getCanvasRelativePosition(event) {
    const rect = canvas.getBoundingClientRect();
    return {
        x: (event.clientX - rect.left) * canvas.width  / rect.width,
        y: (event.clientY - rect.top ) * canvas.height / rect.height,
    };
}

// function setPickPosition(event) {
//     const pos = getCanvasRelativePosition(event);
//     pickPosition.x = (pos.x / canvas.width ) *  2 - 1;
//     pickPosition.y = (pos.y / canvas.height) * -2 + 1;  // note we flip Y
// }

function clearPickPosition() {
    // unlike the mouse which always has a position
    // if the user stops touching the screen we want
    // to stop picking. For now we just pick a value
    // unlikely to pick something
    pickPosition.x = -100000;
    pickPosition.y = -100000;
}



// window.addEventListener('mousemove', setPickPosition);
// window.addEventListener('mouseout', clearPickPosition);
// window.addEventListener('mouseleave', clearPickPosition);



