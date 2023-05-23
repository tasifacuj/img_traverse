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


        struct BinarySerialize {
            std::streambuf& out;
            BinarySerialize(std::streambuf& out_): out(out_) {}
        };

        // https://developers.google.com/protocol-buffers/docs/encoding?hl=en#signed-integers
        inline void write_unsigned_int(std::streambuf& out, uint64_t value) {
            do {
                uint8_t c = value & 0x7f;
                value >>= 7;
                if (value) c |= 0x80;
                out.sputc(c);
            } while(value);
        }


        inline void visit(BinarySerialize& writer, const std::string& string) {
            uint64_t size = string.size();
            write_unsigned_int(writer.out, size);
            writer.out.sputn(&string[0], size);
        }


        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_signed_v<T>>
            visit(BinarySerialize& writer, const T& value) {
            uint64_t wide_value = uint64_t(value);
            write_unsigned_int(writer.out, wide_value);
        }


        inline void visit(BinarySerialize& writer, bool value) {
            uint64_t wide_value = uint64_t(value);
            write_unsigned_int(writer.out, wide_value);
        }

        template<typename T>
        inline void visit(BinarySerialize& writer, std::vector< T > const& vector) {
            uint64_t size = vector.size();
            write_unsigned_int(writer.out, size);

            for (auto const& element : vector) {
                visit(writer, element);
            }
        }

    //-------------------------------------- deserialize ----------------------------------------------------//
        struct BinaryDeserialize {
            std::streambuf& in;
            std::stringstream errors;
            BinaryDeserialize(std::streambuf& buf): in(buf) {}

            std::string Errors() { return errors.str(); }
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

        inline void visit(BinaryDeserialize& reader, std::string& string) {
            uint64_t size = 0;
            if (!read_unsigned_int(reader.in, size)) {
                reader.errors << "Error: not enough data in buffer to read string size\n";
                return;
            }

            const size_t buffersize = 1024;
            char buffer[buffersize];
            string.clear();
            size_t bytes_remaining = size;

            while ( bytes_remaining > 0 && reader.in.sgetc() != std::streambuf::traits_type::eof() ) {
                size_t bytes_to_read = std::min(bytes_remaining, buffersize);
                size_t bytes_actually_read = reader.in.sgetn(buffer, bytes_to_read);
                string.append(buffer, buffer + bytes_actually_read);

                if (bytes_actually_read < bytes_to_read) {
                    reader.errors << "Error: expected " << size
                                  << " bytes in string but only found "
                                  << (string.size() + bytes_actually_read) << "\n";
                    return;
                }

                bytes_remaining -= bytes_actually_read;
            }
        }



        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_signed_v<T>>
            visit(BinaryDeserialize& reader, T& value) {
            uint64_t v;

            if (!read_unsigned_int(reader.in, v)) {
                reader.errors << "Error: not enough data in buffer to read number\n";
                return;
            }

            value = static_cast<T>(v);
        }

        inline void visit(BinaryDeserialize& reader, bool& value) {
            uint64_t v;

            if ( !read_unsigned_int( reader.in, v ) ) {
                reader.errors << "Error: not enough data in buffer to read bool\n";
                return;
            }

            value = static_cast< bool >( v );
        }

        template<typename T>
        inline void visit(BinaryDeserialize& reader, std::vector<T>& vector) {
            uint64_t i = 0, size = 0;

            if (!read_unsigned_int(reader.in, size)) {
                reader.errors << "Error: not enough data in buffer to read vector size\n";
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
