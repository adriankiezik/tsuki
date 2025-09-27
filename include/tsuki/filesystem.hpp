#pragma once

#include <string>
#include <vector>
#include <iosfwd>

namespace tsuki {

enum class FileMode {
    Read,
    Write,
    Append
};

class File {
public:
    File() = default;
    explicit File(const std::string& filename, FileMode mode = FileMode::Read);
    ~File();

    File(const File&) = delete;
    File& operator=(const File&) = delete;
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    bool open(const std::string& filename, FileMode mode = FileMode::Read);
    void close();

    bool isOpen() const;
    bool isEOF() const;

    // Reading
    std::string read();
    std::string read(size_t bytes);
    std::string readLine();
    std::vector<uint8_t> readBytes();
    std::vector<uint8_t> readBytes(size_t bytes);

    // Writing
    bool write(const std::string& data);
    bool write(const std::vector<uint8_t>& data);
    bool writeLine(const std::string& line);

    // File properties
    size_t getSize() const;
    std::string getFilename() const { return filename_; }

private:
    void* file_impl_; // PIMPL idiom to avoid including fstream
    std::string filename_;
    FileMode mode_;
};

class Filesystem {
public:
    Filesystem() = default;
    ~Filesystem() = default;

    // File operations
    bool exists(const std::string& filename) const;
    bool isFile(const std::string& path) const;
    bool isDirectory(const std::string& path) const;

    size_t getSize(const std::string& filename) const;
    std::string read(const std::string& filename) const;
    std::vector<uint8_t> readBytes(const std::string& filename) const;

    bool write(const std::string& filename, const std::string& data) const;
    bool write(const std::string& filename, const std::vector<uint8_t>& data) const;
    bool append(const std::string& filename, const std::string& data) const;

    // Directory operations
    std::vector<std::string> getDirectoryItems(const std::string& path) const;
    bool createDirectory(const std::string& path) const;
    bool remove(const std::string& path) const;

    // Path manipulation
    std::string getWorkingDirectory() const;
    bool setWorkingDirectory(const std::string& path);

    std::string getRealPath(const std::string& path) const;
    std::string getParentDirectory(const std::string& path) const;
    std::string getBasename(const std::string& path) const;
    std::string getExtension(const std::string& path) const;

    // Path joining
    std::string joinPath(const std::string& a, const std::string& b) const;

    // Special directories
    std::string getUserDirectory() const;
    std::string getDocumentsDirectory() const;
    std::string getTempDirectory() const;

private:
    std::string normalizePath(const std::string& path) const;
};

} // namespace tsuki