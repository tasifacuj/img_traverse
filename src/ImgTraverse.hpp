#pragma once

// std
#include <string>
#include <vector>

namespace img_traverse{
    using Byte = unsigned char;

    struct RawImageData {
        unsigned width; // image width in pixels
        unsigned height; // image height in pixels
        Byte * data; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.
    };


    using ByteArray = std::vector<Byte>;


    struct PackedData{
        std::string ext;
        unsigned width; // image width in pixels
        unsigned height; // image height in pixels
        std::vector<Byte> emptyRowIndices;
        std::vector< ByteArray > rows;
    };
    /*
     * string, unsigned int,  bool, vector<bool>, uint8_t, vector<uint8_t>, vector<vector<uint8_t>>
     */

    class ImageHelper{
    public:
        static constexpr int FULL_WHITE = 0xFF;
        static constexpr int FULL_BLACK = 0x00;
        static constexpr int STEP = 4;

        static constexpr int PACKED_WHITE = 0b0;
        static constexpr int PACKED_BLACK = 0b10;
        static constexpr int PACKED_OTHER = 0b11;
    public:
        static PackedData pack(RawImageData const& data, std::string const& ext);
        static bool unpack( RawImageData& data, PackedData const& pd );
    };


    class Serializer{
    public:
        static void write( PackedData const& data, std::string const& filename );
        static bool read( PackedData& data, std::string const& filename );
    };
}// namespace img_traverse
