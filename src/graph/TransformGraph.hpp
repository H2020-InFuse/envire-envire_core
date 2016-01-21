/**\file TransformTree.hpp
 *
 * This class provided stores and manages the transformation graph
 *
 * @author Javier Hidalgo Carrio et. al (see THANKS  for the full Author list)
 * See LICENSE for the license information
 */
#pragma once

#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <type_traits>


#include "TransformGraphTypes.hpp"
#include "TransformGraphExceptions.hpp"
#include "TransformGraphVisitors.hpp"
#include "TreeView.hpp"

#include <envire_core/events/GraphEventPublisher.hpp>
#define BOOST_RESULT_OF_USE_DECLTYPE //this is important for the transform_iterator
#include <boost/iterator/transform_iterator.hpp>
#include <envire_core/util/MetaProgramming.hpp>
#include <envire_core/events/ItemAddedEvent.hpp>
#include <envire_core/events/ItemRemovedEvent.hpp>
#include <envire_core/serialization/BoostTypes.hpp>



namespace envire { namespace core
{
    /**
     * A tree-like graph structure.
     * Each vertex contains a labeled Frame. The label must be unique.
     *
     * @warning Do **not** manipulate the graph directly using boost functions.
     *          If you do that you will **break** the event system.
     *          I.e. others will not be notified about the changes to the graph.
     *          Instead use the methods provided by this class for manipulation.
     * 
     * @see test/transform_graph_test.cpp for usage examples.     
     *
     * Methods Naming convention
     * Overloading boost methods uses delimited separated
     * words, new methods use Camel Case separated words
     *    
    */
    class TransformGraph : public LabeledTransformGraph, public GraphEventPublisher,
                           public TreeUpdatePublisher
    {
    public:
        /**Transform iterator used to down cast from ItemBase to @p T while
         * iterating */
        template<class T>
        using ItemIterator = boost::transform_iterator<ItemBaseCaster<typename T::element_type>, std::vector<ItemBase::Ptr>::const_iterator, T>;

        TransformGraph(envire::core::Environment const &environment = Environment());
        
        
        /** Adds an unconnected frame to the graph.
         * 
         * @throw FrameAlreadyExistsException if the frame already exists
         */
        void addFrame(const FrameId& frame);

        /**Adds a transform from frame @p origin to frame @p target.
         * If the frames do not exist, they will be created.
         * If the transform already exists, it will **not** be updated.
         *
         * Causes a TransformAddedEvent.
         * Causes FrameAddedEvents if the frames did not exist before. Those events
         * will occur **before** the TransformAddedEvent.
         *
         * The inverse transform will be added automatically.
         *
         * @throw TransformAlreadyExistsException if the transformation already exists.*/
        void addTransform(const FrameId& origin, const FrameId& target,
                          const Transform& tf);
        void addTransform(const vertex_descriptor origin, const vertex_descriptor target,
                          const Transform& tf);

        /**Updates the value of the transform from @p origin to
         * @p target and the inverse transformation according to @p tf.
         * Both frames need to exist beforehand.
         * A direct transformation has to exist between @p orign and @p target
         * for this method to work.
         *
         * Causes a TransformUpdated event.
         *
         * @throw UnknownTransformException if no direct transformation between
         *                                  @p origin and @p target exists.
         * @throw UnknownFrameException if @p origin or @p target do not exist.
         */
        void updateTransform(const FrameId& origin, const FrameId& target,
                             const Transform& tf);
        void updateTransform(const vertex_descriptor origin,
                             const vertex_descriptor target, const Transform& tf);

        /**Removes the transform between @p origin and @p target as well as
         * the transform between @p target and @p origin.
         *
         * Causes TransformRemoved event.
         *
         * @throw UnknownTransformException if the transformation doesn't exist */
        void removeTransform(const FrameId& origin, const FrameId& target);

        /** @return the transform between a and b. Calculating it if necessary.
         * @throw UnknownTransformException if the transformation doesn't exist
         * @throw UnknownFrameException if the @p origin or @p target does not exist*/
        const Transform getTransform(const FrameId& origin, const FrameId& target) const;
        const Transform getTransform(const vertex_descriptor origin, const vertex_descriptor target) const;

         /** @return the transform between a and b. Calculating it if necessary.
         * @throw UnknownTransformException if the transformation doesn't exist
         * @throw UnknownFrameException if the @p origin or @p target does not exist*/
        const Transform getTransform(const FrameId& origin, const FrameId& target, const TreeView &view) const;
        const Transform getTransform(const vertex_descriptor origin, const vertex_descriptor target, const TreeView &view) const;
        /** @return the transform between source(edge) and target(edge) */
        const Transform getTransform(const edge_descriptor edge) const;

        /** @return the edge between frame @p origin and @p target
         *  @throw UnknownFrameException If @p orign or @p target do not exist.
         *  @throw UnknownTransformException if there is no such edge  */
        edge_descriptor getEdge(const FrameId& origin, const FrameId& target) const;
        edge_descriptor getEdge(const vertex_descriptor origin, const vertex_descriptor target) const;

        /** @return a reference to the frame identified by the id.
         *  @throw UnknownFrameException if the frame id is invalid **/
        const envire::core::Frame& getFrame(const FrameId& frame) const;
        const envire::core::Frame& getFrame(const vertex_descriptor desc) const;

        /** @return the frame id of the specified @p vertex */
        const envire::core::FrameId& getFrameId(const vertex_descriptor vertex) const;

        /**Removes @p item from @p frame.
         *  Causes ItemRemovedEvent.
         * @throw UnknownFrameException if the frame does not exist.
         * @throw UnknownItemException if the item is not part of the frame's
         *                             item list.
         * @param T should be ItemBase::PtrType<X> where X is derived from ItemBase
         * @note Invalidates all iterators of type ItemIterator<ItemBase::PtrType<T>> for the
         *       specified @p frame.
         * @return A pair of iterators. The first one points to the element that
         *         comes after @p item and the second one points to the end of the
         *         list.*/ 
        template <class T>
        std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>>
        removeItemFromFrame(const FrameId& frameId, ItemIterator<T> item);
        
        /**Searches for @p item in @p frame and removes it if present.
         * Causes ItemRemovedEvent.
         * @throw UnknownFrameException if the @ frame does not exist.
         * @throw UnknownItemException if the @p frame does not contain the @p item.
         * @note Invalidates all iterators of type ItemIterator<ItemBase::PtrType<T>> for the
         *       specified @p frame.
         *       I.e. you cannot iterate over all items and use this method at the same time.
         * */
        template <class T>
        void removeItemFromFrame(const FrameId& frameId, ItemBase::PtrType<T> item);
        
        
        /**Disconnects @p frame from the Graph.
         * I.e. all transformations from and to @p frame will be removed.
         * @throw UnknownFrameException if the frame does not exist. */
        void disconnectFrame(const FrameId& frame);
        
        /**Removes @p frame from the Graph.
         * A frame can only be removed if there are no transforms connected to
         * or coming from that frame.
         * @throw UnknownFrameException if the frame does not exist.
         * @ŧhrow FrameStillConnectedException if there are still transforms
         *                                     coming from or leading to this
         *                                     frame. */
        void removeFrame(const FrameId& frame);
        
        /**Removes all items from @p frame.
         * Causes ItemRemovedEvent for each item that is removd.
         * @throw UnknownFrameException if the frame does not exist.    */
        void clearFrame(const FrameId& frame);
        
        /**Builds a TreeView containing all vertices that are accessible starting
         * from @p root.  */
        TreeView getTree(const vertex_descriptor root) const;
        
        /**Builds a TreeView containing all vertices that are accessible starting
         * from @p rootId.
         * @note The tree is ***not** updated when the Graph changes.
         * @throw UnknownFrameException if the frame does not exist */
        TreeView getTree(const FrameId rootId) const;
        void getTree(const FrameId rootId, TreeView* outView) const;
        void getTree(const vertex_descriptor root, TreeView* outView) const;
        
        /**Builds a TreeView containing all vertices that are accessible starting
         * from @p root and writes it to @p outView.
         * if @p keepTreeUpdated is true, the TransformGraph will retain a pointer
         * to @p outView and update it whenenver transformations are added or removed.
         * If the TreeView is destroyed it will automatically unsubscribe from
         * the graph. The view can also be unsubscribed manually by calling
         * unsubscribeTreeView()*/
        void getTree(const vertex_descriptor root, const bool keepTreeUpdated, TreeView* outView);
        void getTree(const FrameId rootId, const bool keepTreeUpdated, TreeView* outView);
        
        
        /**Returns the shortest path from @p origin to @p target.
         * Returns an empty vector if the path doesn't exist.
         * @throw UnknownFrameException if @p orign or @p target don't exist */
        Path getPath(FrameId origin, FrameId target) const;
        
        vertices_size_type num_vertices() const;
        edges_size_type num_edges() const;

         /** Adds @p item to the item list in the frame of item
         *  Causes ItemAddedEvent.
         *  @throw UnknownFrameException if the frame id is invalid
         *  @param item that should be added. The item provides the frame to which it will be added */
        void addItem(ItemBase::Ptr item);
        
         /** Adds @p item to the item list of the specified frame 
         *  Causes ItemAddedEvent.
         *  @throw UnknownFrameException if the frame id is invalid
         *  @param T type of the item that should be added. The item has to 
         *           be of type ItemBase::PtrType<U> where U derives from ItemBase */
        template<class T>
        void addItemToFrame(const FrameId& frame, T item);
        
        /**Returns all items of type @p T that are stored in @p frame.
         * @throw UnknownFrameException if the @p frame id is invalid.
         * @param T has to be of type ItemBase::PtrType<X> where X derives from ItemBase.
         * @return a pair iterators [begin, end]. If no items of type @p T
         *         exist, both iterators are equal and invalid */
        template<class T>
        const std::pair<ItemIterator<T>, ItemIterator<T>> getItems(const FrameId& frame) const;
        template<class T>
        const std::pair<ItemIterator<T>, ItemIterator<T>> getItems(const vertex_descriptor frame) const;
        /** @return a list of all items of @p tyoe in @p frame
         *  @throw NoItemsOfTypeInFrameException if no items of the type are in the frame*/
        const envire::core::Frame::ItemList& getItems(const vertex_descriptor frame,
                                                      const std::type_index& type) const;
        
        
        
        /**Convenience method that returns the @p i'th item of type @p T from @p frame.
         * @throw UnknownFrameException if the @p frame id is invalid.
         * @throw NoItemsOfTypeInFrameException if no items of type @p T exist in the frame.
         * @throw std::out_of_range if @p i is out of range*/
        template <class T>
        const T getItem(const FrameId& frame, const int i = 0) const;
        template <class T>
        const T getItem(const vertex_descriptor frame, const int i = 0) const;
        
        
        
        /** @return true if the @p frame contains at least one item of type @p T
         *  @throw UnknownFrameException if the @p frame id is invalid.*/
        template <class T>
        bool containsItems(const FrameId& frame) const;
        template <class T>
        bool containsItems(const vertex_descriptor frame) const;
        /** @return true if @p frame contains at least one item of @p type */
        bool containsItems(const vertex_descriptor frame, const std::type_index& type) const;
        
        /** @return the number of items of type @p T in @p frame.
         *  @throw UnknownFrameException if the @p frame id is invalid.*/
        template <class T>
        size_t getItemCount(const FrameId& frame) const;
        template <class T>
        size_t getItemCount(const vertex_descriptor vd) const;        

        /** @return the number of all items independent of their type in @p frame.
         *  @throw UnknownFrameException if the @p frame id is invalid.*/
        size_t getTotalItemCount(const FrameId& frame) const;
        size_t getTotalItemCount(const vertex_descriptor vd) const;
        
        const vertex_descriptor source(const edge_descriptor edge) const;
        const vertex_descriptor target(const edge_descriptor edge) const;
        
        /**Gets the vertex corresponding to @p frame.
         * @throw UnknownFrameException if the frame does not exist */
        vertex_descriptor getVertex(const FrameId& frame) const;
        
        /**Unsubscribe @p view from TreeView updates */
        virtual void unsubscribeTreeView(TreeView* view);
        
        /**Subscribe @p view to TreeView updates.
         * @note Make sure that the tree is not out of sync already when 
         *       subscribing*/
        virtual void subscribeTreeView(TreeView* view);
        
        virtual ~TransformGraph();
        
    protected:
        using EdgePair = std::pair<edge_descriptor, bool>;
      
         /**@brief Add a vertex
         * @note the frame's name must be unique. */
        vertex_descriptor add_vertex(const FrameId& frameId);

        /**@brief Add an Edge
         * Add an edge between two vertices.
         *
         * Causes TransformAddedEvent or TransformModifiedEvent.
         *
         * @return an edge descriptor pointing to the new edge and a boolean.
         *         The boolean is false if the edge already existed.
         *         In that case no new edge was added, instead the existing
         *         edge was updated.
         */
        edge_descriptor add_edge(const vertex_descriptor node_from,
                                 const vertex_descriptor node_to,
                                 const envire::core::Transform &tf,
                                 const FrameId& originName,
                                 const FrameId& targetName);
        

        /**Sets the transform value and causes transformModified event. */
        void updateTransform(edge_descriptor ed, const Transform& tf);
        
        /** @return A range that contains all items of type @p T in frame @p frame
         **/
        template<class T>
        const std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>> 
        getItemsInternal(const vertex_descriptor frame, const FrameId& frameId) const;
        
        /**@see addTransform(origin, target, tf) */
        void addTransform(const FrameId& origin, const FrameId& target,
                          vertex_descriptor originDesc, vertex_descriptor targetDesc,
                          const Transform& tf);
        
        /**Update all subscribed TreeViews with the new edge.
         * @note When adding a new transform the back-edge is added to the graph
         *       as well. I.e. for each transform two edges are added to the graph.
         *       However only one of the edges should be added to the tree,
         *       because the tree does not care about edge direction.
         *       It does not matter whether you add the back-edge or the edge
         *       to the tree as long as you do ***not*** add both.
         */
        void addEdgeToTreeViews(edge_descriptor newEdge) const;
        void addEdgeToTreeView(edge_descriptor newEdge, TreeView* view) const;
        
        /**Rebuild all subscribed TreeViews */
        void rebuildTreeViews() const;
        
        /**Returns true if an edge between a and b exists in @p view.
         * @note only call this method if you are sure that both a and b
         *       are part of the tree. Otherwise it will crash*/
        bool edgeExists(const vertex_descriptor a, const vertex_descriptor b,
                        const TreeView * view) const;
        
    private:
        /**Ensures that T is ItemBase::PtrType<X> where X derives from ItemBase  */
        template <class T>
        static void checkItemType();
        
        /** @throw UnknownFrameException if the frame does not exist */
        void checkFrameValid(const FrameId& frame) const;
        
        /**TreeViews that need to be updated when the graph is modified */
        std::vector<TreeView*> subscribedTreeViews;

        /**Grants access to boost serialization */
        friend class boost::serialization::access;

        /**Unserializes this class*/
        template <typename Archive>
        void load(Archive &ar, const unsigned int version)
        {
            ar >> boost::serialization::make_nvp("directed_graph", _graph);

            // regenerate mapping of the labeled graph
            boost::graph_traits<TransformGraphBase>::vertex_iterator it, end;
            for (boost::tie( it, end ) = boost::vertices(_graph); it != end; ++it)
            {
                _map[_graph[*it].frame.name] = *it;
            }
        }

        /**Serializes this class. Only the directed graph is serialized,
         * subscribers are excluded and the mapping of the labeled graph
         * is regenerated. */
        template <typename Archive>
        void save(Archive &ar, const unsigned int version) const
        {
            ar << boost::serialization::make_nvp("directed_graph", _graph);
        }

        /**Splits boost serialize into a load and save method */
        template <typename Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            boost::serialization::split_member(ar, *this, version);
        }

    };
    
    template <class T>
    void TransformGraph::checkItemType()
    {
        using ItemType = typename extract_value_type<T>::value_type;
        static_assert(std::is_same<T, ItemBase::PtrType<ItemType>>::value,
            "Item should be of type ItemBase::PtrType<T>");
        static_assert(std::is_base_of<ItemBase, ItemType>::value,
            "Item is of type ItemBase::PtrType<T> but T does not derive from ItemBase"); 
    }

    
    template <class T>
    void TransformGraph::addItemToFrame(const FrameId& frame, T item)
    {
        checkItemType<T>();
        checkFrameValid(frame);
        std::type_index i(typeid(T));
        (*this)[frame].frame.items[i].push_back(item);
        ItemBase::Ptr baseItem = boost::dynamic_pointer_cast<ItemBase>(item);
        notify(ItemAddedEvent(frame, baseItem, i));
    }
    
    template<class T>
    const std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>>
    TransformGraph::getItems(const FrameId& frame) const
    {
        checkItemType<T>();
        const vertex_descriptor desc = vertex(frame);
        if(desc == null_vertex())
        {
            throw UnknownFrameException(frame);
        }

        return getItemsInternal<T>(desc, frame);
    }
    
    template<class T>
    const std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>> 
    TransformGraph::getItems(const vertex_descriptor frame) const
    {
        checkItemType<T>();
        return getItemsInternal<T>(frame, getFrameId(frame));
    }  
    
    template<class T>
    const std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>> 
    TransformGraph::getItemsInternal(const vertex_descriptor frame, const FrameId& frameId) const
    {
        // T has to be ItemBase::PtrType<X> where X derives ItemBase for this method to work
        const Frame::ItemMap& items = graph()[frame].frame.items;
        const std::type_index key(typeid(T));
        
        if(items.find(key) == items.end())
        {
            ItemIterator<T> invalid;
            return std::make_pair(invalid, invalid);
        }
        
        auto begin = items.at(std::type_index(typeid(T))).begin();
        auto end = items.at(std::type_index(typeid(T))).end();
        assert(begin != end); //if a list exists it should not be empty
        
        ItemIterator<T> beginIt(begin, ItemBaseCaster<typename T::element_type>()); 
        ItemIterator<T> endIt(end, ItemBaseCaster<typename T::element_type>()); 
        return std::make_pair(beginIt, endIt);        
    }
    
    template <class T>
    const T TransformGraph::getItem(const FrameId& frame, const int i) const
    {
        checkItemType<T>();
        const vertex_descriptor vd = getVertex(frame); //may throw
        return getItem<T>(vd, i);

    }
    
    template <class T>
    const T TransformGraph::getItem(const vertex_descriptor frame, const int i) const
    {
        checkItemType<T>();
        const std::unordered_map<std::type_index, Frame::ItemList>& items = graph()[frame].frame.items;
        const std::type_index key(typeid(T));
        if(items.find(key) == items.end())
        {
            throw NoItemsOfTypeInFrameException(getFrameId(frame), typeid(T).name());
        }
        const Frame::ItemList& list = items.at(std::type_index(typeid(T)));
        assert(list.size() > 0); //if everything is implemented correctly empty lists can never exist in the map
        T casted = boost::dynamic_pointer_cast<typename T::element_type>(list.at(i)); //list.at(i) may throw std::out_of_range
        //the cast can only fail if there is a programming error somewhere in here...
        assert(nullptr != casted.get());
        return casted; 
    }
    
    template <class T>
    std::pair<TransformGraph::ItemIterator<T>, TransformGraph::ItemIterator<T>>
    TransformGraph::removeItemFromFrame(const FrameId& frameId, ItemIterator<T> item)
    {
        checkItemType<T>();
        checkFrameValid(frameId);

        Frame& frame = (*this)[frameId].frame;
        const std::type_index key(typeid(T));
        auto mapEntry = frame.items.find(key);
        if(mapEntry == frame.items.end())
        {
            throw NoItemsOfTypeInFrameException(frameId, key.name());
        }
        std::vector<ItemBase::Ptr>& items = mapEntry->second;
        std::vector<ItemBase::Ptr>::const_iterator baseIterator = item.base();
        //HACK This is a workaround for gcc bug 57158.
        //     In C++11 the parameter type of vector::erase changed from iterator
        //     to const_iterator (which is exactly what we need), but  gcc has not
        //     yet implemented that change. 
        std::vector<ItemBase::Ptr>::iterator nonConstBaseIterator = items.begin() + (baseIterator - items.cbegin()); //vector iterator const cast hack
        ItemBase::Ptr deletedItem = *nonConstBaseIterator;//backup item so we can notify the user
        std::vector<ItemBase::Ptr>::const_iterator next = items.erase(nonConstBaseIterator);

        ItemBase::Ptr baseItem = boost::dynamic_pointer_cast<ItemBase>(deletedItem);
        notify(ItemRemovedEvent(frameId, baseItem, key));
        
        ItemIterator<T> nextIt(next, ItemBaseCaster<typename T::element_type>()); 
        ItemIterator<T> endIt(items.cend(), ItemBaseCaster<typename T::element_type>()); 
        
        //remove the map entry if there are no more values in the vector
        if(nextIt == endIt)
        {
          //erase invalidates the iterators that we are about to return, but
          //that doesnt matter because it only happens when they both point
          //to end() anyway.
          frame.items.erase(key);
        }
        
        return std::make_pair(nextIt, endIt);
    }
    
    template <class T>
    void TransformGraph::removeItemFromFrame(const FrameId& frameId, ItemBase::PtrType<T> item)
    {
        static_assert(std::is_base_of<ItemBase, T>::value,
            "T does not derive from ItemBase"); 
        
        checkFrameValid(frameId);
        
        Frame& frame = (*this)[frameId].frame;
        const std::type_index key(typeid(ItemBase::PtrType<T>));
        auto mapEntry = frame.items.find(key);
        if(mapEntry == frame.items.end())
        {
            throw NoItemsOfTypeInFrameException(frameId, key.name());
        }
        std::vector<ItemBase::Ptr>& items = mapEntry->second;
        auto searchResult = std::find(items.begin(), items.end(), item);
        if(searchResult == items.end())
        {
            throw UnknownItemException(frameId, item->getID());
        }
        items.erase(searchResult);
        //remove the map entry if it does not contain any more items
        if(items.size() <= 0)
        {
            frame.items.erase(key);
        }
        ItemBase::Ptr baseItem = boost::dynamic_pointer_cast<ItemBase>(item);
        notify(ItemRemovedEvent(frameId, baseItem, key));
    }   
    
    template <class T>
    bool TransformGraph::containsItems(const FrameId& frameId) const
    {
        checkItemType<T>();
        const vertex_descriptor vd = getVertex(frameId); //may throw
        return containsItems<T>(vd);
    }
    
    template <class T>
    bool TransformGraph::containsItems(const vertex_descriptor vertex) const
    {
        checkItemType<T>();
        const std::type_index type(typeid(T));
        return containsItems(vertex, type);
    }
    
    template <class T>
    size_t TransformGraph::getItemCount(const FrameId& frameId) const
    {
        checkItemType<T>();
        vertex_descriptor vd = getVertex(frameId);
        return getItemCount<T>(vd);
    }
    
    template <class T>
    size_t TransformGraph::getItemCount(const vertex_descriptor vd) const
    {
        checkItemType<T>();
        const Frame& frame = graph()[vd].frame;
        const std::type_index key(typeid(T));
        auto mapEntry = frame.items.find(key);
        if(mapEntry == frame.items.end())
        {
            return 0;
        }
        else
        {
            return mapEntry->second.size();
        }
    }
    
}}
