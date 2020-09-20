#ifndef STATICSERVER_FILE_H
#define STATICSERVER_FILE_H

#pragma once

#include <fcntl.h>

#include <iostream>
#include <filesystem>

class file_exception : public std::runtime_error
{
public:
	file_exception(std::string str) : std::runtime_error(str) {};
};

std::string readAll(const std::string &path);

inline bool exists(const std::string &path) { return std::filesystem::exists(path); }

#endif //STATICSERVER_FILE_H
