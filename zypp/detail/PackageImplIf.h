/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PACKAGEIMPLIF_H
#define ZYPP_DETAIL_PACKAGEIMPLIF_H

#include <set>

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/CheckSum.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/Changelog.h"
#include "zypp/DiskUsage.h"
#include "zypp/PackageKeyword.h"
#include "zypp/repo/PackageDelta.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Package;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageImplIf
    //
    /** Abstract Package implementation interface.
    */
    class PackageImplIf : public ResObjectImplIf
    {
    public:
      typedef Package ResType;

    public:
      typedef packagedelta::DeltaRpm    DeltaRpm;
      typedef packagedelta::PatchRpm    PatchRpm;
      typedef std::set<PackageKeyword>  Keywords;

    public:
      /** Overloaded ResObjectImpl attribute.
       * \return The \ref location media number.
       */
      virtual unsigned mediaNr() const;

      /** Overloaded ResObjectImpl attribute.
       * \return The \ref location downloadSize.
       */
      virtual ByteCount downloadSize() const;

    public:
      /** \name Rpm Package Attributes. */
      //@{

      virtual std::string buildhost() const PURE_VIRTUAL;
      /** */
      virtual std::string distribution() const PURE_VIRTUAL;
      /** */
      virtual Label license() const PURE_VIRTUAL;
      /** */
      virtual std::string packager() const PURE_VIRTUAL;
      /** */
      virtual PackageGroup group() const PURE_VIRTUAL;
      /** */
      virtual Keywords keywords() const PURE_VIRTUAL;
      /** */
      virtual Changelog changelog() const PURE_VIRTUAL;
      /** */
      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      virtual std::string url() const PURE_VIRTUAL;
      /** */
      virtual std::string os() const PURE_VIRTUAL;
      /** */
      virtual Text prein() const PURE_VIRTUAL;
      /** */
      virtual Text postin() const PURE_VIRTUAL;
      /** */
      virtual Text preun() const PURE_VIRTUAL;
      /** */
      virtual Text postun() const PURE_VIRTUAL;
      /** */
      virtual ByteCount sourcesize() const PURE_VIRTUAL;
      /** */
      virtual std::list<std::string> authors() const PURE_VIRTUAL;
      /** */
      virtual std::list<std::string> filenames() const PURE_VIRTUAL;
      /** */
      virtual std::list<DeltaRpm> deltaRpms() const PURE_VIRTUAL;
      /** */
      virtual std::list<PatchRpm> patchRpms() const PURE_VIRTUAL;

      virtual OnMediaLocation location() const PURE_VIRTUAL;

      /** Name of the source rpm this package was built from.
       * Empty if unknown.
       */
      virtual std::string sourcePkgName() const PURE_VIRTUAL;

      /** Edition of the source rpm this package was built from.
       * Empty if unknown.
       */
      virtual Edition sourcePkgEdition() const PURE_VIRTUAL;

      //@}

    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPLIF_H