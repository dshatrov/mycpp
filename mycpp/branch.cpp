#include <mycpp/mycpp_thread_local.h>
#include <mycpp/util.h>

#include <mycpp/branch.h>

namespace MyCpp {

// TODO FIXME Объяснить логику выбора append/prepend.

static List< List< List< Ref<SimplyReferenced> > > >&
getBranchLayers ()
{
    return myCpp_getThreadLocal ()->branch_layers;
}

static List< List< Ref<SimplyReferenced> > >&
getTopmostSavingGroup (List< List< List< Ref<SimplyReferenced> > > > &branch_layers)
{
    if (branch_layers.first == NULL)
	branch_layers.appendEmpty ();

    return branch_layers.first->data;
}

BranchLayerKey
beginNewBranch (SimplyReferenced *branch)
{
    if (branch == NULL)
	return NULL;

    List< Ref<SimplyReferenced> > &target_layer = getBranchLayers ().appendEmpty ()->data.appendEmpty ()->data;
    target_layer.append (branch);
    return &target_layer;
}

BranchLayerKey
mergeNewBranch (SimplyReferenced *branch)
{
    if (branch == NULL)
	return NULL;

    List< Ref<SimplyReferenced> > &layer = getTopmostSavingGroup (getBranchLayers ()).appendEmpty ()->data;
    layer.append (branch);
    return &layer;
}

Ref<BranchLayer>
saveCurrentBranchLayer ()
{
    List< List< List< Ref<SimplyReferenced> > > > &branch_layers = getBranchLayers ();
    if (branch_layers.first == NULL)
	return NULL;

    Ref<BranchLayer> branch_layer = grab (new BranchLayer);

    List< List< Ref<SimplyReferenced> > >::DataIterator iter (branch_layers.first->data);
    while (!iter.done ()) {
	List< Ref<SimplyReferenced> >::DataIterator sub_iter (iter.next ());
	while (!sub_iter.done ())
	    branch_layer->branches.append (sub_iter.next ());
    }

    return branch_layer;
}

BranchLayerKey
pushBranchLayer (BranchLayer *layer)
{
    if (layer == NULL)
	return NULL;

    List< List< List< Ref<SimplyReferenced> > > > &branch_layers = getBranchLayers ();

    branch_layers.prependEmpty ();
    List< Ref<SimplyReferenced> > &target_layer = branch_layers.first->data.appendEmpty ()->data;

    List< Ref<SimplyReferenced> >::DataIterator iter (layer->branches);
    while (!iter.done ())
	target_layer.append (iter.next ());

    return &target_layer;
}

BranchLayerKey
mergeBranchLayer (BranchLayer *layer)
{
    if (layer == NULL)
	return NULL;

    List< List< Ref<SimplyReferenced> > > &saving_group = getTopmostSavingGroup (getBranchLayers ());
    List< Ref<SimplyReferenced> > &target_list = saving_group.prependEmpty ()->data;

    List< Ref<SimplyReferenced> >::DataIterator iter (layer->branches);
    while (!iter.done ())
	target_list.append (iter.next ());

    return &target_list;
}

void
endTopmostBranchLayer (const BranchLayerKey &key)
{
    if (key.branch_layer_ptr == NULL)
	return;

    List< List< List< Ref<SimplyReferenced> > > > &branch_layers = getBranchLayers ();
    if (branch_layers.first == NULL ||
	branch_layers.first->data.first == NULL)
    {
	abortIfReached ();
    }

    if (&branch_layers.first->data.first->data != key.branch_layer_ptr)
	// TODO errf->print (...)
	abortIfReached ();

    branch_layers.first->data.remove (branch_layers.first->data.first);
    if (branch_layers.first->data.first == NULL)
	branch_layers.remove (branch_layers.first);
}

}

