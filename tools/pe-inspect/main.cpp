#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

class PeFile {
public:
    explicit PeFile(const std::string& path) {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) {
            throw std::runtime_error("Unable to open file");
        }

        const std::streamoff size = input.tellg();
        if (size < 0 || static_cast<std::uintmax_t>(size) > std::numeric_limits<std::size_t>::max()) {
            throw std::runtime_error("Unsupported file size");
        }

        bytes.resize(static_cast<std::size_t>(size));
        input.seekg(0, std::ios::beg);

        if (!bytes.empty() && !input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
            throw std::runtime_error("Unable to read file");
        }
    }

    std::uint16_t u16(std::size_t offset) const {
        require(offset, 2);
        return static_cast<std::uint16_t>(bytes[offset]) |
               (static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
    }

    std::uint32_t u32(std::size_t offset) const {
        require(offset, 4);
        return static_cast<std::uint32_t>(bytes[offset]) |
               (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
               (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
               (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
    }

    std::string section_name(std::size_t offset) const {
        require(offset, 8);
        std::string name;
        for (std::size_t i = 0; i < 8 && bytes[offset + i] != 0; ++i) {
            name.push_back(static_cast<char>(bytes[offset + i]));
        }
        return name;
    }

    void require(std::size_t offset, std::size_t count) const {
        if (offset > bytes.size() || count > bytes.size() - offset) {
            throw std::runtime_error("Truncated or invalid PE structure");
        }
    }

private:
    std::vector<std::uint8_t> bytes;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: pe-inspect <path-to-pe-file>\n";
        return 1;
    }

    try {
        const PeFile file(argv[1]);
        if (file.u16(0) != 0x5A4D) {
            throw std::runtime_error("Not an MZ executable");
        }

        const std::size_t nt = file.u32(0x3C);
        if (file.u32(nt) != 0x00004550) {
            throw std::runtime_error("Missing PE signature");
        }

        const std::size_t coff = nt + 4;
        file.require(coff, 20);

        const std::uint16_t machine = file.u16(coff);
        const std::uint16_t sections = file.u16(coff + 2);
        const std::uint16_t optional_size = file.u16(coff + 16);
        const std::uint16_t characteristics = file.u16(coff + 18);

        const std::size_t optional = coff + 20;
        file.require(optional, optional_size);

        if (optional_size < 20) {
            throw std::runtime_error("Optional header too small");
        }

        const std::uint16_t magic = file.u16(optional);
        if (magic != 0x10B && magic != 0x20B) {
            throw std::runtime_error("Unknown PE optional header format");
        }

        const std::uint32_t entry = file.u32(optional + 16);
        const std::size_t table = optional + optional_size;
        file.require(table, static_cast<std::size_t>(sections) * 40);

        std::cout << "File: " << argv[1] << "\n";
        std::cout << "Format: " << (magic == 0x20B ? "PE32+" : "PE32") << "\n";
        std::cout << "Machine: 0x" << std::hex << machine << "\n";
        std::cout << "Characteristics: 0x" << characteristics << "\n";
        std::cout << "Entry point RVA: 0x" << entry << "\n";
        std::cout << "Sections: " << std::dec << sections << "\n";

        for (std::size_t i = 0; i < sections; ++i) {
            const std::size_t section = table + i * 40;
            const std::string name = file.section_name(section);
            const std::uint32_t virtual_size = file.u32(section + 8);
            const std::uint32_t virtual_address = file.u32(section + 12);
            const std::uint32_t raw_size = file.u32(section + 16);
            const std::uint32_t raw_offset = file.u32(section + 20);
            const std::uint32_t flags = file.u32(section + 36);

            std::cout << "  " << (i + 1) << ". " << name
                      << " VA=0x" << std::hex << virtual_address
                      << " VS=0x" << virtual_size
                      << " RAW=0x" << raw_offset
                      << " SIZE=0x" << raw_size
                      << " FLAGS=0x" << flags << std::dec << "\n";
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 2;
    }
}
