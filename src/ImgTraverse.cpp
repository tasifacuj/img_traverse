#include "ImgTraverse.hpp"
#include "Serialize.hpp"

#include <QDebug>

#include <algorithm>
#include <cassert>
#include <fstream>

namespace img_traverse{

PackedData ImageHelper::pack(RawImageData const& data, std::string const& ext ){
    PackedData pd;
    pd.ext = ext;
    pd.width = data.width;
    pd.height = data.height;

    for( unsigned row = 0; row < data.height; row ++ ){
        ByteArray packedRow;
        Byte empty = 1;

        for( unsigned col = 0; col < data.width; col += STEP ){
            int offset = row * data.width + col ;
            int dist = std::min( STEP, int(data.width - col) );

            if( std::all_of( data.data + offset , data.data + offset + dist , []( Byte b ){ return b == FULL_WHITE; }  )  ){
                packedRow.push_back( PACKED_WHITE );
            }else if( std::all_of( data.data + offset , data.data + offset + dist, []( Byte b ){ return b == FULL_BLACK; }  ) ){
                packedRow.push_back( PACKED_BLACK );
                empty = 0;
            }else{
                packedRow.push_back( PACKED_OTHER );
                packedRow.insert( packedRow.end(), data.data + offset, data.data + offset + dist );
                empty = 0;
            }
        }

        pd.emptyRowIndices.push_back( empty );

        if( !empty )
            pd.rows.push_back( packedRow );
    }

    return pd;
}

bool ImageHelper::unpack( RawImageData& data, PackedData const& pd ){
    data.width = pd.width;
    data.height = pd.height;
    int packedRowId = 0;
    data.data = new Byte[ pd.width * pd.height ];

    for( int row = 0; row < int(pd.height); row ++ ){
        int offset = row * data.width;
        int x = 0;

        if( pd.emptyRowIndices.at( row ) > 0 ){
            std::fill( data.data + offset, data.data + offset + data.width, FULL_WHITE );
            continue;
        }

        ByteArray const& packedRow = pd.rows.at( packedRowId );

        for( int packedcolId = 0; packedcolId < int(packedRow.size()); ){
            int fillDistance = std::min( STEP, int(data.width) - x );

            switch ( packedRow[ packedcolId ] ) {
            case PACKED_WHITE:
                std::fill( data.data + offset + x , data.data + offset + x + fillDistance , FULL_WHITE );
                x += fillDistance;
                packedcolId ++;
                break;
            case PACKED_BLACK:
                std::fill( data.data + offset + x , data.data + offset + x + fillDistance , FULL_BLACK );
                x += fillDistance;
                packedcolId ++;
                break;
            case PACKED_OTHER:{
                int srcDist = std::min( STEP, int( packedRow.size() ) - packedcolId - 1 );
                std::copy( packedRow.begin() + packedcolId + 1, packedRow.begin() +  packedcolId + 1 + srcDist , data.data + offset + x );
                x += srcDist;
                packedcolId += STEP + 1; // 0b11 + 4 bytes of payload
            }
                break;
            default:
                return false;
            }
        }

        packedRowId ++;
    }

    return true;
}

template<typename Visitor>
void visit( Visitor& visitor, PackedData& pd) {
    serialize::makeVisitor("PackedData", visitor)
        .field(pd.ext)
        .field(pd.width)
        .field( pd.height)
        .field(pd.emptyRowIndices )
        .field( pd.rows )
   ;
}

template<typename Visitor>
void visit( Visitor& visitor, PackedData const& pd) {
    serialize::makeVisitor("PackedData", visitor)
        .field(pd.ext)
        .field( pd.width)
        .field(pd.height)
        .field( pd.emptyRowIndices )
        .field( pd.rows )
        ;
}


void Serializer::write( PackedData const& data, std::string const& filename ){
    std::stringbuf buf;
    serialize::BinarySerialize writer( buf );
    visit( writer, data );
    std::ofstream oustrm( filename.c_str(), std::ios_base::binary );
    oustrm << buf.str();
}

bool Serializer::read( PackedData& data, std::string const& filename ){
    std::ifstream instrm( filename.c_str(), std::ios_base::binary );
    std::stringstream strm;
    strm << instrm.rdbuf();
    serialize::BinaryDeserialize reader( *strm.rdbuf() );
    visit( reader, data);

    const std::string& errs = reader.Errors();

    if(errs.size() > 0){
        qDebug() << "READ ERR:" << errs.c_str();
        return false;
    }

    return true;
}

}// namespace img_traverse
