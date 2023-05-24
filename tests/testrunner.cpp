#include "ImgTraverse.hpp"
#include "Serialize.hpp"

#include <cassert>

#include <iostream>

using namespace img_traverse;

static void test_pack_unpack(){
    unsigned char rawData[ 36 ] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01
    };
    RawImageData img{ 12, 3, rawData };
    PackedData pd = ImageHelper::pack( img, "png" );
    std::vector<Byte> ie{ 1, 0,0};
    assert( pd.emptyRowIndices == ie );
    std::vector< ByteArray > expectedPackedRows{
        { 0b10, 0b10, 0b10 },
        { 0b0, 0b10, 0b11, 0b01, 0b01, 0b01, 0b01 }
    };

    assert( pd.rows == expectedPackedRows );

    RawImageData img2{ 12, 3, nullptr };
    ImageHelper::unpack( img2, pd );
    ByteArray b1( img.data, img.data + 36 );
    ByteArray b2( img2.data, img2.data + 36 );
    assert( b1 == b2 );
}

static bool operator == ( PackedData const& lhs, PackedData const& rhs ){
    return lhs.ext == rhs.ext
        && lhs.width == rhs.width
        && lhs.height == rhs.height
        && lhs.rows == rhs.rows
        ;
}

static void test_serialize(){
    std::vector<Byte> rawData{
        0x0,  0x0,  0x00,  0xFF,  0x00,  0x0,  0x0,  0x01,
        0x0,  0x0,  0x00,  0xFF,  0x00,  0x00, 0x0,  0x0,
        0x0,  0xFF, 0xFF,  0xFF,  0xFF,  0xFF, 0xFF, 0x0,
        0x0,  0x0,  0x00,  0xFF,  0x00,  0x00, 0x0,  0x0,
        0x0,  0x0,  0x00,  0xFF,  0x00,  0x00, 0x0A,  0x0,
    };

    RawImageData img{ 8, 5, &rawData[0] };
    PackedData pd = ImageHelper::pack( img, "png" );
    Serializer::write( pd, "test.barch" );
    PackedData pd2;
    Serializer::read( pd2, "test.barch" );
    assert( pd == pd2 );
    RawImageData restoredImage{ 8, 5, nullptr };
    ImageHelper::unpack( restoredImage, pd2 );

    ByteArray b2( restoredImage.data, restoredImage.data + 40 );
    assert( rawData == b2 );

    std::cout << "Original data:\n";
    for( int row = 0; row < img.height; row ++ ){
        for( int col = 0; col < img.width; col ++ ){
            std::cout << std::hex << int(img.data[ row * img.width + col ]) << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Restored data:\n";
    for( int row = 0; row < restoredImage.height; row ++ ){
        for( int col = 0; col < restoredImage.width; col ++ ){
            std::cout << std::hex << int( restoredImage.data[ row * restoredImage.width + col ] ) << " ";
        }
        std::cout << std::endl;
    }
}

static void test_serialize2(){
    PackedData pd2;
    Serializer::read( pd2, "test.barch" );
    RawImageData restoredImage{ 8, 5, nullptr };
    ImageHelper::unpack( restoredImage, pd2 );


    std::cout << "\nOnly read data:\n";
    for( int row = 0; row < restoredImage.height; row ++ ){
        for( int col = 0; col < restoredImage.width; col ++ ){
            std::cout << std::hex << int( restoredImage.data[ row * restoredImage.width + col ] ) << " ";
        }
        std::cout << std::endl;
    }
}


int main(){
//    test_pack_unpack();
    test_serialize();
    test_serialize2();
    return 0;
}
