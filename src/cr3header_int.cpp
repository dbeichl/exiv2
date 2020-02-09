#include "cr3header_int.hpp"

namespace Exiv2 {
    namespace Internal {

    const char* Cr3Header::cr3sig_ = "ftypcrx ";

    Cr3Header::Cr3Header(ByteOrder byteOrder)
        : TiffHeaderBase(42, 8, byteOrder, 0, 0)
    {
    }

    Cr3Header::~Cr3Header()
    {
    }

    bool Cr3Header::read(const byte* pData, size_t size)
    {
        if (!pData || size < 16) {
            return false;
        }

        if (0 != memcmp(pData + 4, cr3sig_, 8)) return false;

        return true;
    } // Cr3Header::read

    DataBuf Cr3Header::write() const
    {
        DataBuf buf(0);
        return buf;
    } // Cr3Header::write

    bool Cr3Header::isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* /*pPrimaryGroups*/) const
    {
        // CR3 image tags are all IFD2 and IFD3 tags
        if (group == ifd2Id || group == ifd3Id) return true;
        // ...and any (IFD0) tag that is in the TIFF image tags list
        return isTiffImageTag(tag, group);
    }

}}                                      // namespace Internal, Exiv2
