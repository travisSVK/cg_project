#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D frameBufferTexture;
uniform float time;
layout(location = 0) out vec4 fragmentColor;


float offsets9[9] = float[9](-7.302940716, -5.35180578, -3.403984807, -1.458429517, 0.0, 1.458429517, 3.403984807, 5.35180578, 7.302940716);
float offsets7[7] = float[7](-5.35180578, -3.403984807, -1.458429517, 0.0, 1.458429517, 3.403984807, 5.35180578);
float offsets5[5] = float[5](-3.403984807, -1.458429517, 0.0, 1.458429517, 3.403984807);
float offsets3[3] = float[3](-1.458429517, 0.0, 1.458429517);
float weights9[9] = float[9](0.011954, 0.044953, 0.115735, 0.204083, 0.246551, 0.204083, 0.115735, 0.044953, 0.011954);
float weights7[7] = float[7](0.046054, 0.118569, 0.209081, 0.25259, 0.209081, 0.118569, 0.046054);
float weights5[5] = float[5](0.130598, 0.230293, 0.278216, 0.230293, 0.130598);
float weights3[3] = float[3](0.311711, 0.376577, 0.311711);

/**
* Helper function to sample with pixel coordinates, e.g., (511.5, 12.75)
* This functionality is similar to using sampler2DRect.
* TexelFetch only work with integer coordinates and do not perform bilinerar filtering.
*/
vec4 textureRect(in sampler2D tex, vec2 rectangleCoord)
{
	return texture(tex, rectangleCoord / textureSize(tex, 0));
}

/**
 * Implements the vertical part of a 17 tap separable gaussian blur, using standard deviation of 3.0, calculated
 * using the formluas and excel sheet from  this blog: http://theinstructionlimit.com/gaussian-blur-revisited-part-two
 * The weights have been normalized to make the kernel not brighten or darken the image.
 */
void main() 
{
	vec4 result = vec4(0.0);
	// do a 17 tap blur by sampling a biliniarly filtered texture.
	vec4 pixel = textureRect(frameBufferTexture, gl_FragCoord.xy);
    float coc = pixel.w;
    int finalNumOfSamples = 0;
    if(coc == 0.0f)
    {
        // blur also if near field values are nearby
        // first check left neighbour pixels up to 4 (max coc value)
        
        for(int i = 3; i >= 0; i--)
        {
            vec4 neighbor = textureRect(frameBufferTexture, gl_FragCoord.xy + vec2(0.0, offsets9[i]));
            if(neighbor.w > 0.0f)
            {
                int numOfSamples = (2 * int(coc / 2.0f)) + 1;
                if(numOfSamples < 3)
                {
                    numOfSamples = 3;
                }
                // if the distance is not more than the coc of the pixel, blur with the same coc length
                if(numOfSamples >= (i + (3 * (3 - i))))
                {
                    finalNumOfSamples = numOfSamples;
                    break;
                }
            }
        }
        for(int i = 5; i < 9; i++)
        {
            vec4 neighbor = textureRect(frameBufferTexture, gl_FragCoord.xy + vec2(0.0, offsets9[i]));
            if(neighbor.w > 0.0f)
            {
                int numOfSamples = (2 * int(coc / 2.0f)) + 1;
                if(numOfSamples < 3)
                {
                    numOfSamples = 3;
                }
                // if the distance is not more than the coc of the pixel, blur with the same coc length
                if((numOfSamples >= (i + (i-7))))
                {
                    if(numOfSamples > finalNumOfSamples)
                    {
                        finalNumOfSamples = numOfSamples;
                    }
                    break;
                }
            }
        }
    }
    else
    {
        float cocTmp = coc;
        if(coc < 0.0f)
        {
            cocTmp = -1.0f * coc;
        }
        // round the CoC to the nearest odd number to use in kernel
        finalNumOfSamples = (2 * int(cocTmp / 2.0f)) + 1;
        if(finalNumOfSamples < 3)
        {
            finalNumOfSamples = 3;
        }
    }
    if(finalNumOfSamples == 0)
    {
        result = vec4(pixel.xyz, 1.0);
    }
    else
    {
        float weights[9];
        float offsets[9];
        for(int i = 0; i < finalNumOfSamples; i++)
        {
            if(finalNumOfSamples == 3)
            {
                weights[i] = weights3[i];
                offsets[i] = offsets3[i];
            }
            else if(finalNumOfSamples == 5)
            {
                weights[i] = weights5[i];
                offsets[i] = offsets5[i];
            }
            else if(finalNumOfSamples == 7)
            {
                weights[i] = weights7[i];
                offsets[i] = offsets7[i];
            }
            else if(finalNumOfSamples == 9)
            {
                weights[i] = weights9[i];
                offsets[i] = offsets9[i];
            }
        }
        if(coc < 0.0f)
        {
            for (int i = 0; i < finalNumOfSamples; ++i)
            {
	        	result += vec4(textureRect(frameBufferTexture, gl_FragCoord.xy + vec2(0.0, offsets[i])).xyz * weights[i], 1.0);
	        }
        }
	    else
        {
            for (int i = 0; i < finalNumOfSamples; ++i)
            {
                vec4 neighbor = textureRect(frameBufferTexture, gl_FragCoord.xy + vec2(0.0, offsets[i]));
                if(neighbor.w > 0.0f)
                {
                    result += vec4(neighbor.xyz * weights[i], 1.0);
                }
                else
                {
                    result += vec4(pixel.xyz * weights[i], 1.0);
                }
	        }
        }
    }
	fragmentColor = result;
}
