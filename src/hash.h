// Copyright Mattias Larsson SKöld 2020

#pragma once

#include "msl/range.h"
#include <array>
#include <iostream>
#include <string_view>

class Hash {
public:
    void process(std::istream &stream) {
        while (stream) {
            std::array<char, hashLen> d = {};
            stream.read(d.data(), d.size());

            for (auto i : msl::range(data.size())) {
                data[i] ^= d[i];
            }
        }
    }

    //! This hash alghorithm probably sucks. I have not googled it, i have just
    //! heard that xor could be used fore someothing :P
    std::string_view hash() {
        for (auto i : msl::range(data.size())) {
            strData[i * 2] = ((data[i] & 0xf0) >> 4) + 'a';
            strData[i * 2 + 1] = (data[i] & 0x0f) + 'a';
        }

        return {strData.data(), strData.size()};
    }

    static constexpr size_t hashLen = 28;
    std::array<char, hashLen> data{0};

    std::array<char, hashLen * 2> strData;
};

//! Funcction for hashing when it can be done in one swoop
std::string hash(std::istream &stream) {
    Hash hash;
    hash.process(stream);

    return std::string{hash.hash()};
}
