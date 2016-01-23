#include "displace.h"
#include <locale>

Displace::DisplaceFactory* Displace::factory{nullptr};

std::size_t Displace::DisplaceFactory::build(EntityManager* manager, size_t idx, std::string instructions) {
    auto& pool = Displace::componentPools[manager];
    pool[idx].build(tokenize(instructions));
    return idx;
}

std::size_t Displace::DisplaceFactory::build(EntityManager* manager, size_t idx, ComponentBase* cmp) {
    auto& pool = Displace::componentPools[manager];
    pool[idx] = *(Displace*)cmp;
    return idx;
}

std::size_t Displace::DisplaceFactory::build(EntityManager* manager, std::string instructions) {
    auto& pool = Displace::componentPools[manager];
    pool.emplace_back(tokenize(instructions));
    return pool.size() - 1;

}

std::size_t Displace::DisplaceFactory::build(EntityManager* manager, ComponentBase* cmp) {
    auto& pool = Displace::componentPools[manager];
    pool.emplace_back(*(Displace*)cmp);
    return pool.size() - 1;
}

std::vector<std::string> Displace::DisplaceFactory::tokenize(std::string instructions) {
    auto beginCmp = instructions.find_first_of(" \n\f\r\t\v", instructions.find("component:displace"));
    std::vector<std::string> tokenizedString;
    tokenizedString.reserve(5);
    int8_t i = 0;
    std::string::size_type tokenStart = std::string::npos;
    std::locale loc;
    if (beginCmp != std::string::npos) {
        tokenizedString.push_back("component:displace");
        for(auto start = beginCmp; start != std::string::npos &&
                start < instructions.size() && i < 4; ++start) {
            auto ch = instructions[start];
            if (!std::isspace(ch, loc) && ch != ',' && ch != '\n' && tokenStart == std::string::npos) {
                tokenStart = start;
            }
            if ( (std::isspace(ch, loc) || ch == ',' || ch== '\n') && tokenStart != std::string::npos) {
                tokenizedString.emplace_back(instructions.substr(tokenStart, start - tokenStart));
                ++i;
                tokenStart = std::string::npos;
            }

        }

        if (tokenStart != std::string::npos)
            tokenizedString.emplace_back(instructions.substr(tokenStart));

    }

    for( auto token : tokenizedString) {

    }

    return tokenizedString;
}

void Displace::DisplaceFactory::registerManager(EntityManager* manager) {

    Displace::componentPools[manager];

}

void Displace::DisplaceFactory::deregisterManager(EntityManager* manager) {

    Displace::componentPools.erase(manager);

}

void Displace::registerComponent() {
    if (!factory) factory = new Displace::DisplaceFactory();
    componentUtils::factoryMap["displace"] = factory;
}

std::map<EntityManager*, std::vector<Displace>> Displace::componentPools{};

const std::string Displace::getName(){ return "displace"; };

void Displace::build(std::vector<std::string> instructions) {

    if (instructions.size() >= 5) {
        posX = buildFromString<float>(instructions, 1);
        posY = buildFromString<float>(instructions, 2);
        velX = buildFromString<float>(instructions, 3);
        velY = buildFromString<float>(instructions, 4);
    }

}
