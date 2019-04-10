#version 420
// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

#define GHOST_TINT_PER_SAMPLE          1  // Apply txGhostGradientColor inside the sample loop instead of at the end.
#define DISABLE_HALO_ASPECT_RATIO      0  // Code is simpler/cheaper without this, but the halo shape is fixed.
#define DISABLE_CHROMATIC_ABERRATION   0  // Takes 3x fewer samples.

layout(binding = 0) uniform sampler2D frameBufferTexture;
layout(binding = 1) uniform sampler2D txGhostColorGradient;
layout(location = 0) out vec4 fragmentColor;
uniform int uGhostCount;
uniform float uGhostSpacing;
uniform float uGhostThreshold;


uniform float uHaloRadius;
uniform float uHaloThickness;
uniform float uHaloThreshold;
uniform float uHaloAspectRatio;

uniform float uChromaticAberration;

vec4 textureRect(in sampler2D tex, vec2 rectangleCoord)
{
	return texture(tex, rectangleCoord / textureSize(tex, 0));
}

vec3 applyThreshold(in vec3 rgb, in float threshold)
{
	return max(rgb - vec3(threshold), vec3(0.0));
}

vec3 sampleSceneColor(in vec2 uv)
{
#if DISABLE_CHROMATIC_ABERRATION
	return texture(frameBufferTexture, uv).rgb;
#else
	vec2 offset = normalize(vec2(0.5) - uv) * uChromaticAberration;
	return vec3(
		texture(frameBufferTexture, uv + offset).r,
        texture(frameBufferTexture, uv).g,
        texture(frameBufferTexture, uv - offset).b
		);
#endif
}

vec3 sampleGhosts(in vec2 uv, in float threshold)
{
	vec3 ret = vec3(0.0);
	vec2 ghostVec = (vec2(0.5) - uv) * uGhostSpacing;
	for (int i = 0; i < uGhostCount; ++i) {
	 // sample scene color
		vec2 suv = fract(uv + ghostVec * vec2(i));
		vec3 s = sampleSceneColor(suv);
		s = applyThreshold(s, threshold);
		
	 // tint/weight
		float distanceToCenter = distance(suv, vec2(0.5));
		#if GHOST_TINT_PER_SAMPLE
			s *= texture(txGhostColorGradient, vec2(distanceToCenter, 0.5)).rgb;  //incorporate weight into tint gradient
		#else
			float weight = 1.0 - smoothstep(0.0, 0.75, distanceToCenter);  //analytical weight
			s *= weight;
		#endif
        ret += s;
    //TEST
//        float weight = length(vec2(0.5) - suv) / length(vec2(0.5));
//        weight = pow(1.0 - weight, 10.0);
		
	}
	#if !GHOST_TINT_PER_SAMPLE
		ret *= textureLod(txGhostColorGradient, vec2(distance(uv, vec2(0.5)), 0.5), 0.0).rgb;
	#endif

	return ret;
}

// Cubic window; map [0, _radius] in [1, 0] as a cubic falloff from _center.
float windowCubic(float x, float center, float radius)
{
	x = min(abs(x - center) / radius, 1.0);
	return 1.0 - x * x * (3.0 - 2.0 * x);
}

vec3 sampleHalo(in vec2 uv, in float radius, in float aspectRatio, in float threshold)
{
	vec2 haloVec = vec2(0.5) - uv;
	#if DISABLE_HALO_ASPECT_RATIO
		haloVec = normalize(haloVec);
		float haloWeight = distance(_uv, vec2(0.5));
	#else
		haloVec.x /= aspectRatio;
		haloVec = normalize(haloVec);
		haloVec.x *= aspectRatio;
		vec2 wuv = (uv - vec2(0.5, 0.0)) / vec2(aspectRatio, 1.0) + vec2(0.5, 0.0);
		float haloWeight = distance(wuv, vec2(0.5));
	#endif
	haloVec *= radius;
	haloWeight = windowCubic(haloWeight, radius, uHaloThickness);
	return applyThreshold(sampleSceneColor(uv + haloVec), threshold) * haloWeight;
}

void main()
{
    vec4 pixel = textureRect(frameBufferTexture, gl_FragCoord.xy);
	vec2 uv = vec2(1.0) - pixel.xy; // flip the texture coordinates
	vec3 ret = vec3(0.0);

	ret += sampleGhosts(uv, uGhostThreshold);
	//ret += sampleHalo(uv, uHaloRadius, uHaloAspectRatio, uHaloThreshold);
	
	fragmentColor = vec4(ret, 1.0);
    //fragmentColor = vec4(1.0);
}