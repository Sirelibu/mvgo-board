#pragma once 

#include "network/HttpGetter.hpp"

class Board {
public:
    Board(const std::string& globalId = "de:09162:1740") : httpGetter(globalId) {} 
    void run();

private:
    HttpGetter httpGetter;
};
