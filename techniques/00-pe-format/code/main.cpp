#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Section {
    std::string name;
    std::uint32_t virtual_size;
    std::uint32_t virtual_address;
    std::uint32_t raw_size;
    std::uint32_t raw_offset;
    std::uint32_t flags;
};

class PeFile {
public:
    explicit PeFile(const std::string& path) {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) {
            throw std::runtime_error("Unable to open file");
        }
        const std::streamoff length = input.tellg();
        if (length < 0 || static_cast<std::uintmax_t>(length) > 256ULL * 1024 * 1024) {
            throw std::runtime_error("File size is unsupported");
        }
        bytes.resize(static_cast<std::size_t>(length));
        input.seekg(0, std::ios::beg);
        if (!bytes.empty() && !input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
            throw std::runtime_error("Unable to read file");
        }
    }

    void require(std::size_t offset, std::size_t length) const {
        if (offset > bytes.size() || length > bytes.size() - offset) {
            throw std::runtime_error("Truncated or invalid PE structure");
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

    std::uint64_t u64(std::size_t offset) const {
        return static_cast<std::uint64_t>(u32(offset)) |
               (static_cast<std::uint64_t>(u32(offset + 4)) << 32);
    }

    std::string str(std::size_t offset, std::size_t limit) const {
        require(offset, 1);
        std::string value;
        for (std::size_t i = 0; i < limit && offset + i < bytes.size(); ++i) {
            const auto ch = bytes[offset + i];
            if (ch == 0) {
                return value;
            }
            if (ch < 32 || ch > 126) {
                throw std::runtime_error("Non-printable string in PE metadata");
            }
            value.push_back(static_cast<char>(ch));
        }
        throw std::runtime_error("Unterminated string in PE metadata");
    }

    std::string name(std::size_t offset) const {
        require(offset, 8);
        std::string result;
        for (std::size_t i = 0; i < 8 && bytes[offset + i] != 0; ++i) {
            result.push_back(static_cast<char>(bytes[offset + i]));
        }
        return result;
    }

    std::size_t raw_from_rva(std::uint32_t rva, const std::vector<Section>& sections,
                             std::uint32_t header_size) const {
        if (rva < header_size) {
            require(rva, 1);
            return rva;
        }
        for (const auto& section : sections) {
            const auto relative = static_cast<std::uint64_t>(rva) - section.virtual_address;
            if (rva >= section.virtual_address && relative < section.raw_size) {
                const auto raw = static_cast<std::uint64_t>(section.raw_offset) + relative;
                if (raw > std::numeric_limits<std::size_t>::max()) {
                    break;
                }
                require(static_cast<std::size_t>(raw), 1);
                return static_cast<std::size_t>(raw);
            }
        }
        throw std::runtime_error("RVA does not map to file contents");
    }

private:
    std::vector<std::uint8_t> bytes;
};

std::string machine_name(std::uint16_t machine) {
    switch (machine) {
        case 0x14C: return "x86";
        case 0x8664: return "x64";
        case 0xAA64: return "ARM64";
        default: return "unknown";
    }
}

void inspect(const std::string& path) {
    const PeFile file(path);
    if (file.u16(0) != 0x5A4D) {
        throw std::runtime_error("Not an MZ executable");
    }
    const std::size_t nt = file.u32(0x3C);
    file.require(nt, 24);
    if (file.u32(nt) != 0x00004550) {
        throw std::runtime_error("Missing PE signature");
    }

    const std::size_t coff = nt + 4;
    const std::uint16_t machine = file.u16(coff);
    const std::uint16_t section_count = file.u16(coff + 2);
    const std::uint16_t optional_size = file.u16(coff + 16);
    const std::uint16_t characteristics = file.u16(coff + 18);
    const std::size_t optional = coff + 20;
    file.require(optional, optional_size);

    const std::uint16_t magic = file.u16(optional);
    if (magic != 0x10B && magic != 0x20B) {
        throw std::runtime_error("Unknown optional header format");
    }
    const bool pe64 = magic == 0x20B;
    const std::size_t minimum_size = pe64 ? 112 : 96;
    if (optional_size < minimum_size) {
        throw std::runtime_error("Truncated optional header");
    }

    const auto entry = file.u32(optional + 16);
    const auto image_base = pe64 ? file.u64(optional + 24) : file.u32(optional + 28);
    const auto subsystem = file.u16(optional + 68);
    const auto dll_characteristics = file.u16(optional + 70);
    const auto image_size = file.u32(optional + 56);
    const auto header_size = file.u32(optional + 60);
    const auto directory_count = file.u32(optional + (pe64 ? 108 : 92));
    const std::size_t table = optional + optional_size;
    file.require(table, static_cast<std::size_t>(section_count) * 40);

    std::vector<Section> sections;
    for (std::size_t i = 0; i < section_count; ++i) {
        const auto offset = table + i * 40;
        sections.push_back({
            file.name(offset), file.u32(offset + 8), file.u32(offset + 12),
            file.u32(offset + 16), file.u32(offset + 20), file.u32(offset + 36)
        });
    }

    std::cout << "File: " << path << "\n";
    std::cout << "Format: " << (pe64 ? "PE32+" : "PE32")
              << " machine=" << machine_name(machine)
              << " (0x" << std::hex << machine << ")\n";
    std::cout << "Characteristics: 0x" << characteristics
              << " DLLCharacteristics: 0x" << dll_characteristics << "\n";
    std::cout << "ImageBase: 0x" << image_base
              << " EntryPoint RVA: 0x" << entry
              << " SizeOfImage: 0x" << image_size
              << " Subsystem: " << std::dec << subsystem << "\n";
    std::cout << "Sections (" << sections.size() << "):\n";

    for (const auto& section : sections) {
        std::cout << "  " << std::setw(8) << std::left << section.name
                  << " RVA=0x" << std::hex << section.virtual_address
                  << " VirtualSize=0x" << section.virtual_size
                  << " RawOffset=0x" << section.raw_offset
                  << " RawSize=0x" << section.raw_size
                  << " Flags=0x" << section.flags << std::dec << "\n";
    }

    std::cout << "Import DLLs:\n";
    if (directory_count <= 1) {
        std::cout << "  none\n";
        return;
    }

    const std::size_t directory = optional + (pe64 ? 112 : 96) + 8;
    if (directory + 8 > optional + optional_size) {
        throw std::runtime_error("Import directory is outside optional header");
    }
    const auto import_rva = file.u32(directory);
    const auto import_size = file.u32(directory + 4);
    if (import_rva == 0 || import_size == 0) {
        std::cout << "  none\n";
        return;
    }

    const std::size_t count_limit = std::min<std::size_t>(256, import_size / 20);
    bool found = false;
    for (std::size_t i = 0; i < count_limit; ++i) {
        const auto descriptor = file.raw_from_rva(
            static_cast<std::uint32_t>(static_cast<std::uint64_t>(import_rva) + i * 20),
            sections, header_size
        );
        file.require(descriptor, 20);
        const auto original_thunk = file.u32(descriptor);
        const auto name_rva = file.u32(descriptor + 12);
        const auto first_thunk = file.u32(descriptor + 16);
        if (original_thunk == 0 && name_rva == 0 && first_thunk == 0) {
            break;
        }
        if (name_rva == 0) {
            throw std::runtime_error("Import descriptor missing name");
        }
        const auto raw = file.raw_from_rva(name_rva, sections, header_size);
        std::cout << "  " << file.str(raw, 260) << "\n";
        found = true;
    }
    if (!found) {
        std::cout << "  none\n";
    }
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: pe-inspect <path-to-pe-file>\n";
        return 1;
    }
    try {
        inspect(argv[1]);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 2;
    }
}
