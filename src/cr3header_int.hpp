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
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /// @brief Canon CR3 header structure.
    class Cr3Header : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        explicit Cr3Header(ByteOrder byteOrder = littleEndian);
        //! Destructor.
        ~Cr3Header() override;
        //@}

        //! @name Manipulators
        //@{
        bool read(const byte* pData, size_t size) override;
        //@}

        //! @name Accessors
        //@{
        DataBuf write() const override;
        bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const override;
        //@}

    private:
        // DATA
        static const char*    cr3sig_;    //!< Signature for CR3 type TIFF
    }; // class Cr3Header

}}                                      // namespace Internal, Exiv2
