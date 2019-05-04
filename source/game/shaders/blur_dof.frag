//#version 420
// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D frameBufferTexture;
layout(location = 0) out vec4 focusFarResult;
layout(location = 1) out vec4 nearResult;

#ifdef HORIZONTAL
	const vec2 direction = vec2(1, 0);
#else
	const vec2 direction = vec2(0, 1);
    /** For the second pass, the output of the previous near-field blur pass. */
	layout(binding = 1) uniform sampler2D nearSourceBuffer;
#endif

vec4 textureRect(in sampler2D tex, vec2 rectangleCoord)
{
	return texture(tex, rectangleCoord / textureSize(tex, 0));
}

bool inNearField(float radiusPixels) {
    return radiusPixels > 0.0f;
}

void main() 
{
    const int KERNEL_TAPS = 6;
    float kernel[KERNEL_TAPS + 1];
    
    // 11 x 11 separated kernel weights.  This does not dictate the 
    // blur kernel for depth of field; it is scaled to the actual
    // kernel at each pixel.
	//    Custom disk-like // vs. Gaussian
    kernel[6] = 0.00; //0.00000000000000;  // Weight applied to outside-radius values
    kernel[5] = 0.50;//0.04153263993208;
    kernel[4] = 0.60;//0.06352050813141;
    kernel[3] = 0.75;//0.08822292796029;
    kernel[2] = 0.90;//0.11143948794984;
    kernel[1] = 1.00;//0.12815541114232;
    kernel[0] = 1.00;//0.13425804976814;

	focusFarResult = vec4(0.0);
    float blurWeightSum = 0.0f;
    
    nearResult = vec4(0.0f);
    float nearWeightSum = 0.000f;
    
    // Color of this pixel
    vec4 pixel = textureRect(frameBufferTexture, gl_FragCoord.xy);
    float cocRadius = pixel.w;
    
    int maxCoCRadius = 4; // max possible value is 4 TODO take as input from cpu
    // Map radius of this pixel - radius << 0 to 0, radius >> 0 to 1
    float smallestRadius = 1.0f / (70.0f - 10.0f) * maxCoCRadius; // for now hardcoded as 4 / nearMax - nearMin * maxCoCRadius TODO take as input from cpu
    float multiple = 1.0f / smallestRadius;
    float nearFieldnessThis = clamp(cocRadius * multiple, 0, 1);

    // Maximum blur radius for any point in the scene, in pixels.
       
    for (int offset = -maxCoCRadius; offset <= maxCoCRadius; ++offset)	
    {
        vec4 neighbor = textureRect(frameBufferTexture, gl_FragCoord.xy + (direction * offset));
        float neighborCocRadius = neighbor.w;

        // Compute blurry buffer
        float weight = 0.0;
        float wNormal  = 
            // Only consider mid- or background pixels (allows inpainting of the near-field)
            float(! inNearField(neighborCocRadius)) * 
            
            // Only blur B over A if B is closer to the viewer (allow 0.5 pixels of slop, and smooth the transition)
			// This term avoids "glowy" background objects but leads to aliasing under 4x downsampling
            // clamp(abs(cocRadius) - abs(neighborCocRadius) + 1.5, 0, 1) *
            
            // Blur depending on distance + neighbor coc radius
            kernel[clamp(int(float(abs(offset) * (KERNEL_TAPS - 1)) / (0.001 + abs(neighborCocRadius * 0.8))), 0, KERNEL_TAPS)];

        weight = mix(wNormal, 1.0, nearFieldnessThis);

        // far + mid-field output 
        blurWeightSum += weight;
        focusFarResult.rgb += neighbor.rgb * weight;

        // Compute near field blurry buffer
        vec4 nearInput;
#ifdef HORIZONTAL
        // On the first pass, extract coverage from the near field radius
        // Note that the near field gets a box-blur instead of a kernel 
        // blur; we found that the quality improvement was not worth the 
        // performance impact of performing the kernel computation here as well.

        // Curve the contribution based on the radius.  We tuned this particular
        // curve to blow out the near field while still providing a reasonable
        // transition into the focus field.
        nearInput.a = float(abs(offset) <= neighborCocRadius) * (neighborCocRadius - abs(offset)) / maxCoCRadius;
	    // Optionally increase edge fade contrast in the near field
	    //nearInput.a *= nearInput.a; nearInput.a *= nearInput.a;

        // Compute premultiplied-alpha color
        nearInput.rgb = neighbor.rgb * nearInput.a;
#else
        // On the second pass, use the already-available alpha values
        nearInput = textureRect(nearSourceBuffer, gl_FragCoord.xy + (direction * offset));
#endif

        // We subsitute the following efficient expression for the more complex: weight = kernel[clamp(int(float(abs(delta) * (KERNEL_TAPS - 1)) * invNearBlurRadiusPixels), 0, KERNEL_TAPS)];
        weight = float(abs(offset) <= neighborCocRadius);
        nearResult += nearInput * weight;
        nearWeightSum += weight;
    }
    
#ifdef HORIZONTAL
        // Retain the coc radius on the first pass.  On the second pass it is not needed.
        focusFarResult.a = cocRadius;
#else
        focusFarResult.a = 1.0;
#endif

    // Normalize the blur
    focusFarResult.rgb /= blurWeightSum;
    nearResult /= max(nearWeightSum, 0.00001);
}
