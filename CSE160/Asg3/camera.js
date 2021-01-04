class Camera{
    constructor(){
        this.fov = 60.0;
        this.eye = new Vector3([0,0,1]);
        this.at = new Vector3([0,0.001,0]);
        this.up = new Vector3([0,1,0]);
        this.viewMatrix = new Matrix4();
        
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
        // console.log(this.viewMatrix.elements.toString());
        console.log(this.viewMatrix.elements.toString());
        this.projectionMatrix = new Matrix4();
        this.projectionMatrix.setPerspective(this.fov, canvas.width/canvas.height, 0.1, 1000);
    }
    moveForward(){
        let f = new Vector3();
        f.set(this.at);
        f.sub(this.eye);
        f.normalize();
        f.mul(speed);
        this.eye.add(f);
        this.at.add(f);
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    
        return this;
    }
    moveBackward(){
        let b = new Vector3();
        b.set(this.eye);
        b.sub(this.at);
        b.normalize();
        b.mul(speed);
        this.eye.add(b);
        this.at.add(b);
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    
        return this;
    }
    moveRight(){
        let r = new Vector3();
        r.set(this.at);
        r.sub(this.eye);
        let s = Vector3.cross(r,this.up);
        s.normalize();
        s.mul(speed);
        this.eye.add(s);
        this.at.add(s);
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    
        return this;
    }
    moveLeft(){
        let l = new Vector3();
        l.set(this.at);
        l.sub(this.eye);
        let s = Vector3.cross(this.up,l);
        s.normalize();
        s.mul(speed);
        this.eye.add(s);
        this.at.add(s);
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    
        return this;
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
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    
        return this;
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
        this.viewMatrix.setLookAt(this.eye.elements[0],this.eye.elements[1],this.eye.elements[2],
                                  this.at.elements[0],this.at.elements[1],this.at.elements[2],
                                  this.up.elements[0],this.up.elements[1],this.up.elements[2]);
    }
}
