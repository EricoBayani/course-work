// Shapes class file


// This class uses global variables found in asg1.js
class Shape {
    
    constructor(xy, color, size) {
        this.xy = xy;
        this.color = color;
        this.size = size;
    }

    render(){
        
        gl.vertexAttrib3f(a_Position, this.xy[0], this.xy[1], 0.0);
        gl.vertexAttrib1f(a_PointSize, this.size);
        gl.uniform4f(u_FragColor, this.color[0], this.color[1], this.color[2], this.color[3]);
        
        gl.drawArrays(gl.POINTS, 0, 1);
    }
    
}
