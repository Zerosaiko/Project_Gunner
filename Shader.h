#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include "SDL_gpu.h"
#include <string>
#include <vector>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <type_traits>

namespace shader {
    enum class TypeTag : uint8_t {
        Bool,
        Int,
        Sampler1D,
        Sampler2D,
        Sampler3D,
        UInt,
        Float,
        Vec,
        Mat,
        TypeCount

    };

    class TypeSet final : private std::bitset<static_cast<std::underlying_type<TypeTag>::type>(TypeTag::TypeCount)> {
        using enum_type = std::underlying_type<TypeTag>::type;
        using bitset_type = std::bitset<static_cast<enum_type>(TypeTag::TypeCount)>;

        public:
        TypeSet() = default;
        TypeSet(TypeSet const &) = default;
        TypeSet(TypeSet &&) = default;
        TypeSet& operator=(TypeSet const &) = default;
        TypeSet& operator=(TypeSet &&) = default;
        TypeSet(std::initializer_list<TypeTag> il) : TypeSet(std::begin(il), std::end(il)) {}
        TypeSet(TypeTag type) : TypeSet({type}) {}
        template <typename Iterable>
        TypeSet(Iterable settings) : TypeSet(std::begin(settings), std::end(settings)) {}
        template <typename InputIterator>
        TypeSet(InputIterator beg, InputIterator ed) {
            std::for_each(beg, ed, [this](auto tag) {this->set(tag);});
        }

        reference operator[](TypeTag tag) {
            return bitset_type::operator[](static_cast<std::underlying_type<TypeTag>::type>(tag));
        }

        bool operator[](TypeTag tag) const {
            return bitset_type::operator[](static_cast<std::underlying_type<TypeTag>::type>(tag));
        }

        void set(TypeTag tag, bool val = true);

        void set(std::initializer_list<TypeTag> il, bool val = true);

        template <typename Iterable>
        void set(Iterable settings, bool val = true) {
            set(std::begin(settings), std::end(settings), val);
        }

        template <typename InputIterator>
        void set(InputIterator beg, InputIterator ed, bool val = true) {
            std::for_each(beg, ed, [this, val](auto tag) {this->set(tag, val);});
        }

        bool any() const;

        bool none() const;

        bool isSet(TypeTag tag) const;

        bool isSet(std::initializer_list<TypeTag> il) const;

        template <typename Iterable>
        bool isSet(Iterable const &settings) const{
            return isSet(std::begin(settings), std::end(settings));
        }

        template <typename Iterable>
        bool isSet(Iterable &&settings) const{
            return isSet(std::begin(settings), std::end(settings));
        }

        template <typename InputIterator>
        bool isSet(InputIterator beg, InputIterator ed) const{
            bool allSet = true;
            while(beg != ed && (allSet = isSet(*beg))) {
                ++beg;
            }
            return allSet;
        }

        bool operator==(TypeSet const& other) const;

        bool operator!=(TypeSet const& other) const;

    };

    struct VarType {

        union {
            int elements;
            struct {
                int rows, cols;
            };
        };
        int valueCount;

        TypeSet flags;

        VarType(int valueCount, int elements, TypeSet typeFlags = TypeSet());

        VarType(int valueCount, int rows, int cols, TypeSet typeFlags = TypeSet());

    };

    class Value {
        VarType type;
        union {
            std::vector<int32_t> intVals;
            std::vector<uint32_t> uintVals;
            std::vector<float> floatVals;
        };
    public:
        Value(VarType type);
        Value(Value const &);
        Value &operator=(Value const &);
        Value(Value  &&);
        Value &operator=(Value &&);
        ~Value();
        void set(int32_t val);
        void set(uint32_t val);
        void set(float val);

        VarType getType() const {
            return type;
        }

        void setAttribute(int location);

        void setUniform(int location);

        template<typename Iterable>
        void set(Iterable vals) {
            set(std::begin(vals), std::end(vals));
        }

        template<typename InputIterator>
        void set(InputIterator valBeg, InputIterator valEnd) {
            if (type.flags[TypeTag::Int]) {
                auto &vals = intVals;
                for (auto beg = vals.begin(); beg != vals.end() && valBeg != valEnd; ++beg, ++valBeg) {
                    *beg = *valBeg;
                }

            } else if (type.flags[TypeTag::UInt]) {
                auto &vals = uintVals;
                for (auto beg = vals.begin(); beg != vals.end() && valBeg != valEnd; ++beg, ++valBeg) {
                    *beg = *valBeg;
                }

            } else if (type.flags[TypeTag::Float]) {
                auto &vals = floatVals;
                for (auto beg = vals.begin(); beg != vals.end() && valBeg != valEnd; ++beg, ++valBeg) {
                    *beg = *valBeg;
                }

            }
        }
    };

    struct Parameter {

        enum class PassMethod : uint8_t {
            IN = 0,
            OUT,
            INOUT
        };

        PassMethod passMethod;
        VarType type;
        std::string name;

        Parameter(PassMethod passmethod, VarType type, std::string name);

    };

    const extern VarType Void, Bool, Int, UInt, Sampler2D, Float, Vec2, Vec3, Vec4, Mat3, Mat4;

    struct ShaderFunction {
        VarType returnType;
        std::vector<Parameter> parameters;
        std::string name;
        std::string body;

        template<typename Iterable>
        ShaderFunction(VarType returnType, Iterable parameters, std::string name, std::string body = std::string());

        template<typename InputIterator>
        ShaderFunction(VarType returnType, InputIterator it1, InputIterator it2, std::string name, std::string body = std::string());

    };

    template<typename Iterable>
    ShaderFunction::ShaderFunction(VarType returnType, Iterable parameters, std::string name, std::string body) :
        ShaderFunction(returnType, parameters.begin(), parameters.end(), name, body) {}

    template<typename InputIterator>
    ShaderFunction::ShaderFunction(VarType returnType, InputIterator it1, InputIterator it2, std::string name, std::string body) : returnType(returnType),
        parameters(it1, it2), name(name), body(body) {}

    struct ShaderInput {
        std::string name;
        VarType type;
        ShaderInput(std::string name, VarType type) : name(std::move(name)), type(std::move(type)) {}
    };

    class Shader {
    private:
        GPU_ShaderEnum type;
    public:
        std::vector<ShaderInput> attributes;
        std::vector<ShaderInput> uniforms;
        std::vector<ShaderInput> varying;
        std::vector<ShaderFunction> functions;
        ShaderFunction mainFunction;
        template<typename Iterable, typename IterableFunc>
        Shader(GPU_ShaderEnum type,
            Iterable attr, Iterable unif, Iterable vary,
            IterableFunc func, ShaderFunction mainFunction = ShaderFunction(Void, std::vector<Parameter>(), "main")) :
            Shader(type, attr.begin(), attr.end(), unif.begin(), unif.end(), vary.begin(), vary.end(), func.begin(), func.end(), mainFunction) {}

        template<typename InputIterator, typename InputIteratorFunc>
        Shader(GPU_ShaderEnum type,
            InputIterator attrBeg, InputIterator attrEnd,
            InputIterator unifBeg, InputIterator unifEnd,
            InputIterator varyBeg, InputIterator varyEnd,
            InputIteratorFunc funcBeg, InputIteratorFunc funcEnd, ShaderFunction mainFunction = ShaderFunction(Void, std::vector<Parameter>(), "main")) : type(type),
            attributes(attrBeg, attrEnd), uniforms(unifBeg, unifEnd), varying(varyBeg, varyEnd), functions(funcBeg, funcEnd),
            mainFunction(std::move(mainFunction)) {}
        GPU_ShaderEnum getType() const;
    };

    class ShaderProgram {
    private:
        struct InputData {
            ShaderInput input;
            Value value;
            InputData(ShaderInput const &input, Value const &value);

            void setAttribute(uint32_t);

            void setUniform(uint32_t);
        };
        uint32_t programObject;
        std::vector<InputData> attributes, uniforms;
    public:
        template<typename Iterable>
        ShaderProgram(Iterable shaders);
        template<typename InputIterator>
        ShaderProgram(InputIterator shadBegin, InputIterator shadEnd);
        ShaderProgram(std::initializer_list<Shader> il) : ShaderProgram(std::begin(il), std::end(il)) {}

        ShaderProgram(ShaderProgram const &) = delete;
        ShaderProgram &operator=(ShaderProgram const &) = delete;
        ShaderProgram(ShaderProgram &&);
        ShaderProgram &operator=(ShaderProgram &&);
        ~ShaderProgram();

        auto getObj() const {return programObject;}

        GPU_ShaderBlock loadBlock();

        void activate(GPU_ShaderBlock &block);

        void sendInputs();

        void setAttribute(std::string name, int32_t val);

        void setAttribute(std::string name, uint32_t val);

        void setAttribute(std::string name, float val);

        template<typename Iterable>
        void setAttribute(std::string name, Iterable vals) {
            setAttribute(name, std::begin(vals), std::end(vals));
        }

        template<typename InputIterator>
        void setAttribute(std::string name, InputIterator valBeg, InputIterator valEnd) {
            for (auto &data : attributes) {
                if (data.input.name == name) {
                    data.value.set(valBeg, valEnd);
                    break;
                }
            }
        }

        void setUniform(std::string name, int32_t val);

        void setUniform(std::string name, uint32_t val);

        void setUniform(std::string name, float val);

        template<typename Iterable>
        void setUniform(std::string name, Iterable vals) {
            setUniform(name, vals.begin(), vals.end());
        }

        template<typename InputIterator>
        void setUniform(std::string name, InputIterator valBeg, InputIterator valEnd) {
            for (auto &data : uniforms) {
                if (data.input.name == name) {
                    data.value.set(valBeg, valEnd);
                    break;
                }
            }
        }

    };

    template<typename Iterable>
    ShaderProgram::ShaderProgram(Iterable shaders) : ShaderProgram(shaders.begin(), shaders.end()){}

    template<typename InputIterator>
    ShaderProgram::ShaderProgram(InputIterator shadBegin, InputIterator shadEnd) : programObject(GPU_CreateShaderProgram()) {
        std::string shaderBody;
        shaderBody.reserve(512);
        shaderBody += "#version 120\n";
        bool valid = true;
        std::vector<uint32_t> shaderObjs;
        using namespace std::string_literals;
        using std::for_each;

        auto processShader = [&](auto &shader) {
            if (!valid) return;
            auto decodeType = [](VarType const &type) {
                if (type.flags.none()) {
                    return "void"s;
                }
                auto decodeMatrix = [](VarType const &type) {
                    std::string decoded = "mat";
                    decoded += std::to_string(type.cols);
                    if (type.rows != type.cols) {
                        decoded += "x" + std::to_string(type.rows);
                    }
                    return decoded;
                };
                auto decodeNonMatrix = [](VarType const &type) {
                    if (type.elements > 1) {
                        if (type.flags[TypeTag::Bool]) {
                            return "bvec" + std::to_string(type.elements);
                        } else if (type.flags[TypeTag::Int]) {
                            return "ivec" + std::to_string(type.elements);
                        } else if (type.flags[TypeTag::UInt]) {
                            return "uvec" + std::to_string(type.elements);
                        } else if (type.flags[TypeTag::Float]) {
                            return "vec" + std::to_string(type.elements);
                        }
                    } else {
                        if (type.flags[TypeTag::Bool]) {
                            return "bool"s;
                        } else if (type.flags[TypeTag::Sampler1D]) {
                            return "sampler1D"s;
                        } else if (type.flags[TypeTag::Sampler2D]) {
                            return "sampler2D"s;
                        } else if (type.flags[TypeTag::Sampler3D]) {
                            return "sampler3D"s;
                        } else if (type.flags[TypeTag::Int]) {
                            return "int"s;
                        } else if (type.flags[TypeTag::UInt]) {
                            return "uint"s;
                        } else if (type.flags[TypeTag::Float]) {
                            return "float"s;
                        }
                    }
                    return "ERROR"s;
                };
                std::string decoded;
                decoded.reserve(16);
                if (type.flags[TypeTag::Mat]) {
                    decoded = decodeMatrix(type);
                } else {
                    decoded = decodeNonMatrix(type);
                }
                if (type.valueCount > 1) {
                    decoded += "[" + std::to_string(type.valueCount) + "]";
                }
                return decoded;
            };
            for (auto &attr : shader.attributes) {
                shaderBody += "attribute "s + decodeType(attr.type) + " "s + attr.name + ";\n"s;
                auto valueCount = (attr.type.flags[TypeTag::Mat]) ? attr.type.rows * attr.type.cols : attr.type.elements;
                bool alreadyExists = false;
                auto it = attributes.begin();
                while(!alreadyExists && it != attributes.end()) {
                    alreadyExists = it->input.name == attr.name;
                    ++it;
                }
                if (!alreadyExists) {
                    attributes.emplace_back(attr, Value(attr.type));
                }
            }
            for (auto &unif : shader.uniforms) {
                shaderBody += "uniform "s + decodeType(unif.type) + " "s + unif.name + ";\n"s;
                auto valueCount = (unif.type.flags[TypeTag::Mat]) ? unif.type.rows * unif.type.cols : unif.type.elements;
                bool alreadyExists = false;
                auto it = uniforms.begin();
                while(!alreadyExists && it != uniforms.end()) {
                    alreadyExists = it->input.name == unif.name;
                    ++it;
                }
                if (!alreadyExists) {
                    uniforms.emplace_back(unif, Value(unif.type));
                }
            }
            for (auto &vary : shader.varying) {
                shaderBody += "varying "s + decodeType(vary.type) + " "s + vary.name + ";\n"s;
            }
            bool hasMain = !shader.mainFunction.body.empty();
            std::string mainBody;
            auto processFunction = [&decodeType, &shaderBody, &mainBody, &hasMain](auto shaderFunc) {
                shaderBody += decodeType(shaderFunc.returnType) + " "s + shaderFunc.name + "("s;
                for (auto it = shaderFunc.parameters.begin(); it != shaderFunc.parameters.end(); ++it) {
                    if (it->passMethod != Parameter::PassMethod::OUT) {
                        shaderBody += "in"s;
                    }
                    if (it->passMethod != Parameter::PassMethod::IN) {
                        shaderBody += "out"s;
                    }
                    auto typeStr = decodeType(it->type);
                    shaderBody += " "s + typeStr + " "s + it->name + (it + 1 == shaderFunc.parameters.end() ? " "s : ", "s);
                }
                shaderBody += ") {\n"s + shaderFunc.body + "\n}\n"s;
                if (!hasMain)
                    mainBody += shaderFunc.name + "();\n"s;
            };
            for_each(shader.functions.begin(), shader.functions.end(), processFunction);
            processFunction(hasMain ? shader.mainFunction : ShaderFunction(Void, std::vector<Parameter>(), "main", mainBody));

            uint32_t shaderObj = GPU_CompileShader(shader.getType(), shaderBody.c_str());
            //std::cout << "compiling " << shaderBody << '\n';
            valid = shaderObj != 0;
            if (!valid) {
                std::cout << "Error compiling this shader:\n" << shaderBody << '\n';
                std::cout << "{{\n" << GPU_GetShaderMessage() << "}}\n";
                return;
            } else {
                shaderObjs.emplace_back(shaderObj);
            }
            GPU_AttachShader(programObject, shaderObj);
            shaderBody.clear();
        };

        for_each(shadBegin, shadEnd, processShader);
        if (valid){
            for (auto shaderObj : shaderObjs) {
            }
            if (!GPU_LinkShaderProgram(programObject)) {
                std::cout << "Error linking: \n" << GPU_GetShaderMessage() << '\n';
                GPU_FreeShaderProgram(programObject);
                programObject = 0;
            }

        }
        for (auto shaderObj : shaderObjs) {
            GPU_FreeShader(shaderObj);
        }

    }

}

#endif // SHADER_H_INCLUDED
