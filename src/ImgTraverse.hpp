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
     * @brief class that packs raw image bits into compressed bits and unpacks backward
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
        /*
         * @param data raw image bits
         * @param ext image type
         * @return packed image
         */
        static PackedData pack(RawImageData const& data, std::string const& ext);

        /*
         * @param data place where to unpack bits
         * @param pd compressed image bits
         * @return success or failure
         */
        static bool unpack( RawImageData& data, PackedData const& pd );
    };

    /*
     * @brief file serialzier, that writes packed image into file
     */
    class Serializer{
    public:
        static void write( PackedData const& data, std::string const& filename );
        static bool read( PackedData& data, std::string const& filename );
    };
}// namespace img_traverse
