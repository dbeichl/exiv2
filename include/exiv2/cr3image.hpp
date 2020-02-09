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
#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Class to access raw Canon CR3 images.  Exif metadata
          is supported directly, IPTC is read from the Exif data, if present.
     */
    class EXIV2API Cr3Image : public Image {
    private:
        int FindBox(byte *data, uint32_t size, uint32_t t, uint8_t *u=0);
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing CR3 image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten. Since the constructor can not return
              a result, callers should check the good() method after object
              construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        Cr3Image(BasicIo::UniquePtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;
        /*!
          @brief Print out the structure of image file.
          @throw Error if reading of the file fails or the image data is
                not valid (does not look like data of the specific image type).
          @warning This function is not thread safe and intended for exiv2 -pS for debugging.
         */
        void printStructure(std::ostream& out, PrintStructureOption option,int depth) override;
        /*!
          @brief Not supported. CR3 format does not contain a comment.
              Calling this function will throw an Error(kerInvalidSettingForImage).
         */
        void setComment(const std::string& comment) override;
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const override;
        int pixelWidth() const override;
        int pixelHeight() const override;
        //@}

        Cr3Image& operator=(const Cr3Image& rhs) = delete;
        Cr3Image& operator=(const Cr3Image&& rhs) = delete;
        Cr3Image(const Cr3Image& rhs) = delete;
        Cr3Image(const Cr3Image&& rhs) = delete;
    }; // class Cr3Image

    /*!
      @brief Stateless parser class for data in CR3 format. Images use this
             class to decode CR3 data.
             See class TiffParser for details.
     */
    class EXIV2API Cr3Parser {
    public:
        /*!
          @brief Decode metadata from a buffer \em pData of length \em size
                 with data in CR3 format to the provided metadata containers.
                 See TiffParser::decode().
        */
        static ByteOrder decode(
                  ExifData& exifData,
                  IptcData& iptcData,
                  XmpData&  xmpData,
            const byte*     pData,
                  uint32_t  size
        );
    }; // class Cr3Parser

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new Cr3Image instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::UniquePtr newCr3Instance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a CR3 image.
    EXIV2API bool isCr3Type(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2
