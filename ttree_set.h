#ifndef STX_STX_TTREE_SET_H_HEADER
#define STX_STX_TTREE_SET_H_HEADER

/* ttree_set.h
 * Contains the specialized T tree template class tree_set
 * */

#include "ttree.h"
#include <stdlib.h>
namespace stx {
template <typename _Key,
		  typename _Compare = std::less<_Key>,
		  typename _Traits = ttree_default_set_traits<_Key>,
		  typename _Alloc = std::allocator<_Key> >	
class ttree_set
{
public:
	typedef _Key key_type;
	typedef _Compare key_compare;
	typedef _Traits traits;
	typedef _Alloc allocator_type;
private:
/// \internal The empty struct used as a placeholder for the data_type
	struct empty_struct 
	{};
public:
/// *** Constructed Types

	typedef struct empty_struct data_type;

	typedef key_type value_type;

	typedef ttree_set<key_type, key_compare, traits, allocator_type> self_type;

	typedef stx::CTtree ttree_impl;

//	typedef ttree_impl::iterator iterator;
    

private:
	ttree_impl tree;
    char buffer[16];
public:
	inline void insert(unsigned int x)
	{
        snprintf(buffer,sizeof(buffer),"%d",x);
		return tree.Insert(buffer, buffer);
	}

	inline int find(unsigned int x){
        
        snprintf(buffer,sizeof(buffer),"%d",x);
		return tree.Find(buffer);
	}
};
}
#endif
