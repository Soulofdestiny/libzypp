/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverQueue.h
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERQUEUE_H
#define ZYPP_SOLVER_DETAIL_RESOLVERQUEUE_H

#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/QueueItem.h"

#include "zypp/Capability.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef std::list <ResolverQueue_Ptr> ResolverQueueList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverQueue

class ResolverQueue : public base::ReferenceCounted, private base::NonCopyable {

  private:

    ResolverContext_Ptr _context;
    QueueItemList _qitems;

    ResolverQueue_Ptr copy_queue_except_for_branch (QueueItem_Ptr branch_qitem, QueueItem_Ptr subqitem) const;

  public:
    ResolverQueue (const ResPool & pool, const Arch & arch, ResolverContext_Ptr context = NULL);
    virtual ~ResolverQueue();

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ResolverQueue & context);

    // ---------------------------------- accessors

    ResolverContext_Ptr context (void) const { return _context; }
    QueueItemList qitems(void) const { return _qitems; }

    // ---------------------------------- methods

    void addPoolItemToInstall (PoolItem_Ref poolItem);
    void addPoolItemToEstablish (PoolItem_Ref poolItem);
    void addPoolItemToRemove (PoolItem_Ref poolItem, bool remove_only_mode);
    void addPoolItemToVerify (PoolItem_Ref poolItem);
    void addExtraCapability (const Capability & cap);
    void addExtraConflict (const Capability & cap);
    void addItem (QueueItem_Ptr qtem);

    bool isEmpty () const { return _qitems.empty(); }
    bool isInvalid ();
    bool containsOnlyBranches ();

    bool processOnce ();
    void process ();

    void splitFirstBranch (ResolverQueueList & new_queues, ResolverQueueList & deferred_queues);

    void spew ();

};
///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_DETAIL_RESOLVERQUEUE_H
