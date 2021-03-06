
#include <cstdint>
#include "glExtra.h"
#ifndef _WIN32
#include <t5.h>
#include <GL/gl.h>
#else
#include "t5.h"
#define __func__ __FUNCTION__
#endif
#include <cassert>
#include "configuration.h"
#include "shader.h"
#include <cstring>
#include <string>

using std::string;

const char *EmbeddedFrag110 =
"                                                                       \n\
#version 130                                                            \n\
                                                                        \n\
uniform sampler2D textureSampler;                                       \n\
                                                                        \n\
void main(void){                                                        \n\
    vec4 texcolor = texture2D(textureSampler, gl_TexCoord[0].st);       \n\
	gl_FragColor = texcolor*gl_Color;                                   \n\
}                                                                       \n\
";

const char *EmbeddedVert110 =
"\
#version 130                                                            \n\
                                                                        \n\
uniform float ScreenWidth;                                              \n\
uniform float ScreenHeight;                                             \n\
                                                                        \n\
void main(void){                                                        \n\
                                                                        \n\
    gl_TexCoord[0]  = gl_MultiTexCoord0;                                \n\
    gl_FrontColor   = gl_Color;                                         \n\
	gl_Position     = gl_ModelViewProjectionMatrix*gl_Vertex;           \n\
}                                                                       \n\
";

const char *EmbeddedFrag140 =
"                                                                       \n\
#version 140                                                            \n\
                                                                        \n\
uniform sampler2D textureSampler;                                       \n\
in vec4 VColor;                                                         \n\
                                                                        \n\
void main(void){                                                        \n\
    vec4 texcolor = texture2D(textureSampler, gl_TexCoord[0].st);       \n\
	gl_FragColor = texcolor*VColor;                                     \n\
}                                                                       \n\
";

const char *EmbeddedVert140 =
"\
#version 140                                                            \n\
                                                                        \n\
uniform float ScreenWidth;                                              \n\
uniform float ScreenHeight;                                             \n\
                                                                        \n\
in vec2 Vertex;                                                         \n\
in vec4 Color;                                                          \n\
out vec4 VColor;                                                        \n\
                                                                        \n\
void main(void){                                                        \n\
    gl_TexCoord[0] = gl_MultiTexCoord0;                                 \n\
    VColor = Color;                                                     \n\
    gl_Position = (vec4(Vertex, 1.0, 1.0)/vec4(ScreenWidth/2.0, -ScreenHeight/2.0, 1.0, 1.0))-vec4(1.0, -1.0, 0.0, 0.0); \n\
}                                                                       \n\
";

const char *EmbeddedFragHq2x =
"                                                                       \n\
#version 130                                                            \n\
                                                                        \n\
uniform float ScreenWidth;                                              \n\
uniform float ScreenHeight;                                             \n\
                                                                        \n\
uniform sampler2D textureSampler;                                       \n\
                                                                        \n\
void main(void){                                                        \n\
                                                                        \n\
    vec4 texcolor = texture2D(textureSampler, gl_TexCoord[0].st);       \n\
    vec4 color = texcolor*gl_Color;                                     \n\
    vec4 rcolor = color;                                                \n\
    color.r = (rcolor.r+rcolor.g+rcolor.b)/6.75;                        \n\
    color.b = (rcolor.r+rcolor.g+rcolor.b)/12.5;                        \n\
    color.g = (rcolor.r+rcolor.g+rcolor.g+rcolor.b)/5.0;                \n\
                                                                        \n\
    float nx = gl_FragCoord.x;                                          \n\
                                                                        \n\
    while(nx>ScreenWidth/64){                                           \n\
      nx-=0.001;                                                        \n\
    }                                                                   \n\
                                                                        \n\
    if(nx<0.001){                                                       \n\
      color/=8.0;                                                       \n\
    }                                                                   \n\
    gl_FragColor = color;                                               \n\
}                                                                       \n\
";


const char *shaderDir;
const char *systemShader;

GLuint LoadEmbeddedShader(void){
    #ifdef __APPLE__
    const char *glslver = NULL;
    #else
    const char *glslver = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    #endif // __APPLE__

    if(!glslver)
        glslver = "No version string found for GLSL. Assuming 1.10 compliance.";
    printf("%s\n", glslver);
    //Build the program
    GLuint frag = TS_CreateShader(EmbeddedFrag110, GL_FRAGMENT_SHADER);
    GLuint vert = TS_CreateShader(EmbeddedVert110, GL_VERTEX_SHADER);

    GLuint prog = TS_CreateProgram(frag, vert);

    if(prog==0)
        fprintf(stderr, "[FJ-GL] %s Error: Could not create embedded program. Something is seriously wrong here.\n", __func__);

    return prog;

}

GLuint TS_LoadSystemShader(const char *file){

    assert(file!=NULL);

    assert(shaderDir!=NULL);
    assert(strnlen(shaderDir, 8)>0);

    //Concatenate up the filename.

    size_t len1 = strlen(systemShader);
    size_t len2 = len1+strlen(file);

    char *fullfile = (char *)malloc(len2+1);

    fullfile[len2] = '\0';

    memcpy(fullfile, shaderDir, len1+1);

    strcat(fullfile, file);

    assert(T5_IsDir(shaderDir));
    assert(T5_IsFile(fullfile));

    //Load the specified shader manifest
    t5::DataSource *source = t5::DataSource::FromPath(t5::DataSource::eRead, fullfile);
    t5::map::Group group(nullptr);
    group.ReadDataSource(source);

    const char *fragmentname = group.GetString("fragment");
    const char *vertexname = group.GetString("vertex");

    //Load the filetext of the shaders
    t5::DataSource *fragment_source = t5::DataSource::FromPath(t5::DataSource::eRead, string(shaderDir).append(fragmentname).c_str());
    t5::DataSource *vertex_source = t5::DataSource::FromPath(t5::DataSource::eRead, string(shaderDir).append(vertexname).c_str());

    char *fragment_text = (char *)malloc(fragment_source->Length()+1);
    fragment_text[fragment_source->Length()] = '\0';

    char *vertex_text = (char *)malloc(vertex_source->Length()+1);
    vertex_text[vertex_source->Length()] = '\0';

    fragment_source->Read(fragment_text, fragment_source->Length());
    vertex_source->Read(vertex_text, vertex_source->Length());


    printf("[FJ-GL] Frag Shader:\n%s\n\nVertex Shader:\n%s\n", fragment_text, vertex_text);

    //Build the program
    GLuint frag = TS_CreateShader(fragment_text, GL_FRAGMENT_SHADER);
    GLuint vert = TS_CreateShader(vertex_text, GL_VERTEX_SHADER);

    GLuint prog = TS_CreateProgram(frag, vert);

    if(prog==0)
        fprintf(stderr, "[FJ-GL] %s Error: Could not create program from %s.\n", __func__, file);

    free(fragment_text);
    free(vertex_text);

    return prog;
}

GLuint TS_LoadShader(const char *file){

    assert(file!=NULL);

    assert(shaderDir!=NULL);
    assert(strnlen(shaderDir, 8)>0);

    //Concatenate up the filename.

    size_t len1 = strlen(shaderDir);
    size_t len2 = len1+strlen(file);

    char *fullfile = (char *)malloc(len2+1);

    fullfile[len2] = '\0';

    memcpy(fullfile, shaderDir, len1+1);

    strcat(fullfile, file);

    assert(T5_IsDir(shaderDir));
    assert(T5_IsFile(fullfile));

    //Load the specified shader manifest
    t5::DataSource *source = t5::DataSource::FromPath(t5::DataSource::eRead, fullfile);
    t5::map::Group group(nullptr);
    group.ReadDataSource(source);

    const char *fragmentname = group.GetString("fragment");
    const char *vertexname = group.GetString("vertex");

    //Load the filetext of the shaders

    t5::DataSource *fragment_source = t5::DataSource::FromPath(t5::DataSource::eRead, string(systemShader).append(fragmentname).c_str());
    t5::DataSource *vertex_source = t5::DataSource::FromPath(t5::DataSource::eRead, string(systemShader).append(vertexname).c_str());

    char *fragment_text = (char *)malloc(fragment_source->Length()+1);
    fragment_text[fragment_source->Length()] = '\0';

    char *vertex_text = (char *)malloc(vertex_source->Length()+1);
    vertex_text[vertex_source->Length()] = '\0';

    fragment_source->Read(fragment_text, fragment_source->Length());
    vertex_source->Read(vertex_text, vertex_source->Length());

    //Build the program
    GLuint frag = TS_CreateShader(fragment_text, GL_FRAGMENT_SHADER);
    GLuint vert = TS_CreateShader(vertex_text, GL_VERTEX_SHADER);

    GLuint prog = TS_CreateProgram(frag, vert);

    if(prog==0)
        fprintf(stderr, "[FJ-GL] %s Error: Could not create program from %s.\n", __func__, file);

    free(fragment_text);
    free(vertex_text);

    return prog;
}

GLuint TS_CreateShader(const char *text, GLenum type){
    if(text==NULL){
        printf("[FJ-GL] Error: Emtpy string given.\n");
        return 0;
    }
    GLuint shader = glCreateShader(type);
    if(shader==0){
        fprintf(stderr, "[FJ-GL] Error: Something went terribly wrong, the shader index was 0.\n");
    }
    GLint tsize = strlen(text)+2;

    glShaderSource(shader, 1, &text, &tsize);

    glCompileShader(shader);

    GLint shader_status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);

    if (shader_status==GL_FALSE) {
        printf("[FJ-GL] Error: Failed to compile shader.\n");

        GLint log_size;
        GLint written = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        char *log_text = (char *)malloc(log_size);
        glGetShaderInfoLog(shader, log_size, &written, log_text);
        if(written==0)
            printf("[FJ-GL] Error: No log was written.\n");
        if(written>log_size)
            printf("[FJ-GL] Error: Your OpenGL driver just wrote past the end of my buffer. I told it not to, really!\n");
        printf("%s\n", log_text);
        free(log_text);
        glDeleteShader(shader);

        return 0;
    }
    printf("[FJ-GL] Info: Shader compiled ok. ID number %i.\n", shader);
    return shader;

}

GLuint TS_CreateProgram(GLuint frag, GLuint vert){
    if((glIsShader(frag)==GL_FALSE)||(glIsShader(vert)==GL_FALSE)){
        printf("[FJ-GL] Error: One or more shader was invalid\n\tFrag %s\tVert %s\n", (glIsShader(frag)==GL_TRUE)?"good":"bad", (glIsShader(vert)==GL_TRUE)?"good":"bad");
        //return 0;
    }

    GLint program_status;

    GLint fragstat, vertstat;

    glGetShaderiv(frag, GL_SHADER_TYPE, &fragstat);

    if(fragstat!=GL_FRAGMENT_SHADER)
        printf("[FJ-GL] Error: Invalid fragment shader.\n");

    glGetShaderiv(vert, GL_SHADER_TYPE, &vertstat);

    if(vertstat!=GL_VERTEX_SHADER)
        printf("[FJ-GL] Error: Invalid vertex shader.\n");

    if((fragstat!=GL_FRAGMENT_SHADER)||(vertstat!=GL_VERTEX_SHADER)){
        printf("[FJ-GL] Error: Bad shader(s). Exiting.\n");
        //return 0;

    }

    GLuint prog = glCreateProgram();

    glAttachShader(prog, frag);
    glAttachShader(prog, vert);
    printf("[FJ-GL] Info: Linking Program.\n");
    glLinkProgram(prog);

    glGetProgramiv(prog, GL_LINK_STATUS, &program_status);

    if(!program_status){
        printf("[FJ-GL] Error: Could not link program.\n");
        GLint log_size;
        char *log_text;

        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_size);
        log_text = (char *)malloc(log_size+1);
        glGetProgramInfoLog(prog, log_size, NULL, log_text);
        printf("%s\n", log_text);
        free(log_text);
        glDeleteProgram(prog);

        return 0;

    }
    printf("[FJ-GL] Info: Program linked ok.\n");
    return prog;

}
