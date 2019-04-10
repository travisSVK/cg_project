#version 420

in vec2 in_position;

out vec2 pass_textureCoords;

//position and scale of the quad. Position stored in transform.xy, and scale in transform.zw
uniform vec4 transform;

void main(void){
	
	//calc texture coords based on position
	pass_textureCoords = in_position + vec2(0.5, 0.5);
	//apply position and scale to quad
	vec2 screenPosition = in_position * transform.zw + transform.xy;
	
	//convert to OpenGL coordinate system (with (0,0) in center of screen)
	screenPosition.x = screenPosition.x * 2.0 - 1.0;
	screenPosition.y = screenPosition.y * -2.0 + 1.0;
	gl_Position = vec4(screenPosition, 0.0, 1.0);

}