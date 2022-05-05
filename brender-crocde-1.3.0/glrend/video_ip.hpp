#ifndef _VIDEO_IP_HPP
#define _VIDEO_IP_HPP

#include <array>
#include "video.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "compiler.h"
#include "brlimits.h"
#include "brfont.h"
#include "fixed.h"
#include "matrix.h"
#include "pixelmap.h"

typedef struct _VIDEO_FONT
{
	struct
	{
		float x0;
		float x1;
		float y0;
		float y1;
	} glyph[256];
	GLuint glTex;
	br_font *font;
} VIDEO_FONT, *HVIDEO_FONT;

struct _VIDEO
{
	GLint majorVersion;
	GLint minorVersion;
	const char *version;
	const char *vendor;
	const char *renderer;
	GLint maxUniformBlockSize;
	GLint maxUniformBufferBindings;
	GLint maxVertexUniformBlocks;
	GLint maxFragmentUniformBlocks;
	GLint maxSamples;
	GLfloat maxAnisotropy;

	struct
	{
		GLuint program;
		/* Position, vec3 */
		GLint aPosition;
		/* Colour, vec3 */
		GLint aColour;
		/* UV, vec2 */
		GLint aUV;

		/* Sampler, sampler2D */
		GLint uSampler;
		/* Model-View-Projection Matrix, mat4 */
		GLint uMVP;
		/* Boolean (actually a float). Should the image be vertically flipped. */
		GLint uVerticalFlip;
	} defaultProgram;

	struct
	{
		GLuint program;

		/* Position, vec3 */
		GLint aPosition;
		/* UV, vec2 */
		GLint aUV;

		/* Sampler, sampler2D */
		GLint uSampler;
		/* Model-View-Projection Matrix, mat4 */
		GLint uMVP;
		/* Text colour, vec3 */
		GLint uColour;
	} textProgram;

	struct
	{
		GLuint program;

		struct 
		{

			/* Vectex Position, vec3 */
			GLint aPosition;

			/* UV, vec2 */
			GLint aUV;

			/* Vertex Normal, vec3 */
			GLint aNormal;

			/* Vertex colour, vec4 */
			GLint aColour;
		} attributes;

		struct
		{
			/* sampler2D */
			GLint main_texture;
		} uniforms;

		GLuint uboScene;
		GLuint blockIndexScene;
		GLuint blockBindingScene;

		GLuint uboModel;
		GLuint blockIndexModel;
		GLuint blockBindingModel;

		GLint mainTextureBinding;
	} brenderProgram;

	union
	{
		struct
		{
			GLuint white;
			GLuint checkerboard;
			GLuint fnt3x5;
			GLuint fnt4x6;
			GLuint fnt7x9;
		};
		GLuint t[5];
	} texture;

	struct
	{
		VIDEO_FONT fixed3x5;
		VIDEO_FONT prop4x6;
		VIDEO_FONT prop7x9;
	} fonts;

};

#pragma pack(push, 1)
/* std140-compatible light structure */
typedef struct alignas(16) _GLSTD140_LIGHT
{
	/* (X, Y, Z, T), if T == 0, direct, otherwise point/spot */
	glm::vec4 position;
	/* (X, Y, Z, 0), normalised */
	glm::vec4 direction;
	/* (X, Y, Z, 0), normalised */
	glm::vec4 half;
	/* (R, G, B, 0) */
	glm::vec4 colour;
	/* (intensity, constant, linear, attenutation) */
	glm::vec4 iclq;
	/* (inner, outer), if (0.0, 0.0), then this is a point light. */
	glm::vec2 spot_angles;
} GLSTD140_LIGHT, *HGLSTD140_LIGHT;
static_assert(sizeof(GLSTD140_LIGHT) % 16 == 0, "GLSTD140 is not aligned");

typedef struct _GLSTD140_SCENE_DATA
{
	alignas(16) glm::vec4 eye_view;
	alignas(16) glm::vec4 global_ambient_colour;
	alignas(16) std::array<GLSTD140_LIGHT, BR_MAX_LIGHTS> lights;
	alignas(4) uint32_t num_lights;
} GLSTD140_SCENE_DATA, *HGLSTD140_SCENE_DATA;
static_assert(sizeof(GLSTD140_SCENE_DATA::lights) == sizeof(GLSTD140_LIGHT) * BR_MAX_LIGHTS, "std::array<GLSTD140_LIGHT> fucked up");

typedef struct _GLSTD140_MODEL_DATA
{
	alignas(16) glm::mat4x4 model_view;
	alignas(16) glm::mat4x4 projection;
	alignas(16) glm::mat4x4 mvp;
	alignas(16) glm::mat4x4 normal_matrix;
	alignas(16) glm::vec4 surface_colour;
	alignas(16) glm::vec4 ambient_colour;
	alignas(16) glm::vec4 clear_colour;
	alignas(4) float ka;
	alignas(4) float ks;
	alignas(4) float kd;
	alignas(4) float power;
	alignas(4) uint32_t unlit;
} GLSTD140_MODEL_DATA, *HGLSTD140_MODEL_DATA;
#pragma pack(pop)

extern "C" const uint8_t VIDEOI_RawCheckerboard64x64[];

GLuint VIDEOI_CreateAndCompileShader(GLenum type, const char *shader, size_t size);
GLuint VIDEOI_CreateAndCompileProgram(GLuint vert, GLuint frag);
bool VIDEOI_CompileDefaultShader(HVIDEO hVideo);
bool VIDEOI_CompileTextShader(HVIDEO hVideo);
bool VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath);

void VIDEOI_SetupTextures(HVIDEO hVideo);
void VIDEOI_BuildWhiteTexture(GLuint tex);
void VIDEOI_BuildCheckerboardTexture(GLuint tex);
void VIDEOI_BuildFontAtlas(HVIDEO hVideo, HVIDEO_FONT hFont, br_font *font, br_int_32 width, br_int_32 height);
void VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm);
void VIDEOI_BrPixelmapGetTypeDetails(br_uint_8 pmType, GLenum *internalFormat, GLenum *format, GLenum *type, GLsizeiptr *elemBytes);

struct br_device_pixelmap_gl;
void VIDEOI_BrRectToGL(br_pixelmap *pm, br_rectangle *r);
void VIDEOI_BrRectToGL(br_device_pixelmap_gl* pm, br_rectangle *r);

void VIDEOI_BrRectToUVs(br_pixelmap *pm, br_rectangle *r, float& x0, float& y0, float& x1, float& y1);
void VIDEOI_BrRectToUVs(br_device_pixelmap_gl *pm, br_rectangle *r, float& x0, float& y0, float& x1, float& y1);

glm::mat4x4 VIDEOI_BrMatrix34ToGLM44(const br_matrix34 *m);
glm::mat4 VIDEOI_BrMatrix4ToGLM(const br_matrix4 *m);

#endif /* _VIDEO_IP_HPP */
