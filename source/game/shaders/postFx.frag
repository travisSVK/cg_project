#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D frameBufferTexture;
layout(binding = 1) uniform sampler2D blurredFrameBufferTexture;
layout(binding = 2) uniform sampler2D depthBufferTexture;
layout(binding = 3) uniform sampler2D bloomTexture;
layout(binding = 4) uniform sampler2D focusFarDoFBuffer;
layout(binding = 5) uniform sampler2D nearDoFBuffer;
layout(binding = 6) uniform sampler2D lensFlareTexture;
layout(binding = 7) uniform sampler2D lensDirtTexture;
layout(binding = 8) uniform sampler2D starburstTexture;

uniform float time;
uniform int currentEffect;
uniform int filterSize;
uniform int numSamples;
uniform int maxCocRadius;
uniform float uGlobalBrightness;
uniform float uStarburstOffset;

layout(location = 0) out vec4 fragmentColor;
uniform mat4 viewProjectionInverseMatrix;
uniform mat4 previousViewProjectionMatrix;

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
 * Perturps the sampling coordinates of the pixel and returns the new coordinates
 * these can then be used to sample the frame buffer. The effect uses a sine wave to make us
 * feel woozy.
 */
vec2 mushrooms(vec2 inCoord);

/**
 * Samples a region of the frame buffer using gaussian filter weights to blur the image
 * as the kernel width is not that large, it doesnt produce a very large effect. Making it larger
 * is both tedious and expensive, for real time purposes a separable blur is preferable, but this
 * requires several passes.
 * takes as input the centre coordinate to sample around.
 */
vec3 blur(vec2 coord);

/**
 * Simply returns the luminance of the input sample color.
 */
vec3 grayscale(vec3 rgbSample);

/**
 * Converts the color sample to sepia tone (by transformation to the yiq color space).
 */
vec3 toSepiaTone(vec3 rgbSample);

vec3 mosaic(vec2 coord);

vec3 separableBlur(vec2 coord);

vec3 motionBlur(vec2 coord);

vec3 bloom(vec2 coord);

vec3 dof(vec2 coord);

vec3 lensFlare(vec2 coord);

void main()
{
	switch (currentEffect)
	{
	case 0:
		fragmentColor = textureRect(frameBufferTexture, gl_FragCoord.xy);
		break;
	case 1:
		fragmentColor = vec4(toSepiaTone(textureRect(frameBufferTexture, gl_FragCoord.xy).xyz), 1.0);
		break;
	case 2:
		fragmentColor = textureRect(frameBufferTexture, mushrooms(gl_FragCoord.xy));
		break;
	case 3:
		fragmentColor = vec4(blur(gl_FragCoord.xy), 1.0);
		break;
	case 4:
		fragmentColor = vec4(grayscale(textureRect(frameBufferTexture, gl_FragCoord.xy).xyz), 1.0);
		break;
	case 5:
		// all at once
		fragmentColor = vec4(toSepiaTone(blur(mushrooms(gl_FragCoord.xy))), 1.0);
		break;
	case 6:
		fragmentColor = vec4(mosaic(gl_FragCoord.xy), 1.0);
		break;
	case 7:
		fragmentColor = vec4(separableBlur(gl_FragCoord.xy), 1.0);
		break;
	case 8:
		fragmentColor = vec4(bloom(gl_FragCoord.xy), 1.0);
		break;
    case 9:
		fragmentColor = vec4(motionBlur(gl_FragCoord.xy), 1.0);
		break;
    case 10:
		fragmentColor = vec4(dof(gl_FragCoord.xy), 1.0);
		break;
    case 11:
	    fragmentColor = vec4(lensFlare(gl_FragCoord.xy), 1.0);
	    break;
    default:
        fragmentColor = textureRect(frameBufferTexture, gl_FragCoord.xy);
        break;
	}
}

vec3 motionBlur(vec2 coord)
{
    vec2 texCoord = coord / textureSize(depthBufferTexture, 0);
    float zOverW = texture(depthBufferTexture, texCoord).x;
    // H is the viewport position at this pixel in the range -1 to 1.
    vec4 currentPos = vec4(vec2((texCoord.x * 2.0) - 1.0, (1.0 - texCoord.y) * 2.0 - 1.0), zOverW, 1.0);
    // Transform by the view-projection inverse.
    vec4 D =  viewProjectionInverseMatrix * currentPos;
    // Divide by w to get the world position.
    vec4 worldPos = D / D.w;
    // Use the world position, and transform by the previous view-projection matrix.
    vec4 previousPos = previousViewProjectionMatrix * worldPos;
    previousPos /= previousPos.w;
    // get previous screen space position:
    //previousPos.xy = previousPos.xy * 0.5 + 0.5;

    // Use this frame's position and last frame's to compute the pixel velocity.
    vec2 velocity = (currentPos - previousPos).xy/2.0;
    //vec2 velocity = previousPos.xy - texCoord;
    vec3 color = texture(frameBufferTexture, texCoord).xyz;

    //test
    //color = texture(frameBufferTexture, vec2((previousPos.x + 1.0) / 2.0, (previousPos.y + 1.0) / 2.0)).xyz;
    texCoord += velocity;
    int realNumSamples = numSamples;
    for(int i = 1; i < numSamples; ++i, texCoord += velocity)
    {
        // get offset in range [-0.5, 0.5]:
        //vec2 offset = velocity * (float(i) / float(numSamples - 1) - 0.5);
  
        // sample & add to result:
        //color += texture(frameBufferTexture, texCoord + offset).xyz;

        // Sample the color buffer along the velocity vector.
        //vec2 texCoordClamped = clamp(texCoord, 0, 1);
        if(texCoord.x < 0.0 || texCoord.x > 1.0 || texCoord.y < 0.0 || texCoord.y > 1.0)
        {
            --realNumSamples;
            continue;
        }
        vec3 currentColor = texture(frameBufferTexture, texCoord).xyz;
//        // Add the current color to our color sum.
        color += currentColor;
    }
    return color / float(realNumSamples);
    //return color;
}

vec3 mosaic(vec2 coord)
{
    vec2 newCoord = coord / textureSize(frameBufferTexture, 0);
    vec2 flooredCoord = floor(newCoord * 100);
    vec2 finalCoord = flooredCoord / 100;
//    finalCoord.x = clamp(finalCoord.x, 0.0f, 1.0f);
//    finalCoord.y = clamp(finalCoord.y, 0.0f, 1.0f);
	return texture(frameBufferTexture, finalCoord).xyz;
}

vec3 dof(vec2 coord)
{
    vec3 result = vec3(0.0f);
    const float coverageBoost = 1.5;
    vec4 color = textureRect(frameBufferTexture, coord);
    float cocRadius = color.a;
    vec3 blurred = textureRect(focusFarDoFBuffer, coord / 2).rgb;
    vec4 near    = textureRect(nearDoFBuffer, coord / 2);

    float normRadius = cocRadius / float(maxCocRadius);
    if (coverageBoost != 1.0f) {
        float a = clamp(coverageBoost * near.a, 0, 1);
        near.rgb = near.rgb * (a / max(near.a, 0.001f));
        near.a = a;
    }

    // Decrease sharp image's contribution rapidly in the near field.
    if (normRadius > 0.1) {
        normRadius = min(normRadius * 1.5, 1.0);
    }
    // for near - take most of the near buffer blur, for far, take most of the far blurred, for focus, take the sharp color
    result = mix(color.rgb, blurred, abs(normRadius)) * (1.0 - near.a) + near.rgb;
    return result;
    //return mix(color.rgb, blurred, abs(normRadius)).rgb;
    //return near.rgb;
}

vec3 lensFlare(vec2 coord)
{
    vec3 result = vec3(0.0f);
    vec3 color = textureRect(lensFlareTexture, coord / 2).rgb;
    // starburst
//    vec2 centerVec = color.xy - vec2(0.5);
//	float d = length(centerVec);
//	float radial = acos(centerVec.x / d);
//	float mask = 
//		  texture(starburstTexture, vec2(radial + uStarburstOffset * 1.0, 0.0)).r
//		* texture(starburstTexture, vec2(radial - uStarburstOffset * 0.5, 0.0)).r;
//	mask = saturate(mask + (1.0 - smoothstep(0.0, 0.3, d)));
    // lens dirt
    float mask = texture(lensDirtTexture, color.xy).r;
    //result = textureRect(frameBufferTexture, coord).xyz + (color * mask * uGlobalBrightness);
    result = textureRect(frameBufferTexture, coord).xyz + color * uGlobalBrightness;
    return result;
    //return color;
}

vec3 bloom(vec2 coord)
{
    vec3 color = textureRect(frameBufferTexture, coord).xyz + textureRect(bloomTexture, coord / 2).xyz;
	return color;
}

vec3 separableBlur(vec2 coord)
{
    vec3 color = textureRect(frameBufferTexture, coord / 2).xyz;
	return color;
}

vec3 toSepiaTone(vec3 rgbSample)
{
	//-----------------------------------------------------------------
	// Variables used for YIQ/RGB color space conversion.
	//-----------------------------------------------------------------
	vec3 yiqTransform0 = vec3(0.299, 0.587, 0.144);
	vec3 yiqTransform1 = vec3(0.596,-0.275,-0.321);
	vec3 yiqTransform2 = vec3(0.212,-0.523, 0.311);

	vec3 yiqInverseTransform0 = vec3(1, 0.956, 0.621);
	vec3 yiqInverseTransform1 = vec3(1,-0.272,-0.647);
	vec3 yiqInverseTransform2 = vec3(1,-1.105, 1.702);

	// transform to YIQ color space and set color information to sepia tone
	vec3 yiq = vec3(dot(yiqTransform0, rgbSample), 0.2, 0.0);

	// inverse transform to RGB color space
	vec3 result = vec3(dot(yiqInverseTransform0, yiq), dot(yiqInverseTransform1, yiq), dot(yiqInverseTransform2, yiq));
	return result;
}

vec2 mushrooms(vec2 inCoord)
{
	return inCoord + vec2(sin(time * 4.3127 + inCoord.y / 9.0) * 15.0, 0.0);
}

vec3 blur(vec2 coord)
{
	vec3 result = vec3(0.0);
        float weight = 1.0 / (filterSize * filterSize);

        for (int i = -filterSize / 2; i <= filterSize / 2; ++i)
            for (int j = -filterSize / 2; j <= filterSize / 2; ++j)
            {
				result += weight * textureRect(frameBufferTexture, coord + vec2(i, j)).xyz;
            }

	return result;
}

vec3 grayscale(vec3 rgbSample)
{
	return vec3(rgbSample.r * 0.2126 + rgbSample.g * 0.7152 + rgbSample.b * 0.0722);
}
