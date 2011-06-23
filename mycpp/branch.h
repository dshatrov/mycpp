#ifndef __MYCPP__BRANCH_H__
#define __MYCPP__BRANCH_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/list.h>


/*
 * См. docs/branch_layer.txt
 */


namespace MyCpp {

class BranchLayer : public virtual SimplyReferenced
{
public:
    // This is an opaque data field. One should not manipulate it.
    List< Ref<SimplyReferenced> > branches;
};

class BranchLayerKey
{
public:
    // This is an opaque data field. One should not manipulate it.
    List< Ref<SimplyReferenced> > *branch_layer_ptr;

    BranchLayerKey (List< Ref<SimplyReferenced> > *branch_layer)
	: branch_layer_ptr (branch_layer)
    {
    }
};

BranchLayerKey beginNewBranch (SimplyReferenced *branch);

BranchLayerKey mergeNewBranch (SimplyReferenced *branch);

Ref<BranchLayer> saveCurrentBranchLayer ();

BranchLayerKey pushBranchLayer (BranchLayer *layer);

BranchLayerKey mergeBranchLayer (BranchLayer *layer);

void endTopmostBranchLayer (const BranchLayerKey &key);

class BranchHolder
{
protected:
    BranchLayerKey key;

public:
    BranchHolder (SimplyReferenced *branch)
	: key (beginNewBranch (branch))
    {
    }

    ~BranchHolder ()
    {
	endTopmostBranchLayer (key);
    }
};

class BranchMerger
{
protected:
    BranchLayerKey key;

public:
    BranchMerger (SimplyReferenced *branch)
	: key (mergeNewBranch (branch))
    {
    }

    ~BranchMerger ()
    {
	endTopmostBranchLayer (key);
    }
};

class BranchLayerHolder
{
protected:
    BranchLayerKey key;

public:
    BranchLayerHolder (BranchLayer *branch_layer)
	: key (pushBranchLayer (branch_layer))
    {
    }

    ~BranchLayerHolder ()
    {
	endTopmostBranchLayer (key);
    }
};

class BranchLayerMerger
{
protected:
    BranchLayerKey key;

public:
    BranchLayerMerger (BranchLayer *branch_layer)
	: key (mergeBranchLayer (branch_layer))
    {
    }

    ~BranchLayerMerger ()
    {
	endTopmostBranchLayer (key);
    }
};

}

#endif /* __MYCPP__BRANCH_H__ */

