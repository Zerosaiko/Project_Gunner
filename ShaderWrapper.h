#ifndef SHADERWRAPPER_H_INCLUDED
#define SHADERWRAPPER_H_INCLUDED

#include "SDL_gpu.h"
#include <string>
#include <vector>
#include <unordered_map>

class ShaderG {
private:
    uint32_t shaderObj;

public:
    ShaderG();
    ShaderG(GPU_ShaderEnum type, std::string const data, bool file = false);
    ShaderG(ShaderG const & other ) = delete;
    ShaderG &operator=(ShaderG const & other ) = delete;
    ShaderG(ShaderG && other);
    ShaderG &operator=(ShaderG && other);
    ~ShaderG();
    uint32_t getObj() const;

};

class ShaderProgramG {
private:
    uint32_t progObj;
    std::vector<ShaderG const*> shaderObjs;
public:
    ShaderProgramG();
    ShaderProgramG(std::vector<ShaderG const*> const &shaderObjs);
    ShaderProgramG(ShaderProgramG const &other);
    ShaderProgramG(ShaderProgramG &&other);
    ShaderProgramG &operator=(ShaderProgramG const &other);
    ShaderProgramG &operator=(ShaderProgramG &&other);

    void attachShader(ShaderG const &shd);

    void detachShader(ShaderG const &shd);

    decltype(GPU_LinkShaderProgram(0)) linkProgram();

    ~ShaderProgramG();

    uint32_t getObj() const;

};

extern std::unordered_map<std::string, ShaderG> shaders;

#endif // SHADERWRAPPER_H_INCLUDED
