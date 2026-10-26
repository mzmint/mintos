#include "MintLibFile.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <system_error>
#include <stdexcept>

namespace Mint::File {

namespace {
    thread_local std::string errorMessage;

    void setError(const std::error_code& error) {
        errorMessage = error.message();
    }

    void setError(const std::string& error) {
        errorMessage = error;
    }

    void clearErrorInternal() {
        errorMessage.clear();
    }

    bool checkError(const std::error_code& error) {
        if (error) {
            setError(error);
            return true;
        }

        clearErrorInternal();
        return false;
    }

    DirectoryEntry makeEntry(const fs::directory_entry& entry) {
        DirectoryEntry result;
        std::error_code ec;

        result.name = entry.path().filename().string();
        result.path = entry.path().string();
        result.isFile = entry.is_regular_file(ec);

        ec.clear();
        result.isDirectory = entry.is_directory(ec);

        ec.clear();
        if (result.isFile)
            result.size = entry.file_size(ec);

        return result;
    }

    std::vector<DirectoryEntry> listImpl(
        const std::string& path,
        bool recursive
    ) {
        std::vector<DirectoryEntry> entries;
        std::error_code ec;

        if (!fs::is_directory(path, ec)) {
            setError(ec ? ec : std::make_error_code(
                std::errc::not_a_directory
            ));
            return entries;
        }

        if (recursive) {
            fs::recursive_directory_iterator it(
                path, fs::directory_options::skip_permission_denied, ec
            );
            fs::recursive_directory_iterator end;

            if (ec) {
                setError(ec);
                return entries;
            }

            for (; it != end; it.increment(ec)) {
                if (ec) {
                    setError(ec);
                    ec.clear();
                    continue;
                }

                entries.push_back(makeEntry(*it));
            }
        } else {
            fs::directory_iterator it(
                path, fs::directory_options::skip_permission_denied, ec
            );
            fs::directory_iterator end;

            if (ec) {
                setError(ec);
                return entries;
            }

            for (; it != end; it.increment(ec)) {
                if (ec) {
                    setError(ec);
                    ec.clear();
                    continue;
                }

                entries.push_back(makeEntry(*it));
            }
        }

        clearErrorInternal();
        return entries;
    }
}

// PATHS

std::string filename(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string stem(const std::string& path) {
    return fs::path(path).stem().string();
}

std::string extension(const std::string& path) {
    return fs::path(path).extension().string();
}

std::string parentPath(const std::string& path) {
    return fs::path(path).parent_path().string();
}

std::string absolutePath(const std::string& path) {
    std::error_code ec;
    fs::path result = fs::absolute(path, ec);

    if (checkError(ec))
        return {};

    return result.lexically_normal().string();
}

std::string normalizePath(const std::string& path) {
    return fs::path(path).lexically_normal().string();
}

std::string combinePath(
    const std::string& first,
    const std::string& second
) {
    return (fs::path(first) / fs::path(second)).lexically_normal().string();
}

bool isAbsolutePath(const std::string& path) {
    return fs::path(path).is_absolute();
}

// EXISTENCE AND TYPE

bool exists(const std::string& path) {
    std::error_code ec;
    bool result = fs::exists(path, ec);
    checkError(ec);
    return result;
}

bool isFile(const std::string& path) {
    std::error_code ec;
    bool result = fs::is_regular_file(path, ec);
    checkError(ec);
    return result;
}

bool isDirectory(const std::string& path) {
    std::error_code ec;
    bool result = fs::is_directory(path, ec);
    checkError(ec);
    return result;
}

bool isEmpty(const std::string& path) {
    std::error_code ec;
    bool result = fs::is_empty(path, ec);
    checkError(ec);
    return result;
}

bool isReadable(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        setError("Cannot open path for reading: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

bool isWritable(const std::string& path) {
    if (isDirectory(path)) {
        setError("Path is a directory: " + path);
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        setError("Cannot open path for writing: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

// FILE INFORMATION

std::uintmax_t fileSize(const std::string& path) {
    std::error_code ec;
    auto result = fs::file_size(path, ec);

    if (checkError(ec))
        return 0;

    return result;
}

std::int64_t modifiedTimeTicks(const std::string& path) {
    std::error_code ec;
    auto time = fs::last_write_time(path, ec);

    if (checkError(ec))
        return 0;

    return static_cast<std::int64_t>(
        time.time_since_epoch().count()
    );
}

FileInfo getInfo(const std::string& path) {
    FileInfo info;
    fs::path p(path);
    std::error_code ec;

    info.name = p.filename().string();
    info.path = p.string();
    info.extension = p.extension().string();
    info.exists = fs::exists(p, ec);

    if (ec) {
        setError(ec);
        return info;
    }

    if (!info.exists) {
        clearErrorInternal();
        return info;
    }

    info.isFile = fs::is_regular_file(p, ec);
    ec.clear();
    info.isDirectory = fs::is_directory(p, ec);

    ec.clear();
    if (info.isFile)
        info.size = fs::file_size(p, ec);

    ec.clear();
    auto time = fs::last_write_time(p, ec);
    if (!ec) {
        info.modifiedTimeTicks = static_cast<std::int64_t>(
            time.time_since_epoch().count()
        );
    }

    clearErrorInternal();
    return info;
}

// FILE CREATION AND DELETION

bool createFile(const std::string& path) {
    std::error_code ec;

    if (fs::exists(path, ec)) {
        setError("Path already exists: " + path);
        return false;
    }

    if (ec)
        return !checkError(ec);

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        setError("Cannot create file: " + path);
        return false;
    }

    file.close();

    if (file.fail()) {
        setError("Failed to finish creating file: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

bool deleteFile(const std::string& path) {
    std::error_code ec;

    if (!fs::is_regular_file(path, ec)) {
        if (ec)
            setError(ec);
        else
            setError("Not a regular file: " + path);
        return false;
    }

    bool result = fs::remove(path, ec);
    if (checkError(ec))
        return false;

    if (!result) {
        setError("File was not removed: " + path);
        return false;
    }

    return true;
}

bool renameFile(
    const std::string& oldPath,
    const std::string& newPath
) {
    std::error_code ec;
    fs::rename(oldPath, newPath, ec);
    return !checkError(ec);
}

bool copyFile(
    const std::string& source,
    const std::string& destination,
    bool overwrite
) {
    std::error_code ec;

    auto options = overwrite
        ? fs::copy_options::overwrite_existing
        : fs::copy_options::none;

    bool result = fs::copy_file(source, destination, options, ec);

    if (checkError(ec))
        return false;

    if (!result) {
        setError("File was not copied.");
        return false;
    }

    return true;
}

bool moveFile(
    const std::string& source,
    const std::string& destination,
    bool overwrite
) {
    std::error_code ec;

    if (overwrite && fs::exists(destination, ec)) {
        if (ec) {
            setError(ec);
            return false;
        }

        if (fs::is_directory(destination, ec)) {
            setError("Destination is a directory.");
            return false;
        }

        fs::remove(destination, ec);
        if (checkError(ec))
            return false;
    }

    fs::rename(source, destination, ec);

    if (!ec) {
        clearErrorInternal();
        return true;
    }

    // A rename can fail when source and destination are
    // on different drives. Try copying and then deleting.
    ec.clear();

    if (!fs::is_regular_file(source, ec)) {
        setError(ec ? ec : std::make_error_code(
            std::errc::invalid_argument
        ));
        return false;
    }

    if (!copyFile(source, destination, overwrite))
        return false;

    if (!deleteFile(source)) {
        setError("Copied file, but could not remove the original.");
        return false;
    }

    clearErrorInternal();
    return true;
}

// TEXT FILES

std::string readText(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        setError("Cannot open file for reading: " + path);
        throw std::runtime_error(errorMessage);
    }

    std::string result{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    if (file.bad()) {
        setError("Error while reading file: " + path);
        throw std::runtime_error(errorMessage);
    }

    clearErrorInternal();
    return result;
}

bool tryReadText(
    const std::string& path,
    std::string& output
) {
    try {
        output = readText(path);
        return true;
    } catch (const std::exception& e) {
        setError(e.what());
        return false;
    }
}

bool writeText(
    const std::string& path,
    const std::string& text
) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        setError("Cannot open file for writing: " + path);
        return false;
    }

    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    file.close();

    if (file.fail()) {
        setError("Error while writing file: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

bool appendText(
    const std::string& path,
    const std::string& text
) {
    std::ofstream file(path, std::ios::binary | std::ios::app);

    if (!file.is_open()) {
        setError("Cannot open file for appending: " + path);
        return false;
    }

    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    file.close();

    if (file.fail()) {
        setError("Error while appending to file: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

bool readLines(
    const std::string& path,
    std::vector<std::string>& lines
) {
    std::ifstream file(path);

    if (!file.is_open()) {
        setError("Cannot open file for reading: " + path);
        return false;
    }

    std::vector<std::string> result;
    std::string line;

    while (std::getline(file, line))
        result.push_back(line);

    if (file.bad()) {
        setError("Error while reading lines: " + path);
        return false;
    }

    lines = std::move(result);
    clearErrorInternal();
    return true;
}

bool writeLines(
    const std::string& path,
    const std::vector<std::string>& lines
) {
    std::ofstream file(path, std::ios::trunc);

    if (!file.is_open()) {
        setError("Cannot open file for writing: " + path);
        return false;
    }

    for (std::size_t i = 0; i < lines.size(); ++i) {
        file << lines[i];

        if (i + 1 < lines.size())
            file << '\n';
    }

    file.close();

    if (file.fail()) {
        setError("Error while writing lines: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

// BINARY FILES

std::vector<std::uint8_t> readBinary(
    const std::string& path
) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        setError("Cannot open binary file: " + path);
        throw std::runtime_error(errorMessage);
    }

    std::vector<std::uint8_t> data{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    if (file.bad()) {
        setError("Error while reading binary file: " + path);
        throw std::runtime_error(errorMessage);
    }

    clearErrorInternal();
    return data;
}

bool tryReadBinary(
    const std::string& path,
    std::vector<std::uint8_t>& output
) {
    try {
        output = readBinary(path);
        return true;
    } catch (const std::exception& e) {
        setError(e.what());
        return false;
    }
}

bool writeBinary(
    const std::string& path,
    const std::vector<std::uint8_t>& data
) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        setError("Cannot open binary file for writing: " + path);
        return false;
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    file.close();

    if (file.fail()) {
        setError("Error while writing binary file: " + path);
        return false;
    }

    clearErrorInternal();
    return true;
}

// DIRECTORIES

bool createDirectory(const std::string& path) {
    std::error_code ec;
    bool result = fs::create_directory(path, ec);

    if (checkError(ec))
        return false;

    if (!result) {
        setError("Directory already exists or was not created: " + path);
        return false;
    }

    return true;
}

bool createDirectories(const std::string& path) {
    std::error_code ec;
    bool result = fs::create_directories(path, ec);

    if (checkError(ec))
        return false;

    // Existing directories count as success.
    if (!result && fs::is_directory(path, ec)) {
        clearErrorInternal();
        return true;
    }

    if (ec)
        return !checkError(ec);

    clearErrorInternal();
    return true;
}

bool deleteDirectory(const std::string& path) {
    std::error_code ec;

    if (!fs::is_directory(path, ec)) {
        if (ec)
            setError(ec);
        else
            setError("Not a directory: " + path);
        return false;
    }

    if (fs::is_empty(path, ec) && !ec) {
        bool result = fs::remove(path, ec);

        if (checkError(ec))
            return false;

        if (!result) {
            setError("Directory was not removed: " + path);
            return false;
        }

        return true;
    }

    if (ec) {
        setError(ec);
        return false;
    }

    setError("Directory is not empty: " + path);
    return false;
}

bool deleteDirectoryRecursive(const std::string& path) {
    std::error_code ec;

    if (!fs::is_directory(path, ec)) {
        if (ec)
            setError(ec);
        else
            setError("Not a directory: " + path);
        return false;
    }

    fs::remove_all(path, ec);
    return !checkError(ec);
}

std::vector<DirectoryEntry> listDirectory(
    const std::string& path
) {
    return listImpl(path, false);
}

std::vector<DirectoryEntry> listDirectoryRecursive(
    const std::string& path
) {
    return listImpl(path, true);
}

std::uintmax_t countEntries(
    const std::string& path,
    bool recursive
) {
    return static_cast<std::uintmax_t>(
        recursive
            ? listDirectoryRecursive(path).size()
            : listDirectory(path).size()
    );
}

std::uintmax_t countFiles(
    const std::string& path,
    bool recursive
) {
    auto entries = recursive
        ? listDirectoryRecursive(path)
        : listDirectory(path);

    return static_cast<std::uintmax_t>(
        std::count_if(entries.begin(), entries.end(),
            [](const DirectoryEntry& entry) {
                return entry.isFile;
            })
    );
}

std::uintmax_t countDirectories(
    const std::string& path,
    bool recursive
) {
    auto entries = recursive
        ? listDirectoryRecursive(path)
        : listDirectory(path);

    return static_cast<std::uintmax_t>(
        std::count_if(entries.begin(), entries.end(),
            [](const DirectoryEntry& entry) {
                return entry.isDirectory;
            })
    );
}

// SEARCH

std::vector<std::string> findFiles(
    const std::string& directory,
    const std::string& wantedExtension,
    bool recursive
) {
    std::vector<std::string> results;

    auto entries = recursive
        ? listDirectoryRecursive(directory)
        : listDirectory(directory);

    std::string wanted = wantedExtension;

    if (!wanted.empty() && wanted.front() != '.')
        wanted.insert(wanted.begin(), '.');

    for (const auto& entry : entries) {
        if (entry.isFile &&
            (wanted.empty() || extension(entry.path) == wanted)) {
            results.push_back(entry.path);
        }
    }

    return results;
}

std::vector<std::string> findName(
    const std::string& directory,
    const std::string& wantedName,
    bool recursive
) {
    std::vector<std::string> results;

    auto entries = recursive
        ? listDirectoryRecursive(directory)
        : listDirectory(directory);

    for (const auto& entry : entries) {
        if (entry.name == wantedName)
            results.push_back(entry.path);
    }

    return results;
}

// ERROR HANDLING

std::string lastError() {
    return errorMessage;
}

void clearError() {
    clearErrorInternal();
}

}
