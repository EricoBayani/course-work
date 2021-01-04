// World Vars


class World {
    constructor(sky, ground){
        this.sky = sky; // skybox texture
        this.ground = ground; // ground texture
        this.world = [];

        for(let i = 0; i < 5; i++){
            this.world.push(new Cube(((i%2)==0?red:blue), i,0,0));
        }
        for(let i = 0; i < 5; i++){
            this.world.push(new Cube(((i%2)==0?red:blue), -i,0,0));
        }
        for(let i = 0; i < 5; i++){
            this.world.push(new Cube(((i%2)==0?red:blue), -4,0,i));
        }
        for(let i = 0; i < 5; i++){
            this.world.push(new Cube(((i%2)==0?red:blue), 4,0,i));
        }    
        this.camera = new Camera();

        console.log('world generation done');
    }
    
}
