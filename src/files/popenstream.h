// Copyright Mattias Larsson SKöld 2020

#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

//! Open a new process and read the output of that process as a stream
class POpenStream : public std::istream {
private:
    struct POpenStreamBuf : public std::streambuf {
        POpenStreamBuf(std::string command,
                       bool captureStdErr = false,
                       size_t bufferSize = 1024)
            : pfile(popen((command + (captureStdErr ? " 2>&1" : "")).c_str(),
                          "r")) {
            buffer.resize(bufferSize);
            if (!pfile) {
                throw std::runtime_error("could not run command " + command);
            }
        }

        std::streambuf::int_type underflow() override {

            if (!pfile) {
                return std::char_traits<char>::eof();
            }

            if (fgets(buffer.data(), buffer.size(), pfile)) {
                auto len = strlen(buffer.data());
                setg(buffer.data(), buffer.data(), buffer.data() + len);
                return std::char_traits<char>::to_int_type(*this->gptr());
            }
            else {
                returnCode = WEXITSTATUS(pclose(pfile));
                pfile = nullptr;
                return std::char_traits<char>::eof();
            }
        }

        ~POpenStreamBuf() override {
            if (pfile) {
                pclose(pfile);
            }
        }

        std::vector<char> buffer;

        FILE *pfile;
        int returnCode = 0;
    };

public:
    POpenStream(std::string command,
                bool captureStdErr = false,
                size_t bufferSize = 1024)
        : std::istream(&buffer)
        , buffer(command, captureStdErr, bufferSize) {}

    POpenStreamBuf buffer;

    //! Returns non-zero result if there was a error during the execution of
    //! command
    int returnCode() {
        return buffer.returnCode;
    }
};
