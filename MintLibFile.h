
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Mint::File {

    namespace fs = std::filesystem;

    // Information about a file or directory
    struct FileInfo {
        std::string name;
        std::string path;
        std::string extension;
        bool exists = false;
        bool isFile = false;
        bool isDirectory = false;
        std::uintmax_t size = 0;
        std::int64_t modifiedTimeTicks = 0;
    };

    // An entry returned when listing a directory
    struct DirectoryEntry {
        std::string name;
        std::string path;
        bool isFile = false;
        bool isDirectory = false;
        std::uintmax_t size = 0;
    };

    // PATHS
    std::string filename(const std::string& path);
    std::string stem(const std::string& path);
    std::string extension(const std::string& path);
    std::string parentPath(const std::string& path);
    std::string absolutePath(const std::string& path);
    std::string normalizePath(const std::string& path);
    std::string combinePath(
        const std::string& first,
        const std::string& second
    );
    bool isAbsolutePath(const std::string& path);

    // EXISTENCE AND TYPE
    bool exists(const std::string& path);
    bool isFile(const std::string& path);
    bool isDirectory(const std::string& path);
    bool isEmpty(const std::string& path);
    bool isReadable(const std::string& path);
    bool isWritable(const std::string& path);

    // FILE INFORMATION
    std::uintmax_t fileSize(const std::string& path);
    std::int64_t modifiedTimeTicks(const std::string& path);
    FileInfo getInfo(const std::string& path);

    // FILE CREATION AND DELETION
    bool createFile(const std::string& path);
    bool deleteFile(const std::string& path);
    bool renameFile(
        const std::string& oldPath,
        const std::string& newPath
    );
    bool copyFile(
        const std::string& source,
        const std::string& destination,
        bool overwrite = false
    );
    bool moveFile(
        const std::string& source,
        const std::string& destination,
        bool overwrite = false
    );

    // TEXT FILES
    std::string readText(const std::string& path);
    bool tryReadText(
        const std::string& path,
        std::string& output
    );
    bool writeText(
        const std::string& path,
        const std::string& text
    );
    bool appendText(
        const std::string& path,
        const std::string& text
    );
    bool readLines(
        const std::string& path,
        std::vector<std::string>& lines
    );
    bool writeLines(
        const std::string& path,
        const std::vector<std::string>& lines
    );

    // BINARY FILES
    std::vector<std::uint8_t> readBinary(
        const std::string& path
    );
    bool tryReadBinary(
        const std::string& path,
        std::vector<std::uint8_t>& output
    );
    bool writeBinary(
        const std::string& path,
        const std::vector<std::uint8_t>& data
    );

    // DIRECTORIES
    bool createDirectory(const std::string& path);
    bool createDirectories(const std::string& path);
    bool deleteDirectory(const std::string& path);
    bool deleteDirectoryRecursive(const std::string& path);
    std::vector<DirectoryEntry> listDirectory(
        const std::string& path
    );
    std::vector<DirectoryEntry> listDirectoryRecursive(
        const std::string& path
    );
    std::uintmax_t countEntries(
        const std::string& path,
        bool recursive = false
    );
    std::uintmax_t countFiles(
        const std::string& path,
        bool recursive = false
    );
    std::uintmax_t countDirectories(
        const std::string& path,
        bool recursive = false
    );

    // SEARCH
    std::vector<std::string> findFiles(
        const std::string& directory,
        const std::string& extension,
        bool recursive = false
    );
    std::vector<std::string> findName(
        const std::string& directory,
        const std::string& name,
        bool recursive = false
    );

    // ERROR HANDLING
    std::string lastError();
    void clearError();

}
