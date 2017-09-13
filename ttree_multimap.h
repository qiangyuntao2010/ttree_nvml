#ifndef STX_STX_TTREE_MULTIMAP_H_HEADER
#define STX_STX_TTREE_MULTIMAP_H_HEADER

/* ttree_multimap.h
 * Contains the specialized T tree template class tree_set
 * */

#include "ttree.h"
#include <stdlib.h>
namespace stx {
template <typename _Key,
		  typename _Compare = std::less<_Key>,
		  typename _Traits = ttree_default_set_traits<_Key>,
		  typename _Alloc = std::allocator<_Key> >	
class ttree_multimap
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

	typedef std::pair<unsigned int, unsigned int> value_type;

	typedef ttree_multimap<key_type, key_compare, traits, allocator_type> self_type;

	typedef stx::CTtree ttree_impl;

//	typedef ttree_impl::iterator iterator;

private:
	ttree_impl tree;
    char k_buff[16];
    char v_buff[16];
public:
	inline void insert(unsigned int key, unsigned int value)
	{
        memset(k_buff,0,16);
        memset(k_buff,0,16);
        snprintf(k_buff,sizeof(k_buff),"%d",key);
		snprintf(v_buff,sizeof(v_buff),"%d",value);
        return tree.Insert(k_buff, v_buff);
	}

	inline void insert(const value_type& x)
	{
		return insert(x.first,x.second);
	}

	inline int find(unsigned int x){
		snprintf(k_buff,sizeof(k_buff),"%d",x);
		return tree.Find(k_buff);
	}

	inline void erase(unsigned int x){
		snprintf(k_buff,sizeof(k_buff),"%d",x);
		tree.Delete(k_buff);
	}
};
}
#endif
