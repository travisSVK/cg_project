
#include <GL/glew.h>

// STB_IMAGE for loading images of many filetypes
#include <stb_image.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <Helper.h>
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include <model/Model.h>
#include "lensFlare/FlareManager.h"
#include "FboInfo.h"
#include "environment/hdr.h"
#include <windows.h>

#include "Game.h"

//extern "C" {
//    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//}

using std::min;
using std::max;

///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint backgroundProgram, shaderProgram, postFxShader, horizontalBlurShader, 
verticalBlurShader, cutoffShader, hBlurDofShader, vBlurDofShader, downsampleShader, pseudoLensFlareShader, lensFlareShader;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
float environment_multiplier = 1.0f;
GLuint environmentMap, irradianceMap, reflectionMap;
const std::string envmap_base_name = "001";
GLuint gradientTexture, lensDirtTexture, starburstTexture;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
float point_light_intensity_multiplier = 1000.0f;
vec3 point_light_color = vec3(1.f, 1.f, 1.f);
const vec3 lightPosition = vec3(20.0f, 80.0f, -50.0f);

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 securityCamPos = vec3(70.0f, 50.0f, -70.0f);
vec3 securityCamDirection = normalize(-securityCamPos);
vec3 cameraPosition(-70.0f, 50.0f, 70.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);
float cameraSpeed = 1.0f;

vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
const std::string model_filename = "../scenes/NewShip.obj";
engine::Model *landingpadModel = nullptr;
engine::Model *fighterModel = nullptr;
engine::Model *sphereModel = nullptr;
engine::Model *cameraModel = nullptr;

///////////////////////////////////////////////////////////////////////////////
// Post processing effects
///////////////////////////////////////////////////////////////////////////////
enum PostProcessingEffect
{
    None = 0,
    Sepia = 1,
    Mushroom = 2,
    Blur = 3,
    Grayscale = 4,
    Composition = 5,
    Mosaic = 6,
    Separable_blur = 7,
    Bloom = 8,
    Motion_Blur = 9,
    DOF = 10,
    Pseudo_Lens_Flare = 11,
    Lens_Flare = 12
};

int currentEffect = PostProcessingEffect::None;
int filterSize = 1;
int filterSizes[12] = {3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25};

mat4 previousViewProjection;
bool firstFrame = true;
engine::FlareManager* flareManager;

///////////////////////////////////////////////////////////////////////////////
// Framebuffers
///////////////////////////////////////////////////////////////////////////////
std::vector<engine::FboInfo> fboList;

void initGL()
{
	// enable Z-buffering
	glEnable(GL_DEPTH_TEST);

	// enable backface culling
	glEnable(GL_CULL_FACE);

	// Load some models.
	landingpadModel = engine::loadModelFromOBJ("../scenes/landingpad.obj");
	cameraModel = engine::loadModelFromOBJ("../scenes/camera.obj");
	//fighterModel = engine::loadModelFromOBJ("../scenes/NewShip.obj");
    //fighterModel = engine::loadModelFromOBJ("../scenes/tree.obj");
    sphereModel = engine::loadModelFromOBJ("../scenes/sphere.obj");

	fighterModel = engine::loadModelFromOBJ("../scenes/NewShip.obj");
    //fighterModel = engine::loadModelFromOBJ("../scenes/testDice.obj");
    
	// load and set up default shader
	backgroundProgram = engine::loadShaderProgram("shaders/background.vert", "shaders/background.frag", "", "");
	shaderProgram     = engine::loadShaderProgram("shaders/simple.vert",     "shaders/simple.frag", "", "");
	postFxShader      = engine::loadShaderProgram("shaders/postFx.vert",     "shaders/postFx.frag", "", "");
    horizontalBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/horizontal_blur.frag", "", "");
    verticalBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/vertical_blur.frag", "", "");
    cutoffShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/cutoff.frag", "", "");
    hBlurDofShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/blur_dof.frag", "", "", "", "#version 420\n#define HORIZONTAL\n");
    vBlurDofShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/blur_dof.frag", "", "", "" , "#version 420\n");
    pseudoLensFlareShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/pseudo_lens_flare.frag", "", "");
    //lensFlareShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/lens_flare.frag");
    downsampleShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/downsample.frag", "", "");

    // load color gradient texture for lens flare
    int w, h, comp;
    unsigned char* texture = stbi_load("../scenes/lens_dirt.png", &w, &h, &comp, STBI_rgb_alpha);
    glGenTextures(1, &lensDirtTexture);
    glBindTexture(GL_TEXTURE_2D, lensDirtTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
    free(texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    texture = stbi_load("../scenes/starburst.png", &w, &h, &comp, STBI_rgb_alpha);
    glGenTextures(1, &starburstTexture);
    glBindTexture(GL_TEXTURE_2D, starburstTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
    free(texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	///////////////////////////////////////////////////////////////////////////
	// Load environment map
	///////////////////////////////////////////////////////////////////////////
	const int roughnesses = 8;
	std::vector<std::string> filenames;
	for (int i = 0; i < roughnesses; i++)
		filenames.push_back("../scenes/envmaps/" + envmap_base_name + "_dl_" + std::to_string(i) + ".hdr");

	reflectionMap = engine::loadHdrMipmapTexture(filenames);
	environmentMap = engine::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + ".hdr");
	irradianceMap = engine::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + "_irradiance.hdr");
    gradientTexture = engine::loadHdrTexture("../scenes/ghost_color_gradient.psd");

	///////////////////////////////////////////////////////////////////////////
	// Setup Framebuffers
	///////////////////////////////////////////////////////////////////////////
	int width, height;
	SDL_GetWindowSize(g_window, &width, &height);
    const int numFbos = 8;
    for (int i = 0; i < numFbos; i++)
    {
        fboList.push_back(engine::FboInfo(width, height, 2));
    }

    flareManager = new engine::FlareManager(0.16f, width, height);
}

void drawScene(const mat4 &view, const mat4 &projection)
{
	glUseProgram(backgroundProgram);
	engine::setUniformSlow(backgroundProgram, "environment_multiplier", environment_multiplier);
	engine::setUniformSlow(backgroundProgram, "inv_PV", inverse(projection * view));
	engine::setUniformSlow(backgroundProgram, "camera_pos", cameraPosition);
    engine::setUniformSlow(backgroundProgram, "nearSharpPlane", 10.0f);
    engine::setUniformSlow(backgroundProgram, "farSharpPlane", -10.0f);
    engine::setUniformSlow(backgroundProgram, "nearBlurryPlane", 70.0f);
    engine::setUniformSlow(backgroundProgram, "farBlurryPlane", -70.0f);
    engine::setUniformSlow(backgroundProgram, "farCoC", -4);
    engine::setUniformSlow(backgroundProgram, "nearCoC", 4);
    engine::setUniformSlow(backgroundProgram, "focusCoC", 0);
	engine::drawFullScreenQuad();

	glUseProgram(shaderProgram);
    // set DOF variables
    engine::setUniformSlow(shaderProgram, "nearSharpPlane", 10.0f);
    engine::setUniformSlow(shaderProgram, "farSharpPlane", -10.0f);
    engine::setUniformSlow(shaderProgram, "nearBlurryPlane", 70.0f);
    engine::setUniformSlow(shaderProgram, "farBlurryPlane", -70.0f);
    engine::setUniformSlow(shaderProgram, "farCoC", -4);
    engine::setUniformSlow(shaderProgram, "nearCoC", 4);
    engine::setUniformSlow(shaderProgram, "focusCoC", 0);

	// Light source
	vec4 viewSpaceLightPosition = view * vec4(lightPosition, 1.0f);
	engine::setUniformSlow(shaderProgram, "point_light_color", point_light_color);
	engine::setUniformSlow(shaderProgram, "point_light_intensity_multiplier", point_light_intensity_multiplier);
	engine::setUniformSlow(shaderProgram, "viewSpaceLightPosition", vec3(viewSpaceLightPosition));

	// Environment
	engine::setUniformSlow(shaderProgram, "environment_multiplier", environment_multiplier);

	// camera
	engine::setUniformSlow(shaderProgram, "viewInverse", inverse(view));

	// landing pad 
	mat4 modelMatrix(1.0f);
	engine::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projection * view * modelMatrix);
	engine::setUniformSlow(shaderProgram, "modelViewMatrix", view * modelMatrix);
	engine::setUniformSlow(shaderProgram, "normalMatrix", inverse(transpose(view * modelMatrix)));
    engine::setUniformSlow(shaderProgram, "modelMatrix", modelMatrix);
	
	engine::render(landingpadModel, true);

	// Fighter
	//mat4 fighterModelMatrix = translate(15.0f * worldUp) * rotate(currentTime * -float(M_PI) / 4.0f, worldUp);
    mat4 fighterModelMatrix = translate(15.0f * worldUp);
	engine::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projection * view * fighterModelMatrix);
	engine::setUniformSlow(shaderProgram, "modelViewMatrix", view * fighterModelMatrix);
	engine::setUniformSlow(shaderProgram, "normalMatrix", inverse(transpose(view * fighterModelMatrix)));
    engine::setUniformSlow(shaderProgram, "modelMatrix", fighterModelMatrix);
	engine::render(fighterModel, true);
}

void debugDrawLight(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 &worldSpaceLightPos)
{
    mat4 modelMatrix = glm::translate(worldSpaceLightPos);
    glUseProgram(shaderProgram);
    engine::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
    engine::render(sphereModel, false);
}

void separableBlur(GLuint hShader, GLuint vShader, const engine::FboInfo &source, engine::FboInfo &downsampledFbo, engine::FboInfo &hBlurFbo, engine::FboInfo &vBlurFbo)
{
    // downsample 
    /*downsampledFbo.resize(downsampledFbo.getWidth() / 2, downsampledFbo.getHeight() / 2);
    glViewport(0, 0, downsampledFbo.getWidth(), downsampledFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, downsampledFbo.getFrameBufferId());
    glUseProgram(downsampleShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source.getColorTextureTarget(0));
    engine::drawFullScreenQuad();*/

    // horizontal blur
    //hBlurFbo.resize(hBlurFbo.getWidth() / 2, hBlurFbo.getHeight() / 2);
    //glViewport(0, 0, hBlurFbo.getWidth(), hBlurFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, hBlurFbo.getFrameBufferId());
    glUseProgram(hShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source.getColorTextureTarget(0));
    engine::drawFullScreenQuad();

    // vertical blur
    /*vBlurFbo.resize(vBlurFbo.getWidth() / 2, vBlurFbo.getHeight() / 2);
    glViewport(0, 0, vBlurFbo.getWidth(), vBlurFbo.getHeight());*/
    glBindFramebuffer(GL_FRAMEBUFFER, vBlurFbo.getFrameBufferId());
    glUseProgram(vShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hBlurFbo.getColorTextureTarget(0));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, hBlurFbo.getColorTextureTarget(1));
    engine::drawFullScreenQuad();
}

void pseudoLensFlare(GLuint hShader, GLuint vShader, const engine::FboInfo &source, engine::FboInfo &lensFlareFbo, engine::FboInfo &downsampledFbo, engine::FboInfo &hBlurFbo, engine::FboInfo &vBlurFbo)
{
    // downsample 
    downsampledFbo.resize(downsampledFbo.getWidth() / 2, downsampledFbo.getHeight() / 2);
    glViewport(0, 0, downsampledFbo.getWidth(), downsampledFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, downsampledFbo.getFrameBufferId());
    glUseProgram(downsampleShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source.getColorTextureTarget(0));
    engine::drawFullScreenQuad();

    // horizontal blur
    lensFlareFbo.resize(lensFlareFbo.getWidth() / 2, lensFlareFbo.getHeight() / 2);
    glViewport(0, 0, lensFlareFbo.getWidth(), lensFlareFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, lensFlareFbo.getFrameBufferId());
    glUseProgram(pseudoLensFlareShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, downsampledFbo.getColorTextureTarget(0));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gradientTexture);
    engine::setUniformSlow(pseudoLensFlareShader, "uGhostCount", 4);
    engine::setUniformSlow(pseudoLensFlareShader, "uGhostSpacing", 0.1f);
    engine::setUniformSlow(pseudoLensFlareShader, "uGhostThreshold", 4.0f);
    engine::setUniformSlow(pseudoLensFlareShader, "uHaloRadius", 0.6f);
    engine::setUniformSlow(pseudoLensFlareShader, "uHaloThickness", 0.1f);
    engine::setUniformSlow(pseudoLensFlareShader, "uHaloThreshold", 4.0f);
    engine::setUniformSlow(pseudoLensFlareShader, "uHaloAspectRatio", 1.0f);
    engine::setUniformSlow(pseudoLensFlareShader, "uChromaticAberration", 0.01f);
    engine::drawFullScreenQuad();

    // horizontal blur
    hBlurFbo.resize(hBlurFbo.getWidth() / 2, hBlurFbo.getHeight() / 2);
    glViewport(0, 0, hBlurFbo.getWidth(), hBlurFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, hBlurFbo.getFrameBufferId());
    glUseProgram(hShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lensFlareFbo.getColorTextureTarget(0));
    engine::drawFullScreenQuad();

    // vertical blur
    vBlurFbo.resize(vBlurFbo.getWidth() / 2, vBlurFbo.getHeight() / 2);
    glViewport(0, 0, vBlurFbo.getWidth(), vBlurFbo.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, vBlurFbo.getFrameBufferId());
    glUseProgram(vShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hBlurFbo.getColorTextureTarget(0));
    engine::drawFullScreenQuad();
}

void display()
{
	///////////////////////////////////////////////////////////////////////////
	// Check if any framebuffer needs to be resized
	///////////////////////////////////////////////////////////////////////////
	int w, h;
	SDL_GetWindowSize(g_window, &w, &h);

	for (int i = 0; i < fboList.size(); i++) {
		if (fboList[i].getWidth() != w || fboList[i].getHeight() != h)
			fboList[i].resize(w, h);
	}

	///////////////////////////////////////////////////////////////////////////
	// setup matrices
	///////////////////////////////////////////////////////////////////////////
	mat4 securityCamViewMatrix = lookAt(securityCamPos, securityCamPos + securityCamDirection, worldUp);
	mat4 securityCamProjectionMatrix = perspective(radians(30.0f), float(w) / float(h), 15.0f, 1000.0f);

	mat4 projectionMatrix = perspective(radians(45.0f), float(w) / float(h), 0.01f, 1000.0f);
	mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);

	///////////////////////////////////////////////////////////////////////////
	// Bind the environment map(s) to unused texture units
	///////////////////////////////////////////////////////////////////////////
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, irradianceMap);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, reflectionMap);

	///////////////////////////////////////////////////////////////////////////
	// draw scene from security camera
	///////////////////////////////////////////////////////////////////////////
	// >>> @task 2
    engine::FboInfo &securityFB = fboList[0];
    glBindFramebuffer(GL_FRAMEBUFFER, securityFB.getFrameBufferId());
    glViewport(0, 0, securityFB.getWidth(), securityFB.getHeight());
    glClearColor(0.2, 0.2, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    drawScene(securityCamViewMatrix, securityCamProjectionMatrix);

    engine::Material &screen = landingpadModel->m_materials[8];
    screen.m_emission_texture.gl_id = securityFB.getColorTextureTarget(0);

	///////////////////////////////////////////////////////////////////////////
	// draw scene from camera
	///////////////////////////////////////////////////////////////////////////
    engine::FboInfo &cameraFB = fboList[1];
    glBindFramebuffer(GL_FRAMEBUFFER, cameraFB.getFrameBufferId());
    glViewport(0, 0, w, h);
	glClearColor(0.2, 0.2, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawScene(viewMatrix, projectionMatrix); // using both shaderProgram and backgroundProgram
    debugDrawLight(viewMatrix, projectionMatrix, lightPosition);

    engine::FboInfo &horizontalBlurFbo = fboList[2];
    engine::FboInfo &verticalBlurFbo = fboList[3];
    engine::FboInfo &hBlurFboNear = fboList[4];
    engine::FboInfo &vBlurFboNear = fboList[5];
    engine::FboInfo &downsampledFbo = fboList[6];
    engine::FboInfo &lensFlareFbo = fboList[7];
    
    if (currentEffect == PostProcessingEffect::Bloom)
    {
        // cutoff
        engine::FboInfo &cutoffFbo = fboList[4];
        glBindFramebuffer(GL_FRAMEBUFFER, cutoffFbo.getFrameBufferId());
        glUseProgram(cutoffShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cameraFB.getColorTextureTarget(0));
        engine::drawFullScreenQuad();

        separableBlur(horizontalBlurShader, verticalBlurShader, cutoffFbo, downsampledFbo, horizontalBlurFbo, verticalBlurFbo);
        glViewport(0, 0, w, h);
    }
    if (currentEffect == PostProcessingEffect::Separable_blur)
    {
        separableBlur(horizontalBlurShader, verticalBlurShader, cameraFB, downsampledFbo, horizontalBlurFbo, verticalBlurFbo);
        glViewport(0, 0, w, h);
    }

    if (currentEffect == PostProcessingEffect::DOF)
    {
        separableBlur(hBlurDofShader, vBlurDofShader, cameraFB, downsampledFbo, horizontalBlurFbo, verticalBlurFbo);
        glViewport(0, 0, w, h);
    }
    if (currentEffect == PostProcessingEffect::Pseudo_Lens_Flare)
    {
        pseudoLensFlare(horizontalBlurShader, verticalBlurShader, cameraFB, lensFlareFbo, downsampledFbo, horizontalBlurFbo, verticalBlurFbo);
        glViewport(0, 0, w, h);
    }

	// camera (obj-model)
	glUseProgram(shaderProgram);
	engine::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * inverse(securityCamViewMatrix) * scale(vec3(10.0f)) * rotate(float(M_PI), vec3(0.0f, 1.0, 0.0)));
	engine::setUniformSlow(shaderProgram, "modelViewMatrix", viewMatrix * inverse(securityCamViewMatrix));
	engine::setUniformSlow(shaderProgram, "normalMatrix", inverse(transpose(viewMatrix * inverse(securityCamViewMatrix))));
	
	//engine::render(cameraModel);

	///////////////////////////////////////////////////////////////////////////
	// Post processing pass(es)
	///////////////////////////////////////////////////////////////////////////
    if (firstFrame)
    {
        previousViewProjection = projectionMatrix * viewMatrix;
        firstFrame = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram( postFxShader );
    glActiveTexture(GL_TEXTURE0);
    if (currentEffect == PostProcessingEffect::Separable_blur)
    {
        glBindTexture(GL_TEXTURE_2D, verticalBlurFbo.getColorTextureTarget(0));
    }
    else if (currentEffect == PostProcessingEffect::DOF)
    {
        glBindTexture(GL_TEXTURE_2D, cameraFB.getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, verticalBlurFbo.getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, verticalBlurFbo.getColorTextureTarget(1));
    }
    else if (currentEffect == PostProcessingEffect::Bloom)
    {
        glBindTexture(GL_TEXTURE_2D, cameraFB.getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, verticalBlurFbo.getColorTextureTarget(0));
    }
    else if (currentEffect == PostProcessingEffect::Pseudo_Lens_Flare)
    {
        glBindTexture(GL_TEXTURE_2D, cameraFB.getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, verticalBlurFbo.getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, lensDirtTexture);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, starburstTexture);
        engine::setUniformSlow(postFxShader, "uGlobalBrightness", 0.01f);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, cameraFB.getColorTextureTarget(0));
    }
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, cameraFB.getDepthBuffer());
    engine::setUniformSlow(postFxShader, "time", currentTime);
    engine::setUniformSlow(postFxShader, "currentEffect", currentEffect);
    engine::setUniformSlow(postFxShader, "filterSize", filterSizes[filterSize - 1]);
    engine::setUniformSlow(postFxShader, "viewProjectionInverseMatrix", inverse(projectionMatrix * viewMatrix));
    engine::setUniformSlow(postFxShader, "previousViewProjectionMatrix", previousViewProjection);
    engine::setUniformSlow(postFxShader, "numSamples", 3);
    engine::setUniformSlow(postFxShader, "maxCocRadius", 4);
    engine::drawFullScreenQuad();

    if (currentEffect == PostProcessingEffect::Lens_Flare)
    {
        //flareManager->render(viewMatrix, projectionMatrix, lightPosition);
    }

    previousViewProjection = projectionMatrix * viewMatrix;

	CHECK_GL_ERROR();
}

bool handleEvents(void)
{
	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
			quitEvent = true;
		}
		if (event.type == SDL_MOUSEMOTION && !ImGui::IsMouseHoveringAnyWindow()) {
			// More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
			static int prev_xcoord = event.motion.x;
			static int prev_ycoord = event.motion.y;
			int delta_x = event.motion.x - prev_xcoord;
			int delta_y = event.motion.y - prev_ycoord;

			if (event.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				float rotationSpeed = 0.0005f;
				mat4 yaw = rotate(rotationSpeed * -delta_x, worldUp);
				mat4 pitch = rotate(rotationSpeed * -delta_y, normalize(cross(cameraDirection, worldUp)));
				cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
			}

			if (event.button.button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
				float rotationSpeed = 0.005f;
				mat4 yaw = rotate(rotationSpeed * -delta_x, worldUp);
				mat4 pitch = rotate(rotationSpeed * -delta_y, normalize(cross(securityCamDirection, worldUp)));
				securityCamDirection = vec3(pitch * yaw * vec4(securityCamDirection, 0.0f));
			}

			prev_xcoord = event.motion.x;
			prev_ycoord = event.motion.y;
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t *state = SDL_GetKeyboardState(nullptr);
	vec3 cameraRight = cross(cameraDirection, worldUp);
	
	if (state[SDL_SCANCODE_W]) {
		cameraPosition += cameraSpeed* cameraDirection;
	}
	if (state[SDL_SCANCODE_S]) {
		cameraPosition -= cameraSpeed * cameraDirection;
	}
	if (state[SDL_SCANCODE_A]) {
		cameraPosition -= cameraSpeed * cameraRight;
	}
	if (state[SDL_SCANCODE_D]) {
		cameraPosition += cameraSpeed * cameraRight;
	}
	if (state[SDL_SCANCODE_Q]) {
		cameraPosition -= cameraSpeed * worldUp;
	}
	if (state[SDL_SCANCODE_E]) {
		cameraPosition += cameraSpeed * worldUp;
	}

	return quitEvent;
}

void gui()
{
	// Inform imgui of new frame
	ImGui_ImplSdlGL3_NewFrame(g_window);

	// ----------------- Set variables --------------------------
	ImGui::Text("Post-processing effect");
	ImGui::RadioButton("None", &currentEffect, PostProcessingEffect::None);
	ImGui::RadioButton("Sepia", &currentEffect, PostProcessingEffect::Sepia);
	ImGui::RadioButton("Mushroom", &currentEffect, PostProcessingEffect::Mushroom);
	ImGui::RadioButton("Blur", &currentEffect, PostProcessingEffect::Blur);
	ImGui::SameLine();
	ImGui::SliderInt("Filter size", &filterSize, 1, 12);
	ImGui::RadioButton("Grayscale", &currentEffect, PostProcessingEffect::Grayscale);
	ImGui::RadioButton("All of the above", &currentEffect, PostProcessingEffect::Composition);
	ImGui::RadioButton("Mosaic", &currentEffect, PostProcessingEffect::Mosaic);
	ImGui::RadioButton("Separable Blur", &currentEffect, PostProcessingEffect::Separable_blur);
	ImGui::RadioButton("Bloom", &currentEffect, PostProcessingEffect::Bloom);
    ImGui::RadioButton("Motion blur", &currentEffect, PostProcessingEffect::Motion_Blur);
    ImGui::RadioButton("DOF", &currentEffect, PostProcessingEffect::DOF);
    ImGui::RadioButton("Pseudo Lens flare", &currentEffect, PostProcessingEffect::Pseudo_Lens_Flare);
    ImGui::RadioButton("Lens flare", &currentEffect, PostProcessingEffect::Lens_Flare);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// ----------------------------------------------------------

	// Render the GUI.
	ImGui::Render();
}

int main(int argc, char *argv[])
{
	//g_window = engine::init_window_SDL("Game");
    Game game;
    game.initialize();
	//initGL();

	bool stopRendering = false;
	auto startTime = std::chrono::system_clock::now();

	while (!stopRendering) {
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		currentTime = timeSinceStart.count();

		// render to window
		//display();
        game.render();
        stopRendering = game.update();
		// Render overlay GUI.
		//gui();

		// Swap front and back buffer. This frame will now been displayed.
		//SDL_GL_SwapWindow(g_window);

		// check events (keyboard among other)
		//stopRendering = handleEvents();
	}

	// Free Models
	/*engine::freeModel(landingpadModel);
	engine::freeModel(cameraModel);
	engine::freeModel(fighterModel);
	engine::freeModel(sphereModel);
    flareManager->destroy();
    delete flareManager;*/

	// Shut down everything. This includes the window and all other subsystems.
	//engine::shutDown(g_window);
    game.destroy();
	return 0;
}
