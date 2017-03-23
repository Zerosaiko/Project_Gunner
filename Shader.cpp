#include "Shader.h"
#include <new>

void shader::TypeSet::set(TypeTag tag, bool val) {
    bitset_type::set(static_cast<uint8_t>(tag), val);
}

void shader::TypeSet::set(std::initializer_list<TypeTag> il, bool val) {
    set(std::begin(il), std::end(il), val);
}

bool shader::TypeSet::any() const {
    return bitset_type::any();
}

bool shader::TypeSet::none() const {
    return !any();
}

bool shader::TypeSet::isSet(TypeTag tag) const {
    return bitset_type::operator[](static_cast<uint8_t>(tag));
}

bool shader::TypeSet::isSet(std::initializer_list<TypeTag> il) const {
    return isSet(std::begin(il), std::end(il));
}

bool shader::TypeSet::operator==(TypeSet const& other) const {
    return bitset_type::operator==((bitset_type)other);
}

bool shader::TypeSet::operator!=(TypeSet const& other) const {
    return !(*this == other);
}

shader::VarType::VarType(int valueCount, int elements, TypeSet typeFlags) :
    elements(elements), valueCount(valueCount) {
    flags = std::move(typeFlags);
}

shader::VarType::VarType(int valueCount, int rows, int cols, TypeSet typeFlags) :
    rows(rows), cols(cols), valueCount(valueCount) {
    flags = std::move(typeFlags);
}

GPU_ShaderEnum shader::Shader::getType() const {
    return type;
}

shader::Value::Value(VarType otherType) : type(std::move(otherType)){
    if (type.flags[TypeTag::Int]) {
        new (&intVals) std::vector<int32_t>(type.elements * type.valueCount);
    } else if (type.flags[TypeTag::UInt]){
        new (&uintVals) std::vector<uint32_t>(type.elements * type.valueCount);
    } else if (type.flags[TypeTag::Mat]) {
        new (&floatVals) std::vector<float>(type.valueCount * type.rows * type.cols);
    } else if (type.flags[TypeTag::Float]){
        new (&floatVals) std::vector<float>(type.elements * type.valueCount);
    }
    /*switch (type.tag) {
        case TypeTag::Bool:
        case TypeTag::Int:
        case TypeTag::Sampler2D:
            new (&intVals) std::vector<int32_t>(type.elements * type.valueCount);
            break;
        case TypeTag::UInt:
            new (&uintVals) std::vector<uint32_t>(type.elements * type.valueCount);
            break;
        case TypeTag::Float:
            new (&floatVals) std::vector<float>(type.elements * type.valueCount);
            break;
        case TypeTag::Mat:
            new (&floatVals) std::vector<float>(type.elements * type.rows * type.cols);
            break;
    }
    */
}

shader::Value::Value(Value const &other) : type(other.type){
    if (type.flags[TypeTag::Int]) {
        new (&intVals) std::vector<int32_t>(other.intVals);
    } else if (type.flags[TypeTag::UInt]){
        new (&uintVals) std::vector<uint32_t>(other.uintVals);
    } else if (type.flags[TypeTag::Float]){
        new (&floatVals) std::vector<float>(other.floatVals);
    }
    /*switch (other.type.tag) {
        case TypeTag::Bool:
        case TypeTag::Int:
        case TypeTag::Sampler2D:
            new (&intVals) std::vector<int32_t>{other.intVals};
            break;
        case TypeTag::UInt:
            new (&uintVals) std::vector<uint32_t>{other.uintVals};
            break;
        case TypeTag::Float:
        case TypeTag::Mat:
            new (&floatVals) std::vector<float>{other.floatVals};
            break;
    }*/
}

shader::Value &shader::Value::operator=(Value const &other){
    using std::vector;
    if (type.flags[TypeTag::Int]) {
        intVals.~vector<int32_t>();
    } else if (type.flags[TypeTag::UInt]) {
        uintVals.~vector<uint32_t>();
    } else if (type.flags[TypeTag::Float]) {
        floatVals.~vector<float>();
    }
    type = other.type;
    if (type.flags[TypeTag::Int]) {
        new (&intVals) std::vector<int32_t>(other.intVals);
    } else if (type.flags[TypeTag::UInt]){
        new (&uintVals) std::vector<uint32_t>(other.uintVals);
    } else if (type.flags[TypeTag::Float]){
        new (&floatVals) std::vector<float>(other.floatVals);
    }
    return *this;
}

shader::Value::Value(Value &&other) : type(other.type) {
    using std::vector;
    if (type.flags[TypeTag::Int]) {
        new (&intVals) std::vector<int32_t>{std::move(intVals)};
    } else if (type.flags[TypeTag::UInt]){
        new (&uintVals) std::vector<uint32_t>{std::move(uintVals)};
    } else if (type.flags[TypeTag::Float]){
        new (&floatVals) std::vector<float>{std::move(floatVals)};
    }
    other.type.flags = TypeSet();

}

shader::Value &shader::Value::operator=(Value &&other) {
    using std::vector;
    bool intBased = type.flags[TypeTag::Int], otherIntBased = other.type.flags[TypeTag::Int];
    bool uIntBased = type.flags[TypeTag::UInt], otherUIntBased = other.type.flags[TypeTag::UInt];
    bool floatBased = type.flags[TypeTag::Float], otherFloatBased = other.type.flags[TypeTag::Float];
    if (intBased && otherIntBased) {
        std::swap(intVals, other.intVals);
        std::swap(type, other.type);
    } else if (uIntBased && otherUIntBased) {
        std::swap(intVals, other.intVals);
        std::swap(type, other.type);
    } else if (floatBased && otherFloatBased) {
        std::swap(intVals, other.intVals);
        std::swap(type, other.type);
    } else {
        using std::vector;
        if (intBased) {
            intVals.~vector<int32_t>();
        } else if (uIntBased) {
            uintVals.~vector<uint32_t>();
        } else if (floatBased) {
            floatVals.~vector<float>();
        }
        type = other.type;
        other.type.flags = TypeSet();
        if (type.flags[TypeTag::Int]) {
            new (&intVals) std::vector<int32_t>{std::move(intVals)};
        } else if (type.flags[TypeTag::UInt]){
            new (&uintVals) std::vector<uint32_t>{std::move(uintVals)};
        } else if (type.flags[TypeTag::Float]){
            new (&floatVals) std::vector<float>{std::move(floatVals)};
        }
        other.type.flags = TypeSet();
    }
    return *this;
}

shader::Value::~Value() {
    using std::vector;
    if (type.flags[TypeTag::Int]) {
        intVals.~vector<int32_t>();
    } else if (type.flags[TypeTag::UInt]) {
        uintVals.~vector<uint32_t>();
    } else if (type.flags[TypeTag::Float]) {
        floatVals.~vector<float>();
    }
}

void shader::Value::setAttribute(int location) {
    if (type.flags[TypeTag::Int]) {
        GPU_SetAttributeiv(location, intVals.size(), intVals.data());
    } else if (type.flags[TypeTag::UInt]) {
        GPU_SetAttributeuiv(location, uintVals.size(), uintVals.data());
    } else if (type.flags[TypeTag::Float]) {
        GPU_SetAttributefv(location, floatVals.size(), floatVals.data());
    }
}

void shader::Value::setUniform(int location) {

    if (type.flags[TypeTag::Int]) {
        GPU_SetUniformiv(location, type.elements, type.valueCount, intVals.data());
    } else if (type.flags[TypeTag::UInt]) {
        GPU_SetUniformuiv(location, type.elements, type.valueCount, uintVals.data());
    } else if (type.flags[TypeTag::Mat]) {
        GPU_SetUniformMatrixfv(location, type.valueCount, type.rows, type.cols, 0, floatVals.data());
    } else if (type.flags[TypeTag::Float]) {
        GPU_SetUniformfv(location, type.elements, type.valueCount, floatVals.data());
    }
}

void shader::Value::set(int32_t val) {
    if (type.flags[TypeTag::Int]) {
        intVals.front() = val;
    }
}
void shader::Value::set(uint32_t val) {
    if (type.flags[TypeTag::UInt]) {
        uintVals.front() = val;
    }
}
void shader::Value::set(float val) {
    if (type.flags[TypeTag::Float]) {
        floatVals.front() = val;
    }
}

shader::ShaderProgram::InputData::InputData(ShaderInput const &input, Value const &value) : input(input), value(value) {};


shader::ShaderProgram::~ShaderProgram() {
    if (programObject) GPU_FreeShaderProgram(programObject);
    programObject = 0;
}

shader::Parameter::Parameter(PassMethod passMethod, VarType type, std::string name) : passMethod(passMethod), type(type), name(name) {}

const shader::VarType shader::Void{0, 0};

const shader::VarType shader::Bool{1, 1, {shader::TypeTag::Bool, shader::TypeTag::Int}};

const shader::VarType shader::Int{1, 1, shader::TypeTag::Int};

const shader::VarType shader::Sampler2D{1, 1, {shader::TypeTag::Sampler2D, shader::TypeTag::Int}};

const shader::VarType shader::UInt{1, 1, shader::TypeTag::UInt};

const shader::VarType shader::Float{1, 1, shader::TypeTag::Float};

const shader::VarType shader::Vec2{1, 2, {shader::TypeTag::Float, shader::TypeTag::Vec}};

const shader::VarType shader::Vec3{1, 3, {shader::TypeTag::Float, shader::TypeTag::Vec}};

const shader::VarType shader::Vec4{1, 4, {shader::TypeTag::Float, shader::TypeTag::Vec}};

const shader::VarType shader::Mat3{1, 3, 3, {shader::TypeTag::Float, shader::TypeTag::Mat}};

const shader::VarType shader::Mat4{1, 4, 4, {shader::TypeTag::Float, shader::TypeTag::Mat}};

shader::ShaderProgram::ShaderProgram(ShaderProgram &&other) : programObject(0) {
    std::swap(programObject, other.programObject);
    std::swap(attributes, other.attributes);
    std::swap(uniforms, other.uniforms);
}

shader::ShaderProgram& shader::ShaderProgram::operator=(ShaderProgram &&other) {
    if (programObject) {
        GPU_FreeShaderProgram(programObject);
    }
    programObject = 0;
    std::swap(programObject, other.programObject);
    std::swap(attributes, other.attributes);
    std::swap(uniforms, other.uniforms);
}

GPU_ShaderBlock shader::ShaderProgram::loadBlock() {
    if (programObject) {
        return GPU_LoadShaderBlock(programObject, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
    }
    return {-1, -1, -1, -1};
}

void shader::ShaderProgram::activate(GPU_ShaderBlock &block) {
    if (!programObject) return;
    GPU_ActivateShaderProgram(programObject, &block);
}

void shader::ShaderProgram::sendInputs() {
    if (!programObject) return;
    for (auto &data : attributes) {
        using namespace std::literals::string_literals;
        std::vector<std::string> autoAttributes = {"gpu_Vertex"s, "gpu_TexCoord"s, "gpu_Color"s, "gpu_ModelViewProjectionMatrix"s};
        bool sendData = true;
        std::for_each(autoAttributes.begin(), autoAttributes.end(),
            [&sendData, &autoAttributes, &data](auto const &s) {
                sendData = sendData && s != data.input.name;
            }
        );
        if (sendData) {
            data.setAttribute(programObject);
        }
    }
    for (auto &data : uniforms) {
        data.setUniform(programObject);
    }

}

void shader::ShaderProgram::setAttribute(std::string name, int32_t val) {
    for (auto &attr : attributes) {
        if (attr.input.name == name && attr.value.getType().flags[TypeTag::Int]) {
            attr.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::setAttribute(std::string name, uint32_t val) {
    for (auto &attr : attributes) {
        if (attr.input.name == name && attr.value.getType().flags[TypeTag::UInt]) {
            attr.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::setAttribute(std::string name, float val) {
    for (auto &attr : attributes) {
        if (attr.input.name == name && attr.value.getType().flags[TypeTag::Float]) {
            attr.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::setUniform(std::string name, int32_t val) {
    for (auto &unif : uniforms) {
        if (unif.input.name == name && unif.value.getType().flags[TypeTag::Int]) {
            unif.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::setUniform(std::string name, uint32_t val) {
    for (auto &unif : uniforms) {
        if (unif.input.name == name && unif.value.getType().flags[TypeTag::UInt]) {
            unif.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::setUniform(std::string name, float val) {
    for (auto &unif : uniforms) {
        if (unif.input.name == name && unif.value.getType().flags[TypeTag::Float]) {
            unif.value.set(val);
            return;
        }
    }
}

void shader::ShaderProgram::InputData::setAttribute(uint32_t programObject) {
    value.setAttribute(GPU_GetAttributeLocation(programObject, input.name.c_str()));
}

void shader::ShaderProgram::InputData::setUniform(uint32_t programObject) {
    value.setUniform(GPU_GetUniformLocation(programObject, input.name.c_str()));
}
