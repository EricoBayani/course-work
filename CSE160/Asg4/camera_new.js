let forward, backward, left, right, panL, panR;
forward = backward = left = right = panL = panR = false;

class Camera{
    constructor(){
        this.fov = 60.0;
        this.eye = new Vector3([0,0,1]);
        this.at = new Vector3([0,0.001,0]);
        this.up = new Vector3([0,1,0]);
        this.viewMatrix = new Matrix4();
        this.updateView();
        this.projectionMatrix = new Matrix4();
        this.projectionMatrix.setPerspective(this.fov, canvas.width/canvas.height, 0.1, 1000);
        this.updateView();
    }


    move(forward, backward, left, right, panR, panL){
        let direction = new Vector3([0,0,0]);
        let side = new Vector3([0,0,0]);
        if (forward && !backward){
            direction.set(this.at);
            direction.sub(this.eye);
            if (left && !right){
                side = Vector3.cross(this.up,direction);
                this.readyVector(side);
            }
            else if (right && !left){
                side = Vector3.cross(direction,this.up);
                this.readyVector(side);
            }            
            this.readyVector(direction);
        }
        else if (backward && !forward){
            direction.set(this.eye);
            direction.sub(this.at);
            if (left && !right){
                side = Vector3.cross(direction,this.up);
                this.readyVector(side);
            }
            else if (right && !left){
                side = Vector3.cross(this.up,direction);
                this.readyVector(side);
            }            
            this.readyVector(direction);
        }
        else{
            direction.set(this.at);
            direction.sub(this.eye);
            if (left && !right){
                side = Vector3.cross(this.up,direction);
                this.readyVector(side);
            }
            else if (right && !left){
                side = Vector3.cross(direction,this.up);
                this.readyVector(side);
            }            
            // this.readyVector(direction);
            this.eye.add(side);
            this.at.add(side);



            if(panR == true){
                this.panRight();
            }
            else if(panL == true){
                this.panLeft();
            }
            this.updateView();
            return;
        }
        
       
        // logic for panning goes here sometime
        direction.add(side);

        this.eye.add(direction);
        this.at.add(direction);

        if(panR == true){
            this.panRight();
        }
        else if(panL == true){
            this.panLeft();
        }
        this.updateView();
        
    }

    readyVector(vector){
        vector.normalize();
        vector.mul(speed);
    }
    

    panLeft(){
        let f = new Vector3();
        f.set(this.at);
        f.sub(this.eye);
        let rotationMatrix = new Matrix4();
        rotationMatrix.setRotate(rotateBy,
                                 this.up.elements[0],
                                 this.up.elements[1],
                                 this.up.elements[2]);
        let f_prime = rotationMatrix.multiplyVector3(f);
        let toAdd = new Vector3();
        toAdd.set(this.eye);
        toAdd.add(f_prime);
        this.at.set(toAdd);
    }
    panRight(){
        let f = new Vector3();
        f.set(this.at);
        f.sub(this.eye);
        let rotationMatrix = new Matrix4();
        rotationMatrix.setRotate(-rotateBy,
                                 this.up.elements[0],
                                 this.up.elements[1],
                                 this.up.elements[2]);
        let f_prime = rotationMatrix.multiplyVector3(f);
        let toAdd = new Vector3();
        toAdd.set(this.eye);
        toAdd.add(f_prime);
        this.at.set(toAdd);
    }

    updateView(){
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    }
}
