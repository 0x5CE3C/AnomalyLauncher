#pragma once

#include <memory>
#include <string>

class UnZipperPrivate;
class UnZipper {
public:
   
    enum class OpenMode {
        Read,  ///< Mode for reading an existing ZIP file.
        Create,///< Mode for creating a new ZIP file.
        None,  ///< Default mode.
    };

public:
    
    explicit UnZipper(const std::string& zipName, OpenMode mode);    
    explicit UnZipper(const std::string& zipName);
    ~UnZipper();

    void setmode(OpenMode mode);

    bool extract(const std::string& member, const std::string& path);
    
    void extractall(const std::string& path);

    void close();

private:
    std::unique_ptr<UnZipperPrivate> d_ptr;
};