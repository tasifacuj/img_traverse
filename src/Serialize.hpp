#pragma once

#include <string>
#include <type_traits>
#include <streambuf>
#include <sstream>
#include <vector>

namespace img_traverse{
    namespace serialize{
        template<typename Visitor>
        struct StructVisitor {
            char const* name;
            Visitor& visitor;

            template<typename T>
            StructVisitor& field( T& value ) {
                visit(visitor, value);
                return *this;
            }
        };

        template<typename Visitor>
        inline  StructVisitor<Visitor> makeVisitor( char const* name, Visitor& visitor ) {
            return StructVisitor<Visitor>{name, visitor};
        }


        struct BinaryWriter {
            std::ostream& out_;
            BinaryWriter(std::ostream& o): out_(o) {}
        };

        // https://developers.google.com/protocol-buffers/docs/encoding?hl=en#signed-integers
        // 0b111 ==> 0b00000111
        // 0b1111111100000000 ==> 0b10000000 0b11111110 0b00000011
        inline void write_unsigned_int(std::ostream& out, uint64_t value) {
            do {
                uint8_t c = value & 0x7f;
                value >>= 7;
                if (value) c |= 0x80;
                out.put(c);
            } while(value);
        }


        inline void visit(BinaryWriter& writer, const std::string& string) {
            uint64_t size = string.size();
            write_unsigned_int(writer.out_, size);
            writer.out_.write(&string[0], size);
        }


        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_signed_v<T>>
            visit(BinaryWriter& writer, const T& value) {
            uint64_t wide_value = uint64_t(value);
            write_unsigned_int(writer.out_, wide_value);
        }


        inline void visit(BinaryWriter& writer, bool value) {
            uint64_t wide_value = uint64_t(value);
            write_unsigned_int(writer.out_, wide_value);
        }

        template<typename T>
        inline void visit(BinaryWriter& writer, std::vector< T > const& vector) {
            uint64_t size = vector.size();
            write_unsigned_int(writer.out_, size);

            for (auto const& element : vector) {
                visit(writer, element);
            }
        }

    //-------------------------------------- deserialize ----------------------------------------------------//
        struct BinaryReader {
            std::streambuf& in;
            std::stringstream errors;
            BinaryReader(std::streambuf& buf): in(buf) {}

            std::string getErrors() { return errors.str(); }
        };

        inline bool read_unsigned_int(std::streambuf& in, uint64_t& value) {
            uint64_t result = 0;

            for (int byte = 0; ; ++byte) {
                int c = in.sbumpc();

                if (c < 0) {
                    return false;
                }

                bool endbit = (c & 0x80) == 0;
                result |= uint64_t(c & 0x7f) << (byte * 7);

                if (endbit) {
                    break;
                }
            }

            value = result;
            return true;
        }

        inline void visit(BinaryReader& reader, std::string& string) {
            uint64_t size = 0;
            if (!read_unsigned_int(reader.in, size)) {
                reader.errors << "Error: failed to read string size\n";
                return;
            }

            const size_t buffersize = 1024;
            char buffer[buffersize];
            string.clear();
            size_t bytes_remaining = size;

            while ( bytes_remaining > 0 && reader.in.sgetc() != std::streambuf::traits_type::eof() ) {
                size_t bytes_to_read = std::min(bytes_remaining, buffersize);
                size_t bytesRead = reader.in.sgetn(buffer, bytes_to_read);
                string.append(buffer, buffer + bytesRead);

                if (bytesRead < bytes_to_read) {
                    reader.errors << "Error: expected " << size
                                  << " bytes in string but only found "
                                  << (string.size() + bytesRead) << "\n";
                    return;
                }

                bytes_remaining -= bytesRead;
            }
        }



        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_signed_v<T>>
            visit(BinaryReader& reader, T& value) {
            uint64_t v;

            if (!read_unsigned_int(reader.in, v)) {
                reader.errors << "Error: failed to read number\n";
                return;
            }

            value = static_cast<T>(v);
        }

        inline void visit(BinaryReader& reader, bool& value) {
            uint64_t v;

            if ( !read_unsigned_int( reader.in, v ) ) {
                reader.errors << "Error: failed read bool\n";
                return;
            }

            value = static_cast< bool >( v );
        }

        template<typename T>
        inline void visit(BinaryReader& reader, std::vector<T>& vector) {
            uint64_t i = 0, size = 0;

            if (!read_unsigned_int(reader.in, size)) {
                reader.errors << "Error: failed to read vector size\n";
                return;
            }

            vector.clear();

            for (; i < size && reader.in.sgetc() != std::streambuf::traits_type::eof(); ++i) {
                vector.emplace_back();
                visit(reader, vector.back());
            }

            if (i != size) {
                reader.errors << "Error: expected " << size
                              << " elements in vector but only found "
                              << i << "\n";
            }
        }
    }//namespace serialize
}//namespace img_traverse
