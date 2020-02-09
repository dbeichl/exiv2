// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2020 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      cr3image.cpp
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "cr3image.hpp"
#include "tiffimage.hpp"
#include "cr3header_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "tiffimage_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    Cr3Image::Cr3Image(BasicIo::UniquePtr io, bool /*create*/)
        : Image(ImageType::cr3, mdExif | mdIptc | mdXmp, std::move(io))
    {
    } // Cr3Image::Cr3Image

    std::string Cr3Image::mimeType() const
    {
        return "image/x-canon-cr3";
    }

    int Cr3Image::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int Cr3Image::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void Cr3Image::printStructure(std::ostream& out, Exiv2::PrintStructureOption option,int depth)
    {
        if (io_->open() != 0) throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        io_->seek(0,BasicIo::beg);
        printTiffStructure(io(),out,option,depth-1);
    }

    void Cr3Image::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(kerInvalidSettingForImage, "Image comment", "CR3"));
    }

    void Cr3Image::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Reading CR3 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isCr3Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "CR3");
        }
        clearMetadata();
        ByteOrder bo = Cr3Parser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         (uint32_t) io_->size());
        setByteOrder(bo);
    } // Cr3Image::readMetadata

    void Cr3Image::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(kerWritingImageFormatUnsupported, "CR3"));
    } // Cr3Image::writeMetadata

    int FindAtom(const byte *data, uint32_t size, uint32_t t, uint8_t *u=0)
    {
        uint32_t nextatom = 0;
        uint32_t offs;
        uint32_t type;
        uint64_t length;
        const byte *uuid;

        while (nextatom < size)
        {
            length = getULong(data+nextatom, ByteOrder());
            type = getULong(data+nextatom+4, ByteOrder());
            offs = nextatom + 8;
            uuid = 0;

            if (length == 1) {
                length = getULongLong(data+offs, ByteOrder());
                offs += 8;
            }

            if (type == 0x75756964) { // uuid
                uuid = data + offs;
                offs += 16;
            }

            //fprintf(stderr, "Found Atom %08x at %d len %lu\n", type, offs, length);

            if ((type == t) || (u && uuid && !memcmp(u, uuid, 16))) {
                if (offs < size)
                    return offs;

                return -1;
            }

            if (!u && (type == 0x75756964 || type == 0x6d6f6f76)) { // uuid or moov
                if ((offs + length) <= size)
                    return FindAtom(data+offs, length-offs, t, u) + offs;
            }
            nextatom += length;
        }
        return -1;
    }

    ByteOrder Cr3Parser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        Cr3Header cr3Header;
        uint8_t thumbUuid[16] = {0xea,0xf4,0x2b,0x5e,0x1c,0x98,0x4b,0x88,0xb9,0xfb,0xb7,0xdc,0x40,0x6e,0x4d,0x16};
        int thmbOffs = 0;
        int thmbSize = 0;
        int cmtOffs;
        int validDir = 0;
        int tag[4] = { Tag::root, Tag::exif, Tag::canon, Tag::gps };
        TiffComponent::UniquePtr dir[4];

        for (int i=0;i<4;i++) {
            cmtOffs = FindAtom(pData, size, 0x434d5431+i); // CMT1-4
            if (cmtOffs > 0) {
                cr3Header.setFileOffset(cmtOffs);
                cr3Header.setOffset(cmtOffs+8);
                dir[validDir++] = TiffParserWorker::parse(pData, size, tag[i], &cr3Header);
            }
        }

        if (!validDir)
            return ByteOrder();

        for (int i=validDir-1;i>0;i--)
            dir[i-1]->addNext(std::move(dir[i]));

        TiffDecoder decoder(exifData,
                            iptcData,
                            xmpData,
                            dir[0].get(),
                            TiffMapping::findDecoder);

        dir[0]->accept(decoder);

        thmbOffs = FindAtom(pData, size, -1, thumbUuid);
        if (thmbOffs > 0) {
            thmbOffs += 32;
            thmbSize = getULong(pData+thmbOffs-4, ByteOrder());
            ExifThumb exifThumb(exifData);
            exifThumb.setJpegThumbnail(pData+thmbOffs, thmbSize);
        }

        return ByteOrder();
    }

    // *************************************************************************
    // free functions
    Image::UniquePtr newCr3Instance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new Cr3Image(std::move(io), create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isCr3Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 16;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        Cr3Header header;
        bool rc = header.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc;
    }

}                                       // namespace Exiv2
