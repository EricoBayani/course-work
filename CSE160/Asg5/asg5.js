
// == Globals ==


// HTML vars
let canvas;

let out_fps;
// GL vars

function setUpThreejs(){
    // Retrieve <canvas> element
    canvas = document.getElementById('webgl');

}

function setUpDocElements(){
    out_fps = document.getElementById('fps');

}

function connectDocElementsToHandlers(){

}

function main() {

    setUpThreejs();
    setUpDocElements();


    connectDocElementsToHandlers();    

    refreshLoop(); 
    setupInputEvents();
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
        let g_seconds = now/1000;

        refreshLoop();
    });
}
